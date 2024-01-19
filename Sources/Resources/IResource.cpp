#include "Resources/IResource.hpp"
#include <filesystem>
#include "Maths/Maths.hpp"
#include "Core/Debugging/Log.hpp"
#include "Core/App.hpp"

using namespace Resources;

Core::App* IResource::app = nullptr;

void IResource::Write(Core::Serialization::Serializer& sr)
{
	sr.Write(path);
}

void IResource::Load(Core::Serialization::Deserializer& dr)
{
	dr.Read(path);
}

bool Resources::IResource::CanBeEdited() const
{
	return canBeEdited;
}

void Resources::IResource::WindowCreateResource(bool& open)
{
	Assert(0);
}

IResource::IResource()
{
	if (!app) app = Core::App::GetInstance();
}

bool IResource::ShowEditWindow()
{
	auto& gui = app->GetInterfacing();
	bool open = IResource::BeginEditWindow(gui);
	return IResource::EndEditWindow(gui) || !open;
}

bool IResource::BeginEditWindow(Wrappers::Interfacing& gui)
{
	bool open = true;
	gui.BeginClose("Edit Resource", open);
	if (!gui.IsEscPressed() && gui.InputText("Resource Name", path))
	{
		isSaved = false;
	}
	return open;
}

bool IResource::EndEditWindow(Wrappers::Interfacing& gui)
{
	gui.Separator();
	bool result = gui.Button("Close") || gui.IsEscPressed();
	gui.SameLine();
	gui.SetCursorX(gui.GetCursorX() + 10);

	if (gui.Button("Save") || ImGui::Shortcut(ImGuiKey_S | ImGuiMod_Ctrl))
	{
		app->GetResources().SaveAsset(this);
		LOG(DEBUG_LEVEL::LINFO, "Asset %s was saved", this->path);
	}

	gui.End();

	return result;
}
