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

		// stretch header
		m_tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
		m_tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Name"));
		m_tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("Type"));

#ifdef ECHO_PLATFORM_WINDOWS
		// hide window hwnd
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		//menubar->setNativeMenuBar(false);
#endif

		m_add->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/add.png").c_str()));
		m_delete->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/delete.png").c_str()));
		m_moveUp->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/move_up.png").c_str()));
		m_moveDown->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/move_down.png").c_str()));

		EditorApi.qConnectWidget(m_add, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onAdd));
		EditorApi.qConnectWidget(m_delete, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onDelete));
		EditorApi.qConnectWidget(m_moveUp, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onMoveUp));
		EditorApi.qConnectWidget(m_moveDown, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onMoveDown));
		EditorApi.qConnectWidget(m_ok, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onOk));
		EditorApi.qConnectWidget(m_cancel, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
	}

	ParamterListEditorDialog::~ParamterListEditorDialog()
	{

	}

	void ParamterListEditorDialog::onAdd()
	{
		m_tableWidget->insertRow(m_tableWidget->rowCount());
	}

	void ParamterListEditorDialog::onDelete()
	{
		int currentRow = m_tableWidget->currentRow();
		m_tableWidget->removeRow(currentRow);
	}

	void ParamterListEditorDialog::onMoveUp()
	{

	}

	void ParamterListEditorDialog::onMoveDown()
	{

	}

	void ParamterListEditorDialog::onOk()
	{

	}

	void ParamterListEditorDialog::onCancel()
	{
		close();
	}

#endif
}