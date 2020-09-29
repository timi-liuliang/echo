#pragma once

#include <base/TextureCube.h>

namespace Echo
{
	class GLESTextureCube: public TextureCube
	{
		friend class GLESRenderer;

	protected:
		GLESTextureCube();
		GLESTextureCube(const String& name);
		virtual ~GLESTextureCube();

		// load
		virtual bool load() override;

		// unload
		virtual bool unload();

	protected:
		// create cube texture
		void createCubeTexture();

		// set surface data
		void setCubeSurfaceData(int face, int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

	public:
		GLuint		m_glesTexture;
	};
}
