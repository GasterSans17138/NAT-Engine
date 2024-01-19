#pragma once

#include "Renderer/IRendererResource.hpp"
#include "Core/Serialization/Deserializer.hpp"
#include "Core/Serialization/Serializer.hpp"

namespace Core
{
	class App;
}

namespace Wrappers
{
	class Interfacing;
}

namespace Resources
{	
	//
	//IResource is a class supposed to be cached by the resource manager, should only contain raw data.
	//

	enum class ObjectType : u8;

	class NAT_API IResource
	{
	public:
		virtual void DeleteData() = 0;
		virtual bool EndEditWindow(Wrappers::Interfacing& gui);
		virtual bool BeginEditWindow(Wrappers::Interfacing& gui);
		virtual bool ShowEditWindow();
		virtual void Write(Core::Serialization::Serializer& sr);
		virtual void Load(Core::Serialization::Deserializer& dr);
		virtual void WindowCreateResource(bool& open);
		virtual ObjectType GetType() = 0;
		bool CanBeEdited() const;

		IResource();
		virtual ~IResource() = default;
		
		std::string path;
		u64 hash = 0;
		u32 refCount = 0;
		bool isLoaded = false;
		bool shouldDeleteData = false;
		bool isSaved = false;

	protected:
		bool canBeEdited = false;
		static Core::App* app;
	};
}