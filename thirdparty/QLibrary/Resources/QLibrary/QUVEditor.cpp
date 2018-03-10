#include "QPropertyModel.h"
#include "QUVEditor.h"
#include <QFile>
#include <QDialog>
#include <QObject>
#include <QUiLoader>
#include <QPushButton>

namespace QT_UI
{
	// 构造函数
	QUVEditor::QUVEditor(QPropertyModel* model, QString name, QWidget* parent)
		:QWidget(parent), propertyModel(model), propertyName(name)
	{
		// 布局控件
		m_horizonLayout = new QHBoxLayout(this);
		m_horizonLayout->setSpacing(0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

		// LineEdit
		m_lineEdit = new QLineEdit(this);
		m_lineEdit->setObjectName(QString::fromUtf8("lineEdit"));
		m_lineEdit->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_lineEdit);

		// ToolButton
		m_toolButton = new QToolButton(this);
		m_toolButton->setObjectName(QString::fromUtf8("toolButton"));
		m_toolButton->setText("...");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_toolButton);

		setFocusProxy(m_toolButton);

		//消息
		connect(m_toolButton, SIGNAL(clicked()), this, SLOT(OnEditUV()));
		connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(UpdateUV()));
	}

	void QUVEditor::OnEditUV()
	{
		if (propertyModel)
		{
			propertyModel->ThrowSelfDefineSig("UVPropertyEdit",propertyName,NULL);
		}
	}

	void QUVEditor::UpdateUV()
	{
		QString text = m_lineEdit->text();
		QStringList strlist = text.split(" ");
		if (propertyModel && strlist.size() == 4)
		{
			propertyModel->setValue(propertyName, text);
		}
		
	}
}