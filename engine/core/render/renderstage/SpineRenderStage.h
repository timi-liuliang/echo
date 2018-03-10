#pragma once

#include "RenderStage.h"

namespace Echo
{
	/**
	 * SpineäÖÈ¾½×¶Î
	 */
	class SpineRenderStage : public RenderStage
	{
		friend class RenderStageManager;

	public:
		virtual bool initialize();
		virtual void render();
		virtual void destroy();

		// Ìí¼ÓäÖÈ¾µ¥Ôª
		void addRenderable(int renderalbe) { m_renderables.push_back(renderalbe); }

	protected:
		SpineRenderStage();
		virtual ~SpineRenderStage();

	private:
		vector<int>::type	m_renderables;		// 2d¹Ç÷À¶¯»­äÖÈ¾µ¥Ôª
	};
}