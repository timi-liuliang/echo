#include <QSplitter>

namespace Echo
{
	// modify qsplitter stretch factor
	void qSplitterSetStretchFactor(QWidget* widget, int index, int stretch)
	{
		QSplitter* splitter = qobject_cast<QSplitter*>(widget);
		if (splitter)
		{
			splitter->setStretchFactor(index, stretch);
		}
	}
}
