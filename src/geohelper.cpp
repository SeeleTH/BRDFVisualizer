#include "geohelper.h"
#include "mathhelper.h"

#include <glm/glm.hpp>

namespace NPGeoHelper
{
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
				vert.pos = NPMathHelper::Vec3(x, y, z);
				vert.norm = NPMathHelper::Vec3::normalize(vert.pos);
				vert.binorm = NPMathHelper::Vec3::normalize(NPMathHelper::Vec3::cross(vert.norm,(NPMathHelper::Vec3(0.f, 1.f, 0.f))));
				vert.tan = NPMathHelper::Vec3::normalize(vert.binorm);
				vert.tex._x = (float)h / (float)(horiSlice);
				vert.tex._y = (float)v / (float)(vertSlice);
				result.vertices.push_back(vert);
			}
		}
		//last
		{
			vertex vert;
			vert.pos = NPMathHelper::Vec3(0.f, radius, 0.f);
			vert.norm = NPMathHelper::Vec3(0.f, 1, 0.f);
			vert.binorm = NPMathHelper::Vec3(1.f, 0.f, 0.f);
			vert.tan = NPMathHelper::Vec3(0.f, 0.f, 1.f);
			vert.tex = NPMathHelper::Vec2(1.f, 1.f);
			result.vertices.push_back(vert);
		}

		//add indices
		for (unsigned int v = 0; v < vertSlice; v++)
		{
			for (unsigned int h = 0; h < horiSlice; h++)
			{
				unsigned int index = v * (horiSlice + 1) + h;
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(index + 1);
				result.indices.push_back(index);
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(index + 1 + horiSlice + 1);
				result.indices.push_back(index + 1);
			}
			// last
			{
				unsigned int index = v * (horiSlice + 1) + horiSlice;
				result.indices.push_back(index + horiSlice + 1);
				result.indices.push_back(v * (horiSlice + 1));
				result.indices.push_back(index);
				result.indices.push_back((v + 1) * (horiSlice + 1));
				result.indices.push_back(v * (horiSlice + 1));
				result.indices.push_back(index + horiSlice + 1);
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

	Geometry GetPlaneShape(const float width, const float height, const NPMathHelper::Vec3 normDir)
	{
		Geometry result;

		return result;
	}

	Geometry GetBoxShape(const float width, const float height, const float depth)
	{
		Geometry result;

		return result;
	}
}