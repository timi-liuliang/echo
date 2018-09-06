#pragma once

#include <QWidget>

namespace Studio
{
	class LuaTextEdit;
	class LuaTextEditLineNumberArea : public QWidget
	{
	public:
		LuaTextEditLineNumberArea(LuaTextEdit* editor);

		// sizeHint
		virtual QSize sizeHint() const override;

	protected:
		// paint event
		virtual void paintEvent(QPaintEvent* event) override;

	private:
		LuaTextEdit*		m_luaTextEditor;
	};
}