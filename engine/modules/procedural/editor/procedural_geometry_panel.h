#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"
#include "../procedural_geometry.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ProceduralGeometryPanel : public PanelTab
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		// Background style
		struct BackgroundStyle
		{
			Color m_backgroundColor;
			Color m_fineGridColor;
			Color m_coarseGridColor;
		};

	public:
		ProceduralGeometryPanel(Object* obj);
		virtual ~ProceduralGeometryPanel();

		// update
		void update();

	public:
		// new
		void onNewAtla();

		// import
		void onImport();
		void onImportFromImages();

		// split
		void onSplit();
		void onSelectItem();

		// change name
		void onChangedAtlaName();

		// refresh list
		void refreshUiDisplay();
		void refreshAtlaList();
		void refreshImageDisplay();

		// clear
		void clearImageItemAndBorder();

	private:
		// draw
		void drawBackground();

	protected:
		ProceduralGeometry*				m_proceduralGeometry = nullptr;
		QWidget*						m_importMenu = nullptr;
		QObject*						m_graphicsScene;
		BackgroundStyle					m_backgroundStyle;
		vector<QGraphicsItem*>::type	m_backgroundGrids;
	};
}

#endif
