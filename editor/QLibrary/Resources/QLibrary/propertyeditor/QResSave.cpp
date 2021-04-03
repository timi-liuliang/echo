#include "QResSave.h"
#include <QFileDialog>
#include "QPropertyModel.h"
#include "ResChooseDialog.h"
#include "ResPanel.h"
#include "PathChooseDialog.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>

namespace QT_UI
{
	QResSave::QResSave(class QPropertyModel* model, QString propertyName, const char* exts, const char* files, QWidget* parent)
		: QWidget( parent)
        , m_exts(exts ? exts : "")
        , m_files(files)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
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
		m_resSelectButton = new QToolButton( this);
		//m_resSelectButton->setText( "...");
		m_resSelectButton->setIcon(QIcon(":/icon/Icon/res/file.png"));
		m_resSelectButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_resSelectButton);

		// find button
		m_resFindButton = new QToolButton(this);
		m_resFindButton->setIcon(QIcon(":/icon/Icon/res/loupe.png"));
		m_resFindButton->setToolTip("Show In Res Panel");
		m_resFindButton->setContentsMargins(1, 0, 0, 0);
		m_horizonLayout->addWidget(m_resFindButton);

		setFocusProxy( m_resSelectButton);

		// signal|slots
		QObject::connect(m_resSelectButton, SIGNAL(clicked()), this, SLOT(onSelectPath()));
		QObject::connect(m_resFindButton, SIGNAL(clicked()), this, SLOT(onShowRes()));
		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));

		adjustHeightSize();
	}

	void QResSave::onSelectPath()
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
			Echo::String qFileName = Studio::PathChooseDialog::getExistingPathName(this, m_exts.c_str(), "Select save path name").toStdString().c_str();
			if (!qFileName.empty())
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(qFileName, resPath))
				{
					m_lineEdit->setText(resPath.c_str());
					onEditFinished();
				}
			}
		}
	}

	void QResSave::onShowRes()
	{
		Echo::String resPathName = m_lineEdit->text().toStdString().c_str();
		if (!resPathName.empty())
		{
			Echo::String fullPathName = Echo::IO::instance()->convertResPathToFullPath(resPathName);
			if(!fullPathName.empty())
				Studio::ResPanel::instance()->onSelectFile(fullPathName.c_str());
		}
	}

	bool QResSave::isTextureRes()
	{
		Echo::StringArray exts = Echo::StringUtil::Split(m_exts, "|");
		for (Echo::String ext : exts)
		{
			if (Echo::StringUtil::Equal(ext, ".png", false))
				return true;
		}

		return false;
	}

	void QResSave::adjustHeightSize()
	{
		if (isTextureRes())
		{
			m_lineEdit->setMinimumHeight(m_lineEdit->geometry().height()*1.6);
			m_resSelectButton->setMinimumHeight(m_resSelectButton->geometry().height() * 1.6);
		}
	}

	void QResSave::paintEvent(QPaintEvent* event)
	{
		if (isTextureRes())
		{

		}
		else
		{
			QWidget::paintEvent(event);
		}
	}

	void QResSave::onEditFinished()
	{
		Echo::String value = m_lineEdit->text().toStdString().c_str();
		m_propertyModel->setValue(m_propertyName, value.c_str());

		Echo::String resPathName = m_lineEdit->text().toStdString().c_str();
		m_resFindButton->setVisible(resPathName.empty() ? false : true);
	}

	bool QResSave::ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val)
	{
		Echo::String path = val;
		if (!path.empty())
		{
			Echo::String ext = Echo::PathUtil::GetFileExt(path, true);
			if (!ext.empty())
			{
				if (Echo::IO::instance()->isExist(path))
				{
					Echo::String fullPath = Echo::IO::instance()->convertResPathToFullPath(path);
					if (Echo::StringUtil::Equal(ext, ".png", false))
					{
						QPixmap pixmap(fullPath.c_str());
						QRect tRect = QRect(rect.left() + 3, rect.top() + 2, rect.height() - 4, rect.height() - 4);
						painter->drawPixmap(tRect, pixmap);

						return true;
					}
				}
			}
		}

		return false;
	}
}
