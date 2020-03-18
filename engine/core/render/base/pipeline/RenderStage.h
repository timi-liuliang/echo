#pragma once

#include "RenderQueue.h"

namespace Echo
{
	class RenderStage
	{
	public:
		RenderStage();
		~RenderStage();

		// destroy
		void destroy();

		// add renderable
		void addRenderable(const String& name, RenderableID id);

		// process
		void process();

	protected:
		vector<RenderQueue*>::type	m_items;
	};
}