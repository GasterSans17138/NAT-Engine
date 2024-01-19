#version 450

layout(location = 0) in vec2 fragUV;

layout(binding = 2) uniform sampler2D texSampler;

void main()
{
	vec4 tex = texture(texSampler, fragUV);
	if (tex.a < 0.1) discard;
}