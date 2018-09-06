#pragma once

#include <QPlainTextEdit>
#include <QCompleter>
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	class LuaTextEditLineNumberArea;
	class LuaTextEdit : public  QPlainTextEdit
	{
		Q_OBJECT

	public:
		LuaTextEdit(QWidget* parent = nullptr);
		virtual ~LuaTextEdit();

		void setModel(const QStringList& words);
		QCompleter* getCompleter() const;

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
		virtual void keyPressEvent(QKeyEvent* e) override;
		virtual void focusInEvent(QFocusEvent* e) override;
		virtual void resizeEvent(QResizeEvent* e) override;

	private:
		// insert completion
		void insertCompletion(const QString& completion);

		// get text under cursor
		QString textUnderCursor() const;

		// current line
		Echo::String textCurrentLine() const;

		// auto indent
		void autoIndent(QKeyEvent* e);

	private:
		QStringList					m_keyWords;
		QCompleter*					m_completer;
		LuaTextEditLineNumberArea*	m_lineNumberArea;
	};
}