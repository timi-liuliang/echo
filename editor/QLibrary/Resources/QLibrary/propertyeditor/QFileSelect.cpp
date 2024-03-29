#include "QFileSelect.h"
#include <QFileDialog>

namespace QT_UI
{
	QFileSelect::QFileSelect( QWidget* parent)
		: QWidget( parent)
	{
		// layout
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

		// connect
		connect( m_toolButton, SIGNAL(clicked()), this, SLOT(OnSelectPath()));
	}

	void QFileSelect::OnSelectPath()
	{
		QString qFileName = QFileDialog::getOpenFileName( this, tr("Open Picture"), "", tr("*.tga;; *.png;; *.*"), nullptr, QFileDialog::DontUseNativeDialog);
		if( !qFileName.isEmpty())
		{
			m_lineEdit->setText( QFileInfo( qFileName).fileName().toStdString().c_str());
		}
	}
}
