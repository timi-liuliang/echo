#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/render/base/image/image.h"
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
		// center panel
        virtual void showCenterPanel(QDockWidget * panel, float widthRatio=0.75) = 0;
        virtual void removeCenterPanel(QDockWidget* panel)=0;

        // bottom panel
		virtual void showBottomPanel(PanelTab* bottomPanel)=0;

	public:
		// select a node object
		virtual const String selectANodeObject()=0;

		// select a setting object
		virtual const String selectASettingObject()=0;

		// select a resource object
		virtual const String selectAResObject(const char* exts)=0;

		// select a object's property
		virtual const String selectAProperty(Object* objectPtr)=0;

        // select a file in os
        virtual const String selectAFile(const char* title, const char* exts)=0;

	public:
		// get node icon by class name
		virtual Echo::ImagePtr getNodeIcon(Node* node) = 0;

    public:
        // show object's property in property panel
        virtual void showObjectProperty(Object* object)=0;
        
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
        qObjectGetEventAllFun qObjectGetEventAll = nullptr;
        qGraphicsItemGetEventAllFun qGraphicsItemGetEventAll = nullptr;
        
    public:
        // get sender item
        qSenderItemFun qSenderItem = nullptr;
        
        // get rect
        qGraphicsItemSceneRectFun qGraphicsItemSceneRect = nullptr;
        
        // get item width
        qGraphicsItemWidthFun qGraphicsItemWidth = nullptr;
        
        // set pos
        qGraphicsProxyWidgetSetPosFun qGraphicsProxyWidgetSetPos = nullptr;
        
        // set z value
        qGraphicsProxyWidgetSetZValueFun qGraphicsProxyWidgetSetZValue = nullptr;
        qGraphicsItemSetZValueFun qGraphicsItemSetZValue = nullptr;       
        
        // user data
        qGraphicsItemUserDataFun qGraphicsItemUserData = nullptr;
        
        // set user data
        qGraphicsItemSetUserDataFun qGraphicsItemSetUserData = nullptr;
        
        // set tool tip
        qGraphicsItemSetToolTipFun qGraphicsItemSetToolTip = nullptr;
        
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
       
        // add eclipse
        qGraphicsSceneAddEclipseFun qGraphicsSceneAddEclipse = nullptr;

        // add pixmap
        qGraphicsSceneAddPixmapFun qGraphicsSceneAddPixmap = nullptr;
        
    public:
        // disable viewport update
        qGraphicsViewDisableViewportAutoUpdateFun qGraphicsViewDisableViewportAutoUpdate = nullptr;
        
        // get view port
        qGraphicsViewSceneRectFun qGraphicsViewSceneRect = nullptr;

        // set background brush
        qGraphicsViewSetBackgroundBrushFun qGraphicsViewSetBackgroundBrush = nullptr;

        // fit in view
        qGraphicsViewFitInViewFun qGraphicsViewFitInView = nullptr;
        
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
        // load ui file by path. then return the widget
        qLoadUiFun qLoadUi = nullptr;      
        
    public:
        // sender
        qSenderFun qSender = nullptr;

        // connect signal slot
        qConnectObjectFun qConnectObject = nullptr;
        qConnectWidgetFun qConnectWidget = nullptr;
        qConnectActionFun qConnectAction = nullptr;
        qConnectGraphicsItemFun qConnectGraphicsItem = nullptr;
        
        // find child widget by name
        qFindChildFun qFindChild = nullptr;
        
        // find child action by name
        qFindChildActionFun qFindChildAction = nullptr;
	};
}

#define EditorApi (*Echo::Editor::instance())

#endif
