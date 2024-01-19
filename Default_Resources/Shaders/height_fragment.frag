#version 450

layout(location = 0) in VertexOutput
{
	vec3 worldPos;
	vec3 worldNormal;
	vec3 worldTangent;
	vec3 fragColor;
	vec2 fragUV;
	vec3 fragPos;
	vec3 fragCamPos;
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

const float heightScale = 0.1;

vec3 view;

vec2 ParallaxMapping(vec2 texCoords)
{ 
	// number of depth layers
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), view)));
	// calculate the size of each layer
	float layerDepth = 1.0 / numLayers;
	// depth of current layer
	float currentLayerDepth = 0.0;
	// the amount to shift the texture coordinates per layer (from vector P)
	vec2 P = view.xy / view.z * heightScale; 
	P.y = -P.y;
	vec2 deltaTexCoords = P / numLayers;

	// get initial values
	vec2  currentTexCoords	 = texCoords;
	float currentDepthMapValue = 1 - texture(heightSampler, currentTexCoords).r;
	
	while(currentLayerDepth < currentDepthMapValue)
	{
		// shift texture coordinates along direction of P
		currentTexCoords -= deltaTexCoords;
		// get depthmap value at current texture coordinates
		currentDepthMapValue = 1 - texture(heightSampler, currentTexCoords).r;
		// get depth of next layer
		currentLayerDepth += layerDepth;
	}

	// get texture coordinates before collision (reverse operations)
	vec2 prevTexCoords = currentTexCoords + deltaTexCoords;

	// get depth after and before collision for linear interpolation
	float afterDepth = currentDepthMapValue - currentLayerDepth;
	float beforeDepth = (1 - texture(heightSampler, prevTexCoords).r) - currentLayerDepth + layerDepth;
 
	// interpolation of texture coordinates
	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalTexCoords = prevTexCoords * weight + currentTexCoords * (1.0 - weight);

	return finalTexCoords;
}

vec3 CalcBumpedNormal(in vec2 texCoords)
{
    vec3 Normal = normalize(vOut.worldNormal);
    vec3 Tangent = normalize(vOut.worldTangent);
	Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = -cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(normSampler, texCoords).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

void main()
{
	view = normalize(vOut.fragCamPos-vOut.fragPos);
	vec2 texCoords = ParallaxMapping(vOut.fragUV);
	if(texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
	{
		//discard;
	}
	vec4 color = texture(texSampler, texCoords);
	if (color.a < 0.1) discard;
	vec3 col = color.rgb * vOut.fragColor * ubo.matAmbient;
	outColor = vec4(col, 1.0);
	outNormal = vec4(CalcBumpedNormal(texCoords), 1);
	outPosition = vec4(vOut.worldPos, ubo.matShininess);
}