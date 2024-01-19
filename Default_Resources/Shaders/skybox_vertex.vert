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

layout(location = 0) out VertexOutput
{
	vec3 outUVW;
} vOut;

void main()
{
    vOut.outUVW = vec3(ubo.model * vec4(inPosition, 0));
	// Convert cubemap coordinates into Vulkan coordinate space
	vOut.outUVW.xy *= -1.0;
	vec3 pos = inPosition;
	pos.x *= -1.0;
	gl_Position = (ubo.mvp * vec4(pos, 1.0)).xyww;
}
