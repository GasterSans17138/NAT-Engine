#version 450

layout(location = 0) in VertexOutput
{
	vec3 worldPos;
	vec3 worldNormal;
	vec3 worldTangent;
	vec3 fragColor;
	vec2 fragUV;
} vOut;

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
	vec3 col = vOut.fragColor * ubo.matAmbient;
    outColor = vec4(col, 1.0);
	outNormal = vec4(0, 0, 0, 0);
	outPosition = vec4(vOut.worldPos, ubo.matShininess);
}