/**********************************************************************
 *<
	FILE: itreevw.h

	DESCRIPTION: Tree View Interface

	CREATED BY:	Rolf Berteig

	HISTORY: Created 17 April 1995
			 Moved into public SDK, JBW 5.25.00
			 Extended by Adam Felt for R5

 *>	Copyright (c) 1994-2002, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "actiontable.h"
#include "iFnPub.h"
#include "point2.h"
// forward declarations
class Animatable;

#define WM_TV_SELCHANGED			WM_USER + 0x03001
#define WM_TV_MEDIT_TV_DESTROYED	WM_USER + 0x03b49

// Sent to a track view window to force it to do a horizontal zoom extents
#define WM_TV_DOHZOOMEXTENTS		WM_USER + 0xb9a1

// Style flag options for Interface::CreateTreeViewChild
#define TVSTYLE_MAXIMIZEBUT			(1<<0) // Has a maximize button for TV in a viewport.
#define TVSTYLE_INVIEWPORT			(1<<1)
//Setting the TVSTYLE_NAMEABLE flag will give the user maxscript access to your treeview through the trackviews.current interface.   
//Set this flag if you want the user to be able to modify your trackview settings.
#define TVSTYLE_NAMEABLE			(1<<2)
#define TVSTYLE_INMOTIONPAN			(1<<3)
#define TVSTYLE_SHOW_NONANIMATABLE	(1<<4)
// this flag specifies the treeview child window contains a menu bar and toolbars. (R5 and later only)
// if this style has been set you can use ITreeViewUI::GetHWnd() to get the handle to the window housing the CUI elements.
// this automatically sets the TVSTYLE_NAMEABLE flag since UI macroScripts may depend in the window being scriptable
#define TVSTYLE_SHOW_CUI			(1<<5)

// Docking options for ITrackViewArray::OpenTrackViewWindow
#define TV_FLOAT		0	// float window.  can't dock on top (the default)
#define TV_DOCK_TOP		1	// dock on top
#define TV_DOCK_BOTTOM	2	// dock on bottom.  can't dock on top
#define TV_CAN_DOCK_TOP	3	// floating but able to dock on top

// Major modes
#define MODE_EDITKEYS		0
#define MODE_EDITTIME		1
#define MODE_EDITRANGES		2
#define MODE_POSRANGES		3
#define MODE_EDITFCURVE		4

// Operations on keys can be performed on one of the following set types
#define EFFECT_ALL_SEL_KEYS				0
#define EFFECT_SEL_KEYS_IN_SEL_TRACKS	1
#define EFFECT_ALL_KEYS_IN_SEL_TRACKS	2
#define EFFECT_ALL_KEYS					3


class TrackViewFilter;
class TrackViewPick;

typedef Animatable* AnimatablePtr;

//This is an interface into many of the UI layout functions of trackview.
//You can get an instance of this class by calling ITreeView::GetInterface(TREEVIEW_UI_INTERFACE)
//***********************************************************************************************
#define TREEVIEW_UI_INTERFACE Interface_ID(0x1bcd78ef, 0x21990819)
class ITreeViewUI : public FPMixinInterface
{
public:
	virtual HWND GetHWnd()=0;

	virtual MSTR GetMenuBar()=0;
	virtual void SetMenuBar(MSTR name)=0;

	virtual MSTR GetControllerQuadMenu()=0;
	virtual void SetControllerQuadMenu(MSTR name)=0;
	virtual MSTR GetKeyQuadMenu()=0;
	virtual void SetKeyQuadMenu(MSTR name)=0;
	
	virtual int ToolbarCount()=0;
	virtual void AddToolbar()=0;
	virtual void DeleteToolbar()=0;
	virtual bool AddToolbar(const MCHAR *name)=0;
	virtual bool DeleteToolbar(int index)=0;
	virtual bool DeleteToolbar(const MCHAR *name)=0;
	virtual const MCHAR* GetToolbarName(int index)=0;
	virtual void ShowToolbar(const MCHAR *name)=0;
	virtual void HideToolbar(const MCHAR *name)=0;
	virtual bool IsToolbarVisible(const MCHAR *name)=0;
	virtual void ShowAllToolbars()=0;
	virtual void HideAllToolbars()=0;

	virtual void ShowMenuBar(bool visible)=0;
	virtual bool IsMenuBarVisible()=0;

	virtual void ShowScrollBars(bool visible)=0;
	virtual bool IsScrollBarsVisible()=0;

	virtual void ShowTrackWindow(bool visible)=0;
	virtual bool IsTrackWindowVisible()=0;

	virtual void ShowKeyWindow(bool visible)=0;
	virtual bool IsKeyWindowVisible()=0;

	virtual void ShowTimeRuler(bool visible)=0;
	virtual bool IsTimeRulerVisible()=0;

	virtual void ShowKeyPropertyIcon(bool visible)=0;
	virtual bool IsKeyPropertyIconVisible()=0;

	virtual void ShowIconsByClass(bool byClass)=0;
	virtual bool ShowIconsByClass()=0;

	virtual void SaveUILayout()=0;
	virtual void SaveUILayout(const MCHAR* name)=0;
	virtual void LoadUILayout(const MCHAR* name)=0;

	virtual int LayoutCount()=0;
	virtual MSTR GetLayoutName(int index)=0;
	virtual MSTR GetLayoutName()=0; //the current layout

	//!\brief Turn the unlocked icon on and off before a trackview item.
	//!\ This function is used to turn on or off the display of the unlocked icon before a trackview item.
	//!\ param visible If true the unlocked or locked icon is displayed before the track name, if false the icons are not shown.
	virtual void ShowUnlockedIcon(bool visible)=0;
	//!\brief Tells you wether or not the unlocked icon is on or off.
	//!\return If true then an unlocked or locked item will be displayed before a trackview item, if false then none will.
	virtual bool IsUnlockedIconVisible()=0;

	enum { tv_getHWnd, tv_getMenuBar, tv_setMenuBar, tv_getControllerQuadMenu, tv_setControllerQuadMenu,
			tv_getKeyQuadMenu, tv_setKeyQuadMenu,
			tv_getMenuBarVisible, tv_setMenuBarVisible, tv_getScrollBarsVisible, tv_setScrollBarsVisible,
			tv_getTrackWindowVisible, tv_setTrackWindowVisible, tv_getKeyWindowVisible, tv_setKeyWindowVisible,
			tv_getTimeRulerVisible, tv_setTimeRulerVisible, tv_showAllToolbars, tv_hideAllToolbars,
			tv_showToolbar, tv_hideToolbar, tv_getToolbarName, tv_deleteToolbar, tv_addToolbar, tv_toolbarCount,
			tv_saveUILayout, tv_saveCurrentUILayout, tv_loadUILayout, tv_layoutCount, tv_getLayoutName, tv_layoutName,
			tv_isToolbarVisible, tv_getKeyPropertyVisible, tv_setKeyPropertyVisible,
			tv_getIconsByClass, tv_setIconsByClass,tv_getUnlockedVisible,tv_setUnlockedVisible
		};
	#pragma warning (push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		RO_PROP_FN(tv_getHWnd, GetHWnd, TYPE_HWND);
		RO_PROP_FN(tv_layoutName, GetLayoutName, TYPE_TSTR_BV);
		PROP_FNS(tv_getMenuBar, GetMenuBar, tv_setMenuBar, SetMenuBar, TYPE_TSTR_BV);
		PROP_FNS(tv_getControllerQuadMenu, GetControllerQuadMenu, tv_setControllerQuadMenu, SetControllerQuadMenu, TYPE_TSTR_BV);
		PROP_FNS(tv_getKeyQuadMenu, GetKeyQuadMenu, tv_setKeyQuadMenu, SetKeyQuadMenu, TYPE_TSTR_BV);
		PROP_FNS(tv_getMenuBarVisible, IsMenuBarVisible, tv_setMenuBarVisible, ShowMenuBar, TYPE_bool);
		PROP_FNS(tv_getScrollBarsVisible, IsScrollBarsVisible, tv_setScrollBarsVisible, ShowScrollBars, TYPE_bool);
		PROP_FNS(tv_getTrackWindowVisible, IsTrackWindowVisible, tv_setTrackWindowVisible, ShowTrackWindow, TYPE_bool);
		PROP_FNS(tv_getKeyWindowVisible, IsKeyWindowVisible, tv_setKeyWindowVisible, ShowKeyWindow, TYPE_bool);
		PROP_FNS(tv_getTimeRulerVisible, IsTimeRulerVisible, tv_setTimeRulerVisible, ShowTimeRuler, TYPE_bool);
		PROP_FNS(tv_getKeyPropertyVisible, IsKeyPropertyIconVisible, tv_setKeyPropertyVisible, ShowKeyPropertyIcon, TYPE_bool);
		PROP_FNS(tv_getIconsByClass, ShowIconsByClass, tv_setIconsByClass, ShowIconsByClass, TYPE_bool);
		PROP_FNS(tv_getUnlockedVisible, IsUnlockedIconVisible, tv_setUnlockedVisible, ShowUnlockedIcon, TYPE_bool);
		VFN_0(tv_showAllToolbars, ShowAllToolbars);
		VFN_0(tv_hideAllToolbars, HideAllToolbars);
		VFN_1(tv_showToolbar, ShowToolbar, TYPE_STRING);
		VFN_1(tv_hideToolbar, HideToolbar, TYPE_STRING);
		FN_1(tv_getToolbarName, TYPE_STRING, GetToolbarName, TYPE_INDEX);
		FN_1(tv_deleteToolbar, TYPE_bool, fp_DeleteToolbar, TYPE_FPVALUE);
		FN_1(tv_addToolbar, TYPE_bool, AddToolbar, TYPE_STRING);
		FN_0(tv_toolbarCount, TYPE_INT, ToolbarCount);
		FN_1(tv_isToolbarVisible, TYPE_bool, IsToolbarVisible, TYPE_STRING);
		VFN_0(tv_saveCurrentUILayout, SaveUILayout);
		VFN_1(tv_saveUILayout, SaveUILayout, TYPE_STRING);
		VFN_1(tv_loadUILayout, LoadUILayout, TYPE_STRING);
		FN_0(tv_layoutCount, TYPE_INT, LayoutCount);
		FN_1(tv_getLayoutName, TYPE_TSTR_BV, GetLayoutName, TYPE_INDEX);
	END_FUNCTION_MAP
	#pragma warning (pop)

	FPInterfaceDesc* GetDesc() { return GetDescByID(TREEVIEW_UI_INTERFACE); }
		Interface_ID	GetID() { return TREEVIEW_UI_INTERFACE; }

private:
	virtual bool fp_DeleteToolbar(FPValue* val)=0;

};

#define TREEVIEW_OPS_INTERFACE Interface_ID(0x60fb7eef, 0x1f6d6dd3)
//These are the operations you can do on any open trackview
//Added by AF (09/12/00)
//*********************************************************
/*! \sa  Class FPMixinInterface,  Class Animatable, Class ReferenceTarget\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class contains the operations you can do on any open trackview.  */
class ITreeViewOps : public FPMixinInterface
{
protected:
		BOOL fpAssignControllerToSelected(ReferenceTarget* ctrl)
		{
			// upcast
			return AssignControllerToSelected((Animatable*)ctrl);
		}
		void fpZoomOn(ReferenceTarget* owner, int subnum)
		{
			// upcast
			ZoomOn((Animatable*)owner, subnum);
		}
		int fpGetIndex(ReferenceTarget* refAnim)
		{
			// upcast
			return GetIndex((Animatable*)refAnim);
		}
		void fpSelectTrack(ReferenceTarget* anim, BOOL clearSelection = TRUE)
		{
			// upcast
			SelectTrack((Animatable*) anim, clearSelection);
		}
	public:
		/*! \remarks Constructor. */
		virtual ~ITreeViewOps() {}

