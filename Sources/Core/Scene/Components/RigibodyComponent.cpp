#include "Core/App.hpp"

#include "Core\Scene\Components\RigibodyComponent.hpp"

namespace Core::Scene::Components
{
    void RigibodyComponent::Init()
    {
        if (mIsInitialised)
            return;
        IPhysicalComponent::Init();
        Core::App::GetInstance()->GetPhysicsEngine().AddRigibody(this);

        mIsInitialised = true;
    }

    IComponent* RigibodyComponent::CreateCopy()
    {
        RigibodyComponent* copy = new RigibodyComponent();

        copy->mMass = mMass;

        //copy->Init();

        return copy;
    }

    void RigibodyComponent::RenderGui()
    {

    }

    ComponentType RigibodyComponent::GetType()
    {
        return ComponentType::Rigibody;
    }

    const char* Core::Scene::Components::RigibodyComponent::GetName()
    {
        return "Rigibody";
    }

    void RigibodyComponent::Serialize(Core::Serialization::Serializer& sr) const
    {
        IPhysicalComponent::Serialize(sr);
        sr.Write(mMass);
    }
    
    void RigibodyComponent::Deserialize(Core::Serialization::Deserializer& dr)
    {
        IPhysicalComponent::Deserialize(dr);
        dr.Read(mMass);
    }

    void RigibodyComponent::SetMass(const float& pMass)
    {
        mMass = pMass;
        //TODO : Jolt : SetMass
    }

    float RigibodyComponent::GetMass() const
    {
        return mMass;
    }
}
