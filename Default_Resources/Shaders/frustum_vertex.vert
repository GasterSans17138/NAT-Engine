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
	vec3 fragColor;
} vOut;

const int[36] values = int[](
	5, 3, 1,
	3, 8, 4,
	7, 6, 8,
	2, 8, 6,
	1, 4, 2,
	5, 2, 6,
	5, 7, 3,
	3, 7, 8,
	7, 5, 6,
	2, 4, 8,
	1, 3, 4,
	5, 1, 2
);

void main()
{
	int index = values[gl_VertexIndex%36] - 1;
	float x;
	if ((index & 2) > 0) x = 1.0f;
	else x = -1.0f;
	float y;
	if ((index & 1) > 0) y = 1.0f;
	else y = -1.0f;
	float z;
	if ((index & 4) > 0) z = 1.0f;
	else z = 0.0f;
	vec3 pos = vec3(x,y,z);
    gl_Position = ubo.mvp * vec4(pos, 1.0);
	vOut.worldPos = vec3(ubo.model * vec4(pos, 1.0));
	vOut.fragColor = vec3(1);
}
