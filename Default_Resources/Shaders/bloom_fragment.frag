#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outGlow;

layout(binding = 0) uniform FragmentUniform
{
	vec2 direction;
	vec2 resolution;
} ubo;

layout(binding = 2) uniform sampler2D colorSampler;
layout(binding = 3) uniform sampler2D glowSampler;

const float offset[3] = float[](0.0, 1.3846153846, 3.2307692308);
const float weight[3] = float[](0.2270270270, 0.3162162162, 0.0702702703);
 
void main()
{
	vec2 coords = vec2(gl_FragCoord);
	outGlow = texture(glowSampler, coords / ubo.resolution) * weight[0];
	for (int i=1; i<3; i++)
	{
		outGlow += texture(glowSampler, (coords + offset[i] * ubo.direction) / ubo.resolution) * weight[i];
		outGlow += texture(glowSampler, (coords - offset[i] * ubo.direction) / ubo.resolution) * weight[i];
	}
	outColor = texelFetch(colorSampler, ivec2(coords), 0);
}