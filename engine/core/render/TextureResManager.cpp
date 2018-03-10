#include "TextureResManager.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/LogManager.h"
#include "Engine/core/main/Root.h"
#include "render/PixelFormat.h"

namespace Echo
{
	__ImplementSingleton(TextureResManager);

	// 构造函数
	TextureResManager::TextureResManager()
	{
		__ConstructSingleton;
	}

	// 析构函数
	TextureResManager::~TextureResManager()
	{
		__DestructSingleton;
		// subclasses should unregister with resource group manager
	}

	// 创建纹理资源
	TextureRes* TextureResManager::createTexture(const String& name, Dword usage)
	{
		TextureRes* pTexture = ECHO_DOWN_CAST<TextureRes*>(ResourceManager::createResource(name, false));
		if(pTexture)
		{
			pTexture->getTexture()->m_usage = usage;
		}

		return pTexture;
	}

	TextureRes* TextureResManager::createTextureCubeFromFiles(const String& x_posi_name,const String& x_nega_name,const String& y_posi_name,
		const String& y_nage_name,const String& z_posi_name,const String& z_nega_name, Dword usage)
	{
		String name = x_posi_name;
		TextureRes* pTextureRes =ECHO_DOWN_CAST<TextureRes*>(ResourceManager::createResource(name, false));
		Texture*    pTexture = pTextureRes->getTexture();
		if(pTexture)
		{
			pTexture->m_usage = usage;
			pTexture->m_texType = Texture::TT_CUBE;
			pTexture->m_bUploadFromFiles = true;
			pTexture->m_surfaceFilename[Texture::CF_Positive_X] = x_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_X] = x_nega_name;
			pTexture->m_surfaceFilename[Texture::CF_Positive_Y] = y_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_Y] = y_nage_name;
			pTexture->m_surfaceFilename[Texture::CF_Positive_Z] = z_posi_name;
			pTexture->m_surfaceFilename[Texture::CF_Negative_Z] = z_nega_name;
		}

		return pTextureRes;
	}

	TextureRes* TextureResManager::createManual(const String& name, Texture::TexType texType, PixelFormat format, Dword usage,
		ui32 width, ui32 height, ui32 depth, int numMipmaps, const Buffer& buff)
	{
		TextureRes* textureRes = ECHO_DOWN_CAST<TextureRes*>( createNewResource(name, true));
		Texture *pTexture = (Texture*)textureRes->getTexture();
		pTexture->m_texType = texType;
		pTexture->m_pixFmt = format;
		pTexture->m_usage = usage;
		pTexture->m_width = width;
		pTexture->m_height = height;
		pTexture->m_depth = depth;
		pTexture->m_numMipmaps = numMipmaps;

		if(pTexture->m_numMipmaps > Texture::MAX_MINMAPS)
		{
			pTexture->m_numMipmaps = Texture::MAX_MINMAPS;
			EchoLogWarning("Over the max support mipmaps, using the max mipmaps num.");
		}
		else
		{
			pTexture->m_numMipmaps = (numMipmaps > 0? numMipmaps : 1);
		}

		if(!pTexture->create2D(format, usage, width, height, numMipmaps, buff))
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

	// 创建纹理实现
	Resource* TextureResManager::createImpl(const String& name, bool isManual)
	{
		// 创建TextureRes, TextureRes再创建Texture
		return EchoNew(TextureRes(name, isManual));
	}
}