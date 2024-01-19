#include "Resources/IResource.hpp"
#include "Resources/Mesh.hpp"
#include "Resources/Model.hpp"

#include "Core/App.hpp"

#include "Core/Scene/Components/Colliders/MeshCollider.hpp"

namespace Core::Scene::Components::Colliders
{
	void			MeshCollider::Serialize(Core::Serialization::Serializer& sr) const
	{
		ICollider::Serialize(sr);
		sr.Write(mResourceHash);
		sr.Write((u8)mResourceType);
		sr.Write(mIsConvexShape);
	}

	void			MeshCollider::Deserialize(Core::Serialization::Deserializer& dr)
	{
		ICollider::Deserialize(dr);
		dr.Read(mResourceHash);
		
		u8 typeValue = 0;
		dr.Read(typeValue);

		mResourceType = (Resources::ObjectType)typeValue;

		dr.Read(mIsConvexShape);
	}

	void			MeshCollider::Init()
	{
		if (mIsInitialised)
			return;

		ICollider::Init();

		if (mResourceHash != 0) //Create Collider
		{
			if		(mResourceType == Resources::ObjectType::ModelType)		physicEngine->AddMeshCollider(*App::GetInstance()->GetResources().Get<Resources::Model>(mResourceHash), this, mIsConvexShape);
			else if (mResourceType == Resources::ObjectType::MeshType)		physicEngine->AddMeshCollider(*App::GetInstance()->GetResources().Get<Resources::Mesh> (mResourceHash), this, mIsConvexShape);

			physicEngine->SetColliderRotation(this, mColliderRotationOffset);
			physicEngine->SetColliderPosition(this, mColliderOffset);
			physicEngine->SetMotionType(this, colliderType);
			physicEngine->SetLayerType(this, layerType);
			physicEngine->SetColliderTrigger(this, isTrigger);
			//Size
		}

		mIsInitialised = true;
	}

	IComponent*		MeshCollider::CreateCopy()
	{
		auto result = new MeshCollider(*this);
		result->mIsInitialised = false;
		return result;
	}

	void			MeshCollider::RenderGui()
	{
		static bool selectingModel = false;
		static bool selectingMesh = false;

		auto& gui = App::GetInstance()->GetInterfacing();

		gui.Text("Mesh Collider :");

		if (mResourcePtr != nullptr)
		{
			const char* resourceText = mResourceType == Resources::ObjectType::MeshType ? "Current Mesh : " : "Current Model : ";
			gui.Text(resourceText);
			gui.SameLine();
			gui.Text(mResourcePtr->path.c_str());
		}

		if (!selectingModel && !selectingMesh)
		{
			if (gui.Button("Select new Model"))
				selectingModel = true;

			gui.SameLine();
			gui.SetCursorX(gui.GetCursorX() + 20);

			if (gui.Button("Select new Mesh"))
				selectingMesh = true;
		}

		Resources::Model* selectedModel = nullptr;
		Resources::Mesh*  selectedMesh  = nullptr;
		
		if (selectingModel && gui.ModelListCombo(&selectedModel))	{ this->UseModel(selectedModel);	selectedModel = nullptr;	selectingModel = false; };
		if (selectingMesh  && gui.MeshListCombo(&selectedMesh  ))   { this->UseMesh(selectedMesh);		selectedMesh = nullptr;		selectingMesh = false;	};
		
		if (gui.CheckBox("Convex shape", &mIsConvexShape) && mResourcePtr != nullptr)
		{
			//Recreate collider
			physicEngine->RemovedBodyFromSimulation(this);
			
			if		(mResourceType == Resources::ObjectType::ModelType)	this->UseModel((Resources::Model*)mResourcePtr);
			else if (mResourceType == Resources::ObjectType::MeshType)  this->UseMesh ((Resources::Mesh*) mResourcePtr);
		}

		gui.Separator();
		gui.Text("Parameters :");

		ICollider::RenderGui();
	}

	ComponentType	MeshCollider::GetType()
	{
		return ComponentType::MeshCollider;
	}

	const char*		MeshCollider::GetName()
	{
		return "Mesh Collider";
	}

	//Use Model
	//Use Mesh

	void			MeshCollider::UseModel(const Resources::Model* pModel)
	{
		App::GetInstance()->GetResources().Release(mResourcePtr);
		mResourceHash = pModel->hash;
		mResourceType = Resources::ObjectType::ModelType;
		mResourcePtr = (Resources::IResource*)pModel;

		physicEngine->AddMeshCollider(*pModel, this, mIsConvexShape);
	}

	void			MeshCollider::UseMesh(const Resources::Mesh* pMesh)
	{
		App::GetInstance()->GetResources().Release(mResourcePtr);
		mResourceHash = pMesh->hash;
		mResourceType = Resources::ObjectType::MeshType;
		mResourcePtr = (Resources::IResource*)pMesh;

		physicEngine->AddMeshCollider(*pMesh, this, mIsConvexShape);
	}

	void MeshCollider::Delete()
	{
		App::GetInstance()->GetResources().Release(mResourcePtr);
		IPhysicalComponent::Delete();
	}

	void			MeshCollider::OnScaleUpdate(const Maths::Vec3& pSize)
	{
		//Need to stay empty or it will EXPLODE. (fr, no resize on mesh collider.)

		/*
		*    OOOOOO
		* OOO      O
		*O  O   OOOOO
		*O  O  O sus O
		*O  O   OOOOO
		*O  O      O
		* OOO      O
		*   O      O
		*   O  OO  O
		*   OOO  OOO
		*/
	}
}