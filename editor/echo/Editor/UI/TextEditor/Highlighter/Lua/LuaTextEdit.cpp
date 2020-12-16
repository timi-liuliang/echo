#include "LuaTextEdit.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QKeyEvent>
#include <QPainter>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>
#include <QFontMetrics>
#include <engine/core/math/Math.h>
#include <engine/core/util/StringUtil.h>
#include <engine/core/log/Log.h>
#include "Studio.h"
#include "LuaSyntaxHighLighter.h"

namespace Studio
{
	LuaTextEdit::LuaTextEdit(QWidget* parent)
		: TextEdit(parent)
	{
		// set words
		m_keyWords << "self" << "function" << "if" << "then" << "for" << "return" << "end" << "do" << "pairs" << "ipairs";
		setModel( m_keyWords);
	}

	LuaTextEdit::~LuaTextEdit()
	{

	}

	void LuaTextEdit::autoCompleteEnd(QKeyEvent* e)
	{
		if (e->key() != Qt::Key_Space)
			return;

		int tabCount = 0;
		Echo::String currentLineText = textCurrentLine();
		for (size_t i = 0; i < currentLineText.size(); i++)
		{
			if (currentLineText[i] == '\t')
			{
				tabCount++;
			}
			else
			{
				currentLineText = currentLineText.substr(tabCount, currentLineText.size() - tabCount);
				break;
			}
		}

		if (/*Echo::StringUtil::StartWith(currentLineText, "for") ||
			Echo::StringUtil::StartWith(currentLineText, "if") ||*/
			Echo::StringUtil::StartWith(currentLineText, "function"))
		{
			QPlainTextEdit::insertPlainText(" \n");
			for (int i = 0; i < tabCount; i++)
				QPlainTextEdit::insertPlainText("\t");

			QPlainTextEdit::insertPlainText("end\n");

			QTextCursor curCursor = textCursor();
			textCursor().movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor,  2);
			setTextCursor(curCursor);
		}
	}

	// auto indent
	void LuaTextEdit::autoIndent(QKeyEvent* e)
	{
		if (e->key() != Qt::Key_Return)
			return;

		int tabCount = 0;
		Echo::String currentLineText = textCurrentLine();
		for (size_t i = 0; i < currentLineText.size(); i++)
		{
			if (currentLineText[i] == '\t')
			{
				tabCount++;
			}
			else
			{
				currentLineText = currentLineText.substr(tabCount, currentLineText.size() - tabCount);
				break;
			}		
		}

		if (Echo::StringUtil::StartWith(currentLineText, "for") || 
			Echo::StringUtil::StartWith(currentLineText, "if") || 
			Echo::StringUtil::StartWith(currentLineText, "function"))
		{
			QPlainTextEdit::insertPlainText("\n");
			for (int i = 0; i < tabCount+1; i++)
			{
				QPlainTextEdit::insertPlainText("\t");
			}
		}
		else if (Echo::StringUtil::StartWith(currentLineText, "end"))
		{
			QPlainTextEdit::insertPlainText("\n");
		}
		else
		{
			QPlainTextEdit::insertPlainText("\n");
			for (int i = 0; i < tabCount; i++)
			{
				QPlainTextEdit::insertPlainText("\t");
			}
		}
	}
}
