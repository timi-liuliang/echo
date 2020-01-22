#pragma once

#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <engine/core/math/color.h>

using namespace std;

namespace QT_UI
{
	class QTextureSelect : public QPushButton
	{
		Q_OBJECT
	public:
		QTextureSelect( QWidget* parent = 0);

		// MVC
		static bool ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);

	protected:
		// paint event
		void paintEvent( QPaintEvent* event);

		// size hint
		QSize sizeHint() const;

	public slots:
		// on select color
		void OnSelectColor();

	private:
		Echo::String		m_texturePath;
	};
}
