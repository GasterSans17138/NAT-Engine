#pragma once

#ifdef _WIN32
	#pragma warning(disable : 26812)
#endif

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>
#include <vector>
#include <optional>
#include <array>

#include "Maths/Maths.hpp"
#include "Core/Types.hpp"

#include "Renderer/RendererFrameBuffer.hpp"
#include "Renderer/RendererDepthBuffer.hpp"
#include "Renderer/RendererPipeline.hpp"
#include "Renderer/RendererShader.hpp"
#include "Renderer/RendererShaderProgram.hpp"
#include "Renderer/RendererTexture.hpp"
#include "Renderer/RendererTextureSampler.hpp"
#include "Renderer/Uniform/RendererMainUniform.hpp"
#include "Renderer/Uniform/RendererLightUniform.hpp"
#include "Renderer/Uniform/RendererPostUniform.hpp"
#include "Renderer/Uniform/RendererWindowUniform.hpp"
#include "Renderer/RendererMesh.hpp"
#include "Renderer/RendererFrameBuffer.hpp"
#include "Renderer/RendererBuffer.hpp"
#include "Renderer/rendererImageView.hpp"

#include "Core/Scene/Scene.hpp"

#include "Core/Scene/Components/RenderModelComponent.hpp"
#include "Core/Scene/Components/Rendering/CameraComponent.hpp"

#include "LowRenderer/RenderPassType.hpp"
#include "LowRenderer/FrameBuffer.hpp"

namespace Resources
{
	class Shader;
	class Mesh;
	class Model;
	class StaticTexture;
	class Texture;
	class TextureSampler;
	class SkinnedModel;
	class CubeMap;
	class StaticCubeMap;
	class ShaderProgram;
	class Material;
}

namespace LowRenderer::PostProcess
{
	class PostProcessEffect;
}

namespace Core::Scene::Components
{
	class RenderModelComponent;

	namespace Lights
	{
		class DirectionalLightComponent;
		class PointLightComponent;
		class SpotLightComponent;
	}
}

#define DESCRIPTOR_POOL_SIZE 16
const s32 MAX_FRAMES_IN_FLIGHT = 1;
//const u32 SHADOWMAP_RESOLUTION = 2048u;
namespace Wrappers
{
	class Interfacing;
}

namespace Renderer
{
	struct QueueFamilyIndices
	{
		std::optional<u32> graphicsFamily;
		std::optional<u32> transferFamily;
		std::optional<u32> presentFamily;
		bool isComplete()
		{
			return graphicsFamily.has_value() && transferFamily.has_value() && presentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	class VulkanRenderer;
	class UniformBufferPool;

	class NAT_API UniformElement
	{
	public:
		UniformElement() = default;
		UniformElement(void* ptr, u64 id, VkBuffer buf, VkDeviceMemory mem);
		~UniformElement() = default;

		Uniform::MainVertexUniform* GetVertexUniform(VulkanRenderer& renderer);
		Uniform::MainFragmentUniform* GetFragmentUniform(VulkanRenderer& renderer);
		Uniform::LightFragmentUniform* GetLightFragmentUniform(VulkanRenderer& renderer);
		Uniform::PostFragmentUniform* GetPostFragmentUniform(VulkanRenderer& renderer);
		u64 GetOffset();
		VkBuffer& GetBuffer();
		VkDeviceMemory& GetMemory();
	private:
		void* mappedPointer = nullptr;
		u64 index = 0;
		VkBuffer buffer = {};
		VkDeviceMemory memory = {};
		friend UniformBufferPool;
	};

	struct UniformBufferObject
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		void* mappedMemory;
	};

	class NAT_API FrameDescriptorPool
	{
	public:
		FrameDescriptorPool();
		~FrameDescriptorPool();

		void CreatePools(VulkanRenderer& renderer, u32 count = 1024, u32 uniformBuf = 2, u32 images = 3);
		void DestroyPools(VkDevice& device);

