#pragma once

#include "RenderStageItem.h"

namespace Echo
{
	class RenderStage
	{
	public:
		// get instance
		static RenderStage* instance();

		// destroy
		void destroy();

		// add renderable
		void addRenderable(const String& name, RenderableID id);

		// process
		void process();

	protected:
		RenderStage();
		~RenderStage();

	protected:
		vector<RenderStageItem*>::type	m_items;				// äÖÈ¾½×¶Î
	};
}