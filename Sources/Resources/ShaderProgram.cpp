#include "Resources/ShaderProgram.hpp"

#include "Core/Debugging/Log.hpp"
#include "Renderer/VulkanRenderer.hpp"
#include "Core/App.hpp"
using namespace Resources;
using namespace Core::Serialization;

ShaderProgram* ShaderProgram::defaultShaderProgram = nullptr;

void ShaderProgram::CreateShaderProgram(VertexShader* vertexIn, FragmentShader* fragmentIn, GeometryShader* geometryIn, ShaderVariant variant)
{
	type = variant;
	vertex = vertexIn;
	fragment = fragmentIn;
	geometry = geometryIn;
	if (geometry)
	{
		program = Renderer::RendererShaderProgram(&vertex->shader, &fragment->shader, &geometry->shader);
	}
	else
	{
		program = Renderer::RendererShaderProgram(&vertex->shader, &fragment->shader);
	}
}

ShaderProgram* Resources::ShaderProgram::GetDefaultShader()
{
	return defaultShaderProgram;
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::DeleteData()
{
	app->GetRenderer().UnLoadShaderProgram(this);
	auto& res = app->GetResources();
	res.Release(fragment);
	res.Release(vertex);
	res.Release(geometry);
	res.Release(shadowVariant);
	res.Release(cubeVariant);
	res.Release(shadowCubeVariant);
	res.Release(objectVariant);
	res.Release(haloVariant);
}

void ShaderProgram::AddShaderVariant(ShaderProgram* variant, ShaderVariant type)
{
	switch (type)
	{
	case ShaderVariant::Shadow:
		AddShadowVariant(variant);
		break;
	case ShaderVariant::Cube:
		AddCubeVariant(variant);
		break;
	case ShaderVariant::ShadowCube:
		AddShadowCubeVariant(variant);
		break;
	case ShaderVariant::Object:
		AddObjectVariant(variant);
		break;
	case ShaderVariant::Halo:
		AddHaloVariant(variant);
		break;
	default:
		LOG(DEBUG_LEVEL::LWARNING, "Trying to add invalid shader variant");
		break;
	}
}

void ShaderProgram::AddShadowVariant(ShaderProgram* variant)
{
	shadowVariant = variant;
}

void ShaderProgram::AddCubeVariant(ShaderProgram* variant)
{
	cubeVariant = variant;
}

void ShaderProgram::AddShadowCubeVariant(ShaderProgram* variant)
{
	shadowCubeVariant = variant;
}

void Resources::ShaderProgram::AddObjectVariant(ShaderProgram* variant)
{
	objectVariant = variant;
}

void Resources::ShaderProgram::AddHaloVariant(ShaderProgram* variant)
{
	haloVariant = variant;
}

void ShaderProgram::SetDefaultShader()
{
	defaultShaderProgram = this;
}

void Resources::ShaderProgram::Write(Serializer& sr)
{
	sr.Write(static_cast<u8>(ObjectType::ShaderProgramType));
	IResource::Write(sr);
	u8 bitMask = 0;
	if (geometry)
	{
		bitMask |= 1;
	}
	if (shadowVariant)
	{
		bitMask |= 2;
	}
	if (cubeVariant)
	{
		bitMask |= 4;
	}
	if (shadowCubeVariant)
	{
		bitMask |= 8;
	}
	if (objectVariant)
	{
		bitMask |= 16;
	}
	if (haloVariant)
	{
		bitMask |= 32;
	}
	sr.Write(static_cast<u8>(type));
	sr.Write(bitMask);
	sr.Write(vertex->hash);
	sr.Write(fragment->hash);
	if (geometry) sr.Write(geometry->hash);
	if (shadowVariant) sr.Write(shadowVariant->hash);
	if (cubeVariant) sr.Write(cubeVariant->hash);
	if (shadowCubeVariant) sr.Write(shadowCubeVariant->hash);
	if (objectVariant) sr.Write(objectVariant->hash);
	if (haloVariant) sr.Write(haloVariant->hash);
}

void Resources::ShaderProgram::Load(Deserializer& dr)
{
	IResource::Load(dr);
	dr.Read(reinterpret_cast<u8&>(type));
	u8 bitMask = 0;
	dr.Read(bitMask);
	u64 hash;
	dr.Read(hash);
	vertex = app->GetResources().Get<VertexShader>(hash);
	dr.Read(hash);
	fragment = app->GetResources().Get<FragmentShader>(hash);
	if (bitMask & 1)
	{
		dr.Read(hash);
		geometry = app->GetResources().Get<GeometryShader>(hash);
	}
	if (bitMask & 2)
	{
		dr.Read(hash);
		shadowVariant = app->GetResources().Get<ShaderProgram>(hash);
	}
	if (bitMask & 4)
	{
		dr.Read(hash);
		cubeVariant = app->GetResources().Get<ShaderProgram>(hash);
	}
	if (bitMask & 8)
	{
		dr.Read(hash);
		shadowCubeVariant = app->GetResources().Get<ShaderProgram>(hash);
	}
	if (bitMask & 16)
	{
		dr.Read(hash);
		objectVariant = app->GetResources().Get<ShaderProgram>(hash);
	}
	if (bitMask & 32)
	{
		dr.Read(hash);
		haloVariant = app->GetResources().Get<ShaderProgram>(hash);
	}
	CreateShaderProgram(vertex, fragment, geometry, type);

	app->GetRenderer().LoadShaderProgram(this, GetRenderPass());
	isLoaded = true;
}

LowRenderer::RenderPassType Resources::ShaderProgram::GetRenderPass()
{
	LowRenderer::RenderPassType pass;
	switch (type)
	{
	case Resources::ShaderVariant::Default:
		pass = LowRenderer::RenderPassType::DEFAULT;
		break;
	case Resources::ShaderVariant::Shadow:
		pass = LowRenderer::RenderPassType::SHADOWMAP;
		break;
	case Resources::ShaderVariant::Cube:
		pass = LowRenderer::RenderPassType::CUBEMAP;
		break;
	case Resources::ShaderVariant::ShadowCube:
		pass = static_cast<LowRenderer::RenderPassType>(LowRenderer::RenderPassType::SHADOWMAP & LowRenderer::RenderPassType::CUBEMAP);
		break;
	case Resources::ShaderVariant::Object:
		pass = LowRenderer::RenderPassType::OBJECT;
		break;
	case Resources::ShaderVariant::Halo:
		pass = LowRenderer::RenderPassType::HALO;
		break;
	case Resources::ShaderVariant::Light:
		pass = LowRenderer::RenderPassType::LIGHT;
		break;
	case Resources::ShaderVariant::PostProcess:
		pass = LowRenderer::RenderPassType::POST;
		break;
	case Resources::ShaderVariant::WireFrame:
		pass = LowRenderer::RenderPassType::WIRE;
		break;
	case Resources::ShaderVariant::Depth:
		pass = LowRenderer::RenderPassType::DEPTH;
		break;
	case Resources::ShaderVariant::Window:
		pass = LowRenderer::RenderPassType::ALL;
		break;
	case Resources::ShaderVariant::Line:
		pass = static_cast<LowRenderer::RenderPassType>(LowRenderer::RenderPassType::WIRE | LowRenderer::RenderPassType::LINE);
		break;
	default:
		pass = LowRenderer::RenderPassType::DEFAULT;
		break;
	}

	return pass;
}

bool Resources::ShaderProgram::ShowEditWindow()
{
	static bool isValid = isLoaded;

	bool isModified = false;
	auto& gui = app->GetInterfacing();
	bool close = IResource::BeginEditWindow(gui);

	gui.Text("Shaders :");

#define SHADER_MODIFIED(){ isModified = true; isSaved = false;}

	//There will be a way to edit theses shaders in the future
	gui.Text("Vertex Shader :");	gui.SameLine();	gui.SetCursorX(140);	if (gui.VertexShaderCombo(&vertex))		SHADER_MODIFIED();
	gui.Text("Fragment Shader :");	gui.SameLine();	gui.SetCursorX(140);	if (gui.FragmentShaderCombo(&fragment)) SHADER_MODIFIED();
	gui.Text("Geometry Shader :");	gui.SameLine();	gui.SetCursorX(140);	if (gui.GeometryShaderCombo(&geometry)) SHADER_MODIFIED();

	gui.Separator();

	gui.Text("Parameters :");
	gui.Text("Shader Variant : ");
	gui.SameLine();
	gui.SetCursorX(gui.GetCursorX() + 10);

	if (gui.BeginCombo("###ShaderVariant", shaderVariantStr[(int)type], 0))
	{
		for (int i = 0; i < shaderVariantStr.size(); i++)
		{
			if (gui.Selectable(shaderVariantStr[i]))
			{
				type = (ShaderVariant)i;
				SHADER_MODIFIED()
			}
		}
		gui.EndCombo();
	}

	gui.Separator();
	gui.Text("Variants :");

#define ShaderVariantCombo(variantType, programPtr) if(variantType != this->type){ gui.Text(shaderVariantStr[(int)variantType]);gui.SameLine(); gui.SetCursorX(100); if(gui.ShaderCombo(variantType, &programPtr)) SHADER_MODIFIED()}

	if (gui.CollapsingHeader("##VariantsHeader")) 
	{
		ShaderVariantCombo(ShaderVariant::ShadowCube, shadowCubeVariant);
		ShaderVariantCombo(ShaderVariant::Object, objectVariant);
		ShaderVariantCombo(ShaderVariant::Shadow, shadowVariant);
		ShaderVariantCombo(ShaderVariant::Cube, cubeVariant);
		ShaderVariantCombo(ShaderVariant::Halo, haloVariant);
	}

	if (!isValid)
		gui.ColoredText("Shader is not compiling ! Please double check.", { 1,0,0,1 });

	if (isModified) 
	{
		auto& renderer = app->GetRenderer();

		Resources::ShaderProgram old(*this);

		this->CreateShaderProgram(vertex, fragment, geometry, type);

		if (renderer.LoadShaderProgram(this, GetRenderPass()))
		{
			renderer.UnLoadShaderProgram(&old);

			isValid = true;
		}
		else
		{
			LOG(DEBUG_LEVEL::LERROR, "Something went wrong in shader compilation. Please double check.");

			isValid = false;

			renderer.UnLoadShaderProgram(this);
			*this = old;
		}
	}

	return IResource::EndEditWindow(gui) || !close;
}

void Resources::ShaderProgram::WindowCreateResource(bool& open)
{
	if(!prevbool && open)
		newShaderprog = app->GetResources().CreateResource<ShaderProgram>("newShaderProgram");
	prevbool = open;
	if (open)
	{
		Wrappers::Interfacing* gui = &app->GetInterfacing();
		gui->OpenPopup("Create ShaderProgram");
		if (gui->BeginPopupModal("Create ShaderProgram"))
		{
			static std::string tempName;
			gui->InputText("name", tempName);
			if (gui->BeginCombo("###TypeListCombo", shaderVariantStr[int(newShaderprog->type)], 0))
			{
				for (u8 t = 0; t <= (u8)ShaderVariant::SkyBox; t++)
				{
					if (gui->Selectable(shaderVariantStr[int(t)]))
					{
						newShaderprog->type = (ShaderVariant)t;
						ResetShaderprog(newShaderprog);
					}
				}
				gui->EndCombo();
			}

			gui->Text("vertex shader            :"); gui->SameLine();
			gui->VertexShaderCombo(&newShaderprog->vertex);
			gui->Text("fragment shader          :"); gui->SameLine();
			gui->FragmentShaderCombo(&newShaderprog->fragment);


			gui->Text("geometry shader          :"); gui->SameLine();
			gui->GeometryShaderCombo(&newShaderprog->geometry);

			if (newShaderprog->type == ShaderVariant::Shadow)
			{
				gui->Text("shadowVariant shader     :"); gui->SameLine();
				gui->ShaderCombo(newShaderprog->type, &newShaderprog->shadowVariant);
			}

			if (newShaderprog->type == ShaderVariant::Cube)
			{
				gui->Text("cubeVariant shader       :"); gui->SameLine();
				gui->ShaderCombo(newShaderprog->type, &newShaderprog->cubeVariant);
			}

			if (newShaderprog->type == ShaderVariant::ShadowCube)
			{
				gui->Text("shadowCubeVariant shader :"); gui->SameLine();
				gui->ShaderCombo(newShaderprog->type, &newShaderprog->shadowCubeVariant);
			}

			if (newShaderprog->type == ShaderVariant::Object)
			{
				gui->Text("objectVariant shader     :"); gui->SameLine();
				gui->ShaderCombo(newShaderprog->type, &newShaderprog->objectVariant);
			}

			if (newShaderprog->type == ShaderVariant::Halo)
			{
				gui->Text("haloVariant shader       :"); gui->SameLine();
				gui->ShaderCombo(newShaderprog->type, &newShaderprog->haloVariant);
			}




			if (gui->Button("create"))
			{
				newShaderprog->path = tempName;
				if (newShaderprog->path.size() > 0 && newShaderprog->vertex != nullptr && newShaderprog->fragment != nullptr)
				{
					newShaderprog->CreateShaderProgram(newShaderprog->vertex, newShaderprog->fragment, newShaderprog->geometry, newShaderprog->type);
					newShaderprog->isLoaded = app->GetRenderer().LoadShaderProgram(newShaderprog, GetRenderPass());
					gui->CloseCurrentPopup();
					open = false;
					tempName = "";
					
				}
				else
				{
					if (newShaderprog->path.size() <= 0)
					{
						DEBUG_LOG("your resource need name\n");
					}
					if (newShaderprog->vertex != nullptr || newShaderprog->fragment != nullptr)
					{
						DEBUG_LOG("your resource need fragment and vertex shader\n");
					}
				}
			}


			if (gui->Button("close"))
			{
				if (!newShaderprog->isLoaded)
					app->GetResources().GetShaderProgramList().pop_back();
				gui->CloseCurrentPopup();
				open = false;
			}
			gui->EndPopup();

		}
	
	}
}

void ShaderProgram::ResetShaderprog(ShaderProgram* shaderprog)
{
	shaderprog->path = "";
	shaderprog->vertex = nullptr;
	shaderprog->fragment = nullptr;
	shaderprog->geometry = nullptr;
	shaderprog->shadowVariant = nullptr;
	shaderprog->cubeVariant = nullptr;
	shaderprog->shadowCubeVariant = nullptr;
	shaderprog->objectVariant = nullptr;
	shaderprog->haloVariant = nullptr;
}

const ShaderProgram* ShaderProgram::GetShader(ShaderVariant variant) const
{
	switch (variant)
	{
	case ShaderVariant::Default:
		return this;
	case ShaderVariant::Shadow:
		return shadowVariant ? shadowVariant->GetShader() : defaultShaderProgram->GetShader();
	case ShaderVariant::Cube:
		return cubeVariant ? cubeVariant->GetShader() : defaultShaderProgram->GetShader();
	case ShaderVariant::ShadowCube:
		return shadowCubeVariant ? shadowCubeVariant->GetShader() : defaultShaderProgram->GetShader();
	case ShaderVariant::Object:
		return objectVariant ? objectVariant->GetShader() : defaultShaderProgram->GetShader();
	case ShaderVariant::Halo:
		return haloVariant ? haloVariant->GetShader() : defaultShaderProgram->GetShader();
	default:
		return defaultShaderProgram->GetShader();
	}
}