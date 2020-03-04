#include "procedural_sphere.h"
#include "node/primitive/pcg_sphere.h"

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
			m_mesh = PCGSphere::buildUvSphere(1.f, 20, 20);

			m_localAABB = m_mesh->getLocalBox();
		}
	}
}