		/*! \remarks This method returns the number of tracks in the TreeView.
		*/
		virtual int GetNumTracks()=0;
		/*! \remarks This method returns the number of currently selected
		tracks in the TreeView. */
		virtual int NumSelTracks()=0;
 		/*! \remarks This method retrieves a track by its specified index.
 		\par Parameters:
 		<b>int i</b>\n\n
 		The index of the track you wish you retrieve.\n\n
 		<b>AnimatablePtr \&anim</b>\n\n
 		A reference to the Animatable object of the track that was
 		specified.\n\n
 		<b>AnimatablePtr \&client</b>\n\n
 		A reference to the client object of the track that was specified. This
 		is the 'parent' or 'owner' of the specified item.\n\n
 		<b>int \&subNum</b>\n\n
 		The index of the sub-anim of the track that was specified. */
 		virtual void GetSelTrack(int i,AnimatablePtr &anim,AnimatablePtr &client,int &subNum)=0;
		/*! \remarks This method returns a pointer to the reference target
		associated with a TrackView entry.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the TrackView entry for which to retrieve the reference
		target. */
		virtual ReferenceTarget* GetAnim(int index)=0;
		/*! \remarks This method returns a pointer to the client of the
		specified track. This is the 'parent' or 'owner' of the specified item.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the TrackView entry for which to retrieve the client. */
		virtual ReferenceTarget* GetClient(int index)=0;

		/*! \remarks This method returns TRUE if a controller can be assigned
		and FALSE if no controller can be assigned. */
		virtual BOOL CanAssignController()=0;
		/*! \remarks This method will invoke the assign controller dialog.
		\par Parameters:
		<b>BOOL clearMot</b>\n\n
		TRUE to clear the current settings, FALSE to leave the current
		settings. */
		virtual void DoAssignController(BOOL clearMot=TRUE)=0;
		/*! \remarks This method allows you to set the show controller type
		flag, on or off.
		\par Parameters:
		<b>BOOL show</b>\n\n
		Set the parameter to TRUE if the controller type should be shown. FALSE
		if the controller type should not be shown. */
		virtual void ShowControllerType(BOOL show)=0;	

		/*! \remarks This method returns the name of the TreeView name. */
		virtual const MCHAR *GetTVName()=0;
		// added for scripter access, JBW - 11/11/98
		/*! \remarks This method allows you to set the TreeView name.
		\par Parameters:
		<b>MCHAR* s</b>\n\n
		The name of the TreeView you wish to set. */
		virtual void SetTVName(const MCHAR *)=0;
		/*! \remarks This method will close the TreeView window. */
		virtual void CloseTreeView()=0;

		/*! \remarks This method allows you to set set a TreeView selection
		filter by adding to the selection mask.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The filter selection mask. See the List of TrackView Filter Mask Types
		for details.\n\n
		<b>int which</b>\n\n
		The filter you wish to set, 0 for filter 1, and 1 for filter 2.\n\n
		<b>BOOL redraw</b>\n\n
		Signal that a redraw should be issued. */
		virtual void SetFilter(DWORD mask,int which, BOOL redraw=TRUE)=0;
		/*! \remarks This method allows you to set set a TreeView selection
		filter by subtracting from the selection mask.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The filter selection mask. See the List of TrackView Filter Mask Types
		for details.\n\n
		<b>int which</b>\n\n
		The filter you wish to clear, 0 for filter 1, and 1 for filter 2.\n\n
		<b>BOOL redraw</b>\n\n
		Signal that a redraw should be issued. */
		virtual void ClearFilter(DWORD mask,int which, BOOL redraw=TRUE)=0; 
		/*! \remarks This method allows you to test if a filter has been set.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The filter selection mask. See the List of TrackView Filter Mask Types
		for details.\n\n
		<b>int which</b>\n\n
		The filter you wish to test, 0 for filter 1, and 1 for filter 2. */
		virtual DWORD TestFilter(DWORD mask,int which)=0; 
		
		// added for param wiring, JBW - 5.26.00
		/*! \remarks This method allows you to zoom/focus on a specified
		entry.
		\par Parameters:
		<b>Animatable* owner</b>\n\n
		A pointer to the Animatable you wish to zoom/focus on.\n\n
		<b>int subnum</b>\n\n
		The sub-anim you wish to zoom/focus on. */
		virtual void ZoomOn(Animatable* owner, int subnum)=0;
		/*! \remarks This method will zoom/focus on the selected entry. */
		virtual void ZoomSelected()=0;
		/*! \remarks This method will expand the tracks in the TreeView. */
		virtual void ExpandTracks()=0;

		//added for completeness by AF (09/12/00) 
		/*! \remarks This method returns the index of the specified
		Animatable.
		\par Parameters:
		<b>Animatable *anim</b>\n\n
		A pointer to the Animatable you wish to get the index of. */
		virtual int GetIndex(Animatable *anim)=0;
		/*! \remarks This method allows you to select a specific track by
		providing its index.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the track you wish to select.\n\n
		<b>BOOL clearSelection</b>\n\n
		Set this parameter to TRUE to signal the TreeView to clear the current
		selection, otherwise FALSE. */
		virtual void SelectTrackByIndex(int index, BOOL clearSelection=TRUE)=0;
		/*! \remarks This method allows you to select the track in trackview
		corresponding to the Animatable* passed as the argument.
		\par Parameters:
		<b>Animatable* anim</b>\n\n
		A pointer to the Animatable you wish to select.\n\n
		<b>BOOL clearSelection</b>\n\n
		Set this parameter to TRUE to signal the TreeView to clear the current
		selection, otherwise FALSE. */
		virtual void SelectTrack(Animatable* anim, BOOL clearSelection=TRUE)=0;
		/*! \remarks This method allows you to assign a controller to the
		selected entry. The function checks to make sure a controller can be
		assigned to the selected tracks, and that the controller passed in is
		of the appropriate SuperClass_ID. Returns FALSE if the tracks don't all
		have the same SuperClassID, or if the controller argument is of the
		wrong type.
		\par Parameters:
		<b>Animatable* ctrl</b>\n\n
		A pointer to the Animatable controller you wish to assign.
		\return  TRUE if the assignment was successful, otherwise FALSE. */
		virtual BOOL AssignControllerToSelected(Animatable* ctrl)=0;

