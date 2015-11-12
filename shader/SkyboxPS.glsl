#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec3 outDir;

out vec4 color;

uniform samplerCube envmap;

void main()
{
	color = texture(envmap, outDir);
}