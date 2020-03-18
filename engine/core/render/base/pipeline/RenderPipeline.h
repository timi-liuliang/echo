#pragma once

#include "engine/core/scene/node.h"
#include "../FrameBuffer.h"
#include "../Renderer.h"

namespace Echo
{
	class RenderPipeline : public Res
	{
		ECHO_RES(RenderPipeline, Res, ".pipeline", Res::create<RenderPipeline>, RenderPipeline::load);

	public:
		enum FrameBufferType
		{
			FB_Window = 0,
			FB_End
		};

	public:
		RenderPipeline();
		RenderPipeline(const ResourcePath& path);
		virtual ~RenderPipeline();

		// add renderable
		void addRenderable(const String& name, RenderableID id);

		// render target operate
		bool beginFramebuffer(ui32 id, bool clearColor = true, const Color& bgColor = Renderer::BGCOLOR, bool clearDepth = true, float depthValue = 1.0f, bool clearStencil = false, ui8 stencilValue = 0, ui32 rbo = 0xFFFFFFFF);
		bool endFramebuffer(ui32 id);

		// on Resize
		void onSize(ui32 width, ui32 height);

		// process
		void process();

	public:
		// current
		static ResRef<RenderPipeline> current();
		static void setCurrent(const ResourcePath& path);

	public:
		// load and save
		static Res* load(const ResourcePath& path);
		virtual void save() override;

	private:
		FramebufferMap			m_framebuffers;
	};
	typedef ResRef<RenderPipeline> RenderPipelinePtr;
}
