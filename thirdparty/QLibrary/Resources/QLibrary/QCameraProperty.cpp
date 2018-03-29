#include "QCameraProperty.h"
#include "Engine/core/main/Root.h"

namespace QT_UI
{
	// 构造函数
	QCameraPosDir::QCameraPosDir(QWidget* parent)
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
		m_toolButton->setText("Sync");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_toolButton);

		setFocusProxy( m_toolButton);

		// 消息
		connect( m_toolButton, SIGNAL(clicked()), this, SLOT(OnCatchData()));
	}

	// 选择路径
	void QCameraPosDir::OnCatchData()
	{
		Echo::Vector3 postion = Echo::NodeTree::instance()->get3dCamera()->getPosition();
		Echo::Vector3 direction  = Echo::NodeTree::instance()->get3dCamera()->getDirection();
		Echo::String  format = Echo::StringUtil::ToString(postion) + ";" + Echo::StringUtil::ToString(direction);

		m_lineEdit->setText(format.c_str());
	}
}