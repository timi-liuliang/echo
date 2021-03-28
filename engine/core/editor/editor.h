#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/render/base/image/image.h"
#include "editor_dock_panel.h"
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
		virtual void showBottomPanel(EditorDockPanel* bottomPanel)=0;

	public:
        // select a class
        virtual const String selectAClass()=0;

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
        qObjectGetEventAllFun qObjectGetEventAll = nullptr;
        qGraphicsItemGetEventAllFun qGraphicsItemGetEventAll = nullptr;
        
    public:
        // get sender item
        qSenderItemFun qSenderItem = nullptr;
        
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

        // fit in view
        qGraphicsViewFitInViewFun qGraphicsViewFitInView = nullptr;
        
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
	};
}

#define EditorApi (*Echo::Editor::instance())

#endif
