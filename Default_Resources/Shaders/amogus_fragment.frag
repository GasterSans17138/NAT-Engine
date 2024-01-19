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

float smin(float a, float b, float delta) {
    float h = clamp(0.5 + 0.5 * (a - b) / delta, 0.0, 1.0);
    return mix(a, b, h) - delta * h * (1.0 - h);
}

float circle(vec2 pos, vec2 center, float diameter)
{
    return length(pos-center) > diameter ? 0. : 1.;
}

float circle2(vec2 pos, vec2 centerA, vec2 centerB, float diameter, float delta)
{
    float a = length(pos-centerA) - diameter;
    float b = length(pos-centerB) - diameter;
    return smin(a,b,delta) < 0. ? 1.0 : 0.0;
}

void main()
{
    vec3 col = texture(texSampler, vOut.fragUV).rgb;
    col = mix(col, vec3(0.9,0.15,0.15)*0.7, circle2(vOut.fragUV, vec2(0.58,0.35), vec2(0.58,0.15), 0.03, 0.37));
    col = mix(col, vec3(0.9,0.15,0.15)*0.7, circle2(vOut.fragUV, vec2(0.35,0.65), vec2(0.35,0.45), 0.04, 0.35));
    col = mix(col, vec3(1.0,0.2,0.2)  *0.7, circle2(vOut.fragUV, vec2(0.5,0.7), vec2(0.5,0.4), 0.15, 0.3));
    col = mix(col, vec3(1.0,0.2,0.2)  *0.7, circle2(vOut.fragUV, vec2(0.42,0.35), vec2(0.42,0.15), 0.03, 0.37));
    col = mix(col, vec3(0.1,0.1,1.0)  *0.7, circle(vOut.fragUV, vec2(0.6,0.65), 0.1));
    col = mix(col, vec3(0.5,0.5,1.0)  *0.7, circle(vOut.fragUV, vec2(0.55,0.7), 0.015));
    col = mix(col, vec3(1.0,1.0,1.0)  *0.7, circle(vOut.fragUV, vec2(0.55,0.7), 0.01));

    outColor = vec4(col,1.0);
	outNormal = vec4(0);
	outPosition = vec4(0);
}