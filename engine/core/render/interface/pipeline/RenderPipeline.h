#pragma once

#include "engine/core/scene/node.h"
#include "../FrameBuffer.h"
#include "../Renderer.h"

namespace Echo
{
	class RenderPipeline : public Node
	{
		ECHO_CLASS(RenderPipeline, Node)

	public:
		enum RenderTargetID
		{
			RTI_DefaultBackBuffer,
			RTI_End
		};

	public:
		RenderPipeline();
		~RenderPipeline();

		// inst
		static RenderPipeline* instance();

		// render target operate
		bool beginFramebuffer(ui32 id, bool clearColor = true, const Color& bgColor = Renderer::BGCOLOR, bool clearDepth = true, float depthValue = 1.0f, bool clearStencil = false, ui8 stencilValue = 0, ui32 rbo = 0xFFFFFFFF);
		bool endFramebuffer(ui32 id);

		// on Resize
		void onSize(ui32 width, ui32 height);

	private:
		FramebufferMap			m_framebuffers;
	};
}
