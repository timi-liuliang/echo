#include "ProjectMgr.h"
#include <QFileDialog>
#include "MainWindow.h"
#include "LuaEditorMdiArea.h"
#include "Studio.h"
#include <engine/core/util/PathUtil.h>

namespace Studio
{
	LuaEditorMdiArea::LuaEditorMdiArea(QWidget* parent)
		: QDockWidget(parent)
	{
		setupUi( this);
	}

	LuaEditorMdiArea::~LuaEditorMdiArea()
	{
		EchoSafeDeleteContainer(m_luaEditors, LuaEditor);
	}

	void LuaEditorMdiArea::open(const Echo::String& fullPath)
	{
		LuaEditor* newEditor = EchoNew(LuaEditor(m_mdiArea));
		if (newEditor)
		{
			m_mdiArea->addSubWindow(newEditor);
			newEditor->open(fullPath);
			newEditor->show();

			m_luaEditors.push_back(newEditor);
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
