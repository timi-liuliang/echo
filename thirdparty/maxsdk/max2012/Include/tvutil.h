/**********************************************************************
 *<
	FILE: tvutil.h

	DESCRIPTION: Track view utility plug-in class

	CREATED BY:	Rolf Berteig

	HISTORY: 12/18/96
    03/11/02 -- Adam Felt.  Expanded the interfaces.  
	
	WARNING: Plugin compatibility is not preserved between R4.x and R5 
	for TrackviewUtility plugins.  R4.x TrackViewUtility plugins need 
	to be recompiled before running on R5.x and later.  R5 and later 
	plugins will not run properly on previous versions.


 *>	Copyright (c) 1996-2002, All Rights Reserved.
 **********************************************************************/

#pragma once


#include "ref.h"
// forward declarations
class ITreeView;
class TrackViewUtility;
class Interface;

// The five track view major modes
#define TVMODE_EDITKEYS			0
#define TVMODE_EDITTIME			1
#define TVMODE_EDITRANGES		2
#define TVMODE_POSRANGES		3
#define TVMODE_EDITFCURVE		4

// This is an interface that is given to track view utilities
// that allows them to access the track view they were launched from.
/*! \sa  Class Animatable, Class TrackViewUtility, Class Interval, Class Interface, Class Control.\n\n
\par Description:
This class is an interface that is given to track view utilities that allows
them to access the track view they were launched from. All methods of this
class are implemented by the system.  */
class ITVUtility : public InterfaceServer {
	public:
		/*! \remarks Returns the total number of visible (open) tracks in
		Track %View. This determines valid values to use for '<b>i</b>' in the
		methods below. */
		virtual int GetNumTracks()=0;
		/*! \remarks Returns a pointer to the Animatalbe for the 'i-th' track.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the track. */
		virtual Animatable *GetAnim(int i)=0;
		/*! \remarks Returns a pointer to the client of the 'i-th' track. This
		is the 'parent' or 'owner' of the 'i-th' item.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the track. */
		virtual Animatable *GetClient(int i)=0;
		/*! \remarks Returns the sub-anim number of the 'i-th' track.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies the track. */
		virtual int GetSubNum(int i)=0;
		/*! \remarks Returns the name of the 'i-th' track.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the track whose name is returned. */
		virtual MSTR GetTrackName(int i)=0;
		/*! \remarks Returns TRUE if the 'i-th' track is selected; otherwise
		FALSE.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the track to test. */
		virtual BOOL IsSelected(int i)=0;
		/*! \remarks Sets the selected state of the 'i-th' track to the state
		passed.
		\par Parameters:
		<b>int i</b>\n\n
		The index of the track whose selected state is set.\n\n
		<b>BOOL sel</b>\n\n
		Specifies the selected state. TRUE for selected; FALSE for not
		selected. */
		virtual void SetSelect(int i,BOOL sel)=0;
		/*! \remarks Returns the windows handle of the main Track %View
		window. */
		virtual HWND GetTrackViewHWnd()=0;
		/*! \remarks Returns a value to indicate the current mode Track %View
		is operating in. This is one of five modes.
		\return  One of the following values:\n\n
		<b>TVMODE_EDITKEYS</b>\n\n
		<b>TVMODE_EDITTIME</b>\n\n
		<b>TVMODE_EDITRANGES</b>\n\n
		<b>TVMODE_POSRANGES</b>\n\n
		<b>TVMODE_EDITFCURVE</b> */
		virtual int GetMajorMode()=0;
		/*! \remarks Returns the current interval of selected time. */
		virtual Interval GetTimeSelection()=0;
		/*! \remarks Returns TRUE if 'Modify Subtree' mode is active;
		otherwise FALSE. */
		virtual BOOL SubTreeMode()=0;
		/*! \remarks Returns a pointer to the Track %View Root. */
		virtual Animatable *GetTVRoot()=0;

		// This must be called when a track view utility is closing
		// so that it can be unregistered from notifications
		/*! \remarks This must be called when a track view utility is closing
		so that it can be unregistered from notifications
		\par Parameters:
		<b>TrackViewUtility *util</b>\n\n
		Points to the utility that is closing. This is usually called by
		passing the <b>this</b> pointer as in: <b>TVUtilClosing(this);</b> */
		virtual void TVUtilClosing(TrackViewUtility *util)=0;

		// Access to the trackview interface.  R5 and later only
		virtual ITreeView* GetTVInterface()=0;
	};

