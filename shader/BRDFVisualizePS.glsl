#version 330 core

in vec2 outTexCoord;
in vec3 outColor;

out vec4 color;

//uniform sampler2D dTexture;

void main()
{
	color = vec4(outColor, 1.0f);// texture(dTexture, outTexCoord);
}