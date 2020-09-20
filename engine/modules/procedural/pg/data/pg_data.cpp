#include "pg_data.h"

namespace Echo
{
	void PCGData::clear()
	{
		m_pointIdx = 0;
		EchoSafeDeleteContainer(m_points, PGPoint);

		m_points.clear();
		m_vertices.clear();

		m_primitiveIdx = 0;
		EchoSafeDeleteContainer(m_primitives, PGPrimitive);
	}

	PGPoint* PCGData::addPoint()
	{
		m_pointIdx++;

		PGPoint* point = new PGPoint;
		point->m_id = m_pointIdx++;
		m_points.push_back(point);

		return point;
	}

	PGPrimitive* PCGData::addPrimitive()
	{
		m_primitiveIdx++;

		PGPrimitive* prim = new PGPrimitive(m_primitiveIdx++);
		m_primitives.push_back(prim);

		return prim;
	}

	MeshPtr PCGData::buildMesh()
	{
		// Vertex Format
		struct VertexFormat
		{
			Vector3        m_position;
			Vector3        m_normal;
			Vector2        m_uv;
		};

		typedef vector<VertexFormat>::type  VertexArray;
		typedef vector<ui32>::type          IndiceArray;

		VertexArray    vertices;
		IndiceArray    indices;

		MeshPtr mesh = Mesh::create(true, true);

		MeshVertexFormat define;
		define.m_isUseNormal = true;
		define.m_isUseUV = true;

		for (PGPrimitive* primitive : m_primitives)
		{
			for (PGPoint* point : primitive->getPoints())
			{
				indices.push_back(vertices.size());

				VertexFormat vertex;
				vertex.m_position = point->m_position;
				vertex.m_normal   =	point->m_normal;
				vertex.m_uv		  = point->m_uv;

				vertices.push_back(vertex);
			}
		}

		mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
		mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

		return mesh;
	}
}