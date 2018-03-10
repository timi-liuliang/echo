#include "Triangle.h"

namespace Echo
{
	void Triangle::set(const Vector3& vert0, const Vector3& vert1, const Vector3& vert2)
	{
		v0 = vert0;
		v1 = vert1;
		v2 = vert2;
	}

	Echo::Vector3 Triangle::GetNormal() const
	{
		// calc triangle normal
		Vector3 normal = (v1 - v0).cross(v2 - v1);
		normal.normalize();

		return normal;
	}
}
