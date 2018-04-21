#include "QResSelect.h"
#include <QFileDialog>

namespace QT_UI
{
	QResSelect::OpenFileDialogFunction QResSelect::m_openFileFunction;

	// 构造函数
	QResSelect::QResSelect( QWidget* parent)
		: QWidget( parent)
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
		connect( m_toolButton, SIGNAL(clicked()), this, SLOT(OnSelectPath()));
	}

	// 选择路径
	void QResSelect::OnSelectPath()
	{
		Q_ASSERT_X(m_openFileFunction, "", "");
		Echo::String qFileName = m_openFileFunction(this, m_exts.toStdString().c_str(), m_files.toStdString().c_str(), "");
		if (!qFileName.empty())
		{
			m_lineEdit->setText(qFileName.c_str());
		}
	}
}