		VkDescriptorSet GetNext(VulkanRenderer& renderer, Resources::ShaderVariant variant = Resources::ShaderVariant::Default);
		void UpdatePool(VkDevice& device, VulkanRenderer& renderer);
		
	private:
		std::array<VkDescriptorPool, DESCRIPTOR_POOL_SIZE> framePool = {};
		std::array<bool, DESCRIPTOR_POOL_SIZE> poolState = {};
		u32 currentPos = 0;
	};

	class NAT_API UniformBufferPool
	{
	public:
		UniformBufferPool();
		~UniformBufferPool();

		void CreatePools(VulkanRenderer& renderer, u32 count, Renderer::Uniform::RendererUniformObject* uniform);
		void DestroyPools(VkDevice& device);

		UniformElement GetNext();
		void UpdatePool();

	private:
		std::array<UniformBufferObject, DESCRIPTOR_POOL_SIZE> framePool = {};
		u64 currentPos = 0;
		u64 bufferSize = 0;
		u32 count = 0;
	};

	enum PipelineParams : u32
	{
		DEFAULT = 0,
		WIREFRAME = 1,
		LINE = 2,
		SHADOW = 4,
		EXTRA_ATTACHMENT = 8,
		LIGHT_PASS = 16,
		POST_PROCESS = 32,
		STENCIL = 64,
		DEPTH_CLEAR = 128,
		WINDOW = 256,
	};

	enum class NAT_API StencilState : u8
	{
		DEFAULT = 0,
		INCREMENT,
		NO_DEPTH,
		GREATER,
	};

	class NAT_API VulkanRenderer
	{
	public:
		VulkanRenderer();
		VulkanRenderer(const VulkanRenderer&) = delete;
		VulkanRenderer(VulkanRenderer&) = delete;

		~VulkanRenderer();

		bool LoadShader(Resources::Shader* p_shader);
		bool LoadShaderProgram(Resources::ShaderProgram* p_shader, LowRenderer::RenderPassType targetPass);
		bool LoadShaderProgram(Resources::ShaderProgram* p_shader, u32 targetPass);
		bool LoadModel(Resources::Model* p_model);
		bool LoadMesh(Resources::Mesh* p_Mesh);
		//bool LoadMesh(Renderer::RendererMesh* pMesh, vertices, indices);
		bool LoadSkinnedModel(Resources::SkinnedModel* p_model);
		bool LoadTexture(Resources::StaticTexture* p_texture);
		bool LoadCubeMap(Resources::StaticCubeMap* p_cubemap);
		// TODO add support for more filtering & wrapping modes
		void LoadTextureSampler(Resources::TextureSampler* sampler, bool linearFiltering = true, bool wrap = true);
		std::string ReadTexture(const Resources::Texture* tex, Maths::IVec2 offset, Maths::IVec2 size, u64 pixelSize);

		void PreInit();
		void Init(void* window, Maths::IVec2 defaultRes, bool useVSync, bool pInitEditor = false);
		void CleanupBuffers();
		void Cleanup();
		void ResizeFrameBuffer(Maths::IVec2 newRes);
		void SetVSync(bool newValue);
		void* GetInstance();
		void* GetSurface();
		void BeginFrame(Wrappers::Interfacing* pInterface);
		void EndFrame(Wrappers::Interfacing* pInterface);
		void BeginPass(LowRenderer::FrameBuffer* fb = nullptr, LowRenderer::RenderPassType renderPass = LowRenderer::RenderPassType::DEFAULT);
		void EndPass();
		Maths::Vec3 GetClearColor();
		void SetClearColor(Maths::Vec4 color);

		void CreateFrameBuffer(RendererFrameBuffer& buf, const std::vector<RendererImageView>& attachments, Maths::IVec2 targetResolution, Maths::Vec4 clearColor, VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT, Resources::ShaderVariant variant = Resources::ShaderVariant::Default);
		void CreateDepthBuffer(RendererDepthBuffer& db, Maths::IVec2 targetResolution);
		void CreateBuffer(Renderer::RendererBuffer& b, Maths::IVec2 resolution, bool transitLayout = false);
		void DeleteFrameBuffer(RendererFrameBuffer& buf);
		void DeleteDepthBuffer(RendererDepthBuffer& buf);
		void DeleteBuffer(RendererBuffer& buf);
		void WaitForVSync();