		//added by AF (09/25/00) for MAXScript exposure
		/*! \remarks This method allows you to set the edit mode that the
		trackview displays.
		\par Parameters:
		<b>int mode</b>\n\n
		The edit mode, which is one of the following;\n\n
		<b>MODE_EDITKEYS</b>\n\n
		Edit keys mode.\n\n
		<b>MODE_EDITTIME</b>\n\n
		Edit time mode.\n\n
		<b>MODE_EDITRANGES</b>\n\n
		Edit ranges mode.\n\n
		<b>MODE_POSRANGES</b>\n\n
		Edit pos ranges model\n\n
		<b>MODE_EDITFCURVE</b>\n\n
		Edit function curves mode. */
		virtual void SetEditMode(int mode)=0;
		/*! \remarks This method returns the current edit mode.
		\return  The edit mode, which is one of the following;\n\n
		<b>MODE_EDITKEYS</b>\n\n
		Edit keys mode.\n\n
		<b>MODE_EDITTIME</b>\n\n
		Edit time mode.\n\n
		<b>MODE_EDITRANGES</b>\n\n
		Edit ranges mode.\n\n
		<b>MODE_POSRANGES</b>\n\n
		Edit pos ranges model\n\n
		<b>MODE_EDITFCURVE</b>\n\n
		Edit function curves mode. */
		virtual int GetEditMode()=0;

		//added by AF (09/25/00) for more MAXScript exposure
		//These differ from "active" because the trackview 
		//doesn't have to be selected for it to be the currently used trackview
		/*! \remarks This method returns TRUE if this trackview is the last
		trackview used by the user, otherwise FALSE. */
		virtual BOOL IsCurrent()=0;
		/*! \remarks This method allows you to set this trackview to be the
		current trackview. */
		virtual void SetCurrent()=0;

		enum {  tv_getName, tv_setName, tv_close, tv_numSelTracks, tv_getNumTracks, tv_getSelTrack, 
				tv_canAssignController, tv_doAssignController, tv_assignController, tv_showControllerTypes, 
				tv_expandTracks, tv_zoomSelected, tv_zoomOnTrack,
				tv_getAnim, tv_getClient, tv_getSelAnim, tv_getSelClient, tv_getSelAnimSubNum, 
				tv_getIndex, tv_selectTrackByIndex, tv_selectTrack,
				tv_setFilter, tv_clearFilter, 
				tv_setEditMode, tv_getEditMode, tv_setEditModeProp, tv_getEditModeProp,
				tv_setCurrent, tv_getCurrent, tv_getUIInterface, tv_getModifySubTree, tv_setModifySubTree, 
				tv_getModifyChildren, tv_setModifyChildren,
				tv_launchUtilityDialog, tv_launchUtility, tv_getUtilityCount, tv_getUtilityName, tv_closeUtility,
				tv_getInteractiveUpdate, tv_setInteractiveUpdate, tv_getSyncTime, tv_setSyncTime,
				tv_setTangentType, tv_setInTangentType, tv_setOutTangentType,
				tv_getFreezeSelKeys, tv_setFreezeSelKeys, 
				tv_getFreezeNonSelCurves, tv_setFreezeNonSelCurves, tv_getShowNonSelCurves, tv_setShowNonSelCurves,
				tv_getShowTangents, tv_setShowTangents, tv_getShowFrozenKeys, tv_setShowFrozenKeys,
				tv_getEffectSelectedObjectsOnly, tv_setEffectSelectedObjectsOnly, 
				tv_getAutoExpandChildren, tv_setAutoExpandChildren, tv_getAutoExpandTransforms, tv_setAutoExpandTransforms,
				tv_getAutoExpandObjects, tv_setAutoExpandObjects, tv_getAutoExpandModifiers, tv_setAutoExpandModifiers, 
				tv_getAutoExpandMaterials, tv_setAutoExpandMaterials, tv_getAutoExpandXYZ, tv_setAutoExpandXYZ,
				tv_getAutoSelectAnimated, tv_setAutoSelectAnimated, tv_getAutoSelectPosition, tv_setAutoSelectPosition,
				tv_getAutoSelectRotation, tv_setAutoSelectRotation, tv_getAutoSelectScale, tv_setAutoSelectScale,
				tv_getAutoSelectXYZ, tv_setAutoSelectXYZ, tv_getManualNavigation, tv_setManualNavigation,
				tv_getAutoZoomToRoot, tv_setAutoZoomToRoot, tv_getAutoZoomToSelected, tv_setAutoZoomToSelected,
				tv_getAutoZoomToEdited, tv_setAutoZoomToEdited, tv_getUseSoftSelect, tv_setUseSoftSelect, 
				tv_getSoftSelectRange, tv_setSoftSelectRange, tv_getSoftSelectFalloff, tv_setSoftSelectFalloff,
				tv_getRootTrack, tv_setRootTrack, tv_restoreRootTrack, tv_getScaleValuesOrigin, tv_setScaleValuesOrigin,
				tv_getIsolateCurve, tv_setIsolateCurve,
				tv_updateList,
				//symbolic enums
				tv_enumEffectTracks, tv_enumKeyTangentType, tv_editModeTypes, tv_enumTangentDisplay,
			};
		#pragma warning(push)
		#pragma warning(disable:4100 4238)
		BEGIN_FUNCTION_MAP
			FN_0(tv_getName, TYPE_STRING, GetTVName);
			VFN_1(tv_setName, SetTVName, TYPE_STRING);
			VFN_0(tv_close, CloseTreeView);
			FN_0(tv_getNumTracks, TYPE_INT, GetNumTracks);
			FN_0(tv_numSelTracks, TYPE_INT, NumSelTracks);
			
			FN_0(tv_canAssignController, TYPE_BOOL, CanAssignController);
			VFN_0(tv_doAssignController, DoAssignController);
			FN_1(tv_assignController, TYPE_BOOL, fpAssignControllerToSelected, TYPE_REFTARG);
			VFN_1(tv_showControllerTypes, ShowControllerType, TYPE_BOOL);
			
			VFN_0(tv_expandTracks, ExpandTracks);
			VFN_0(tv_zoomSelected, ZoomSelected);
			VFN_2(tv_zoomOnTrack, fpZoomOn, TYPE_REFTARG, TYPE_INT);

			FN_1(tv_getAnim, TYPE_REFTARG, GetAnim, TYPE_INDEX);
			FN_1(tv_getClient, TYPE_REFTARG, GetClient, TYPE_INDEX);

			FN_1(tv_getSelAnim, TYPE_REFTARG, fpGetSelectedAnimatable, TYPE_INDEX);
			FN_1(tv_getSelClient, TYPE_REFTARG, fpGetSelectedClient, TYPE_INDEX);
			FN_1(tv_getSelAnimSubNum, TYPE_INDEX, fpGetSelectedAnimSubNum, TYPE_INDEX);

			FN_1(tv_getIndex, TYPE_INDEX, fpGetIndex, TYPE_REFTARG);
			VFN_2(tv_selectTrackByIndex, SelectTrackByIndex, TYPE_INDEX, TYPE_BOOL);
			VFN_2(tv_selectTrack, fpSelectTrack, TYPE_REFTARG, TYPE_BOOL);

			FN_VA(tv_setFilter, TYPE_BOOL, fpSetFilter);
			FN_VA(tv_clearFilter, TYPE_BOOL, fpClearFilter);

