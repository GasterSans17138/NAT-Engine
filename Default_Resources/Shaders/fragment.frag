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

vec3 CalcBumpedNormal()
{
    vec3 Normal = normalize(vOut.worldNormal);
    vec3 Tangent = normalize(vOut.worldTangent);
	Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = -cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(normSampler, vOut.fragUV).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

void main()
{
	vec3 normal = CalcBumpedNormal();
	vec4 tex = texture(texSampler, vOut.fragUV);
	if (tex.a < 0.1) discard;
	vec3 col = tex.rgb * vOut.fragColor * ubo.matAmbient;
    outColor = vec4(col, tex.a);
	outNormal = vec4(normal, 1);
	outPosition = vec4(vOut.worldPos, ubo.matShininess);
}