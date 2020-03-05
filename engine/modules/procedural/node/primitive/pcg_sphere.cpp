#include "pcg_sphere.h"
#include "engine/core/render/base/mesh/Mesh.h"

namespace Echo
{
	PCGSphere::PCGSphere()
	{

	}

	PCGSphere::~PCGSphere()
	{

	}

	Mesh* PCGSphere::buildUvSphere(float radius, i32 stackCount, i32 sectorCount)
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
	
		Mesh* mesh = Mesh::create(true, true);

		float x, y, z, xz;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = 2 * Math::PI / sectorCount;
		float stackStep = Math::PI / stackCount;
		float sectorAngle, stackAngle;

		for (int i = 0; i <= stackCount; ++i)
		{
			stackAngle = Math::PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xz = radius * cosf(stackAngle);             // r * cos(u)
			y = radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				VertexFormat vertex;

				// vertex position (x, y, z)
				x = xz * cosf(sectorAngle);             // r * cos(u) * cos(v)
				z = xz * sinf(sectorAngle);             // r * cos(u) * sin(v)
				vertex.m_position = Vector3(x, y, z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				vertex.m_normal = Vector3(nx, ny, nz);

				// vertex texture coordinate (s, t) range between [0, 1]
				s = (float)j / sectorCount;
				t = (float)i / stackCount;
				vertex.m_uv = Vector2(s, t);

				vertices.push_back(vertex);
			}
		}

		// indices
		//  k1--k1+1
		//  |  / |
		//  | /  |
		//  k2--k2+1
		unsigned int k1, k2;
		for (int i = 0; i < stackCount; ++i)
		{
			k1 = i * (sectorCount + 1);     // beginning of current stack
			k2 = k1 + sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding 1st and last stacks
				if (i != 0)
				{
					indices.push_back(k1);
					indices.push_back(k1 + 1);
					indices.push_back(k2);
				}

				if (i != (stackCount - 1))
				{
					indices.push_back(k1+1);
					indices.push_back(k2 + 1);
					indices.push_back(k2);
				}

				//// vertical lines for all stacks
				//indices.push_back(k1);
				//indices.push_back(k2);
				//if (i != 0)  // horizontal lines except 1st stack
				//{
				//	indices.push_back(k1);
				//	indices.push_back(k1 + 1);
				//}
			}
		}

		MeshVertexFormat define;
		define.m_isUseNormal = true;
		define.m_isUseUV = true;

		mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(ui32), indices.data());
		mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data());

		return mesh;
	}
}