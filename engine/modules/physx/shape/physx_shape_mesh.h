#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeMesh : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeMesh, PhysxShape)

	public:
		PhysxShapeMesh();
		virtual ~PhysxShapeMesh();

		// font res path
		void setDataPath(const ResourcePath& path);
		const ResourcePath& getDataPath() { return m_dataPath; }

	protected:
		// create shape
		virtual physx::PxShape* createPxShape() override;

	private:
		ResourcePath            m_dataPath = ResourcePath("", "");
		ImagePtr				m_heightmapImage;
		physx::PxHeightField*	m_pxHeightField = nullptr;
	};
}