#include "Renderer/VulkanRenderer.hpp"

#include <map>
#include <set>

#include "Core/App.hpp"
#include "Core/Debugging/Log.hpp"
#include "Core/Scene/Components/RenderModelComponent.hpp"

#include "Renderer/RendererVertex.hpp"
#include "Wrappers/Interfacing.hpp"

#include "Resources/ResourceManager.hpp"
#include "Resources/ShaderProgram.hpp"
#include "Resources/Model.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/StaticTexture.hpp"
#include "Resources/StaticCubeMap.hpp"
#include "Core/Scene/Components/Lights/DirectionalLightComponent.hpp"
#include "Core/Scene/Components/Lights/PointLightComponent.hpp"
#include "Core/Scene/Components/Lights/SpotLightComponent.hpp"
#include "LowRenderer/PostProcess/PostProcessEffect.hpp"

using namespace Renderer;

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanRenderer::VulkanRenderer()
{
}

VulkanRenderer::~VulkanRenderer()
{
}

bool VulkanRenderer::LoadShader(Resources::Shader* p_shader)
{
	return p_shader->GetRendererShader().LoadShader(p_shader->shaderData, device);
}

bool VulkanRenderer::LoadShaderProgram(Resources::ShaderProgram* p_shader, u32 targetPass)
{
	return LoadShaderProgram(p_shader, static_cast<LowRenderer::RenderPassType>(targetPass));
}

bool VulkanRenderer::LoadShaderProgram(Resources::ShaderProgram* p_shader, LowRenderer::RenderPassType targetPass)
{
	if (targetPass == LowRenderer::RenderPassType::ALL)
	{
		return CreateGraphicsPipeline(windowRenderPass, p_shader->program.vertex, p_shader->program.fragment, p_shader->program.pipeline, Renderer::PipelineParams::WINDOW);
	}
	else if (targetPass & LowRenderer::RenderPassType::OBJECT)
	{
		return CreateGraphicsPipeline(objectRenderPass, p_shader->program.vertex, p_shader->program.fragment, p_shader->program.pipeline);
	}
	else if (targetPass & LowRenderer::RenderPassType::LIGHT)
	{
		return 	CreateGraphicsPipeline(lightRenderPass, p_shader->program.vertex, p_shader->program.fragment, p_shader->program.pipeline, PipelineParams::LIGHT_PASS);
	}
	else if (targetPass & LowRenderer::RenderPassType::POST)
	{
		return CreateGraphicsPipeline(postRenderPass, p_shader->program.vertex, p_shader->program.fragment, p_shader->program.pipeline, static_cast<PipelineParams>(PipelineParams::POST_PROCESS));
	}
	else
	{
		return CreateGraphicsPipeline(geometryRenderPass, p_shader->program.vertex, p_shader->program.fragment, p_shader->program.pipeline,
			static_cast<PipelineParams>(PipelineParams::EXTRA_ATTACHMENT | PipelineParams::STENCIL | (targetPass & LowRenderer::RenderPassType::LINE ? PipelineParams::LINE : 0) | (targetPass & LowRenderer::RenderPassType::WIRE ? PipelineParams::WIREFRAME : 0) | (targetPass & LowRenderer::RenderPassType::DEPTH ? PipelineParams::DEPTH_CLEAR : 0)));
	}
}

bool VulkanRenderer::LoadModel(Resources::Model* p_model)
{
	bool isFull = true;
	for (Resources::Mesh* mesh : p_model->meshes)
	{
		if (!LoadMesh(mesh)) isFull = false;
	}
	return isFull;
}

Renderer::VertexBuffer VulkanRenderer::CreateVertexBuffer(const RendererVertex* pVertices, const unsigned int& pVerticesCount)
{
	VertexBuffer buffer{};

	VkDeviceSize bufferSize = sizeof(RendererVertex) * pVerticesCount;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;

	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	std::copy(pVertices, pVertices + pVerticesCount, reinterpret_cast<RendererVertex*>(data));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer.handle, buffer.memory);
	CopyBuffer(stagingBuffer, buffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	return buffer;
}

Renderer::IndiceBuffer VulkanRenderer::CreateIndiceBuffer(const u32* pIndices, const unsigned int& pIndicesCount)
{
	IndiceBuffer buffer{};

	VkDeviceSize bufferSize = sizeof(u32) * pIndicesCount;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;

	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	std::copy(pIndices, pIndices + pIndicesCount, reinterpret_cast<u32*>(data));
	vkUnmapMemory(device, stagingBufferMemory);

	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer.handle, buffer.memory);

	CopyBuffer(stagingBuffer, buffer.handle, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);


	return buffer;
}

bool VulkanRenderer::LoadMesh(Resources::Mesh* p_Mesh)
{
	if (!p_Mesh->GetVertexCount() || !p_Mesh->GetIndexCount()) return false;

	p_Mesh->rendererMesh.vertexBuffer = CreateVertexBuffer(p_Mesh->vertices.data(), p_Mesh->GetVertexCount());
	p_Mesh->rendererMesh.indexBuffer = CreateIndiceBuffer(p_Mesh->indices.data(), p_Mesh->GetIndexCount());

	p_Mesh->isLoaded = true;

	return true;
}

bool VulkanRenderer::LoadSkinnedModel(Resources::SkinnedModel* p_model)
{
	// TODO
	return false;
}

