#version 330 core
#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 outTexCoord;
out vec3 outColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform sampler2D brdfTexture;
uniform int n_th;
uniform int n_ph;
uniform float i_yaw;
uniform float i_pitch;

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
	fTarget.y = (n_ph * n_th - 1) -( o_th * n_ph + o_ph);
	return texelFetch(brdfTexture, fTarget, 0);
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
	vec3 inVec;
	inVec.y = sin(i_yaw);
	inVec.x = cos(i_yaw) * sin(i_pitch);
	inVec.z = cos(i_yaw) * cos(i_pitch);
	inVec = normalize(inVec);
	//vec3 testIn = normalize(vec3(1.0f, 1.0f, 0.1f));
	vec3 result = SampleBRDF_Linear(inVec, normalize(position));
	//vec3 result = SampleBRDF_Linear(normalize(vec3(1.f, 1.f, 0.f)), normalize(position));

	//{
	//	float ith, iph;
	//	GetVectorIndex(inVec, ith, iph);
	//	float th, ph;
	//	GetVectorIndex(normalize(position), th, ph);
	//	result = FetchBRDF(int(ith), int(iph), int(th), int(ph));
	//}

	vec3 newposition = normalize(position) * (result.x + result.y + result.z) / 3.f;
	gl_Position = projection * view * model * vec4(newposition, 1.0);
	outTexCoord = texCoord;
	outColor = result;

	// debug
	//float th, ph;
	//GetVectorIndex(normalize(position), th, ph);
	//th = th / 16.f;
	//ph = ph / 64.f;
	//outColor = vec3(ph, 0.f, 0.f);
}