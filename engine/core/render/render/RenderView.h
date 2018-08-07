#pragma once

#include <engine/core/memory/MemManager.h>
#include "PixelFormat.h"

namespace Echo
{
	class RenderView
	{
	public:
		enum RenderViewType
		{
			RVT_RENDERTARGET, 
			RVT_DEPTHSTENCIL, 
		};

	public:
		RenderView(RenderViewType type, PixelFormat pixFmt);
		virtual ~RenderView();

		// 获取渲染视界类型
		virtual RenderViewType getRenderViewType() const;

		// 获取像素类型
		virtual PixelFormat	getPixelFormat() const;

	protected:
		RenderViewType		m_type;
		PixelFormat			m_pixFmt;
	};
}
