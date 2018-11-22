#include "QResEditor.h"
#include <QFileDialog>
#include "QPropertyModel.h"
#include "ResChooseDialog.h"
#include "NodeTreePanel.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/resource/Res.h>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QResEditor::QResEditor(class QPropertyModel* model, QString propertyName, const char* resType, QWidget* parent)
		: QWidget( parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
		, m_resType(resType)
		, m_menu(nullptr)
	{
		// 根据资源类型获取后缀
		m_exts = Echo::Res::getResFunByClassName(m_resType)->m_ext;

		// 布局控件
		m_horizonLayout = new QHBoxLayout( this);
		m_horizonLayout->setSpacing( 0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName( QString::fromUtf8("horizontalLayout"));

		// LineEdit
		m_lineEdit = new QLineEdit( this);
		m_lineEdit->setReadOnly(true);
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
		QObject::connect( m_toolButton, SIGNAL(clicked()), this, SLOT(onShowMenu()));
		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));

		adjustHeightSize();
	}

	void QResEditor::onShowMenu()
	{
		showMenu(QCursor::pos());
	}

	// is texture res
	bool QResEditor::isTextureRes()
	{
		Echo::StringArray exts = Echo::StringUtil::Split(m_exts, "|");
		for (Echo::String ext : exts)
		{
			if (Echo::StringUtil::Equal(ext, ".png", false))
				return true;
		}

		return false;
	}

	// correct size
	void QResEditor::adjustHeightSize()
	{
		if (isTextureRes())
		{
			m_lineEdit->setMinimumHeight(m_lineEdit->geometry().height()*1.6);
			m_toolButton->setMinimumHeight(m_toolButton->geometry().height() * 1.6);
		}
	}

	// redefine paintEvent
	void QResEditor::paintEvent(QPaintEvent* event)
	{
		if (isTextureRes())
		{

		}
		else
		{
			QWidget::paintEvent(event);
		}
	}

	// edit finished
	void QResEditor::onEditFinished()
	{
		m_propertyModel->setValue(m_propertyName, m_lineEdit->text().toStdString().c_str());
	}

	// MVC渲染
	bool QResEditor::ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val)
	{
		Echo::String id = val;
		//Echo::String fullPath = Echo::IO::instance()->getFullPath(path);
		//Echo::String ext = Echo::PathUtil::GetFileExt(path, true);
		//if (Echo::StringUtil::Equal(ext, ".png", false))
		//{
		//	QPixmap pixmap(fullPath.c_str());
		//	QRect tRect = QRect(rect.left() + 3, rect.top() + 2, rect.height() - 4, rect.height() - 4);
		//	painter->drawPixmap( tRect, pixmap);
		//}

		return false;
	}


	// node tree widget show menu
	void QResEditor::showMenu(const QPoint& point)
	{
		EchoSafeDelete(m_menu, QMenu);
		m_menu = EchoNew(QMenu(this));

		// new
		QAction* createAction = new QAction(m_menu);
		createAction->setText(("New " + m_resType).c_str());
		m_menu->addAction(createAction);

		m_menu->addSeparator();

		// load
		QAction* loadAction = new QAction(m_menu);
		loadAction->setText("Load");
		loadAction->setIcon(QIcon(":/icon/res/folder.png"));
		m_menu->addAction(loadAction);
		QObject::connect(loadAction, SIGNAL(triggered()), this, SLOT(onLoad()));

		if (GetId() != "-1")
		{
			// edit
			QAction* editAction = new QAction(m_menu);
			editAction->setText("Edit");
			editAction->setIcon(QIcon(":/icon/res/edit.png"));
			m_menu->addAction(editAction);
			QObject::connect(editAction, SIGNAL(triggered()), this, SLOT(onEdit()));

			// clear
			QAction* clearAction = new QAction(m_menu);
			clearAction->setText("Clear");
			m_menu->addAction(clearAction);
			QObject::connect(clearAction, SIGNAL(triggered()), this, SLOT(onClearRes()));
		}

		m_menu->exec(QCursor::pos());
	}

	// on load
	void QResEditor::onLoad()
	{
		Echo::String qFileName = Studio::ResChooseDialog::getExistingFile(this, m_exts.c_str(), "", "");
		if (!qFileName.empty())
		{
			Echo::Res* res = Echo::Res::get(qFileName);
			if (res)
			{
				m_lineEdit->setText(Echo::StringUtil::ToString(res->getId()).c_str());
				onEditFinished();
			}
		}
	}

	// on edit
	void QResEditor::onEdit()
	{
		Echo::ui32 id = Echo::StringUtil::ParseI32(GetId().toStdString().c_str());
		Echo::Object* obj = Echo::Object::getById(id);
		if (obj)
		{
			Echo::Res* res = ECHO_DOWN_CAST<Echo::Res*>(obj);
			if(res)
				Studio::NodeTreePanel::instance()->setNextEditObject(res);
		}
	}

	// on clear
	void QResEditor::onClearRes()
	{
		m_lineEdit->setText(Echo::StringUtil::ToString(Echo::i32(-1)).c_str());
		onEditFinished();
	}
}