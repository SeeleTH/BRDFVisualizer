#include "geohelper.h"
#include "mathhelper.h"

#include <glm/glm.hpp>

namespace NPGeoHelper
{
	bool operator == (const vec3& lhs, const vec3& rhs)
	{
		return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
	}
	bool operator != (const vec3& lhs, const vec3& rhs)
	{
		return !(lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
	}

	Geometry GetSlicedHemisphereShape(const float radius, const unsigned int vertSlice, unsigned int horiSlice)
	{
		Geometry result;
		
		// add vertices
		float vertStep = (float)M_PI * 0.5f / (float)(vertSlice + 1);
		float horiStep = (float)M_PI * 2.f / (float)(horiSlice + 1); // IN RAD
		for (unsigned int v = 0; v < vertSlice + 1; v++)
		{
			float y = sin(v * vertStep) * radius;
			float wide = sqrtf(radius * radius - y*y);
			for (unsigned int h = 0; h < horiSlice + 1; h++)
			{
				float angle = h * horiStep;
				float x = cos(angle) * wide;
				float z = sin(angle) * wide;

				vertex vert;
				vert.pos = vec3(x, y, z);
				vert.norm = vert.pos.normalize();
				vert.binorm = vert.norm.cross(vec3(0.f,1.f,0.f)).normalize();
				vert.tan = vert.norm.cross(vert.binorm).normalize();
				vert.tex.x = (float)h / (float)(horiSlice);
				vert.tex.y = (float)v / (float)(vertSlice);
				result.vertices.push_back(vert);
			}
		}
		//last
		{
			vertex vert;
			vert.pos = vec3(0.f, radius, 0.f);
			vert.norm = vec3(0.f, 1, 0.f);
			vert.binorm = vec3(1.f, 0.f, 0.f);
			vert.tan = vec3(0.f, 0.f, 1.f);
			vert.tex = vec2(1.f, 1.f);
			result.vertices.push_back(vert);
		}

		//add indices
		for (unsigned int v = 0; v < vertSlice; v++)
		{
			for (unsigned int h = 0; h < horiSlice; h++)
			{
				unsigned int index = v * (horiSlice + 1) + h;
				result.indices.push_back(index);
				result.indices.push_back(index + 1);
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(index + 1);
				result.indices.push_back(index + 1 + horiSlice + 1);
				result.indices.push_back(index + horiSlice + 1);
			}
			// last
			{
				unsigned int index = v * (horiSlice + 1) + horiSlice;
				result.indices.push_back(index);
				result.indices.push_back(v * (horiSlice + 1));
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(v * (horiSlice + 1));
				result.indices.push_back((v+1) * (horiSlice + 1));
			}
		}
		//last
		{
			for (unsigned int h = 0; h < horiSlice; h++)
			{
				unsigned int index = vertSlice * (horiSlice + 1) + h;
				result.indices.push_back(index);
				result.indices.push_back(vertSlice * (horiSlice + 1) + horiSlice + 1);
				result.indices.push_back(index + 1);
			}
			//last
			{
				unsigned int index = vertSlice * (horiSlice + 1) + horiSlice;
				result.indices.push_back(index);
				result.indices.push_back(index + 1);
				result.indices.push_back(vertSlice * (horiSlice + 1));
			}
		}

		return result;
	}
}