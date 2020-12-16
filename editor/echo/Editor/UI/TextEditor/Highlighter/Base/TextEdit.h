#pragma once

#include <QPlainTextEdit>
#include <QCompleter>
#include <engine/core/util/StringUtil.h>
#include "SyntaxHighLighter.h"
#include "TextEditLineNumberArea.h"

namespace Studio
{
	class TextEdit : public  QPlainTextEdit
	{
		Q_OBJECT

	public:
		TextEdit(QWidget* parent = nullptr);
		virtual ~TextEdit();

		void setModel(const QStringList& words);
		QCompleter* getCompleter() const;
        
    public:
        // set syntax hightlighter
		void setSyntaxHighter(SyntaxHighLighter* highLighter);

	public:
		// draw line number paint event
		void lineNumberAreaPaintEvent(QPaintEvent* event);

		// calculates the width of the LineNumberArea widget
		int lineNumberAreaWidth();

	private slots:
		// update line number area width
		void updateLineNumberAreaWidth(int newBlockCount);

		// update line number area
		void updateLineNumberArea(const QRect& rect, int dy);

		// high light current line
		void highlightCurrentLine();

	protected:
		virtual void wheelEvent(QWheelEvent* e) override;
		virtual void mouseMoveEvent(QMouseEvent* e) override;
        virtual void mousePressEvent(QMouseEvent* e) override;
		virtual void mouseDoubleClickEvent(QMouseEvent *e) override;
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void focusInEvent(QFocusEvent* e) override;
		virtual void resizeEvent(QResizeEvent* e) override;

		virtual void autoIndent(QKeyEvent* e);
		virtual void autoCompleteEnd(QKeyEvent* e);

	protected:
		// insert completion
		void insertCompletion(const QString& completion);

		// get text under cursor
		QString textUnderCursor() const;

		// current line
		Echo::String textCurrentLine() const;

	protected:
		QCompleter*					m_completer;
		TextEditLineNumberArea*		m_lineNumberArea;
        SyntaxHighLighter*			m_syntaxHighLighter = nullptr;
	};
}
