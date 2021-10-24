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
		// Default path name
		static const ResourcePath DefaultPipeline;

	public:
		RenderPipeline();
		RenderPipeline(const ResourcePath& path);
		virtual ~RenderPipeline();

		// add render able
		void addRenderable(const String& name, RenderableID id);

		// on Resize
		void onSize(ui32 width, ui32 height);

		// process
		void render();

	public:
		// current
		static ResRef<RenderPipeline> current();
		static void setCurrent(const ResourcePath& path);

		// Set src
		void setSrc(const String& src);

	public:
		// stages
		vector<RenderStage*>::type& getRenderStages() { return m_stages; }

		// stage operate
		void addStage(RenderStage* stage, ui32 position=-1);
		void deleteStage(RenderStage* stage);

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
		vector<RenderStage*>::type	m_stages;
	};
	typedef ResRef<RenderPipeline> RenderPipelinePtr;
}
