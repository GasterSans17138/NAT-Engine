#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec3 fragPos;
layout(location = 4) in vec4 shadowPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform FragmentUniform
{
    vec3 lightdir;
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	vec3 cameraPos;
	float shininess;
} ubo;

layout(binding = 2) uniform sampler2D texSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

void main()
{
	ivec2 uv = ivec2(fragUV*2.);
	float col = (uv.x + uv.y) % 2 == 0 ? 1. : 0.;
    outColor = vec4(col, 0, col, 1.);
}