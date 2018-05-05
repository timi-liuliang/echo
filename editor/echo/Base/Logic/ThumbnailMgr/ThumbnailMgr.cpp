#include "ThumbnailMgr.h"
#include "FreeImageHelper.h"
#include <Engine/core/Render/RenderTargetManager.h>
#include <engine/core/Util/StringUtil.h>
#include "Studio.h"
#include <shellapi.h>

namespace Studio
{
	// 构造函数
	ThumbnailMgr::ThumbnailMgr()
	{}

	// 析构函数
	ThumbnailMgr::~ThumbnailMgr()
	{}

	// instance
	ThumbnailMgr* ThumbnailMgr::instance()
	{
		static ThumbnailMgr* inst = new ThumbnailMgr;
		return inst;
	}

	// 保存缩略图
	bool ThumbnailMgr::saveThumbnail( const Echo::String& fileName, THUMBNAIL_TYPE type /* = THUMB_NON */ )
	{
		Echo::String WaterMarkType[ThumbnailMgr::THUMB_NUM] = 
		{ 
			"", 
			AStudio::instance()->getAppPath() + Echo::String( "assets/watermark/scene.bmp"), 
			AStudio::instance()->getAppPath() + Echo::String( "assets/watermark/att.bmp"),														
		};

		Echo::RenderTarget* defauldBackTarget = Echo::RenderTargetManager::instance()->getRenderTargetByID( Echo::RTI_DefaultBackBuffer);
		if ( defauldBackTarget)
		{
			Echo::String appPath   = AStudio::instance()->getAppPath();
			Echo::String ofileName = Echo::StringUtil::Format( "%sCache/thumbnail/%s.bmp", appPath.c_str(), fileName.c_str() ) ;
			defauldBackTarget->saveTo( ofileName.c_str() );

			// 添加水印
			Echo::FreeImageHelper::addWaterMark( ofileName.c_str(), WaterMarkType[type].c_str() );

			return true;
		}

		return false;
	}

	// 更新缩略图
	void ThumbnailMgr::update()
	{
		//Echo::String project = EchoRoot->getConfig().projectFile;

		//生成缩略图
		//ShellExecute(0, "open", "Thumbnail.exe", project.c_str(), "", SW_HIDE);
	}
}
