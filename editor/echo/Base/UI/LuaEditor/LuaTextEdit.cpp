#include "LuaTextEdit.h"
#include <QTextCursor>
#include <QTextBlock>
#include <QKeyEvent>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QStringListModel>
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	LuaTextEdit::LuaTextEdit(QWidget* parent)
		: QTextEdit(parent)
		, m_completer(nullptr)
	{
		m_completer = new QCompleter(this);
		m_completer->setWidget(this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setCaseSensitivity(Qt::CaseInsensitive);
		m_completer->setWrapAround(false);

		// set words
		m_keyWords << "self" << "function" << "if" << "then" << "for" << "return" << "end" << "do";
		setModel( m_keyWords);
	}

	LuaTextEdit::~LuaTextEdit()
	{

	}

	void LuaTextEdit::setModel(const QStringList& words)
	{
		QStringListModel* model = new QStringListModel(words, m_completer);
		m_completer->setModel(model);
		m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	}

	QCompleter* LuaTextEdit::getCompleter() const
	{
		return m_completer;
	}

	void LuaTextEdit::keyPressEvent(QKeyEvent* e) 
	{
		if (m_completer->popup()->isVisible() && e->key() == Qt::Key_Return)
		{
			// 0.insert completer when key Return pressed
			insertCompletion(m_completer->currentCompletion());
			m_completer->popup()->hide();
		}
		else
		{
			if (e->key() == Qt::Key_Return)
			{
				// 1.support auto indent
				autoIndent(e);
			}
			//else if (e->key() == Qt::Key_Space)
			//{
			//	// 2.
			//}
			else
			{
				QTextEdit::keyPressEvent(e);

				// end of word
				Echo::String currentCompletionPrefix = m_completer->completionPrefix().toStdString().c_str();
				Echo::String completionPrefix = textUnderCursor().toStdString().c_str();
				if(completionPrefix.length()>0)
				{
					// modify completion prefix
					if (completionPrefix != currentCompletionPrefix)
					{
						m_completer->setCompletionPrefix(completionPrefix.c_str());
						m_completer->popup()->setCurrentIndex(m_completer->completionModel()->index(0, 0));
					}

					// popup it up!
					QRect cr = cursorRect();
					cr.setWidth(m_completer->popup()->sizeHintForColumn(0) + m_completer->popup()->verticalScrollBar()->sizeHint().width());
					m_completer->complete(cr);
				}
				else
				{
					m_completer->popup()->hide();
				}
			}
		}
	}

	void LuaTextEdit::focusInEvent(QFocusEvent* e)
	{
		if (m_completer)
			m_completer->setWidget(this);

		QTextEdit::focusInEvent(e);
	}

	void LuaTextEdit::insertCompletion(const QString& completion)
	{
		if (m_completer->widget() != this)
			return;

		QTextCursor tc = textCursor();
		int extra = completion.length() - m_completer->completionPrefix().length();
		tc.movePosition(QTextCursor::Left);
		tc.movePosition(QTextCursor::EndOfWord);
		tc.insertText(completion.right(extra));
		setTextCursor(tc);
	}

	QString LuaTextEdit::textUnderCursor() const
	{
		QTextCursor tc = textCursor();
		tc.select(QTextCursor::WordUnderCursor);
		
		return tc.selectedText();
	}

	// current line
	Echo::String LuaTextEdit::textCurrentLine() const
	{
		QString text = textCursor().block().text().trimmed();
		return text.toStdString().c_str();
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
			QTextEdit::insertPlainText("\n");
			for (int i = 0; i < tabCount+1; i++)
			{
				QTextEdit::insertPlainText("\t");
			}
		}
		else if (Echo::StringUtil::StartWith(currentLineText, "end"))
		{
			QTextEdit::insertPlainText("\n");
		}
		else
		{
			QTextEdit::insertPlainText("\n");
			for (int i = 0; i < tabCount; i++)
			{
				QTextEdit::insertPlainText("\t");
			}
		}
	}
}