		void RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, const Maths::Vec3& color = Maths::Vec3(1));
		void RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, const Resources::Material* mat);
		void RenderMesh(const Resources::Mesh* mesh, const Maths::Mat4& m, const Maths::Mat4& mvp, std::vector<const RendererTexture*> textures, Resources::Material* materialOverride = Resources::Material::GetDefaultMaterial());
		void RenderMeshObject(const Resources::Mesh* mesh, const Maths::Mat4& mvp, u32 objectID);
		void DrawVertices(u32 count, const Maths::Mat4& m, const Maths::Mat4& mvp);
		void DrawVertices(u32 count, const Maths::Mat4& m, const Maths::Mat4& mvp, std::vector<const RendererTexture*> textures);
		void DrawIndexedRenderMesh(const Renderer::RendererMesh* pRenderMesh, const Maths::Mat4& pModelMatrix, u32 pVertexCount, u32 pIndiceCount);
		void ApplyLightPass(const Resources::ShaderProgram* shader);
		void ApplyPostProcess(const LowRenderer::PostProcess::PostProcessEffect* effect);
		void RenderToWindow();
		void BindShader(const Resources::ShaderProgram* p_shader);
		void UnLoadShader(Resources::Shader* p_shader);
		void UnLoadShaderProgram(Resources::ShaderProgram* p_shader);
		void UnLoadModel(Resources::Model* p_model);
		void UnLoadTexture(Resources::StaticTexture* p_texture);
		void UnLoadTextureSampler(Resources::TextureSampler* sampler);
		void UnLoadCubeMap(Resources::StaticCubeMap* p_cubemap);
		void UnLoadSkinnedModel(Resources::SkinnedModel* p_model);
		void UnloadMesh(Resources::Mesh* pMesh);

		void FreeVertexBuffer(VertexBuffer& pBuffer);
		void FreeIndiceBuffer(IndiceBuffer& pBuffer);

		void WaitIdle();
		void PrepareFrameBuffer(LowRenderer::FrameBuffer* fb);
		void AddDirectionalLight(Core::Scene::Components::Lights::DirectionalLightComponent* dl);
		void AddPointLight(Core::Scene::Components::Lights::PointLightComponent* dl);
		void AddSpotLight(Core::Scene::Components::Lights::SpotLightComponent* dl);
		void ClearLights();
		Maths::Vec2 GetLineRange() const { return lineRange; }
		f32 GetLineWidth() const { return currentWidth; }
		void SetLineWidth(f32 newWidth) { currentWidth = Maths::Util::Clamp(newWidth, lineRange.x, lineRange.y); };
		void SetStencilState(u8 compareValue, StencilState state);

		Renderer::VertexBuffer CreateVertexBuffer(const RendererVertex* pVertices, const unsigned int& pVerticesCount);
		Renderer::IndiceBuffer CreateIndiceBuffer(const u32* pIndices, const unsigned int& pIndicesCount);

		void InitRendererData();

		LowRenderer::FrameBuffer GetMainColorBuffer() const;
		LowRenderer::FrameBuffer GetMainLightBuffer() const;
		LowRenderer::FrameBuffer GetMainObjectBuffer() const;
		void ResetMainFB();
		void ToggleMainFB();

		Maths::Vec3 currentCameraPos;
		bool enableValidationLayers = true;

		void SetCurrentCamera(const LowRenderer::Rendering::Camera* pCamera);

	private:
		const LowRenderer::Rendering::Camera* currentCamera = nullptr;
		
