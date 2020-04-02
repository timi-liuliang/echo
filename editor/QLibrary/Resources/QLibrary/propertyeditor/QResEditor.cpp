#include "QResEditor.h"
#include <QFileDialog>
#include "QPropertyModel.h"
#include "ResChooseDialog.h"
#include "NodeTreePanel.h"
#include "PathChooseDialog.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/resource/Res.h>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QResEditor::QResEditor(class QPropertyModel* model, QString propertyName, const char* resType, QWidget* parent)
		: QWidget( parent)
        , m_resType(resType)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
		, m_menu(nullptr)
	{
		m_exts = Echo::Res::getResFunByClassName(m_resType)->m_ext;

		// ²¼¾Ö¿Ø¼þ
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

		// connect signal and slot
		QObject::connect( m_toolButton, SIGNAL(clicked()), this, SLOT(onShowMenu()));
		QObject::connect(m_lineEdit, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));

		adjustHeightSize();
	}

	void QResEditor::onShowMenu()
	{
		showMenu(QCursor::pos());
	}

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

	void QResEditor::adjustHeightSize()
	{
		if (isTextureRes())
		{
			m_lineEdit->setMinimumHeight(m_lineEdit->geometry().height()*1.6);
			m_toolButton->setMinimumHeight(m_toolButton->geometry().height() * 1.6);
		}
	}

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

	void QResEditor::onEditFinished()
	{
		m_propertyModel->setValue(m_propertyName, m_lineEdit->text().toStdString().c_str());
	}

	bool QResEditor::ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val)
	{
		Echo::String text = "None";

		Echo::ui32 id = Echo::StringUtil::ParseI32(val);
		Echo::Object* obj = Echo::Object::getById(id);
		if (obj)
		{
			Echo::Res* res = ECHO_DOWN_CAST<Echo::Res*>(obj);
			if (res)
			{
				if(res->getPath().empty())
					text = Echo::StringUtil::Format("%s:[%d]", res->getClassName().c_str(), res->getId());
				else
					text = Echo::StringUtil::Format("%s", res->getPath().c_str());
			}
		}

		QRect textRect(rect.left() + 3, rect.top() + 3, rect.width() - 6, rect.height() - 6);
		QFont font = painter->font(); font.setBold(false);
		painter->setFont(font);
		painter->setPen(QColor(232, 232, 232));
		painter->drawText(textRect, Qt::AlignLeft, text.c_str());

		return true;
	}

	void QResEditor::showMenu(const QPoint& point)
	{
		EchoSafeDelete(m_menu, QMenu);
		m_menu = EchoNew(QMenu(this));

		// new
		QAction* createAction = new QAction(m_menu);
		createAction->setText(("New " + m_resType).c_str());
		m_menu->addAction(createAction);
        QObject::connect(createAction, SIGNAL(triggered()), this, SLOT(onCreateRes()));

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
            
            m_menu->addSeparator();
            
            // save
            QAction* saveAction = new QAction(m_menu);
            saveAction->setText("Save");
            m_menu->addAction(saveAction);
            QObject::connect(saveAction, SIGNAL(triggered()), this, SLOT(onSaveRes()));
		}

		m_menu->exec(QCursor::pos());
	}

    void QResEditor::onCreateRes()
    {
        Echo::Res* res = Echo::Res::createByFileExtension(m_exts.c_str(), true);
        if (res)
        {
            m_lineEdit->setText(Echo::StringUtil::ToString(res->getId()).c_str());
            onEditFinished();
        }
    }

	void QResEditor::onLoad()
	{
		Echo::String qFileName = Studio::ResChooseDialog::getSelectingFile(this, m_exts.c_str(), "", "");
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

	void QResEditor::onClearRes()
	{
		m_lineEdit->setText(Echo::StringUtil::ToString(Echo::i32(-1)).c_str());
		onEditFinished();
	}

    void QResEditor::onSaveRes()
    {
        Echo::ui32 id = Echo::StringUtil::ParseI32(GetId().toStdString().c_str());
        Echo::Object* obj = Echo::Object::getById(id);
        if (obj)
        {
            Echo::Res* res = ECHO_DOWN_CAST<Echo::Res*>(obj);
            if(res)
            {
                if(res->getPath().empty())
                {
                    Echo::String savePath = Studio::PathChooseDialog::getExistingPathName(this, m_exts.c_str(), "Save").toStdString().c_str();
                    if (!savePath.empty() && !Echo::PathUtil::IsDir(savePath))
                    {
                        Echo::String resPath;
                        if(Echo::IO::instance()->convertFullPathToResPath(savePath, resPath))
                        {
                            res->setPath(resPath);
                            res->save();
                        }
                    }
                }
                else
                {
                    res->save();
                }
            }
        }
    }
}
