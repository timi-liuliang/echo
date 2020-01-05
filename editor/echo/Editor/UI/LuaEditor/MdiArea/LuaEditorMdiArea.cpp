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
        
        QObject::connect(m_tabWidgetScript, SIGNAL(currentChanged(int)), this, SLOT(onTabIdxChanged(int)));
	}

	LuaEditorMdiArea::~LuaEditorMdiArea()
	{
		EchoSafeDeleteContainer(m_luaEditors, LuaEditor);
	}

	void LuaEditorMdiArea::open(const Echo::String& fullPath, bool isRememberOpenStates)
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

                QObject::connect(newEditor, SIGNAL(titleChanged(LuaEditor*)), this, SLOT(onLuaEditorTitleChanged(LuaEditor*)));
                
                m_luaEditors.push_back(newEditor);
            }
        }
        
        m_tabWidgetScript->setTabToolTip(tabIndex, fullPath.c_str());
        this->setVisible(true);
        
        rememberScriptOpenStates();
	}

    void LuaEditorMdiArea::rememberScriptOpenStates()
    {
        Echo::StringArray openedFiles;
        for(int i=0; i<m_tabWidgetScript->count(); i++)
        {
            LuaEditor* luaEditor = qobject_cast<LuaEditor*>(m_tabWidgetScript->widget(i));
            if(luaEditor)
                openedFiles.push_back(luaEditor->getCurrentLuaFilePath());
        }
        
        ConfigMgr* configMgr = AStudio::instance()->getConfigMgr();
        if(configMgr)
        {
            configMgr->setValue("luascripteditor_current_file", Echo::StringUtil::ToString(openedFiles,";").c_str());
            configMgr->setValue("luascripteditor_current_file_index", Echo::StringUtil::ToString(m_tabWidgetScript->currentIndex()).c_str());
        }
    }

    void LuaEditorMdiArea::recoverScriptOpenStates()
    {
        Echo::String currentEditLuaScript = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_current_file");
        Echo::String currentIndex = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_current_file_index");
        Echo::StringArray luaScripts = Echo::StringUtil::Split(currentEditLuaScript, ";");
        for(Echo::String& luaScript : luaScripts)
            open(luaScript, false);
        
        if(!currentIndex.empty())
            m_tabWidgetScript->setCurrentIndex(Echo::StringUtil::ParseI32( currentIndex));
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

    void LuaEditorMdiArea::onTabIdxChanged(int idx)
    {
        rememberScriptOpenStates();
    }

    void LuaEditorMdiArea::onLuaEditorTitleChanged(LuaEditor* editor)
    {
        if(editor)
        {
            int tabIndex = 0;
            if(getTabIndex(editor->getCurrentLuaFilePath(), tabIndex))
            {
                m_tabWidgetScript->setTabText(tabIndex, editor->windowTitle());
            }
        }
    }

	void LuaEditorMdiArea::save()
	{
		for (LuaEditor* luaEditor : m_luaEditors)
		{
			luaEditor->save();
		}
	}
}
