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
	};
}

#define EditorApi (*Editor::instance())

#endif