bool VulkanRenderer::LoadTexture(Resources::StaticTexture* p_texture)
{
	if (p_texture->resolution.x <= 0 || p_texture->resolution.y <= 0 || !p_texture->textureData)
	{
		return false;
	}
	VkBuffer stagingBuffer = {};
	VkDeviceMemory stagingBufferMemory = {};

	VkDeviceSize imageSize = (p_texture->isFloat ? sizeof(f32) * 4 : sizeof(u8) * 4) * p_texture->resolution.x * p_texture->resolution.y;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;

	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	if (p_texture->isFloat)
	{
		std::copy(reinterpret_cast<f32*>(p_texture->textureData), reinterpret_cast<f32*>(p_texture->textureData) + (imageSize / sizeof(f32)), static_cast<f32*>(data));
	}
	else
	{
		std::copy(reinterpret_cast<u8*>(p_texture->textureData), reinterpret_cast<u8*>(p_texture->textureData) + imageSize, static_cast<u8*>(data));
	}
	vkUnmapMemory(device, stagingBufferMemory);

	CreateImage(p_texture->resolution, p_texture->mipLevels, p_texture->renderTexture.textureImage, p_texture->renderTexture.textureImageMemory, p_texture->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	TransitionImageLayout(p_texture->renderTexture.textureImage, p_texture->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, p_texture->mipLevels);
	CopyBufferToImage(stagingBuffer, p_texture->renderTexture.textureImage, static_cast<u32>(p_texture->resolution.x), static_cast<u32>(p_texture->resolution.y));

	GenerateMipmaps(p_texture->renderTexture.textureImage, p_texture->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, p_texture->resolution, p_texture->mipLevels);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	p_texture->renderTexture.imageView.imageView = CreateImageView(p_texture->renderTexture.textureImage, p_texture->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, p_texture->mipLevels);
	//

	if (p_texture->shouldDeleteData)
		p_texture->DeleteTextureData();
	p_texture->renderTexture.CreateGuiView(Resources::TextureSampler::GetDefaultSampler()->renderSampler);
	return true;
}

bool VulkanRenderer::LoadCubeMap(Resources::StaticCubeMap* p_cubemap)
{
	if (p_cubemap->resolution.x <= 0 || p_cubemap->resolution.y <= 0 || !p_cubemap->cubeMapData)
	{
		return false;
	}
	VkBuffer stagingBuffer = {};
	VkDeviceMemory stagingBufferMemory = {};

	VkDeviceSize imageSize = (p_cubemap->isFloat ? sizeof(f32) * 4 * 6 : sizeof(u8) * 4 * 6) * p_cubemap->resolution.x * p_cubemap->resolution.y;

	CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

	void* data;

	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	if (p_cubemap->isFloat)
	{
		std::copy(reinterpret_cast<f32*>(p_cubemap->cubeMapData), reinterpret_cast<f32*>(p_cubemap->cubeMapData) + (imageSize / sizeof(f32)), static_cast<f32*>(data));
	}
	else
	{
		std::copy(reinterpret_cast<u8*>(p_cubemap->cubeMapData), reinterpret_cast<u8*>(p_cubemap->cubeMapData) + imageSize, static_cast<u8*>(data));
	}
	vkUnmapMemory(device, stagingBufferMemory);

	CreateImage(p_cubemap->resolution, p_cubemap->mipLevels, p_cubemap->renderTexture.textureImage, p_cubemap->renderTexture.textureImageMemory, p_cubemap->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

	TransitionImageLayout(p_cubemap->renderTexture.textureImage, p_cubemap->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, p_cubemap->mipLevels, true);
	CopyBufferToImage(stagingBuffer, p_cubemap->renderTexture.textureImage, static_cast<u32>(p_cubemap->resolution.x), static_cast<u32>(p_cubemap->resolution.y), true);

	GenerateMipmaps(p_cubemap->renderTexture.textureImage, p_cubemap->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, p_cubemap->resolution, p_cubemap->mipLevels, true);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	p_cubemap->renderTexture.imageView.imageView = CreateImageView(p_cubemap->renderTexture.textureImage, p_cubemap->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, p_cubemap->mipLevels);
	p_cubemap->cubeImageView.imageView.imageView = CreateImageView(p_cubemap->renderTexture.textureImage, p_cubemap->isFloat ? VK_FORMAT_R32G32B32A32_SFLOAT : VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, p_cubemap->mipLevels, true);

	if (p_cubemap->shouldDeleteData)
		p_cubemap->DeleteCubeMapData();
	p_cubemap->renderTexture.CreateGuiView(Resources::TextureSampler::GetDefaultSampler()->renderSampler);
	p_cubemap->cubeImageView.imageView.imGuiDS = p_cubemap->renderTexture.imageView.imGuiDS;
	return true;
}

void VulkanRenderer::LoadTextureSampler(Resources::TextureSampler* sampler, bool linearFiltering, bool wrap)
{
	sampler->renderSampler = RendererTextureSampler(device, linearFiltering ? VK_FILTER_LINEAR : VK_FILTER_NEAREST, wrap ? VK_SAMPLER_ADDRESS_MODE_REPEAT : VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}

std::string VulkanRenderer::ReadTexture(const Resources::Texture* tex, Maths::IVec2 offset, Maths::IVec2 size, u64 pixelSize)
{
	std::string result;
	u64 bufferSize = pixelSize * size.x * size.y;
	result.resize(bufferSize);
	VkBuffer stagingBuffer = {};
	VkDeviceMemory stagingBufferMemory = {};
	TransitionImageLayout(tex->GetRendererTexture().textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tex->mipLevels);
	CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
	CopyImageToBuffer(stagingBuffer, tex->GetRendererTexture().textureImage, offset, size);
	char* data = nullptr;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, reinterpret_cast<void**>(&data));
	std::copy(data, data + bufferSize, result.data());
	vkUnmapMemory(device, stagingBufferMemory);
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
	TransitionImageLayout(tex->GetRendererTexture().textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, tex->mipLevels);
	return result;
}

void VulkanRenderer::Init(void* window, Maths::IVec2 defaultRes, bool useVSync, bool pInitEditor)
{
	InitVulkan(static_cast<GLFWwindow*>(window), defaultRes, useVSync, pInitEditor);
}

void VulkanRenderer::CleanupBuffers()
{
	mainFB.DeleteData();
	objectFrameBuffer.DeleteFrameBuffer(device);
}

void VulkanRenderer::Cleanup()
{
	mainUniform.DestroyDescriptorSetLayout(device);
	lightUniform.DestroyDescriptorSetLayout(device);
	postUniform.DestroyDescriptorSetLayout(device);
	windowUniform.DestroyDescriptorSetLayout(device);

	CleanupSwapChain();

	for (u32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		descriptorPools[i].DestroyPools(device);
		uniformPools[i].DestroyPools(device);
		ldescriptorPools[i].DestroyPools(device);
		luniformPools[i].DestroyPools(device);
		pdescriptorPools[i].DestroyPools(device);
		puniformPools[i].DestroyPools(device);
		wdescriptorPools[i].DestroyPools(device);
	}

	vkDestroyRenderPass(device, windowRenderPass, nullptr);
	vkDestroyRenderPass(device, geometryRenderPass, nullptr);
	vkDestroyRenderPass(device, lightRenderPass, nullptr);
	vkDestroyRenderPass(device, postRenderPass, nullptr);
	vkDestroyRenderPass(device, objectRenderPass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, graphicCommandPool, nullptr);
	vkDestroyCommandPool(device, transferCommandPool, nullptr);
	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanRenderer::ResizeFrameBuffer(Maths::IVec2 newRes)
{
	if (targetResolution == newRes) return;
	targetResolution = newRes;
	shouldRecreateFB = true;
}

void VulkanRenderer::SetVSync(bool newValue)
{
	if (targetVSync == newValue) return;
	targetVSync = newValue;
	shouldRecreate = true;
}

void* VulkanRenderer::GetInstance()
{
	return &instance;
}

void* VulkanRenderer::GetSurface()
{
	return &surface;
}

LowRenderer::FrameBuffer VulkanRenderer::GetMainColorBuffer() const
{
	LowRenderer::FrameBuffer result;
	result.hash = 0;
	result.path = "main_framebuffer";
	result.isLoaded = true;
	result.resolution = targetResolution;
	if (shouldDeleteOldFB)
	{
		result.db = mainFB.old_db;
		result.lb[0] = mainFB.old_fb;
	}
	else
	{
		result.db = mainFB.db;
		result.lb[0] = mainFB.fb;
	}
	return result;
}

LowRenderer::FrameBuffer Renderer::VulkanRenderer::GetMainLightBuffer() const
{
	LowRenderer::FrameBuffer result;
	result.hash = 0;
	result.path = "light_framebuffer";
	result.isLoaded = true;
	result.resolution = targetResolution;
	if (shouldDeleteOldFB)
	{
		result.db = mainFB.old_db;
		result.lb[0] = mainFB.old_lb[mainFB.actualBuffer];
	}
	else
	{
		result.db = mainFB.db;
		result.lb[0] = mainFB.lb[mainFB.actualBuffer];
	}
	return result;
}

LowRenderer::FrameBuffer VulkanRenderer::GetMainObjectBuffer() const
{
	LowRenderer::FrameBuffer result;
	result.isLoaded = true;
	result.hash = 1;
	result.path = "object_framebuffer";
	result.resolution = targetResolution;
	if (shouldDeleteOldFB)
	{
		result.db = mainFB.old_db;
		result.lb[0] = oldObjectBuffer;
	}
	else
	{
		result.db = mainFB.db;
		result.lb[0] = objectFrameBuffer;
	}
	return result;
}

void VulkanRenderer::ResetMainFB()
{
	mainFB.ResetBuffer();
}

void VulkanRenderer::ToggleMainFB()
{
	mainFB.ToggleBuffer();
}

void Renderer::VulkanRenderer::SetCurrentCamera(const LowRenderer::Rendering::Camera* pCamera)
{
	this->currentCameraPos = pCamera->position;
	this->currentCamera = pCamera;
}

void VulkanRenderer::WaitForVSync()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
}

void VulkanRenderer::BeginFrame(Wrappers::Interfacing* pInterface)
{
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain(pInterface, swapChainExtent, targetVSync);
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to acquire swap chain image!");
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	vkResetFences(device, 1, &inFlightFences[currentFrame]);
	vkResetCommandBuffer(commandBuffers[currentFrame], 0);

	descriptorPools[currentFrame].UpdatePool(device, *this);
	uniformPools[currentFrame].UpdatePool();
	ldescriptorPools[currentFrame].UpdatePool(device, *this);
	luniformPools[currentFrame].UpdatePool();
	pdescriptorPools[currentFrame].UpdatePool(device, *this);
	puniformPools[currentFrame].UpdatePool();
	wdescriptorPools[currentFrame].UpdatePool(device, *this);
	BeginCommandBuffer();
	swapBuffer.fb = swapChainFramebuffers[imageIndex];
	swapBuffer.lb[0] = swapChainFramebuffers[imageIndex];
	activePassParams = LowRenderer::RenderPassType::DEFAULT;
	BeginRenderPass(windowRenderPass, &swapBuffer);
	EndRenderPass();
}

void VulkanRenderer::EndFrame(Wrappers::Interfacing* pInterface)
{
	if (!pInterface)
	{
		swapBuffer.fb = swapChainFramebuffers[imageIndex];
		BeginPass(&swapBuffer, LowRenderer::RenderPassType::ALL);
		RenderToWindow();
		EndRenderPass();
	}

	EndCommandBuffer();

	if (pInterface)
		SubmitCurrentCommandBuffer(pInterface->RecordImGuiCommandBuffer(imageIndex));
	else
		SubmitCurrentCommandBuffer(nullptr);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };

	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional
	VkResult result = vkQueuePresentKHR(presentQueue, &presentInfo);
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	if (shouldDeleteOldFB)
	{
		WaitIdle();
		shouldDeleteOldFB = false;
		mainFB.old_fb.DeleteFrameBuffer(device);
		oldObjectBuffer.DeleteFrameBuffer(device);
		mainFB.old_db.DeleteDepthBuffer(device);
		mainFB.old_nb.DeleteBuffer(device);
		mainFB.old_pb.DeleteBuffer(device);
		for (u8 i = 0; i < 3; i++)
		{
			mainFB.old_lb[i].DeleteFrameBuffer(device);
			mainFB.old_gb[i].DeleteBuffer(device);
		}
	}
	if (shouldRecreateFB)
	{
		shouldRecreateFB = false;
		mainFB.old_fb = mainFB.fb;
		oldObjectBuffer = objectFrameBuffer;
		mainFB.old_db = mainFB.db;
		mainFB.old_nb = mainFB.nb;
		mainFB.old_pb = mainFB.pb;
		shouldDeleteOldFB = true;
		mainFB.db = RendererDepthBuffer(targetResolution.x, targetResolution.y, device, true);
		mainFB.nb = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, false);
		mainFB.pb = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, false);
		CreateMainFrameBuffer();
		std::vector<RendererImageView> vec;
		vec.push_back(mainFB.db.GetDepthImageView());
		CreateFrameBuffer(objectFrameBuffer, vec, targetResolution, Maths::Vec4(), VK_FORMAT_R32_UINT, Resources::ShaderVariant::Object);
		for (u8 i = 0; i < 3; i++)
		{
			mainFB.old_lb[i] = mainFB.lb[i];
			mainFB.old_gb[i] = mainFB.gb[i];
			mainFB.gb[i] = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, true);
			vec.clear();
			vec.push_back(mainFB.gb[i].GetImageView());
			CreateFrameBuffer(mainFB.lb[i], vec, targetResolution, Maths::Vec4(), VK_FORMAT_R32G32B32A32_SFLOAT, Resources::ShaderVariant::Light);
		}
	}
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || shouldRecreate)
	{
		shouldRecreate = false;
		RecreateSwapChain(pInterface, swapChainExtent, targetVSync);
	}
	else if (result != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to present swap chain image!");
		throw std::runtime_error("Failed to present swap chain image!");
	}
}

void VulkanRenderer::BeginPass(LowRenderer::FrameBuffer* fb, LowRenderer::RenderPassType renderPass)
{
	if (!fb) fb = &mainFB;
	activePassParams = renderPass;
	if (renderPass == LowRenderer::RenderPassType::ALL)
	{
		BeginRenderPass(windowRenderPass, fb);
		activePassParams = LowRenderer::RenderPassType::DEFAULT;
	}
	else if (renderPass & LowRenderer::RenderPassType::OBJECT)
	{
		BeginRenderPass(objectRenderPass, fb);
	}
	else if (renderPass & LowRenderer::RenderPassType::LIGHT)
	{
		BeginRenderPass(lightRenderPass, fb);
	}
	else if (renderPass & LowRenderer::RenderPassType::POST)
	{
		BeginRenderPass(postRenderPass, fb);
	}
	else
	{
		BeginRenderPass(geometryRenderPass, fb);
	}
}

