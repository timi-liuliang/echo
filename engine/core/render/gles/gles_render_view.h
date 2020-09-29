#pragma once

#include "base/RenderView.h"

namespace Echo
{
	class GLESRenderView : public RenderView
	{
	public:
        GLESRenderView(ui32 width, ui32 height, PixelFormat pixelFormat);
		virtual ~GLESRenderView();

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;

	private:
		// create
		bool create();
	};
}
