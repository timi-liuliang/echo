#pragma once

#include <QWidget>

namespace Studio
{
	class TextEdit;
	class TextEditLineNumberArea : public QWidget
	{
	public:
		TextEditLineNumberArea(TextEdit* editor);

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
		TextEdit*		m_textEditor;
	};
}