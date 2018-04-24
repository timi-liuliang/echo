#pragma once

#include <QMainWindow>
#include "ui_LuaEditor.h"
#include "QProperty.hpp"
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	class LuaEditor : public QWidget, public Ui_LuaEditor
	{
		Q_OBJECT

	public:
		LuaEditor(QWidget* parent = 0);
		~LuaEditor();

		// 显示纹理
		void open( const Echo::String& tex);

	protected slots:
		// 保存
		void save();

		// 内容被修改
		void onTextChanged();

		// 更新标题显示
		void updateTitle();

	private:
		Echo::String				m_fullPath;					// 全路径
		QString						m_origContent;				// 文件中内容
		LuaSyntaxHighLighter*		m_luaSyntaxHighLighter;
	};
}