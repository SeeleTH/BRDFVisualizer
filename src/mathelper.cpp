#include "mathhelper.h"

namespace NPMathHelper
{
	Vec3 Vec3::transform(const Mat4x4& mat4x4Left, const Vec3& v3Right, bool pos)
	{
		Vec4 v4Right = Vec4(v3Right, (float)pos);
		Mat4x4 tLeft = Mat4x4::transpose(mat4x4Left);
		Vec4 result = Vec4::transform(tLeft, v4Right);
		return Vec3(result._x, result._y, result._z);
	}

	Vec4 Vec4::transform(const Mat4x4& mat4x4Left, const Vec4& v4Right)
	{
		return Vec4(mat4x4Left._00 * v4Right._x + mat4x4Left._01 * v4Right._y + mat4x4Left._02 * v4Right._z + mat4x4Left._03 * v4Right._w
			, mat4x4Left._10 * v4Right._x + mat4x4Left._11 * v4Right._y + mat4x4Left._12 * v4Right._z + mat4x4Left._13 * v4Right._w
			, mat4x4Left._20 * v4Right._x + mat4x4Left._21 * v4Right._y + mat4x4Left._22 * v4Right._z + mat4x4Left._23 * v4Right._w
			, mat4x4Left._30 * v4Right._x + mat4x4Left._31 * v4Right._y + mat4x4Left._32 * v4Right._z + mat4x4Left._33 * v4Right._w);
	}
}