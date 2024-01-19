#include <ImGUI/imgui.h>
#include <ImGUI/imgui_impl_glfw.h>
#include <ImGUI/imgui_impl_vulkan.h>
#include <ImGUI/imgui_stdlib.hpp>
#include <ImGUI/imgui_internal.h>

#include "Maths/Maths.hpp"

#ifdef _WIN32
#pragma warning(disable : 26812)
#endif

#include <vulkan/vk_enum_string_helper.h>

#include "Wrappers/ModelLoader/AssimpModelLoader.hpp"
#include "Wrappers/ImageLoader.hpp"
#include "Wrappers/ShaderLoader.hpp"
#include "Wrappers/Interfacing.hpp"
#include "Wrappers/Sound/SoundLoader.hpp"
#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

#include "Resources/CubeMap.hpp"
#include "Resources/Sound.hpp"
#include "Resources/SoundData.hpp"

#ifndef _WIN32
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <Windows.h>
#endif

using namespace Core::Scene::Components;

constexpr std::array<const char*, (int)Resources::ObjectType::Size> dragDropResourcesPayload = {
	IMGUI_DRAG_DROP_MODEL_PAYLOAD,
	IMGUI_DRAG_DROP_SKINNED_MODEL_PAYLOAD,
	IMGUI_DRAG_DROP_TEXTURE_SAMPLER_PAYLOAD,
	IMGUI_DRAG_DROP_TEXTURE_PAYLOAD,
	IMGUI_DRAG_DROP_MATERIAL_PAYLOAD,
	IMGUI_DRAG_DROP_MESH_PAYLOAD,
	IMGUI_DRAG_DROP_VERTEX_SHADER_PAYLOAD,
	IMGUI_DRAG_DROP_FRAGMENT_SHADER_PAYLOAD,
	IMGUI_DRAG_DROP_GEOMETRY_SHADER_PAYLOAD,
	IMGUI_DRAG_DROP_SHADER_PROGRAM_PAYLOAD,
	IMGUI_DRAG_DROP_FRAME_BUFFER_PAYLOAD,
	IMGUI_DRAG_DROP_DEPTH_BUFFER_PAYLOAD,
	IMGUI_DRAG_DROP_CUBE_MAP_PAYLOAD,
	IMGUI_DRAG_DROP_SOUND_PAYLOAD,
	IMGUI_DRAG_DROP_SOUND_DATA_PAYLOAD,
	IMGUI_DRAG_DROP_SCENE_PAYLOAD,
};
std::vector<const char*> nameResourceList{ "Model", "Texture", "CubeMap", "Material", "Mesh", "VertexShader", "FragmentShader", "GeometryShader", "ShaderProgram", "TextureSampler", "Sound", "SoundData", "Scenes"};
std::vector<const char*> nameCreateResourceList{ "Material", "Texture Sampler", "Shader Program", "Sound", "Scene"};
std::vector<Resources::ObjectType> typeList{
	Resources::ObjectType::ModelType,
	Resources::ObjectType::TextureType,
	Resources::ObjectType::CubeMapType,
	Resources::ObjectType::MaterialType,
	Resources::ObjectType::MeshType,
	Resources::ObjectType::VertexShaderType,
	Resources::ObjectType::FragmentShaderType,
	Resources::ObjectType::GeometryShaderType,
	Resources::ObjectType::ShaderProgramType,
	Resources::ObjectType::TextureSamplerType,
	Resources::ObjectType::SoundType,
	Resources::ObjectType::SoundDataType,
	Resources::ObjectType::SceneType,

};

namespace Wrappers
{
#pragma region Init

	std::string Interfacing::GetExtentionFile(std::string path)
	{
		std::string extension;
		for (u16 i = 0; i < path.size(); i++)
		{
			if (path[i] == '.')
				extension = path.substr(i + 1llu);
		}
		return extension;
	}

	void CheckVKResult(VkResult result)
	{
		if (result)
		{
			LOG(DEBUG_LEVEL::LERROR, string_VkResult(result));
		}
	}

	void Interfacing::InitImgui(GLFWwindow* window)
	{
		ReadFolder();
		logBitMask.set();
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		isInitialised = true;

		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 2.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
			style.WindowPadding = ImVec2(15, 15);
			style.FramePadding = ImVec2(2, 2);
			style.FrameRounding = 5.0f;
			style.ItemSpacing = ImVec2(12, 8);
			style.ItemInnerSpacing = ImVec2(8, 6);
			style.IndentSpacing = 25.0f;
			style.ScrollbarSize = 15.0f;
			style.ScrollbarRounding = 9.0f;
			style.GrabMinSize = 5.0f;
			style.GrabRounding = 3.0f;
			style.FrameRounding = 0;
		}

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (u32)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		appInstance = Core::App::GetInstance();
		vulkanRenderer = &appInstance->GetRenderer();

		if (vkCreateDescriptorPool(vulkanRenderer->device, &pool_info, nullptr, &imguiPool) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Could not create descriptor pool for ImGui!");
			throw std::runtime_error("Could not create descriptor pool for ImGui!");
		}

		CreateImGuiRenderPass();
		vulkanRenderer->CreateCommandPool(&imguiCommandPool, vulkanRenderer->queueFamilyIndices.graphicsFamily.value());
		CreateImGuiCommandBuffers();
		CreateImGuiFramebuffers();

		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = vulkanRenderer->instance;
		init_info.PhysicalDevice = vulkanRenderer->GetPhysicalDevice();
		init_info.Device = vulkanRenderer->device;
		init_info.QueueFamily = vulkanRenderer->queueFamilyIndices.graphicsFamily.value();
		init_info.Queue = vulkanRenderer->graphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = imguiPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = vulkanRenderer->imageCount;
		init_info.ImageCount = vulkanRenderer->imageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = CheckVKResult;
		ImGui_ImplVulkan_Init(&init_info, imguiRenderPass);

		VkCommandBuffer commandBuffer = vulkanRenderer->BeginSingleTimeCommands(imguiCommandPool);
		ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);
		vulkanRenderer->EndSingleTimeCommands(commandBuffer, imguiCommandPool, init_info.Queue);
		ImGui_ImplVulkan_DestroyFontUploadObjects();

		auto& resourceManager = appInstance->GetResources();
		iconFolder = resourceManager.Get<Resources::StaticTexture>(0x60);
		iconTxt = resourceManager.Get<Resources::StaticTexture>(0x61);
		iconPng = resourceManager.Get<Resources::StaticTexture>(0x62);
		iconUnknown = resourceManager.Get<Resources::StaticTexture>(0x63);
		iconObj_Fbx = resourceManager.Get<Resources::StaticTexture>(0x64);
		iconJpg = resourceManager.Get<Resources::StaticTexture>(0x65);
		iconCpp = resourceManager.Get<Resources::StaticTexture>(0x66);
		iconH = resourceManager.Get<Resources::StaticTexture>(0x67);
		saveTexture = resourceManager.Get<Resources::StaticTexture>(0x68);
	}

	void Interfacing::CreateImGuiRenderPass()
	{
		VkAttachmentDescription attachment = {};
		attachment.format = vulkanRenderer->swapChainImageFormat;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0; // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;

		if (vkCreateRenderPass(vulkanRenderer->device, &info, nullptr, &imguiRenderPass) != VK_SUCCESS)
			throw std::runtime_error("failed to create render pass!");
	}

	void Interfacing::Destroy()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		for (size_t i = 0; i < imguiFramebuffers.size(); i++)
		{
			imguiFramebuffers[i].DeleteFrameBuffer(vulkanRenderer->device);
		}
		vkDestroyCommandPool(vulkanRenderer->device, imguiCommandPool, nullptr);
		vkDestroyRenderPass(vulkanRenderer->device, imguiRenderPass, nullptr);
		vkDestroyDescriptorPool(vulkanRenderer->device, imguiPool, nullptr);
	}

	void Interfacing::CreateImGuiCommandBuffers()
	{
		imguiCommandBuffers.resize(vulkanRenderer->swapChainFramebuffers.size());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = imguiCommandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)imguiCommandBuffers.size();

		if (vkAllocateCommandBuffers(vulkanRenderer->device, &allocInfo, imguiCommandBuffers.data()) != VK_SUCCESS)
		{
			LOG(DEBUG_LEVEL::LERROR, "Failed to allocate ImGui command buffers!");
			throw std::runtime_error("Failed to allocate ImGui command buffers!");
		}
	}

	void Interfacing::CreateImGuiFramebuffers()
	{
		imguiFramebuffers.resize(vulkanRenderer->swapChainFramebuffers.size());

		VkImageView attachment[1];
		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = imguiRenderPass;
		info.attachmentCount = 1;
		info.pAttachments = attachment;
		info.width = vulkanRenderer->swapChainExtent.width;
		info.height = vulkanRenderer->swapChainExtent.height;
		info.layers = 1;
		for (uint32_t i = 0; i < vulkanRenderer->swapChainFramebuffers.size(); i++)
		{
			attachment[0] = vulkanRenderer->swapChainFramebuffers.at(i).rendererTex.imageView.imageView;
			if (vkCreateFramebuffer(vulkanRenderer->device, &info, nullptr, &imguiFramebuffers[i].buffer) != VK_SUCCESS)
			{
				LOG(DEBUG_LEVEL::LERROR, "Failed to create ImGui framebuffer!");
				throw std::runtime_error("Failed to create ImGui framebuffer!");
			}
		}
	}

	void* Interfacing::RecordImGuiCommandBuffer(u32 imageIndex)
	{
		u32 currentFrame = vulkanRenderer->currentFrame;
		vkResetCommandBuffer(imguiCommandBuffers[currentFrame], 0);
		VkCommandBufferBeginInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(imguiCommandBuffers[currentFrame], &info);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = imguiRenderPass;
		renderPassInfo.framebuffer = imguiFramebuffers[imageIndex].buffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vulkanRenderer->swapChainExtent;
		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass(imguiCommandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Record dear imgui primitives into command buffer
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), imguiCommandBuffers[currentFrame]);

		vkCmdEndRenderPass(imguiCommandBuffers[currentFrame]);
		vkEndCommandBuffer(imguiCommandBuffers[currentFrame]);

		return imguiCommandBuffers[currentFrame];
	}

	void Interfacing::RecreateImGuiSwapChain()
	{
		for (size_t i = 0; i < imguiFramebuffers.size(); i++)
		{
			imguiFramebuffers[i].DeleteFrameBuffer(vulkanRenderer->device);
		}
		vkDestroyRenderPass(vulkanRenderer->device, imguiRenderPass, nullptr);
		ImGui_ImplVulkan_SetMinImageCount(vulkanRenderer->imageCount);
		CreateImGuiRenderPass();
		CreateImGuiFramebuffers();
		CreateImGuiCommandBuffers();
	}

	void Interfacing::BeginFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
	}

	void Interfacing::EndFrame()
	{
		ImGui::EndFrame();
	}

	void Interfacing::Render()
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui::Render();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void Interfacing::ShowFps(std::array<f32, 128> frames, f32 average, u64 frameCounter)
	{
		if (openFps)
		{
			if (ImGui::Begin("FrameGraph", &openFps))
			{
				ImGui::PlotHistogram("Frames", frames.data(), static_cast<s32>(frames.size()), 0, "", 0.0f, 0.1f, ImVec2(ImGui::GetWindowWidth() - 10, 100));
				ImGui::Text("FPS: %.2f", average);
				ImGui::Text("Frames: %lu", frameCounter);
			}
			ImGui::End();
		}
	}
