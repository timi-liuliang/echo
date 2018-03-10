#pragma once

#include "GLES2RenderTargetView.h"
#include "GLES2DepthStencilView.h"
#include "Render/FrameBuffer.h"

namespace LORD
{
	class LORD_GLES2RENDER_API GLES2FrameBuffer: public FrameBuffer
	{
	public:
		GLES2FrameBuffer();
		~GLES2FrameBuffer();
	};
}
