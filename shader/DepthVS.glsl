#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 lightMat;

void main()
{
	gl_Position = lightMat * model * vec4(position, 1.0f);
}