			VFN_1(tv_setEditMode, SetEditMode, TYPE_ENUM);
			FN_0(tv_getEditMode, TYPE_ENUM, GetEditMode);
			PROP_FNS(tv_getEditModeProp, GetEditMode, tv_setEditModeProp, SetEditMode, TYPE_ENUM);
			FN_0(tv_getCurrent, TYPE_BOOL, IsCurrent);
			VFN_0(tv_setCurrent, SetCurrent);
			// UI interface property
			RO_PROP_FN(tv_getUIInterface, fpGetUIInterface, TYPE_INTERFACE);
			// dope sheet mode property
			PROP_FNS(tv_getModifySubTree, ModifySubTree, tv_setModifySubTree, ModifySubTree, TYPE_BOOL);
			PROP_FNS(tv_getModifyChildren, ModifyChildren, tv_setModifyChildren, ModifyChildren, TYPE_BOOL);
			//trackview utility methods
			VFN_0(tv_launchUtilityDialog, LaunchUtilityDialog);
			VFN_1(tv_launchUtility, LaunchUtility, TYPE_TSTR_BV);
			VFN_1(tv_closeUtility, CloseUtility, TYPE_TSTR_BV);
			FN_0(tv_getUtilityCount, TYPE_INT, GetUtilityCount);
			FN_1(tv_getUtilityName, TYPE_TSTR_BV, GetUtilityName, TYPE_INDEX);
			// key tangent type methods
			VFN_2(tv_setTangentType, SetTangentType, TYPE_ENUM, TYPE_ENUM);
			VFN_2(tv_setInTangentType, SetInTangentType, TYPE_ENUM, TYPE_ENUM);
			VFN_2(tv_setOutTangentType, SetOutTangentType, TYPE_ENUM, TYPE_ENUM);
			// button state properties
			PROP_FNS(tv_getInteractiveUpdate, InteractiveUpdate, tv_setInteractiveUpdate, InteractiveUpdate, TYPE_BOOL);
			PROP_FNS(tv_getSyncTime, SyncTime, tv_setSyncTime, SyncTime, TYPE_BOOL);
			PROP_FNS(tv_getFreezeSelKeys, FreezeSelKeys, tv_setFreezeSelKeys, FreezeSelKeys, TYPE_BOOL);
			PROP_FNS(tv_getFreezeNonSelCurves, FreezeNonSelCurves, tv_setFreezeNonSelCurves, FreezeNonSelCurves, TYPE_BOOL);
			PROP_FNS(tv_getShowNonSelCurves, ShowNonSelCurves, tv_setShowNonSelCurves, ShowNonSelCurves, TYPE_BOOL);
			PROP_FNS(tv_getShowTangents, ShowTangents, tv_setShowTangents, ShowTangents, TYPE_ENUM);
			PROP_FNS(tv_getShowFrozenKeys, ShowFrozenKeys, tv_setShowFrozenKeys, ShowFrozenKeys, TYPE_BOOL);
			PROP_FNS(tv_getIsolateCurve, IsolateCurve, tv_setIsolateCurve, IsolateCurve, TYPE_BOOL);
			// soft selection properties
			PROP_FNS(tv_getUseSoftSelect, UseSoftSelect, tv_setUseSoftSelect, UseSoftSelect, TYPE_BOOL);
			PROP_FNS(tv_getSoftSelectRange, SoftSelectRange, tv_setSoftSelectRange, SoftSelectRange, TYPE_TIMEVALUE);
			PROP_FNS(tv_getSoftSelectFalloff, SoftSelectFalloff, tv_setSoftSelectFalloff, SoftSelectFalloff, TYPE_FLOAT);
			// workflow properties
			PROP_FNS(tv_getEffectSelectedObjectsOnly, EffectSelectedObjectsOnly, tv_setEffectSelectedObjectsOnly, EffectSelectedObjectsOnly, TYPE_BOOL);
			PROP_FNS(tv_getManualNavigation, ManualNavigation, tv_setManualNavigation, ManualNavigation, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandChildren, AutoExpandChildren, tv_setAutoExpandChildren, AutoExpandChildren, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandTransforms, AutoExpandTransforms, tv_setAutoExpandTransforms, AutoExpandTransforms, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandObjects, AutoExpandObjects, tv_setAutoExpandObjects, AutoExpandObjects, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandModifiers, AutoExpandModifiers, tv_setAutoExpandModifiers, AutoExpandModifiers, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandMaterials, AutoExpandMaterials, tv_setAutoExpandMaterials, AutoExpandMaterials, TYPE_BOOL);
			PROP_FNS(tv_getAutoExpandXYZ, AutoExpandXYZ, tv_setAutoExpandXYZ, AutoExpandXYZ, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectAnimated, AutoSelectAnimated, tv_setAutoSelectAnimated, AutoSelectAnimated, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectPosition, AutoSelectPosition, tv_setAutoSelectPosition, AutoSelectPosition, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectRotation, AutoSelectRotation, tv_setAutoSelectRotation, AutoSelectRotation, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectScale, AutoSelectScale, tv_setAutoSelectScale, AutoSelectScale, TYPE_BOOL);
			PROP_FNS(tv_getAutoSelectXYZ, AutoSelectXYZ, tv_setAutoSelectXYZ, AutoSelectXYZ, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToRoot, AutoZoomToRoot, tv_setAutoZoomToRoot, AutoZoomToRoot, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToSelected, AutoZoomToSelected, tv_setAutoZoomToSelected, AutoZoomToSelected, TYPE_BOOL);
			PROP_FNS(tv_getAutoZoomToEdited, AutoZoomToEdited, tv_setAutoZoomToEdited, AutoZoomToEdited, TYPE_BOOL);
			//root node methods
			PROP_FNS(tv_getRootTrack, GetRootTrack, tv_setRootTrack, SetRootTrack, TYPE_REFTARG);
			VFN_0(tv_restoreRootTrack, RestoreDefaultRootTrack);
			// scale values mode -- scale origin
			PROP_FNS(tv_getScaleValuesOrigin, ScaleValuesOrigin, tv_setScaleValuesOrigin, ScaleValuesOrigin, TYPE_FLOAT);
			VFN_0(tv_updateList, UpdateList);
		END_FUNCTION_MAP
		#pragma warning(pop)
		
			/*! \remarks This method returns a pointer to the Function
			Publishing Interface Description. */
			FPInterfaceDesc* GetDesc() { return GetDescByID(TREEVIEW_OPS_INTERFACE); }
			Interface_ID	GetID() { return TREEVIEW_OPS_INTERFACE; }

//	private:
		//these methods are created to massage data into a format the function publishing system can interpret
		//these functions just call other public functions above
		//Added by AF (09/12/00)
		// LAM - 4/1/04 - made public. No access to SetTVDisplayFlag/GetTVDisplayFlag, which is needed for the 
		// AutoXXXX methods below.
		virtual Animatable* fpGetSelectedAnimatable(int index)=0;
		virtual Animatable* fpGetSelectedClient(int index)=0;
		virtual int fpGetSelectedAnimSubNum(int index)=0;
		virtual BOOL fpSetFilter(FPParams* val)=0;
		virtual BOOL fpClearFilter(FPParams* val)=0;
		
		// these are here so we don't break plugin compatibility
		// these can be called using FPInterface::Invoke()

		// Dope Sheet Mode effect children
		virtual BOOL ModifySubTree()=0;
		virtual void ModifySubTree(BOOL onOff)=0;
		virtual BOOL ModifyChildren()=0;
		virtual void ModifyChildren(BOOL onOff)=0;

		// Track View Utilities
		virtual void LaunchUtility(MSTR name)=0;
		virtual void LaunchUtilityDialog()=0;
		virtual void CloseUtility(MSTR name)=0;
		virtual int GetUtilityCount()=0;
		virtual MSTR GetUtilityName(int index)=0;

		// Update the viewport interactively, or on mouse up
		virtual BOOL InteractiveUpdate()=0;
		virtual void InteractiveUpdate(BOOL update)=0;

		// Sync Time with the mouse while clicking and dragging selections, keys, etc.
		virtual BOOL SyncTime()=0;
		virtual void SyncTime(BOOL sync)=0;

		// Button States
		virtual BOOL FreezeSelKeys()=0;
		virtual void FreezeSelKeys(BOOL onOff)=0;
		virtual BOOL FreezeNonSelCurves()=0;
		virtual void FreezeNonSelCurves(BOOL onOff)=0;
		virtual BOOL ShowNonSelCurves()=0;
		virtual void ShowNonSelCurves(BOOL onOff)=0;