// Results for TrackViewUtility::FilterAnim
#define FILTER_SUBTREE	0	// Filter out this anim and it's subtree
#define FILTER_NONE		1 	// Don't filter anything
#define FILTER_ANIM		-1	// Filter out this anim, but include it's subtree
#define FILTER_NODE		-2	// Filter out this node and subAnims, but include it's children


// This is the base class for track view utilities. Plug-ins will
// derive their classes from this class.
/*! \sa  Class ITVUtility, Class Interface, Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the base class for Track %View Utility plug-ins. These plug-ins are
launched via the 'Track %View Utility' icon just to the left of the track view
name field in the toolbar. Clicking on this button brings up a dialog of all
the track view utilities currently installed in the system. Most utilities will
probably be modeless floating dialogs, however modal utilities may be created
as well.\n\n
The developer will derive their classes from this class. Methods are provided
to bracket the beginning and ending of parameter editing, and responding to
various changes in Track %View (such as key selection, time selection, node
selection, etc.). There is also a method to delete this instance of the plug-in
class. Sample code is available in
<b>/MAXSDK/SAMPLES/UTILITIES/RANDKEYS.CPP</b>, <b>ORTKEYS.CPP</b> and
<b>SELKEYS.CPP</b>. \n\n
\par Plug-In Information:
Class Defined In: TVUTIL.H \n
Super Class ID:   TRACKVIEW_UTILITY_CLASS_ID \n
Standard File Name Extension: DLU \n
Extra Include File Needed: None \n
*/
class TrackViewUtility : public InterfaceServer {
	public:

		/*! \remarks This method is called to delete this instance of the
		plug-in class. This method should free the memory allocated in
		<b>ClassDesc::Create()</b>. See the Advanced Topics section on
		<a href="ms-its:3dsmaxsdk.chm::/alloc_memory_allocation.html">Memory
		Management</a> for more details. */
		virtual void DeleteThis()=0;		
		/*! \remarks This method is called to begin editing of the Track %View
		utility plug-in's parameters.
		\par Parameters:
		<b>Interface *ip</b>\n\n
		An interface for calling functions provided by 3ds Max.\n\n
		<b>ITVUtility *iu</b>\n\n
		An interface for allowing track view utilities to access the Track
		%View they are launched from.
		\par Default Implementation:
		<b>{}</b> */
		#pragma warning(push)
		#pragma warning(disable:4100)
		virtual void BeginEditParams(Interface *ip,ITVUtility *iu) {}
		/*! \remarks This method is called when the user has closed the Track
		%View utility or Track %View itself.
		\par Parameters:
		<b>Interface *ip</b>\n\n
		An interface for calling functions provided by 3ds Max.\n\n
		<b>ITVUtility *iu</b>\n\n
		An interface for allowing track view utilities to access the Track
		%View they are launched from.
		\par Default Implementation:
		<b>{}</b> */
		virtual void EndEditParams(Interface *ip,ITVUtility *iu) {}

		/*! \remarks This method is called when the selection of tracks has
		changed.
		\par Default Implementation:
		<b>{}</b> */
		virtual void TrackSelectionChanged() {}
		/*! \remarks This method is called when the selection of nodes has
		changed.
		\par Default Implementation:
		<b>{}</b> */
		virtual void NodeSelectionChanged() {}
		/*! \remarks This method is called when the selection of keys has
		changed.
		\par Default Implementation:
		<b>{}</b> */
		virtual void KeySelectionChanged() {}
		/*! \remarks This method is called when the amount of time selected
		changes in Edit Time mode. See <b>ITVUtility::GetTimeSelection()</b>.
		\par Default Implementation:
		<b>{}</b> */
		virtual void TimeSelectionChanged() {}
		/*! \remarks This method is called if the current mode of Track %View
		changes. These are the modes such as Edit Keys, Edit Time, Edit Ranges,
		Position Ranges, and Edit Function Curves. See
		<b>ITVUtility::GetMajorMode()</b>.
		\par Default Implementation:
		<b>{}</b> */
		virtual void MajorModeChanged() {}
		/*! \remarks This method is called when the Track %View list is
		rebuild. This is the list of items that are visible (currently open).
		\par Default Implementation:
		<b>{}</b> */
		virtual void TrackListChanged() {}
		
		// available in R5 and later only
		virtual int  FilterAnim(Animatable* anim, Animatable* client, int subNum) {return FILTER_NONE;}
		#pragma warning(pop)
		// used to identify registered utilities
		virtual void		GetClassName(MSTR& s)	{ s = MSTR(_M("")); }  
		virtual Class_ID	ClassID()				{ return Class_ID(0,0); }
		// end of R5 additions
	};



