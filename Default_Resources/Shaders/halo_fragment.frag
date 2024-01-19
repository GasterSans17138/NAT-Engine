#version 450

layout(location = 0) in VertexOutput
{
	vec2 fragUV;
} vOut;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform FragmentUniform
{
	vec4 matAmbient;
} ubo;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D heightSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

void main()
{
    outColor = vec4(ubo.matAmbient, 1.0);
}