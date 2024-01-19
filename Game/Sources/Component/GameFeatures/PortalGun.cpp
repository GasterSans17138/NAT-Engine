#include "../Game/Header/Component/GameFeatures/PortalGun.hpp"
#include "Core/App.hpp"
#include "Wrappers/PhysicsEngine/JoltPhysicsEngine.hpp"
#include "../Game/Header/PlayerManager.hpp"
#include "../Game/Header/Component/GameFeatures/PortalObject.hpp"

using namespace  Core::Scene::Components::Game;
using namespace Core::Scene::Components;
using namespace Core::Serialization;

PortalGun::PortalGun()
{
}

PortalGun::~PortalGun()
{
}

void PortalGun::Init()
{
}

void PortalGun::Update()
{
}

IComponent* PortalGun::CreateCopy()
{
	return new PortalGun(*this);
}

void PortalGun::RenderGui()
{
	auto& interfaceGui = App::GetInstance()->GetInterfacing();
	interfaceGui.DragFloat("Max distance", &maxDistance, 1.0f, 0.0f, 1e20f);
	interfaceGui.Text("Portal Mesh :");
	interfaceGui.SameLine();
	interfaceGui.MeshListCombo(&portalMesh, "Portal Mesh");
	interfaceGui.Text("Portal Model :");
	interfaceGui.SameLine();
	interfaceGui.ModelListCombo(&portalModel, "Portal Model");
	interfaceGui.Text("Portal Collider :");
	interfaceGui.SameLine();
	interfaceGui.MeshListCombo(&portalMeshCollider, "Portal Collider");
	interfaceGui.Text("Portal Material A :");
	interfaceGui.SameLine();
	interfaceGui.MaterialListCombo(&portalMaterials[0], "Portal material A");
	interfaceGui.Text("Portal Material B :");
	interfaceGui.SameLine();
	interfaceGui.MaterialListCombo(&portalMaterials[1], "Portal material B");
	interfaceGui.Text("Portal Model 2 :");
	interfaceGui.SameLine();
	interfaceGui.ModelListCombo(&secondPortalModel, "Second Portal Model");
	interfaceGui.Text("Portal Shader 2 :");
	interfaceGui.SameLine();
	interfaceGui.ShaderCombo(Resources::ShaderVariant::Default, &secondPortalShader);
}

ComponentType PortalGun::GetType()
{
	return ComponentType::PortalGun;
}

void PortalGun::Serialize(Serializer& sr) const
{
	sr.Write(maxDistance);
	sr.Write(portalMesh ? portalMesh->hash : (u64)0);
	sr.Write(portalModel ? portalModel->hash : (u64)0);
	sr.Write(portalMeshCollider ? portalMeshCollider->hash : (u64)0);
	sr.Write(portalMaterials[0] ? portalMaterials[0]->hash : (u64)0);
	sr.Write(portalMaterials[1] ? portalMaterials[1]->hash : (u64)0);
	sr.Write(secondPortalModel ? secondPortalModel->hash : (u64)0);
	sr.Write(secondPortalShader ? secondPortalShader->hash : (u64)0);
}

void PortalGun::Deserialize(Deserializer& dr)
{
	auto& resources = App::GetInstance()->GetResources();
	dr.Read(maxDistance);
	u64 hash = 0;
	dr.Read(hash);
	if (hash) portalMesh = resources.Get<Resources::Mesh>(hash);
	dr.Read(hash);
	if (hash) portalModel = resources.Get<Resources::Model>(hash);
	dr.Read(hash);
	if (hash) portalMeshCollider = resources.Get<Resources::Mesh>(hash);
	dr.Read(hash);
	if (hash) portalMaterials[0] = resources.Get<Resources::Material>(hash);
	dr.Read(hash);
	if (hash) portalMaterials[1] = resources.Get<Resources::Material>(hash);
	dr.Read(hash);
	if (hash) secondPortalModel = resources.Get<Resources::Model>(hash);
	dr.Read(hash);
	if (hash) secondPortalShader = resources.Get<Resources::ShaderProgram>(hash);
}

