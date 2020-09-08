#include "procedural_geometry_panel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/MemoryReader.h"
#include "engine/core/util/Buffer.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/image/Image.h"
#include "engine/core/render/base/atla/TextureAtlas.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ProceduralGeometryPanel::ProceduralGeometryPanel(Object* obj)
	{
		m_proceduralGeometry = ECHO_DOWN_CAST<ProceduralGeometry*>(obj);

		m_ui = EditorApi.qLoadUi("engine/modules/procedural/editor/procedural_geometry_panel.ui");

		QWidget* splitter = EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			EditorApi.qSplitterSetStretchFactor(splitter, 0, 0);
			EditorApi.qSplitterSetStretchFactor(splitter, 1, 1);
		}

		// Tool button icons
		EditorApi.qToolButtonSetIcon(EditorApi.qFindChild(m_ui, "m_import"), "engine/core/render/base/editor/icon/import.png");

		// connect signal slots
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_import"), QSIGNAL(clicked()), this, createMethodBind(&ProceduralGeometryPanel::onImport));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"), QSIGNAL(itemClicked(QTreeWidgetItem*, int)), this, createMethodBind(&ProceduralGeometryPanel::onSelectItem));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"), QSIGNAL(itemChanged(QTreeWidgetItem*, int)), this, createMethodBind(&ProceduralGeometryPanel::onChangedAtlaName));

		// create QGraphicsScene
		m_graphicsScene = EditorApi.qGraphicsSceneNew();
		EditorApi.qGraphicsViewSetScene(EditorApi.qFindChild(m_ui, "m_graphicsView"), m_graphicsScene);
	}

	ProceduralGeometryPanel::~ProceduralGeometryPanel()
	{
		clearImageItemAndBorder();
	}

	void ProceduralGeometryPanel::update()
	{
		refreshUiDisplay();

	}

	void ProceduralGeometryPanel::onNewAtla()
	{
	}

	void ProceduralGeometryPanel::onImport()
	{
		if (!m_importMenu)
		{
			m_importMenu = EditorApi.qMenuNew(m_ui);

			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionAddNewOne"));
			EditorApi.qMenuAddSeparator(m_importMenu);
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionBuildFromGrid"));
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"));

			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionAddNewOne"), QSIGNAL(triggered()), this, createMethodBind(&ProceduralGeometryPanel::onNewAtla));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"), QSIGNAL(triggered()), this, createMethodBind(&ProceduralGeometryPanel::onImportFromImages));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionBuildFromGrid"), QSIGNAL(triggered()), this, createMethodBind(&ProceduralGeometryPanel::onSplit));
		}

		EditorApi.qMenuExec(m_importMenu);
	}

	void ProceduralGeometryPanel::onImportFromImages()
	{

	}

	void ProceduralGeometryPanel::onSplit()
	{
	}

	void ProceduralGeometryPanel::refreshUiDisplay()
	{
		drawBackground();
	}

	void ProceduralGeometryPanel::drawBackground()
	{
		m_backgroundStyle.m_backgroundColor.setRGBA(77, 77, 77, 255);
		m_backgroundStyle.m_fineGridColor.setRGBA(84, 84, 84, 255);
		m_backgroundStyle.m_coarseGridColor.setRGBA(64, 64, 64, 255);

		EditorApi.qGraphicsViewSetBackgroundBrush(EditorApi.qFindChild(m_ui, "m_graphicsView"), m_backgroundStyle.m_backgroundColor);

		auto drawGrid = [&](double gridStep, const Color& color)
		{
			Rect viewRect;
			EditorApi.qGraphicsViewSceneRect(EditorApi.qFindChild(m_ui, "m_graphicsView"), viewRect);

			double left = std::floor(viewRect.left / gridStep - 0.5);
			double right = std::floor(viewRect.right / gridStep + 1.0);
			double bottom = std::floor(viewRect.bottom / gridStep - 0.5);
			double top = std::floor(viewRect.top / gridStep + 1.0);

			// vertical lines
			for (int xi = int(left); xi <= int(right); ++xi)
			{
				m_backgroundGrids.push_back(EditorApi.qGraphicsSceneAddLine(m_graphicsScene, xi * gridStep, bottom * gridStep, xi * gridStep, top * gridStep, color));
			}

			// horizontal lines
			for (int yi = int(top); yi <= int(bottom); ++yi)
			{
				m_backgroundGrids.push_back(EditorApi.qGraphicsSceneAddLine(m_graphicsScene, left * gridStep, yi * gridStep, right * gridStep, yi * gridStep, color));
			}
		};

		m_backgroundGrids.clear();

		drawGrid(15, m_backgroundStyle.m_fineGridColor);
		drawGrid(150, m_backgroundStyle.m_coarseGridColor);
	}

	void ProceduralGeometryPanel::refreshAtlaList()
	{
	}

	void ProceduralGeometryPanel::clearImageItemAndBorder()
	{
	}

	void ProceduralGeometryPanel::refreshImageDisplay()
	{

	}

	void ProceduralGeometryPanel::onSelectItem()
	{

	}

	void ProceduralGeometryPanel::onChangedAtlaName()
	{

	}
#endif
}
