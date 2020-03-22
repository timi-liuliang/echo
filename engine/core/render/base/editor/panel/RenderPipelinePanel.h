#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"
#include "../../pipeline/RenderPipeline.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class RenderpipelinePanel : public BottomPanelTab
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		RenderpipelinePanel(Object* obj);

		// update
		void update();

	public:
		// import
		void onNew();
		void onNewCamera();
		void onNewImageFilter();

	protected:
		RenderPipeline*				m_pipeline;
		QWidget*					m_importMenu = nullptr;
	};
}

#endif