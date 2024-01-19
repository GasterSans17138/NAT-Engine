#version 450

layout(location = 0) in VertexOutput
{
	vec3 outUVW;
} vOut;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform FragmentUniform
{
	double time;
} ubo;

layout(binding = 2) uniform samplerCube skySampler;
layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D heightSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

void main()
{
    outColor = texture(skySampler, vOut.outUVW);
	outNormal = vec4(0);
	outPosition = vec4(0);
}