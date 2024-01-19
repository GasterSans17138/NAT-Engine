#include "../Game/Header/Component/GameFeatures/PortalObject.hpp"
#include "Core/App.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"
#include "../Game/Header/PlayerManager.hpp"


using namespace  Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace Core::Serialization;

PortalObject::PortalObject()
{
}

PortalObject::~PortalObject()
{
}

void PortalObject::Init()
{
	portalRenderer = reinterpret_cast<Rendering::PortalComponent*>(gameObject->GetComponent(ComponentType::PortalComponent));
}

void PortalObject::Update()
{
	if (!parentObj) return;
	gameObject->transform.SetPosition(Maths::Quat(parentObj->transform.GetGlobal()) * offset + parentObj->transform.GetGlobal().GetPositionFromTranslation());
	gameObject->transform.SetRotation(Maths::Quat(parentObj->transform.GetGlobal()) * rotation);
	Maths::Vec3 newPos = gameObject->transform.GetGlobal().GetPositionFromTranslation();
	velocity = newPos - lastPos;
	lastPos = newPos;
	if (!destination) return;
	const Maths::Mat4& dest = destination->gameObject->transform.GetGlobal();
	portalRenderer->targetPosition = dest.GetPositionFromTranslation();
	portalRenderer->targetRotation = Maths::Quat(dest) * Maths::Quat::AxisAngle(Maths::Vec3(0,1,0), (f32)M_PI);
	if (animationDelay > 0.0f)
	{
		bool half = (animationDelay >= animationTime / 2);
		animationDelay = Maths::Util::MaxF(animationDelay -  App::GetInstance()->GetWindow().GetDeltaTime(), 0.0f);
		portalRenderer->gameObject->transform.SetScale(Maths::Util::Clamp((animationTime - animationDelay) / animationTime, 0.05f, 1.0f));
		if (half && animationDelay < animationTime / 2)
		{
			for (auto& obj : nearObjects)
			{
				obj.collider->SetLayerType(static_cast<LayerType>(obj.collider->layerType & ~(isWall ? LayerType::WALL : (LayerType::GROUND | LayerType::CEILING))));
			}
		}
	}
	if (animationDelay < animationTime / 2)
	{
		Maths::Vec3 forward = (gameObject->transform.GetGlobal() * Maths::Vec4(0,0,1,0)).GetVector().UnitVector();
		for (u64 i = 0; i < nearObjects.size(); ++i)
		{
			auto& obj = nearObjects[i];
			if (obj.ignoreObject)
			{
				obj.ignoreObject--;
				continue;
			}
			Maths::Vec3 ref = (obj.player ? obj.player->head : obj.collider->gameObject)->transform.GetGlobal().GetPositionFromTranslation() - gameObject->transform.GetPosition();
			if (ref.DotProduct(forward) < 0.0f)
			{
				Maths::Vec3 inVel = obj.collider->GetVelocity() - velocity;
				Maths::Quat rot = Maths::Quat(gameObject->transform.GetGlobal()).Inverse() * Maths::Quat::AxisAngle(Maths::Vec3(0,1,0), (f32)M_PI) * Maths::Quat(dest);
				obj.collider->SetVelocity(rot * inVel + destination->velocity);
				if (obj.player)
				{
					Maths::Vec3 rotation = rot * Maths::Vec3(0, 0, 1);
					obj.player->rotationY += Maths::Util::ToDegrees(Maths::Vec2(rotation.z, rotation.x).GetAngle());
				}
				else
				{
					obj.collider->gameObject->transform.SetRotation(obj.collider->gameObject->transform.GetRotation() * rot);
				}
				obj.collider->gameObject->transform.SetPosition(dest.GetPositionFromTranslation() + rot * ref);
				if (isWall != destination->isWall)
				{
					obj.collider->SetLayerType(static_cast<LayerType>((obj.collider->layerType | LayerType::WALL | LayerType::GROUND | LayerType::CEILING) & ~(destination->isWall ? LayerType::WALL : (LayerType::GROUND | LayerType::CEILING))));
				}
				destination->AddObject(obj);
				RemoveObject(obj.collider);
			}
		}
	}
}

