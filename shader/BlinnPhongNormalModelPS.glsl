#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;

out vec4 color;

uniform sampler2D texture_normal1;
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
uniform vec3 viewDir;

void main()
{
	vec3 n = normalize(outNormal);
	vec3 t = normalize(outTangent.xyz - dot(n, outTangent.xyz) * n);
	vec3 b = normalize(cross(t, n));
	mat3 tbn = mat3(t, n, b);
	mat3 ttbn = transpose(tbn);

	vec3 normValue = texture(texture_normal1, outTexCoord).rgb;
	normValue = normValue * 2.f - vec3(1.f);
	vec4 diffTex = texture(texture_diffuse1, outTexCoord);
	vec3 normalW = tbn * normValue;

	float kEnergyConvervation = (8.0f + material.shininess) / (8.0 * M_PI);
	vec3 halfVec = normalize(- viewDir - light.dir);
	vec3 ambient = light.ambient * material.ambient;
	vec3 spec = light.specular * material.specular * kEnergyConvervation 
		* pow(clamp(dot(normalW, halfVec), 0.f, 1.f), material.shininess);
	vec4 diff = diffTex * vec4(light.diffuse * material.diffuse 
		* clamp(dot(-light.dir, normalW), 0.f, 1.f), 1.f);

	color = vec4(ambient + spec + diff.xyz, 1.0f * diff.w);
}