#pragma once

#include <interface/TextureCube.h>

namespace Echo
{
	class GLESTextureCube: public TextureCube
	{
		friend class GLES2Renderer;

	protected:
		GLESTextureCube(const String& name);
		GLESTextureCube(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak = true);
		virtual ~GLESTextureCube();

		// load
		virtual bool load() override;

		// unload
		virtual bool unload();

	protected:
		// create cube texture
		void createCubeTexture();

		// set surface data
		void setCubeSurfaceData(int face, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

	public:
		GLuint		m_glesTexture;
	};
}
