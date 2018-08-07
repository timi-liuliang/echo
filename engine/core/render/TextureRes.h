#pragma once

#include "engine/core/resource/Res.h"
#include "engine/core/util/Buffer.h"
#include <engine/core/render/render/Texture.h>

namespace Echo
{
	/**
	 * 纹理封装
	 */
	class TextureRes: public Res
	{
	public:
		enum MASK
		{
			MASK_NONE = 0,
			MASK_ALPHA = 1,
			MASK_IMAGE = 2,
		};

	private:
		friend class RenderTarget;

	public:
		TextureRes(const String& name, bool isManual);
		virtual ~TextureRes();

		// 创建纹理
		static TextureRes* createTexture(const String& name, Dword usage = Texture::TU_STATIC);

		// 手动创建
		static TextureRes* createManual(const String& name, Texture::TexType texType, PixelFormat format, Dword usage, ui32 width, ui32 height, ui32 depth, int num_mips, const Buffer& buff);

		// 创建立方体贴图
		static TextureRes* createTextureCubeFromFiles(const String& x_posi_name, const String& x_nega_name, const String& y_posi_name, const String& y_nage_name, const String& z_posi_name, const String& z_nega_name, Dword usage = Texture::TU_STATIC);

		// 加载资源
		void prepareLoad();

		// 获取纹理
		Texture* getTexture() { return m_texture;  }

		// 获取像素格式
		PixelFormat getPixelFormat() const{ return m_texture->getPixelFormat(); }

		// 获取宽度
		ui32 getWidth() const { return m_texture->getWidth(); }

		// 获取高度
		ui32 getHeight() const{ return m_texture->getHeight(); }

		// 编辑器使用
		virtual bool getData(Byte*& data);

		// 更新纹理数据
		bool updateSubTex2D(ui32 level, const Rect& rect, void* pData, ui32 size);

		// 重新创建纹理
		bool reCreate2D(PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, ui32 numMipmaps, const Buffer& buff);

		// 填充指定区域的像素
		void setPixelsByArea(ui32 inPosX,ui32 inPosY, ui32 outPosX, ui32 outPosY ,ui32 width, ui32 height,Texture* pTexture);

		// 设置是否保留CPU图片数据
		void setRetainPreparedData(bool setting);

	protected:
		// 计算尺寸
		virtual size_t	calculateSize() const;

	protected:
		Texture*		m_texture;
	};
	typedef ResRef<TextureRes> TextureResPtr;
}
