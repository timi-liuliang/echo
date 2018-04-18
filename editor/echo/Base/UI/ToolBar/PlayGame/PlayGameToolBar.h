#pragma once

#include <QWidget>
#include "ui_PlayGameToolBar.h"

namespace Studio
{
	class PlayGameToolBar : public QWidget, public Ui_PlayGameToolBar
	{
		Q_OBJECT

	public:
		PlayGameToolBar(QWidget* parent = 0);
		~PlayGameToolBar();
	};
}