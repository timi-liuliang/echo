/*********************************************************************
 *<
	FILE: frontend.h

	DESCRIPTION: A new plug-in type that controls the main UI for MAX

	CREATED BY:	Rolf Berteig

	HISTORY: 4/01/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "custcont.h"

// layout bits
#define FE_LAYOUT_NO_TIMESLIDER			(1<<0)
#define FE_LAYOUT_NO_TOOLBAR			(1<<1)
#define FE_LAYOUT_NO_COMMAND_PANEL		(1<<2)
#define FE_LAYOUT_CONTROL_COMMAND_PANEL	(1<<3)
#define FE_LAYOUT_DEFAULT				0


// Tool bar buttons
#define TOOLBUT_HELP				0
#define TOOLBUT_UNDO				1
#define TOOLBUT_REDO				2
#define TOOLBUT_LINK				3
#define TOOLBUT_UNLINK				4
#define TOOLBUT_SPACEWARP_BIND		5
#define TOOLBUT_SELECT				6
#define TOOLBUT_REGION_TYPE			7
#define TOOLBUT_SELECT_FILTER		8
#define TOOLBUT_SELECT_BYNAME		9
#define TOOLBUT_MOVE				10
#define TOOLBUT_ROTATE				11
#define TOOLBUT_SCALE				12
#define TOOLBUT_REF_COORDSYS		13
#define TOOLBUT_REF_CENTER			14
#define TOOLBUT_CONST_X				15
#define TOOLBUT_CONST_Y				16
#define TOOLBUT_CONST_Z				17
#define TOOLBUT_CONST_PLANE			18
#define TOOLBUT_IK					19
#define TOOLBUT_MIRROR				20
#define TOOLBUT_ARRAY				21
#define TOOLBUT_ALIGN				22
#define TOOLBUT_NAMED_SELSETS		23
#define TOOLBUT_TRACKVIEW			24
#define TOOLBUT_MEDIT				25
#define TOOLBUT_RENDER				26
#define TOOLBUT_QUICK_RENDER		27
#define TOOLBUT_RENDER_LAST			29
#define TOOLBUT_SEPERATOR			30
#define TOOLBUT_LAYER_CURRENT		31
#define TOOLBUT_LAYER_PROP			32
#define TOOLBUT_LAYER_CONTROL		33
#define TOOLBUT_LAYER_OBJ_PROP		34
#define TOOLBUT_MEASURE				35
#define TOOLBUT_SCHEMATIC_VIEW		36
#define TOOLBUT_RENDER_PRESETS		37
#define TOOLBUT_SHOW_VFB			38

// Standard MAX tool buttons use IDs greater then this value.
// FE plug-in tool buttons should use IDs less then this max value
// and greater then this min value.
// Note that the actual resource IDs for standard MAX buttons will
// be converted to the above #defines when passed to ProcessToolButton().
#define MIN_FE_CUSTOM_TOOLID		1000
#define MAX_FE_CUSTOM_TOOLID		30000


// Menu items
#define MENUITEM_FILE_NEW						0
#define MENUITEM_FILE_RESET						1
#define MENUITEM_FILE_OPEN						2
#define MENUITEM_FILE_MERGE						3
#define MENUITEM_FILE_REPLACE					4
#define MENUITEM_FILE_INSERTTRACKS				5
#define MENUITEM_FILE_SAVE						6
#define MENUITEM_FILE_SAVEAS					7
#define MENUITEM_FILE_SAVESELECTED				8
#define MENUITEM_FILE_IMPORT					9
#define MENUITEM_FILE_EXPORT					10
#define MENUITEM_FILE_ARCHIVE					11
#define MENUITEM_FILE_SUMMARYINFO				12
#define MENUITEM_FILE_VIEWFILE					13
#define MENUITEM_FILE_CONFIGUREPATHS			14
#define MENUITEM_FILE_PREFERENCES				15
#define MENUITEM_FILE_EXIT						16

#define MENUITEM_EDIT_UNDO						17
#define MENUITEM_EDIT_REDO						18
#define MENUITEM_EDIT_HOLD						19
#define MENUITEM_EDIT_FETCH						20
#define MENUITEM_EDIT_DELETE					21
#define MENUITEM_EDIT_CLONE						22
#define MENUITEM_EDIT_SELECTALL					23
#define MENUITEM_EDIT_SELECTNONE				24
#define MENUITEM_EDIT_SELECTINVERT				25
#define MENUITEM_EDIT_SELECTBY_COLOR			26
#define MENUITEM_EDIT_SELECTBY_MATERIAL			27
#define MENUITEM_EDIT_SELECTBY_SMOOTHING		28
#define MENUITEM_EDIT_SELECTBY_NAME				29
#define MENUITEM_EDIT_REGION_WINDOW				30
#define MENUITEM_EDIT_REGION_CROSSING			31
#define MENUITEM_EDIT_REMOVENAMEDSELECTIONS		32
#define MENUITEM_EDIT_PROPERTIES				33

#define MENUITEM_TOOLS_TRANSFORMTYPEIN			34
#define MENUITEM_TOOLS_DISPLAYFLOATER			35
#define MENUITEM_TOOLS_SELECTIONFLOATER			36
#define MENUITEM_TOOLS_MIRROR					37
#define MENUITEM_TOOLS_ARRAY					38
#define MENUITEM_TOOLS_SNAPSHOT					39
#define MENUITEM_TOOLS_ALIGN					40
#define MENUITEM_TOOLS_ALIGNNORMALS				41
#define MENUITEM_TOOLS_PLACEHIGHLIGHT			42
#define MENUITEM_TOOLS_MTLEDIT					43
#define MENUITEM_TOOLS_MTLBROWSE				44

#define MENUITEM_GROUP_GROUP					45
#define MENUITEM_GROUP_OPEN						46
#define MENUITEM_GROUP_CLOSE					47
#define MENUITEM_GROUP_UNGROUP					48
#define MENUITEM_GROUP_EXPLODE					49
#define MENUITEM_GROUP_ATTACH					50
#define MENUITEM_GROUP_DETACH					51

#define MENUITEM_VIEWS_UNDO							52
#define MENUITEM_VIEWS_REDO							53
#define MENUITEM_VIEWS_SAVEACTIVEVIEW				54
#define MENUITEM_VIEWS_RESTOREACTIVEVIEW			55
#define MENUITEM_VIEWS_UNITSETUP					56
#define MENUITEM_VIEWS_DRAWINGAIDS					57
#define MENUITEM_VIEWS_GRIDS_SHOWHOMEGRID			58
#define MENUITEM_VIEWS_GRIDS_ACTIVATEHOMEGRID		59
#define MENUITEM_VIEWS_GRIDS_ACTIVATEGRIDOBJECT		60
#define MENUITEM_VIEWS_GRIDS_ALIGN					61
#define MENUITEM_VIEWS_IMAGEBACKGROUND				62
#define MENUITEM_VIEWS_UPDATEBKGIMAGE				63
#define MENUITEM_VIEWS_RESETBKGXFORM				64
#define MENUITEM_VIEWS_SHOWAXISICON					65
#define MENUITEM_VIEWS_SHOWGHOSTING					66
#define MENUITEM_VIEWS_SHOWKEYTIMES					67
#define MENUITEM_VIEWS_SHADESELECTED				68
#define MENUITEM_VIEWS_SHOWDEP						69
#define MENUITEM_VIEWS_MATCHCAMERATOVIEW			70
#define MENUITEM_VIEWS_REDRAW						71
#define MENUITEM_VIEWS_DEACT_MAPS					72
#define MENUITEM_VIEWS_UPDATEDURINGSPINNERDRAG		73
#define MENUITEM_VIEWS_EXPERTMODE					74
#define MENUITEM_VIEWS_VIEWPORTCONFIG				75

#define MENUITEM_RENDER_RENDER						76
#define MENUITEM_RENDER_VIDEOPOST					77
#define MENUITEM_RENDER_SHOW_LAST_IMG				78
#define MENUITEM_RENDER_ATMOSPHERE					79
#define MENUITEM_RENDER_PREVIEW						80
#define MENUITEM_RENDER_VIEWPREVIEW					81
#define MENUITEM_RENDER_RENAMEPREVIEW				82

#define MENUITEM_HELP_CONTENTS						83
#define MENUITEM_HELP_INDEX							84
#define MENUITEM_HELP_PLUGINHELP					85
#define MENUITEM_HELP_ABOUT							86

// VIZ specific
#define MENUITEM_TOOLS_SPACETOOL					87
#define MENUITEM_TOOLS_LAYERPROP					88

// JBW added MAXScript menu (11/11/98 & 12/15/98)
#define MENUITEM_MAXSCRIPT_LISTENER					89
#define MENUITEM_MAXSCRIPT_OPEN						90
#define MENUITEM_MAXSCRIPT_NEW						91
#define MENUITEM_MAXSCRIPT_RUN						92
#define MENUITEM_MAXSCRIPT_MACROREC					93


#define MENUITEM_VIEWS_ACT_MAPS					    94

#define MENUITEM_FILE_SAVECOPYAS					95

#define MENUITEM_FILE_SAVEXMLANIMATION				96
#define MENUITEM_FILE_LOADXMLANIMATION				97

#define MENUITEM_EDIT_SELECTBY_SIMILAR				98

#define MENUITEM_MAXSCRIPT_EDITOR					99

#define MENUITEM_RENDER_SHOW_VFB					100

// Standard MAX menu items use IDs greater then this value.
// FE plug-in menu items should use IDs less then this max value
// and greater then this min value.
// Note that the actual resource IDs for standard MAX menu items will
// be converted to the above #defines when passed to ProcessMenuItem().
#define MIN_FE_CUSTOM_MENUEITEM		1000
#define MAX_FE_CUSTOM_MENUEITEM		30000




// Special interface given only to front end controllers
/*! This class and its former purpose has been replaced by the
CUIFrameMgr. */
class IFrontEnd: public MaxHeapOperators {
	public:
		virtual HWND GetCommandPanelHWND()=0;
		
