#include <engine/core/base/class.h>
#include "PlayGameToolBar.h"

namespace Studio
{
	PlayGameToolBar::PlayGameToolBar(QWidget* parent)
		: QWidget(parent)
	{
		setupUi(this);
	}

	PlayGameToolBar::~PlayGameToolBar()
	{

	}
}