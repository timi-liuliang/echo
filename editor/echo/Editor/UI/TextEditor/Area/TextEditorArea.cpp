#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "TextEditorArea.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

namespace Studio
{
	TextEditorArea::TextEditorArea(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi( this);
        
        m_tabWidgetScript->clear();
        
        QObject::connect(m_tabWidgetScript, SIGNAL(currentChanged(int)), this, SLOT(onTabIdxChanged(int)));
        QObject::connect(m_tabWidgetScript->tabBar(), SIGNAL(tabCloseRequested(int)), this, SLOT(onRequestCloseTab(int)));
	}

	TextEditorArea::~TextEditorArea()
	{
		EchoSafeDeleteContainer(m_textEditors, TextEditor);
	}

	void TextEditorArea::open(const Echo::String& fullPath, bool isRememberOpenStates)
	{
        int tabIndex = 0;
        if(getTabIndex(fullPath, tabIndex))
        {
            m_tabWidgetScript->setCurrentIndex( tabIndex);
        }
        else
        {
            TextEditor* newEditor = EchoNew(TextEditor(this));
            if (newEditor)
            {
                Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
                tabIndex = m_tabWidgetScript->addTab(newEditor, fileName.c_str());
                m_tabWidgetScript->setCurrentIndex( tabIndex);
                
                newEditor->open(fullPath);
                newEditor->show();

                QObject::connect(newEditor, SIGNAL(titleChanged(TextEditor*)), this, SLOT(onTextEditorTitleChanged(TextEditor*)));
                
                m_textEditors.emplace_back(newEditor);
            }
        }
        
        m_tabWidgetScript->setTabToolTip(tabIndex, fullPath.c_str());
        this->setVisible(true);
        
        rememberScriptOpenStates();
	}

    void TextEditorArea::rememberScriptOpenStates()
    {
        Echo::StringArray openedFiles;
        for(int i=0; i<m_tabWidgetScript->count(); i++)
        {
            TextEditor* luaEditor = qobject_cast<TextEditor*>(m_tabWidgetScript->widget(i));
            if(luaEditor)
                openedFiles.emplace_back(luaEditor->getFilePath());
        }
        
        EditorConfig* configMgr = AStudio::instance()->getConfigMgr();
        if(configMgr)
        {
            configMgr->setValue("luascripteditor_current_file", Echo::StringUtil::ToString(openedFiles,";").c_str());
            configMgr->setValue("luascripteditor_current_file_index", Echo::StringUtil::ToString(m_tabWidgetScript->currentIndex()).c_str());
        }
    }

    void TextEditorArea::recoverScriptOpenStates()
    {
        Echo::String currentEditLuaScript = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_current_file");
        Echo::String currentIndex = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_current_file_index");
        Echo::StringArray luaScripts = Echo::StringUtil::Split(currentEditLuaScript, ";");
        for(Echo::String& luaScript : luaScripts)
            open(luaScript, false);
        
        if(!currentIndex.empty())
            m_tabWidgetScript->setCurrentIndex(Echo::StringUtil::ParseI32( currentIndex));
    }

    bool TextEditorArea::getTabIndex(const Echo::String& fullPath, int& index)
    {
        for(int i=0; i<m_tabWidgetScript->count(); i++)
        {
            TextEditor* luaEditor = qobject_cast<TextEditor*>(m_tabWidgetScript->widget(i));
            if(luaEditor && luaEditor->getFilePath() == fullPath)
            {
                index = i;
                return true;
            }
        }
        
        return false;
    }

    void TextEditorArea::onTabIdxChanged(int idx)
    {
        rememberScriptOpenStates();
    }

    void TextEditorArea::onTextEditorTitleChanged(TextEditor* editor)
    {
        if(editor)
        {
            int tabIndex = 0;
            if(getTabIndex(editor->getFilePath(), tabIndex))
            {
                m_tabWidgetScript->setTabText(tabIndex, editor->windowTitle());
            }
        }
    }

    void TextEditorArea::onRequestCloseTab(int idx)
    {
        if(idx<m_tabWidgetScript->count())
            m_tabWidgetScript->removeTab(idx);

        rememberScriptOpenStates();

        if (!m_tabWidgetScript->count())
            close();
    }

	void TextEditorArea::save()
	{
		for (TextEditor* luaEditor : m_textEditors)
		{
			luaEditor->save();
		}
	}
}
