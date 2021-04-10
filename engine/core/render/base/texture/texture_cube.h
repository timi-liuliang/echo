#pragma once

#include <engine/core/util/Array.hpp>
#include "../texture.h"

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

		// type
		virtual TexType getType() const override { return TT_Cube; }

	public:
		// surface positive x
		void setPositiveX(const ResourcePath& path);
		const ResourcePath& getPositiveX() { return m_surfaces[0]; }

		// surface negative x
		void setNegativeX(const ResourcePath& path);
		const ResourcePath& getNegativeX() { return m_surfaces[1]; }

		// surface positive y
		void setPositiveY(const ResourcePath& path);
		const ResourcePath& getPositiveY() { return m_surfaces[2]; }

		// surface negative y
		void setNegativeY(const ResourcePath& path);
		const ResourcePath& getNegativeY() { return m_surfaces[3]; }

		// surface positive z
		void setPositiveZ(const ResourcePath& path);
		const ResourcePath& getPositiveZ() { return m_surfaces[4]; }

		// surface negative z
		void setNegativeZ(const ResourcePath& path);
		const ResourcePath& getNegativeZ() { return m_surfaces[5]; }

	protected:
		array<ResourcePath, 6>	m_surfaces;
	};
}
