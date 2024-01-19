#version 450

layout(binding = 0) uniform VertexUniform
{
    mat4 model;
    mat4 mvp;
	vec3 cameraPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inCoord;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;

layout(location = 0) out VertexOutput
{
	vec3 worldPos;
	vec3 worldNormal;
	vec3 worldTangent;
	vec3 fragColor;
	vec2 fragUV;
	vec3 fragPos;
	vec3 fragCamPos;
} vOut;

void main()
{
    gl_Position = ubo.mvp * vec4(inPosition, 1);
	vOut.worldPos = vec3(ubo.model * vec4(inPosition, 1));
	vOut.worldNormal = vec3(ubo.model * vec4(inNormal, 0));
	vOut.worldTangent = vec3(ubo.model * vec4(inTangent, 0));
    vOut.fragColor = inColor;
	vOut.fragUV = inCoord;
	
	vec3 fragNormal = normalize(inNormal);
	vec3 fragTang = normalize(inTangent);
	fragTang = normalize(fragTang - dot(fragTang, fragNormal) * fragNormal);
	vec3 fragCoTang = -cross(fragTang, fragNormal);
	
	mat3 md = mat3(ubo.model);
	vec3 T = normalize(md * inTangent);
    vec3 B = normalize(md * fragCoTang);
    vec3 N = normalize(md * inNormal);
    mat3 TBN = transpose(mat3(T, B, N));

    vOut.fragCamPos  = TBN * ubo.cameraPos;
    vOut.fragPos  = TBN * vOut.worldPos;
	
	//vOut.shadowPos = ubo.smvp * vec4(inPosition, 1);
}
