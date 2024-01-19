#pragma once

#include "Maths/Maths.hpp"
#include "LowRenderer/RenderPassType.hpp"
#include "Core/Serialization/Serializer.hpp"
#include "Core/Serialization/Deserializer.hpp"

#ifdef _WIN32
#pragma warning(disable : 26812)
#endif

#ifdef NAT_EngineDLL
#define NAT_API __declspec(dllexport)
#else
#define NAT_API __declspec(dllimport)
#endif // NAT_EngineDLL

namespace Core::Scene
{
	class GameObject;
};

namespace Wrappers
{
	class Interfacing;
}

namespace Core::Scene::Components
{
	namespace Colliders
	{
		class ICollider;
	}

	enum class ComponentType : u8
	{
		RenderModel = 0,
		SkinnedRenderModel,
		PointLight,
		SpotLight,
		DirectionalLight,
		BillBoard,
		CameraComponent,
		PortalComponent,
		MirrorComponent,
		CubeCollider,
		SphereCollider,
		CapsuleCollider,
		MeshCollider,
		Rigibody,
		SkyBox,
		SoundPlayerComponent,
		SoundListenerComponent,
		PlayerManager,
		Laser,
		Bumper,
		DoubleJump,
		Button,
		PortalGun,
		ButtonDoor,
		PortalObject,
		ButtonPortal,
		LoadSceneComponent,
		All
	};

	class NAT_API IComponent
	{
	public:
		IComponent();
		virtual ~IComponent() = default;

		virtual void Init();
		virtual void Delete();
		virtual void Update();
		virtual void DataUpdate();
		virtual void PhysicsUpdate();
		virtual void Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass);

		virtual IComponent* CreateCopy() = 0;
		virtual void RenderGui() = 0;
		virtual ComponentType GetType() = 0;
		virtual const char* GetName() = 0;
		IComponent* GetComponent(ComponentType type);

		virtual void Serialize(Core::Serialization::Serializer& sr) const = 0;
		virtual void Deserialize(Core::Serialization::Deserializer& dr) = 0;

		static IComponent* CreateComponent(ComponentType type);
		
		virtual void OnStateChanged(const bool& pNewState);

		virtual void OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal);
		virtual void OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal);
		virtual void OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider);

		virtual void OnPositionUpdate(const Maths::Vec3& newPos);
		virtual void OnRotationUpdate(const Maths::Quat& newRot);
		virtual void OnScaleUpdate(const Maths::Vec3& newScale);

	public :
		GameObject* gameObject = nullptr;

	protected:
		static Wrappers::Interfacing* interfaceGui;
		bool mIsInitialised = false;
	};
}