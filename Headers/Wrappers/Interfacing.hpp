#pragma once
#include <ImGUI/imgui.h>
#include <bitset>
#include <iostream>
#include <filesystem>
#include <vector>

#include "Maths/Maths.hpp"
#include "Core/Debugging/Log.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Renderer/RendererFrameBuffer.hpp"
#include "Resources/Texture.hpp"
#include "Resources/Sound.hpp"
#include "Resources/SoundData.hpp"
#include "Resources/StaticTexture.hpp"
#include "WindowManager.hpp"

#include "Core/Scene/Components/RenderModelComponent.hpp"
#include "Core/Scene/Components/Lights/DirectionalLightComponent.hpp"
#include "Core/Scene/Components/Lights/PointLightComponent.hpp"
#include "Core/Scene/Components/Lights/SpotLightComponent.hpp"
#include "Core/Scene/Components/Rendering/CameraComponent.hpp"
#include "Core/Scene/Components/Rendering/PortalComponent.hpp"
#include "Core/Scene/Components/Rendering/MirrorComponent.hpp"
#include "Core/Scene/Components/Colliders/CapsuleCollider.hpp"
#include "Core/Scene/Components/Colliders/CubeCollider.hpp"
#include "Core/Scene/Components/Colliders/MeshCollider.hpp"
#include "Core/Scene/Components/Colliders/SphereCollider.hpp"
#include "Core/Scene/Components/SkyBoxComponent.hpp"
#include "Core/Scene/Components/Sounds/SoundListenerComponent.hpp"
#include "Core/Scene/Components/Sounds/SoundPlayerComponent.hpp"
#include "../Game/Header/Component/GameFeatures/Bumper.hpp"
#include "../Game/Header/Component/GameFeatures/Button.hpp"
#include "../Game/Header/Component/GameFeatures/DoubleJump.hpp"
#include "../Game/Header/Component/GameFeatures/Laser.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonDoor.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonPortal.hpp"
#include "../Game/Header/Component/LoadSceneComponent.hpp"
#include "../Game/Header/Component/GameFeatures/PortalObject.hpp"
#include "../Game/Header/Component/GameFeatures/PortalGun.hpp"
#include "../Game/Header/PlayerManager.hpp"

#define IMGUI_DRAG_DROP_FRAGMENT_SHADER_PAYLOAD	"FragmentShaderPayload"
#define IMGUI_DRAG_DROP_TEXTURE_SAMPLER_PAYLOAD	"TextureSamplerPayload"
#define IMGUI_DRAG_DROP_GEOMETRY_SHADER_PAYLOAD	"GeometryPayload"
#define IMGUI_DRAG_DROP_SHADER_PROGRAM_PAYLOAD	"ShaderProgramPayload"
#define IMGUI_DRAG_DROP_VERTEX_SHADER_PAYLOAD	"VertexShaderPayload"
#define IMGUI_DRAG_DROP_SKINNED_MODEL_PAYLOAD	"SkinnedModelPayload"
#define IMGUI_DRAG_DROP_FRAME_BUFFER_PAYLOAD	"FrameBufferPayload"
#define IMGUI_DRAG_DROP_DEPTH_BUFFER_PAYLOAD	"DepthBufferPayload"
#define IMGUI_DRAG_DROP_MATERIAL_PAYLOAD		"MaterialPayload"
#define IMGUI_DRAG_DROP_TEXTURE_PAYLOAD			"TexturePayload"
#define IMGUI_DRAG_DROP_CUBE_MAP_PAYLOAD		"CubeMapPayload"
#define IMGUI_DRAG_DROP_MODEL_PAYLOAD			"ModelPayload"
#define IMGUI_DRAG_DROP_SCENE_PAYLOAD			"ScenePayload"
#define IMGUI_DRAG_DROP_MESH_PAYLOAD			"MeshPayload"
#define IMGUI_DRAG_DROP_SOUND_PAYLOAD			"SoundPayload"
#define IMGUI_DRAG_DROP_SOUND_DATA_PAYLOAD		"SoundDataPayload"

namespace Resources
{
	enum class ObjectType : u8;
}

namespace Core::Scene::Components
{
	class IComponent;
}

namespace Wrappers
{
	class NAT_API Interfacing
	{
	public:
		Interfacing()  = default;
		~Interfacing() = default;

		void InitImgui(GLFWwindow* window);

		void UpdateFps(f32 dt, u64 gTime);

		void Destroy();

		void BeginFrame();
		void EndFrame();

		void Render();

		bool BeginClose(const char* name, bool& closing, int flags = 0);
		bool Begin(const char* name);
		void End();
		void PushBGColor(Maths::Vec3 color);
		void PopBGColor();

