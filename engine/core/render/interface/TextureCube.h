#pragma once

#include "Texture.h"

namespace Echo
{
	class TextureCube : public Texture
	{
		ECHO_RES(TextureCube, Res, ".cube", Res::create<TextureCube>, Res::load)

	public:
		TextureCube();
		TextureCube(const String& name);
		virtual ~TextureCube();

	public:
		// positive x
		void setPositiveX(const ResourcePath& path);
		const ResourcePath& getPositiveX() { return m_textures[0]; }

		// positive x
		void setNegativeX(const ResourcePath& path);
		const ResourcePath& getNegativeX() { return m_textures[1]; }

		// positive x
		void setPositiveY(const ResourcePath& path);
		const ResourcePath& getPositiveY() { return m_textures[2]; }

		// positive x
		void setNegativeY(const ResourcePath& path);
		const ResourcePath& getNegativeY() { return m_textures[3]; }

		// positive x
		void setPositiveZ(const ResourcePath& path);
		const ResourcePath& getPositiveZ() { return m_textures[4]; }

		// positive x
		void setNegativeZ(const ResourcePath& path);
		const ResourcePath& getNegativeZ() { return m_textures[5]; }

	private:
		ResourcePath	m_textures[6];
	};
}