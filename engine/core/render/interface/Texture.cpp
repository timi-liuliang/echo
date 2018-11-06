#include <engine/core/io/IO.h>
#include <engine/core/io/DataStream.h>
#include <engine/core/Util/PathUtil.h>
#include <engine/core/Base/echo_def.h>
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

	Texture::Texture()
		: m_pixFmt(PF_UNKNOWN)
		, m_usage(TU_DYNAMIC)
		, m_width(0)
		, m_height(0)
		, m_depth(1)
		, m_numMipmaps(1)
		, m_compressType(CompressType_Unknown)
		, m_isCompressed(false)
		, m_faceNum(1)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_blockSize(0)
		, m_samplerState(NULL)
		, m_isMipMapEnable(false)
	{

	}

	Texture::Texture(const String& name)
		: Res(name)
		, m_pixFmt(PF_UNKNOWN)
		, m_usage(TU_DYNAMIC)
		, m_width(0)
		, m_height(0)
		, m_depth(1)
		, m_numMipmaps(1)
		, m_compressType(CompressType_Unknown)
		, m_isCompressed(false)
		, m_faceNum(1)
		, m_xDimension(0)
		, m_yDimension(0)
		, m_zDimension(0)
		, m_blockSize(0)
		, m_samplerState(NULL)
		, m_isMipMapEnable(false)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::bindMethods()
	{
		CLASS_BIND_METHOD(Texture, isMipmapEnable, DEF_METHOD("isMipmapEnable"));
		CLASS_BIND_METHOD(Texture, setMipmapEnable, DEF_METHOD("setMipmapEnable"));

		CLASS_REGISTER_PROPERTY(Texture, "MipMap", Variant::Type::Bool, "isMipmapEnable", "setMipmapEnable");
	}

	Res* Texture::load(const ResourcePath& path)
	{
		if (IO::instance()->isResourceExists(path.getPath()))
		{
			Texture* texture = Renderer::instance()->createTexture2D(path.getPath());
			texture->load();

			return texture;
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

	void Texture::setGlobal(ui32 globalTextureIdx, Texture* texture)
	{
		g_globalTextures[globalTextureIdx] = texture;
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