void VulkanRenderer::EndPass()
{
	EndRenderPass();
}

Maths::Vec3 VulkanRenderer::GetClearColor()
{
	return mainFB.ClearColor.GetVector();
}

void VulkanRenderer::SetClearColor(Maths::Vec4 color)
{
	mainFB.ClearColor = color;
	mainFB.UpdateClearColor();
}

void Renderer::VulkanRenderer::RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, const Resources::Material* mat)
{
	if (!mat) return;
	VkDescriptorSet desc = descriptorPools[currentFrame].GetNext(*this);
	auto uniform = uniformPools[currentFrame].GetNext();
	std::vector<const RendererTexture*> textures;
	textures.push_back(&(mat->albedo ? mat->albedo : Resources::StaticTexture::GetDefaultTexture())->GetRendererTexture());
	textures.push_back(&(mat->normal ? mat->normal : Resources::StaticTexture::GetDefaultNormal())->GetRendererTexture());
	textures.push_back(&(mat->height ? mat->height : Resources::StaticTexture::GetDefaultTexture())->GetRendererTexture());
	UpdateDescriptorSet(desc, uniform.GetBuffer(), &mainUniform, true, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateUniformBuffer(uniform, mat, m, mvp);

	DrawIndexedMesh(mesh, desc, uniform);
}

void Renderer::VulkanRenderer::RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, std::vector<const RendererTexture*> textures, Resources::Material* materialOverride)
{
	VkDescriptorSet desc = descriptorPools[currentFrame].GetNext(*this);
	auto uniform = uniformPools[currentFrame].GetNext();
	UpdateDescriptorSet(desc, uniform.GetBuffer(), &mainUniform, true, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateUniformBuffer(uniform, materialOverride, m, mvp);

	DrawIndexedMesh(mesh, desc, uniform);
}

void VulkanRenderer::RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, const Maths::Vec3& color)
{
	Resources::Material mat;
	mat.ambientColor = color;
	RenderMesh(mesh, m, mvp, &mat);
}

void VulkanRenderer::RenderMeshObject(const Resources::Mesh* mesh, const Maths::Mat4& mvp, u32 objectID)
{
	VkDescriptorSet desc = descriptorPools[currentFrame].GetNext(*this);
	auto uniform = uniformPools[currentFrame].GetNext();
	std::vector<const RendererTexture*> textures;
	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultNormal()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
	UpdateDescriptorSet(desc, uniform.GetBuffer(), &mainUniform, true, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateUniformBuffer(uniform, mvp, objectID);

	DrawIndexedMesh(mesh, desc, uniform);
}

void VulkanRenderer::DrawVertices(u32 count, const Maths::Mat4& m, const Maths::Mat4& mvp)
{
	std::vector<const RendererTexture*> textures;
	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultNormal()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
	DrawVertices(count, m, mvp, textures);
}

void Renderer::VulkanRenderer::DrawVertices(u32 count, const Maths::Mat4& m, const Maths::Mat4& mvp, std::vector<const RendererTexture*> textures)
{
	VkDescriptorSet desc = descriptorPools[currentFrame].GetNext(*this);
	auto uniform = uniformPools[currentFrame].GetNext();
	UpdateDescriptorSet(desc, uniform.GetBuffer(), &mainUniform, true, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateUniformBuffer(uniform, Resources::Material::GetDefaultMaterial(), m, mvp);

	std::array<u32, 2> uniformOffsets = { static_cast<u32>(uniform.GetOffset() * mainUniform.GetTotalOffset()), static_cast<u32>(uniform.GetOffset() * mainUniform.GetTotalOffset()) };
	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &desc, 2, uniformOffsets.data());
	vkCmdDraw(activeCommandBuffer, count, 1, 0, 0);
}

void VulkanRenderer::DrawIndexedRenderMesh(const Renderer::RendererMesh* pRenderMesh, const Maths::Mat4& pModelMatrix, u32 pVertexCount, u32 pIndiceCount)
{
	VkDescriptorSet desc = descriptorPools[currentFrame].GetNext(*this);

	std::vector<const RendererTexture*> textures;

	textures.reserve(3);

	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultNormal()->GetRendererTexture());
	textures.push_back(&Resources::StaticTexture::GetDefaultTexture()->GetRendererTexture());

	auto uniform = uniformPools[currentFrame].GetNext();

	Maths::Mat4 mvp = this->currentCamera->GetProjectionMatrix() * this->currentCamera->GetViewMatrix() * pModelMatrix;

	UpdateDescriptorSet(desc, uniform.GetBuffer(), &mainUniform, true, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateUniformBuffer(uniform, Resources::Material::GetDefaultMaterial(), pModelMatrix, mvp);

	std::array<u32, 2> uniformOffsets = { static_cast<u32>(uniform.GetOffset() * mainUniform.GetTotalOffset()), static_cast<u32>(uniform.GetOffset() * mainUniform.GetTotalOffset()) };

	VkDeviceSize vertexbufferOffset = 0;

	vkCmdBindVertexBuffers(activeCommandBuffer, 0, 1, &pRenderMesh->vertexBuffer.handle, &vertexbufferOffset);

	if (pIndiceCount > 0)
		vkCmdBindIndexBuffer(activeCommandBuffer, pRenderMesh->indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &desc, 2, uniformOffsets.data());

	if (pIndiceCount > 0)
		vkCmdDrawIndexed(activeCommandBuffer, pIndiceCount, 1, 0, 0, 0);
	else
		vkCmdDraw(activeCommandBuffer, pVertexCount, 1, 0, 0);
}

void VulkanRenderer::ApplyLightPass(const Resources::ShaderProgram* shader)
{
	if (!shader) return;
	BindShader(shader->GetShader());
	VkDescriptorSet desc = ldescriptorPools[currentFrame].GetNext(*this, Resources::ShaderVariant::Light);
	auto elem = luniformPools[currentFrame].GetNext();
	std::vector<const RendererTexture*> textures;
	if ((activeFrameBuffer == &mainFB && shouldDeleteOldFB) || activeFrameBuffer->resize > 3)
	{
		textures.push_back(&activeFrameBuffer->old_fb.rendererTex);
		textures.push_back(&activeFrameBuffer->old_nb.texture);
		textures.push_back(&activeFrameBuffer->old_pb.texture);
	}
	else
	{
		textures.push_back(&activeFrameBuffer->fb.rendererTex);
		textures.push_back(&activeFrameBuffer->nb.texture);
		textures.push_back(&activeFrameBuffer->pb.texture);
	}
	UpdateDescriptorSet(desc, elem.GetBuffer(), &lightUniform, false, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdateLightUniformBuffer(elem);
	std::array<u32, 1> uniformOffsets = { static_cast<u32>(elem.GetOffset() * lightUniform.GetTotalOffset()) };
	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &desc, 1, uniformOffsets.data());
	vkCmdDraw(activeCommandBuffer, 3, 1, 0, 0);
}

void VulkanRenderer::ApplyPostProcess(const LowRenderer::PostProcess::PostProcessEffect* effect)
{
	if (!effect->GetShader()) return;
	VkDescriptorSet desc = pdescriptorPools[currentFrame].GetNext(*this, Resources::ShaderVariant::PostProcess);
	auto elem = puniformPools[currentFrame].GetNext();
	std::vector<const RendererTexture*> textures;
	textures.push_back(&activeFrameBuffer->lb[activeFrameBuffer->actualBuffer].rendererTex);
	textures.push_back(&activeFrameBuffer->gb[activeFrameBuffer->actualBuffer].texture);
	UpdateDescriptorSet(desc, elem.GetBuffer(), &postUniform, false, textures, Resources::TextureSampler::GetDefaultSampler());
	UpdatePostUniformBuffer(elem, effect);
	BindShader(effect->GetShader()->GetShader());
	std::array<u32, 1> uniformOffsets = { static_cast<u32>(elem.GetOffset() * postUniform.GetTotalOffset()) };
	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &desc, 1, uniformOffsets.data());
	vkCmdDraw(activeCommandBuffer, 3, 1, 0, 0);
}


void VulkanRenderer::RenderToWindow()
{
	static Resources::ShaderProgram* windowShader = Core::App::GetInstance()->GetResources().Get<Resources::ShaderProgram>(0x6b);

	VkDescriptorSet desc = wdescriptorPools[currentFrame].GetNext(*this, Resources::ShaderVariant::Window);
	RendererTexture* texture;
	if ((shouldDeleteOldFB) || activeFrameBuffer->resize > 3)
	{
		texture = &mainFB.old_lb[activeFrameBuffer->actualBuffer].rendererTex;
	}
	else
	{
		texture = &mainFB.lb[activeFrameBuffer->actualBuffer].rendererTex;
	}
	UpdateDescriptorSet(desc, texture);
	BindShader(windowShader->GetShader());
	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &desc, 0, nullptr);
	vkCmdDraw(activeCommandBuffer, 3, 1, 0, 0);
}

