#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "../../atla/texture_atlas.h"

#ifdef ECHO_EDITOR_MODE

#include <QtWidgets/QGraphicsItem>

namespace Echo
{
	class TextureAtlasPanel : public EditorDockPanel
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		TextureAtlasPanel(Object* obj);
		virtual ~TextureAtlasPanel();

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

	protected:
		QTreeWidget*				m_nodeTreeWidget = nullptr;
		TextureAtlas*				m_textureAtlas;
		QMenu*						m_importMenu = nullptr;
		QGraphicsView*				m_graphicsView = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		QGraphicsItem*				m_imageItem = nullptr;
		QGraphicsItem*				m_imageBorder = nullptr;
		QGraphicsItem*				m_atlaBorder = nullptr;
		QDialog*					m_splitDialog = nullptr;
	};
}

#endif