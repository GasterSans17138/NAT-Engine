#include "Core/Scene/SceneManager.hpp"

#include "Core/Scene/GameObject.hpp"

namespace Core::Scene
{
	GameObject::GameObject(GameObject* pParent) : parent(pParent), transform(this)
	{

	}

	GameObject::GameObject(const GameObject& other)
	{
		for (auto& component : other.components)
		{
			components.push_back(component);
			components.back()->gameObject = this;
		}
		for (auto& child : other.childs)
			childs.push_back(child);
		name = other.name;
		transform = other.transform;
		transform.gameObject = this;
		isNodeOpen = other.isNodeOpen;
	}

	void Core::Scene::GameObject::CopyTo(GameObject* dest, GameObject* parent)
	{
		dest->parent = parent;
		for (auto& component : components)
		{
			dest->components.push_back(component->CreateCopy());
			dest->components.back()->gameObject = dest;
		}
		for (auto& child : childs)
		{
			dest->childs.push_back(new GameObject());
			child->CopyTo(dest->childs.back(), dest);
		}
		dest->name = name;
		dest->transform = transform;
		dest->transform.gameObject = dest;
		dest->isNodeOpen = isNodeOpen;
	}

	GameObject::~GameObject()
	{
		//TODO : Remove reference from parent ?
	}

	void GameObject::Clean()
	{
		for (auto& component : components)
		{
			component->Delete();
			delete component;
		}
		components.clear();
		for (auto& child : childs)
		{
			child->Clean();
			delete child;
		}
		childs.clear();
	}

	void GameObject::Init()
	{
		transform.Update();

		for (auto& component : components)
			component->Init();
		
		for (auto& child : childs)
			child->Init();
	}

	void GameObject::Delete()
	{
		auto& childs = parent ? parent->childs : SceneManager::GetInstance()->GetGameObjectScene(this)->gameObjects;
		for (u64 i = 0; i < childs.size(); i++)
		{
			if (childs[i] == this)
			{
				Clean();
				for (u64 j = i; j < childs.size() - 1; j++)
				{
					childs[j] = childs[j + 1];
				}
				childs.pop_back();
				delete this;
				return;
			}
		}
		LOG(DEBUG_LEVEL::LWARNING, "Could not delete GameObject %s; not found !", name.c_str());
		return;
	}

	void GameObject::Update()
	{
		if (!mIsActive)
			return;

		transform.Update();

		for (u64 i = 0; i < components.size(); ++i)
			components[i]->Update();

		for (u64 i = 0; i < childs.size(); ++i)
		{
			if (!childs[i]->parent) childs[i]->parent = this;
			childs[i]->Update();
		}
	}

	void GameObject::DataUpdate(bool updateTransform)
	{
		if (updateTransform) transform.Update();

		for (u64 i = 0; i < components.size(); ++i)
			components[i]->DataUpdate();

		for (u64 i = 0; i < childs.size(); ++i)
		{
			if (!childs[i]->parent) childs[i]->parent = this;
			childs[i]->DataUpdate(updateTransform);
		}
	}

	void GameObject::Render(const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& frustum, LowRenderer::RenderPassType pass)
	{
		if (!mIsActive)
			return;
		Maths::Mat4 mvp = vp * transform.global;
		for (auto& component : components)
			component->Render(mvp, vp, modelOverride, frustum, pass);

		for (GameObject* child : childs)
			child->Render(vp, modelOverride, frustum, pass);
	}

