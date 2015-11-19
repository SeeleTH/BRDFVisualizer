#version 330 core
#define M_PI 3.1415926535897932384626433832795
#define ITR_COUNT 1

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;
in vec3 outPosW;
in vec4 outShadowPosW;

out vec4 color;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_shadow;

uniform samplerCube envmap;
uniform float env_multiplier;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;
uniform vec3 viewPos;
uniform vec3 samp_dir_w;
uniform int max_samp;
uniform int init_samp;
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
	vec3 n = normalize(outNormal);
	vec3 t = normalize(outTangent.xyz - dot(n, outTangent.xyz) * n);
	vec3 b = normalize(cross(t, n));
	mat3 tnb = mat3(t, n, b);
	mat3 ttnb = transpose(tnb);
	vec3 viewDir = normalize(outPosW - viewPos);

	vec3 normValue = texture(texture_normal1, outTexCoord).rbg;
	normValue = normValue * 2.f - vec3(1.f);
	vec3 normalW = tnb * normValue;

	vec4 result = vec4(0.f, 0.f, 0.f, 0.f);
	vec3 viewDirL = ttnb * viewDir;
	vec4 diffTex = texture(texture_diffuse1, outTexCoord);

	vec3 sampDir = ttnb * normalize(samp_dir_w);
	if (sampDir.y > 0.f)
	{
		vec3 lightColor = env_multiplier * texture(envmap, samp_dir_w).rgb;
		float kEnergyConvervation = (8.0f + material.shininess) / (8.0 * M_PI);
		vec3 halfVec = normalize(-viewDir + samp_dir_w);
		vec3 spec = lightColor * material.specular * kEnergyConvervation
			* pow(clamp(dot(normalW, halfVec), 0.f, 1.f), material.shininess);
		vec4 diff = diffTex * vec4(lightColor * material.diffuse
			* clamp(dot(samp_dir_w, normalW), 0.f, 1.f), 1.f);

		result = 2.f * vec4(spec + diff.xyz, 1.0f * diff.w);

		float shadowBias = max(biasMax * (1.0f - dot(n, samp_dir_w)), biasMin);
		float shadowFraction = shadowCalculation(outShadowPosW, shadowBias);
		result = (1.f - shadowFraction) * result;
	}
	result.a = float(ITR_COUNT) / (init_samp + float(ITR_COUNT));
	color = result;
}