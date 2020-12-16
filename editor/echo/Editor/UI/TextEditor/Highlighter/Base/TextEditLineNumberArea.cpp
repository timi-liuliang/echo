#include "TextEditLineNumberArea.h"
#include "TextEdit.h"

namespace Studio
{
	TextEditLineNumberArea::TextEditLineNumberArea(TextEdit* editor)
		: QWidget(editor)
	{
		m_textEditor = editor;
	}

	// sizeHint
	QSize TextEditLineNumberArea::sizeHint() const
	{
		return QSize(m_textEditor->lineNumberAreaWidth(), 0);
	}

	// paint event
	void TextEditLineNumberArea::paintEvent(QPaintEvent* event)
	{
		m_textEditor->lineNumberAreaPaintEvent(event);
	}
}