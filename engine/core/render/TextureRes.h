#pragma once

#include "Engine/core/Resource/Resource.h"
#include "engine/core/Util/Buffer.h"
#include <engine/core/render/render/Texture.h>

namespace Echo
{
	/**
	 * 纹理封装
	 */
	class TextureRes: public Resource
	{
	public:
		enum MASK
		{
			MASK_NONE = 0,
			MASK_ALPHA = 1,
			MASK_IMAGE = 2,
		};

	private:
		friend struct commonTextureUploadFunc;
		friend class RenderTarget;
		friend class TextureResManager;
		friend struct TexturePrepareFunc;
		typedef vector<Buffer>::type TexArray;

	public:
		TextureRes(const String& name, bool isManual);
		virtual ~TextureRes();

		// 获取纹理
		Texture* getTexture() { return m_texture;  }

		void setTexture(Texture* texture) { m_texture = texture; }

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

		// 准备实现
		virtual bool prepareImpl( DataStream* stream);

		// 卸载实现 
		virtual void unprepareImpl();

		// 加载
		virtual bool loadImpl();

		// 卸载
		virtual void unloadImpl();

	protected:
		Texture*		m_texture;
	};
}
