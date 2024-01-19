#version 450

layout(binding = 0) uniform VertexUniform
{
    mat4 model;
    mat4 mvp;
	mat4 smvp;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 0) out vec2 fragUV;

void main()
{
    gl_Position = ubo.smvp * vec4(inPosition, 1);
	fragUV = inCoord;
}
