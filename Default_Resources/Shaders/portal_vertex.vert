#version 450

layout(binding = 0) uniform VertexUniform
{
    mat4 model;
    mat4 mvp;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

void main()
{
    gl_Position = ubo.mvp * vec4(inPosition, 1);
}
