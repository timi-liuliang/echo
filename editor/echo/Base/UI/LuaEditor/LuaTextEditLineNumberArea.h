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

		// area left space
		int leftSpace() const { return 3; }

		// right space
		int rightSpace() const { return 10; }

	protected:
		// paint event
		virtual void paintEvent(QPaintEvent* event) override;

	private:
		LuaTextEdit*		m_luaTextEditor;
	};
}