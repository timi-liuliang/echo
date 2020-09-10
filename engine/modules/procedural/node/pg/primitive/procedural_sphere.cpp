#include "procedural_sphere.h"
#include "pg_sphere.h"

namespace Echo
{
	ProceduralSphere::ProceduralSphere()
	{

	}

	ProceduralSphere::~ProceduralSphere()
	{

	}

	void ProceduralSphere::bindMethods()
	{

	}

	void ProceduralSphere::buildMesh()
	{
		ProceduralGeometry::buildMesh();

		if (!m_mesh)
		{
			m_mesh = PGSphere::buildUvSphere(1.f, 50, 50);

			m_localAABB = m_mesh->getLocalBox();
		}
	}
}