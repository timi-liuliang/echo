#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "LuaEditorMdiArea.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

static const char* customStyleSheet = R"(
QTabBar::tab
{
    background:rgb(66,66,66);
    color : rgb(180,180,180);
    font : bold;
    padding: 3 7px;
    min-width: 80px;

    /*order-left:1px solid rgb(66,66,66);*/
    border: 1px solid rgb(56,56,56);
}

QTabBar::tab:selected,QTabBar::tab:hover
{
    background:rgb(77,77,77);
    color:rgb(243,243,243);

    border-bottom: none;
}
)";

namespace Studio
{
	LuaEditorMdiArea::LuaEditorMdiArea(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi( this);
        
        this->setStyleSheet(customStyleSheet);
        
        m_tabWidgetScript->clear();
	}

	LuaEditorMdiArea::~LuaEditorMdiArea()
	{
		EchoSafeDeleteContainer(m_luaEditors, LuaEditor);
	}

	void LuaEditorMdiArea::open(const Echo::String& fullPath)
	{
        int tabIndex = 0;
        if(getTabIndex(fullPath, tabIndex))
        {
            m_tabWidgetScript->setCurrentIndex( tabIndex);
        }
        else
        {
            LuaEditor* newEditor = EchoNew(LuaEditor(this));
            if (newEditor)
            {
                Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
                tabIndex = m_tabWidgetScript->addTab(newEditor, fileName.c_str());
                m_tabWidgetScript->setCurrentIndex( tabIndex);
                
                newEditor->open(fullPath);
                newEditor->show();

                m_luaEditors.push_back(newEditor);
            }
        }
        
        m_tabWidgetScript->setTabToolTip(tabIndex, fullPath.c_str());
	}

    bool LuaEditorMdiArea::getTabIndex(const Echo::String& fullPath, int& index)
    {
        for(int i=0; i<m_tabWidgetScript->count(); i++)
        {
            LuaEditor* luaEditor = qobject_cast<LuaEditor*>(m_tabWidgetScript->widget(i));
            if(luaEditor && luaEditor->getCurrentLuaFilePath() == fullPath)
            {
                index = i;
                return true;
            }
        }
        
        return false;
    }

	void LuaEditorMdiArea::save()
	{
		for (LuaEditor* luaEditor : m_luaEditors)
		{
			luaEditor->save();
		}
	}
}
