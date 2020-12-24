#include "parameter_list_editor_dialog.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ParamterListEditorDialog::ParamterListEditorDialog(QWidget* parent, const String& params)
		: QDialog(parent)
	{
		setupUi(this);

		m_statusBar = new QStatusBar(this);
		layout()->addWidget(m_statusBar);

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
		m_close->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/core/render/base/editor/shader/icon/close.png").c_str()));

		EditorApi.qConnectWidget(m_add, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onAdd));
		EditorApi.qConnectWidget(m_delete, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onDelete));
		EditorApi.qConnectWidget(m_moveUp, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onMoveUp));
		EditorApi.qConnectWidget(m_moveDown, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onMoveDown));
		EditorApi.qConnectWidget(m_ok, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onOk));
		EditorApi.qConnectWidget(m_cancel, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));
		EditorApi.qConnectWidget(m_close, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorDialog::onCancel));

		parse(params);
	}

	ParamterListEditorDialog::~ParamterListEditorDialog()
	{

	}

	String ParamterListEditorDialog::getParms()
	{
		String result;
		i32 finalIdx = m_tableWidget->rowCount() - 1;
		for (size_t i = 0; i <= finalIdx; i++)
		{
			String name = qobject_cast<QLineEdit*>(m_tableWidget->cellWidget(i, 0))->text().toStdString().c_str();
			String type = qobject_cast<QComboBox*>(m_tableWidget->cellWidget(i, 1))->currentText().toStdString().c_str();
			if (name.size() && type.size())
			{
				result += type + " " + name + (i==finalIdx ? "" : ",");
			}
		}

		return result;
	}

	void ParamterListEditorDialog::parse(const String& params)
	{
		StringArray paramArray = StringUtil::Split(params, ",");
		for (const String& parm : paramArray)
		{
			StringArray nameType = StringUtil::Split(parm, " ");
			if (nameType.size() == 2)
			{
				add(nameType[1], nameType[0]);
			}
		}
	}

	void ParamterListEditorDialog::add(const String& name, const String& type)
	{
		m_tableWidget->insertRow(m_tableWidget->rowCount());

		QLineEdit* lineEdit = new QLineEdit;
		lineEdit->setAlignment(Qt::AlignCenter);
		lineEdit->setText(name.c_str());
		lineEdit->setStyleSheet("background-color: rgb( 83, 83, 83); color : rgb(243, 243, 243); ");
		m_tableWidget->setCellWidget(m_tableWidget->rowCount() - 1, 0, lineEdit);

		QComboBox* comboBox = new QComboBox;
		for (const String& type : m_types)
		{
			comboBox->addItem(type.c_str());
		}
		comboBox->setCurrentText(type.c_str());
		m_tableWidget->setCellWidget(m_tableWidget->rowCount() - 1, 1, comboBox);

		EditorApi.qConnectWidget(lineEdit, QSIGNAL(textChanged(const QString&)), this, createMethodBind(&ParamterListEditorDialog::onChanged));
		EditorApi.qConnectWidget(comboBox, QSIGNAL(currentTextChanged(const QString&)), this, createMethodBind(&ParamterListEditorDialog::onChanged));

		onChanged();
	}

	void ParamterListEditorDialog::onAdd()
	{
		add("", "float");
	}

	void ParamterListEditorDialog::onDelete()
	{
		int currentRow = m_tableWidget->currentRow();
		m_tableWidget->removeRow(currentRow);
	}

	void ParamterListEditorDialog::onChanged()
	{
		String message = Echo::StringUtil::Format("function(%s)", getParms().c_str());
		m_statusBar->showMessage(message.c_str());
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