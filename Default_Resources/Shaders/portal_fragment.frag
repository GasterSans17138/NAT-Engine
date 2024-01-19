#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPosition;

layout(binding = 1) uniform FragmentUniform
{
	vec3 matAmbient;
	float matShininess;
} ubo;

layout(binding = 2) uniform sampler2D texSampler;
layout(binding = 3) uniform sampler2D normSampler;
layout(binding = 4) uniform sampler2D heightSampler;

void main()
{
    outColor = vec4(ubo.matAmbient, 1.0);
	outNormal = vec4(0);
	outPosition = vec4(0);
}