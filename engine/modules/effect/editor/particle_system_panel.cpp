#include "particle_system_panel.h"
#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/memory_reader.h"
#include "engine/core/util/Buffer.h"
#include "engine/core/io/IO.h"
#include "engine/core/main/Engine.h"
#include "engine/core/render/base/image/image.h"
#include "engine/core/render/base/texture/texture_atlas.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ParticleSystemPanel::ParticleSystemPanel(Object* obj)
	{
		m_particleSystem = ECHO_DOWN_CAST<ParticleSystem*>(obj);

		setupUi(this);

		if (m_splitter)
		{
			m_splitter->setStretchFactor(0, 0);
			m_splitter->setStretchFactor(1, 1);
		}

		// Tool button icons
		m_import->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/import.png").c_str()));

		// connect signal slots
		EditorApi.qConnectWidget(m_import, QSIGNAL(clicked()), this, createMethodBind(&ParticleSystemPanel::onImport));
		EditorApi.qConnectWidget(m_nodeTreeWidget, QSIGNAL(itemClicked(QTreeWidgetItem*, int)), this, createMethodBind(&ParticleSystemPanel::onSelectItem));
		EditorApi.qConnectWidget(m_nodeTreeWidget, QSIGNAL(itemChanged(QTreeWidgetItem*, int)), this, createMethodBind(&ParticleSystemPanel::onChangedAtlaName));

		// create QGraphicsScene
		m_graphicsScene = EditorApi.qGraphicsSceneNew();
		m_graphicsView->setScene(m_graphicsScene);

		refreshUiDisplay();
	}

	ParticleSystemPanel::~ParticleSystemPanel()
	{
		clearImageItemAndBorder();
	}

	void ParticleSystemPanel::update()
	{
	}

	void ParticleSystemPanel::onNewAtla()
	{
	}

	void ParticleSystemPanel::onImport()
	{
		if (!m_importMenu)
		{
			m_importMenu = new QMenu(this);
			
		}

		m_importMenu->exec(QCursor::pos());
	}

	void ParticleSystemPanel::onImportFromImages()
	{

	}

	void ParticleSystemPanel::onSplit()
	{
	}

	void ParticleSystemPanel::refreshUiDisplay()
	{
	}

	void ParticleSystemPanel::refreshAtlaList()
	{
	}

	void ParticleSystemPanel::clearImageItemAndBorder()
	{
		if (m_imageItem)
		{
			m_graphicsScene->removeItem(m_imageItem);
			m_imageItem = nullptr;

			// because m_imageBorder is a child of m_imageItem.
			// so m_imageBorder will be delete too.
		}
	}

	void ParticleSystemPanel::refreshImageDisplay()
	{

	}

	void ParticleSystemPanel::onSelectItem()
	{

	}

	void ParticleSystemPanel::onChangedAtlaName()
	{

	}
#endif
}
