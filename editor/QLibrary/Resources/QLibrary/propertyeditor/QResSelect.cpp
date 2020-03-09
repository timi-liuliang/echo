#include "QResSelect.h"
#include <QFileDialog>
#include "QPropertyModel.h"
#include "ResChooseDialog.h"
#include "PathChooseDialog.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>

namespace QT_UI
{
	QResSelect::QResSelect(class QPropertyModel* model, QString propertyName, const char* exts, const char* files, QWidget* parent)
		: QWidget( parent)
        , m_exts(exts ? exts : "")
        , m_files(files)
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

		adjustHeightSize();
	}

	void QResSelect::OnSelectPath()
	{
		if (m_exts.empty())
		{
			QString qFileName = Studio::PathChooseDialog::getExistingPath(this);
			if (!qFileName.isEmpty())
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(qFileName.toStdString().c_str(), resPath))
				{
					m_lineEdit->setText(resPath.c_str());
					onEditFinished();
				}
			}
		}
		else
		{
			Echo::String qFileName = Studio::ResChooseDialog::getSelectingFile(this, m_exts.c_str(), m_files.toStdString().c_str(), "");
			if (!qFileName.empty())
			{
				m_lineEdit->setText(qFileName.c_str());
				onEditFinished();
			}
		}
	}

	bool QResSelect::isTextureRes()
	{
		Echo::StringArray exts = Echo::StringUtil::Split(m_exts, "|");
		for (Echo::String ext : exts)
		{
			if (Echo::StringUtil::Equal(ext, ".png", false))
				return true;
		}

		return false;
	}

	void QResSelect::adjustHeightSize()
	{
		if (isTextureRes())
		{
			m_lineEdit->setMinimumHeight(m_lineEdit->geometry().height()*1.6);
			m_toolButton->setMinimumHeight(m_toolButton->geometry().height() * 1.6);
		}
	}

	void QResSelect::paintEvent(QPaintEvent* event)
	{
		if (isTextureRes())
		{

		}
		else
		{
			QWidget::paintEvent(event);
		}
	}

	void QResSelect::onEditFinished()
	{
		Echo::String value = m_lineEdit->text().toStdString().c_str();
		m_propertyModel->setValue(m_propertyName, value.c_str());
	}

	bool QResSelect::ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val)
	{
		Echo::String path = val;
		if (!path.empty())
		{
			Echo::String fullPath = Echo::IO::instance()->convertResPathToFullPath(path);
			Echo::String ext = Echo::PathUtil::GetFileExt(path, true);
			if (Echo::StringUtil::Equal(ext, ".png", false))
			{
				QPixmap pixmap(fullPath.c_str());
				QRect tRect = QRect(rect.left() + 3, rect.top() + 2, rect.height() - 4, rect.height() - 4);
				painter->drawPixmap(tRect, pixmap);

				return true;
			}
		}

		return false;
	}
}
