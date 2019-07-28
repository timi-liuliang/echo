#pragma once

#include "engine/core/scene/node.h"
#include "../RenderTarget.h"
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

	public:
		// render target operate
		bool beginRenderTarget(ui32 id, bool clearColor = true, const Color& bgColor = Renderer::BGCOLOR, bool clearDepth = true, float depthValue = 1.0f, bool clearStencil = false, ui8 stencilValue = 0, ui32 rbo = 0xFFFFFFFF);
		bool endRenderTarget(ui32 id);

		// on Resize
		void onSize(ui32 width, ui32 height);

	private:
		RenderTargetMap			m_renderTargets;
	};
}
