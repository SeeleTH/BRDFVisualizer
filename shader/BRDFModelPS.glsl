#version 330 core
#define M_PI 3.1415926535897932384626433832795

in vec2 outTexCoord;
in vec3 outNormal;
in vec4 outTangent;

out vec4 color;

uniform sampler2D texture_brdf;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

uniform int n_th;
uniform int n_ph;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 viewDir;

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
			pAngle = 3.f / 2.f * M_PI + asin(valueHori.x);
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
	fTarget.y = (n_ph * n_th - 1) - (o_th * n_ph + o_ph);
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
	vec3 bitangent = normalize(cross(tangent, normal));
	mat3 tbn = transpose(mat3(tangent, normal, bitangent));

	vec3 lightDirL = tbn * lightDir;
	vec3 viewDirL = tbn * viewDir;
	vec3 brdf = SampleBRDF_Linear(-lightDirL, -viewDirL);
	vec4 diff = texture(texture_diffuse1, outTexCoord);
	vec4 result = vec4(brdf, 1.0f) + diff * clamp(dot(-lightDir, normal), 0.f, 1.f);
	color = result;
	//color = diff;
	//color = vec4(vec3(dot(-lightDirL,vec3(0.f,0.f,1.f))),1.f);

	//color = texture(texture_diffuse1, outTexCoord);
	//color = vec4(outTexCoord, 1.0f, 1.0f);
	//color = vec4(normal, 1.0f);
	
	//vec3 test = normalize(tbn * vec3(0.f, 1.f, 0.f));
	//test = (test + vec3(1.0f, 1.0f, 1.0f))*0.5f;
	//color = vec4(test, 1.0f);
	//color = vec4(brdf, 1.0f);
}