	void GameObject::OnCollisionBegin(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
	{
		for (Components::IComponent* component : components)
			component->OnCollisionBegin(pCollider, pOther, pOtherCollider, normal);
	}

	void GameObject::OnCollisionStay(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
	{
		for (Components::IComponent* component : components)
			component->OnCollisionStay(pCollider, pOther, pOtherCollider, normal);
	}

	void GameObject::OnCollisionEnd(Components::Colliders::ICollider* pCollider, GameObject* pOther, Components::Colliders::ICollider* pOtherCollider)
	{
		for (Components::IComponent* component : components)
			component->OnCollisionEnd(pCollider, pOther, pOtherCollider);
	}

	GameObject* GameObject::Find(std::string pName) const
	{
		SceneManager* manager = SceneManager::GetInstance();

		for (Scene* scene : manager->activeScenes)
		{
			for (GameObject* object : scene->gameObjects)
			{
				if (object->name == pName)
					return object;
			}
		}
		return nullptr;
	}

	void GameObject::SetActive(const bool& pState)
	{
		mIsActive = pState;

		for (Components::IComponent* component : components)
			component->OnStateChanged(pState);

		for (GameObject* gameObject : childs)
			gameObject->SetActive(pState);
	}

	bool GameObject::IsActive() const
	{
		return mIsActive;
	}

	GameObject* GameObject::GetParent()
	{
		return this->parent;
	}

	GameObject* GameObject::AddChild()
	{
		childs.push_back(new GameObject(this));
		childs.back()->name = "newGameObject";
		return childs.back();
	}

	GameObject* GameObject::AddChild(GameObject* pChildObject)
	{
		pChildObject->parent = this;
		this->childs.push_back(pChildObject);
		return childs.back();
	}

	GameObject& GameObject::operator=(const GameObject& other)
	{
		childs.clear();
		components.clear();
		for (auto& component : other.components)
		{
			components.push_back(component);
			components.back()->gameObject = this;
		}
		for (auto& child : other.childs)
			childs.push_back(child);
		name = other.name;
		transform = other.transform;
		transform.gameObject = this;
		isNodeOpen = other.isNodeOpen;
		return *this;
	}

	void GameObject::Serialize(Core::Serialization::Serializer& sr) const
	{
		sr.Write(name);
		sr.Write(static_cast<u8>((u8)isNodeOpen | ((u8)mIsActive << 1)));
		sr.Write(transform.position);
		sr.Write(transform.rotation);
		sr.Write(transform.scale);
		sr.Write(components.size());
		{
			for (auto& component : components)
			{
				sr.Write(static_cast<u8>(component->GetType()));
				component->Serialize(sr);
			}
		}
		sr.Write(childs.size());
		{
			for (auto& child : childs)
			{
				child->Serialize(sr);
			}
		}
	}

	void GameObject::Deserialize(Core::Serialization::Deserializer& dr)
	{
		dr.Read(name);
		u8 res = 0;
		dr.Read(res);
		isNodeOpen = res & 0x1;
		mIsActive = res & 0x2;
		dr.Read(transform.position);
		dr.Read(transform.rotation);
		dr.Read(transform.scale);
		u64 size = 0;
		dr.Read(size);
		for (u64 i = 0; i < size; i++)
		{
			u8 type;
			dr.Read(type);
			if (type > static_cast<u8>(Components::ComponentType::All))
			{
				LOG(DEBUG_LEVEL::LERROR, "Component of type id %d is invalid !", type);
				return;
			}
			auto comp = Components::IComponent::CreateComponent(static_cast<Components::ComponentType>(type));
			if (!comp)
			{
				LOG(DEBUG_LEVEL::LERROR, "Cannot create component of type id %d !", type);
				return;
			}
			comp->gameObject = this;
			comp->Deserialize(dr);
			components.push_back(comp);
		}
		dr.Read(size);
		for (u64 i = 0; i < size; i++)
		{
			childs.push_back(new GameObject(this));
			childs.back()->Deserialize(dr);
		}
	}

	void Core::Scene::GameObject::ForceUpdate()
	{
		transform.Update();
		for (auto& comp : components)
		{
			comp->OnPositionUpdate(transform.GetGlobal().GetPositionFromTranslation());
			comp->OnRotationUpdate(transform.GetGlobal());
			comp->OnScaleUpdate(transform.GetGlobal().GetScaleFromTranslation());
		}
		for (auto& child : childs)
		{
			child->ForceUpdate();
		}
	}

	Components::IComponent* GameObject::GetComponent(Components::ComponentType type)
	{
		for (auto& component : components)
		{
			if (type == component->GetType())
				return component;
		}
		return nullptr;
	}

}