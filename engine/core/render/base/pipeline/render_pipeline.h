#pragma once

#include "engine/core/scene/node.h"
#include "../frame_buffer.h"
#include "../renderer.h"

namespace Echo
{
	class RenderStage;
	class RenderPipeline : public Res
	{
		ECHO_RES(RenderPipeline, Res, ".pipeline", Res::create<RenderPipeline>, RenderPipeline::load);

	public:
		// template
		enum Template
		{
			Empty,
			Default,
		};

	public:
		RenderPipeline();
		RenderPipeline(const ResourcePath& path);
		virtual ~RenderPipeline();

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// render target operate
		bool beginFramebuffer(ui32 id, bool clearColor = true, const Color& bgColor = Renderer::BGCOLOR, bool clearDepth = true, float depthValue = 1.0f, bool clearStencil = false, ui8 stencilValue = 0, ui32 rbo = 0xFFFFFFFF);
		bool endFramebuffer(ui32 id);

		// on Resize
		void onSize(ui32 width, ui32 height);

		// process
		void render();

	public:
		// current
		static ResRef<RenderPipeline> current();
		static void setCurrent(const ResourcePath& path);

		// set src
		void setSrc(Template type);
		void setSrc(const String& src);

		// stages
		const vector<RenderStage*>::type& getRenderStages() { return m_renderStages; }

	public:
		// load and save
		static Res* load(const ResourcePath& path);
		virtual void save() override;

	private:
		// parse
		void parseXml();

	private:
		String						m_srcData;
		bool						m_isParsed = false;
		FramebufferMap				m_framebuffers;
		vector<RenderStage*>::type	m_renderStages;
	};
	typedef ResRef<RenderPipeline> RenderPipelinePtr;
}