#pragma endregion

	bool Interfacing::IsInitialised()
	{
		return isInitialised;
	}

#pragma region ImGuiFunction
	bool Interfacing::BeginClose(const char* name, bool& closing, int flags)
	{
		return ImGui::Begin(name, &closing, flags);
	}

	bool Interfacing::Begin(const char* name)
	{
		return ImGui::Begin(name);
	}

	void Interfacing::End()
	{
		ImGui::End();
	}

	void Interfacing::PushBGColor(Maths::Vec3 color)
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(color.x, color.y, color.z, 1.0f));
	}

	void Interfacing::PopBGColor()
	{
		ImGui::PopStyleColor();
	}

	void Interfacing::Bullet()
	{
		ImGui::Bullet();
	}

	void Interfacing::BulletText(const char* text)
	{
		ImGui::BulletText(text);
	}

	bool Interfacing::CheckBox(const char* name, bool var)
	{
		return ImGui::Checkbox(name, &var);
	}

	bool Wrappers::Interfacing::CheckBox(const char* name, bool* var)
	{
		return ImGui::Checkbox(name, var);
	}

	bool Interfacing::ColorEdit3(const char* name, Maths::Vec3& color)
	{
		return ImGui::ColorEdit3(name, &color.x, ImGuiColorEditFlags_PickerHueWheel);
	}

	bool Interfacing::ColorEdit4(const char* name, Maths::Vec4& color)
	{
		return ImGui::ColorEdit4(name, &color.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel);
	}

	bool Interfacing::ColorPicker4(const char* name, Maths::Vec4& color)
	{
		return ImGui::ColorPicker4(name, &color.x, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_PickerHueWheel);
	}

	void Interfacing::Column(u16 count, const char* name, bool border)
	{
		ImGui::Columns(count, name, border);
	}

	void Interfacing::Dummy(Maths::Vec2 size)
	{
		ImGui::Dummy(size);
	}

	void Interfacing::NextColumn()
	{
		ImGui::NextColumn();
	}

	bool Interfacing::CollapsingHeader(const char* name)
	{
		return ImGui::CollapsingHeader(name);
	}

	bool Interfacing::CollapsingHeader(const char* name, bool* closed, int flags)
	{
		return ImGui::CollapsingHeader(name, closed, flags);
	}

	///Deprecated : Use Begin/End combo instead
	bool Interfacing::Combo(const char* label, int* current_item, const char* const items[], int items_count)
	{
		return ImGui::Combo(label, current_item, items, items_count);
	}

	u64 previousShaderSize = 0;
	std::vector<Resources::ShaderProgram*> cachedShaders;
	std::vector<Resources::ShaderVariant> cachedFilters;

	bool Interfacing::ShaderCombo(const std::vector<Resources::ShaderVariant>& pFilters, Resources::ShaderProgram** pOutSelectedShader)
	{
		internalIDStack++;

		PushId(internalIDStack);

		if (appInstance->GetResources().GetShaderProgramList().size() != previousShaderSize || pFilters != cachedFilters)
		{
			cachedShaders.clear();
			for (Resources::ShaderProgram* comboShader : appInstance->GetResources().GetShaderProgramList())
			{
				for (auto& filter : pFilters)
				{
					if (comboShader->GetShaderType() == filter)
					{
						cachedShaders.push_back(comboShader);

						break;
					}
				}
			}
			cachedFilters = pFilters;
			previousShaderSize = appInstance->GetResources().GetShaderProgramList().size();
		}

		const char* defaultText = *pOutSelectedShader != nullptr ? (*pOutSelectedShader)->path.c_str() : "No Shader";
		if (BeginCombo("###ShaderList", defaultText, 0))
		{
			if (cachedShaders.empty())Selectable("No shader corresponding.");

			for (Resources::ShaderProgram* comboShader : cachedShaders)
			{
				if (Selectable(comboShader->path.c_str()))
				{
					*pOutSelectedShader = comboShader;
					EndCombo();
					PopId();
					return true;
				}
			}

			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_SHADER_PROGRAM_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::ShaderProgram* dragDropShader = *(Resources::ShaderProgram**)payload->Data;

				for (auto& filter : pFilters)
				{
					if (dragDropShader->GetShaderType() == filter)
					{
						*pOutSelectedShader = dragDropShader;
						ImGui::EndDragDropTarget();
						PopId();
						return true;
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::ShaderCombo(Resources::ShaderVariant filter, Resources::ShaderProgram** pOutSelectedShader)
	{
		std::vector<Resources::ShaderVariant> filters = { filter };
		return ShaderCombo(filters, pOutSelectedShader);
	}

	bool Interfacing::BeginCombo(const char* pLabel, const char* pPreviewValue, const ImGuiComboFlags& pFlags)
	{
		return ImGui::BeginCombo(pLabel, pPreviewValue, pFlags);
	}

	void Interfacing::EndCombo()
	{
		return ImGui::EndCombo();
	}

	void Interfacing::Image(Resources::Texture* texture, Maths::Vec2 size)
	{
		ImGui::Image(texture->GetImageView().imGuiDS, size);
	}

	void Wrappers::Interfacing::Image(Resources::CubeMap* cubeMap, Maths::Vec2 size)
	{
		ImGui::Image(cubeMap->GetImageView().imGuiDS, size);
	}

	void Interfacing::Image(Renderer::RendererTexture texture, Maths::Vec2 size)
	{
		ImGui::Image(texture.imageView.imGuiDS, size);
	}

	void Interfacing::SameLine(float start, float spacing)
	{
		ImGui::SameLine(start, spacing);
	}

	void Interfacing::Text(const char* text)
	{
		ImGui::TextUnformatted(text);
	}

	VkDescriptorSet Interfacing::CreateImageRef(const VkImageView& imageView, const VkSampler& sampler)
	{
		if (isInitialised)
			return ImGui_ImplVulkan_AddTexture(sampler, imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		else
			return VK_NULL_HANDLE;
	}

	void Interfacing::DeleteImageRef(VkDescriptorSet& set)
	{
		if (!isInitialised)
			return;
		
		ImGui_ImplVulkan_RemoveTexture(set);
		set = {};
	}

	Maths::Vec2 Interfacing::GetWindowSize()
	{
		return Maths::Vec2(ImGui::GetWindowContentRegionMax()) - ImGui::GetWindowContentRegionMin();
	}

	Maths::Vec2 Interfacing::GetWindowPos()
	{
		return Maths::Vec2(ImGui::GetWindowPos());
	}

	Maths::Vec2 Interfacing::GetWindowPadding()
	{
		return Maths::Vec2(ImGui::GetWindowContentRegionMin());
	}

	Maths::Vec2 Interfacing::GetMouseDragDeta()
	{
		return ImGui::GetMouseDragDelta();
	}

	void Interfacing::ResetMouseDragDelta()
	{
		ImGui::ResetMouseDragDelta();
	}

	void Interfacing::SetWindowFocused()
	{
		ImGui::SetWindowFocus();
	}

	bool Interfacing::IsClicked()
	{
		return ImGui::IsItemClicked();
	}

	bool Interfacing::IsHovered()
	{
		return ImGui::IsItemHovered();
	}

	bool Interfacing::IsActive()
	{
		return ImGui::IsItemActive();
	}

	void Interfacing::SetCursor(Maths::Vec2 localPos)
	{
		ImGui::SetCursorPos(localPos);
	}

	void Interfacing::SetCursorX(float x)
	{
		ImGui::SetCursorPosX(x);
	}

	void Interfacing::SetCursorY(float y)
	{
		ImGui::SetCursorPosY(y);
	}

	Maths::Vec2 Interfacing::GetCursor()
	{
		return ImGui::GetCursorPos();
	}

	float Interfacing::GetCursorX()
	{
		return ImGui::GetCursorPosX();
	}

	float Interfacing::GetCursorY()
	{
		return ImGui::GetCursorPosY();
	}

	bool Interfacing::InputText(const char* name, std::string& text)
	{
		return ImGui::InputText(name, &text);
	}

	//bool Interfacing::IsItemClicked()
	//{
	//	ImGui::IsItemClicked(ImGuiMouseButton_Left);
	//}

	
#pragma endregion

#pragma region Button
	bool Interfacing::ArrowButton(const char* name, int Dir)//None = -1, Left = 0, Right = 1, Up = 2, Down = 3
	{
		return ImGui::ArrowButton(name, Dir);
	}

	bool Interfacing::Button(const char* name, Maths::Vec2 size)
	{
		return ImGui::Button(name, size);
	}


	bool Interfacing::ColorButton(const char* name, Maths::Vec4 color, Maths::Vec2 size)
	{
		return ImGui::ColorButton(name, color, ImGuiColorEditFlags_NoLabel, size);
	}

	bool Interfacing::ImageButton(const char* name, VkDescriptorSet user_texture_id, Maths::Vec2 size)
	{
		return ImGui::ImageButton(name, user_texture_id, size);
	}

	bool Interfacing::InvisibleButton(const char* name, Maths::Vec2 size)
	{
		return ImGui::InvisibleButton(name, size);
	}

	void Interfacing::LogButtons()
	{
		ImGui::LogButtons();
	}

	bool Interfacing::Selectable(const char* name)
	{
		return ImGui::Selectable(name);
	}

	bool Interfacing::Selectable(const char* name, bool* selectable)
	{
		return ImGui::Selectable(name, selectable);
	}
#pragma endregion

#pragma region TabItem

	bool Interfacing::BeginTabBar(const char* name)
	{
		return ImGui::BeginTabBar(name);
	}

	bool Interfacing::BeginTabItem(const char* label, bool* p_open, bool withflag)
	{
		if (withflag)
		{
			return ImGui::BeginTabItem(label, p_open, ImGuiTabBarFlags_FittingPolicyMask_);
		}
		else
		{
			if (p_open != NULL)
				return ImGui::BeginTabItem(label, p_open);
			else
				return ImGui::BeginTabItem(label);
		}
	}

	void Interfacing::SetTabItemClosed(const char* name)
	{
		ImGui::SetTabItemClosed(name);
	}

	void Interfacing::EndTabBar()
	{
		ImGui::EndTabBar();
	}

	void Interfacing::EndTabItem()
	{
		ImGui::EndTabItem();
	}

	bool Interfacing::TabItemButton(const char* name)
	{
		return ImGui::TabItemButton(name);
	}

#pragma endregion

#pragma region DragFloat_SliderFloat
	bool Interfacing::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max)
	{
		return ImGui::DragFloat(label, v, v_speed, v_min, v_max);
	}

	bool Interfacing::DragFloat2(const char* label, float* v, float v_speed, float v_min, float v_max)
	{
		return ImGui::DragFloat2(label, v, v_speed, v_min, v_max);
	}

	bool Interfacing::DragFloat3(const char* label, float* v, float v_speed, float v_min, float v_max)
	{
		return ImGui::DragFloat3(label, v, v_speed, v_min, v_max);
	}

	bool Interfacing::DragFloat4(const char* label, float* v, float v_speed, float v_min, float v_max)
	{
		return ImGui::DragFloat4(label, v, v_speed, v_min, v_max);
	}

	bool Interfacing::DragFloatRange2(const char* label, float* v, float* v2, float v_speed, float v_min, float v_max)
	{
		return ImGui::DragFloatRange2("label", v, v2, v_speed, v_min, v_max);
	}
	bool Interfacing::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max)
	{
		return ImGui::DragInt(label, v, v_speed, v_min, v_max);
	}
	bool Interfacing::DragInt2(const char* label, int* v, float v_speed, int v_min, int v_max)
	{
		return ImGui::DragInt2(label, v, v_speed, v_min, v_max);
	}

	bool Interfacing::SliderInt(const char* label, int* v, int v_min, int v_max)
	{
		return ImGui::SliderInt(label, v, v_min, v_max);
	}

	bool Interfacing::SliderAngle(const char* label, float* v_rad)
	{
		return ImGui::SliderAngle(label, v_rad);
	}
	bool Interfacing::SliderFloat(const char* label, float* v, float v_min, float v_max, bool logaritmique)
	{
		return ImGui::SliderFloat(label, v, v_min, v_max, "%.3f", ImGuiSliderFlags_Logarithmic * logaritmique);
	}

	bool Interfacing::SliderFloat2(const char* label, float* v, float v_min, float v_max)
	{
		return ImGui::SliderFloat2(label, v, v_min, v_max);
	}

	bool Interfacing::SliderFloat3(const char* label, float* v, float v_min, float v_max)
	{
		return ImGui::SliderFloat3(label, v, v_min, v_max);
	}

	bool Interfacing::SliderFloat4(const char* label, float* v, float v_min, float v_max)
	{
		return ImGui::SliderFloat4(label, v, v_min, v_max);
	}

	bool Interfacing::DragQuat(const char* label, Maths::Quat& quat, float v_speed)
	{
		Maths::Vec3 rot = quat.GetRotationMatrix3().GetRotationFromTranslation();
		if (Maths::Util::IsNear(Maths::Util::Mod(rot.z, 360.0f), 180.0f, 0.01f))
		{
			rot.x = 180.0f - rot.x;
			rot.y = 180.0f + rot.y;
			rot.z = 0.0f;
		}
		if (DragFloat3(label, &rot.x, 0.1f))
		{
			quat = Maths::Quat::FromEuler(rot * (f32)(M_PI / 180.0f));
			return true;
		}
		return false;
	}

	void Interfacing::PlotHistogram(const char* name, const f32* data, s32 dataCount, const char* overlayText, f32 minScale, f32 maxScale, Maths::Vec2 size)
	{
		ImGui::PlotHistogram(name, data, dataCount, 0, overlayText, minScale, maxScale, size);
	}

#pragma endregion

#pragma region Menu
	bool Interfacing::BeginMainMenuBar()
	{
		return ImGui::BeginMainMenuBar();
	}

	bool Interfacing::BeginMenuBar()
	{
		return ImGui::BeginMenuBar();
	}

	bool Interfacing::BeginMenu(const char* name)
	{
		return ImGui::BeginMenu(name);
	}

	bool Interfacing::MenuItem(const char* name, const char* shortcut, bool selected, bool enabled)
	{
		return ImGui::MenuItem(name, shortcut, selected, enabled);
	}


	void Interfacing::EndMenu()
	{
		ImGui::EndMenu();
	}

	void Interfacing::PushId(int Id)
	{
		ImGui::PushID(Id);
	}

	void Interfacing::PopId()
	{
		ImGui::PopID();
	}

	void Interfacing::EndMainMenuBar()
	{
		ImGui::EndMainMenuBar();
	}

	void Interfacing::EndMenuBar()
	{
		ImGui::EndMenuBar();
	}
#pragma endregion


#pragma region Popup
	void Interfacing::OpenPopup(const char* name)
	{
		ImGui::OpenPopup(name);
	}

	bool Interfacing::BeginPopupModal(const char* name)
	{
		return ImGui::BeginPopupModal(name);
	}

	bool Interfacing::BeginPopupContextItem()
	{
		return ImGui::BeginPopupContextItem();
	}

	bool Interfacing::IsItemHovered()
	{
		return ImGui::IsItemHovered();
	}

	void Interfacing::SetTooltip(const char* text)
	{
		ImGui::SetTooltip(text);
	}

	void Interfacing::EndPopup()
	{
		ImGui::EndPopup();
	}

	void Interfacing::CloseCurrentPopup()
	{
		ImGui::CloseCurrentPopup();
	}

	bool Interfacing::IsEscPressed()
	{
		return ImGui::IsKeyPressed(ImGuiKey_Escape);
	}

#pragma endregion

#pragma region Combo
	bool Interfacing::ModelListCombo(Resources::Model** pOutSelectedModel, const char* id)
	{
		Text("Model  : ");
		SameLine();

		const char* defaultText = !(*pOutSelectedModel) ? "missingno" : (*pOutSelectedModel)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo(id, defaultText, 0))
		{
			for (Resources::Model* model : appInstance->GetResources().GetModelList())
			{
				if (Selectable(model->path.c_str()))
				{
					if (!pOutSelectedModel) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedModel = model;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_MODEL_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::Model* model = *(Resources::Model**)payload->Data;
				*pOutSelectedModel = model;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::MaterialListCombo(Resources::Material** pOutSelectedMaterial, const char* id)
	{
		const char* defaultText = !(*pOutSelectedMaterial) ? "missingno" : (*pOutSelectedMaterial)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo(id, defaultText, 0))
		{
			for (Resources::Material* material : appInstance->GetResources().GetMaterialList())
			{
				if (Selectable(material->path.c_str()))
				{
					if (!pOutSelectedMaterial) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedMaterial = material;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_MATERIAL_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::Material* dragDropMat = *(Resources::Material**)payload->Data;
				*pOutSelectedMaterial = dragDropMat;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::MeshListCombo(Resources::Mesh** pOutSelectedMesh, const char* id)
	{
		const char* defaultText = !(*pOutSelectedMesh) ? "missingno" : (*pOutSelectedMesh)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo(id, defaultText, 0))
		{
			for (Resources::Mesh* mesh : appInstance->GetResources().GetMeshList())
			{
				if (Selectable(mesh->path.c_str()))
				{
					if (!pOutSelectedMesh) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedMesh = mesh;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_MESH_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::Mesh* dragDropMesh = *(Resources::Mesh**)payload->Data;
				*pOutSelectedMesh = dragDropMesh;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::SceneListCombo(Core::Scene::Scene** pOutSelectedScene, const char* id)
	{

		const char* defaultText = !(*pOutSelectedScene) ? "missingno" : (*pOutSelectedScene)->path.c_str();
		if (BeginCombo(id, defaultText, 0))
		{
			for (Core::Scene::Scene* scene : appInstance->GetResources().GetSceneList())
			{
				if (Selectable(scene->path.c_str()))
				{
					if (!pOutSelectedScene) //No output
					{
						return false;
					}

					*pOutSelectedScene = scene;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_SCENE_PAYLOAD);

			if (payload != nullptr)
			{
				Core::Scene::Scene* dragDropScene = *(Core::Scene::Scene**)payload->Data;
				*pOutSelectedScene = dragDropScene;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::SoundListCombo(Resources::Sound** pOutSelectedSound, const char* id)
	{
		const char* defaultText = !(*pOutSelectedSound) ? "missingno" : (*pOutSelectedSound)->path.c_str();

		if (BeginCombo(id, defaultText, 0))
		{
			for (Resources::Sound* sound : appInstance->GetResources().GetSoundList())
			{
				if (Selectable(sound->path.c_str()))
				{
					if (!pOutSelectedSound) //No output
					{
						return false;
					}

					*pOutSelectedSound = sound;

					EndCombo();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_SOUND_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::Sound* dragDropSound = *(Resources::Sound**)payload->Data;
				*pOutSelectedSound = dragDropSound;
				ImGui::EndDragDropTarget();
				return true;
			}
			ImGui::EndDragDropTarget();
		}

		return false;
	}

	bool Interfacing::SoundDataListCombo(Resources::SoundData** pOutSelectedSound)
	{
		const char* defaultText = !(*pOutSelectedSound) ? "missingno" : (*pOutSelectedSound)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo("###MeshListCombo", defaultText, 0))
		{
			for (Resources::SoundData* mesh : appInstance->GetResources().GetSoundDataList())
			{
				if (Selectable(mesh->path.c_str()))
				{
					if (!pOutSelectedSound) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedSound = mesh;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_SOUND_DATA_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::SoundData* dragDropSound = *(Resources::SoundData**)payload->Data;
				*pOutSelectedSound = dragDropSound;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::TextureListCombo(Resources::Texture** pOutSelectedTexture)
	{
		PushId(++internalIDStack);

		const char* defaultText = !(*pOutSelectedTexture) ? "missingno" : (*pOutSelectedTexture)->path.c_str();

		if (BeginCombo("###TextureListCombo", defaultText, 0))
		{
			for (Resources::Texture* texture : appInstance->GetResources().GetTextureList())
			{
				if (Selectable(texture->path.c_str()))
				{
					if (!pOutSelectedTexture) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedTexture = texture;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_TEXTURE_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::Texture* dragDropTexture = *(Resources::Texture**)payload->Data;
				*pOutSelectedTexture = dragDropTexture;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::VertexShaderCombo(Resources::VertexShader** pOutSelectedVertexShader)
	{
		const char* defaultText = !(*pOutSelectedVertexShader) ? "missigno" : (*pOutSelectedVertexShader)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo("###VertexShaderCombo", defaultText, 0))
		{
			const auto& list = appInstance->GetResources().GetVertexShaderList();

			if (list.empty()) Selectable("No shader corresponding.");

			for (Resources::VertexShader* shader : list)
			{
				if (Selectable(shader->path.c_str()))
				{
					if (!pOutSelectedVertexShader) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedVertexShader = shader;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_VERTEX_SHADER_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::VertexShader* dragDropShader = *(Resources::VertexShader**)payload->Data;
				*pOutSelectedVertexShader = dragDropShader;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}

		PopId();
		return false;
	}

	bool Interfacing::FragmentShaderCombo(Resources::FragmentShader** pOutSelectedFragmentShader)
	{
		const char* defaultText = !(*pOutSelectedFragmentShader) ? "missigno" : (*pOutSelectedFragmentShader)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo("###FragmentShaderCombo", defaultText, 0))
		{
			const auto& list = appInstance->GetResources().GetFragmentShaderList();

			if (list.empty()) Selectable("No shader corresponding.");

			for (Resources::FragmentShader* shader : list)
			{
				if (Selectable(shader->path.c_str()))
				{
					if (!pOutSelectedFragmentShader) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedFragmentShader = shader;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_FRAGMENT_SHADER_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::FragmentShader* dragDropShader = *(Resources::FragmentShader**)payload->Data;
				*pOutSelectedFragmentShader = dragDropShader;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}
		PopId();
		return false;
	}

	bool Interfacing::GeometryShaderCombo(Resources::GeometryShader** pOutSelectedGeometryShader)
	{
		const char* defaultText = !(*pOutSelectedGeometryShader) ? "missigno" : (*pOutSelectedGeometryShader)->path.c_str();

		PushId(++internalIDStack);

		if (BeginCombo("###FragmentShaderCombo", defaultText, 0))
		{
			const auto& list = appInstance->GetResources().GetGeometryShaderList();

			if (list.empty()) Selectable("No shader corresponding.");

			for (Resources::GeometryShader* shader : list)
			{
				if (Selectable(shader->path.c_str()))
				{
					if (!pOutSelectedGeometryShader) //No output
					{
						PopId();
						return false;
					}

					*pOutSelectedGeometryShader = shader;

					EndCombo();
					PopId();
					return true;
				}
			}
			EndCombo();
		}

		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_GEOMETRY_SHADER_PAYLOAD);

			if (payload != nullptr)
			{
				Resources::GeometryShader* dragDropShader = *(Resources::GeometryShader**)payload->Data;
				*pOutSelectedGeometryShader = dragDropShader;
				ImGui::EndDragDropTarget();
				PopId();
				return true;
			}
			ImGui::EndDragDropTarget();
		}

		PopId();
		return false;
	}

#pragma endregion

#pragma region Log

	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;
	bool                ScrollToBottom;

	const char* levelStrings[] =
	{
		"INFO    : ",
		"LOG     : ",
		"WARNING : ",
		"ERROR   : ",
	};

	const char* buttonStrings[] =
	{
		"Infos",
		"Logs",
		"Warnings",
		"Errors",
	};

	void Interfacing::Clear()
	{
		Buf.clear();
		LineOffsets.clear();
		DebugLevelList.clear();
	}

	void Interfacing::AddLog(const char* fmt)
	{
		int old_size = Buf.size();
		Buf.append(fmt);
		for (int new_size = Buf.size(); old_size < new_size; old_size++)
			if (Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void Interfacing::ShowLog()//Log Interface
	{
		if (openLog)
		{
			bool result = ImGui::Begin("Log", &openLog);
			if (!result)
			{
				ImGui::End();
				return;
			}
			if (ImGui::Button("Clear")) Clear();
			ImGui::SameLine();
			bool copy = ImGui::Button("Copy");
			ImGui::SameLine();
			for (u8 i = 0; i < logBitMask.size(); i++)
			{
				bool tmp = logBitMask[i];
				ImGui::Checkbox(buttonStrings[i], &tmp);
				logBitMask[i] = tmp;
				ImGui::SameLine();
			}
			Filter.Draw("Filter", -100.0f);
			ImGui::Separator();
			ImGui::BeginChild("scrolling");
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
			if (copy) ImGui::LogToClipboard();
			u64 it = 0;
			const char* buf_begin = Buf.begin();
			const char* line = buf_begin;
			for (int line_no = 0; line != nullptr && line[0] != 0; line_no++)
			{
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : nullptr;
				u8 level = DebugLevelList[it];
				if (logBitMask[static_cast<u64>(level)] && (!Filter.IsActive() || Filter.PassFilter(line, line_end) || Filter.PassFilter(levelStrings[DebugLevelList[it]])))
				{
					switch (level)
					{
					case 0:
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
						break;
					case 1:
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
						break;
					case 2:
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						break;
					case 3:
						ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
						break;
					default:
						break;
					}
					ImGui::TextUnformatted(levelStrings[level]);
					ImGui::PopStyleColor();
					ImGui::SameLine();
					ImGui::TextUnformatted(line, line_end);
				}
				it++;
				line = line_end && line_end[1] ? line_end + 1 : NULL;
			}

			if (ScrollToBottom)
				ImGui::SetScrollHereX(1.0f);
			ScrollToBottom = false;
			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::End();
		}
	}

	void Interfacing::SetLevel(DEBUG_LEVEL level)//Set Log Level
	{
		DebugLevelList.push_back(static_cast<char>(level));
	}

	std::array<f32, 128> frames = std::array<f32, 128>();
	u64 currentFrame = 0;
	f32	average = 0.0f;
	u64 frameCounter = 0;
	u64 timeCounter = 0;

	void Interfacing::UpdateFps(f32 dt, u64 gTime)
	{
		if (currentFrame < frames.size())
		{
			frames[currentFrame++] = dt;
		}
		else
		{
			std::copy(frames.data() + 1, frames.data() + frames.size(), frames.data());
			frames.back() = dt;

			if (gTime != timeCounter)
			{
				average = 0.0f;
				for (u32 i = 0; i < 16; i++)
				{
					average += 1 / frames[currentFrame - 1 - i];
				}
				average /= 16.0f;
				timeCounter = gTime;
			}
		}
		frameCounter++;
	}
#pragma endregion

#pragma region FileExplorer

	std::string searchText;
	std::filesystem::path defaultPath = std::filesystem::current_path();
	std::filesystem::path path = defaultPath;
	std::vector<std::filesystem::path > filteredFiles;
	std::string         clickedFolder;
	std::vector<std::string> dirList;
	std::vector<std::string> fileList;
	std::vector<std::string> extentionList;
	std::vector<std::string> pathList;

	void Interfacing::ReadFolder()
	{
		ClearList();
		struct stat sb;
		if (!std::filesystem::exists(path) || (std::filesystem::status(path).permissions() & std::filesystem::perms::others_read) == std::filesystem::perms::none) return;


		if (path.string().size() < defaultPath.string().size())
			path = defaultPath;

		std::string lowerSearchText;
		for (char c : searchText)
		{
			lowerSearchText.push_back(tolower(c));
		}
		for (const auto& entry : std::filesystem::directory_iterator(path, std::filesystem::directory_options::skip_permission_denied))
		{

			std::filesystem::path outfilename = entry.path();
			std::string outfilename_str = outfilename.string();
			std::string lowerFileName;
			for (char c : outfilename_str)
			{
				lowerFileName.push_back(tolower(c));
			}
			if (lowerFileName.find(lowerSearchText) == std::string::npos) continue;
			const char* path1 = outfilename_str.c_str();
			if (stat(path1, &sb)) continue;

			if ((sb.st_mode & S_IFDIR))
			{
				dirList.push_back(outfilename.filename().string());
				pathList.push_back(outfilename.string());
			}
			else
			{
				fileList.push_back(outfilename.filename().string());
				extentionList.push_back(outfilename.extension().string());
				pathList.push_back(outfilename.string());
			}
		}
	}

	void Interfacing::DrawFolder()
	{
		float windowWidth = GetWindowSize().x;
		ImGui::SetCursorPosX(-110);
		u32 totalID = 0;
		for (u64 i = 0; i < dirList.size(); i++)
		{
			ImGui::PushID(totalID);
			if (ImGui::GetCursorPosX() + 210 > windowWidth)
			{
				ImGui::SetCursorPos(Maths::Vec2(0, ImGui::GetCursorPos().y + 110 + 3 * ImGui::GetTextLineHeightWithSpacing()));
			}
			else
			{
				ImGui::SetCursorPos(Maths::Vec2(ImGui::GetCursorPos().x + 110, ImGui::GetCursorPos().y));
			}
			DrawIcon(reinterpret_cast<Resources::Texture*>(iconFolder), dirList[i], false);
			ImGui::PopID();
			totalID++;
		}
		for (u64 i = 0; i < fileList.size(); i++)
		{
			ImGui::PushID(totalID);
			if (ImGui::GetCursorPosX() + 210 > windowWidth)
			{
				ImGui::SetCursorPos(Maths::Vec2(0, ImGui::GetCursorPos().y + 110 + 3 * ImGui::GetTextLineHeightWithSpacing()));
			}
			else
			{
				ImGui::SetCursorPos(Maths::Vec2(ImGui::GetCursorPos().x + 110, ImGui::GetCursorPos().y));
			}
			if (extentionList[i] == ".png")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconPng), fileList[i], true);
			}
			else if (extentionList[i] == ".txt")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconTxt), fileList[i], true);
			}
			else if (extentionList[i] == ".obj" || extentionList[i] == ".fbx")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconObj_Fbx), fileList[i], true);
			}
			else if (extentionList[i] == ".jpg")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconJpg), fileList[i], true);
			}
			else if (extentionList[i] == ".cpp")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconCpp), fileList[i], true);
			}
			else if (extentionList[i] == ".h" || extentionList[i] == ".hpp")
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconH), fileList[i], true);
			}
			else
			{
				DrawIcon(reinterpret_cast<Resources::Texture*>(iconUnknown), fileList[i], true);
			}
			ImGui::PopID();
			totalID++;
		}
	}
	void Interfacing::DrawIcon(Resources::Texture* icon, const std::string& text, bool open)//DrawIcon with text bellow
	{
		Maths::Vec2 cursor = ImGui::GetCursorPos();
		ImGui::ImageButton("File", icon->GetImageView().imGuiDS, Maths::Vec2(100, 100));
		bool doubleClicked = ImGui::IsMouseDoubleClicked(0);
		bool clicked = ImGui::IsItemHovered() && doubleClicked;
		for (u32 i = 0; i < 3 && 12llu * i < text.size(); i++)
		{
			ImGui::SetCursorPos(cursor + Maths::Vec2(10, 110 + i * ImGui::GetTextLineHeightWithSpacing()));
			ImGui::TextUnformatted(text.substr(i * 12llu, 12).c_str());
			if (ImGui::IsItemHovered() && doubleClicked) clicked = true;
		}

		if (clicked)
		{
			if (open)
			{
				const std::string scenePath = std::string().append(std::filesystem::path(path).append(text).string());
				if (GetExtentionFile(text) == "sbin")
				{
					appInstance->GetSceneManager().LoadScene(scenePath.c_str());
				}
				else
				{
					appInstance->GetWindow().ExecuteCommand(std::string("\"").append(std::filesystem::path(path).append(text).string()).append("\""));
				}
			}
			else
			{
				clickedFolder = text;
			}
		}
		ImGui::SetCursorPos(cursor);
	}

	void Interfacing::ShowFileExplorer()//Interface file explorer
	{
		if (openFileExplorer)
		{
			BeginClose("File Explorer", openFileExplorer);

			bool edited = ImGui::InputText("Search File", &searchText);
			ImGui::SameLine();
			if (ImGui::Button("Parent folder"))
			{
				path = path.parent_path();
				clickedFolder.clear();
				ReadFolder();
				edited = false;
			}

			if (!clickedFolder.empty())
			{
				path.append(clickedFolder);
				clickedFolder.clear();
				ReadFolder();
			}
			else if (edited)
			{
				ReadFolder();
			}

			DrawFolder();

			End();
		}
	}

	void Interfacing::ShowMaterialPopup()
	{
		if (materialPopup)
		{
			bool closing = true;
			BeginClose("Material List", closing);
			for (Resources::Material* mat : Core::App::GetInstance()->GetResources().GetMaterialList())
			{
				if (Button(mat->path.c_str()))
				{
					*materialPopup = mat;
					closing = false;
				}
			}
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				closing = false;
			}
			End();
			if (!closing) materialPopup = nullptr;
		}
	}

	void Interfacing::ShowTexturePopup()
	{
		if (selectedTexture)
		{
			bool closing = true;
			BeginClose("Texture List", closing);
			for (auto* texture : appInstance->GetResources().GetTextureList())
			{
				Bullet();
				if (Button(texture->path.c_str()))
				{
					*selectedTexture = texture;
					closing = false;
				}
				SameLine();
				Image(texture, Maths::Vec2(20, 20));
			}
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				closing = false;
			}
			End();
			if (!closing) selectedTexture = nullptr;
		}
	}

	void Interfacing::ShowCubeMapPopup()
	{
		if (selectedCubeMap)
		{
			bool closing = true;
			BeginClose("Cube Map List", closing);
			for (auto* cubeMap : appInstance->GetResources().GetCubeMapList())
			{
				Bullet();
				if (Button(cubeMap->path.c_str()))
				{
					*selectedCubeMap = cubeMap;
					closing = false;
				}
				SameLine();
				Image(cubeMap, Maths::Vec2(20, 20));
			}
			if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				closing = false;
			}
			End();
			if (!closing) selectedCubeMap = nullptr;
		}
	}

	void Interfacing::SelectMaterial(Resources::Material** material)
	{
		materialPopup = material;
	}

	void Interfacing::ShowMeshPopup()
	{
		if (meshPopup)
		{
			bool closing = true;
			BeginClose("Mesh List", closing);
			for (Resources::Mesh* mesh : Core::App::GetInstance()->GetResources().GetMeshList())
			{
				if (Button(mesh->path.c_str()))
				{
					*meshPopup = mesh;
					closing = false;
				}
			}
			End();
			if (!closing) meshPopup = nullptr;
		}
	}

	void Interfacing::SelectMesh(Resources::Mesh** mesh)
	{
		meshPopup = mesh;
	}

	void Interfacing::SelectTexture(Resources::Texture** tex)
	{
		selectedTexture = tex;
	}

	void Interfacing::SelectCubeMap(Resources::CubeMap** cube)
	{
		selectedCubeMap = cube;
	}

	void Interfacing::ClearList()
	{
		dirList.clear();
		fileList.clear();
		extentionList.clear();
		pathList.clear();
	}

#pragma endregion

#pragma region RessourcesExplorer
	bool ifResourcesListSelected;
	Resources::ObjectType resourcesSelected;
	void Interfacing::ShowResourceExplorer()//Like file explorer but with cahe resources upload in project
	{
		if (openResourceExplorer)
		{
			BeginClose("Resources Explorer", openResourceExplorer);
			if (Button("Add resource"))
			{
				openFileExplorerSelectNewResource = true;
			}
			SameLine();
			ImGui::SetCursorPosX(110);
			if (Button("Create Ressource"))
			{
				openListeCreateResourceWindow = true;
			}
			if(openListeCreateResourceWindow) ListeCreateRessources();
			baseResources[idResource]->WindowCreateResource(createResource);
			if (openFileExplorerSelectNewResource)
				OpenFile("All\0*.*\0Text\0*.TXT\0");
			if (ifResourcesListSelected)
			{
				if (Button("Back"))
					ifResourcesListSelected = false;
			}
			ImGui::SetCursorPosX(-100);
			if (!ifResourcesListSelected)
			{
				for (int i = 0; i < nameResourceList.size(); i++)
				{
					IconResource(reinterpret_cast<Resources::Texture*>(iconFolder), nameResourceList[i], true, typeList[i], nullptr, i);
					if (ifResourcesListSelected) break;
				}
			}
			else
			{
				ReadResources(resourcesSelected);
			}

			End();
		}
	}

	void Interfacing::IconResource(Resources::Texture* icon, const std::string& text, bool open, Resources::ObjectType Id, Resources::IResource* resource, s32 pId)
	{
		PushId(pId);

		float windowWidth = GetWindowSize().x;

		if (ImGui::GetCursorPosX() + 210 > windowWidth)
		{
			ImGui::SetCursorPos(Maths::Vec2(10, ImGui::GetCursorPos().y + 110 + 3 * ImGui::GetTextLineHeightWithSpacing()));
		}
		else
		{
			ImGui::SetCursorPos(Maths::Vec2(ImGui::GetCursorPos().x + 110, ImGui::GetCursorPos().y));
		}
		Maths::Vec2 cursor = ImGui::GetCursorPos();
		ImageButton("File", icon->GetImageView().imGuiDS, Maths::Vec2(100, 100));

		if (resource)
		{
			if (Id == Resources::ObjectType::SceneType && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				auto& scenes = appInstance->GetSceneManager().activeScenes;
				scenes.push_back(reinterpret_cast<Core::Scene::Scene*>(resource)->CreateCopy());
				scenes.back()->Init();
				scenes.back()->ForceUpdate();
			}

			if (ImGui::BeginDragDropSource()) //DragDropSource
			{
				ImGui::SetDragDropPayload(dragDropResourcesPayload[(int)Id], &resource, sizeof(resource));
				const ImGuiPayload* payload = ImGui::GetDragDropPayload();
				ImGui::Image(icon->GetImageView().imGuiDS, { 60, 60 });
				ImGui::EndDragDropSource();
			}

			if (!resource->isSaved)
			{
				ImGui::SetCursorPos(cursor);
				ImageButton("File", saveTexture->GetImageView().imGuiDS, Maths::Vec2(100, 100));
			}
		}

		//edited
		if (ifResourcesListSelected)
		{
			if (ImGui::BeginPopupContextItem())
			{
				if (resource->hash >= 0x70)
				{
					if (ImGui::Button("Edit"))
					{
						{
							currentEditResource = resource;
							ImGui::CloseCurrentPopup();
						}
					}
					if (ImGui::Button("Delete"))
					{
						appInstance->GetResources().DeleteResource(resource);
						ImGui::CloseCurrentPopup();
					}
					if (!resource->isSaved)
					{
						if (Button("Save"))
						{
							if (Id == Resources::ObjectType::ModelType)
							{
								tempModel = reinterpret_cast<Resources::Model*>(resource);
								for (auto mesh : tempModel->meshes)
								{
									appInstance->GetResources().SaveAsset(mesh);
									mesh->isSaved = true;
								}
								for (auto mat : tempModel->materials)
								{
									appInstance->GetResources().SaveAsset(mat);
									mat->isSaved = true;
									appInstance->GetResources().SaveAsset(mat->albedo);
									mat->albedo->isSaved = true;
									appInstance->GetResources().SaveAsset(mat->normal);
									mat->normal->isSaved = true;
									appInstance->GetResources().SaveAsset(mat->height);
									mat->height->isSaved = true;
								}
							}
							appInstance->GetResources().SaveAsset(resource);
							resource->isSaved = true;
						}
					}
				}
				else
				{
					ImGui::TextUnformatted("Default assets cannot be edited");
				}
				ImGui::EndPopup();
			}
			if (resource->hash >= 0x70 && ImGui::IsItemHovered())
				ImGui::SetTooltip("Right-click to edit");

		}
		//
		bool doubleClicked = ImGui::IsMouseDoubleClicked(0);
		bool rightClick = ImGui::IsItemClicked(1);
		bool clicked = ImGui::IsItemHovered() && doubleClicked;
		for (u32 i = 0; i < 3 && 12llu * i < text.size(); i++)
		{
			ImGui::SetCursorPos(cursor + Maths::Vec2(10, 110 + i * ImGui::GetTextLineHeightWithSpacing()));
			ImGui::TextUnformatted(text.substr(i * 12llu, 12).c_str());
			if (ImGui::IsItemHovered() && doubleClicked) clicked = true;
		}

		if (clicked)
		{
			if (open)
			{
				resourcesSelected = Id;
				ifResourcesListSelected = true;
			}
			else
			{
				IResourceClicked = resource;
			}
		}
		ImGui::SetCursorPos(cursor);
		PopId();
	}

	void Interfacing::ReadResources(Resources::ObjectType resourceId)
	{
		s32 index = 0;
		switch (resourceId)
		{
		case Resources::ObjectType::ModelType:
			for (Resources::Model* model : appInstance->GetResources().GetModelList())
			{
				IconResource(iconObj_Fbx, model->path, false, resourceId, model, ++index);
			}
			break;
		case Resources::ObjectType::SkinnedModelType:
			// TODO
			break;
		case Resources::ObjectType::TextureSamplerType:
			for (Resources::TextureSampler* sampler : appInstance->GetResources().GetTextureSamplerList())
			{
				IconResource(iconUnknown, sampler->path, false, resourceId, sampler, ++index);
			}
			break;
		case Resources::ObjectType::TextureType:
			for (Resources::Texture* texture : appInstance->GetResources().GetTextureList())
			{
				if (!texture) continue;
				IconResource(texture, texture->path, false, resourceId, texture, ++index);
			}
			break;
		case Resources::ObjectType::MaterialType:
			for (Resources::Material* material : appInstance->GetResources().GetMaterialList())
			{
				IconResource(iconUnknown, material->path, false, resourceId, material, ++index);
			}
			break;
		case Resources::ObjectType::MeshType:
			for (Resources::Mesh* mesh : appInstance->GetResources().GetMeshList())
			{
				IconResource(iconUnknown, mesh->path, false, resourceId, mesh, ++index);
			}
			break;
		case Resources::ObjectType::VertexShaderType:
			for (Resources::VertexShader* vertex : appInstance->GetResources().GetVertexShaderList())
			{
				IconResource(iconTxt, vertex->path, false, resourceId, vertex, ++index);
			}
			break;
		case Resources::ObjectType::FragmentShaderType:
			for (Resources::FragmentShader* fragment : appInstance->GetResources().GetFragmentShaderList())
			{
				IconResource(iconTxt, fragment->path, false, resourceId, fragment, ++index);
			}
			break;
		case Resources::ObjectType::GeometryShaderType:
			for (Resources::GeometryShader* goemetry : appInstance->GetResources().GetGeometryShaderList())
			{
				IconResource(iconTxt, goemetry->path, false, resourceId, goemetry, ++index);
			}
			break;
		case Resources::ObjectType::ShaderProgramType:
			for (Resources::ShaderProgram* shaderProg : appInstance->GetResources().GetShaderProgramList())
			{
				IconResource(iconUnknown, shaderProg->path, false, resourceId, shaderProg, ++index);
			}
			break;
		case Resources::ObjectType::CubeMapType:
			for (Resources::CubeMap* cubeMap : appInstance->GetResources().GetCubeMapList())
			{
				Resources::StaticTexture tmp;
				tmp.renderTexture.imageView = cubeMap->GetImageView();
				tmp.isLoaded = cubeMap->isLoaded;
				IconResource(&tmp, cubeMap->path, false, resourceId, cubeMap, ++index);
			}
			break;
		case Resources::ObjectType::SoundType:
			for (Resources::Sound* sound : appInstance->GetResources().GetSoundList())
			{
				IconResource(iconUnknown, sound->path, false, resourceId, sound, ++index);
			}
			break;
		case Resources::ObjectType::SoundDataType:
			for (Resources::SoundData* sound : appInstance->GetResources().GetSoundDataList())
			{
				IconResource(iconUnknown, sound->path, false, resourceId, sound, ++index);
			}
			break;
		case Resources::ObjectType::SceneType:
			for (Core::Scene::Scene* scene : appInstance->GetResources().GetSceneList())
			{
				IconResource(iconUnknown, scene->path, false, resourceId, scene, ++index);
			}
			break;
		default:
			break;
		}

	}

	void Interfacing::LoadRessources(std::string path)
	{
		auto loadShaderBase = [&](Resources::Shader* newShader) -> bool
		{
			newShader->shaderData = ShaderLoader::LoadCompileShader(path.c_str());
			if (!newShader->shaderData.size()) return false;
			newShader->isLoaded = appInstance->GetRenderer().LoadShader(newShader);
			if (!newShader->isLoaded) return false;
			newShader->isSaved = false;
			return true;
		};
		std::string extension = GetExtentionFile(path);
		if (extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "hdr")
		{
			Resources::StaticTexture* newTexture = appInstance->GetResources().CreateResource<Resources::StaticTexture>("newTexture");
			ImageLoader::ParseTexture(path.c_str(), newTexture, extension == "hdr" ? Wrappers::TextureLoadingArg::SRGB : Wrappers::TextureLoadingArg::DEFAULT);
			newTexture->UpdateMipLevel();
			newTexture->isLoaded = appInstance->GetRenderer().LoadTexture(newTexture);
			newTexture->isSaved = false;
		}
		else if (extension == "obj" || extension == "fbx" || extension == "blend" || extension == "smd" || extension == "dae" || extension == "gltf" || extension == "glb" || extension == "3ds" || extension == "ase" || extension == "ifc" || extension == "dxf" || extension == "x" || extension == "ms3d")
		{
			Resources::Model* newModel = appInstance->GetResources().CreateResource<Resources::Model>("newModel");
			ModelLoader::AssimpModelLoader::ParseModel(path.c_str(), newModel);
			appInstance->GetRenderer().LoadModel(newModel);
			appInstance->GetResources().Load(newModel->hash);
			newModel->isSaved = false;
		}
		else if (extension == "cbm")
		{
			Resources::StaticCubeMap* newCube = appInstance->GetResources().CreateResource<Resources::StaticCubeMap>("newCubeMap");
			ImageLoader::ParseCubeMap(path.c_str(), newCube);
			newCube->UpdateMipLevel();
			newCube->isLoaded = appInstance->GetRenderer().LoadCubeMap(newCube);
			newCube->isSaved = false;
		}
		else if (extension == "ogg" || extension == "wav" || extension == "mp3" || extension == "flac")
		{
			Resources::SoundData* newSound = appInstance->GetSoundLoader().CreateSoundAsset(path.c_str());
		}
		else if (extension == "frag")
		{
			Resources::Shader* newShader = appInstance->GetResources().CreateResource<Resources::FragmentShader>("newFragmentShader");
			if (!loadShaderBase(newShader))
			{
				appInstance->GetResources().GetFragmentShaderList().pop_back();
			}
		}
		else if (extension == "vert")
		{
			Resources::Shader* newShader = appInstance->GetResources().CreateResource<Resources::VertexShader>("newVertexShader");
			if (!loadShaderBase(newShader))
			{
				appInstance->GetResources().GetVertexShaderList().pop_back();
			}
		}
		else if (extension == "geom")
		{
			Resources::Shader* newShader = appInstance->GetResources().CreateResource<Resources::GeometryShader>("newGeometryShader");
			if (!loadShaderBase(newShader))
			{
				appInstance->GetResources().GetGeometryShaderList().pop_back();
			}
		}
		else
		{
			LOG(DEBUG_LEVEL::LERROR, "Resources can't be charged");
		}
	}

	std::string Interfacing::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = reinterpret_cast<HWND>(appInstance->GetWindow().GetWindowPtr());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			LoadRessources(ofn.lpstrFile);
			openFileExplorerSelectNewResource = false;
			return ofn.lpstrFile;
		}
		else
		{
			openFileExplorerSelectNewResource = false;
		}
		return std::string();
	}

	std::string Interfacing::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = reinterpret_cast<HWND>(appInstance->GetWindow().GetWindowPtr());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
	}

	void Interfacing::ListeCreateRessources()
	{
		ImGui::OpenPopup("Create Ressource");
		if (ImGui::BeginPopupModal("Create Ressource"))
		{
			for (int i = 0; i < nameCreateResourceList.size(); i++)
			{
				if (Button(nameCreateResourceList[i]))
				{
					idResource = i;
					createResource = true;
					openListeCreateResourceWindow = false;
				}

			}
			
			if (Button("close")) openListeCreateResourceWindow = false;
			ImGui::EndPopup();
		}
	
	}



#pragma endregion

	void Interfacing::ShowPostProcessEditor()
	{
		if (openPostProcessEditor)
		{
			BeginClose("Post Process Editor", openPostProcessEditor);
			appInstance->GetSceneManager().ConfigurePostProcesses();
			End();
		}
	}

	void Interfacing::SceneGraph()// show tree of object in the scene
	{
		sceneGraphSelectedObject = clickedGameObject;

		if (BeginClose("Scene Graph", openSceneGraph))
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Delete) && ImGui::IsWindowFocused())
			{
				if (sceneGraphSelectedObject != nullptr)
				{
					sceneGraphSelectedObject->Delete();
					sceneGraphSelectedObject = nullptr;
					clickedGameObject = nullptr;
				}
			}

			if (BeginCombo("Default Scene", appInstance->defaultScene->path.c_str(), 0))
			{
				bool clicked = false;
				for (Core::Scene::Scene* scene : appInstance->GetResources().GetSceneList())
				{
					if (Selectable(scene->path.c_str()))
					{
						appInstance->defaultScene = scene;
						break;
					}
				}
				EndCombo();
			}

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DRAG_DROP_SCENE_PAYLOAD);

				if (payload != nullptr)
				{
					Core::Scene::Scene* dragDropScene = *(Core::Scene::Scene**)payload->Data;
					appInstance->defaultScene = dragDropScene;
					ImGui::EndDragDropTarget();
				}
				ImGui::EndDragDropTarget();
			}

			auto& scenes = appInstance->GetSceneManager();
			Core::Scene::Scene* scene = nullptr;
			for (u64 index = 0; index < scenes.activeScenes.size(); index++)
			{
				scene = scenes.activeScenes[index];
				ImGui::Separator();

				PushId(++internalIDStack);
				std::string cSave = scene->path;
				if (!scene->isSaved)
					cSave += " *";
				if (ImGui::TreeNodeEx(cSave.c_str(), ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (ImGui::IsItemClicked())
					{
						clickedGameObject = nullptr;
					}
					scene->isNodeOpen = true;
					for (u32 i = 0; i < scene->gameObjects.size(); i++)
					{
						PushId((s32)i);
						bool up = ArrowButton("Up", 2);
						SameLine();
						bool down = ArrowButton("Down", 3);
						SameLine();
						if (up && i)
						{
							std::swap(scene->gameObjects[i], scene->gameObjects[i - 1]);
						}
						else if (down && i < scene->gameObjects.size() - 1)
						{
							std::swap(scene->gameObjects[i], scene->gameObjects[i + 1]);
						}
						GameObjectTreeNode(scene->gameObjects[i]);
						PopId();
						
						
					}
					
				}
				else
				{
					scene->isNodeOpen = false;
				}
				if (scene->isNodeOpen == true && !sceneGraphSelectedObject)
				{
					if (Button("Add GameObject"))
					{
						Core::Scene::GameObject* obj = scene->AddChild();
						
						if (obj)
						{
							sceneGraphSelectedObject = obj;
							clickedGameObject = obj;
						}

						scene->isSaved = false;
					}
					if (Button("Unload scene"))
					{
						scenes.UnLoadScene(index);
					}
					if (Button("Save scene"))
					{
						scenes.SaveScene(scene);
					}
				}
				PopId();
			}

			if (sceneGraphSelectedObject)
			{
				if (Button("Add GameObject"))
				{
					Core::Scene::GameObject* obj = sceneGraphSelectedObject->AddChild();

					if (obj)
					{
						sceneGraphSelectedObject = obj;
						clickedGameObject = obj;
					}
					
					scene->isSaved = false;
				}
				if (Button("Duplicate"))
				{
					Core::Scene::GameObject tmp;
					scene->AddChild();
					sceneGraphSelectedObject->CopyTo(scene->gameObjects.back());
					scene->gameObjects.back()->Init();
					scene->gameObjects.back()->ForceUpdate();
					scene->isSaved = false;
				}
				if (Button("Delete"))
				{
					sceneGraphSelectedObject->Delete();
					sceneGraphSelectedObject = nullptr;
					clickedGameObject = nullptr;
					scene->isSaved = false;
				}
				if (Button("Rename") || ImGui::IsKeyPressed(ImGuiKey_F2))
				{
					scene->isSaved = false;
					openRenameWindow = true;
				}
				if (openRenameWindow)
				{
					ImGui::OpenPopup("Rename");
					if (ImGui::BeginPopupModal("Rename"))
					{
						ImGui::InputText("##rename", &sceneGraphSelectedObject->name, sizeof(sceneGraphSelectedObject->name));
						if (ImGui::IsKeyPressed(ImGuiKey_Enter))
						{
							openRenameWindow = false;
						}
						ImGui::EndPopup();
					}
				}
			}
		}
	}