		virtual int ShowTangents()=0;
		virtual void ShowTangents(int type)=0;
		virtual BOOL ShowFrozenKeys()=0;
		virtual void ShowFrozenKeys(BOOL onOff)=0;
		//! \brief Turn or off the Isolate Curve state.
		//! \param[in] onOff  When TRUE the trackview will isolate the current selection to those tracks which have keys selected and
		//! enter the Isolate Curve state.  If FALSE, and the Isolate Curve state is active, it will revert back to 
		//! the selection before Isolate Curve was activated. Note that if the track view selection changes in any way
		//! when the Isolate Curve state is active, the state automatically deactivates.
		virtual void IsolateCurve(BOOL onOff)=0;
		//! \brief Get whether the Isolate Curve state is active.
		//! \return Return TRUE if the Isolate Curve state is active, FALSE otherwise.
		virtual BOOL IsolateCurve()=0;
		// Soft selection
		virtual void UseSoftSelect(BOOL use)=0;
		virtual BOOL UseSoftSelect()=0;
		virtual void SoftSelectRange(TimeValue range)=0;
		virtual TimeValue SoftSelectRange()=0;
		virtual void SoftSelectFalloff(float falloff)=0;
		virtual float SoftSelectFalloff()=0;

		// Adjust the key tangent types
		virtual void SetTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;
		virtual void SetInTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;
		virtual void SetOutTangentType(int type, int effect = EFFECT_ALL_SEL_KEYS)=0;

		ITreeViewUI* fpGetUIInterface() { return (ITreeViewUI*)GetInterface(TREEVIEW_UI_INTERFACE); }

		// Workflow settings
		virtual BOOL EffectSelectedObjectsOnly()=0;
		virtual void EffectSelectedObjectsOnly(BOOL effect)=0;

		virtual BOOL ManualNavigation()=0;
		virtual void ManualNavigation(BOOL manual)=0;

		virtual BOOL AutoExpandChildren()=0;
		virtual void AutoExpandChildren(BOOL expand)=0;
		virtual BOOL AutoExpandTransforms()=0;
		virtual void AutoExpandTransforms(BOOL expand)=0;
		virtual BOOL AutoExpandObjects()=0;
		virtual void AutoExpandObjects(BOOL expand)=0;
		virtual BOOL AutoExpandModifiers()=0;
		virtual void AutoExpandModifiers(BOOL expand)=0;
		virtual BOOL AutoExpandMaterials()=0;
		virtual void AutoExpandMaterials(BOOL expand)=0;
		virtual BOOL AutoExpandXYZ()=0;
		virtual void AutoExpandXYZ(BOOL expand)=0;
			
		virtual BOOL AutoSelectAnimated()=0;
		virtual void AutoSelectAnimated(BOOL select)=0;
		virtual BOOL AutoSelectPosition()=0;
		virtual void AutoSelectPosition(BOOL select)=0;
		virtual BOOL AutoSelectRotation()=0;
		virtual void AutoSelectRotation(BOOL select)=0;
		virtual BOOL AutoSelectScale()=0;
		virtual void AutoSelectScale(BOOL select)=0;
		virtual BOOL AutoSelectXYZ()=0;
		virtual void AutoSelectXYZ(BOOL select)=0;

		virtual BOOL AutoZoomToRoot()=0;
		virtual void AutoZoomToRoot(BOOL zoom)=0;
		virtual BOOL AutoZoomToSelected()=0;
		virtual void AutoZoomToSelected(BOOL zoom)=0;
		virtual BOOL AutoZoomToEdited()=0;
		virtual void AutoZoomToEdited(BOOL zoom)=0;

		virtual ReferenceTarget* GetRootTrack()=0;
		virtual void SetRootTrack(ReferenceTarget* root)=0;
		virtual void RestoreDefaultRootTrack()=0;

		virtual float ScaleValuesOrigin()=0;
		virtual void ScaleValuesOrigin(float origin)=0;
		
		// force an update of the list
		virtual void UpdateList()=0;
};


/*! \sa  Class ITreeViewOps,  Class IObject, Class ITrackViewArray, Class TrackViewActionCallback,  Class TrackViewFilter,  Class ReferenceTarget,  Class Animatable\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
While this is the main TreeView class used for trackview operations, most of
the operations for TreeView's are inherited through the <b>ITreeViewOps</b>
class.\n\n
   */
class ITreeView : public IObject, public ITreeViewOps{
	public:
		
		/*! \remarks Constructor */
		virtual ~ITreeView() {}
		/*! \remarks This method will position the TreeView window at the
		specified position using the specified size.
		\par Parameters:
		<b>int x, int y</b>\n\n
		The x and y position of the TreeView window, in screen pixels.\n\n
		<b>int w, int h</b>\n\n
		The width and height of the TreeView window, in screen pixels. */
		virtual void SetPos(int x, int y, int w, int h)=0;
		/*! \remarks This method shows the TreeView window. */
		virtual void Show()=0;
		/*! \remarks This method hides the TreeView window. */
		virtual void Hide()=0;
		/*! \remarks This method allows you to enquire if the TreeView window
		is currently visible.
		\return  TRUE if the TreeView window is visible, otherwise FALSE. */
		virtual BOOL IsVisible()=0;
		/*! \remarks This method allows you to enquire whether the TreeView is
		being displayed in a viewport.
		\return  TRUE if the TreeView is displayed in a viewport, otherwise
		FALSE. */
		virtual BOOL InViewPort()=0; 

		/*! \remarks This method allows you to set the TreeView root which
		represents the initial tree branch.
		\par Parameters:
		<b>ReferenceTarget *root</b>\n\n
		A pointer to a reference target to use as the root in the TreeView.\n\n
		<b>ReferenceTarget *client</b>\n\n
		A pointer to the reference target which is the root's client.\n\n
		<b>int subNum</b>\n\n
		The sub-animatable number of the root you wish to set. */
		virtual void SetTreeRoot(ReferenceTarget *root,ReferenceTarget *client=NULL,int subNum=0)=0;
		/*! \remarks This method will instruct the TreeView to show the labels
		only.
		\par Parameters:
		<b>BOOL only</b>\n\n
		The only parameter specifies if the label only flag should be set
		(TRUE) or not (FALSE). */
		virtual void SetLabelOnly(BOOL only)=0;

		/*! \remarks This method controls the state of the TreeView's
		multi-select capability and allows you to enable or disable the
		selection of multiple selections.
		\par Parameters:
		<b>BOOL on</b>\n\n
		Set this parameter to TRUE to enable multi-select. FALSE to disable
		multi-select. */
		virtual void SetMultiSel(BOOL on)=0;
		/*! \remarks This method allows you to set set a TreeView selection
		filter which controls the amount of information displayed in the
		TreeView.
		\par Parameters:
		<b>TrackViewFilter *f</b>\n\n
		A pointer to a trackview filter which defines the displayable sub-set.
		*/
		virtual void SetSelFilter(TrackViewFilter *f=NULL)=0;
		/*! \remarks This method allows you to activate or inactivate the
		treeview.
		\par Parameters:
		<b>BOOL active</b>\n\n
		TRUE to activate, FALSE to deactivate. */
		virtual void SetActive(BOOL active)=0;
		/*! \remarks This method returns whether the TreeView is active (TRUE)
		or inactive (FALSE). */
		virtual BOOL IsActive()=0;
		/*! \remarks This method returns the handle to the TreeView window. */
		virtual HWND GetHWnd()=0;
		/*! \remarks This method\n\n
		This method returns the parent index of a specific TrackView entry. If
		no parent is found, -1 will be returned.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the TrackView entry for which to return the parent index.
		*/
		virtual int GetTrackViewParent(int index)=0; // returns -1 if no parent is found

		/*! \remarks This method will flush the TreeView and resets its size
		to 0. */
		virtual void Flush()=0;
		/*! \remarks This method will recalculate the sub-tree and signal the
		list has changed. */
		virtual void UnFlush()=0;
		/*! \remarks This method will set the material browser flag. */
		virtual void SetMatBrowse()=0;
		/*! \remarks This method returns the TrackView ID. */
		virtual DWORD GetTVID()=0;

		//from IObject
		virtual MCHAR* GetIObjectName(){return _M("ITrackView");}
		virtual int NumInterfaces() { return IObject::NumInterfaces() + 1; }
		virtual BaseInterface* GetInterfaceAt(int index) {
							if (index == 0)
								return (ITreeViewOps*)this; 
							return IObject::GetInterfaceAt(index-1);
						}

		virtual BaseInterface* GetInterface(Interface_ID id) 
				{ 
					if (id == TREEVIEW_OPS_INTERFACE) 
						return (BaseInterface*)this; 
					else { 
						return IObject::GetInterface(id);
		 			}
		 			return NULL;
				} 

	};


//Added by AF (09/07/00)
//A CORE interface to get the trackview windows
//Use GetCOREInterface(ITRACKVIEWS) to get a pointer to this interface
//**************************************************
#define ITRACKVIEWS Interface_ID(0x531c5f2c, 0x6fdf29cf)

