#include <engine/core/io/IO.h>
#include <engine/core/io/stream/DataStream.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/base/echo_def.h>
#include "base/texture.h"
#include "base/renderer.h"
#include "engine/core/log/Log.h"
#include "engine/core/math/Function.h"
#include "image/pixel_format.h"
#include "image/image.h"
#include "image/texture_loader.h"
#include <iostream>

namespace Echo
{
	static map<ui32, Texture*>::type	g_globalTextures;

	Texture::Texture()
	{

	}

	Texture::Texture(const String& name)
		: Res(name)
	{
	}

	Texture::~Texture()
	{
	}

	void Texture::bindMethods()
	{
		CLASS_BIND_METHOD(Texture, isMipmapEnable, DEF_METHOD("isMipmapEnable"));
		CLASS_BIND_METHOD(Texture, setMipmapEnable, DEF_METHOD("setMipmapEnable"));
		CLASS_BIND_METHOD(Texture, getWidth, DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(Texture, setWidth, DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(Texture, getHeight, DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(Texture, setHeight, DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(Texture, "MipMap", Variant::Type::Bool, "isMipmapEnable", "setMipmapEnable");
		CLASS_REGISTER_PROPERTY(Texture, "Width",  Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(Texture, "Height", Variant::Type::Int, "getHeight", "setHeight");
	}

	Res* Texture::load(const ResourcePath& path)
	{
		if (IO::instance()->isExist(path.getPath()))
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

    const SamplerState* Texture::getSamplerState()
    {
        if(!m_samplerState)
        {
            SamplerState::SamplerDesc desc;
            setSamplerState(desc);
        }
        
        return m_samplerState;
    }

	void Texture::setSamplerState(const SamplerState::SamplerDesc& desc)
	{
		m_samplerState = Renderer::instance()->createSamplerState(desc);
	}

	size_t Texture::calculateSize() const
	{
		return (size_t)PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
	}
}
