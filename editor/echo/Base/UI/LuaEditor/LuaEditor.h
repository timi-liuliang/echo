#pragma once

#include <QMainWindow>
#include "ui_LuaEditor.h"
#include "QProperty.hpp"
#include "LuaSyntaxHighLighter.h"
#include <engine/core/script/lua/LuaScript.h>

namespace Studio
{
	class LuaEditor : public QDockWidget, public Ui_ScriptEditorPanel
	{
		Q_OBJECT

	public:
		LuaEditor(QWidget* parent = 0);
		~LuaEditor();

		// open lua file
		void open( const Echo::String& tex);

		// get current edit lua file path
		const Echo::String& getCurrentLuaFilePath();

	protected slots:
		// 保存
		void save();

		// 内容被修改
		void onTextChanged();

		// 更新标题显示
		void updateTitle();

	protected:
		// size hint
		virtual QSize sizeHint() const override;

	private:
		Echo::LuaResPtr				m_luaRes;
		LuaSyntaxHighLighter*		m_luaSyntaxHighLighter;
	};
}