/*! \sa  Class FPStaticInterface,  Class ITreeViewOps,  Class Animatable\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the track views array. An
<b>ITrackViewArray</b> pointer can be obtained by calling:
<b>GetCOREInterface(ITRACKVIEWS)</b>.  */
class ITrackViewArray : public FPStaticInterface
{
protected:
		BOOL fpTrackViewZoomOn(MSTR tvName, ReferenceTarget* anim, int subNum)
		{
			// upcast
			return TrackViewZoomOn(tvName, (Animatable*) anim, subNum);
		}
	public:	
		/*! \remarks This method returns the number of currently available
		TrackViews. */
		virtual int GetNumAvailableTrackViews();
		/*! \remarks This method returns a pointer to the ITreeViewOps of a
		specific TrackView.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the TrackView you wish to obtain. */
		virtual ITreeViewOps* GetTrackView(int index);
		/*! \remarks This method returns a pointer to the ITreeViewOps of a
		specific TrackView.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the TrackView you wish to obtain. */
		virtual ITreeViewOps* GetTrackView(MSTR name);
		/*! \remarks This method returns a table of ITreeViewOps pointers
		representing the list of TrackViews that are currently available. */
		virtual Tab<ITreeViewOps*> GetAvaliableTrackViews();
		/*! \remarks This method returns an interface to the last trackview
		used by the user. */
		virtual ITreeViewOps* GetLastActiveTrackView();

		/*! \remarks This method returns TRUE if the specified trackview is
		open, otherwise FALSE.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the trackview. */
		virtual BOOL IsTrackViewOpen(MSTR name);
		/*! \remarks This method returns TRUE if the specified trackview is
		open, otherwise FALSE.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview. */
		virtual BOOL IsTrackViewOpen(int index);
		/*! \remarks This method will open a specific TrackView window.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the TrackView window you wish to open.
		\return  TRUE if the window was opened, otherwise FALSE. */
		virtual BOOL OpenTrackViewWindow(
			MSTR name, 
			MSTR layoutName = _M(""), 
			Point2 pos = Point2(-1.0f,-1.0f), 
			int width = -1, 
			int height = -1, 
			int dock = TV_FLOAT);
		/*! \remarks This method will open a trackview by its specified index.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview.
		\return  TRUE if the trackview could be opened, otherwise FALSE. */
		virtual BOOL OpenTrackViewWindow(int index);
		/*! \remarks This method will open the last edited trackview if it was
		closed by the user.
		\return  TRUE if the last active trackview could be opened, otherwise
		FALSE. */
		virtual BOOL OpenLastActiveTrackViewWindow();

		/*! \remarks This method will close a specific TrackView window.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the TrackView window you wish to close.
		\return  TRUE if the window was closed, otherwise FALSE. */
		virtual BOOL CloseTrackView(MSTR name);
		/*! \remarks This method will close a specific TrackView window.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview you wish to close.
		\return  TRUE if the window was closed, otherwise FALSE. */
		virtual BOOL CloseTrackView(int index);
		/*! \remarks This method deletes the specified trackview from the list
		of saved trackviews.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the trackview you wish to delete. 
		\return  TRUE if the specified track view was deleted, FALSE otherwise. 
		Open or non-existent track views cannot be deleted.*/
		virtual BOOL DeleteTrackView(MSTR name);
		/*! \remarks This method deletes the specified trackview from the list
		of saved trackviews.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview you wish to delete.
		\return  TRUE if the specified track view was deleted, FALSE otherwise. 
		Open or non-existent track views cannot be deleted.*/
		virtual BOOL DeleteTrackView(int index);

		/*! \remarks This method will return the name of a TrackView,
		specified by it's index.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the TrackView for which you wish to obtain the name. */
		virtual const MCHAR* GetTrackViewName(int index);
		/*! \remarks This method will get the name of the last used trackview.
		*/
		virtual const MCHAR* GetLastUsedTrackViewName();

		/*! \remarks This method returns TRUE if the specified trackview is
		the current trackview, otherwise FALSE.
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview. */
		virtual BOOL IsTrackViewCurrent(int index);
		/*! \remarks This method returns TRUE if the specified trackview is
		the current trackview, otherwise FALSE.
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the trackview. */
		virtual BOOL IsTrackViewCurrent(MSTR name);
		/*! \remarks This method allows you to set the specified trackview to
		be the current trackview
		\par Parameters:
		<b>int index</b>\n\n
		The index of the trackview. */
		virtual BOOL SetTrackViewCurrent(int index);
		/*! \remarks This method allows you to set the specified trackview to
		be the current trackview
		\par Parameters:
		<b>MSTR name</b>\n\n
		The name of the trackview. */
		virtual BOOL SetTrackViewCurrent(MSTR name);

		/*! \remarks This method will zoom on the selected entries in a
		specific TrackView
		\par Parameters:
		<b>MSTR tvName</b>\n\n
		The name of the TrackView window you wish to have execute the zoom
		selected function.
		\return  TRUE if zooming was successful, FALSE if it failed. */
		virtual BOOL TrackViewZoomSelected(MSTR tvName);
		/*! \remarks This method will zoom on a specific TrackView entry.
		\par Parameters:
		<b>MSTR tvName</b>\n\n
		The name of the TrackView you wish to execute the zoom function
		for.\n\n
		<b>Animatable* anim</b>\n\n
		A pointer to the Animatable object you wish to zoom on.\n\n
		<b>int subNum</b>\n\n
		The sub-anim index.
		\return  TRUE if zooming was successful, FALSE if it failed. */
		virtual BOOL TrackViewZoomOn(MSTR tvName, Animatable* anim, int subNum);
	
		enum{ getTrackView, getAvaliableTrackViews, getNumAvailableTrackViews,
			  openTrackView, closeTrackView, getTrackViewName, trackViewZoomSelected, trackViewZoomOn,
			  setFilter, clearFilter, pickTrackDlg, isOpen, openLastTrackView, currentTrackViewProp,
			  lastUsedTrackViewNameProp, deleteTrackView, isTrackViewCurrent, setTrackViewCurrent,
			  doesTrackViewExist,

			  dockTypeEnum,
			};

		DECLARE_DESCRIPTOR(ITrackViewArray);
		#pragma warning(push)
		#pragma warning(disable:4100 4238)
		BEGIN_FUNCTION_MAP
		FN_1(getTrackView, TYPE_INTERFACE, fpGetTrackView, TYPE_FPVALUE); 
		FN_0(getAvaliableTrackViews, TYPE_INTERFACE_TAB_BV, GetAvaliableTrackViews);
		FN_0(getNumAvailableTrackViews, TYPE_INT, GetNumAvailableTrackViews);

		FN_6(openTrackView, TYPE_BOOL, fpOpenTrackViewWindow, TYPE_FPVALUE, TYPE_TSTR_BV, TYPE_POINT2_BV, TYPE_INT, TYPE_INT, TYPE_ENUM);
		FN_1(closeTrackView, TYPE_BOOL, fpCloseTrackView, TYPE_FPVALUE);
		VFN_1(deleteTrackView, fpDeleteTrackView, TYPE_FPVALUE);

		FN_1(getTrackViewName, TYPE_STRING, GetTrackViewName, TYPE_INDEX);
		FN_1(trackViewZoomSelected, TYPE_BOOL, TrackViewZoomSelected, TYPE_TSTR);
		FN_3(trackViewZoomOn, TYPE_BOOL, fpTrackViewZoomOn, TYPE_TSTR, TYPE_REFTARG, TYPE_INDEX);
		FN_VA(setFilter, TYPE_BOOL, fpSetTrackViewFilter);
		FN_VA(clearFilter, TYPE_BOOL, fpClearTrackViewFilter);
		FN_VA(pickTrackDlg, TYPE_FPVALUE_BV, fpDoPickTrackDlg);
		FN_1(isOpen, TYPE_BOOL, fpIsTrackViewOpen, TYPE_FPVALUE);
		FN_0(openLastTrackView, TYPE_BOOL, OpenLastActiveTrackViewWindow);		
		RO_PROP_FN(currentTrackViewProp, GetLastActiveTrackView, TYPE_INTERFACE);
		RO_PROP_FN(lastUsedTrackViewNameProp, GetLastUsedTrackViewName, TYPE_STRING);
		FN_1(isTrackViewCurrent, TYPE_BOOL, fpIsTrackViewCurrent, TYPE_FPVALUE);
		FN_1(setTrackViewCurrent, TYPE_BOOL, fpSetTrackViewCurrent, TYPE_FPVALUE);
		FN_1(doesTrackViewExist, TYPE_BOOL, fpDoesTrackViewExist, TYPE_FPVALUE);
		END_FUNCTION_MAP
		#pragma warning(pop)
	private:
		// these functions are wrapper functions to massage maxscript specific values into standard values
		// These methods just call one of the corresponding public methods
		BOOL fpSetTrackViewFilter(FPParams* val);
		BOOL fpClearTrackViewFilter(FPParams* val);
		FPValue fpDoPickTrackDlg(FPParams* val);
		BOOL fpIsTrackViewOpen(FPValue* val);
		BOOL fpCloseTrackView(FPValue* val);
		void fpDeleteTrackView(FPValue* val);
		BOOL fpIsTrackViewCurrent(FPValue* val);
		BOOL fpSetTrackViewCurrent(FPValue* val);
		ITreeViewOps* fpGetTrackView(FPValue* val);
		BOOL fpOpenTrackViewWindow(FPValue* val,MSTR layoutName, Point2 pos, int width, int height, int dock);
		BOOL fpDoesTrackViewExist(FPValue* val);
	};


