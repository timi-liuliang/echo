#pragma once

#include "engine/core/resource/Res.h"
#include <engine/core/util/Buffer.h>
#include <engine/core/math/Rect.h>
#include <engine/core/io/stream/DataStream.h>
#include "render_state.h"
#include "image/pixel_format.h"

namespace Echo
{
	class Texture : public Res
	{
		ECHO_RES(Texture, Res, ".png|.jpeg|.bmp|.tga|.jpg", nullptr, Texture::load);

		friend class Renderer;
		friend class FrameBuffer;

	public:
		enum TexType
		{
			TT_Unknown,
			TT_1D,
			TT_2D,
			TT_3D,
			TT_Cube,
			TT_Render,
		};

		enum CubeFace
		{
			CF_Positive_X,
			CF_Negative_X,
			CF_Positive_Y,
			CF_Negative_Y,
			CF_Positive_Z,
			CF_Negative_Z,
			CF_End
		};

		enum TexUsage
		{
			TU_CPU_READ			= 0x00000001,
			TU_CPU_WRITE		= 0x00000002,
			TU_GPU_READ			= 0x00000004,	// texture is used as shader resource	
			TU_GPU_WRITE		= 0x00000008,	// texture is used as depth or render target (depend on pixel format)
			TU_GPU_UNORDERED	= 0x00000010,
			TU_GPU_MIPMAP		= 0x00000020,		
			TU_DEFAULT			= TU_GPU_READ | TU_GPU_WRITE | TU_GPU_MIPMAP,		// default to automatic mipmap generation static textures
			TU_STATIC			= TU_GPU_READ | TU_GPU_MIPMAP,
			TU_DYNAMIC			= TU_GPU_READ | TU_CPU_WRITE | TU_GPU_MIPMAP,
		};

		enum CompressType
		{
			CompressType_Unknown = 0,
		};

		static const ui32	MAX_MINMAPS = 16;

	public:
		Texture();

		// get|set global texture
		static Texture* getGlobal(ui32 globalTextureIdx);
		static void setGlobal(ui32 globalTextureIdx, Texture* texture);

		// type
		virtual TexType getType() const { return TT_Unknown; }

		// pixel format
		PixelFormat getPixelFormat() const{ return m_pixFmt; }

		// width
		virtual ui32 getWidth() const { return m_width; }
		virtual void setWidth(ui32 width) {}

		// height
		virtual ui32 getHeight() const{ return m_height; }
		virtual void setHeight(ui32 height) {}

		// depth
		ui32 getDepth() const { return m_depth; }

		// mip map
		bool isMipmapEnable() const { return m_isMipMapEnable; }
		void setMipmapEnable(bool isEanble) { m_isMipMapEnable = isEanble; }
		ui32 getNumMipmaps() const { return m_numMipmaps; }
		
		// sampler state
		void setSamplerState( const SamplerState::SamplerDesc& desc);
        const SamplerState* getSamplerState();

		// get compress type
		ui32 getCompressType() { return m_compressType; }

		// calc size
		virtual size_t	calculateSize() const;

		// load operate
		virtual bool load() { return false; }

	protected:
		Texture(const String& name);
		virtual ~Texture();

	protected:
		// static load
		static Res* load(const ResourcePath& path);

	protected:
		PixelFormat			m_pixFmt = PF_UNKNOWN;
		bool				m_isCompressed = false;
		ui32				m_compressType = CompressType_Unknown;
		TexUsage			m_usage = TU_DYNAMIC;
		ui32				m_width = 0;
		ui32				m_height = 0;
		ui32				m_depth = 1;
		bool				m_isMipMapEnable = true;
		ui32				m_numMipmaps = 1;
		ui32				m_faceNum = 1;
		ui32				m_blockSize = 0;
		ui32				m_xDimension = 0;
		ui32				m_yDimension = 0;
		ui32				m_zDimension = 0;
		ui32				m_surfaceNum;
		const SamplerState*	m_samplerState = nullptr;
	};
	typedef ResRef<Texture> TexturePtr;
}
