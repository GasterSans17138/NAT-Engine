#pragma once

#include "IPhysicalComponent.hpp"

#include "Maths/Maths.hpp"

namespace Core::Scene::Components
{
    class NAT_API RigibodyComponent : public IPhysicalComponent
    {
    public :

         RigibodyComponent() = default;
        ~RigibodyComponent() = default;

        virtual void Init() override;

        virtual IComponent*      CreateCopy() override;
        virtual void             RenderGui()  override;
        virtual ComponentType    GetType()    override;
        virtual const char*      GetName()    override;

        virtual void Serialize(Core::Serialization::Serializer& sr) const override;
        virtual void Deserialize(Core::Serialization::Deserializer& dr) override;
   
        void  SetMass(const float& pMass);
        float GetMass() const;

    private :
        float mMass = 1.0f;
    };
}