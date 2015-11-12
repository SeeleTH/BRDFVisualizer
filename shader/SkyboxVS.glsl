#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec2 texCoord;

out vec3 outDir;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 tranInvModel;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	outDir = normalize(gl_Position);
}