#pragma region Inspector
	void DeleteComponentOfGameObject(IComponent* component, Core::Scene::GameObject* object)
	{
		for (u64 i = 0; i < object->components.size(); i++)
		{
			if (object->components[i] == component)
			{
				component->Delete();

				delete component;
				for (u64 j = i; j < object->components.size() - 1; j++)
				{
					object->components[j] = object->components[j + 1];
				}
				object->components.pop_back();
				break;
			}
		}
	}

	void Interfacing::ShowComponent(Core::Scene::GameObject* gameobject)	/// Show component of object selected in the scene
	{
		if (openInspector)
		{
			std::string windowName = gameobject != nullptr ? "Inspector - " + gameobject->name + "###Inspector" : "Inspector###Inspector";

			BeginClose(windowName.c_str(), openInspector);

			if (!gameobject)
			{
				End();
				return;
			}
			ImGui::TextUnformatted("Name : ");
			ImGui::SameLine();
			ImGui::InputText("GameObject Name", &gameobject->name);

			if (CheckBox("Active", gameobject->IsActive()))
				gameobject->SetActive(!gameobject->IsActive());

			ImGui::Separator();

			bool transformHeaderOpen = true;
			if (ImGui::CollapsingHeader("Transform", &transformHeaderOpen, ImGuiTreeNodeFlags_DefaultOpen))
			{
				Maths::Vec3 tmpPos = gameobject->transform.GetPosition();
				if (DragFloat3("Position", &tmpPos.x, 0.1f))
				{
					gameobject->transform.SetPosition(tmpPos);
					gameobject->ForceUpdate();
				}
				Maths::Quat tmpRot = gameobject->transform.GetRotation();
				if (DragQuat("Rotation", tmpRot, 0.1f))
				{
					gameobject->transform.SetRotation(tmpRot);
					gameobject->ForceUpdate();
				}
				Maths::Vec3 tmpScale = gameobject->transform.GetScale();
				if (unifiedScale)
				{
					if (DragFloat("###Scale", &tmpScale.x, 0.1f))
					{
						gameobject->transform.SetScale(Maths::Vec3(tmpScale.x));
						gameobject->ForceUpdate();
					}
					SameLine();
					SetCursorX(GetCursorX() + 10);
					if (CheckBox("##unifiedScale", unifiedScale)) unifiedScale = !unifiedScale;
				}
				else
				{
					if (DragFloat3("###Scale", &tmpScale.x, 0.1f))
					{
						gameobject->transform.SetScale(tmpScale);
						gameobject->ForceUpdate();
					}
					SameLine();
					SetCursorX(GetCursorX() + 10);
					if (CheckBox("##unifiedScale", unifiedScale)) unifiedScale = !unifiedScale;
				}
			}

			ImGui::Separator();

			s32 index = 0;
			for (Core::Scene::Components::IComponent* c : gameobject->components)
			{
				PushId(++index);
				bool headerIsOpen = true;
				if (CollapsingHeader(c->GetName(), &headerIsOpen, ImGuiTreeNodeFlags_DefaultOpen))
				{
					c->RenderGui();
					if (Button("Delete Component"))
					{
						headerIsOpen = false;
					}
				}

				Separator();

				if (!headerIsOpen)
				{
					DeleteComponentOfGameObject(c, gameobject);
				}
				PopId();
			}

			ImGui::Separator();

			if (openComponentList)
			{
				if (!ImGui::IsKeyPressed(ImGuiKey::ImGuiKey_Escape) && openInspector && sceneGraphSelectedObject != nullptr)
					ComponentList();
				else
					openComponentList = !openComponentList;
			}
			else if (Button("Add Component", Maths::Vec2(100, 25)))
			{
				openComponentList = !openComponentList;
			}

			End();
		}
	}

	void Interfacing::GameObjectTreeNode(Core::Scene::GameObject* gameobject)
	{
		bool noChild = gameobject->childs.empty();
		bool isSelected = gameobject == clickedGameObject;

		ImGuiTreeNodeFlags nodeFlags = (noChild ? ImGuiTreeNodeFlags_Leaf : 0) | (isSelected ? ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_OpenOnArrow : 0) | ImGuiTreeNodeFlags_OpenOnDoubleClick;

		if (ImGui::TreeNodeEx(gameobject->name.c_str(), nodeFlags))
		{
			if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
				clickedGameObject = gameobject;
			if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
				clickedGameObject = nullptr;

			for (auto child : gameobject->childs)
			{
				GameObjectTreeNode(child);
			}

			ImGui::TreePop();
		}
	}

	void Interfacing::ComponentList()
	{
		if (ImGui::BeginCombo("##newComponentCombo", "Add New Component", ImGuiComboFlags_HeightRegular))
		{
			s32 i = 0;
			for (auto& comp : baseComponents)
			{
				PushId(++i);
				Bullet();
				if (ImGui::Selectable(comp->GetName()))
				{
					IComponent* newComponent = comp->CreateCopy();
					sceneGraphSelectedObject->components.push_back(newComponent);
					sceneGraphSelectedObject->components.back()->gameObject = sceneGraphSelectedObject;
					newComponent->Init();

					openComponentList = false;
				}
				PopId();
			}

			ImGui::EndCombo();
		}
	}
