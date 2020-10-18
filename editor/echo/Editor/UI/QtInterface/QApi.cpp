#include "Studio.h"
#include "QComboBox.cx"
#include "QEvent.cx"
#include "QGraphicsItem.cx"
#include "QGraphicsScene.cx"
#include "QGraphicsView.cx"
#include "QLineEdit.cx"
#include "QUiLoader.cx"
#include "QWidget.cx"
#include "QFileDialog.cx"

#define BIND_EDITOR_FUN(Fun) this->Fun = Echo::Fun

namespace Studio
{
    // init functions
    void AStudio::initFunctions()
    {
        // QComboBox->addItem(const QIcon &icon, const QString &text)
        BIND_EDITOR_FUN(qComboBoxAddItem);
        BIND_EDITOR_FUN(qComboBoxCurrentIndex);
        BIND_EDITOR_FUN(qComboBoxCurrentText);
        BIND_EDITOR_FUN(qComboBoxSetCurrentIndex);
        BIND_EDITOR_FUN(qComboBoxSetCurrentText);
        BIND_EDITOR_FUN(qComboBoxSetItemText);
        BIND_EDITOR_FUN(qComboBoxClear);
        
        // QEvent
        BIND_EDITOR_FUN(qObjectGetEventAll);
        BIND_EDITOR_FUN(qGraphicsItemGetEventAll);

		// get sender item
		BIND_EDITOR_FUN(qSenderItem);
		BIND_EDITOR_FUN(qGraphicsItemSceneRect);
		BIND_EDITOR_FUN(qGraphicsItemWidth);
		BIND_EDITOR_FUN(qGraphicsProxyWidgetSetPos);
		BIND_EDITOR_FUN(qGraphicsProxyWidgetSetZValue);
		BIND_EDITOR_FUN(qGraphicsItemSetZValue);
		BIND_EDITOR_FUN(qGraphicsItemUserData);
		BIND_EDITOR_FUN(qGraphicsItemSetUserData);
		BIND_EDITOR_FUN(qGraphicsItemSetToolTip);
		BIND_EDITOR_FUN(qGraphicsSceneNew);
		BIND_EDITOR_FUN(qGraphicsSceneAddWidget);
		BIND_EDITOR_FUN(qGraphicsSceneAddLine);
		BIND_EDITOR_FUN(qGraphicsSceneAddRect);
		BIND_EDITOR_FUN(qGraphicsSceneAddPath);
		BIND_EDITOR_FUN(qGraphicsSceneAddEclipse);
		BIND_EDITOR_FUN(qGraphicsSceneAddPixmap);

		// disable viewport update
		BIND_EDITOR_FUN(qGraphicsViewDisableViewportAutoUpdate);
		BIND_EDITOR_FUN(qGraphicsViewSceneRect);
		BIND_EDITOR_FUN(qGraphicsViewSetBackgroundBrush);
		BIND_EDITOR_FUN(qGraphicsViewFitInView);

		// new QLineEdit
		BIND_EDITOR_FUN(qLineEditNew);
		BIND_EDITOR_FUN(qLineEditSetText);
		BIND_EDITOR_FUN(qLineEditSetMaximumWidth);
		BIND_EDITOR_FUN(qLineEditSetCursorPosition);
		BIND_EDITOR_FUN(qLineEditText);
		BIND_EDITOR_FUN(qLineEditWidth);
		BIND_EDITOR_FUN(qLineEditHeight);

		// load ui file by path. then return the widget
		BIND_EDITOR_FUN(qLoadUi);

		// qt file dialog
		BIND_EDITOR_FUN(qGetOpenFileNames);

		// connect signal slot
		BIND_EDITOR_FUN(qSender);
		BIND_EDITOR_FUN(qConnectObject);
		BIND_EDITOR_FUN(qConnectWidget);
		BIND_EDITOR_FUN(qConnectAction);
		BIND_EDITOR_FUN(qConnectGraphicsItem);
		BIND_EDITOR_FUN(qFindChild);
		BIND_EDITOR_FUN(qFindChildAction);
    }
}
