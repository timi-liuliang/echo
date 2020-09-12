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
#include "engine/core/render/base/atla/TextureAtlas.h"
#include "TextureAtlasPackage.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TextureAtlasPanel::TextureAtlasPanel(Object* obj)
	{
		m_textureAtlas = ECHO_DOWN_CAST<TextureAtlas*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/atlas/TextureAtlasPanel.ui");
		m_splitDialog = EditorApi.qLoadUi("engine/core/render/base/editor/atlas/TextureAtlasSplitDialog.ui");

		QWidget* splitter = EditorApi.qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			EditorApi.qSplitterSetStretchFactor(splitter, 0, 0);
			EditorApi.qSplitterSetStretchFactor(splitter, 1, 1);
		}

		// Tool button icons
		EditorApi.qToolButtonSetIcon(EditorApi.qFindChild(m_ui, "m_import"), "engine/core/render/base/editor/icon/import.png");

		// connect signal slots
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_import"), QSIGNAL(clicked()), this, createMethodBind(&TextureAtlasPanel::onImport));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"), QSIGNAL(itemClicked(QTreeWidgetItem*, int)), this, createMethodBind(&TextureAtlasPanel::onSelectItem));
		EditorApi.qConnectWidget(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"), QSIGNAL(itemChanged(QTreeWidgetItem*, int)), this, createMethodBind(&TextureAtlasPanel::onChangedAtlaName));

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

	void TextureAtlasPanel::onNewAtla()
	{
		if (m_textureAtlas && m_textureAtlas->getTexture())
		{
			String atlaName = StringUtil::Format("NewAtla");
			m_textureAtlas->addAtla(atlaName, Vector4(0, 0, 128, 128));

			refreshUiDisplay();
		}
	}

	void TextureAtlasPanel::onImport()
	{
		if (!m_importMenu)
		{
			m_importMenu = EditorApi.qMenuNew(m_ui);
			
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionAddNewOne"));
			EditorApi.qMenuAddSeparator(m_importMenu);
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionBuildFromGrid"));
			EditorApi.qMenuAddAction(m_importMenu, EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"));

			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionAddNewOne"), QSIGNAL(triggered()), this, createMethodBind(&TextureAtlasPanel::onNewAtla));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionImportFromImages"), QSIGNAL(triggered()), this, createMethodBind(&TextureAtlasPanel::onImportFromImages));
			EditorApi.qConnectAction(EditorApi.qFindChildAction(m_ui, "m_actionBuildFromGrid"), QSIGNAL(triggered()), this, createMethodBind(&TextureAtlasPanel::onSplit));
		}

		EditorApi.qMenuExec(m_importMenu);
	}

	void TextureAtlasPanel::onImportFromImages()
	{
		StringArray files = Editor::instance()->qGetOpenFileNames(nullptr, "Select Images", "", "*.png");
		if (!files.empty())
		{
			// load images
			multimap<float, Image*, std::greater<float>>::type images;
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
		if (EditorApi.qDialogExec(m_splitDialog))
		{
			ui32 rows = EditorApi.qSpinBoxValue(EditorApi.qFindChild(m_splitDialog, "m_spinBoxRows"));
			ui32 columns = EditorApi.qSpinBoxValue(EditorApi.qFindChild(m_splitDialog, "m_spinBoxColumns"));
			String prefix = EditorApi.qLineEditText(EditorApi.qFindChild(m_splitDialog, "m_prefix"));

			TexturePtr texture = m_textureAtlas->getTexture();
			if (texture)
			{
				m_textureAtlas->clear();

				float stepWidth = texture->getWidth() / columns;
				float stepHeight = texture->getHeight() / rows;

				for (ui32 row = 0; row < rows; row++)
				{
					for (ui32 column = 0; column < columns; column++)
					{
						float left = column * stepWidth;
						float top = row * stepHeight;
						String atlaName = StringUtil::Format("%s_%d_%d", prefix.c_str(), row, column);
						m_textureAtlas->addAtla(atlaName, Vector4(left, top, stepWidth, stepHeight));
					}
				}

				m_textureAtlas->save();

				refreshUiDisplay();
			}
		}
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
					EditorApi.qTreeWidgetItemSetEditable(objetcItem, true);
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

			// because m_imageBorder is a child of m_imageItem.
			// so m_imageBorder will be delete too.
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
			paths.emplace_back(0.f, 0.f);
			paths.emplace_back(rect.getWidth(), 0.f);
			paths.emplace_back(rect.getWidth(), rect.getHeight());
			paths.emplace_back(0.f, rect.getHeight());
			paths.emplace_back(0.f, 0.f);
			m_imageBorder = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, paths, 1.f, color);

			EditorApi.qGraphicsItemSetParentItem(m_imageBorder, m_imageItem);

			// fit in view
			//EditorApi.qGraphicsViewFitInView(EditorApi.qFindChild(m_ui, "m_graphicsView"), rect);
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
					Real atlaWidth = viewPort.z;
					Real atlaHeight = viewPort.w;
					Real atlaPosX = viewPort.x;
					Real atlaPosY = viewPort.y;

					paths.emplace_back(atlaPosX, atlaPosY);
					paths.emplace_back(atlaPosX+atlaWidth, atlaPosY);
					paths.emplace_back(atlaPosX + atlaWidth, atlaPosY + atlaHeight);
					paths.emplace_back(atlaPosX, atlaPosY + atlaHeight);
					paths.emplace_back(atlaPosX, atlaPosY);
				}

				// create qGraphicsItem
				m_atlaBorder = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, paths, 1.f, Color::RED);

				EditorApi.qGraphicsItemSetParentItem(m_atlaBorder, m_imageItem);
			}
		}
	}

	void TextureAtlasPanel::onChangedAtlaName()
	{
		QTreeWidgetItem* item = EditorApi.qTreeWidgetCurrentItem(EditorApi.qFindChild(m_ui, "m_nodeTreeWidget"));
		if (item)
		{
			Vector4 viewPort;

			String newName = EditorApi.qTreeWidgetItemText(item, 0);
			String oldName = EditorApi.qTreeWidgetItemUserData(item, 0);
			if (m_textureAtlas->getViewport(oldName, viewPort))
			{
				m_textureAtlas->removeAtla(oldName);
				m_textureAtlas->addAtla(newName, viewPort);

				EditorApi.qTreeWidgetItemSetUserData(item, 0, newName.c_str());
			}
		}
	}
#endif
}