		void Bullet();
		void BulletText(const char* text);
		void Dummy(Maths::Vec2 size);
		bool CheckBox(const char* name, bool var);
		bool CheckBox(const char* name, bool* var);
		bool ColorEdit3(const char* name, Maths::Vec3& color);
		bool ColorEdit4(const char* name, Maths::Vec4& color);
		bool ColorPicker4(const char* name, Maths::Vec4& color);
		void Column(u16 count, const char* name, bool border);
		void NextColumn();
		bool CollapsingHeader(const char* name);
		bool CollapsingHeader(const char* name, bool* closed, int flags = 0);
		bool Combo(const char* label, int* current_item, const char* const items[], int items_count);
		bool ShaderCombo(Resources::ShaderVariant filter, Resources::ShaderProgram** pOutSelectedShader);
		bool ShaderCombo(const std::vector<Resources::ShaderVariant>& pFilters, Resources::ShaderProgram** pOutSelectedShader);
		bool BeginCombo(const char* pLabel, const char* pPreviewValue, const  ImGuiComboFlags& pFlags);
		void EndCombo();
		void Image(Resources::Texture* texture, Maths::Vec2 size);
		void Image(Resources::CubeMap* texture, Maths::Vec2 size);
		void Image(Renderer::RendererTexture texture, Maths::Vec2 size);
		void SameLine(float start = (0.0F), float spacing = (0.0F));
		void Text(const char* text);
		void PushId(int Id);
		void PopId();
		void SetCursor(Maths::Vec2 localPos);
		void SetCursorX(float x);
		void SetCursorY(float y);
		Maths::Vec2 GetCursor();
		float GetCursorX();
		float GetCursorY();
		bool InputText(const char* name, std::string& text);

		VkDescriptorSet CreateImageRef(const VkImageView& imageView, const VkSampler& sampler);
		void DeleteImageRef(VkDescriptorSet& set);
		Maths::Vec2 GetWindowSize();
		Maths::Vec2 GetWindowPos();
		Maths::Vec2 GetWindowPadding();
		Maths::Vec2 GetMouseDragDeta();
		void ResetMouseDragDelta();
		void SetWindowFocused();
		bool IsClicked();
		bool IsHovered();
		bool IsActive();