		VkInstance instance= {};
		VkDebugUtilsMessengerEXT debugMessenger = {};
		VkPhysicalDevice physicalDevice = {};
		VkDevice device = {};
		VkQueue graphicsQueue = {};
		VkQueue transferQueue = {};
		VkQueue presentQueue = {};
		VkSurfaceKHR surface = {};
		VkSwapchainKHR swapChain = {};
		VkFormat swapChainImageFormat = {};
		VkExtent2D swapChainExtent = {};
		VkRenderPass geometryRenderPass = {};
		VkRenderPass objectRenderPass = {};
		VkRenderPass lightRenderPass = {};
		VkRenderPass postRenderPass = {};
		VkRenderPass windowRenderPass = {};
		std::vector<RendererFrameBuffer> swapChainFramebuffers = {};
		VkCommandPool graphicCommandPool = {};
		VkCommandPool transferCommandPool = {};
		std::vector<VkCommandBuffer> commandBuffers = {};
		std::vector<VkSemaphore> imageAvailableSemaphores = {};
		std::vector<VkSemaphore> renderFinishedSemaphores = {};
		std::vector<VkFence> inFlightFences = {};
		QueueFamilyIndices queueFamilyIndices = {};
		u32 imageCount = {};
		std::vector<FrameDescriptorPool> descriptorPools = {};
		std::vector<UniformBufferPool> uniformPools = {};
		std::vector<FrameDescriptorPool> ldescriptorPools = {};
		std::vector<UniformBufferPool> luniformPools = {};
		std::vector<FrameDescriptorPool> pdescriptorPools = {};
		std::vector<UniformBufferPool> puniformPools = {};
		std::vector<FrameDescriptorPool> wdescriptorPools = {};
		u32 currentFrame = 0;
		u32 imageIndex = 0;
		VkCommandBuffer activeCommandBuffer = {};
		LowRenderer::FrameBuffer* activeFrameBuffer = nullptr;
		RendererFrameBuffer activeRendererFrameBuffer = {};
		LowRenderer::RenderPassType activePassParams = LowRenderer::RenderPassType::DEFAULT;
		const RendererPipeline* activePipeline = nullptr;
		std::vector<Core::Scene::Components::Lights::DirectionalLightComponent*> dLights;
		std::vector<Core::Scene::Components::Lights::PointLightComponent*> pLights;
		std::vector<Core::Scene::Components::Lights::SpotLightComponent*> sLights;

		LowRenderer::FrameBuffer mainFB;
		LowRenderer::FrameBuffer swapBuffer;
		RendererFrameBuffer objectFrameBuffer;
		RendererFrameBuffer oldObjectBuffer;
		bool shouldDeleteOldFB = false;

		bool shouldRecreate = false;
		bool shouldRecreateFB = false;
		bool targetVSync = true;

		Maths::IVec2 targetResolution;
		Maths::Vec2 lineRange;
		f32 currentWidth = 1.0f;
		u8 stencilCompareValue = 0;
		StencilState state = StencilState::DEFAULT;

		Uniform::RendererMainUniform mainUniform;
		Uniform::RendererLightUniform lightUniform;
		Uniform::RendererPostUniform postUniform;
		Uniform::RendererWindowUniform windowUniform;