void VulkanRenderer::BindShader(const Resources::ShaderProgram* p_shader)
{
	Resources::ShaderVariant variant = Resources::ShaderVariant::Default;
	if (activePassParams & (LowRenderer::RenderPassType::SHADOWMAP & LowRenderer::RenderPassType::CUBEMAP)) variant = Resources::ShaderVariant::ShadowCube;
	else if (activePassParams & LowRenderer::RenderPassType::SHADOWMAP) variant = Resources::ShaderVariant::Shadow;
	else if (activePassParams & LowRenderer::RenderPassType::CUBEMAP) variant = Resources::ShaderVariant::Cube;
	else if (activePassParams & LowRenderer::RenderPassType::OBJECT) variant = Resources::ShaderVariant::Object;
	else if (activePassParams & LowRenderer::RenderPassType::HALO) variant = Resources::ShaderVariant::Halo;
	if (p_shader->type == Resources::ShaderVariant::WireFrame)
	{
		activePassParams = static_cast<LowRenderer::RenderPassType>(activePassParams | LowRenderer::RenderPassType::WIRE);
	}
	else
	{
		activePassParams = static_cast<LowRenderer::RenderPassType>(activePassParams & (~LowRenderer::RenderPassType::WIRE));
	}
	activePipeline = &(p_shader->GetShader(variant)->program.pipeline);
	vkCmdBindPipeline(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipeline);

	if (activePassParams & LowRenderer::RenderPassType::WIRE)
	{
		vkCmdSetLineWidth(activeCommandBuffer, currentWidth);
	}
	if (p_shader->type != Resources::ShaderVariant::Window && !(activePassParams & (LowRenderer::RenderPassType::SHADOWMAP | LowRenderer::RenderPassType::OBJECT | LowRenderer::RenderPassType::HALO | LowRenderer::RenderPassType::LIGHT | LowRenderer::RenderPassType::POST)))
	{
		vkCmdSetDepthWriteEnable(activeCommandBuffer, state == StencilState::GREATER ? VK_FALSE : VK_TRUE);
		vkCmdSetStencilReference(activeCommandBuffer, VK_STENCIL_FRONT_AND_BACK, stencilCompareValue);
		switch (state)
		{
		case Renderer::StencilState::DEFAULT:
			vkCmdSetStencilOp(activeCommandBuffer, VK_STENCIL_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL);
			break;
		case Renderer::StencilState::INCREMENT:
			vkCmdSetStencilOp(activeCommandBuffer, VK_STENCIL_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_INCREMENT_AND_CLAMP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL);
			break;
		case Renderer::StencilState::NO_DEPTH:
			vkCmdSetStencilOp(activeCommandBuffer, VK_STENCIL_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_EQUAL);
			break;
		case Renderer::StencilState::GREATER:
			vkCmdSetStencilOp(activeCommandBuffer, VK_STENCIL_FRONT_AND_BACK, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_REPLACE, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_LESS);
			break;
		default:
			LOG(DEBUG_LEVEL::LERROR, "Invalid stencil state %d !", state);
			break;
		}
	}

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<f32>(activeFrameBuffer->GetResolution().x);
	viewport.height = static_cast<f32>(activeFrameBuffer->GetResolution().y);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(activeCommandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent.width = (u32)activeFrameBuffer->GetResolution().x;
	scissor.extent.height = (u32)activeFrameBuffer->GetResolution().y;
	vkCmdSetScissor(activeCommandBuffer, 0, 1, &scissor);

	if (activePassParams & LowRenderer::RenderPassType::SHADOWMAP) vkCmdSetDepthBias(activeCommandBuffer, 0.0f, 0.0f, 0.0f);
}

#pragma region
void VulkanRenderer::UnLoadShader(Resources::Shader* p_shader)
{
	p_shader->GetRendererShader().DeleteShader(device);
}

void VulkanRenderer::UnLoadShaderProgram(Resources::ShaderProgram* p_shader)
{
	vkDestroyPipeline(device, p_shader->program.pipeline.pipeline, nullptr);
	vkDestroyPipelineLayout(device, p_shader->program.pipeline.pipelineLayout, nullptr);

	p_shader->program.pipeline.pipeline			= VK_NULL_HANDLE;
	p_shader->program.pipeline.pipelineLayout	= VK_NULL_HANDLE;
}

void VulkanRenderer::UnLoadModel(Resources::Model* p_model)
{
	// TODO
}

void VulkanRenderer::UnLoadTexture(Resources::StaticTexture* p_texture)
{
	vkDestroyImageView(device, p_texture->renderTexture.imageView.imageView, nullptr);
	vkDestroyImage(device, p_texture->renderTexture.textureImage, nullptr);
	vkFreeMemory(device, p_texture->renderTexture.textureImageMemory, nullptr);

	p_texture->renderTexture.textureImageMemory		= VK_NULL_HANDLE;
	p_texture->renderTexture.imageView.imageView	= VK_NULL_HANDLE;
	p_texture->renderTexture.textureImage			= VK_NULL_HANDLE;
}

void VulkanRenderer::UnLoadTextureSampler(Resources::TextureSampler* sampler)
{
	sampler->renderSampler.DeleteTextureSampler(device);
}

void VulkanRenderer::UnLoadCubeMap(Resources::StaticCubeMap* p_cubemap)
{
	vkDestroyImageView(device, p_cubemap->renderTexture.imageView.imageView, nullptr);
	vkDestroyImageView(device, p_cubemap->cubeImageView.imageView.imageView, nullptr);
	vkDestroyImage(device, p_cubemap->renderTexture.textureImage, nullptr);
	vkFreeMemory(device, p_cubemap->renderTexture.textureImageMemory, nullptr);

	p_cubemap->renderTexture.imageView.imageView = VK_NULL_HANDLE;
	p_cubemap->cubeImageView.imageView.imageView = VK_NULL_HANDLE;
	p_cubemap->renderTexture.textureImage = VK_NULL_HANDLE;
	p_cubemap->renderTexture.textureImageMemory = VK_NULL_HANDLE;
}

void VulkanRenderer::UnLoadSkinnedModel(Resources::SkinnedModel* p_model)
{
	// TODO
}

void VulkanRenderer::UnloadMesh(Resources::Mesh* pMesh)
{
	FreeVertexBuffer(pMesh->rendererMesh.vertexBuffer);
	FreeIndiceBuffer(pMesh->rendererMesh.indexBuffer);
}

void VulkanRenderer::FreeVertexBuffer(VertexBuffer& pBuffer)
{
	vkFreeMemory(device, pBuffer.memory, nullptr);
	vkDestroyBuffer(device, pBuffer.handle, nullptr);
}

void VulkanRenderer::FreeIndiceBuffer(IndiceBuffer& pBuffer)
{
	vkFreeMemory(device, pBuffer.memory, nullptr);
	vkDestroyBuffer(device, pBuffer.handle, nullptr);
}

#pragma endregion Unload Functions

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanRenderer::CleanupSwapChain()
{
	vkDeviceWaitIdle(device);
	for (auto framebuffer : swapChainFramebuffers)
	{
		framebuffer.DeleteFrameBuffer(device, false);
	}
	vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void VulkanRenderer::PreInit()
{
	CreateInstance();
	SetupDebugMessenger();
}

void VulkanRenderer::InitVulkan(GLFWwindow* window, Maths::IVec2 defaultResolution, bool defaultVSync, bool pInitEditor)
{
	if(!pInitEditor)
		enableValidationLayers = false;
	targetResolution = defaultResolution;
	targetVSync = defaultVSync;
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain(VkExtent2D{ (u32)defaultResolution.x, (u32)defaultResolution.y }, defaultVSync);
	CreateImageViews();

	VkImageLayout windowImageLayout = pInitEditor ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	
	CreateRenderPass(windowRenderPass, windowImageLayout, swapChainImageFormat, 1, false, false, true);

	CreateRenderPass(geometryRenderPass, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R32G32B32A32_SFLOAT, 3, true, true, true);
	CreateRenderPass(lightRenderPass, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R32G32B32A32_SFLOAT, 2, false, false, false);
	CreateRenderPass(postRenderPass, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R32G32B32A32_SFLOAT, 2, false, false, false);
	CreateRenderPass(objectRenderPass, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_FORMAT_R32_UINT, 1);
	//CreateShadowPass();
	mainUniform.CreateDescriptorSetLayout(device, physicalDevice);
	lightUniform.CreateDescriptorSetLayout(device, physicalDevice);
	postUniform.CreateDescriptorSetLayout(device, physicalDevice);
	windowUniform.CreateDescriptorSetLayout(device, physicalDevice);

	//CreateShadowPipeline();
	CreateCommandPool(&graphicCommandPool, queueFamilyIndices.graphicsFamily.value());
	CreateCommandPool(&transferCommandPool, queueFamilyIndices.transferFamily.value());
	
}

void VulkanRenderer::PickPhysicalDevice()
{
	u32 deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to find GPUs with Vulkan support!");
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	// Use an ordered map to automatically sort candidates by increasing score
	std::multimap<s32, VkPhysicalDevice> candidates;
	for (const auto& device : devices)
	{
		u32 score = RateDeviceSuitability(device);
		candidates.insert(std::make_pair(score, device));
	}

	// Check if the best candidate is suitable at all
	if (candidates.rbegin()->first > 0)
	{
		physicalDevice = candidates.rbegin()->second;
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(candidates.rbegin()->second, &deviceProperties);
		LOG(DEBUG_LEVEL::LDEFAULT, "selected Device: %s", deviceProperties.deviceName);
		lineRange = Maths::Vec2(deviceProperties.limits.lineWidthRange[0], deviceProperties.limits.lineWidthRange[1]);
		if (lineRange.x > 1.0f || lineRange.y < 1.0f) currentWidth = lineRange.x;
		LOG(DEBUG_LEVEL::LINFO, "Line width range : [%f, %f]", deviceProperties.limits.lineWidthRange[0], deviceProperties.limits.lineWidthRange[1]);
	}
	else
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to find a suitable GPU!");
		throw std::runtime_error("Failed to find a suitable GPU!");
	}
}

u32 VulkanRenderer::RateDeviceSuitability(VkPhysicalDevice device)
{
	if (!CheckDeviceExtensionSupport(device)) return 0;
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
	if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) return 0;

	QueueFamilyIndices indices = FindQueueFamilies(device);

	if (!indices.isComplete()) return 0;

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (!deviceFeatures.geometryShader || !deviceFeatures.samplerAnisotropy || !deviceFeatures.fillModeNonSolid || !deviceFeatures.wideLines || !deviceFeatures.shaderFloat64)
	{
		return 0;
	}

	s32 score = 0;
	// Discrete GPUs have a significant performance advantage
	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		score += 1000;
	}
	// Maximum possible size of textures affects graphics quality
	score += deviceProperties.limits.maxImageDimension2D;

	LOG(DEBUG_LEVEL::LINFO, "Device: %s", deviceProperties.deviceName);
	LOG(DEBUG_LEVEL::LINFO, "Score:  %d", score);
	return score;
}

QueueFamilyIndices VulkanRenderer::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	u32 queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	s32 i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport)
		{
			indices.presentFamily = i;
		}
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}
		else if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			indices.transferFamily = i;
		}
		if (indices.isComplete())
		{
			break;
		}
		i++;
	}
	return indices;
}

