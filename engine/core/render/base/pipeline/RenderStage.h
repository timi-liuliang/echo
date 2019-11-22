#pragma once

#include "RenderQueue.h"

namespace Echo
{
	class RenderStage : public Node
	{
		ECHO_CLASS(RenderStage, Node)

	public:
		RenderStage();
		~RenderStage();

		// get instance
		static RenderStage* instance();

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