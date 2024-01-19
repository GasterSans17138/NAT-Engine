#version 450

layout(binding = 0) uniform VertexUniform
{
    mat4 model;
    mat4 mvp;
	mat4 smvp;
	vec3 cameraPos;
	vec3 lightDirection;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out VertexOutput
{
	vec2 fragUV;
} vOut;

void main()
{
    gl_Position = ubo.mvp * vec4(inPosition, 1);
	vOut.fragUV = inCoord;
}
