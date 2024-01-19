TARGET?=$(shell $(CXX) -dumpmachine)

BIN=NATEngine
CXXFLAGS=-O0 -g -Wall -Wno-unknown-pragmas -Wno-invalid-offsetof -std=c++17
#CXXFLAGS += -pg
CFLAGS=$(CXXFLAGS)
CPPFLAGS=-IIncludes -IHeaders
LDFLAGS=-LExternals/assimp -LExternals/glfw3

ifeq ($(TARGET),x86_64-pc-cygwin)
# CYGWIN SPECIFICS
LDLIBS=-lglfw3 -lgdi32 -lglu32
Includes/ImGUI/imgui_impl_opengl3.o: CXXFLAGS+="-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM"
else ifneq (,$(filter x86_64%linux-gnu,$(TARGET)))
# LINUX SPECIFICS
LDLIBS=-lglfw3 -lvulkan -lassimp -ldl -lX11
endif

# PROGRAM OBJS
OBJS=  Sources/Main.o
OBJS+= Sources/Core/App.o
OBJS+= Sources/Core/FileManager.o
OBJS+= Sources/Core/PRNG.o
OBJS+= Sources/Core/Debugging/Log.o
OBJS+= Sources/Core/Scene/GameObject.o
OBJS+= Sources/Core/Scene/Scene.o
OBJS+= Sources/Core/Scene/SceneManager.o
OBJS+= Sources/Core/Scene/Transform.o
OBJS+= Sources/Core/Scene/Components/CameraComponent.o
OBJS+= Sources/Core/Scene/Components/IComponent.o
OBJS+= Sources/Core/Scene/Components/RenderModelComponent.o
OBJS+= Sources/Core/Serialization/Conversion.o
OBJS+= Sources/Core/Serialization/Deserializer.o
OBJS+= Sources/Core/Serialization/Serializer.o
OBJS+= Sources/LowRenderer/DepthBuffer.o
OBJS+= Sources/LowRenderer/FrameBuffer.o
OBJS+= Sources/LowRenderer/Rendering/Camera.o
OBJS+= Sources/LowRenderer/Rendering/FlyingCamera.o
OBJS+= Sources/Maths/Maths.o
OBJS+= Sources/Renderer/IRenderResource.o
OBJS+= Sources/Renderer/RendererDepthBuffer.o
OBJS+= Sources/Renderer/RendererFrameBuffer.o
OBJS+= Sources/Renderer/RendererMesh.o
OBJS+= Sources/Renderer/RendererShader.o
OBJS+= Sources/Renderer/RendererShaderProgram.o
OBJS+= Sources/Renderer/RendererTexture.o
OBJS+= Sources/Renderer/RendererTextureSampler.o
OBJS+= Sources/Renderer/RendererUniformObject.o
OBJS+= Sources/Renderer/RendererVertex.o
OBJS+= Sources/Renderer/VulkanRenderer.o
OBJS+= Sources/Resources/IResource.o
OBJS+= Sources/Resources/Material.o
OBJS+= Sources/Resources/Mesh.o
OBJS+= Sources/Resources/Model.o
OBJS+= Sources/Resources/ResourceManager.o
OBJS+= Sources/Resources/Shader.o
OBJS+= Sources/Resources/ShaderProgram.o
OBJS+= Sources/Resources/StaticTexture.o
OBJS+= Sources/Resources/Texture.o
OBJS+= Sources/Resources/TextureSampler.o
OBJS+= Sources/Wrappers/ImageLoader.o
OBJS+= Sources/Wrappers/Interfacing.o
OBJS+= Sources/Wrappers/ShaderLoader.o
OBJS+= Sources/Wrappers/WindowManager.o
OBJS+= Sources/Wrappers/ModelLoader/AssimpModelLoader.o
OBJS+= Sources/Wrappers/ModelLoader/IModelLoader.o
OBJS+= Sources/Wrappers/SceneLoader/ISceneLoader.o
OBJS+= Sources/Wrappers/SceneLoader/JsonSceneLoader.o

# IMGUI
OBJS+= Includes/ImGUI/imgui.o
OBJS+= Includes/ImGUI/imgui_demo.o
OBJS+= Includes/ImGUI/imgui_draw.o
OBJS+= Includes/ImGUI/imgui_widgets.o
OBJS+= Includes/ImGUI/imgui_tables.o
OBJS+= Includes/ImGUI/imgui_stdlib.o
# IMGUI (backends)
OBJS+= Includes/ImGUI/imgui_impl_glfw.o Includes/ImGUI/imgui_impl_vulkan.o
# OTHER
OBJS+= Includes/JsonParser/jWrite.o



DEPS=$(OBJS:.o=.d)

.PHONY: all clean

all: $(BIN)

-include $(DEPS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(CPPFLAGS) $< -o $@

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f $(BIN) $(OBJS) $(DEPS)

