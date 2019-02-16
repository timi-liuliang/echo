#pragma once

#include "engine/core/base/echo_def.h"
#include "bottom_panel_tab.h"
#include "qt/QWidgets.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class Editor
	{
	public:
		Editor() {}
		virtual ~Editor() {}

		// get instance
		static Editor* instance();

		// set instance
		static void setInstance(Editor* inst);

	public:
		// show bottom panel
		virtual void showBottomPanel(BottomPanelTab* bottomPanel)=0;

	public:
		// select a node object
		virtual const String selectANodeObject()=0;

		// select a setting object
		virtual const String selectASettingObject()=0;

		// select a resource object
		virtual const String selectAResObject()=0;

		// select a object's property
		virtual const Echo::String selectAProperty(Object* objectPtr)=0;

	public:
		// get node icon by class name
		virtual Echo::String getNodeIcon(Node* node) = 0;
        
    public:
        // new QCheckBox
        qCheckBoxNewFun qCheckBoxNew = nullptr;
        
    public:
        // QComboBox->addItem(const QIcon &icon, const QString &text)
        qComboBoxAddItemFun qComboBoxAddItem = nullptr;
        
        // int QComboBox->currentIndex() const;
        qComboBoxCurrentIndexFun qComboBoxCurrentIndex = nullptr;
        
        // QString QComboBox->currentText() const;
        qComboBoxCurrentTextFun qComboBoxCurrentText = nullptr;
        
        // QComboBox->setCurrentIndex(int index);
        qComboBoxSetCurrentIndexFun qComboBoxSetCurrentIndex = nullptr;
        
        // QComboBox->setCurrentText(const QString &text);
        qComboBoxSetCurrentTextFun qComboBoxSetCurrentText = nullptr;
        
        // void QComboBox->setItemText(int index, const QString &text);
        qComboBoxSetItemTextFun qComboBoxSetItemText = nullptr;
        
        // QComboBox->clear();
        qComboBoxClearFun qComboBoxClear = nullptr;
        
    public:
        // get cursor current pos
        qCursorPosFun qCursorPos = nullptr;
        
    public:
        qObjectGetEventAllFun qObjectGetEventAll = nullptr;
        qGraphicsItemGetEventAllFun qGraphicsItemGetEventAll = nullptr;
        
    public:
        // get sender item
        qSenderItemFun qSenderItem = nullptr;
        
        // get rect
        qGraphicsItemSceneRectFun qGraphicsItemSceneRect = nullptr;
        
        // get item width
        qGraphicsItemWidthFun qGraphicsItemWidth = nullptr;
        
        // pos
        qGraphicsItemPosFun qGraphicsItemPos = nullptr;
        qGraphicsItemSetPosFun qGraphicsItemSetPos = nullptr;
        
        // set pos
        qGraphicsProxyWidgetSetPosFun qGraphicsProxyWidgetSetPos = nullptr;
        
        // set zvalue
        qGraphicsProxyWidgetSetZValueFun qGraphicsProxyWidgetSetZValue = nullptr;
        qGraphicsItemSetZValueFun qGraphicsItemSetZValue = nullptr;
        
        // set visible
        qGraphicsItemSetVisibleFun qGraphicsItemSetVisible = nullptr;
        
        // user data
        qGraphicsItemUserDataFun qGraphicsItemUserData = nullptr;
        
        // set user data
        qGraphicsItemSetUserDataFun qGraphicsItemSetUserData = nullptr;
        
        // set tool tip
        qGraphicsItemSetToolTipFun qGraphicsItemSetToolTip = nullptr;
        
        // set moveable
        qGraphicsItemSetMoveableFun qGraphicsItemSetMoveable = nullptr;
        
    public:
        // create a QGraphicsScene
        qGraphicsSceneNewFun qGraphicsSceneNew = nullptr;
        
        // add widget to QGraphicsScene
        qGraphicsSceneAddWidgetFun qGraphicsSceneAddWidget = nullptr;
        
        // add line
        qGraphicsSceneAddLineFun qGraphicsSceneAddLine = nullptr;
        
        // add rect
        qGraphicsSceneAddRectFun qGraphicsSceneAddRect = nullptr;
        
        // add path
        qGraphicsSceneAddPathFun qGraphicsSceneAddPath = nullptr;
        
        // add simple text
        qGraphicsSceneAddSimpleTextFun qGraphicsSceneAddSimpleText = nullptr;
        
        // add eclipse
        qGraphicsSceneAddEclipseFun qGraphicsSceneAddEclipse = nullptr;
        
        // remove item
        qGraphicsSceneDeleteWidgetFun qGraphicsSceneDeleteWidget = nullptr;
        
        // delete item
        qGraphicsSceneDeleteItemFun qGraphicsSceneDeleteItem = nullptr;
        
    public:
        // disable viewport update
        qGraphicsViewDisableViewportAutoUpdateFun qGraphicsViewDisableViewportAutoUpdate = nullptr;
        
        // get view port
        qGraphicsViewSceneRectFun qGraphicsViewSceneRect = nullptr;
        
        // create a QGraphicsScene
        qGraphicsViewSetSceneFun qGraphicsViewSetScene = nullptr;
        
        // scale
        qGraphicsViewSetScaleFun qGraphicsViewSetScale = nullptr;
        
    public:
        // resize section
        qHeaderViewResizeSectionFun qHeaderViewResizeSection = nullptr;
        
        // set section size mode
        qHeaderViewSetSectionResizeModeFun qHeaderViewSetSectionResizeMode = nullptr;
	};
}

#define EditorApi (*Editor::instance())

#endif
