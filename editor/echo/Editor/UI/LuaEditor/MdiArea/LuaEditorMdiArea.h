#pragma once

#include <QMainWindow>
#include "LuaEditor.h"
#include "ui_LuaEditorMdiArea.h"

namespace Studio
{
	class LuaEditorMdiArea : public QDockWidget, public Ui_LuaEditorMdiArea
	{
		Q_OBJECT

	public:
		LuaEditorMdiArea(QWidget* parent = 0);
		~LuaEditorMdiArea();

		// open lua file
		void open(const Echo::String& fullPath);
        
        // get tab index
        bool getTabIndex(const Echo::String& fullPath, int& index);

	protected slots:
		// save
		void save();

	private:
		Echo::vector<LuaEditor*>::type	m_luaEditors;
	};
}
