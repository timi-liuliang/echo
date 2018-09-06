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
#include "LuaTextEditLineNumberArea.h"

namespace Studio
{
	LuaTextEdit::LuaTextEdit(QWidget* parent)
		: QPlainTextEdit(parent)
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

		// line number area
		m_lineNumberArea = new LuaTextEditLineNumberArea(this);
		
		QObject::connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
		QObject::connect(this, SIGNAL(updateRequest(QRect, int)), this, SLOT(updateLineNumberArea(QRect, int)));
		QObject::connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

		updateLineNumberAreaWidth(0);
		highlightCurrentLine();

		// font
		Echo::String fontSize =  AStudio::instance()->getConfigMgr()->getValue("lua_edit_font_size");
		QFont font;
		font.setFamily("Courier New");
		font.setStyleHint(QFont::Monospace);
		font.setFixedPitch(true);
		font.setPointSize( fontSize.empty() ? 10 : Echo::StringUtil::ParseI32(fontSize));
		setFont(font);

		// Tab Space
		const int tabStop = 4;  // 4 characters
		QFontMetrics metrics(font);
		setTabStopWidth(tabStop * metrics.width(' '));
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

	void LuaTextEdit::wheelEvent(QWheelEvent* e)
	{
		// not all font point size is valid. why?
		if (e->modifiers() & Qt::ControlModifier)
		{
			QFont font = this->font();
			font.setStyleHint(QFont::Monospace);
			int fontPointSize = font.pointSize();
			if (e->delta() < 0)
				fontPointSize  = Echo::Math::Clamp(fontPointSize + 1, 5, 32);
			else
				fontPointSize = Echo::Math::Clamp(fontPointSize - 1, 5, 32);

			font.setPointSize(fontPointSize);
			setFont(font);

			AStudio::instance()->getConfigMgr()->setValue("lua_edit_font_size", Echo::StringUtil::ToString(fontPointSize).c_str());
		}
	}

	void LuaTextEdit::mouseMoveEvent(QMouseEvent* e)
	{
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
				QPlainTextEdit::keyPressEvent(e);

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

		QPlainTextEdit::focusInEvent(e);
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
		QString text = textCursor().block().text();
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

	// draw line number paint event
	void LuaTextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
	{
		QPainter painter(m_lineNumberArea);
		painter.fillRect(event->rect(), QColor(83, 83, 83));

		QTextBlock block = firstVisibleBlock();
		int blockNumber = block.blockNumber();
		QRectF blockBG = blockBoundingGeometry(block);
		blockBG.translate(contentOffset());
		int top = (int)(blockBG.top());
		int bottom = top + (int)blockBoundingGeometry(block).height();

		while (block.isValid() && top <= event->rect().bottom())
		{
			if (block.isVisible() && bottom >= event->rect().top())
			{
				QString number = QString::number(blockNumber + 1);
				painter.setPen(QColor(92, 99, 112));
				painter.drawText(0, top, m_lineNumberArea->width() - m_lineNumberArea->rightSpace(), fontMetrics().height(), Qt::AlignRight | Qt::AlignVCenter, number);
			}

			block = block.next();
			top = bottom;
			bottom = top + (int)blockBoundingGeometry(block).height();
			++blockNumber;
		}
	}

	// get line number area width
	int LuaTextEdit::lineNumberAreaWidth()
	{
		int digits = 1;
		int max = std::max<int>(1, blockCount());
		while (max >= 10)
		{
			max /= 10;
			++digits;
		}

		int space = m_lineNumberArea->leftSpace() + fontMetrics().width(QLatin1Char('9')) * digits + m_lineNumberArea->rightSpace();

		return space;
	}

	// update line number area width
	void LuaTextEdit::updateLineNumberAreaWidth(int newBlockCount)
	{
		setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
	}

	// update line number area
	void LuaTextEdit::updateLineNumberArea(const QRect& rect, int dy)
	{
		if (dy)
			m_lineNumberArea->scroll(0, dy);
		else
			m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

		if (rect.contains(viewport()->rect()))
			updateLineNumberAreaWidth(0);
	}

	void LuaTextEdit::resizeEvent(QResizeEvent* e)
	{
		QPlainTextEdit::resizeEvent(e);

		QRect cr = contentsRect();
		m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
	}

	// high light current line
	void LuaTextEdit::highlightCurrentLine()
	{
		QList<QTextEdit::ExtraSelection> extraSelections;
		if (!isReadOnly())
		{
			QTextEdit::ExtraSelection selection;

			QColor lineColor = QColor(90, 90, 90);

			selection.format.setBackground(lineColor);
			selection.format.setProperty(QTextFormat::FullWidthSelection, true);
			selection.cursor = textCursor();
			selection.cursor.clearSelection();
			extraSelections.append(selection);
		}

		setExtraSelections(extraSelections);
	}
}