#include "Core/App.hpp"

#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"

#include "Core/Scene/Components/IPhysicalComponent.hpp"

namespace Core::Scene::Components
{
	Wrappers::PhysicsEngine::JoltPhysicsEngine* IPhysicalComponent::physicEngine = nullptr;
	
	constexpr std::array<const char*, 3> cColliderTypeText = { "STATIC", "KINEMATIC", "DYNAMIC" };
	constexpr std::array<const char*, 8> layerTypeText = { "GROUND", "WALL", "CEILING", "OBJECT", "PORTAL", "TRIGGER", "BUTTON","ALLBUTPLAYER" };

	void IPhysicalComponent::Init()
	{
		if (mIsInitialised)
			return;

		IComponent::Init();

		if (!physicEngine) physicEngine = &Core::App::GetInstance()->GetPhysicsEngine();
	}

	void IPhysicalComponent::RenderGui()
	{
		//ColliderType dropdown
		if (interfaceGui->BeginCombo("Collider Type", cColliderTypeText[this->colliderType], 0))
		{
			for (int i = 0; i < cColliderTypeText.size(); i++)
			{
				if (interfaceGui->Selectable(cColliderTypeText[i]))
					SetMotionType((ColliderType)i);
			}
			interfaceGui->EndCombo();
		}
		//LayerType dropdown
		if (interfaceGui->CollapsingHeader("Collision Layers"))
		{
			for (int i = 0; i < layerTypeText.size(); i++)
			{
				interfaceGui->SetCursorX(interfaceGui->GetCursorX() + 15);
				u16 id = 1 << i;
				bool selected = layerType & id;
				if (interfaceGui->CheckBox(layerTypeText[i], &selected))
				{
					SetLayerType(static_cast<LayerType>((layerType & ~id) | (selected ? id : 0)));
				}
			}
		}
	}

	void IPhysicalComponent::SetMotionType(const ColliderType& pType)
	{
		this->colliderType = pType;
		physicEngine->SetMotionType(this, pType);
	}

	void IPhysicalComponent::SetLayerType(const LayerType& pType)
	{
		layerType = static_cast<LayerType>(pType & ((1 << layerTypeText.size()) - 1));
		physicEngine->SetLayerType(this, layerType);
	}

	void IPhysicalComponent::Delete()
	{
		if (physicEngine) physicEngine->RemovedBodyFromSimulation(this);
	}

	void IPhysicalComponent::Serialize(Core::Serialization::Serializer& sr) const
	{
		sr.Write(colliderType);
		sr.Write(isTrigger);
		sr.Write((u8)0);
		sr.Write(layerType);
	}

	void IPhysicalComponent::Deserialize(Core::Serialization::Deserializer& dr)
	{
		u32 value = 0;
		dr.Read(value);
		colliderType = (ColliderType)value;
		dr.Read(isTrigger);
		u8 dataFiller;
		dr.Read(dataFiller);
		u16 type = 0;
		dr.Read(type);
		layerType = (LayerType)type;
	}


	Maths::Vec3 IPhysicalComponent::GetVelocity() const
	{
		return physicEngine->GetBodyVelocity(this);
	}

	Maths::Vec3 IPhysicalComponent::GetAngularVelocity() const
	{
		return physicEngine->GetBodyAngularVelocity(this);
	}

	void IPhysicalComponent::SetVelocity(Maths::Vec3 pNewVel)
	{
		physicEngine->SetBodyVelocity(this, pNewVel);
	}

	void IPhysicalComponent::SetAngularVelocity(Maths::Vec3 pNewVel)
	{
		physicEngine->SetBodyAngularVelocity(this, pNewVel);
	}
}
