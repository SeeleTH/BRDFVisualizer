#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;

out vec2 outTexCoord;
out vec3 outNormal;
out vec4 outTangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 tranInvModel;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	outNormal = (tranInvModel * vec4(normal, 0.0)).xyz;
	outTangent = model * vec4(tangent, 0.0);
	outTexCoord = texCoord;
}