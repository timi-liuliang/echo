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
#include "TextureAtlasPackage.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TextureAtlasPanel::TextureAtlasPanel(Object* obj)
	{
		m_textureAtlas = ECHO_DOWN_CAST<TextureAtlas*>(obj);

		m_ui = EditorApi.qLoadUi("engine/core/render/base/editor/panel/TextureAtlasPanel.ui");

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
					}

					m_textureAtlas->save();

					break;
				}
			}

			EchoSafeDeleteMap(images, Image);
		}
	}
#endif
}
