#version 450

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outSat;

struct DirectionalLight
{
    vec3 Direction;
	float Shininess;
    vec3 Ambient;
    vec3 Diffuse;
    vec3 Specular;
};

struct PointLight
{
    vec3 Position;
	float Shininess;
    vec3 Ambient;
	float Radius;
    vec3 Diffuse;
	float Linear;
    vec3 Specular;
	float Quadratic;
};

struct SpotLight
{
    vec3 Position;
	float Shininess;
    vec3 Ambient;
	float Radius;
    vec3 Diffuse;
	float Linear;
    vec3 Specular;
	float Quadratic;
	vec3 Direction;
	vec2 Angles;
};

const int MAX_DIR_LIGHT = 8;
const int MAX_POINT_LIGHT = 64;
const int MAX_SPOT_LIGHT = 64;

layout(binding = 0) uniform FragmentUniform
{
	DirectionalLight dlights[MAX_DIR_LIGHT];
	PointLight plights[MAX_POINT_LIGHT];
	SpotLight slights[MAX_SPOT_LIGHT];
	vec3 viewPos;
	uint dCount;
	uint pCount;
	uint sCount;
} ubo;

layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform sampler2D normalSampler;
layout(binding = 4) uniform sampler2D positionSampler;
//layout(binding = 3) uniform sampler2DShadow shadowMap;

vec3 view;
vec3 pos;
vec3 normal;
float shininess;

#define SHADOWMAP_RESOLUTION 2048
#define DELTA 0.00005

const vec2 sampleOffsetDirections[13] = vec2[]
(
   vec2( 0,  0), 
   vec2( 0,  1),
   vec2( 0, -1),
   vec2( 1,  0),
   vec2( 1,  1),
   vec2( 1, -1), 
   vec2(-1,  0),
   vec2(-1,  1),
   vec2(-1, -1),
   vec2( 0,  2),
   vec2( 0, -2),
   vec2( 2,  0),
   vec2(-2,  0)
);
/*
float CalcShadowFactor()
{
    vec3 ProjCoords = vOut.shadowPos.xyz / vOut.shadowPos.w;
    vec2 UVCoords;
    UVCoords.x = 0.5 * ProjCoords.x + 0.5;
    UVCoords.y = 0.5 * ProjCoords.y + 0.5;
    float z = ProjCoords.z;
	const float dec = 1.0/SHADOWMAP_RESOLUTION;

    float Factor = 0.0;
	
	for (int i = 0; i < 13; i++)
	{
		vec2 Offsets = sampleOffsetDirections[i]*dec;
        vec3 UVC = vec3(UVCoords+Offsets, z + DELTA);
        //Factor += texture(shadowMap, UVC);
	}
    return (Factor / 13.0);
}
*/

vec3 GetDirectional(in DirectionalLight light)
{
	vec3 ldir = normalize(-light.Direction);
	float deltaA = dot(ldir, normal);
	if (deltaA < 0) deltaA = 0;
	vec3 halfV = normalize(ldir + view);
	float deltaB = pow(max(dot(normal, halfV), 0.0), light.Shininess * shininess);
	//float deltaS = CalcShadowFactor();
	float deltaS = 1.0;
	vec3 color = light.Ambient;
	color += light.Diffuse * (deltaA * deltaS);
	color += light.Specular * (deltaB * deltaS);
	return color;
}

vec3 GetPoint(in PointLight light)
{
	vec3 AB = light.Position-pos;
	float att = length(AB);
	if (att > light.Radius) return vec3(0);
	att = clamp(1.0/(1.0+light.Linear*att+light.Quadratic*att*att),0.0,1.0);
	AB = normalize(AB);
	float deltaA = dot(AB, normal);
	if (deltaA < 0) deltaA = 0;
	vec3 halfVV = normalize(AB + view);
	float deltaB = pow(max(dot(normal, halfVV), 0.0), light.Shininess);
	//float deltaS = CalcPointShadow(PIndex[i], lposition);
	float deltaS = 1.0;
	vec3 color = light.Ambient * att;
	color += light.Diffuse * (deltaA * deltaS * att);
	color += light.Specular * (deltaB * deltaS * att);
	return color;
}

vec3 GetSpot(in SpotLight light)
{
	vec3 AB = light.Position-pos;
	float att = length(AB);
	if (att > light.Radius) return vec3(0);
	att = clamp(1.0/(1.0+light.Linear*att+light.Quadratic*att*att),0.0,1.0);
	AB = normalize(AB);
	float ang = acos(dot(AB,-light.Direction));
	if (ang > light.Angles.x) return vec3(0);
	if (1-ang/light.Angles.x < light.Angles.y)
	{
		att *= (1-ang/light.Angles.x)/light.Angles.y;
	}
	float deltaA = dot(AB, normal);
	if (deltaA < 0) deltaA = 0;
	vec3 halfV = normalize(AB + view);
	float deltaB = pow(max(dot(normal, halfV), 0.0), light.Shininess);
	//float deltaS = CalcShadowFactor(SIndex[i], 2);
	float deltaS = 1.0;
	vec3 color = light.Ambient * att;
	color += light.Diffuse * (deltaA * deltaS * att);
	color += light.Specular * (deltaB * deltaS * att);
	return color;
}

vec3 GetLights()
{
	vec3 result = vec3(0);
	for (uint i = 0; i < ubo.dCount; i++)
	{
		result += GetDirectional(ubo.dlights[i]);
	}
	for (uint i = 0; i < ubo.pCount; i++)
	{
		result += GetPoint(ubo.plights[i]);
	}
	for (uint i = 0; i < ubo.sCount; i++)
	{
		result += GetSpot(ubo.slights[i]);
	}
	return result;
}

void main()
{
	ivec2 pixelPos = ivec2(gl_FragCoord.xy);
	vec4 cInput = texelFetch(albedoSampler, pixelPos, 0);
	vec4 nInput = texelFetch(normalSampler, pixelPos, 0);
	if (nInput.w < 1.)
	{
		outColor = cInput;
		outSat = vec4(0);
		return;
	}
	normal = normalize(nInput.xyz);
	vec4 pInput = texelFetch(positionSampler, pixelPos, 0);
	pos = pInput.xyz;
	shininess = pInput.w;
	view = normalize(ubo.viewPos - pos);
	
	vec3 total = cInput.xyz * GetLights();
	float brightness = dot(total, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 1.0)
        outSat = vec4(total, 1.);
    else
        outSat = vec4(0., 0., 0., 1.);
    outColor = vec4(total, 1);
	outSat = vec4(max(total-1., 0.),1.);
}