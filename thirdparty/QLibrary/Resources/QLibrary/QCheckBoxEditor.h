#pragma once

#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <engine/core/render/render/Color.h>

using namespace std;

namespace QT_UI
{
	class QCheckBoxEditor : public QPushButton
	{
		Q_OBJECT

	public:
		// MVC‰÷»æ
		static bool ItemDelegatePaint(QPainter *painter, const QRect& rect, const string& val);
	};
}