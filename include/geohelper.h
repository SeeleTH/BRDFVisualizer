#ifndef GEOHELPER_H
#define GEOHELPER_H

#include <vector>

namespace NPGeoHelper
{
	struct vec2
	{
		float x;
		float y;
		vec2(const float setX = 0.f, const float setY = 0.f) : x(setX), y(setY) {}
		vec2 normalize()
		{
			float length = sqrt(x*x + y*y);
			return vec2(x / length, y / length);
		}
	};

	struct vec3
	{
		float x;
		float y;
		float z;
		vec3(const float setX = 0.f, const float setY = 0.f, const float setZ = 0.f) : x(setX), y(setY), z(setZ) {}
		vec3 cross(const vec3& other)
		{
			return vec3(y*other.z - z*other.y,
				z*other.x - x*other.z,
				x*other.y - y*other.x);
		}
		vec3 normalize()
		{
			float length = sqrt(x*x + y*y + z*z);
			return vec3(x / length, y / length, z / length);
		}
	};
	bool operator == (const vec3& lhs, const vec3& rhs);
	bool operator != (const vec3& lhs, const vec3& rhs);

	struct vertex
	{
		vec3 pos;
		vec3 norm;
		vec3 binorm;
		vec3 tan;
		vec2 tex;
	};

	struct Geometry
	{
		std::vector<vertex> vertices;
		std::vector<unsigned int> indices;
	};

	Geometry GetSlicedHemisphereShape(const float radius, const unsigned int vertSlice, unsigned int horiSlice);
}

#endif