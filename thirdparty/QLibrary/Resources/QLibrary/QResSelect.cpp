#include "QResSelect.h"
#include <QFileDialog>
#include "QPropertyModel.h"

namespace QT_UI
{
	QResSelect::OpenFileDialogFunction QResSelect::m_openFileFunction;

	// 构造函数
	QResSelect::QResSelect(class QPropertyModel* model, QString propertyName, QWidget* parent)
		: QWidget( parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
	{
		// 布局控件
		m_horizonLayout = new QHBoxLayout( this);
		m_horizonLayout->setSpacing( 0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName( QString::fromUtf8("horizontalLayout"));

		// LineEdit
		m_lineEdit = new QLineEdit( this);
		m_lineEdit->setObjectName( QString::fromUtf8("lineEdit"));
		m_lineEdit->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_lineEdit);

		// ToolButton
		m_toolButton = new QToolButton( this);
		m_toolButton->setObjectName( QString::fromUtf8("toolButton"));
		m_toolButton->setText( "...");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_toolButton);

		setFocusProxy( m_toolButton);

		// 消息
		QObject::connect( m_toolButton, SIGNAL(clicked()), this, SLOT(OnSelectPath()));
		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
	}

	// 选择路径
	void QResSelect::OnSelectPath()
	{
		Q_ASSERT_X(m_openFileFunction, "", "");
		Echo::String qFileName = m_openFileFunction(this, m_exts.toStdString().c_str(), m_files.toStdString().c_str(), "");
		if (!qFileName.empty())
		{
			m_lineEdit->setText(qFileName.c_str());
			onEditFinished();
		}
	}

	// edit finished
	void QResSelect::onEditFinished()
	{
		m_propertyModel->setValue(m_propertyName, m_lineEdit->text().toStdString().c_str());
	}
}