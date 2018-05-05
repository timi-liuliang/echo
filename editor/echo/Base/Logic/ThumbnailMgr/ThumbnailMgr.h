#pragma once

#include <Engine/Core.h>
#include <engine/core/Util/StringUtil.h>

namespace Studio
{
	/**
	 * 缩略图管理器
	 */
	class ThumbnailMgr
	{
	public:
		// 缩略图类型
		enum THUMBNAIL_TYPE
		{
			THUMB_NON,
			THUMB_SCENE,
			THUMB_MODEL,
			THUMB_EFFECT,
			THUMB_ACTOR,
			THUMB_ATT,
			THUMB_NUM,
		};

	public:
		ThumbnailMgr();
		virtual ~ThumbnailMgr();

		// instance
		static ThumbnailMgr* instance();

		// 保存缩略图
		bool saveThumbnail( const Echo::String& fileName, THUMBNAIL_TYPE type = THUMB_NON);

		// 更新缩略图
		static void update();
	};
}
