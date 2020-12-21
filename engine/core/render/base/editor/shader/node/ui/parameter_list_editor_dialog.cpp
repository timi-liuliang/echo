#include "parameter_list_editor_dialog.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ParamterListEditorDialog::ParamterListEditorDialog(QWidget* parent)
		: QDialog(parent)
	{
		setupUi(this);

#ifdef ECHO_PLATFORM_WINDOWS
		// hide window hwnd
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		//menubar->setNativeMenuBar(false);
#endif

		m_add->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/add.png").c_str()));
		m_delete->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/add.png").c_str()));
		m_moveUp->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/add.png").c_str()));
		m_moveDown->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/add.png").c_str()));

		EditorApi.qConnectWidget(m_add, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_delete, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_moveUp, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_moveDown, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_ok, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_cancel, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
	}

	ParamterListEditorDialog::~ParamterListEditorDialog()
	{

	}

	void ParamterListEditorDialog::onCancel()
	{
		close();
	}

#endif
}