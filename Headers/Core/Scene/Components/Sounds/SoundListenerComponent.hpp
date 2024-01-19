#pragma once
#include "Core/Scene/Components/IComponent.hpp"

namespace Core::Scene::Components::Sounds
{
	class NAT_API SoundListenerComponent : public IComponent
	{
	public:
		SoundListenerComponent() = default;
		~SoundListenerComponent() = default;

		IComponent* CreateCopy() override;
		void RenderGui() override;
		ComponentType GetType() override;
		const char* GetName() override;
		void Serialize(Core::Serialization::Serializer& sr) const;
		void Deserialize(Core::Serialization::Deserializer& dr);
		void Update() override;

	private:
		Maths::Vec3 cachedPos;
	};

}