#ifndef __ECHO_PIXELFILL_H__
#define __ECHO_PIXELFILL_H__

#include <engine/core/Math/EchoMath.h>
#include "engine/core/render/render/RenderDef.h"
#include "Engine/core/Render/TextureRes.h"

namespace Echo
{
	/************************************************************************/
	/* 贴图像素填充，通过一个配置文件控制                                   */
	/************************************************************************/
	class PixeFill
	{
	public:
		// 元素结构体
		struct PixeFillElement
		{
			String	id;
			Vector4	inArea;
			Vector2 outPoint;
			String	inTexture;

			PixeFillElement()
				: id(StringUtil::BLANK)
				, inArea(0.f,0.f,0.f,0.f)
				, outPoint(Vector2::ZERO)
				, inTexture(StringUtil::BLANK)
			{

			}
		};

		// map定义
		typedef map<String,PixeFillElement*>::type	PixeFillElementMap;
		typedef PixeFillElementMap::iterator		PixeFillElementMapIt;

		typedef map<String,TextureRes*>::type		TextureMap;
		typedef TextureMap::iterator				TextureMapIt;

	public:
		PixeFill(TextureRes* baseTexture);
		~PixeFill();

	public:
		// 磁盘文件的ＩＯ
		bool LoadCFGFile(const String& cfgFile);
		void SaveCFGFile(const String& fileName);

		// 通过ID填充
		void PixeFillByID(const String& id);

	protected:
		// 加载贴图
		void loadTexture(const String& fileName);
		void freeTextures();

	private:
		PixeFillElementMap mElementMap;	  // 元素MAP
		TextureMap         mTextureMap;	  // 贴图MAP
		TextureRes*        mpBaseTexture; // 底图
	};
}

#endif