void VulkanRenderer::CreateLogicalDevice()
{
	queueFamilyIndices = FindQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<u32> uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.transferFamily.value(), queueFamilyIndices.presentFamily.value() };

	f32 queuePriority = 1.0f;
	for (u32 queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.wideLines = VK_TRUE;
	deviceFeatures.shaderFloat64 = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<u32>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.transferFamily.value(), 0, &transferQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue);
}

void VulkanRenderer::CreateInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		LOG(DEBUG_LEVEL::LERROR, "Validation layers requested, but not available!");
		throw std::runtime_error("Validation layers requested, but not available!");
	}
	u32 extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
#ifndef NDEBUG
	LOG(DEBUG_LEVEL::LINFO, "available extensions:");
	for (const auto& extension : extensions)
	{
		LOG(DEBUG_LEVEL::LINFO, "\t%s", extension.extensionName);
	}
#endif

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "NAT Engine";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "NAT Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char*> requiredExtensions = GetRequiredExtensions();

	// Define probably incorrect, but no apples in sight...
#ifdef MAC_OS
	requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif // MAC_OS

	for (const auto& ext : requiredExtensions)
	{
		bool found = false;
		for (const auto& l : extensions)
		{
			if (!std::string(l.extensionName).compare(ext))
			{
				found = true;
				break;
			}
		}
		if (found) continue;
		LOG(DEBUG_LEVEL::LERROR, "Missing required extension %s", ext);
		throw std::runtime_error(std::string("Missing required extension") + ext);
	}

	createInfo.enabledExtensionCount = (u32)requiredExtensions.size();
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

	if (result != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create instance! Error Code %d", result);
		throw std::runtime_error("Failed to create instance!");
	}
}

bool VulkanRenderer::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (std::string layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (!layerName.compare(layerProperties.layerName))
			{
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}

bool VulkanRenderer::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	u32 extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

void VulkanRenderer::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = Core::Debugging::Log::debugCallback;
}

void VulkanRenderer::SetupDebugMessenger()
{
	if (!enableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to set up Vulkan debug messenger!");
		throw std::runtime_error("Failed to set up Vulkan debug messenger!");
	}
}

std::vector<const char*> VulkanRenderer::GetRequiredExtensions()
{
	u32 glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

SwapChainSupportDetails VulkanRenderer::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	u32 formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	u32 presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR VulkanRenderer::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}
	LOG(DEBUG_LEVEL::LWARNING, "No good format found, taking the first one");
	return availableFormats[0];
}

VkPresentModeKHR VulkanRenderer::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync)
{
	if (vsync) return VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) // VK_PRESENT_MODE_IMMEDIATE_KHR
		{
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D resolution)
{
	if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		VkExtent2D actualExtent =
		{
			resolution
		};

		actualExtent.width = Maths::Util::UClamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = Maths::Util::UClamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

void VulkanRenderer::CreateSwapChain(VkExtent2D resolution, bool defaultVSync)
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(physicalDevice);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes, defaultVSync);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, resolution);
	imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	//createInfo.oldSwapchain;
	QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
	u32 queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = sizeof(queueFamilyIndices) / sizeof(u32);
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create swap chain!");
		throw std::runtime_error("Failed to create swap chain!");
	}
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainFramebuffers.resize(imageCount);
	std::vector<VkImage> images;
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());
	for (u32 i = 0; i < imageCount; i++)
	{
		swapChainFramebuffers[i].rendererTex.textureImage = images[i];
	}
	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VulkanRenderer::InitRendererData()
{
	mainFB.db = RendererDepthBuffer(targetResolution.x, targetResolution.y, device, true);
	mainFB.nb = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, false);
	mainFB.pb = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, false);
	CreateSCFramebuffers();
	CreateMainFrameBuffer();
	mainFB.renderer = this;
	mainFB.fb.rendererTex.isValid = false;
	std::vector<RendererImageView> vec;
	vec.push_back(mainFB.db.GetDepthImageView());
	CreateFrameBuffer(objectFrameBuffer, vec, targetResolution, Maths::Vec4(), VK_FORMAT_R32_UINT, Resources::ShaderVariant::Object);
	for (u8 i = 0; i < 3; i++)
	{
		mainFB.gb[i] = RendererBuffer(targetResolution.x, targetResolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, true);
		vec.clear();
		vec.push_back(mainFB.gb[i].GetImageView());
		CreateFrameBuffer(mainFB.lb[i], vec, targetResolution, Maths::Vec4(), VK_FORMAT_R32G32B32A32_SFLOAT, Resources::ShaderVariant::Light);
	}
	descriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
	uniformPools.resize(MAX_FRAMES_IN_FLIGHT);
	ldescriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
	luniformPools.resize(MAX_FRAMES_IN_FLIGHT);
	pdescriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
	puniformPools.resize(MAX_FRAMES_IN_FLIGHT);
	wdescriptorPools.resize(MAX_FRAMES_IN_FLIGHT);
	for (s32 i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		descriptorPools[i] = FrameDescriptorPool();
		descriptorPools[i].CreatePools(*this);
		uniformPools[i] = UniformBufferPool();
		uniformPools[i].CreatePools(*this, 1024, &mainUniform);
		ldescriptorPools[i] = FrameDescriptorPool();
		ldescriptorPools[i].CreatePools(*this, 32, 1, 3);
		luniformPools[i] = UniformBufferPool();
		luniformPools[i].CreatePools(*this, 32, &lightUniform);
		pdescriptorPools[i] = FrameDescriptorPool();
		pdescriptorPools[i].CreatePools(*this, 1024, 1, 2);
		puniformPools[i] = UniformBufferPool();
		puniformPools[i].CreatePools(*this, 1024, &postUniform);
		wdescriptorPools[i] = FrameDescriptorPool();
		wdescriptorPools[i].CreatePools(*this, 1024, 0, 1);
	}

	CreateSyncObjects();
	CreateCommandBuffers();
}

void VulkanRenderer::CreateImageViews()
{
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		swapChainFramebuffers[i].rendererTex.imageView.imageView = CreateImageView(swapChainFramebuffers[i].rendererTex.textureImage, swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
}

bool VulkanRenderer::CreateGraphicsPipeline(VkRenderPass& targetPass, const VertexRendererShader* vertex, const FragmentRendererShader* fragment, RendererPipeline& pipeline, PipelineParams params)
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vertex->GetStageInfo(), fragment->GetStageInfo() };

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	if (params & PipelineParams::STENCIL)
	{
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
	}
	if (params & PipelineParams::WIREFRAME) dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (params & PipelineParams::SHADOW) dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	auto bindingDescription = RendererVertex::GetBindingDescription();
	auto attributeDescriptions = RendererVertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = params & (PipelineParams::LIGHT_PASS | PipelineParams::POST_PROCESS | PipelineParams::WINDOW) ? 0 : 1;
	vertexInputInfo.vertexAttributeDescriptionCount = params & (PipelineParams::LIGHT_PASS | PipelineParams::POST_PROCESS | PipelineParams::WINDOW) ? 0 : static_cast<u32>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = params & PipelineParams::LINE ? VK_PRIMITIVE_TOPOLOGY_LINE_LIST: VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (f32)swapChainExtent.width;
	viewport.height = (f32)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = params & PipelineParams::WIREFRAME ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = params & PipelineParams::SHADOW ? VK_CULL_MODE_FRONT_BIT : (params & (PipelineParams::LINE | PipelineParams::WIREFRAME) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT);
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = params & PipelineParams::SHADOW ? VK_TRUE : VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	std::array<VkPipelineColorBlendAttachmentState, 3> colorBlendAttachment;
	for (u32 i = 0; i < colorBlendAttachment.size(); i++)
	{
		colorBlendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment[i].blendEnable = VK_FALSE;
		colorBlendAttachment[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment[i].colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment[i].alphaBlendOp = VK_BLEND_OP_ADD; // Optional
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = params & PipelineParams::SHADOW ? 0 : (params & (PipelineParams::LIGHT_PASS | PipelineParams::POST_PROCESS) ? 2 : (params & PipelineParams::EXTRA_ATTACHMENT ? static_cast<u32>(colorBlendAttachment.size()) : 1));
	colorBlending.pAttachments = colorBlendAttachment.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = params & PipelineParams::LIGHT_PASS ? &lightUniform.GetLayout() : (params & PipelineParams::POST_PROCESS ? &postUniform.GetLayout() : (params & PipelineParams::WINDOW ? &windowUniform.GetLayout() : &mainUniform.GetLayout()));

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipeline.pipelineLayout) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create pipeline layout!");
		return false;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = params & PipelineParams::DEPTH_CLEAR ? VK_COMPARE_OP_ALWAYS : VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = params & PipelineParams::STENCIL ? VK_TRUE : VK_FALSE;
	depthStencil.back.compareOp = VK_COMPARE_OP_EQUAL;
	depthStencil.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencil.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencil.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencil.back.compareMask = 0xff;
	depthStencil.back.writeMask = 0xff;
	depthStencil.back.reference = 0;
	depthStencil.front = depthStencil.back;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<u32>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipeline.pipelineLayout;
	pipelineInfo.renderPass = targetPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional
	Core::Debugging::Log::ClearErrors();
	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline.pipeline) != VK_SUCCESS || Core::Debugging::Log::HasError() || Core::Debugging::Log::HasWarnings())
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create pipeline!");
		return false;
	}

	return true;
}

void VulkanRenderer::CreateRenderPass(VkRenderPass& targetPass, VkImageLayout finalLayout, VkFormat format, u32 attCount, bool hasDepth, bool hasStencil, bool clearBuffers)
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = clearBuffers ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = clearBuffers ? VK_IMAGE_LAYOUT_UNDEFINED : finalLayout;
	colorAttachment.finalLayout = finalLayout;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = clearBuffers ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = hasStencil ? (clearBuffers ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD) : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = hasStencil ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = clearBuffers ? VK_IMAGE_LAYOUT_UNDEFINED : finalLayout;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentReference> colorAtt;
	colorAtt.resize(attCount);
	{
		for (u32 i = 0; i < attCount; i++)
		{
			colorAtt[i].attachment = i ? i + hasDepth : 0;
			colorAtt[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = attCount;
	subpass.pColorAttachments = colorAtt.data();
	subpass.pDepthStencilAttachment = hasDepth ? &depthAttachmentRef : nullptr;

	std::vector<VkAttachmentDescription> attachments;
	attachments.push_back(colorAttachment);
	if (hasDepth) attachments.push_back(depthAttachment);
	for (u32 i = 1; i < attCount; i++)
	{
		attachments.push_back(colorAttachment);
	}
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &targetPass) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create render pass!");
		throw std::runtime_error("failed to create render pass!");
	}
}

