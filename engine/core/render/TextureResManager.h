#ifndef __ECHO_TEXTUREMANAGER_H__
#define __ECHO_TEXTUREMANAGER_H__

#include <engine/core/render/render/Texture.h>
#include <Engine/core/Resource/ResourceManager.h>
#include "Engine/core/Render/TextureRes.h"
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	/**
	 * 纹理管理器
	 */
	class TextureRes;
	class TextureResManager : public ResourceManager
	{
		__DeclareSingleton(TextureResManager);

	public:
		TextureResManager();
		virtual ~TextureResManager();

		// 创建纹理
		virtual TextureRes* createTexture(const String& name, Dword usage = Texture::TU_STATIC);
		
		// 手动创建
		virtual TextureRes* createManual(const String& name, Texture::TexType texType, PixelFormat format, Dword usage, ui32 width, ui32 height, ui32 depth, int num_mips, const Buffer& buff);
		
		// 创建立方体贴图
		virtual TextureRes* createTextureCubeFromFiles( const String& x_posi_name,const String& x_nega_name,const String& y_posi_name, const String& y_nage_name,const String& z_posi_name,const String& z_nega_name,Dword usage = Texture::TU_STATIC);
	
		// 创建纹理实现
		Resource* createImpl(const String& name, bool isManual);
	};
}

#endif