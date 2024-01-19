#version 450

layout(location = 0) in VertexOutput
{
	vec3 outUVW;
} vOut;

layout(location = 0) out uint outColor;

layout(binding = 1) uniform FragmentUniform
{
	uint objectID;
} ubo;

layout(binding = 2) uniform samplerCube skySampler;
layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D heightSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

void main()
{
    outColor = ubo.objectID;
}