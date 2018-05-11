#include "render/Texture.h"
#include "render/Renderer.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "TextureResManager.h"
#include "render/PixelFormat.h"
#include "engine/core/Math/EchoMathFunction.h"

namespace Echo
{
	// 构造函数
	TextureRes::TextureRes(const String& name, bool isManual)
		: Resource(name, RT_Texture, isManual)
	{
		m_texture = Renderer::instance()->createTexture(name);
		m_texture->setUserData(this);
	}

	// 析构函数
	TextureRes::~TextureRes()
	{
		if (m_texture)
		{
			Renderer::instance()->releaseTexture(m_texture);
		}
	}

	// 计算纹理大小
	size_t TextureRes::calculateSize() const
	{
		// need repaird
		return  m_texture->calculateSize();
	}

	// 更新纹理数据
	bool TextureRes::updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size)
	{
		return m_texture->updateSubTex2D(level, rect, pData, size);
	}

	// 获取纹理数据
	bool TextureRes::getData(Byte*& data)
	{
		return m_texture->getData(data);
	}

	// 重建纹理
	bool TextureRes::reCreate2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff)
	{
		m_texture->reCreate2D(pixFmt, usage, width, height, numMipmaps, buff);

		return true;
	}

	bool TextureRes::prepareImpl( DataStream* stream)
	{
		return m_texture->load( stream);
	}

	bool TextureRes::loadImpl()
	{
		return m_texture->loadToGPU();
	}

	// 设置是否保留CPU图片数据
	void TextureRes::setRetainPreparedData(bool setting)
	{
		m_texture->setRetainPreparedData(setting);
	}

	void TextureRes::unprepareImpl()
	{	
		//m_texture->unloadFromMemory();
	}

	// 卸载
	void TextureRes::unloadImpl()
	{	
		m_texture->unload();
	}

	void TextureRes::setPixelsByArea(ui32 inBeginX, ui32 inBeginY, ui32 inEndX, ui32 inEndY, ui32 outPosX, ui32 outPosY, Texture* pTexture)
	{
		m_texture->setPixelsByArea(inBeginX, inBeginY, inEndX, inEndY, outPosX, outPosY, pTexture);
	}
}