#define TRACK_SELSET_MGR_INTERFACE Interface_ID(0x18f36a84, 0x1f572eb7)
class TrackSelectionSetMgr : public FPStaticInterface
	{
public:
	virtual BOOL	Create(MSTR name, Tab<ReferenceTarget*> tracks, Tab<MCHAR*> trackNames)=0;
	virtual BOOL	Delete(MSTR name)=0;
	virtual int		Count()=0;
	virtual MSTR	GetName(int index)=0;
	virtual void	SetName(int index, MSTR name)=0;
	virtual MSTR	GetCurrent(ITreeView* tv)=0;
	virtual void	GetTracks(MSTR name, Tab<ReferenceTarget*> &tracks, Tab<MCHAR*> &trackNames)=0;
	virtual BOOL	Add(MSTR name, Tab<ReferenceTarget*> tracks, Tab<MCHAR*> trackNames)=0;
	virtual BOOL	Remove(MSTR name, Tab<ReferenceTarget*> tracks)=0;

	enum { kCreate, kDelete, kCount, kGetName, kSetName, 
			kGetCurrent, kGetTracks, kAdd, kRemove,
		};
	};

TrackSelectionSetMgr* GetTrackSelectionSetMgr();

// Defines for the "open" argument to Interface::CreateTreeViewChild
// *****************************************************************
// "Special" windows are TreeViews whose data can not customized and is not saved.
#define OPENTV_SPECIAL	-2
// "Custom" windows are TreeViews whose data can be customized but is not saved.
// R5.1 and later only. A TreeView will not be created if using this flag in a previous release.
#define OPENTV_CUSTOM	-3  
// These arguments should not be used by third party developers.  
// They are used to create the saved trackviews that appear in the Graph Editors Menu.
#define OPENTV_LAST		-1
#define OPENTV_NEW		0
// *****************************************************************

// Sent by a tree view window to its parent when the user right clicks
// on dead area of the toolbar.
// Mouse points are relative to the client area of the tree view window
//
// LOWORD(wParam) = mouse x
// HIWORD(wParam) = mouse y
// lparam         = tree view window handle
#define WM_TV_TOOLBAR_RIGHTCLICK	WM_USER + 0x8ac1

// Sent by a tree view window when the user double
// clicks on a track label.
// wParam = 0
// lParam = HWND of track view window
#define WM_TV_LABEL_DOUBLE_CLICK	WM_USER + 0x8ac2

/*! \sa  Class ActionCallback\n\n
class TrackViewActionCallback : public ActionCallback\n\n

\par Description:
This class is available in release 4.0 and later only.\n\n
This is the callback object for handling TrackView actions.\n\n

\par Data Members:
<b>HWND mhWnd;</b>\n\n
The handle to the.window.  */
class TrackViewActionCallback: public ActionCallback {
public:
    /*! \remarks This method specifies which action to execute. This returns a
    BOOL that indicates if the action was actually executed. If the item is
    disabled, or if the table that owns it is not activated, then it won't
    execute, and returns FALSE. If it does execute then TRUE is returned.
    \par Parameters:
    <b>int id</b>\n\n
    The action ID. */
    BOOL ExecuteAction(int id);
    /*! \remarks This method sets the window handle.
    \par Parameters:
    <b>HWND hWnd</b>\n\n
    The handle to the window you wish to set.
    \par Default Implementation:
    <b>{ mhWnd = hWnd; }</b> */
    void SetHWnd(HWND hWnd) { mhWnd = hWnd; }

    HWND mhWnd;
};

//-----------------------------------------------------------------
//
// Button IDs for the track view
	
#define ID_TV_TOOLBAR			200    // the toolbar itself (not valid in R5 and later)
//#define ID_TV_DELETE			210
#define ID_TV_DELETETIME		215
#define ID_TV_MOVE				220
#define ID_TV_SCALE				230
#define ID_TV_SCALETIME			250
//#define ID_TV_FUNCTION_CURVE	240
#define ID_TV_SNAPKEYS			260
#define ID_TV_ALIGNKEYS			270
#define ID_TV_ADD				280
//#define ID_TV_EDITKEYS			290
//#define ID_TV_EDITTIME			300
//#define ID_TV_EDITRANGE			310
//#define ID_TV_POSITIONRANGE		320
#define ID_TV_FILTERS			330
#define ID_TV_INSERT			340
#define ID_TV_CUT				350
#define ID_TV_COPY				360
#define ID_TV_PASTE				370
#define ID_TV_SLIDE				380
#define ID_TV_SELECT			390
#define ID_TV_REVERSE			400
#define ID_TV_LEFTEND			410
#define ID_TV_RIGHTEND			420
#define ID_TV_SUBTREE			430
#define ID_TV_ASSIGNCONTROL		440
#define ID_TV_MAKEUNIQUE		450
#define ID_TV_CHOOSEORT			460
#define ID_TV_SHOWTANGENTS		470
#define ID_TV_SHOWALLTANGENTS	475
#define ID_TV_SCALEVALUES		480
#define ID_TV_REGION_TOOL		485 //!< Activate/Deactivate the Region Tool mode
#define ID_TV_ISOLATE_CURVE		486 //!< Activate/Deactivate the Isolate Curve state
#define ID_TV_BREAK_TANGENTS	487 //!< Break Tangents on selected keys
#define ID_TV_UNIFY_TANGENTS	488 //!< Unify Tangents on selected keys
#define ID_TV_FREEZESEL			490
#define ID_TV_SHOWKEYSONFROZEN	495
#define ID_TV_TEMPLATE			500	//Same as ID_TV_FREEZENONSELCURVES
#define ID_TV_FREEZENONSELCURVES 500
#define ID_TV_HIDENONSELCURVES	505
#define ID_TV_LOCKTAN			510
#define ID_TV_PROPERTIES		520
#define ID_TV_NEWEASE			530
#define ID_TV_DELEASE			540
#define ID_TV_TOGGLEEASE		550
#define ID_TV_CHOOSE_EASE_ORT	560
#define ID_TV_CHOOSE_MULT_ORT	570
#define ID_TV_ADDNOTE			580
#define ID_TV_DELETENOTE		590
#define ID_TV_RECOUPLERANGE		600
#define ID_TV_COPYTRACK			610
#define ID_TV_PASTETRACK		620
#define ID_TV_REDUCEKEYS		630
#define ID_TV_ADDVIS			640
#define ID_TV_DELVIS			650
#define ID_TV_TVNAME			660
#define ID_TV_TVUTIL			670
//watje
#define ID_TV_GETSELECTED		680
#define ID_TV_DELETECONTROL		690

#define ID_TV_SETUPPERLIMIT		700
#define ID_TV_SETLOWERLIMIT		701
#define ID_TV_TOGGLELIMIT		702
#define ID_TV_REMOVELIMIT		703
#define ID_TV_COPYLIMITONLY		704
#define ID_TV_PASTELIMITONLY	705

#define ID_TV_EXPANDNODES		710
#define ID_TV_EXPANDTRACKS		711
#define ID_TV_EXPANDALL			712
#define ID_TV_COLLAPSENODES		713
#define ID_TV_COLLAPSETRACKS	714
#define ID_TV_COLLAPSEALL		715
#define ID_TV_SELECTALL			720
#define ID_TV_SELECTINVERT		721
#define ID_TV_SELECTNONE		722
#define ID_TV_SELECTCHILDREN	723

#define ID_TV_EDITTRACKSET		730
#define ID_TV_TRACKSETLIST		731

#define ID_TV_AUTOEXPAND_KEYABLE	750
#define ID_TV_AUTOEXPAND_ANIMATED	751
#define ID_TV_AUTOEXPAND_LIMITS		752
#define ID_TV_AUTOEXPAND_LOCKED		753

#define ID_TV_IGNORE_ANIM_RANGE		760
#define ID_TV_RESPECT_ANIM_RANGE	761

