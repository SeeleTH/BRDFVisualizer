#version 330 core

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;

out vec4 color;

uniform sampler2D dTexture;

void main()
{
	color = vec4(1.0f, 1.0f, 1.0f, 1.0f);// texture(dTexture, outTexCoord);
}