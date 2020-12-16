#include "TextEdit.h"
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
#include "TextEditLineNumberArea.h"
#include "SyntaxHighLighter.h"

namespace Studio
{
	TextEdit::TextEdit(QWidget* parent)
		: QPlainTextEdit(parent)
		, m_completer(nullptr)
	{
		m_completer = new QCompleter(this);
		m_completer->setWidget(this);
		m_completer->setCompletionMode(QCompleter::PopupCompletion);
		m_completer->setCaseSensitivity(Qt::CaseInsensitive);
		m_completer->setWrapAround(false);

		// line number area
		m_lineNumberArea = new TextEditLineNumberArea(this);
		
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
		font.setPixelSize( fontSize.empty() ? 12 : Echo::StringUtil::ParseI32(fontSize));
		setFont(font);

		// Tab Space
		const int tabStop = 4;  // 4 characters
		QFontMetrics metrics(font);
		setTabStopWidth(tabStop * metrics.width(' '));
        
        // No wrap
        setLineWrapMode(LineWrapMode::NoWrap);
	}

	TextEdit::~TextEdit()
	{

	}

	void TextEdit::setModel(const QStringList& words)
	{
		QStringListModel* model = new QStringListModel(words, m_completer);
		m_completer->setModel(model);
		m_completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	}

	QCompleter* TextEdit::getCompleter() const
	{
		return m_completer;
	}

	void TextEdit::setSyntaxHighter(SyntaxHighLighter* highLighter)
	{ 
		m_syntaxHighLighter = highLighter;

		setModel(m_syntaxHighLighter->getKeyWords());
	}

	void TextEdit::wheelEvent(QWheelEvent* e)
	{
		// not all font point size is valid. why?
		if (e->modifiers() & Qt::ControlModifier)
		{
			QFont font = this->font();
			font.setStyleHint(QFont::Monospace);
			int fontPixelSize = font.pixelSize();
			if (e->delta() < 0)
				fontPixelSize  = Echo::Math::Clamp(fontPixelSize + 1, 5, 32);
			else
				fontPixelSize = Echo::Math::Clamp(fontPixelSize - 1, 5, 32);

			font.setPixelSize(fontPixelSize);
			setFont(font);

			AStudio::instance()->getConfigMgr()->setValue("text_edit_font_size", Echo::StringUtil::ToString(fontPixelSize).c_str());
		}
		else
		{
			QPlainTextEdit::wheelEvent(e);
		}
	}

	void TextEdit::mouseMoveEvent(QMouseEvent* e)
	{
		QPlainTextEdit::mouseMoveEvent(e);
	}
    
    void TextEdit::mousePressEvent(QMouseEvent* e)
    {
        QPlainTextEdit::mousePressEvent(e);
        
        if(m_syntaxHighLighter->removeRule( SyntaxHighLighter::RG_SelectTextBlock)>0)
			m_syntaxHighLighter->rehighlight();
    }

	void TextEdit::mouseDoubleClickEvent(QMouseEvent *e)
	{
		QPlainTextEdit::mouseDoubleClickEvent(e);

		Echo::String selectText = textUnderCursor().toStdString().c_str();
        if(!selectText.empty() && m_syntaxHighLighter)
        {
			m_syntaxHighLighter->removeRule( SyntaxHighLighter::RG_SelectTextBlock);
			m_syntaxHighLighter->appendBackgroundRule(120, 160, 200, Echo::StringUtil::Format("\\b%s\\b", selectText.c_str()), SyntaxHighLighter::RG_SelectTextBlock);
			m_syntaxHighLighter->rehighlight();
        }
	}

	void TextEdit::keyPressEvent(QKeyEvent* e)
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
				// 2.support auto complete end
			//	autoCompleteEnd(e);
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

	void TextEdit::focusInEvent(QFocusEvent* e)
	{
		if (m_completer)
			m_completer->setWidget(this);

		QPlainTextEdit::focusInEvent(e);
	}

	void TextEdit::insertCompletion(const QString& completion)
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

	QString TextEdit::textUnderCursor() const
	{
		QTextCursor tc = textCursor();
		tc.select(QTextCursor::WordUnderCursor);
		
		return tc.selectedText();
	}

	Echo::String TextEdit::textCurrentLine() const
	{
		QString text = textCursor().block().text();
		return text.toStdString().c_str();
	}

	void TextEdit::lineNumberAreaPaintEvent(QPaintEvent* event)
	{
		QPainter painter(m_lineNumberArea);
		painter.fillRect(event->rect(), QColor(77, 77, 77));

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

	int TextEdit::lineNumberAreaWidth()
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

	void TextEdit::updateLineNumberAreaWidth(int newBlockCount)
	{
		setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
	}

	// update line number area
	void TextEdit::updateLineNumberArea(const QRect& rect, int dy)
	{
		if (dy)
			m_lineNumberArea->scroll(0, dy);
		else
			m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

		if (rect.contains(viewport()->rect()))
			updateLineNumberAreaWidth(0);
	}

	void TextEdit::resizeEvent(QResizeEvent* e)
	{
		QPlainTextEdit::resizeEvent(e);

		QRect cr = contentsRect();
		m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
	}

	void TextEdit::highlightCurrentLine()
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

	void TextEdit::autoCompleteEnd(QKeyEvent* e)
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
			textCursor().movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor, 2);
			setTextCursor(curCursor);
		}
	}

	// auto indent
	void TextEdit::autoIndent(QKeyEvent* e)
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
			for (int i = 0; i < tabCount + 1; i++)
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
