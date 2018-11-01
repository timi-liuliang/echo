#include <QSplitter>
#include <engine/core/editor/qt/QSplitter.h>

namespace Echo
{
	// modify qsplitter stretch factor
	void qSplitterSetStretchFactor(void* widget, int index, int stretch)
	{
		QSplitter* splitter = (QSplitter*)widget;
		if (splitter)
		{
			splitter->setStretchFactor(index, stretch);
		}
	}
}
