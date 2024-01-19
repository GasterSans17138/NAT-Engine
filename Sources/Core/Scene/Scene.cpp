#include "Core/Scene/Scene.hpp"

#include "Core/App.hpp"

namespace Core::Scene
{
    Scene::Scene() : IResource()
    {
    }

    Core::Scene::Scene::~Scene()
    {
    }

    Scene* Scene::CreateCopy()
    {
        Scene* result = new Scene();
        result->isSaved = isSaved;
        for (GameObject* gameObject : gameObjects)
        {
            result->gameObjects.push_back(new GameObject());
            gameObject->CopyTo(result->gameObjects.back());
        }
        result->path = path;
        result->isNodeOpen = isNodeOpen;
        result->hash = hash;
        return result;
    }

    void Scene::Init()
    {
        for (u64 i = 0; i < gameObjects.size(); ++i)
            gameObjects[i]->Init();
    }

    void Scene::Update()
    {
        for (u64 i = 0; i < gameObjects.size(); ++i)
            gameObjects[i]->Update();
    }

    void Scene::DataUpdate(bool updateTransform)
    {
        if (!scenes) scenes = &Core::App::GetInstance()->GetSceneManager();
        for (u64 i = 0; i < gameObjects.size(); ++i)
            gameObjects[i]->DataUpdate(updateTransform);
    }

    void Scene::Render(const Maths::Mat4& vp, const Maths::Mat4& modelOverride, const Maths::Frustum& frustum, LowRenderer::RenderPassType pass)
    {
        for (GameObject* gameObject : gameObjects)
        {
            gameObject->Render(vp, modelOverride, frustum, pass);
        }
    }

    ///Clean all GameObjects of the scene
    void Scene::DeleteData()
    {
        for (auto& child : gameObjects)
        {
            child->Clean();
            delete child;
        }
        gameObjects.clear();
    }

    void Scene::Write(Serialization::Serializer& sr)
    {
        sr.Write(static_cast<u8>(Resources::ObjectType::SceneType));
        IResource::Write(sr);
        sr.Write(static_cast<u8>(isNodeOpen));
        sr.Write(gameObjects.size());
        for (auto& child : gameObjects)
        {
            child->Serialize(sr);
        }
    }

    void Scene::Load(Serialization::Deserializer& dr)
    {
        IResource::Load(dr);
        dr.Read(reinterpret_cast<u8&>(isNodeOpen));
        u64 size = 0;
        dr.Read(size);
        for (u64 i = 0; i < size; i++)
        {
            gameObjects.push_back(new GameObject());
            gameObjects.back()->Deserialize(dr);
        }
    }

    void Scene::ForceUpdate()
    {
        for (auto& child : gameObjects)
        {
            child->ForceUpdate();
        }
    }

    GameObject* Scene::AddChild()
    {
        gameObjects.push_back(new GameObject());
        return gameObjects.back();
    }

    void Scene::WindowCreateResource(bool& open)
    {
        if (!prevbool && open)
            newScene = app->GetResources().CreateResource<Scene>(DEFAULTSCENE_NAME);
        prevbool = open;
        if (open)
        {
            Wrappers::Interfacing* gui = &app->GetInterfacing();
            gui->OpenPopup("Create Scene");
            if (gui->BeginPopupModal("Create Scene"))
            {
                static std::string tempName;
                gui->InputText("name", tempName);

                if (gui->Button("create"))
                {
                    newScene->path = tempName;
                    if (newScene->path.size() > 0)
                    {
                        newScene->isLoaded = true;
                        gui->CloseCurrentPopup();
                        open = false;
                        tempName = "";
                    }
                    else
                    {
                        if (newScene->path.size() <= 0)
                        {
                            LOG(DEBUG_LEVEL::LDEFAULT, "your resource need name");
                        }
                        else
                        {
                            LOG(DEBUG_LEVEL::LDEFAULT, "your resource need a sound data !");
                        }
                    }
                }

                if (gui->Button("close"))
                {
                    if (!newScene->isLoaded)
                        app->GetResources().GetSceneList().pop_back();
                    gui->CloseCurrentPopup();
                    open = false;
                }
                gui->EndPopup();
            }
        }
    }

    Resources::ObjectType Scene::GetType()
    {
        return Resources::ObjectType::SceneType;
    }
}