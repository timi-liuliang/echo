#include "render/Texture.h"
#include "render/Renderer.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "render/PixelFormat.h"
#include "engine/core/Math/EchoMathFunction.h"
#include "engine/core/main/Root.h"
#include "TextureRes.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	// 构造函数
	TextureRes::TextureRes(const String& name, bool isManual)
		: Res(name)
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

	// 创建纹理
	TextureRes* TextureRes::createTexture(const String& name, Dword usage)
	{
		TextureRes* pTexture = EchoNew(TextureRes(name, false));
		if (pTexture)
		{
			pTexture->getTexture()->m_usage = usage;
		}

		return pTexture;
	}

	// 手动创建
	TextureRes* TextureRes::createManual(const String& name, Texture::TexType texType, PixelFormat format, Dword usage, ui32 width, ui32 height, ui32 depth, int num_mips, const Buffer& buff)
	{
		TextureRes* textureRes = EchoNew(TextureRes(name, true));
		Texture *pTexture = (Texture*)textureRes->getTexture();
		pTexture->m_texType = texType;
		pTexture->m_pixFmt = format;
		pTexture->m_usage = usage;
		pTexture->m_width = width;
		pTexture->m_height = height;
		pTexture->m_depth = depth;
		pTexture->m_numMipmaps = num_mips;

		if (pTexture->m_numMipmaps > Texture::MAX_MINMAPS)
		{
			pTexture->m_numMipmaps = Texture::MAX_MINMAPS;
			EchoLogWarning("Over the max support mipmaps, using the max mipmaps num.");
		}
		else
		{
			pTexture->m_numMipmaps = (num_mips > 0 ? num_mips : 1);
		}

		if (!pTexture->create2D(format, usage, width, height, num_mips, buff))
		{
			Renderer::instance()->releaseTexture(pTexture);
			pTexture = NULL;
		}
		else
		{
			pTexture->m_uploadedSize = pTexture->m_uploadedSize;
			if (Root::instance()->getEnableFrameProfile())
			{
				Root::instance()->frameState().incrUploadTextureSizeInBytes(pTexture->m_uploadedSize);
			}
		}

		return textureRes;
	}

	// 创建立方体贴图
	TextureRes* TextureRes::createTextureCubeFromFiles(const String& x_posi_name, const String& x_nega_name, const String& y_posi_name, const String& y_nage_name, const String& z_posi_name, const String& z_nega_name, Dword usage)
	{
		String name = x_posi_name;
		TextureRes* pTextureRes = EchoNew(TextureRes(name, true));
		Texture*    pTexture = pTextureRes->getTexture();
		if (pTexture)
		{
			pTexture->m_usage = usage;
			pTexture->m_texType = Texture::TT_CUBE;
			pTexture->m_isCubeMap = true;
			pTexture->m_surfaceFilename[Texture::CF_Positive_X] = x_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_X] = x_nega_name;
			pTexture->m_surfaceFilename[Texture::CF_Positive_Y] = y_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_Y] = y_nage_name;
			pTexture->m_surfaceFilename[Texture::CF_Positive_Z] = z_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_Z] = z_nega_name;
		}

		return pTextureRes;
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

	// 加载资源
	void TextureRes::prepareLoad()
	{
		if (!isLoaded())
		{
			DataStream* stream = IO::instance()->open(m_path.getPath());
			if (stream)
			{
				m_texture->load(stream);
				m_texture->loadToGPU();

				m_isLoaded = true;
			}
		}
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

	// 设置是否保留CPU图片数据
	void TextureRes::setRetainPreparedData(bool setting)
	{
		m_texture->setRetainPreparedData(setting);
	}

	void TextureRes::setPixelsByArea(ui32 inBeginX, ui32 inBeginY, ui32 inEndX, ui32 inEndY, ui32 outPosX, ui32 outPosY, Texture* pTexture)
	{
		m_texture->setPixelsByArea(inBeginX, inBeginY, inEndX, inEndY, outPosX, outPosY, pTexture);
	}
}