void PortalGun::UseItem(Maths::Vec3 position, Maths::Vec3 direction, bool second)
{
	auto& physics = App::GetInstance()->GetPhysicsEngine();
	Wrappers::RayCastResult result;
	bool touched = physics.RayCast(position + direction * 2.0f, direction * maxDistance, result, ~LayerType::TRIGGER);
	if (portals[second].portal)
	{
		portals[second].portal->ClosePortal();
		portals[second].renderer->UseModel(portalModel);
		portals[second].renderer->materials[0] = portalMaterials[second];
	}
	if (portals[!second].portal)
	{
		portals[!second].portal->ClosePortal();
		portals[!second].renderer->UseModel(portalModel);
		portals[!second].renderer->materials[0] = portalMaterials[!second];
	}
	if (touched && (result.hitCollider->layerType & LayerType::PORTAL) && (!portals[!second].portal || !portals[!second].portal->gameObject->IsActive() || (result.position - portals[!second].portal->gameObject->transform.GetPosition()).GetLength() > 5.0f))
	{
		if (!portals[second].portal)
		{
			auto object = SceneManager::GetInstance()->GetGameObjectScene(gameObject)->AddChild();
			object->name = second ? "Orange Portal" : "Blue Portal";
			auto renderer = object->AddComponent<Rendering::PortalComponent>();
			renderer->boxSize = Maths::Vec3(5.25f, 7.875f, 3.2f);
			renderer->boxOffset = Maths::Vec3(0.0f, 0.0f, -1.6f);
			portals[second].portal = object->AddComponent<PortalObject>();
			portals[second].portal->portalMesh = portalMesh;
			auto mesh = object->AddChild();
			mesh->transform.SetScale(1);
			mesh->transform.SetPosition(Maths::Vec3(0,0, 0.029f));
			portals[second].renderer = mesh->AddComponent<RenderModelComponent>();
			portals[second].renderer->useCulling = false;
			portals[second].renderer->UseModel(portalModel);
			portals[second].renderer->materials[0] = portalMaterials[second];
			auto cd = object->AddComponent<Colliders::MeshCollider>();
			cd->UseMesh(portalMeshCollider);
			cd->SetLayerType(static_cast<LayerType>(LayerType::TRIGGER | LayerType::OBJECT | LayerType::WALL | LayerType::GROUND));
			auto box = object->AddComponent<Colliders::CubeCollider>();
			box->size = Maths::Vec3(1);
			box->isTrigger = true;
			App::GetInstance()->GetPhysicsEngine().SetColliderTrigger(box, true);
			object->ForceUpdate();
		}
		portals[second].portal->gameObject->SetActive(true);
		bool isWall = abs(result.normal.y) < 0.1f;
		Maths::Quat rot = (isWall ? Maths::Quat::AxisAngle(Maths::Vec3(0,1,0), Maths::Vec2(result.normal.z,result.normal.x).GetAngle()) : Maths::Quat::AxisAngle(Maths::Vec3(result.normal.y >= 0 ? -1 : 1,0,0), (f32)M_PI_2)) * Maths::Quat(result.hitCollider->gameObject->transform.GetGlobal()).Inverse();
		portals[second].portal->Attach(result.hitCollider->gameObject, result.position - result.hitCollider->gameObject->transform.GetGlobal().GetPositionFromTranslation(), rot, isWall);
		if (portals[!second].portal && portals[!second].portal->gameObject->IsActive())
		{
			portals[second].portal->OpenPortal(portals[!second].portal);
			portals[second].renderer->UseModel(secondPortalModel);
			portals[second].renderer->shader = secondPortalShader;
			portals[second].renderer->materials[0] = portalMaterials[second];
			portals[second].renderer->materials[1] = portalMaterials[second];
			portals[!second].portal->OpenPortal(portals[second].portal);
			portals[!second].renderer->UseModel(secondPortalModel);
			portals[!second].renderer->shader = secondPortalShader;
			portals[!second].renderer->materials[0] = portalMaterials[!second];
			portals[!second].renderer->materials[1] = portalMaterials[!second];
		}
	}
	else
	{
		if (portals[second].portal) portals[second].portal->gameObject->SetActive(false);
	}
}

const char* PortalGun::GetName()
{
	return "PortalGun";
}