void VulkanRenderer::CreateSCFramebuffers()
{
	const Maths::IVec2 res = Maths::IVec2(swapChainExtent.width, swapChainExtent.height);
	std::vector<VkImageView> attachments;
	for (size_t i = 0; i < swapChainFramebuffers.size(); i++)
	{
		swapChainFramebuffers[i] = RendererFrameBuffer(res, attachments, windowRenderPass, device, swapChainFramebuffers[i].rendererTex.textureImage, swapChainFramebuffers[i].rendererTex.imageView);
		//swapChainFramebuffers[i].rendererTex.CreateGuiView(defaultSampler);
		for (u8 c = 0; c < 4; ++c)
		{
			swapChainFramebuffers[i].clearValue[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		}
	}
}

void VulkanRenderer::CreateFrameBuffer(RendererFrameBuffer& buf, const std::vector<RendererImageView>& attachmentsIn, Maths::IVec2 resolution, Maths::Vec4 clearColor, VkFormat format, Resources::ShaderVariant variant)
{
	std::vector<VkImageView> attachments;
	for (auto& attachment : attachmentsIn)
	{
		attachments.push_back(attachment.imageView);
	}
	switch (variant)
	{
	case Resources::ShaderVariant::Default:
		buf = RendererFrameBuffer(resolution, attachments, geometryRenderPass, device, format);
		break;
	case Resources::ShaderVariant::Shadow:
		// TODO
		break;
	case Resources::ShaderVariant::Cube:
		// TODO
		break;
	case Resources::ShaderVariant::ShadowCube:
		// TODO
		break;
	case Resources::ShaderVariant::Object:
		buf = RendererFrameBuffer(resolution, attachments, objectRenderPass, device, format);
		break;
	case Resources::ShaderVariant::Halo:
		// TODO
		break;
	case Resources::ShaderVariant::Light:
		buf = RendererFrameBuffer(resolution, attachments, lightRenderPass, device, format, true);
		break;
	case Resources::ShaderVariant::PostProcess:
		buf = RendererFrameBuffer(resolution, attachments, postRenderPass, device, format);
		break;
	default:
		break;
	}
	buf.rendererTex.CreateGuiView(Resources::TextureSampler::GetDefaultSampler()->renderSampler);
	buf.clearValue[0].color = { {clearColor.x, clearColor.y, clearColor.z, clearColor.w} };
}

void VulkanRenderer::CreateMainFrameBuffer()
{
	std::vector<VkImageView> attachments =
	{
		mainFB.db.GetDepthImageView().imageView,
		mainFB.nb.GetImageView().imageView,
		mainFB.pb.GetImageView().imageView,
	};
	mainFB.fb = RendererFrameBuffer(targetResolution, attachments, geometryRenderPass, device, VK_FORMAT_R32G32B32A32_SFLOAT);
	mainFB.fb.rendererTex.CreateGuiView(Resources::TextureSampler::GetDefaultSampler()->renderSampler);
	mainFB.UpdateClearColor();
}

void VulkanRenderer::CreateDepthBuffer(Renderer::RendererDepthBuffer& db, Maths::IVec2 resolution)
{
	db = RendererDepthBuffer(resolution.x, resolution.y, device, false);
}

void VulkanRenderer::CreateBuffer(Renderer::RendererBuffer& b, Maths::IVec2 resolution, bool transitLayout)
{
	b = RendererBuffer(resolution.x, resolution.y, device, VK_FORMAT_R32G32B32A32_SFLOAT, transitLayout);
}

void VulkanRenderer::DeleteFrameBuffer(Renderer::RendererFrameBuffer& buf)
{
	buf.DeleteFrameBuffer(device);
}

void VulkanRenderer::DeleteDepthBuffer(Renderer::RendererDepthBuffer& buf)
{
	buf.DeleteDepthBuffer(device);
}

void VulkanRenderer::DeleteBuffer(Renderer::RendererBuffer& buf)
{
	buf.DeleteBuffer(device);
}

void VulkanRenderer::CreateCommandPool(VkCommandPool* cmdPool, u32 index)
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = index;

	if (vkCreateCommandPool(device, &poolInfo, nullptr, cmdPool) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create command pool!");
		throw std::runtime_error("Failed to create command pool!");
	}
}

void VulkanRenderer::CreateCommandBuffers()
{
	commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = graphicCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (u32)MAX_FRAMES_IN_FLIGHT;

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to allocate command buffers!");
		throw std::runtime_error("Failed to allocate command buffers!");
	}
}

void VulkanRenderer::BeginCommandBuffer()
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to begin recording command buffer!");
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
}

void VulkanRenderer::BeginRenderPass(VkRenderPass& targetPass, LowRenderer::FrameBuffer* frameBuffer)
{
	switch (activePassParams)
	{
	case LowRenderer::OBJECT:
		activeRendererFrameBuffer = objectFrameBuffer;
		break;
	case LowRenderer::LIGHT:
		activeRendererFrameBuffer = frameBuffer->lb[0];
		break;
	case LowRenderer::POST:
		activeRendererFrameBuffer = frameBuffer->lb[frameBuffer->actualBuffer == 1 ? 2 : 1];
		break;
	default:
		activeRendererFrameBuffer = frameBuffer->fb;
		break;
	}
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = targetPass;
	renderPassInfo.framebuffer = activeRendererFrameBuffer.buffer;
	renderPassInfo.renderArea.extent.width = frameBuffer->GetResolution().x;
	renderPassInfo.renderArea.extent.height = frameBuffer->GetResolution().y;
	renderPassInfo.renderArea.offset = { 0, 0 };

	renderPassInfo.clearValueCount = (u32)activeRendererFrameBuffer.GetClearValue().size();
	renderPassInfo.pClearValues = activeRendererFrameBuffer.GetClearValue().data();
	vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	activeCommandBuffer = commandBuffers[currentFrame];
	activeFrameBuffer = frameBuffer;
}

void VulkanRenderer::DrawIndexedMesh(const Resources::Mesh* mesh, VkDescriptorSet& meshDescriptor, UniformElement& uniformE)
{
	VkBuffer vertexBuffers[] = { mesh->rendererMesh.vertexBuffer.handle };
	VkDeviceSize offsets[] = { 0 };
	std::array<u32, 2> uniformOffsets = { static_cast<u32>(uniformE.GetOffset() * mainUniform.GetTotalOffset()), static_cast<u32>(uniformE.GetOffset() * mainUniform.GetTotalOffset()) };

	vkCmdBindVertexBuffers(activeCommandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(activeCommandBuffer, mesh->rendererMesh.indexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);
	vkCmdBindDescriptorSets(activeCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, activePipeline->pipelineLayout, 0, 1, &meshDescriptor, 2, uniformOffsets.data());

	vkCmdDrawIndexed(activeCommandBuffer, static_cast<u32>(mesh->GetIndexCount()), 1, 0, 0, 0);
}


void VulkanRenderer::EndRenderPass()
{
	vkCmdEndRenderPass(commandBuffers[currentFrame]);
}

void VulkanRenderer::EndCommandBuffer()
{
	if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to record command buffer!");
		throw std::runtime_error("Failed to record command buffer!");
	}
}

void VulkanRenderer::CreateSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Failed to create seemaphores!");
			throw std::runtime_error("Failed to create seemaphores!");
		}
	}

}

void VulkanRenderer::RecreateSwapChain(Wrappers::Interfacing* pInterface, VkExtent2D newRes, bool useVSync)
{
	CleanupSwapChain();

	CreateSwapChain(newRes, useVSync);

	CreateImageViews();
	CreateSCFramebuffers();

	if (pInterface)
		pInterface->RecreateImGuiSwapChain();
}

VkCommandBuffer VulkanRenderer::BeginSingleTimeCommands(const VkCommandPool& cmdPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = cmdPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void VulkanRenderer::EndSingleTimeCommands(VkCommandBuffer commandBuffer, const VkCommandPool& cmdPool, VkQueue& queue)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	vkFreeCommandBuffers(device, cmdPool, 1, &commandBuffer);
}

void VulkanRenderer::WaitIdle()
{
	vkDeviceWaitIdle(device);
}

void VulkanRenderer::PrepareFrameBuffer(LowRenderer::FrameBuffer* fb)
{
	TransitionImageLayout(fb->GetRendererTexture().textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, fb->mipLevels);
}

void VulkanRenderer::AddDirectionalLight(Core::Scene::Components::Lights::DirectionalLightComponent* dl)
{
	dLights.push_back(dl);
}

void VulkanRenderer::AddPointLight(Core::Scene::Components::Lights::PointLightComponent* dl)
{
	pLights.push_back(dl);
}

void VulkanRenderer::AddSpotLight(Core::Scene::Components::Lights::SpotLightComponent* dl)
{
	sLights.push_back(dl);
}

void VulkanRenderer::ClearLights()
{
	dLights.clear();
	pLights.clear();
	sLights.clear();
}

void VulkanRenderer::SetStencilState(u8 compareValue, StencilState stateIn)
{
	stencilCompareValue = compareValue;
	state = stateIn;
}

u32 VulkanRenderer::FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	LOG(DEBUG_LEVEL::LERROR, "Failed to find suitable memory type!");
	throw std::runtime_error("Failed to find suitable memory type!");
}

VkFormat VulkanRenderer::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates)
	{
		VkFormatProperties props = {};
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{
			return format;
		}
	}
	LOG(DEBUG_LEVEL::LERROR, "Failed to find supported format!");
	throw std::runtime_error("Failed to find supported format!");
}