		virtual void *GetInterface(DWORD id)=0;

		virtual void ClearToolBar()=0;
		virtual void RemoveStandardToolButton(int id)=0;
		virtual void AddStandardToolButton(int id,int sepSize=-1)=0;
		virtual void CheckStandardTool(int id, BOOL onOff)=0;
		virtual BOOL IsStandardToolChecked(int id, BOOL onOdd)=0;
		virtual void EnableStandardTool(int id, BOOL enabled)=0;
		virtual void SetStandardToolFlyoff(int id, int fly)=0;
		virtual int GetStandardToolFlyoff(int id)=0;
		virtual ICustToolbar *GetMainToolbar()=0;

		virtual HMENU GetMainMenu()=0;
		virtual void SetMainMenu(HMENU hMenu)=0;
		virtual int MenuIDtoResID(int id)=0; // given one of the above defines, what is the corrisponding resource ID
		virtual int ResIDtoMenuID(int id)=0; // oppisite of above
	};


/*! This class and its former purpose has been replaced by the
CUIFrameMgr. */
#pragma warning(push)
#pragma warning(disable:4100)
class FrontEndController: public MaxHeapOperators
{
public:
	virtual void DeleteThis()=0;

	virtual void *GetInterface(DWORD id) {return NULL;}

	// Called once on startup. Note that MAX windows have been created
	// but not necessarily sized.
	virtual void Initialize(IFrontEnd *ife,Interface *ip) {}

	virtual DWORD GetLayout() {return FE_LAYOUT_DEFAULT;}		

	// Notifications
	virtual DWORD GeneralNotify(DWORD id, DWORD param1, DWORD param2) {return 0;}		
	virtual void SelectionChanged() {}
	virtual void TimeChanged(TimeValue t) {}
	virtual DWORD ProcessToolButton(int id, int notify) {return 0;}
	virtual DWORD ProcessMenuItem(int id, int notify) {return 0;}
	virtual DWORD ProcessInitMenu(HMENU hMenu) {return 0;}
	virtual DWORD ProcessViewportRightClick(HWND hWnd, IPoint2 m) {return 0;}
	virtual DWORD ProcessViewportLabelClick(HWND hWnd, IPoint2 m) {return 0;}
	virtual DWORD ProcessViewportMenuItem(int id, int notify) {return 0;}
	virtual DWORD ProcessViewportInitMenu(HMENU hMenu) {return 0;}
	virtual void Resize() {}
};
#pragma warning(pop)