		void InitVulkan(GLFWwindow* window, Maths::IVec2 defaultResolution, bool defaultVSync, bool pInitEditor = false);
		void CreateMainFrameBuffer();
		void SetupDebugMessenger();
		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void CreateInstance();
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void CreateSwapChain(VkExtent2D resolution, bool defaultVSync);
		void CreateImageViews();
		bool CreateGraphicsPipeline(VkRenderPass& targetPass, const VertexRendererShader* vertex, const FragmentRendererShader* fragment, RendererPipeline& pipeline, PipelineParams params = (PipelineParams)0);
		void CreateRenderPass(VkRenderPass& targetPass, VkImageLayout finalLayout, VkFormat format, u32 attachemntCount, bool hasDepth = true, bool hasStencil = false, bool clearBuffers = true);
		void CreateSCFramebuffers();
		void CreateCommandPool(VkCommandPool* cmdPool, u32 familyIndex);
		void CreateCommandBuffers();
		void BeginCommandBuffer();
		void BeginRenderPass(VkRenderPass& targetPass, LowRenderer::FrameBuffer* frameBuffer);	
		void DrawIndexedMesh(const Resources::Mesh* mesh, VkDescriptorSet& meshDescriptor, UniformElement& uniform);
		void EndRenderPass();
		void EndCommandBuffer();
		void CreateSyncObjects();
		void RecreateSwapChain(Wrappers::Interfacing* pInterface, VkExtent2D newRes, bool useVSync);
		void CleanupSwapChain();
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer, const VkCommandPool& cmdPool, VkQueue& queue);
		void UpdateUniformBuffer(UniformElement& element, const Resources::Material* mat, const Maths::Mat4& modelMatrix, const Maths::Mat4& mvp);
		void UpdateUniformBuffer(UniformElement& element, const Maths::Mat4& mvp, u32 objectIndex);
		void UpdateLightUniformBuffer(UniformElement& element);
		void UpdatePostUniformBuffer(UniformElement& element, const LowRenderer::PostProcess::PostProcessEffect* effect);
		void UpdateDescriptorSet(VkDescriptorSet& descriptor, VkBuffer& uniformBuff, Renderer::Uniform::RendererUniformObject* uniform, bool hasVertexInfo, const std::vector<const RendererTexture*> textures, const Resources::TextureSampler* sampler);
		void UpdateDescriptorSet(VkDescriptorSet& descriptor, const RendererTexture* tex);
		void UpdateLightDescriptorSet(VkDescriptorSet& descriptor, VkBuffer& uniformBuff, const RendererTexture& albedo, const RendererTexture& normal, const RendererTexture& position);

		void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		VkPhysicalDevice& GetPhysicalDevice() { return physicalDevice; }
		void CreateImage(const Maths::IVec2& resolution, u32 mipLevel, VkImage& image, VkDeviceMemory& memory, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCubeMap = false);
		void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevel, bool cubeMap = false);
		void CopyBufferToImage(VkBuffer buffer, VkImage image, u32 width, u32 height, bool cubeMap = false);
		void CopyImageToBuffer(VkBuffer buffer, VkImage image, Maths::IVec2 offset, Maths::IVec2 size);
		void GenerateMipmaps(VkImage image, VkFormat imageFormat, Maths::IVec2 resolution, u32 mipLevels, bool cubeMap = false);
		VkImageView CreateImageView(const VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevel, bool cubeMap = false);
		VkFormat FindDepthFormat();
		void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
		void FlushMappedMemory(const VkDeviceMemory& mem, u64 offset, u64 size);
		RendererImageView GetValidImage(const RendererTexture* tex);

		bool CreateDescriptorPool(VkDescriptorPool& targetPool, u32 size = 1024, u32 uniformBuf = 2, u32 images = 3);
		bool CreateDescriptorSet(VkDescriptorPool& targetPool, VkDescriptorSet& descriptor, Resources::ShaderVariant targetShader = Resources::ShaderVariant::Default);

		void SubmitCurrentCommandBuffer(void* buffer);

		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		bool HasStencilComponent(VkFormat format);

		u32 RateDeviceSuitability(VkPhysicalDevice device);
		u32 GetGraphicsQueueIndex() const { return queueFamilyIndices.graphicsFamily.value(); }
		u32 GetTransferQueueIndex() const { return queueFamilyIndices.transferFamily.value(); }
		u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties);
		const VkPhysicalDevice& GetPhysicalDevice() const { return physicalDevice; }
		std::vector<const char*> GetRequiredExtensions();
		VkCommandBuffer BeginSingleTimeCommands(const VkCommandPool& cmdPool);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, bool vsync);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D resolution);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		friend UniformBufferPool;
		friend UniformElement;
		friend FrameDescriptorPool;
		friend RendererDepthBuffer;
		friend RendererBuffer;
		friend RendererTextureSampler;
		friend RendererFrameBuffer;
		friend RendererShaderProgram;
		friend Wrappers::Interfacing;
	};
}