VkFormat VulkanRenderer::FindDepthFormat()
{
	// We really need that stencil buffer !
	return FindSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

bool VulkanRenderer::HasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D16_UNORM_S8_UINT;
}

void VulkanRenderer::CreateImage(const Maths::IVec2& resolution, u32 mipLevel, VkImage& image, VkDeviceMemory& memory, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCubeMap)
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<u32>(resolution.x);
	imageInfo.extent.height = static_cast<u32>(resolution.y);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevel;
	imageInfo.arrayLayers = isCubeMap ? 6 : 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = isCubeMap ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0; // Optional
	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create image!");
		throw std::runtime_error("Failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to allocate image memory!");
		throw std::runtime_error("Failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, memory, 0);
}

void VulkanRenderer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

void VulkanRenderer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(transferCommandPool);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	EndSingleTimeCommands(commandBuffer, transferCommandPool, transferQueue);
}

void VulkanRenderer::FlushMappedMemory(const VkDeviceMemory& mem, u64 offset, u64 size)
{
	VkMappedMemoryRange memoryRange = {};
	memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	memoryRange.memory = mem;
	memoryRange.size = size;
	memoryRange.offset = offset;
	vkFlushMappedMemoryRanges(device, 1, &memoryRange);
}

void VulkanRenderer::CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height, bool cubeMap)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(transferCommandPool);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = cubeMap ? 6 : 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent =
	{
		width,
		height,
		1
	};
	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	EndSingleTimeCommands(commandBuffer, transferCommandPool, transferQueue);
}

void VulkanRenderer::CopyImageToBuffer(VkBuffer buffer, VkImage image, Maths::IVec2 offset, Maths::IVec2 size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(transferCommandPool);
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset =
	{
		offset.x,
		offset.y,
		0
	};
	region.imageExtent =
	{
		(u32)size.x,
		(u32)size.y,
		1
	};
	vkCmdCopyImageToBuffer(
		commandBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		buffer,
		1,
		&region
	);
	EndSingleTimeCommands(commandBuffer, transferCommandPool, transferQueue);
}

void VulkanRenderer::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevel, bool cubeMap)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(graphicCommandPool);
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (HasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mipLevel ? mipLevel : VK_REMAINING_MIP_LEVELS;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = cubeMap ? 6 : 1;

	VkPipelineStageFlags sourceStage = {};
	VkPipelineStageFlags destinationStage = {};

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		LOG(DEBUG_LEVEL::LERROR, "Unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(commandBuffer, graphicCommandPool, graphicsQueue);
}

VkImageView VulkanRenderer::CreateImageView(const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevel, bool cubeMap)
{
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = cubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevel;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = cubeMap ? 6 : 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create texture image view!");
		throw std::runtime_error("Failed to create texture image view!");
	}

	return imageView;
}

void VulkanRenderer::GenerateMipmaps(VkImage image, VkFormat imageFormat, Maths::IVec2 resolution, u32 mipLevels, bool cubeMap)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		LOG(DEBUG_LEVEL::LERROR, "Texture image format does not support linear blitting!");
		throw std::runtime_error("Texture image format does not support linear blitting!");
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(graphicCommandPool);

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = cubeMap ? 6 : 1;
	barrier.subresourceRange.levelCount = 1;

	Maths::IVec2 mipRes = resolution;

	for (u32 i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit{};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipRes.x, mipRes.y, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = cubeMap ? 6 : 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipRes.x > 1 ? mipRes.x / 2 : 1, mipRes.y > 1 ? mipRes.y / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = cubeMap ? 6 : 1;
		vkCmdBlitImage(commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipRes.x > 1) mipRes.x /= 2;
		if (mipRes.y > 1) mipRes.y /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleTimeCommands(commandBuffer, graphicCommandPool, graphicsQueue);
}

void VulkanRenderer::UpdateUniformBuffer(UniformElement& element, const Resources::Material* mat, const Maths::Mat4& modelMatrix, const Maths::Mat4& mvp)
{
	Uniform::MainVertexUniform& vertexData = *element.GetVertexUniform(*this);
	vertexData.model = modelMatrix;
	vertexData.mvp = mvp;
	vertexData.cameraPos = currentCameraPos;
	Uniform::MainFragmentUniform& fragmentData = *element.GetFragmentUniform(*this);
	fragmentData.matAmbient = mat->ambientColor;
	fragmentData.matShininess = mat->shininess;
	fragmentData.iTime = Core::App::GetInstance()->GetWindow().GetWindowTime();
	// Not exactly sure why I don't need to flush theses buffers...
	//FlushMappedMemory(element.GetMemory(), mainUniform.GetTotalOffset() * element.GetOffset(), mainUniform.GetTotalOffset());
}

void VulkanRenderer::UpdateUniformBuffer(UniformElement& element, const Maths::Mat4& mvp, u32 objectIndex)
{
	Uniform::MainVertexUniform& vertexData = *element.GetVertexUniform(*this);
	vertexData.mvp = mvp;
	vertexData.cameraPos = currentCameraPos;
	Uniform::MainFragmentUniform& fragmentData = *element.GetFragmentUniform(*this);
	fragmentData.matAmbient = Maths::Vec3();
	fragmentData.iTime = Core::App::GetInstance()->GetWindow().GetWindowTime();
	*reinterpret_cast<u32*>(&fragmentData.matAmbient.x) = objectIndex;

	//FlushMappedMemory(element.GetMemory(), mainUniform.GetTotalOffset() * element.GetOffset(), mainUniform.GetTotalOffset());
}

void VulkanRenderer::UpdateLightUniformBuffer(UniformElement& element)
{
	Uniform::LightFragmentUniform& fragmentData = *element.GetLightFragmentUniform(*this);
	fragmentData.viewPos = currentCameraPos;
	fragmentData.dCount = dLights.size() < MAX_DIR_LIGHT ? (u32)dLights.size() : MAX_DIR_LIGHT;
	for (u32 i = 0; i < fragmentData.dCount; i++)
	{
		fragmentData.dlights[i].Direction = dLights[i]->Direction;
		fragmentData.dlights[i].Shininess = dLights[i]->Shininess;
		fragmentData.dlights[i].Ambient = dLights[i]->AmbientColor;
		fragmentData.dlights[i].Diffuse = dLights[i]->DiffuseColor;
		fragmentData.dlights[i].Specular = dLights[i]->SpecularColor;
	}
	//if (fragmentData.dCount) FlushMappedMemory(element.GetMemory(), lightUniform.GetTotalOffset() * element.GetOffset(), sizeof(Uniform::DirectionalLight) * fragmentData.dCount);
	fragmentData.pCount = pLights.size() < MAX_POINT_LIGHT ? (u32)pLights.size() : MAX_POINT_LIGHT;
	for (u32 i = 0; i < fragmentData.pCount; i++)
	{
		f32 radius = (-pLights[i]->Linear + std::sqrtf(pLights[i]->Linear * pLights[i]->Linear - 4 * pLights[i]->Quadratic * (1.0f - (f32)(256.0 / 5.0) * 1.0f))) / (2 * pLights[i]->Quadratic);
		fragmentData.plights[i].Position = pLights[i]->Position;
		fragmentData.plights[i].Shininess = pLights[i]->Shininess;
		fragmentData.plights[i].Ambient = pLights[i]->AmbientColor;
		fragmentData.plights[i].Radius = radius;
		fragmentData.plights[i].Diffuse = pLights[i]->DiffuseColor;
		fragmentData.plights[i].Linear = pLights[i]->Linear;
		fragmentData.plights[i].Specular = pLights[i]->SpecularColor;
		fragmentData.plights[i].Quadratic = pLights[i]->Quadratic;
	}
	//if (fragmentData.pCount) FlushMappedMemory(element.GetMemory(), lightUniform.GetTotalOffset() * element.GetOffset() + sizeof(Uniform::DirectionalLight) * MAX_DIR_LIGHT, sizeof(Uniform::PointLight) * fragmentData.pCount);
	fragmentData.sCount = sLights.size() < MAX_SPOT_LIGHT ? (u32)sLights.size() : MAX_SPOT_LIGHT;
	for (u32 i = 0; i < fragmentData.sCount; i++)
	{
		f32 radius = (-sLights[i]->Linear + std::sqrtf(sLights[i]->Linear * sLights[i]->Linear - 4 * sLights[i]->Quadratic * (1.0f - (f32)(256.0 / 5.0) * 1.0f))) / (2 * sLights[i]->Quadratic);
		fragmentData.slights[i].Position = sLights[i]->Position;
		fragmentData.slights[i].Shininess = sLights[i]->Shininess;
		fragmentData.slights[i].Ambient = sLights[i]->AmbientColor;
		fragmentData.slights[i].Radius = radius;
		fragmentData.slights[i].Diffuse = sLights[i]->DiffuseColor;
		fragmentData.slights[i].Linear = sLights[i]->Linear;
		fragmentData.slights[i].Specular = sLights[i]->SpecularColor;
		fragmentData.slights[i].Quadratic = sLights[i]->Quadratic;
		fragmentData.slights[i].Direction = sLights[i]->Direction;
		fragmentData.slights[i].Angles = Maths::Vec2(sLights[i]->Angle, sLights[i]->Ratio);
	}
	//if (fragmentData.sCount) FlushMappedMemory(element.GetMemory(), lightUniform.GetTotalOffset() * element.GetOffset() + sizeof(Uniform::DirectionalLight) * MAX_DIR_LIGHT + sizeof(Uniform::PointLight) * MAX_POINT_LIGHT, sizeof(Uniform::SpotLight) * fragmentData.sCount); // TODO
}

void VulkanRenderer::UpdatePostUniformBuffer(UniformElement& element, const LowRenderer::PostProcess::PostProcessEffect* effect)
{
	Uniform::PostFragmentUniform& fragmentData = *element.GetPostFragmentUniform(*this);
	effect->FillBuffer(&fragmentData);
}

