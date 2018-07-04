#pragma once

#include "engine/core/Memory/MemManager.h"
#include "Engine/core/geom/AABB.h"
#include "ChunkyTriMesh.h"

namespace Echo
{
	/**
	 * 几何体数据(用于寻路生成)
	 */
	class InputGeometryData
	{
	public:
		InputGeometryData();
		~InputGeometryData();

		const float* getVerts() const { return (float*)m_verts.data(); }
		const int* getTris() const { return m_tris.data(); }
		const rcChunkyTriInfo* getTriInfos() const { return m_triInfos.data(); }
		int getVertCount() const { return (int)m_verts.size(); }
		int getTriCount() const;
		void addVertex(float x, float y, float z);
		void addTriangle(int a, int b, int c, int area, float maxSlope);
		void destroyAll();
		AABB& getAABB() { return m_box; }

		// OffMeshLinks
		int getOffMeshConnectionCount() const { return m_offMeshConCount; }
		const float* getOffMeshConnectionVerts() const { return m_offMeshConVerts; }
		const float* getOffMeshConnectionRads() const { return m_offMeshConRads; }
		const unsigned char* getOffMeshConnectionDirs() const { return m_offMeshConDirs; }
		const unsigned char* getOffMeshConnectionAreas() const { return m_offMeshConAreas; }
		const unsigned short* getOffMeshConnectionFlags() const { return m_offMeshConFlags; }
		const unsigned int* getOffMeshConnectionId() const { return m_offMeshConId; }
		void addOffMeshConnection(const float* spos, const float* epos, const float rad,unsigned char bidir, unsigned char area, unsigned short flags);
		void deleteOffMeshConnection(int i);

	private:
		vector<Vector3>::type			m_verts;
		vector<int>::type				m_tris;
		vector<rcChunkyTriInfo>::type	m_triInfos;		// 三角形区域分类
		AABB								m_box;

		static const int	MAX_OFFMESH_CONNECTIONS = 1024;
		float				m_offMeshConVerts[MAX_OFFMESH_CONNECTIONS * 3 * 2];
		float				m_offMeshConRads[MAX_OFFMESH_CONNECTIONS];
		unsigned char		m_offMeshConDirs[MAX_OFFMESH_CONNECTIONS];
		unsigned char		m_offMeshConAreas[MAX_OFFMESH_CONNECTIONS];
		unsigned short		m_offMeshConFlags[MAX_OFFMESH_CONNECTIONS];
		unsigned int		m_offMeshConId[MAX_OFFMESH_CONNECTIONS];
		int					m_offMeshConCount;
	};
}
