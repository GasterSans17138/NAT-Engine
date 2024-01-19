#version 450

layout(binding = 0) uniform VertexUniform
{
    mat4 model;
    mat4 mvp;
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
} vOut;

void main()
{
    gl_Position = ubo.mvp * vec4(inPosition, 1);
	vOut.worldPos = vec3(ubo.model * vec4(inPosition, 1));
    vOut.fragColor = inColor;
	vOut.fragUV = inCoord;
	
	mat3 md = mat3(ubo.model);
	vOut.worldNormal = md * inNormal;
	vOut.worldTangent = md * inTangent;
}
