#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outGlow;

layout(binding = 0) uniform FragmentUniform
{
	float exposure;
	float gamma;
} ubo;

layout(binding = 2) uniform sampler2D colorSampler;
layout(binding = 3) uniform sampler2D glowSampler;

void main()
{
	ivec2 pixelPos = ivec2(gl_FragCoord.xy);
	outGlow = texelFetch(glowSampler, pixelPos, 0);
    vec3 hdrColor = texelFetch(colorSampler, pixelPos, 0).rgb + outGlow.rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * ubo.exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / ubo.gamma));
  
    outColor = vec4(mapped, 1.0);
}