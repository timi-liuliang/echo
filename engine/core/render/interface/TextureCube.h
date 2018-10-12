#pragma once

#include <engine/core/util/Array.hpp>
#include "Texture.h"

namespace Echo
{
	class TextureCube : public Texture
	{
		ECHO_RES(TextureCube, Texture, ".cube", TextureCube::create, Res::load)

	public:
		TextureCube();
		TextureCube(const String& name);
		virtual ~TextureCube();

		// create fun
		static Res* create();

	public:
		// positive x
		void setPositiveX(const ResourcePath& path);
		const ResourcePath& getPositiveX() { return m_surfaces[0]; }

		// positive x
		void setNegativeX(const ResourcePath& path);
		const ResourcePath& getNegativeX() { return m_surfaces[1]; }

		// positive x
		void setPositiveY(const ResourcePath& path);
		const ResourcePath& getPositiveY() { return m_surfaces[2]; }

		// positive x
		void setNegativeY(const ResourcePath& path);
		const ResourcePath& getNegativeY() { return m_surfaces[3]; }

		// positive x
		void setPositiveZ(const ResourcePath& path);
		const ResourcePath& getPositiveZ() { return m_surfaces[4]; }

		// positive x
		void setNegativeZ(const ResourcePath& path);
		const ResourcePath& getNegativeZ() { return m_surfaces[5]; }

	protected:
		array<ResourcePath, 6>	m_surfaces;
	};
}