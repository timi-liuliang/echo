#include "pcg_mesh.h"
#include "pcg_vertex.h"

namespace Echo
{
	void PCGMesh::clear()
	{
		m_vertexIdx = 0;
		EchoSafeDeleteContainer(m_vertices, PCGVertex);

		m_primitiveIdx = 0;
		EchoSafeDeleteContainer(m_primitives, PCGPrimitive);
	}

	PCGVertex* PCGMesh::addVertex()
	{
		m_vertexIdx++;

		PCGVertex* point = EchoNew(PCGVertex);
		point->m_id = m_vertexIdx++;
		m_vertices.push_back(point);

		return point;
	}

	PCGPrimitive* PCGMesh::addPrimitive()
	{
		m_primitiveIdx++;

		PCGPrimitive* prim = EchoNew(PCGPrimitive(m_primitiveIdx++));
		m_primitives.push_back(prim);

		return prim;
	}

	MeshPtr PCGMesh::buildMesh()
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

		MeshVertexFormat define;
		define.m_isUseNormal = true;
		define.m_isUseUV = true;

		for (PCGPrimitive* primitive : m_primitives)
		{
			for (PCGVertex* point : primitive->getVertices())
			{
				indices.push_back(vertices.size());

				VertexFormat vertex;
				vertex.m_position = point->m_position;
				vertex.m_normal   =	point->m_normal;
				vertex.m_uv		  = point->m_uv;

				vertices.push_back(vertex);
			}
		}

		if (!indices.empty() && !vertices.empty())
		{
			MeshPtr mesh = Mesh::create(true, true);
			mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
			mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			return mesh;
		}

		return nullptr;
	}
}