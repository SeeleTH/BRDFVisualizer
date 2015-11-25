#version 330 core
#define M_PI 3.1415926535897932384626433832795
#define ITR_COUNT 1

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;
in vec3 outPosW;
in vec4 outShadowPosW;

out vec4 color;

uniform sampler2D texture_brdf;
uniform sampler2D texture_diffuse1;
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

uniform int n_th;
uniform int n_ph;
uniform int max_samp;
uniform int init_samp;
uniform vec3 samp_dir_w;

uniform vec3 forced_tangent_w;

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

void GetVectorIndex(vec3 value, out float th, out float ph)
{
	value = normalize(value);
	float tAngle = acos(value.y);
	th = tAngle / (0.5f * M_PI) * n_th;
	vec3 valueHori = normalize(vec3(value.x, 0.f, value.z));
	float pAngle;
	if (valueHori.x > 0)
	{
		if (valueHori.z >= 0)
		{
			pAngle = 3.f / 2.f * M_PI + acos(valueHori.z);
		}
		else
		{
			pAngle = asin(-valueHori.z);
		}
	}
	else
	{
		if (valueHori.z >= 0)
		{
			pAngle = M_PI + asin(valueHori.z);
		}
		else
		{
			pAngle = 0.5f * M_PI + asin(-valueHori.x);
		}
	}
	ph = pAngle / (2.f * M_PI) * n_ph;
}

vec3 FetchBRDF(int i_th, int i_ph, int o_th, int o_ph)
{
	ivec2 fTarget;
	fTarget.x = i_th * n_ph + i_ph;
	fTarget.y = o_th * n_ph + o_ph;
	return texelFetch(texture_brdf, fTarget, 0).xyz;
}

vec3 OutFourPointsSample(ivec2 i_vec, vec2 o_vec)
{
	ivec2 o_f_vec = ivec2(floor(o_vec));
	ivec2 o_c_vec = ivec2(ceil(o_vec));
	vec2 o_d_vec = o_vec - o_f_vec;

	vec3 resultA = FetchBRDF(i_vec.x, i_vec.y, o_f_vec.x, o_f_vec.y);
	vec3 resultB = FetchBRDF(i_vec.x, i_vec.y, o_c_vec.x, o_f_vec.y);
	vec3 resultFirst = mix(resultA, resultB, o_d_vec.x);

	vec3 resultC = FetchBRDF(i_vec.x, i_vec.y, o_f_vec.x, o_c_vec.y);
	vec3 resultD = FetchBRDF(i_vec.x, i_vec.y, o_c_vec.x, o_c_vec.y);
	vec3 resultSecond = mix(resultC, resultD, o_d_vec.x);

	return mix(resultFirst, resultSecond, o_d_vec.y);
}

vec3 SampleBRDF_Linear(vec3 iL, vec3 oL)
{
	vec2 i_vec, o_vec;
	GetVectorIndex(iL, i_vec.x, i_vec.y);
	GetVectorIndex(oL, o_vec.x, o_vec.y);
	ivec2 i_f_vec = ivec2(floor(i_vec));
	ivec2 i_c_vec = ivec2(ceil(i_vec));
	vec2 i_d_vec = i_vec - i_f_vec;

	vec3 resultA = OutFourPointsSample(i_f_vec, o_vec);
	vec3 resultB = OutFourPointsSample(ivec2(i_c_vec.x, i_f_vec.y), o_vec);
	vec3 resultFirst = mix(resultA, resultB, i_d_vec.x);

	vec3 resultC = OutFourPointsSample(ivec2(i_f_vec.x, i_c_vec.y), o_vec);
	vec3 resultD = OutFourPointsSample(i_c_vec, o_vec);
	vec3 resultSecond = mix(resultC, resultD, i_d_vec.x);

	return mix(resultFirst, resultSecond, i_d_vec.y);
}

void main()
{
	vec3 normal = normalize(outNormal);
	vec3 tangent = normalize(outTangent.xyz - dot(normal, outTangent.xyz) * normal);
	if (length(forced_tangent_w) > 0.1f)
	{
		vec3 temp_tangent = normalize(forced_tangent_w - dot(normal, forced_tangent_w) * normal);
		if (length(temp_tangent) > 0.1f)
		{
			tangent = temp_tangent;
		}
	}
	vec3 bitangent = normalize(cross(tangent, normal));
	mat3 tnb = mat3(tangent, normal, bitangent);
	mat3 ttnb = transpose(tnb);
	vec3 viewDir = normalize(outPosW - viewPos);

	vec4 result = vec4(0.f, 0.f, 0.f, 0.f);
	vec3 viewDirL = ttnb * viewDir;
	vec4 diff = texture(texture_diffuse1, outTexCoord);

	vec3 sampDir = ttnb * normalize(samp_dir_w);
	if (sampDir.y > 0.f)
	{
		vec3 brdf = clamp(SampleBRDF_Linear(sampDir, -viewDirL), vec3(0.f), vec3(1.0f));
		vec3 lightColor = env_multiplier * texture(envmap, samp_dir_w).rgb;
		result = 2.f * vec4(lightColor, 1.0f) * clamp(dot(sampDir, vec3(0.f, 1.f, 0.f)), 0.f, 1.f) * (vec4(material.specular, 1.0f) * vec4(brdf, 1.0f)
			+ vec4(material.diffuse, 1.0f) * diff )
			+ vec4(material.ambient, 1.0f);

		float shadowBias = max(biasMax * (1.0f - dot(normal, samp_dir_w)), biasMin);
		float shadowFraction = shadowCalculation(outShadowPosW, shadowBias);
		result = (1.f - shadowFraction) * result;
	}
	result.a = float(ITR_COUNT) / (init_samp + float(ITR_COUNT));
	color = result;
}