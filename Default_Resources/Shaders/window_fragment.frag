#version 450

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D colorSampler;

void main()
{
	ivec2 pixelPos = ivec2(gl_FragCoord.xy);
    outColor = texelFetch(colorSampler, pixelPos, 0);
}