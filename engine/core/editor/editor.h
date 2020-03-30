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
		// show panel
        virtual void showCenterPanel(PanelTab * bottomPanel) = 0;
		virtual void showBottomPanel(PanelTab* bottomPanel)=0;

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

        // set parent
        qGrphicsItemSetParentItemFun qGraphicsItemSetParentItem = nullptr;
        
        // get rect
        qGraphicsItemSceneRectFun qGraphicsItemSceneRect = nullptr;
        
        // get item width
        qGraphicsItemWidthFun qGraphicsItemWidth = nullptr;
        
        // pos
        qGraphicsItemPosFun qGraphicsItemPos = nullptr;
        qGraphicsItemSetPosFun qGraphicsItemSetPos = nullptr;
        
        // set pos
        qGraphicsProxyWidgetSetPosFun qGraphicsProxyWidgetSetPos = nullptr;
        
        // set z value
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
        
        // set move able
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

        // add pixmap
        qGraphicsSceneAddPixmapFun qGraphicsSceneAddPixmap = nullptr;
        
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

        // fit in view
        qGraphicsViewFitInViewFun qGraphicsViewFitInView = nullptr;
        
    public:
        // resize section
        qHeaderViewResizeSectionFun qHeaderViewResizeSection = nullptr;
        
        // set section size mode
        qHeaderViewSetSectionResizeModeFun qHeaderViewSetSectionResizeMode = nullptr;
        
    public:
        // new QLineEdit
        qLineEditNewFun qLineEditNew = nullptr;
        
        // set text
        qLineEditSetTextFun qLineEditSetText = nullptr;
        
        // set maximum width
        qLineEditSetMaximumWidthFun qLineEditSetMaximumWidth = nullptr;
        
        // set cursor position
        qLineEditSetCursorPositionFun qLineEditSetCursorPosition = nullptr;
        
        // get text of QLineEdit
        qLineEditTextFun qLineEditText = nullptr;
        
        // width
        qLineEditWidthFun qLineEditWidth = nullptr;
        
        // height
        qLineEditHeightFun qLineEditHeight = nullptr;

    public:
		// new QLineEdit
		qSpinBoxNewFun qSpinBoxNew = nullptr;

		// set text
		qSpinBoxSetValueFun qSpinBoxSetValue = nullptr;

		// set maximum width
		qSpinBoxValueFun qSpinBoxValue = nullptr;
        
    public:
        // create a new QMenu
        qMenuNewFun qMenuNew = nullptr;
        
        // void addAction(QAction *action)
        qMenuAddActionFun qMenuAddAction = nullptr;

        // add section
        qMenuAddSeparatorFun qMenuAddSeparator = nullptr;
        
        // QAction *exec();
        qMenuExecFun qMenuExec = nullptr;
        
    public:
        // modify qsplitter stretch factor
        qSplitterSetStretchFactorFun qSplitterSetStretchFactor = nullptr;
        
    public:
        // set tool button fixed width
        qToolButtonSetFixedWidthFun qToolButtonSetFixedWidth = nullptr;
        
        // set tool button icon
        qToolButtonSetIconFun qToolButtonSetIcon = nullptr;
        
        // set icon size
        qToolButtonSetIconSizeFun qToolButtonSetIconSize = nullptr;
        
        // is toolbutton checked
        qToolButtonIsCheckedFun qToolButtonIsChecked = nullptr;
        
    public:
        // load ui file by path. then return the widget
        qLoadUiFun qLoadUi = nullptr;
        
    public:
        // invisible root item
        qTreeWidgetInvisibleRootItemFun qTreeWidgetInvisibleRootItem = nullptr;
        
        // current select item
        qTreeWidgetCurrentItemFun qTreeWidgetCurrentItem = nullptr;
        
        // header
        qTreeWidgetHeaderFun qTreeWidgetHeader = nullptr;
        
        // current column
        qTreeWidgetCurrentColumnFun qTreeWidgetCurrentColumn = nullptr;
        
        // get width
        qTreeWidgetWidthFun qTreeWidgetWidth = nullptr;
        
        // clear
        qTreeWidgetClearFun qTreeWidgetClear = nullptr;
        
    public:
        // create a new QTreeWidgetItem
        qTreeWidgetItemNewFun qTreeWidgetItemNew = nullptr;
        
        // get parent
        qTreeWidgetItemParentFun qTreeWidgetItemParent = nullptr;
        
        // add child item
        qTreeWidgetItemAddChildFun qTreeWidgetItemAddChild = nullptr;
        
        // set text
        qTreeWidgetItemSetTextFun qTreeWidgetItemSetText = nullptr;
        
        // get text
        qTreeWidgetItemTextFun qTreeWidgetItemText = nullptr;
        
        // set icon
        qTreeWidgetItemSetIconFun qTreeWidgetItemSetIcon = nullptr;
        
        // get user data
        qTreeWidgetItemUserDataFun qTreeWidgetItemUserData = nullptr;
        
        // set user data
        qTreeWidgetItemSetUserDataFun qTreeWidgetItemSetUserData = nullptr;

        // set editable
        qTreeWidgetItemSetEditableFun qTreeWidgetItemSetEditable = nullptr;
        
        // expand QTreeWidgetItem
        qTreeWidgetItemSetExpandedFun qTreeWidgetItemSetExpanded = nullptr;

    public:
        // dialog
        qDialogExecFun qDialogExec = nullptr;

    public:
        // get open file names
        qGetOpenFileNamesFun qGetOpenFileNames = nullptr;
        
    public:
        // connect signal slot
        qConnectObjectFun qConnectObject = nullptr;
        qConnectWidgetFun qConnectWidget = nullptr;
        qConnectActionFun qConnectAction = nullptr;
        qConnectGraphicsItemFun qConnectGraphicsItem = nullptr;
        
        // find child widget by name
        qFindChildFun qFindChild = nullptr;
        
        // find child action by name
        qFindChildActionFun qFindChildAction = nullptr;
        
        // set visible
        qWidgetSetVisibleFun qWidgetSetVisible = nullptr;
        
        // set enabled
        qWidgetSetEnableFun qWidgetSetEnable = nullptr;
        
        // delete QWidget
        qDeleteWidgetFun qDeleteWidget = nullptr;
	};
}

#define EditorApi (*Editor::instance())

#endif
