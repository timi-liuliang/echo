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
	QResEditor::QResEditor(class QPropertyModel* model, QString propertyName, const char* resTypes, QWidget* parent)
		: QWidget( parent)
		, m_propertyModel(model)
		, m_propertyName(propertyName)
		, m_menu(nullptr)
	{
		m_resTypes = Echo::StringUtil::Split(resTypes, "|");
		for (const Echo::String& resType : m_resTypes)
		{
			m_exts += m_exts.empty() ? "" : "|";
			m_exts += Echo::Res::getResFunByClassName(resType)->m_ext;
		}

		// root layout
		m_horizonLayout = new QHBoxLayout( this);
		m_horizonLayout->setSpacing( 0);
		m_horizonLayout->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->setObjectName( QString::fromUtf8("horizontalLayout"));

		// LineEdit
		m_displayButton = new QPushButton( this);
		m_displayButton->setFlat(true);
		m_displayButton->setText("");
		m_displayButton->setObjectName( QString::fromUtf8("lineEdit"));
		m_displayButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget(m_displayButton);

		// ToolButton
		m_toolButton = new QToolButton( this);
		m_toolButton->setObjectName( QString::fromUtf8("toolButton"));
		m_toolButton->setText( "...");
		m_toolButton->setContentsMargins(0, 0, 0, 0);
		m_horizonLayout->addWidget( m_toolButton);

		setFocusProxy( m_toolButton);

		// connect signal and slot
		QObject::connect(m_displayButton, SIGNAL(clicked()), this, SLOT(onEdit()));
		QObject::connect(m_displayButton, SIGNAL(returnPressed()), this, SLOT(onEditFinished()));
		QObject::connect(m_toolButton, SIGNAL(clicked()), this, SLOT(onShowMenu()));

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
			m_displayButton->setMinimumHeight(m_displayButton->geometry().height()*1.6);
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
		m_propertyModel->setValue(m_propertyName, Echo::StringUtil::ToString(m_id).c_str());
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
		for (const Echo::String& resType : m_resTypes)
		{
			QAction* createAction = new QAction(m_menu);
			createAction->setText(("New " + resType).c_str());
			createAction->setData(resType.c_str());
			m_menu->addAction(createAction);
			QObject::connect(createAction, SIGNAL(triggered()), this, SLOT(onCreateRes()));
		}

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
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String classType = action->data().toString().toStdString().c_str();
			Echo::String resExt = Echo::Res::getResFunByClassName(classType)->m_ext;
			Echo::Res* res = Echo::Res::createByFileExtension(resExt.c_str(), true);
			if (res)
			{
				m_id = res->getId();
				onEditFinished();
			}
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
				m_id = res->getId();
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
		m_id = -1;
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
