#pragma once

#include "physx_shape.h"

namespace Echo
{
	class PhysxShapeHeightfield : public PhysxShape
	{
		ECHO_CLASS(PhysxShapeHeightfield, PhysxShape)

	public:
		PhysxShapeHeightfield();
		virtual ~PhysxShapeHeightfield();

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
		float					m_heightRange = 256.f;
	};
}