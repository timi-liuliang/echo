#include "parameter_list_editor_glsl.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/log/Log.h"
#include "engine/core/editor/editor.h"
#include "parameter_list_editor_dialog.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ParamterListEditorGLSL::ParamterListEditorGLSL()
	{
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		m_lineEdit = new QLineEdit(this);
		m_lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
		m_horizonLayout->addWidget(m_lineEdit);

		m_toolButton = new QToolButton(this);
		m_toolButton->setObjectName(QString::fromUtf8("toolButton"));
		m_toolButton->setText("P");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_toolButton);

		EditorApi.qConnectWidget(m_lineEdit, QSIGNAL(textChanged(const QString&)), this, createMethodBind(&ParamterListEditorGLSL::onTextChanged));
		EditorApi.qConnectWidget(m_toolButton, QSIGNAL(clicked()), this, createMethodBind(&ParamterListEditorGLSL::onEditParameters));
	}

	ParamterListEditorGLSL::~ParamterListEditorGLSL()
	{

	}

	void ParamterListEditorGLSL::setObject(Object* object)
	{
		PropertyEditor::setObject(object);
		if (m_object)
		{
			Echo::Variant value;
			Echo::Class::getPropertyValue(m_object, m_propertyName, value);

			m_lineEdit->setText(value.toString().c_str());
		}
	}

	void ParamterListEditorGLSL::onTextChanged()
	{
		if (m_object)
		{
			Echo::String value = m_lineEdit->text().toStdString().c_str();
			Class::setPropertyValue(m_object, m_propertyName, value);
		}
	}

	void ParamterListEditorGLSL::onEditParameters()
	{
		ParamterListEditorDialog dialog(this, m_lineEdit->text().toStdString().c_str());
		if (dialog.exec() == QDialog::Accepted)
		{
			m_lineEdit->setText(dialog.getParms().c_str());
		}
	}
#endif
}