#pragma endregion

	void Interfacing::EditorPlayerStart()
	{
		//Save scenes in memory
		mEditorSavedScenes = appInstance->GetSceneManager().SerializeAllScenes();
		appInstance->GetSceneManager().SetPlayMode(Core::Scene::PlayMode::GAME);
	}

	void Interfacing::EditorPlayerStop()
	{
		sceneGraphSelectedObject = nullptr;
		clickedGameObject = nullptr;
		static auto& sceneManager = appInstance->GetSceneManager();

		sceneManager.SetPlayMode(Core::Scene::PlayMode::EDITION);
		sceneManager.Clean();
		sceneManager.DeserializeAllScenes(mEditorSavedScenes);

		mEditorSavedScenes.clear();
	}

	void Interfacing::ButtonPlayPause()
	{
		SetCursorX(appInstance->GetWindow().GetWindowSize().x / 2.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, colorButtonPlay);
		if (Button("|>"))
		{
			switch (appInstance->GetSceneManager().GetPlayMode())
			{
			case Core::Scene::PlayMode::EDITION:
				EditorPlayerStart();
				break;
			case Core::Scene::PlayMode::PLAYING:
				appInstance->GetSceneManager().SetPlayMode(Core::Scene::PlayMode::GAME);
				appInstance->GetWindow().SetCursorCaptured(true);
				break;
			case Core::Scene::PlayMode::GAME:
				EditorPlayerStop();
				break;
			default:
				break;
			}
		}
		ImGui::PopStyleColor();
		SetCursorX(appInstance->GetWindow().GetWindowSize().x / 2.0f);

		if (appInstance->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::PLAYING || appInstance->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Escape))
			{
				if (appInstance->GetSceneManager().IsPlaying())
					EditorPlayerStop();
				else
					EditorPlayerStart();
			}
		}

		if (appInstance->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::PLAYING)
		{
			colorButtonPause.x = 0;
			colorButtonPlay.x = 1;
		}
		if (appInstance->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::EDITION)
		{
			colorButtonPause.x = 1;
			colorButtonPlay.x = 0;
		}
		if (appInstance->GetSceneManager().GetPlayMode() == Core::Scene::PlayMode::GAME)
		{
			colorButtonPause.x = 0;
			colorButtonPlay.x = 0.5;
		}
	}

	void Interfacing::EngineInterface()
	{
		BeginFrame();

		internalIDStack = 0;

		//Menu bar
		BeginMainMenuBar();
		if (BeginMenu("File"))
		{
			if (MenuItem("Exit"))
			{
				glfwSetWindowShouldClose(appInstance->GetWindow().GetGLFWindow(), true);
			}

			EndMenu();
		}
		if (BeginMenu("Display"))
		{
			if (MenuItem("Log", 0, openLog))
			{
				openLog = !openLog;
			}

			if (MenuItem("Fps", 0, openFps))
			{
				openFps = !openFps;
			}

			if (MenuItem("FileExplorer", 0, openFileExplorer))
			{
				openFileExplorer = !openFileExplorer;
			}

			if (MenuItem("Scene Graph", 0, openSceneGraph))
			{
				openSceneGraph = !openSceneGraph;
			}

			if (MenuItem("Inspector", 0, openInspector))
			{
				openInspector = !openInspector;
			}

			if (MenuItem("Resources Explorer", 0, openResourceExplorer))
			{
				openResourceExplorer = !openResourceExplorer;
			}

			if (MenuItem("Post Process editor", 0, openPostProcessEditor))
			{
				openPostProcessEditor = !openPostProcessEditor;
			}

			EndMenu();
		}


		if (appInstance->GetWindow().IsFullScreen())
		{
			SetCursorX(appInstance->GetWindow().GetWindowSize().x - 20.0f);
			if (Button("X", { 20,20 }))
			{
				glfwSetWindowShouldClose(appInstance->GetWindow().GetGLFWindow(), true);
			}
		}


		ShowLog();
		ShowFps(frames, average, frameCounter);
		ShowFileExplorer();
		ShowResourceExplorer();
		ShowPostProcessEditor();
		ShowComponent(sceneGraphSelectedObject);
		ShowMaterialPopup();
		ShowMeshPopup();
		ShowCubeMapPopup();
		ShowTexturePopup();

		if (currentEditResource && currentEditResource->hash >= 0x70 && currentEditResource->ShowEditWindow()) 
			currentEditResource = nullptr;

		if (openSceneGraph)
		{
			SceneGraph();

			if (openComponentList && !sceneGraphSelectedObject)
				openComponentList = false;

			End();
		}

		ButtonPlayPause();

		EndMainMenuBar();
	}

	void Interfacing::SetSelectedObject(Core::Scene::GameObject* object)
	{
		clickedGameObject = object;
	}

	///Deprecated : Use Interfacing::shaderCombo() instead
	void Interfacing::GetShaderList(Resources::ShaderProgram*& shader, bool& open)
	{
		BeginClose("Shader List", open);
		for (Resources::ShaderProgram* s : Core::App::GetInstance()->GetResources().GetShaderProgramList())
		{
			if (s->GetShaderType() != Resources::ShaderVariant::Default && s->GetShaderType() != Resources::ShaderVariant::WireFrame) continue;
			if (Button(s->path.c_str()))
			{
				shader = s;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetVertShaderList(Resources::VertexShader*& shader, bool& open)
	{
		BeginClose("Shader List", open);
		for (Resources::VertexShader* s : Core::App::GetInstance()->GetResources().GetVertexShaderList())
		{
			//if (s->GetType() != Resources::ShaderVariant::Default && s->GetType() != Resources::ShaderVariant::WireFrame) continue;
			if (Button(s->path.c_str()))
			{
				shader = s;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetFragShaderList(Resources::FragmentShader*& shader, bool& open)
	{
		BeginClose("Shader List", open);
		for (Resources::FragmentShader* s : Core::App::GetInstance()->GetResources().GetFragmentShaderList())
		{
			//if (s->GetType() != Resources::ShaderVariant::Default && s->GetType() != Resources::ShaderVariant::WireFrame) continue;
			if (Button(s->path.c_str()))
			{
				shader = s;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetGeoShaderList(Resources::GeometryShader*& shader, bool& open)
	{
		BeginClose("Shader List", open);
		for (Resources::GeometryShader* s : Core::App::GetInstance()->GetResources().GetGeometryShaderList())
		{
			//if (s->GetType() != Resources::ShaderVariant::Default && s->GetType() != Resources::ShaderVariant::WireFrame) continue;
			if (Button(s->path.c_str()))
			{
				shader = s;
				open = false;
			}
		}
		End();
	}

	///Deprecated : Use Interfacing::MaterialListCombo() instead
	void Interfacing::GetMaterialList(Resources::Material*& mat, bool& open)
	{
		BeginClose("Material List", open);
		for (Resources::Material* matlist : Core::App::GetInstance()->GetResources().GetMaterialList())
		{
			if (Button(matlist->path.c_str()))
			{
				mat = matlist;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetMeshList(Resources::Mesh*& mesh, bool& open)
	{
		BeginClose("Mesh List", open);
		for (Resources::Mesh* meshlist : Core::App::GetInstance()->GetResources().GetMeshList())
		{
			if (Button(meshlist->path.c_str()))
			{
				mesh = meshlist;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetModelList(Resources::Model& model, bool& open)
	{
		BeginClose("Model List", open);
		for (Resources::Model* modellist : Core::App::GetInstance()->GetResources().GetModelList())
		{
			if (Button(modellist->path.c_str()))
			{
				model = *modellist;
				open = false;
			}
		}
		End();
	}

	void Interfacing::GetModelListToRenderModelComponent(Core::Scene::Components::RenderModelComponent* model, bool& open)
	{
		BeginClose("Model List", open);
		for (Resources::Model* modellist : Core::App::GetInstance()->GetResources().GetModelList())
		{
			if (Button(modellist->path.c_str()))
			{
				model->UseModel(modellist);
				open = false;
			}
		}
		End();
	}
	///Draw combo list of all models in manager, when a model is selected it will copy the pointer and return true


	void Interfacing::Separator()
	{
		ImGui::Separator();
	}

	void Interfacing::NewLine()
	{
		ImGui::NewLine();
	}

	void Interfacing::ColoredText(const char* pText, Maths::Vec4 pColor)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, pColor);
		Text(pText);
		ImGui::PopStyleColor();
	}

	void Interfacing::SetNextWindowPosition(Maths::Vec2 pPosition)
	{
		ImGui::SetNextWindowSize({pPosition.x, pPosition.y});
	}

	Core::Scene::GameObject* Interfacing::GetSelectedGameObject() const
	{
		return sceneGraphSelectedObject;
	}
}


