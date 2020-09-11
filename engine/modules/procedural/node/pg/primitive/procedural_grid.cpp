#include "procedural_grid.h"

namespace Echo
{
	ProceduralGird::ProceduralGird()
	{

	}

	ProceduralGird::~ProceduralGird()
	{

	}

	void ProceduralGird::bindMethods()
	{

	}

	void ProceduralGird::buildMesh()
	{
		ProceduralGeometry::buildMesh();

		if (!m_mesh)
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

			if (!m_mesh)
				m_mesh = Mesh::create(true, true);

			i32 columns = 11;
			i32 rows = 11;
			if (columns > 0 && rows > 0)
			{
				Vector3 basePosition(-0.5f * (rows - 1), 0.f, -0.5f * (columns - 1));

				// vertex buffer
				for (i32 row = 0; row < rows; row++)
				{
					for (i32 column = 0; column < columns; column++)
					{
						VertexFormat vert;
						vert.m_position = Vector3(row, 0.f, column) + basePosition;
						vert.m_uv = Vector2(row / (rows - 1), column / (columns - 1));
						vert.m_normal = Vector3::UNIT_Y;

						vertices.emplace_back(vert);
					}
				}

				// index buffer
				i32 rowLength = rows - 1;
				i32 columnLength = columns - 1;
				for (i32 row = 0; row < rowLength; row++)
				{
					for (i32 column = 0; column < columnLength; column++)
					{
						i32 indexLeftTop = row * columns + column;
						i32 indexRightTop = indexLeftTop + 1;
						i32 indexLeftBottom = indexLeftTop + columns;
						i32 indexRightBottom = indexRightTop + columns;

						indices.emplace_back(indexLeftTop);
						indices.emplace_back(indexRightBottom);
						indices.emplace_back(indexRightTop);
						indices.emplace_back(indexLeftTop);
						indices.emplace_back(indexLeftBottom);
						indices.emplace_back(indexRightBottom);
					}
				}
			}

			MeshVertexFormat define;
			define.m_isUseNormal = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

			// calculate aabb
			m_localAABB = m_mesh->getLocalBox();
		}
	}
}