#define ID_TV_FILTER_SELECTEDTRACKS	770

#define ID_TV_EDIT_TRACKSETS		800

// Status tool button IDs
#define ID_TV_STATUS			1000
#define ID_TV_ZOOMREGION		1020
#define ID_TV_PAN				1030
#define ID_TV_VFITTOWINDOW		1040
#define ID_TV_HFITTOWINDOW		1050
#define ID_TV_SHOWSTATS			1060
#define ID_TV_TIMETYPEIN		1070
#define ID_TV_VALUETYPEIN		1080
#define ID_TV_ZOOM				1090
#define ID_TV_MAXIMIZE			1100
#define ID_TV_SELWILDCARD		1110
#define ID_TV_ZOOMSEL			1120

// From accelerators
#define ID_TV_K_SNAP			2000
//#define ID_TV_K_LOCKKEYS		2010
#define ID_TV_K_MOVEKEYS		2020
#define ID_TV_K_MOVEVERT		2030
#define ID_TV_K_MOVEHORZ		2040
#define ID_TV_K_SELTIME			2050
#define ID_TV_K_SUBTREE			2060
#define ID_TV_K_LEFTEND			2070
#define ID_TV_K_RIGHTEND		2080
#define ID_TV_K_TEMPLATE		2090
#define ID_TV_K_SHOWTAN			2100
#define ID_TV_K_LOCKTAN			2110
#define ID_TV_K_APPLYEASE		2120
#define ID_TV_K_APPLYMULT		2130
#define ID_TV_K_ACCESSTNAME		2140
#define ID_TV_K_ACCESSSELNAME	2150
#define ID_TV_K_ACCESSTIME		2160
#define ID_TV_K_ACCESSVAL		2170
#define ID_TV_K_ZOOMHORZ		2180
#define ID_TV_K_ZOOMHORZKEYS	2190
#define ID_TV_K_ZOOM			2200
#define ID_TV_K_ZOOMTIME		2210
#define ID_TV_K_ZOOMVALUE		2220
//#define ID_TV_K_NUDGELEFT		2230
//#define ID_TV_K_NUDGERIGHT		2240
//#define ID_TV_K_MOVEUP			2250
//#define ID_TV_K_MOVEDOWN		2260
//#define ID_TV_K_TOGGLENODE		2270
//#define ID_TV_K_TOGGLEANIM		2280
#define ID_TV_K_SHOWSTAT		2290
#define ID_TV_K_MOVECHILDUP		2300
#define ID_TV_K_MOVECHILDDOWN	2310

// Button IDs for the tangent type buttons
#define ID_TV_TANGENT_FLAT		2320
#define ID_TV_TANGENT_CUSTOM	2330
#define ID_TV_TANGENT_FAST		2340
#define ID_TV_TANGENT_SLOW		2350
#define ID_TV_TANGENT_STEP		2360
#define ID_TV_TANGENT_LINEAR	2370
#define ID_TV_TANGENT_SMOOTH	2380

//Button ID for the DrawCurves Mode button
#define ID_TV_DRAWCURVES		2390

// ID for the keyable property toggle action item (R5.1 and later only)
#define ID_TV_TOGGLE_KEYABLE	2400

//! ID for the unlockable property toggle action item
#define ID_TV_TOGGLE_LOCKED	2401

/*! \defgroup trackViewFilterMaskTypes Trackview Filter Mask Types
These filters can be used with methods such as ITreeViewOps::SetFilter, 
ITreeViewOps::ClearFilter and ITreeViewOps::TestFilter. 
\sa Class ITreeViewOps */
//@{
#define FILTER_SELOBJECTS		(1<<0)	//!< Selected objects
#define FILTER_SELCHANNELS		(1<<1)	//!< Selected channels
#define FILTER_ANIMCHANNELS		(1<<2)	//!< Animated channels

#define FILTER_WORLDMODS		(1<<3)	//!< World Space Modifiers
#define FILTER_OBJECTMODS		(1<<4)	//!< Object Space Modifiers
#define FILTER_TRANSFORM		(1<<5)	//!< Transformations
#define FILTER_BASEPARAMS		(1<<6)	//!< base parameters

#define FILTER_POSX				(1<<7)	//!< X positions
#define FILTER_POSY				(1<<8)	//!< Y positions
#define FILTER_POSZ				(1<<9)	//!< Z positions
#define FILTER_POSW				(1<<10)	//!< W Positions
#define FILTER_ROTX				(1<<11)	//!< X rotations
#define FILTER_ROTY				(1<<12)	//!< Y rotations
#define FILTER_ROTZ				(1<<13)	//!< Z rotations
#define FILTER_SCALEX			(1<<14)	//!< X scaling
#define FILTER_SCALEY			(1<<15)	//!< Y scaling
#define FILTER_SCALEZ			(1<<16)	//!< Z scaling
#define FILTER_RED				(1<<17)	//!< Red
#define FILTER_GREEN			(1<<18)	//!< Green
#define FILTER_BLUE				(1<<19)	//!< Blue
#define FILTER_ALPHA			(1<<20)	//!< 

#define FILTER_CONTTYPES		(1<<21)	//!< 
#define FILTER_NOTETRACKS		(1<<22)	//!< 
#define FILTER_SOUND			(1<<23)	//!< 
#define FILTER_MATMAPS			(1<<24)	//!< 
#define FILTER_MATPARAMS		(1<<25)	//!< 
#define FILTER_VISTRACKS		(1<<26)	//!< 
#define FILTER_GLOBALTRACKS		(1<<27)	//!< 


// More filter bits. These are stored in the 2nd DWORD.
#define FILTER_GEOM				(1<<0)	//!<	All geometrical objects
#define FILTER_SHAPES			(1<<1)	//!<	All shape objects
#define FILTER_LIGHTS			(1<<2)	//!<	All lights
#define FILTER_CAMERAS			(1<<3)	//!<	All cameras
#define FILTER_HELPERS			(1<<4)	//!<	All helper objects
#define FILTER_WARPS			(1<<5)	//!<	All space warps
#define FILTER_VISIBLE_OBJS		(1<<6)	//!<	All visible objects
#define FILTER_POSITION			(1<<7)	//!<	All position controllers
#define FILTER_ROTATION			(1<<8)	//!<	All rotation controllers
#define FILTER_SCALE			(1<<9)	//!<	All scaling controllers
#define FILTER_CONTX			(1<<10)	//!<	All X axis controllers
#define FILTER_CONTY			(1<<11)	//!<	All Y axis controllers
#define FILTER_CONTZ			(1<<12)	//!<	All Z axis controllers
#define FILTER_CONTW			(1<<13)	//!<	All W axis controllers
#define FILTER_STATICVALS		(1<<14)	//!<	All static values
#define FILTER_HIERARCHY		(1<<15)	//!<	Based on hierarchy
#define FILTER_NODES			(1<<16)	//!<	All nodes
#define FILTER_BONES			(1<<17)	//!<	
#define FILTER_KEYABLE			(1<<18)	//!<	corresponds to the "Keyable" checkbox in the Filters dialog in TrackView
//! Whether or not we show only active layer controls or all controls.
#define FILTER_ACTIVELAYER		(1<<19)	//!<	
//! Whether or not we show only unlocked tracks or all tracks.
#define FILTER_LOCKED			(1<<20)

#define DEFAULT_TREEVIEW_FILTER0	(FILTER_WORLDMODS|FILTER_OBJECTMODS|FILTER_TRANSFORM| \
	FILTER_BASEPARAMS|FILTER_POSX|FILTER_POSY|FILTER_POSZ|FILTER_POSW|FILTER_ROTX|FILTER_ROTY|FILTER_ROTZ| \
	FILTER_SCALEX|FILTER_SCALEY|FILTER_SCALEZ|FILTER_RED|FILTER_GREEN|FILTER_BLUE|FILTER_ALPHA| \
	FILTER_NOTETRACKS|FILTER_MATMAPS|FILTER_MATPARAMS|FILTER_VISTRACKS|FILTER_SOUND|FILTER_GLOBALTRACKS)

#define DEFAULT_TREEVIEW_FILTER1	(FILTER_POSITION|FILTER_ROTATION|FILTER_SCALE| \
	FILTER_CONTX|FILTER_CONTY|FILTER_CONTZ|FILTER_CONTW|FILTER_VISIBLE_OBJS|FILTER_STATICVALS| \
	FILTER_HIERARCHY|FILTER_NODES)
//@}

// key tangent display setting
#define DISPLAY_TANGENTS_NONE		1
#define DISPLAY_TANGENTS_SELECTED	2
#define DISPLAY_TANGENTS_ALL		3