		//DragFloat
		bool DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
		bool DragFloat2(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
		bool DragFloat3(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
		bool DragFloat4(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
		bool DragFloatRange2(const char* label, float* v, float* v2, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f);
		bool SliderAngle(const char* label, float* v_rad);
		bool SliderFloat(const char* label, float* v, float v_min, float v_max, bool logaritmique);
		bool SliderFloat2(const char* label, float* v, float v_min, float v_max);
		bool SliderFloat3(const char* label, float* v, float v_min, float v_max);
		bool SliderFloat4(const char* label, float* v, float v_min, float v_max);
		bool DragQuat(const char* label, Maths::Quat& quat, float v_speed = 1.0f);
		void PlotHistogram(const char* name, const f32* data, s32 dataCount, const char* overlayText, f32 minScale = FLT_MAX, f32 maxScale = FLT_MAX, Maths::Vec2 size = Maths::Vec2());

		//DragInt
		bool DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0);
		bool DragInt2(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0);
		bool SliderInt(const char* label, int* v, int v_min, int v_max);

		//Button
		bool ArrowButton(const char* name, int Dir);//None = -1, Left = 0, Right = 1, Up = 2, Down = 3
		bool Button(const char* name, Maths::Vec2 size = Maths::Vec2());
		bool ColorButton(const char* name, Maths::Vec4 color, Maths::Vec2 size = Maths::Vec2());
		bool ImageButton(const char* name, VkDescriptorSet user_texture_id, Maths::Vec2 size);
		bool InvisibleButton(const char* name, Maths::Vec2 size);
		void LogButtons();
		bool Selectable(const char* name);
		bool Selectable(const char* name, bool* selectable);

		//TabItem
		bool BeginTabBar(const char* name);
		bool BeginTabItem(const char* label, bool* p_open = NULL, bool withflag = false);
		void EndTabBar();
		void EndTabItem();
		void SetTabItemClosed(const char* name);
		bool TabItemButton(const char* name);

		//Menu
		bool BeginMainMenuBar();
		bool BeginMenuBar();
		bool BeginMenu(const char* name);
		bool MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);
		void EndMenu();
		void EndMainMenuBar();
		void EndMenuBar();

		// Popup
		void OpenPopup(const char* name);
		bool BeginPopupModal(const char* name);
		bool BeginPopupContextItem();
		bool IsItemHovered();
		void SetTooltip(const char* text);
		void EndPopup();
		void CloseCurrentPopup();
		bool IsEscPressed();

		//Log
		void AddLog(const char* fmt);
		void SetLevel(DEBUG_LEVEL level);
		void Clear();

		//FileExplorer
		void ShowFileExplorer();
		void ShowMaterialPopup();
		void ShowTexturePopup();
		void ShowCubeMapPopup();
		void ShowMeshPopup();
		void SelectMaterial(Resources::Material** material);
		void SelectMesh(Resources::Mesh** mesh);
		void SelectTexture(Resources::Texture** tex);
		void SelectCubeMap(Resources::CubeMap** cube);

		void* RecordImGuiCommandBuffer(u32 imageIndex);
		void RecreateImGuiSwapChain();

		void EngineInterface();
		void SetSelectedObject(Core::Scene::GameObject* object);

		//ResousourcesExplorer
		Resources::IResource* GetIResourcesSelected() { return IResourceClicked; }
		void GetShaderList(Resources::ShaderProgram*& shader, bool& open);
		void GetVertShaderList(Resources::VertexShader*& shader, bool& open);
		void GetFragShaderList(Resources::FragmentShader*& shader, bool& open);
		void GetGeoShaderList(Resources::GeometryShader*& shader, bool& open);
		void GetMaterialList(Resources::Material*& mat, bool& open);
		void GetMeshList(Resources::Mesh*& mesh, bool& open);
		void GetModelList(Resources::Model& model, bool& open);
		void GetModelListToRenderModelComponent(Core::Scene::Components::RenderModelComponent* model, bool& open);
		bool ModelListCombo(Resources::Model** pOutSelectedModel, const char* id = "###ModelListCombo");
		bool MaterialListCombo(Resources::Material** pOutSelectedMaterial, const char* id = "###MaterialListCombo");
		bool MeshListCombo(Resources::Mesh** pOutSelectedMesh, const char* id = "###MeshListCombo");
		bool SceneListCombo(Core::Scene::Scene** pOutSelectedScene, const char* id = "###SceneListCombo");
		bool SoundDataListCombo(Resources::SoundData** pOutSelectedSound);
		bool TextureListCombo(Resources::Texture** pOutSelectedTexture);
		bool VertexShaderCombo(Resources::VertexShader** pOutSelectedVertexShader);
		bool FragmentShaderCombo(Resources::FragmentShader** pOutSelectedFragmentShader);
		bool GeometryShaderCombo(Resources::GeometryShader** pOutSelectedGeometryShader);
		bool SoundListCombo(Resources::Sound** pOutSelectedSound, const char* id = "###SoundListCombo");
		void Separator();
		void NewLine();
		void ColoredText(const char* pText, Maths::Vec4 pColor);

		void SetNextWindowPosition(Maths::Vec2 pPosition);
		bool IsInitialised();

		Core::Scene::GameObject* GetSelectedGameObject() const;
		bool openModelList = false;

	private:
		bool isInitialised = false;

		std::string GetExtentionFile(std::string path);
		//Log
		void ShowLog();
		void ShowFps(std::array<f32, 128> frames, f32 average, u64 frameCounter);

		//FileExplorer
		void ReadFolder();
		void DrawFolder();
		void ClearList();
		void DrawIcon(Resources::Texture* icon, const std::string& text, bool open);

		//ResourceExplorer
		void ShowResourceExplorer();
		void ReadResources(Resources::ObjectType resourceId);
		void IconResource(Resources::Texture* icon, const std::string& text, bool open, Resources::ObjectType Id, Resources::IResource* resource, s32 pId);
		std::string OpenFile(const char* filter);
		std::string SaveFile(const char* filter);
		void LoadRessources(std::string path);
		void ListeCreateRessources();
		u64 idResource = 0;

		//SceneGraph
		void SceneGraph();

		void ButtonPlayPause();
		void GameObjectTreeNode(Core::Scene::GameObject* gameobject);
		void ShowComponent(Core::Scene::GameObject* gameobject);
		void ComponentList();
		void EditorPlayerStart();
		void EditorPlayerStop();
		void ShowPostProcessEditor();

		bool openLog = true;
		bool openFps = true;
		bool openFileExplorer = true;
		bool openInspector = true;
		bool openSceneGraph = true;
		bool openResourceExplorer = true;
		bool openPostProcessEditor = true;
		bool openComponentList = false;
		bool openPopupModifRessource = false;
		bool openFileExplorerSelectNewResource = false;
		bool openRenameWindow = false;
		bool openListeCreateResourceWindow = false;
		bool createResource = false;

		bool unifiedScale = false;
		Resources::IResource* currentEditResource = nullptr;
		Resources::Material* openEditMaterialWindow = nullptr;
		Resources::Model* tempModel = nullptr;

		VkDescriptorPool imguiPool = {};
		VkCommandPool imguiCommandPool = {};
		Renderer::VulkanRenderer* vulkanRenderer = nullptr;
		Core::App* appInstance;
		VkRenderPass imguiRenderPass = {};
		std::vector<Renderer::RendererFrameBuffer> imguiFramebuffers;
		std::vector<VkCommandBuffer> imguiCommandBuffers;
		std::vector<char> DebugLevelList;
		std::bitset<4> logBitMask;
		std::vector<Core::Serialization::Serializer> mEditorSavedScenes;
		Core::Scene::GameObject* sceneGraphSelectedObject = nullptr;
		Core::Scene::GameObject* clickedGameObject = nullptr;
		Resources::IResource* IResourceClicked = nullptr;
		Resources::Texture** selectedTexture = nullptr;
		Resources::CubeMap** selectedCubeMap = nullptr;
		Resources::ShaderProgram** shaderPopup = nullptr;
		Resources::Material** materialPopup = nullptr;
		Resources::Mesh** meshPopup = nullptr;
		
		std::array<std::unique_ptr<Core::Scene::Components::IComponent>, 24> baseComponents = {
			std::make_unique<Core::Scene::Components::RenderModelComponent>(),
			std::make_unique<Core::Scene::Components::Lights::DirectionalLightComponent>(),
			std::make_unique<Core::Scene::Components::Lights::PointLightComponent>(),
			std::make_unique<Core::Scene::Components::Lights::SpotLightComponent>(),
			std::make_unique<Core::Scene::Components::Rendering::CameraComponent>(),
			std::make_unique<Core::Scene::Components::Rendering::PortalComponent>(),
			std::make_unique<Core::Scene::Components::Rendering::MirrorComponent>(),
			std::make_unique<Core::Scene::Components::Colliders::CapsuleCollider>(),
			std::make_unique<Core::Scene::Components::Colliders::CubeCollider>(),
			std::make_unique<Core::Scene::Components::Colliders::SphereCollider>(),
			std::make_unique<Core::Scene::Components::Colliders::MeshCollider>(),
			std::make_unique<Core::Scene::Components::SkyBoxComponent>(),
			std::make_unique<Core::Scene::Components::Sounds::SoundPlayerComponent>(),
			std::make_unique<Core::Scene::Components::Sounds::SoundListenerComponent>(),
			std::make_unique<Core::Scene::Components::Game::Laser>(),
			std::make_unique<Core::Scene::Components::Game::Bumper>(),
			std::make_unique<Core::Scene::Components::Game::DoubleJump>(),
			std::make_unique<Core::Scene::Components::Game::PlayerManager>(),
			std::make_unique<Core::Scene::Components::Game::Button>(),
			std::make_unique<Core::Scene::Components::Game::ButtonDoor>(),
			std::make_unique<Core::Scene::Components::Game::ButtonPortal>(),
			std::make_unique<Core::Scene::Components::Game::LoadSceneComponent>(),
			std::make_unique<Core::Scene::Components::Game::PortalObject>(),
			std::make_unique<Core::Scene::Components::Game::PortalGun>(),
		};

		std::array<std::unique_ptr<Resources::IResource>, 5> baseResources = { 
			std::make_unique<Resources::Material>(),
			std::make_unique<Resources::TextureSampler>(),
			std::make_unique<Resources::ShaderProgram>(),
			std::make_unique<Resources::Sound>(),
			std::make_unique<Core::Scene::Scene>()
		};

		Maths::Vec4 colorButtonPlay = { 0,0,0,1 };
		Maths::Vec4 colorButtonPause = { 0,0,0,1 };

		Resources::StaticTexture* iconFolder = nullptr;
		Resources::StaticTexture* iconTxt = nullptr;
		Resources::StaticTexture* iconPng = nullptr;
		Resources::StaticTexture* iconObj_Fbx = nullptr;
		Resources::StaticTexture* iconJpg = nullptr;
		Resources::StaticTexture* iconCpp = nullptr;
		Resources::StaticTexture* iconH = nullptr;
		Resources::StaticTexture* iconUnknown = nullptr;
		Resources::StaticTexture* saveTexture = nullptr;

		int internalIDStack = 0;

		bool loadScene = false;

		void CreateImGuiRenderPass();
		void CreateImGuiCommandBuffers();
		void CreateImGuiFramebuffers();

	};
}