#include "LuaTextEditLineNumberArea.h"
#include "LuaTextEdit.h"

namespace Studio
{
	LuaTextEditLineNumberArea::LuaTextEditLineNumberArea(LuaTextEdit* editor)
		: QWidget(editor)
	{
		m_luaTextEditor = editor;
	}

	// sizeHint
	QSize LuaTextEditLineNumberArea::sizeHint() const
	{
		return QSize(m_luaTextEditor->lineNumberAreaWidth(), 0);
	}

	// paint event
	void LuaTextEditLineNumberArea::paintEvent(QPaintEvent* event)
	{
		m_luaTextEditor->lineNumberAreaPaintEvent(event);
	}
}