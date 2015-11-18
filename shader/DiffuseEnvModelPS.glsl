#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;
in vec3 outPosW;

out vec4 color;

uniform sampler2D texture_brdf;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform samplerCube envmap;

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;
uniform vec3 viewPos;

uniform float env_multiplier;
uniform int max_samp;
uniform int init_samp;

// ==================
// Hammersley - Begin
// ==================

float radicalInverse_VdC(uint bits) {
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 hammersley2d(uint i, uint N) {
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

vec3 hemisphereSample_uniform(float u, float v) {
	float phi = v * 2.0 * M_PI;
	float cosTheta = 1.0 - u;
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return vec3(cos(phi) * sinTheta, cosTheta, sin(phi) * sinTheta);
}

vec3 hemisphereSample_cos(float u, float v) {
	float phi = v * 2.0 * M_PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return vec3(cos(phi) * sinTheta, cosTheta, sin(phi) * sinTheta);
}

// ================
// Hammersley - End
// ================

void main()
{
	vec3 normal = normalize(outNormal);
	vec3 tangent = normalize(outTangent.xyz - dot(normal, outTangent.xyz) * normal);
	//tangent = normalize(cross(vec3(0.f, 1.f, 0.f), normal));
	vec3 bitangent = normalize(cross(tangent, normal));
	mat3 tnb = mat3(tangent, normal, bitangent);
	mat3 ttnb = transpose(tnb);
	vec3 viewDir = normalize(outPosW - viewPos);

	vec3 normalW = normalize(outNormal);
	vec4 diffTex = texture(texture_diffuse1, outTexCoord);

	vec2 sampHemiSpace = hammersley2d(uint(init_samp), uint(max_samp));
	vec3 sampDir = normalize(hemisphereSample_cos(sampHemiSpace.x, sampHemiSpace.y));
	vec3 sampDirW = tnb * sampDir;

	vec3 lightColor = env_multiplier * texture(envmap, sampDirW).rgb;
	float kEnergyConvervation = (8.0f + material.shininess) / (8.0 * M_PI);
	vec3 halfVec = normalize(-viewDir + sampDirW);
	//vec3 spec = lightColor * material.specular * kEnergyConvervation
	//	* pow(clamp(dot(normalW, halfVec), 0.f, 1.f), material.shininess);
	vec4 diff = diffTex * vec4(lightColor * material.diffuse
		* clamp(dot(sampDirW, normalW), 0.f, 1.f), 1.f);

	vec4 result = vec4(diff.xyz, 1.0f * diff.w);
	result.a *= 1.f / float(init_samp + 1);
	color = result;
}