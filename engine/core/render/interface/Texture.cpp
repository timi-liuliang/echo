#include <engine/core/io/IO.h>
#include <engine/core/io/DataStream.h>
#include <engine/core/Util/PathUtil.h>
#include <engine/core/Base/EchoDef.h>
#include "interface/Texture.h"
#include "interface/Renderer.h"
#include "engine/core/log/Log.h"
#include "engine/core/Math/EchoMathFunction.h"
#include "image/PixelFormat.h"
#include "image/Image.h"
#include "image/TextureLoader.h"
#include <iostream>

namespace Echo
{
	static map<ui32, Texture*>::type	g_globalTextures;

	Texture::Texture(const String& name)
		: Res(name)
		, m_texType(TT_2D)
		, m_pixFmt(PF_UNKNOWN)
		, m_usage(TU_DYNAMIC)
		, m_width(0)
		, m_height(0)
		, m_depth(1)
		, m_numMipmaps(1)
		, m_pixelsSize(0)
		, m_compressType(CompressType_Unknown)
		, m_bCompressed(false)
		, m_faceNum(1)
		, m_endian(0)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_bitsPerPixel(0)
		, m_blockSize(0)
		, m_headerSize(0)
		, m_uploadedSize(0)
		, m_samplerState(NULL)
	{
	}

	Texture::Texture(TexType texType, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 depth, ui32 numMipmaps, const Buffer& buff, bool bBak)
		: m_texType(texType)
		, m_pixFmt(pixFmt)
		, m_usage(usage)
		, m_width(width)
		, m_height(height)
		, m_depth(depth)
		, m_numMipmaps(numMipmaps)
		, m_pixelsSize(0)
		, m_compressType(CompressType_Unknown)
		, m_bCompressed(false)
		, m_faceNum(1)
		, m_endian(0)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_bitsPerPixel(0)
		, m_blockSize(0)
		, m_headerSize(0)
		, m_uploadedSize(0)
		, m_samplerState(NULL)
	{
		if (numMipmaps > MAX_MINMAPS)
		{
			m_numMipmaps = MAX_MINMAPS;
			EchoLogWarning("Over the max support mipmaps, using the max mipmaps num.");
		}
		else
		{
			m_numMipmaps = (numMipmaps > 0 ? numMipmaps : 1);
		}
	}

	Texture::~Texture()
	{
	}

	void Texture::bindMethods()
	{

	}

	Res* Texture::load(const ResourcePath& path)
	{
		if (IO::instance()->isResourceExists(path.getPath()))
		{
			return Renderer::instance()->createTexture2D(path.getPath());
		}

		return nullptr;
	}

	Texture* Texture::getGlobal(ui32 globalTextureIdx)
	{
		auto it = g_globalTextures.find(globalTextureIdx);
		if (it != g_globalTextures.end())
			return it->second;

		return nullptr;
	}

	// set global texture
	void Texture::setGlobal(ui32 globalTextureIdx, Texture* texture)
	{
		g_globalTextures[globalTextureIdx] = texture;
	}

	bool Texture::create2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		return false;
	}

	void Texture::setSamplerState(const SamplerState::SamplerDesc& desc)
	{
		m_samplerState = Renderer::instance()->getSamplerState(desc);
	}

	size_t Texture::calculateSize() const
	{
		return (size_t)PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
	}
}