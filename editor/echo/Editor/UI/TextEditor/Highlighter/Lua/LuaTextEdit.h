#pragma once

#include <QPlainTextEdit>
#include <QCompleter>
#include <engine/core/util/StringUtil.h>
#include "TextEdit.h"

namespace Studio
{
	class LuaTextEdit : public  TextEdit
	{
		Q_OBJECT

	public:
		LuaTextEdit(QWidget* parent = nullptr);
		virtual ~LuaTextEdit();

	private:
		// auto indent
		virtual void autoIndent(QKeyEvent* e);

		// auto add end 
		virtual void autoCompleteEnd(QKeyEvent* e);

	private:
		QStringList					m_keyWords;
	};
}
