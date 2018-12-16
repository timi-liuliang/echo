#include <QHeaderView>

namespace Echo
{
	void qHeaderViewResizeSection(QWidget* header, int logicalIndex, int size)
	{
		if (header)
		{
			QHeaderView* headerView = qobject_cast<QHeaderView*>(header);
			if (headerView)
			{
				headerView->resizeSection(logicalIndex, size);
			}
		}
	}

	// set section size mode
	void qHeaderViewSetSectionResizeMode(QWidget* header, int logicalIndex, int mode)
	{
		if (header)
		{
			QHeaderView* headerView = qobject_cast<QHeaderView*>(header);
			if (headerView)
			{
				headerView->setSectionResizeMode(logicalIndex, QHeaderView::ResizeMode(mode));
			}
		}
	}
}