bool VulkanRenderer::CreateDescriptorPool(VkDescriptorPool& targetPool, u32 size, u32 uniformBuf, u32 images)
{
	std::vector<VkDescriptorPoolSize> poolSizes{};
	if (uniformBuf)
	{
		poolSizes.push_back(VkDescriptorPoolSize());
		poolSizes.back().type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSizes.back().descriptorCount = uniformBuf;
	}
	poolSizes.push_back(VkDescriptorPoolSize());
	poolSizes.back().type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes.back().descriptorCount = images;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<u32>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = size;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &targetPool) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to create descriptor pool!");
		return false;
	}
	return true;
}

bool VulkanRenderer::CreateDescriptorSet(VkDescriptorPool& targetPool, VkDescriptorSet& descriptor, Resources::ShaderVariant targetShader)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = targetPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = targetShader == Resources::ShaderVariant::Light ? &lightUniform.GetLayout() : (targetShader == Resources::ShaderVariant::PostProcess ? &postUniform.GetLayout() : (targetShader == Resources::ShaderVariant::Window ? &windowUniform.GetLayout() : &mainUniform.GetLayout()));
	if (vkAllocateDescriptorSets(device, &allocInfo, &descriptor) != VK_SUCCESS)
	{
		//LOG(DEBUG_LEVEL::LERROR, "Failed to allocate descriptor sets!");
		return false;
	}
	return true;
}

RendererImageView VulkanRenderer::GetValidImage(const RendererTexture* tex)
{
	if (tex->imageView.imageView == activeRendererFrameBuffer.rendererTex.imageView.imageView)
	{
		return Resources::StaticTexture::GetDebugTexture()->GetImageView();
	}
	return tex->imageView;
}

void VulkanRenderer::UpdateDescriptorSet(VkDescriptorSet& descriptor, const RendererTexture* tex)
{
	VkDescriptorImageInfo imageInfo;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = GetValidImage(tex).imageView;
	imageInfo.sampler = Resources::TextureSampler::GetDefaultSampler()->renderSampler.GetSampler();
	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptor;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, static_cast<u32>(1), &descriptorWrite, 0, nullptr);
}

void VulkanRenderer::UpdateDescriptorSet(VkDescriptorSet& descriptor, VkBuffer& uniformBuff, Renderer::Uniform::RendererUniformObject* uniform, bool hasVertexInfo, const std::vector<const RendererTexture*> textures, const Resources::TextureSampler* sampler)
{
	VkDescriptorBufferInfo vertBufferInfo{};
	vertBufferInfo.buffer = uniformBuff;
	vertBufferInfo.offset = 0;
	vertBufferInfo.range = uniform->GetVertexBufSize();

	VkDescriptorBufferInfo fragBufferInfo{};
	fragBufferInfo.buffer = uniformBuff;
	fragBufferInfo.offset = uniform->GetFragmentOffset();
	fragBufferInfo.range = uniform->GetFragmentBufSize();

	std::vector<VkDescriptorImageInfo> imageInfos;
	for (auto& tex : textures)
	{
		imageInfos.push_back(VkDescriptorImageInfo());
		imageInfos.back().imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos.back().imageView = GetValidImage(tex).imageView;
		imageInfos.back().sampler = sampler->renderSampler.GetSampler();
	}

	std::vector<VkWriteDescriptorSet> descriptorWrites{};

	u32 index = 0;
	if (hasVertexInfo)
	{
		descriptorWrites.push_back(VkWriteDescriptorSet());
		descriptorWrites.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.back().dstSet = descriptor;
		descriptorWrites.back().dstBinding = index++;
		descriptorWrites.back().dstArrayElement = 0;
		descriptorWrites.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrites.back().descriptorCount = 1;
		descriptorWrites.back().pBufferInfo = &vertBufferInfo;
	}

	descriptorWrites.push_back(VkWriteDescriptorSet());
	descriptorWrites.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites.back().dstSet = descriptor;
	descriptorWrites.back().dstBinding = index++;
	descriptorWrites.back().dstArrayElement = 0;
	descriptorWrites.back().descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorWrites.back().descriptorCount = 1;
	descriptorWrites.back().pBufferInfo = &fragBufferInfo;
	if (!hasVertexInfo) index = 2;
	for (auto& tex : imageInfos)
	{
		descriptorWrites.push_back(VkWriteDescriptorSet());
		descriptorWrites.back().sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.back().dstSet = descriptor;
		descriptorWrites.back().dstBinding = index++;
		descriptorWrites.back().dstArrayElement = 0;
		descriptorWrites.back().descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites.back().descriptorCount = 1;
		descriptorWrites.back().pImageInfo = &tex;
	}

	vkUpdateDescriptorSets(device, static_cast<u32>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
}

Renderer::FrameDescriptorPool::FrameDescriptorPool()
{
}

Renderer::FrameDescriptorPool::~FrameDescriptorPool()
{
}

void FrameDescriptorPool::CreatePools(VulkanRenderer& renderer, u32 count, u32 uniformBuf, u32 images)
{
	for (u32 i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
	{
		renderer.CreateDescriptorPool(framePool[i], count, uniformBuf, images);
		poolState[i] = false;
	}
}

void FrameDescriptorPool::DestroyPools(VkDevice& device)
{
	for (u32 i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
	{
		vkDestroyDescriptorPool(device, framePool[i], nullptr);
	}
}

VkDescriptorSet FrameDescriptorPool::GetNext(VulkanRenderer& renderer, Resources::ShaderVariant variant)
{
	VkDescriptorSet result = {};
	while (!renderer.CreateDescriptorSet(framePool[currentPos], result, variant))
	{
		poolState[currentPos] = true;
		currentPos = (currentPos + 1) % DESCRIPTOR_POOL_SIZE;
		if (poolState[currentPos])
		{
			Wrappers::WindowManager::OpenPopup("Fatal Error", "Out of descriptor sets!", Wrappers::PopupParam::BUTTON_RETRY_CANCEL | Wrappers::PopupParam::ICON_WARNING);
			abort();
		}
	}
	return result;
}

void FrameDescriptorPool::UpdatePool(VkDevice& device, VulkanRenderer& renderer)
{
	for (u32 i = 0; i < DESCRIPTOR_POOL_SIZE; i++)
	{
		if (poolState[i])
		{
			vkResetDescriptorPool(device, framePool[i], 0);
			poolState[i] = false;
		}
	}
}

void VulkanRenderer::SubmitCurrentCommandBuffer(void* buffer)
{
	std::vector<VkCommandBuffer> submitCommandBuffers =
	{
		commandBuffers[currentFrame],
	};
	if (buffer) submitCommandBuffers.push_back(static_cast<VkCommandBuffer>(buffer));

	VkSubmitInfo submitInfo{};

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = static_cast<u32>(submitCommandBuffers.size());
	submitInfo.pCommandBuffers = submitCommandBuffers.data();

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };

	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		LOG(DEBUG_LEVEL::LERROR, "Failed to submit draw command buffer!");
		throw std::runtime_error("Failed to submit draw command buffer!");
	}
}

UniformBufferPool::UniformBufferPool()
{
}

UniformBufferPool::~UniformBufferPool()
{
}

void UniformBufferPool::CreatePools(VulkanRenderer& renderer, u32 countIn, Renderer::Uniform::RendererUniformObject* uniform)
{
	count = countIn;
	bufferSize = count * uniform->GetTotalOffset();
	for (u32 i = 0; i < DESCRIPTOR_POOL_SIZE; ++i)
	{
		renderer.CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, framePool[i].buffer, framePool[i].memory);
		if (vkMapMemory(renderer.device, framePool[i].memory, 0, VK_WHOLE_SIZE, 0, &framePool[i].mappedMemory) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Could not map uniform buffer !");
		}
	}
}

void UniformBufferPool::DestroyPools(VkDevice& device)
{
	for (u32 i = 0; i < DESCRIPTOR_POOL_SIZE; ++i)
	{
		vkDestroyBuffer(device, framePool[i].buffer, nullptr);
		vkFreeMemory(device, framePool[i].memory, nullptr);
	}
}

UniformElement UniformBufferPool::GetNext()
{
	UniformElement result = {};
	u32 poolIndex = static_cast<u32>(currentPos / count);
	assert(poolIndex < DESCRIPTOR_POOL_SIZE);
	result.buffer = framePool[poolIndex].buffer;
	result.memory = framePool[poolIndex].memory;
	result.index = currentPos - poolIndex * 1llu * count;
	result.mappedPointer = framePool[poolIndex].mappedMemory;
	currentPos++;
	return result;
}

void UniformBufferPool::UpdatePool()
{
	currentPos = 0;
}

Renderer::UniformElement::UniformElement(void* ptr, u64 id, VkBuffer buf, VkDeviceMemory mem) : mappedPointer(ptr), index(id), buffer(buf), memory(mem)
{
}

Uniform::MainVertexUniform* UniformElement::GetVertexUniform(VulkanRenderer& renderer)
{
	return reinterpret_cast<Uniform::MainVertexUniform*>(static_cast<u8*>(mappedPointer) + renderer.mainUniform.GetTotalOffset() * index);
}

Uniform::MainFragmentUniform* UniformElement::GetFragmentUniform(VulkanRenderer& renderer)
{
	return reinterpret_cast<Uniform::MainFragmentUniform*>(static_cast<u8*>(mappedPointer) + renderer.mainUniform.GetTotalOffset() * index + renderer.mainUniform.GetFragmentOffset());
}

Uniform::LightFragmentUniform* UniformElement::GetLightFragmentUniform(VulkanRenderer& renderer)
{
	return reinterpret_cast<Uniform::LightFragmentUniform*>(static_cast<u8*>(mappedPointer) + renderer.lightUniform.GetTotalOffset() * index + renderer.lightUniform.GetFragmentOffset());
}

Uniform::PostFragmentUniform* UniformElement::GetPostFragmentUniform(VulkanRenderer& renderer)
{
	return reinterpret_cast<Uniform::PostFragmentUniform*>(static_cast<u8*>(mappedPointer) + renderer.postUniform.GetTotalOffset() * index + renderer.postUniform.GetFragmentOffset());;
}

u64 UniformElement::GetOffset()
{
	return index;
}

VkBuffer& UniformElement::GetBuffer()
{
	return buffer;
}

VkDeviceMemory& UniformElement::GetMemory()
{
	return memory;
}
