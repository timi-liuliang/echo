#include "TextureAtlasPanel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/MemoryReader.h"
#include "engine/core/util/Buffer.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/image/Image.h"
#include "engine/core/render/base/TextureAtlas.h"
#include "TextureAtlasPackage.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TextureAtlasPanel::TextureAtlasPanel(Object* obj)
	{
		m_textureAtlas = ECHO_DOWN_CAST<TextureAtlas*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/atlas/TextureAtlasPanel.ui");

		QWidget* splitter = EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			EditorApi.qSplitterSetStretchFactor(splitter, 0, 0);
			EditorApi.qSplitterSetStretchFactor(splitter, 1, 1);
		}

		// Tool button icons
		EditorApi.qToolButtonSetIcon(EditorApi.qFindChild(m_ui, "m_import"), "engine/core/render/base/editor/icon/import.png");
		EditorApi.qToolButtonSetIcon(EditorApi.qFindChild(m_ui, "m_splitBygrid"), "engine/core/render/base/editor/icon/grid.png");

		// connect signal slots
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_import"), QSIGNAL(clicked()), this, createMethodBind(&TextureAtlasPanel::onImport));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_splitBygrid"), QSIGNAL(clicked()), this, createMethodBind(&TextureAtlasPanel::onSplit));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"), QSIGNAL(itemClicked(QTreeWidgetItem*, int)), this, createMethodBind(&TextureAtlasPanel::onSelectItem));

		// create QGraphicsScene
		m_graphicsScene = EditorApi.qGraphicsSceneNew();
		EditorApi.qGraphicsViewSetScene(EditorApi.qFindChild(m_ui, "m_graphicsView"), m_graphicsScene);

		refreshUiDisplay();
	}

	TextureAtlasPanel::~TextureAtlasPanel()
	{
		clearImageItemAndBorder();
	}

	void TextureAtlasPanel::update()
	{
	}

	void TextureAtlasPanel::onImport()
	{
		if (!m_importMenu)
		{
			m_importMenu = EditorApi.qMenuNew(m_ui);

			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"));
			//EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionAddSetting"));
			//EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionAddResource"));

			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"), QSIGNAL(triggered()), this, createMethodBind(&TextureAtlasPanel::onImportFromImages));
			//EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionAddSetting"), QSIGNAL(triggered()), this, createMethodBind(&TimelinePanel::onAddSetting));
			//EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionAddResource"), QSIGNAL(triggered()), this, createMethodBind(&TimelinePanel::onAddResource));
		}

		EditorApi.qMenuExec(m_importMenu);
	}

	void TextureAtlasPanel::onImportFromImages()
	{
		StringArray files = Editor::instance()->qGetOpenFileNames(nullptr, "Select Images", "", "*.png");
		if (!files.empty())
		{
			// load images
			multimap<float, Image*>::type images;
			for (String& file : files)
			{
				Image* image = Image::loadFromFile(file);
				if (image)
					images.insert(std::pair<float, Image*>(float(image->getWidth()*image->getHeight()), image));
			}

			// built atlas
			array<i32, 7> sizes = { 64, 128, 256, 512, 1024, 2048, 4096 };
			for (size_t i = 0; i < sizes.size(); i++)
			{
				bool isSpaceEnough = true;
				TextureAtlasPackage atlasPackage(sizes[i], sizes[i]);
				for (auto& it : images)
				{
					Image* image = it.second;
					String atlaName = PathUtil::GetPureFilename(image->getFilePath(), false);
					if (image)
					{
						vector<Color>::type colors = image->getColors();
						int id = atlasPackage.insert(colors.data(), image->getWidth(), image->getHeight(), atlaName);
						if (id == -1)
						{
							isSpaceEnough = false;
							break;
						}
					}
				}

				if (isSpaceEnough)
				{
					m_textureAtlas->clear();
					for (const TextureAtlasPackage::Node& node : atlasPackage.getAllNodes())
					{
						if (node.m_id != -1)
						{
							String name = any_cast<String>(node.m_userData);
							Vector4 viewPort = atlasPackage.getViewport(node.m_id);

							m_textureAtlas->addAtla(name, viewPort);
						}
					}

					// save png
					{
						String resPath = m_textureAtlas->getPath();
						String fullPath = IO::instance()->convertResPathToFullPath(resPath);
						fullPath = Echo::PathUtil::GetRenameExtFile(fullPath, ".png");

						atlasPackage.saveImage(fullPath);

						if(IO::instance()->convertFullPathToResPath(fullPath, resPath))
							m_textureAtlas->setTextureRes(resPath);
					}

					m_textureAtlas->save();

					break;
				}
			}

			EchoSafeDeleteMap(images, Image);
		}

		refreshUiDisplay();
	}

	void TextureAtlasPanel::onSplit()
	{

	}

	void TextureAtlasPanel::refreshUiDisplay()
	{
		refreshAtlaList();
		refreshImageDisplay();
	}

	void TextureAtlasPanel::refreshAtlaList()
	{
		QWidget* nodeTreeWidget = EditorApi.qFindChild(m_ui, "m_nodeTreeWidget");
		if (nodeTreeWidget)
		{
			EditorApi.qTreeWidgetClear(nodeTreeWidget);

			QTreeWidgetItem* rootItem = EditorApi.qTreeWidgetInvisibleRootItem(nodeTreeWidget);
			if (rootItem)
			{
				for (const TextureAtlas::Atla atla : m_textureAtlas->getAllAtlas())
				{
					QTreeWidgetItem* objetcItem = EditorApi.qTreeWidgetItemNew();
					EditorApi.qTreeWidgetItemSetText(objetcItem, 0, atla.m_name.c_str());
					EditorApi.qTreeWidgetItemSetUserData(objetcItem, 0, atla.m_name.c_str());
					//EditorApi.qTreeWidgetItemSetIcon(objetcItem, 0, Editor::instance()->getNodeIcon(node).c_str());
					EditorApi.qTreeWidgetItemAddChild(rootItem, objetcItem);
				}
			}
		}
	}

	void TextureAtlasPanel::clearImageItemAndBorder()
	{
		if (m_imageItem)
		{
			EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_imageItem);
			m_imageItem = nullptr;
		}

		if (m_imageBorder)
		{
			EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_imageBorder);
			m_imageBorder = nullptr;
		}
	}

	void TextureAtlasPanel::refreshImageDisplay()
	{
		QWidget* graphicsView = EditorApi.qFindChild(m_ui, "m_graphicsView");
		if (graphicsView)
		{
			clearImageItemAndBorder();

			String resPath = m_textureAtlas->getTextureRes().getPath();
			String fullPath = IO::instance()->convertResPathToFullPath(resPath);

			m_imageItem = EditorApi.qGraphicsSceneAddPixmap(m_graphicsScene, fullPath.c_str());
			EditorApi.qGraphicsItemSetMoveable(m_imageItem, true);

			// image border
			Rect rect;
			EditorApi.qGraphicsItemSceneRect(m_imageItem, rect);

			Color color;
			color.setRGBA(56, 56, 56, 255);

			vector<Vector2>::type paths;
			paths.push_back(Vector2(0.f, 0.f));
			paths.push_back(Vector2(rect.getWidth(), 0.f));
			paths.push_back(Vector2(rect.getWidth(), rect.getHeight()));
			paths.push_back(Vector2(0.f, rect.getHeight()));
			paths.push_back(Vector2(0.f, 0.f));
			m_imageBorder = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, paths, 1.f, color);

			EditorApi.qGraphicsItemSetParentItem(m_imageBorder, m_imageItem);
		}
	}

	void TextureAtlasPanel::onSelectItem()
	{
		QTreeWidgetItem* item = EditorApi.qTreeWidgetCurrentItem(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"));
		if (item && m_textureAtlas->getTexture())
		{
			Vector4 viewPort;
			String userData = EditorApi.qTreeWidgetItemUserData(item, 0);
			if (m_textureAtlas->getViewport(userData, viewPort))
			{
				if (m_atlaBorder)
				{
					EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_atlaBorder);
					m_atlaBorder = nullptr;
				}

				// calculate paths
				vector<Vector2>::type paths;
				{
					TexturePtr texture = m_textureAtlas->getTexture();
					ui32 textureWidth = texture->getWidth();
					ui32 textureHeight = texture->getHeight();
					ui32 atlaWidth = textureWidth * viewPort.z;
					ui32 atlaHeight = textureHeight * viewPort.w;
					ui32 atlaPosX = textureWidth * viewPort.x;
					ui32 atlaPosY = textureHeight * viewPort.y;

					paths.push_back(Vector2(atlaPosX, atlaPosY));
					paths.push_back(Vector2(atlaPosX+atlaWidth, atlaPosY));
					paths.push_back(Vector2(atlaPosX + atlaWidth, atlaPosY + atlaHeight));
					paths.push_back(Vector2(atlaPosX, atlaPosY + atlaHeight));
					paths.push_back(Vector2(atlaPosX, atlaPosY));
				}

				// create qGraphicsItem
				m_atlaBorder = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, paths, 1.f, Color::RED);

				EditorApi.qGraphicsItemSetParentItem(m_atlaBorder, m_imageItem);
			}
		}
	}
#endif
}
