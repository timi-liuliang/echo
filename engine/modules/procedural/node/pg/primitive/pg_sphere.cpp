#include "pg_sphere.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PGSphere::PGSphere()
	{

	}

	PGSphere::~PGSphere()
	{

	}

	void PGSphere::bindMethods()
	{

	}

	void PGSphere::play(PCGData& data)
	{
		float x, y, z, xz;                              // vertex position
		float nx, ny, nz, lengthInv = 1.0f / m_radius;    // vertex normal
		float s, t;                                     // vertex texCoord

		float sectorStep = 2 * Math::PI / m_sectorCount;
		float stackStep = Math::PI / m_stackCount;
		float sectorAngle, stackAngle;

		vector<PCGPoint*>::type points;
		for (int i = 0; i <= m_stackCount; ++i)
		{
			stackAngle = Math::PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
			xz = m_radius * cosf(stackAngle);             // r * cos(u)
			y = m_radius * sinf(stackAngle);              // r * sin(u)

			// add (sectorCount+1) vertices per stack
			// the first and last vertices have same position and normal, but different tex coords
			for (int j = 0; j <= m_sectorCount; ++j)
			{
				sectorAngle = j * sectorStep;           // starting from 0 to 2pi

				PCGPoint* point = data.addPoint();

				// vertex position (x, y, z)
				x = xz * cosf(sectorAngle);             // r * cos(u) * cos(v)
				z = xz * sinf(sectorAngle);             // r * cos(u) * sin(v)
				point->m_position = Vector3(x, y, z);

				// normalized vertex normal (nx, ny, nz)
				nx = x * lengthInv;
				ny = y * lengthInv;
				nz = z * lengthInv;
				point->m_normal = Vector3(nx, ny, nz);

				// vertex texture coordinate (s, t) range between [0, 1]
				s = (float)j / m_sectorCount;
				t = (float)i / m_stackCount;
				point->m_uv = Vector2(s, t);

				points.push_back(point);
			}
		}

		// indices
		//  k1--k1+1
		//  |  / |
		//  | /  |
		//  k2--k2+1
		unsigned int k1, k2;
		for (int i = 0; i < m_stackCount; ++i)
		{
			k1 = i * (m_sectorCount + 1);     // beginning of current stack
			k2 = k1 + m_sectorCount + 1;      // beginning of next stack

			for (int j = 0; j < m_sectorCount; ++j, ++k1, ++k2)
			{
				// 2 triangles per sector excluding 1st and last stacks
				if (i != 0)
				{
					PCGPrimitive* prim= data.addPrimitive();
					prim->addPoint(points[k1]);
					prim->addPoint(points[k2]);
					prim->addPoint(points[k1+1]);
				}

				if (i != (m_stackCount - 1))
				{
					PCGPrimitive* prim = data.addPrimitive();
					prim->addPoint(points[k1+1]);
					prim->addPoint(points[k2]);
					prim->addPoint(points[k2 + 1]);
				}
			}
		}

		m_dirtyFlag = false;
	}
}
