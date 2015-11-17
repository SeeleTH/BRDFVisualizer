#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;
in vec3 outPosW;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

struct DirLight {
	vec3 dir;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material material;
uniform DirLight light;
uniform vec3 viewPos;

void main()
{
	vec3 normalW = normalize(outNormal);
	vec4 diffTex = texture(texture_diffuse1, outTexCoord);
	vec3 viewDir = normalize(outPosW - viewPos);

	float kEnergyConvervation = (8.0f + material.shininess) / (8.0 * M_PI);
	vec3 halfVec = normalize(-viewDir - light.dir);
	vec3 ambient = light.ambient * material.ambient;
	vec3 spec = light.specular * material.specular * kEnergyConvervation
		* pow(clamp(dot(normalW, halfVec), 0.f, 1.f), material.shininess);
	vec4 diff = diffTex * vec4(light.diffuse * material.diffuse
		* clamp(dot(-light.dir, normalW), 0.f, 1.f), 1.f);

	color = vec4(diff.xyz, 1.0f * diff.w);
}