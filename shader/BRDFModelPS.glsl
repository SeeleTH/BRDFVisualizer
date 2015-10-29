#version 330 core

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

void main()
{
	vec3 normal = normalize(outNormal);
	color = texture(texture_diffuse1, outTexCoord);
	//color = vec4(outTexCoord, 1.0f, 1.0f);
	//color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}