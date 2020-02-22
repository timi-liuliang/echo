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

		// texture path
		const Echo::String& getTexture() { return m_texturePath; }
		void setTexture(const Echo::String& texturePath);

		// MVC
		static bool ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);

	protected:
		// paint event
		void paintEvent( QPaintEvent* event);

		// size hint
		QSize sizeHint() const;

	public slots:
		// on select color
		void OnSelectTexture();

	signals:
		//  texturePath changed
		void Signal_TextureChagned();

	private:
		Echo::String		m_texturePath;
	};
}
