#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"
#include "../../TextureAtlas.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class TextureAtlasPanel : public BottomPanelTab
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		TextureAtlasPanel(Object* obj);

		// update
		void update();

	public:
		// import
		void onImport();
		void onImportFromImages();

		// split
		void onSplit();

		// refresh list
		void refreshUiDisplay();
		void refreshAtlaList();
		void refreshImageDisplay();

	protected:
		TextureAtlas*				m_textureAtlas;
		QWidget*					m_importMenu = nullptr;
		QObject*					m_graphicsScene;
		QGraphicsItem*				m_imageItem = nullptr;
	};
}

#endif