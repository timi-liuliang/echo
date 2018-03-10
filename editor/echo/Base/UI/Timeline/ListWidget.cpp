#include "ListWidget.h"
#include <QScrollBar>

namespace Studio
{
	ListWidget::ListWidget(QWidget* parent/* = 0*/)
		: QListWidget(parent)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SIGNAL(verticalScroll(int)));
	}

	ListWidget::~ListWidget()
	{
	}

}  // namespace Studio