Core::Scene::Components::IComponent* PortalObject::CreateCopy()
{
	return new PortalObject(*this);
}

void PortalObject::RenderGui()
{
	auto& guiInterface = App::GetInstance()->GetInterfacing();
	guiInterface.DragFloat("Portal Opening Time", &animationTime, 0.1f, 0.0f, 1e20f);
}

Core::Scene::Components::ComponentType PortalObject::GetType()
{
	return ComponentType::PortalObject;
}

void PortalObject::Serialize(Serializer& sr) const
{
	sr.Write(animationTime);
}

void PortalObject::Deserialize(Deserializer& dr)
{
	dr.Read(animationTime);
}

void PortalObject::OnCollisionBegin(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider, Maths::Vec3 normal)
{
	if (!pCollider->isTrigger || GetObjectIndex(pOtherCollider) != ~0) return;
	if (portalRenderer->portalMesh && animationDelay < animationTime / 2) pOtherCollider->SetLayerType(static_cast<LayerType>(pOtherCollider->layerType & ~((isWall ? LayerType::WALL : (LayerType::GROUND | LayerType::CEILING)) | LayerType::ALLBUTPLAYER)));
	nearObjects.push_back(ObjectRef());
	nearObjects.back().collider = pOtherCollider;
	auto player = pOtherCollider->gameObject->GetComponent(ComponentType::PlayerManager);
	if (player) nearObjects.back().player = reinterpret_cast<PlayerManager*>(player);
}

void PortalObject::OnCollisionEnd(Colliders::ICollider* pCollider, GameObject* pOther, Colliders::ICollider* pOtherCollider)
{
	if (!pCollider->isTrigger) return;
	u64 index = GetObjectIndex(pOtherCollider);
	if (index == ~0 || nearObjects[index].ignoreObject) return;
	if (portalRenderer->portalMesh && animationDelay < animationTime / 2) pOtherCollider->SetLayerType(static_cast<LayerType>(pOtherCollider->layerType | LayerType::WALL | LayerType::GROUND | LayerType::CEILING));
	for (u64 i = index + 1; i < nearObjects.size(); ++i)
	{
		nearObjects[i - 1] = nearObjects[i];
	}
	nearObjects.pop_back();
}

void PortalObject::OpenPortal(PortalObject* dest)
{
	animationDelay = animationTime;
	destination = dest;
	portalRenderer->portalMesh = portalMesh;
}

void PortalObject::Attach(GameObject* parentIn, Maths::Vec3 off, Maths::Quat rot, bool wall)
{
	parentObj = parentIn;
	offset = off;
	rotation = rot;
	isWall = wall;
}

void PortalObject::Detach()
{
	parentObj = nullptr;
}

void PortalObject::ClosePortal()
{
	destination = nullptr;
	portalRenderer->portalMesh = nullptr;
	for (auto& obj : nearObjects)
	{
		obj.collider->SetLayerType(static_cast<LayerType>(obj.collider->layerType | LayerType::WALL | LayerType::GROUND | LayerType::CEILING));
	}
}

void PortalObject::AddObject(ObjectRef obj)
{
	u64 index = GetObjectIndex(obj.collider);
	if (index != ~0)
	{
		nearObjects[index].ignoreObject = 2;
		return;
	}
	nearObjects.push_back(obj);
	nearObjects.back().ignoreObject = 2;
}

u64 PortalObject::GetObjectIndex(Colliders::ICollider* object)
{
	for (u64 i = 0; i < nearObjects.size(); ++i)
	{
		if (nearObjects[i].collider == object) return i;
	}
	return ~0;
}

void PortalObject::RemoveObject(Colliders::ICollider* obj)
{
	u64 index = GetObjectIndex(obj);
	if (index == ~0) return;
	for (u64 i = index + 1; i < nearObjects.size(); ++i)
	{
		nearObjects[i - 1] = nearObjects[i];
	}
	nearObjects.pop_back();
}

const char* PortalObject::GetName()
{
	return "PortalObject";
}