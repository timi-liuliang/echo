#include "InputGeometryData.h"

namespace Echo
{
	InputGeometryData::InputGeometryData()
		: m_offMeshConCount(0)
	{
	}

	InputGeometryData::~InputGeometryData()
	{
		destroyAll();
	}

	void InputGeometryData::addVertex( float x, float y, float z)
	{
		Vector3 vert(x, y, z);
		m_verts.push_back(vert);

		m_box.addPoint(vert);
	}

	void InputGeometryData::addTriangle(int a, int b, int c, int area, float maxSlope)
	{
		m_tris.push_back(a);
		m_tris.push_back(b);
		m_tris.push_back(c);

		rcChunkyTriInfo info;
		info.area = area;
		info.walkableSlopeAngle = maxSlope;
		m_triInfos.push_back(info);
	}

	// 获取三角形数量
	int InputGeometryData::getTriCount() const 
	{ 
		EchoAssert(m_tris.size() % 3 == 0);
		return m_tris.size() / 3; 
	}

	void InputGeometryData::destroyAll()
	{
		m_verts.clear();
		m_tris.clear();
		m_triInfos.clear();
		m_offMeshConCount = 0;
	}

	void InputGeometryData::addOffMeshConnection(const float* spos, const float* epos, const float rad, unsigned char bidir, unsigned char area, unsigned short flags)
	{
		if (m_offMeshConCount >= MAX_OFFMESH_CONNECTIONS) return;
		float* v = &m_offMeshConVerts[m_offMeshConCount * 3 * 2];
		m_offMeshConRads[m_offMeshConCount] = rad;
		m_offMeshConDirs[m_offMeshConCount] = bidir;
		m_offMeshConAreas[m_offMeshConCount] = area;
		m_offMeshConFlags[m_offMeshConCount] = flags;
		m_offMeshConId[m_offMeshConCount] = 1000 + m_offMeshConCount;
		memcpy(&v[0], spos, sizeof(Vector3));
		memcpy(&v[3], epos, sizeof(Vector3));
		m_offMeshConCount++;
	}
}
