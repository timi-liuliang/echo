#include <QMenu>
#include <QCursor>

namespace Echo
{
	QWidget* qMenuNew(QWidget* parent)
	{
		return new QMenu(parent);
	}

	void qMenuAddAction(QWidget* menu, QAction* action)
	{
		if (menu && action)
		{
			QMenu* qMenu = qobject_cast<QMenu*>(menu);
			if (qMenu)
			{
				qMenu->addAction(action);
			}
		}
	}

	void qMenuExec(QWidget* menu)
	{
		if (menu)
		{
			QMenu* qMenu = qobject_cast<QMenu*>(menu);
			if (qMenu)
			{
				qMenu->exec(QCursor::pos());
			}
		}
	}
}