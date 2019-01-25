#pragma once

#include "RenderQueue.h"

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
		vector<RenderQueue*>::type	m_items;
	};
}