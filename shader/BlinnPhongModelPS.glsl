#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;
in vec3 outPosW;
in vec4 outShadowPosW;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_shadow;

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
uniform float biasMin;
uniform float biasMax;

float shadowCalculation(vec4 shadowpos, float bias)
{
	vec3 projCoords = (shadowpos / shadowpos.w).xyz;
	projCoords = projCoords * 0.5f + 0.5f;
	if (projCoords.z > 1.0)
		return 0.0;
	float closestDepth = texture(texture_shadow, projCoords.xy).r;
	float currentDepth = projCoords.z;
	float shadow = 0.f;
	vec2 texelSize = 1.0f / textureSize(texture_shadow, 0);
	for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			float pcfDepth = texture(texture_shadow, projCoords.xy + vec2(x, y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.f;
		}
	}
	shadow /= 9.0f;
	return shadow;
}

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
	float shadowBias = max(biasMax * (1.0f - dot(normalW, -light.dir)), biasMin);
	float shadowFraction = shadowCalculation(outShadowPosW, shadowBias);
	color = vec4(ambient + (1.f - shadowFraction) * (spec + diff.xyz), 1.0f * diff.w);
}