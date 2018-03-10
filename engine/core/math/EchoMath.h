#pragma once

#include "Quaternion.h"
#include "Vector4.h"
#include "Matrix4.h"

namespace Echo
{
	namespace Math
	{
		// 计算法线(仅适用于Int型索�? trianglelist拓扑类型)
		void ECHO_EXPORT_FOUNDATION TBNComputeNormal( Vector3* normals, const Vector3* positions, int numVertex, const Word* tris, int numTris);

		//  Calculate tangent binormal(正确性未�?慎用)
		void ECHO_EXPORT_FOUNDATION TBNComputeTangentBinormal(Vector3* tangents, Vector3* binormals, const Vector3* positions, const Vector2* uvs, const Vector3* normals, int numVertex, const Word* tris, int numTris);
	}
}