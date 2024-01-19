#include "Core/Scene/Components/IComponent.hpp"

#include "Core/Scene/Components/RenderModelComponent.hpp"
#include "Core/Scene/Components/Lights/DirectionalLightComponent.hpp"
#include "Core/Scene/Components/Lights/PointLightComponent.hpp"
#include "Core/Scene/Components/Lights/SpotLightComponent.hpp"
#include "Core/Scene/Components/Rendering/CameraComponent.hpp"
#include "Core/Scene/Components/Rendering/PortalComponent.hpp"
#include "Core/Scene/Components/Rendering/MirrorComponent.hpp"
#include "Core/Scene/Components/SkyBoxComponent.hpp"
#include "Core/Scene/Components/Sounds/SoundPlayerComponent.hpp"
#include "Core/Scene/Components/Sounds/SoundListenerComponent.hpp"
#include "../Game/Header/PlayerManager.hpp"
#include "../Game/Header/Component/GameFeatures/Laser.hpp"
#include "../Game/Header/Component/GameFeatures/Bumper.hpp"
#include "../Game/Header/Component/GameFeatures/DoubleJump.hpp"
#include "../Game/Header/Component/GameFeatures/Button.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonDoor.hpp"
#include "../Game/Header/Component/GameFeatures/ButtonPortal.hpp"
#include "Core/App.hpp"

namespace Core::Scene::Components
{ 
    Wrappers::Interfacing* IComponent::interfaceGui = nullptr;

    IComponent::IComponent()
    {
        interfaceGui = &Core::App::GetInstance()->GetInterfacing();
    }

    void IComponent::Init()
    {
        if (this->mIsInitialised)
            return;
    }

    void IComponent::Delete()
    {
    }

    void IComponent::Update()
    {
    }

    void IComponent::DataUpdate()
    {
    }

    void IComponent::PhysicsUpdate()
    {
    }

    void IComponent::Render(const Maths::Mat4& mvp, const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& cameraFrustum, LowRenderer::RenderPassType pass)
    {
    }

    IComponent* IComponent::CreateComponent(ComponentType type)
    {
        switch (type)
        {
        case ComponentType::RenderModel:
            return new RenderModelComponent();
        case ComponentType::SkinnedRenderModel:
            return nullptr; // TODO
        case ComponentType::PointLight:
            return new Lights::PointLightComponent();
        case ComponentType::SpotLight:
            return new Lights::SpotLightComponent();
        case ComponentType::DirectionalLight:
            return new Lights::DirectionalLightComponent();
        case ComponentType::BillBoard:
            return nullptr; // TODO
        case ComponentType::CameraComponent:
            return new Rendering::CameraComponent();
        case ComponentType::PortalComponent:
            return new Rendering::PortalComponent();
        case ComponentType::MirrorComponent:
            return new Rendering::MirrorComponent();
        case ComponentType::CubeCollider:
            return new Colliders::CubeCollider();
        case ComponentType::SphereCollider:
            return new Colliders::SphereCollider();
        case ComponentType::CapsuleCollider:
            return new Colliders::CapsuleCollider();
        case ComponentType::MeshCollider:
            return new Colliders::MeshCollider();
        case ComponentType::Rigibody:
            return nullptr; // TODO
        case ComponentType::SkyBox:
            return new SkyBoxComponent();
        case ComponentType::SoundPlayerComponent:
            return new Sounds::SoundPlayerComponent();
        case ComponentType::SoundListenerComponent:
            return new Sounds::SoundListenerComponent();
        case ComponentType::PlayerManager:
            return new Game::PlayerManager();
        case ComponentType::Laser:
            return new Game::Laser();
        case ComponentType::Bumper:
            return new Game::Bumper();
        case ComponentType::DoubleJump:
            return new Game::DoubleJump();
        case ComponentType::Button:
            return new Game::Button();
        case ComponentType::PortalGun:
            return new Game::PortalGun();
        case ComponentType::ButtonDoor:
            return new Game::ButtonDoor();
        case ComponentType::PortalObject:
            return new Game::PortalObject();
        case ComponentType::ButtonPortal:
            return new Game::ButtonPortal();
        case ComponentType::LoadSceneComponent:
            return new Game::LoadSceneComponent();
        default:
            LOG(DEBUG_LEVEL::LERROR, "Unimplemented component type %d : are you a time traveller ?", static_cast<u32>(type));
            return nullptr;
        }
    }

    void IComponent::OnStateChanged(const bool& pNewState)
    {
    }

    void IComponent::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
    {

    }

    void IComponent::OnCollisionStay(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
    {

    }

    void IComponent::OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider)
    {

    }

    void IComponent::OnPositionUpdate(const Maths::Vec3& newPos)
    {

    }

    void IComponent::OnRotationUpdate(const Maths::Quat& newRot)
    {

    }

    void IComponent::OnScaleUpdate(const Maths::Vec3& newScale)
    {

    }

    IComponent* IComponent::GetComponent(ComponentType type)
    {
        for (auto& component : gameObject->components)
        {
            if (type == component->GetType())
                return component;
        }
        return nullptr;
    }
}