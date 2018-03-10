/**********************************************************************
 *<
	FILE: notify.h

	DESCRIPTION: Include file for event notification support

	CREATED BY: Tom Hudson

	HISTORY: Created 8 April 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "coreexp.h"
#include "strbasic.h"

// forward declarations
class View;
class FrameRendParams;
class INode;
class ViewParams;

// Pre-defined Max system notification codes
/**
	\defgroup NotificationCodes System Notification Codes
	The following pre-defined system notification codes may be passed to the global functions 
	RegisterNotification(), UnRegisterNotification(), BroadcastNotification().
	
	\sa Structure NotifyInfo, Class Interface, Class Bitmap, Class RenderGlobalContext.
*/
 
//@{

//! \brief Sent if the user changes the unit setting
#define NOTIFY_UNITS_CHANGE				0x00000001
//! \brief Sent if the user changes the time format setting
#define NOTIFY_TIMEUNITS_CHANGE	 		0x00000002 
//! \brief Sent if the user changes the viewport layout
#define NOTIFY_VIEWPORT_CHANGE			0x00000003 
//! \brief Sent if the user changes the reference coordinate system.
#define NOTIFY_SPACEMODE_CHANGE	 		0x00000004 
//! \brief Sent before 3ds Max system is reset
#define NOTIFY_SYSTEM_PRE_RESET	 		0x00000005 
//! \brief Sent after 3ds Max system is reset
#define NOTIFY_SYSTEM_POST_RESET 		0x00000006 
//! \brief Sent before a new scene is requested. 
//! NotifyInfo::callParam is \ref new_scene_options "New Scene Options"
#define NOTIFY_SYSTEM_PRE_NEW			0x00000007 
//! \brief Sent after a new scene requested has been serviced. 
//! NotifyInfo::callParam is \ref new_scene_options "New Scene Options"
#define NOTIFY_SYSTEM_POST_NEW			0x00000008 
//! \brief Sent before a file is opened. NotifyInfo::callParam is a value of type FileIOType.
#define NOTIFY_FILE_PRE_OPEN			0x00000009 
//! \brief Sent after a file is opened successfully. NotifyInfo::callParam is a value of type FileIOType.
#define NOTIFY_FILE_POST_OPEN			0x0000000A 
//! \brief Sent before a file is merged.
/** When merge is called to load an XRef, you can determine this by testing 
void * <b>callParam</b>. The result is <b>not NULL </b> if an XRef is being loaded. */
#define NOTIFY_FILE_PRE_MERGE			0x0000000B
//! \brief Sent after a file is merged successfully.
/** When merge successfully loads an XRef, you can determine 
this by testing <b>void * callParam</b>. The result is not NULL if an XRef was loaded. */	
#define NOTIFY_FILE_POST_MERGE			0x0000000C 
//! \brief Sent before a file is saved ( callParam is MCHAR * to file name )
#define NOTIFY_FILE_PRE_SAVE			0x0000000D 
//! \brief Sent after a file is saved ( callParam is MCHAR * to file name )
#define NOTIFY_FILE_POST_SAVE			0x0000000E 
//! \brief Sent after a file open fails
#define NOTIFY_FILE_OPEN_FAILED	        0x0000000F 
//! \brief Sent before an old version file is saved
#define NOTIFY_FILE_PRE_SAVE_OLD 		0x00000010 
//! \brief Sent after an old version file is saved
#define NOTIFY_FILE_POST_SAVE_OLD		0x00000011 
//! \brief Sent after the selection set has changed
#define NOTIFY_SELECTIONSET_CHANGED		0x00000012 
//! \brief Sent after a bitmap is reloaded.
/** The \ref NotifyInfo structure pointer callParam is passed the MCHAR * to the 
bitmap file name. This is used for updating bitmaps that have changed. The 
callParam is used to pass the name of the bitmap file in case it is used in 
multiple changes. If the callParam is NULL, this notification applies to all 
bitmaps, as is the case when the input file gamma changes.  */
#define NOTIFY_BITMAP_CHANGED			0x00000013 
//! \brief Sent before rendering starts
#define NOTIFY_PRE_RENDER				0x00000014 
//! \brief Sent after rendering has finished.
#define NOTIFY_POST_RENDER				0x00000015 
//! \brief Sent before rendering each frame.
/** The NotifyInfo structure pointer <b>callParam</b> is passed as a 
pointer to the <b>RenderGlobalContext</b>. At the time of this call, the scene <b>must not</b> be modified. 
The renderer has already called <b>GetRenderMesh()</b> on all the object instances, and the materials and 
lights are already updated. If you don't modify anything that is rendered, then it is safe to use this 
callback. */
#define NOTIFY_PRE_RENDERFRAME			0x00000016 
//! \brief Sent after rendering each
/** The NotifyInfo structure pointer <b>callParam</b> is passed a pointer to the RenderGlobalContext.
\sa NOTIFY_PRE_RENDERFRAME */
#define NOTIFY_POST_RENDERFRAME			0x00000017 
//! \brief Sent before a file is imported.
/** The NotifyInfo structure pointer <b>callParam</b> is passed a pointer to the RenderGlobalContext. */
#define NOTIFY_PRE_IMPORT				0x00000018 
//! \brief Sent after a file is imported successfully
#define NOTIFY_POST_IMPORT				0x00000019 
//! \brief Sent if a file import fails or is cancelled
#define NOTIFY_IMPORT_FAILED			0x0000001A 
//! \brief Sent before a file is exported
#define NOTIFY_PRE_EXPORT				0x0000001B 
//! \brief Sent after a file is exported successfully
#define NOTIFY_POST_EXPORT				0x0000001C 
//! \brief Sent if a export fails or is cancelled
#define NOTIFY_EXPORT_FAILED			0x0000001D 

//! \defgroup NameChangeNotifications The name change notifications.
/*! These kinds of notifications are sent when the name of a target has been changed.
All these notifications use the struct NameChange as their call parameter.
\see NameChange
\see NOTIFY_NODE_RENAMED,NOTIFY_SCENESTATE_RENAME,NOTIFY_NAMED_SEL_SET_RENAMED
*/

//! \ingroup NameChangeNotifications
//! \brief Sent if a is node renamed.
/** Call Param is a pointer to <b>struct{ MCHAR* oldname; MCHAR* newname; }</b> \n
See <b>\\MAXSDK\\SAMPLES\\OBJECTS\\LIGHT.CPP</b> for an example of this notification in use. */
#define NOTIFY_NODE_RENAMED				0x0000001E 
//! \brief Sent before the progress bar is displayed.
/** The progress bar is displayed, for example, when the Render 
Preview command is run. If a plug-in uses a modeless window it should hide the window between this event and 
\ref NOTIFY_POST_PROGRESS. */
#define NOTIFY_PRE_PROGRESS				0x0000001F 
//! \brief Sent after the progress bar is finished
#define NOTIFY_POST_PROGRESS			0x00000020 
//! \brief Sent when the modify panel focuses on a new object because of opening the Modify panel or changing selection.
#define NOTIFY_MODPANEL_SEL_CHANGED		0x00000021 
//! \brief VIZ Sent when the user operates the height menu
#define NOTIFY_HEIGHT_CHANGED			0x00000022 
//! \brief VIZ Sent when the common renderer parameters have changed
#define NOTIFY_RENDPARAM_CHANGED		0x00000023

/** \name Material Library File Notifications */
//@{ 
//! \brief Sent before loading a material library
#define NOTIFY_MATLIB_PRE_OPEN			0x00000024 
//! \brief Sent after loading a material library.
/** The callParam is a pointer to <b>MtlBaseLib</b> if success, otherwise NULL. */
#define NOTIFY_MATLIB_POST_OPEN			0x00000025 
//! \brief Sent before saving a material library
#define NOTIFY_MATLIB_PRE_SAVE			0x00000026 
//! \brief Sent after saving a material library
#define NOTIFY_MATLIB_POST_SAVE			0x00000027 
//! \brief Sent before merging a material library
#define NOTIFY_MATLIB_PRE_MERGE			0x00000028 
//! \brief Sent after merging a material library
#define NOTIFY_MATLIB_POST_MERGE		0x00000029 
//@}

//! \brief Sent if a File Link Bind fails
#define NOTIFY_FILELINK_BIND_FAILED		0x0000002A 
//! \brief Sent if a File Link Detach fails
#define NOTIFY_FILELINK_DETACH_FAILED	0x0000002B 
//! \brief Sent if a File Link Reload fails
#define NOTIFY_FILELINK_RELOAD_FAILED	0x0000002C 
//! \brief Sent if a File Link Attach fails
#define NOTIFY_FILELINK_ATTACH_FAILED	0x0000002D 
//! \brief Sent before a File Link Bind
#define NOTIFY_FILELINK_PRE_BIND		0x00000030 
//! \brief Sent after a successful File Link Bind
#define NOTIFY_FILELINK_POST_BIND		0x00000031 
//! \brief Sent before a File Link Detach
#define NOTIFY_FILELINK_PRE_DETACH		0x00000032 
//! \brief Sent after a successful File Link Detach
#define NOTIFY_FILELINK_POST_DETACH		0x00000033 
//! \brief Sent before a File Link Reload (partial, full, or dynamic)
#define NOTIFY_FILELINK_PRE_RELOAD		0x00000034 
//! \brief Sent after a successful File Link Reload (partial, full, or dynamic)
#define NOTIFY_FILELINK_POST_RELOAD		0x00000035 
//! \brief Sent before a File Link Attach
#define NOTIFY_FILELINK_PRE_ATTACH		0x00000036 
//! \brief Sent after a successful File Link
/** \sa NOTIFY_FILELINK_POST_RELOAD_PRE_PRUNE */
#define NOTIFY_FILELINK_POST_ATTACH		0x00000037 

//! \brief VIZ request to load a URL into the Asset Browser
#define NOTIFY_AB_NAVIGATE_URL			0x00000038 
//! \brief Sent before the renderer starts evaluating objects; callParam is pointer to TimeValue
/** The NotifyInfo::callParam is passed as a pointer to TimeValue. 
Renderer plugins must broadcast this notification before they start evaluation 
scene objects. This notification allows plugins (such as modifiers or base objects) 
to perform a custom task before the renderer evaluates them. The custom task is 
usually one that would be invalid to be performed during a call to INode::EvalWorldState().*/
#define NOTIFY_RENDER_PREEVAL			0x00000039 
//! \brief Sent when a node is created (callParam is pointer to node)
#define NOTIFY_NODE_CREATED				0x0000003A 
//! \brief Sent when a node is linked (callParam is pointer to node)
#define NOTIFY_NODE_LINKED				0x0000003B 
//! \brief Sent when a node is unlinked (callParam is pointer to node)
#define NOTIFY_NODE_UNLINKED			0x0000003C 
//! \brief Sent when a node is hidden (callParam is pointer to node)
#define NOTIFY_NODE_HIDE				0x0000003D 
//! \brief Sent when a node is unhidden (callParam is pointer to node)
#define NOTIFY_NODE_UNHIDE				0x0000003E 
//! \brief Sent when a node is frozen (callParam is pointer to node)
#define NOTIFY_NODE_FREEZE				0x0000003F 
//! \brief Sent when a node is unfrozen (callParam is pointer to node)
#define NOTIFY_NODE_UNFREEZE			0x00000040 
//! \brief Node is about to get a new material (callParam is pointer to node)
#define NOTIFY_NODE_PRE_MTL				0x00000041 
//! \brief Node just got a new material (callParam is pointer to node)
#define NOTIFY_NODE_POST_MTL			0x00000042 
//! \brief Node just added to scene (callParam is pointer to node)
#define NOTIFY_SCENE_ADDED_NODE			0x00000043 
//! \brief Node just removed from scene (callParam is pointer to node)
#define NOTIFY_SCENE_PRE_DELETED_NODE	0x00000044 
//! \brief Node just removed from scene
#define NOTIFY_SCENE_POST_DELETED_NODE	0x00000045 
//! \brief selected nodes will be deleted. (callParam is pointer to Tab<INode*>)
#define NOTIFY_SEL_NODES_PRE_DELETE		0x00000046 
//! \brief Selected nodes just deleted.
#define NOTIFY_SEL_NODES_POST_DELETE	0x00000047 

//! \brief Sent when main window gets an WM_ENABLE (BOOL enabled)
#define NOTIFY_WM_ENABLE				0x00000048 

//! \brief 3ds Max is about to exit,  (system shutdown starting)
#define NOTIFY_SYSTEM_SHUTDOWN			0x00000049 
//! \brief 3ds Max just went live
#define NOTIFY_SYSTEM_STARTUP			0x00000050 

//! \brief A plug-in was just loaded. (callParam is pointer to DllDesc)
#define NOTIFY_PLUGIN_LOADED			0x00000051 

//! \brief Last broadcast before exit, after the scene is destroyed.
/** Most plugins will not live long enough to 
receive the notification. It is important to unregister this notification when 
your plugin dies. If not, 3ds Max will try to notify objects that no longer exist. */
#define NOTIFY_SYSTEM_SHUTDOWN2			0x00000052 

//! \brief Sent when Animate UI mode is activated
#define NOTIFY_ANIMATE_ON				0x00000053 
//! \brief Sent when Animate UI mode is de-activated
#define NOTIFY_ANIMATE_OFF				0x00000054 

//! \brief Sent by the system when one or more custom colors have changed.
/** Plug-ins should listen to this notification if they use any of the custom colors
registered with the system. See \ref viewportDrawingColors.
If a plug-in has created a toolbar with a MaxBmpFileIcons object on it, 
it should register for this notification, and call ICustToolbar::ResetIconImages() in response to it. 
See classes ICustToolbar and MAXBmpFileIcon. */
#define NOTIFY_COLOR_CHANGE				0x00000055 
//! \brief Sent just before the current edit object is about to change
/** This notification is sent whenever the object returned by Interface::GetCurEditObject() changes. */
#define NOTIFY_PRE_EDIT_OBJ_CHANGE  	0x00000056 
//! \brief Sent just after the current edit object changes
/** This notification is sent whenever the object returned by Interface::GetCurEditObject() changes. */
#define NOTIFY_POST_EDIT_OBJ_CHANGE  	0x00000057 

//! \brief Sent when radiosity processing is started.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
#define NOTIFY_RADIOSITYPROCESS_STARTED	0x00000058 
//! \brief Sent when radiosity processing is stopped, but not done.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
#define NOTIFY_RADIOSITYPROCESS_STOPPED	0x00000059 
//! \brief Sent when radiosity processing is reset.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
#define NOTIFY_RADIOSITYPROCESS_RESET	0x0000005A 
//! \brief Sent when radiosity processing is complete.
/** The radiosity_process notifications are designed to be broadcast by radiosity plugins (derived from
class RadiosityEffect). The broadcast must be implemented in the plugin for the notification to work. */
#define NOTIFY_RADIOSITYPROCESS_DONE	0x0000005B 

//! \brief Sent when lighting unit display system is changed
#define NOTIFY_LIGHTING_UNIT_DISPLAY_SYSTEM_CHANGE		0x0000005C 

// #define NOTIFY_INSTANCE_CREATED			0x00000053 // Sent when a new plugin instance is created via Interface::CreateInstance(), callParam is ptr to CreateInstanceCallParam (see below)

// 10/29/01 - 1:57pm --MQM-- 
// These are helpful for any plugins needing to know when we're starting a reflect/refract map,
// or when we're starting the actual frame.

//! \brief Sent when starting to render a reflect/refract map
#define NOTIFY_BEGIN_RENDERING_REFLECT_REFRACT_MAP	0x0000005D 
//! \brief Sent when starting to render the full frame.
#define NOTIFY_BEGIN_RENDERING_ACTUAL_FRAME			0x0000005E 
//! \brief Sent when starting to render a tone-mapping image.
#define NOTIFY_BEGIN_RENDERING_TONEMAPPING_IMAGE	0x0000005F 

//! \brief Sent when the radiosity plugin is changed (a new one is assigned)
#define NOTIFY_RADIOSITY_PLUGIN_CHANGED     0x00000060 

// [LAM - 3/13/02] Broadcast on scene undo/redo
//! \brief Sent on scene undo (callParam is MCHAR* with the undo entry name)
#define NOTIFY_SCENE_UNDO                   0x00000061 
//! \brief Sent on scene redo (callParam is MCHAR* with the redo entry name)
#define NOTIFY_SCENE_REDO                   0x00000062 
//! \brief Sent when manipulate mode ends
#define NOTIFY_MANIPULATE_MODE_OFF			0x00000063 
//! \brief Sent when manipulate mode starts
#define NOTIFY_MANIPULATE_MODE_ON			0x00000064 

// 020607  --prs.
/** \name XRef System Notifications
 These notices typically surround Merge notices */
//@{
//! \brief Sent before an XRef scene is merged
#define NOTIFY_SCENE_XREF_PRE_MERGE			0x00000065 
//! \brief Sent after an XRef scene is successfully merged (callParam is INode* pointer to scene XRef tree)
#define NOTIFY_SCENE_XREF_POST_MERGE		0x00000066 
//! \brief Sent before an XRef object is merged
#define NOTIFY_OBJECT_XREF_PRE_MERGE		0x00000067 
//! \brief Sent after an XRef object is successfully merged
#define NOTIFY_OBJECT_XREF_POST_MERGE		0x00000068 
//@}

// [J.Zhao - 6/10/02]
//! \brief Sent before a mirror operation begins.
/** callParam in the NotifyInfo is a pointer to Tab<INode*> consisting 
of the nodes currently in the selection list that the mirror tool is to be applied to. */
#define NOTIFY_PRE_MIRROR_NODES				0x00000069 
//! \brief Sent after a mirror operation ends.
/** callParam in the NotifyInfo is a pointer to Tab<INode*> consisting of 
the nodes currently in the selection list that the mirror tool is to be applied to. */
#define NOTIFY_POST_MIRROR_NODES			0x0000006A 

// [bayboro | 1may2002] Broadcast on node cloning
//! \brief Sent after a node is cloned but before theHold.Accept(..) (callParam is pointer to node)
#define NOTIFY_NODE_CLONED					0x0000006B  

// [J.Zhao - 10/4/02] The following two notifications may be broadcast
// when NotifyDependents from outside the recursion, that is, not during
// the traversal of reference targets.
// Right now, for examples, the very sources of NotifyDependents() of
//  - \ref REFMSG_MOUSE_CYCLE_STARTED
//  - \ref REFMSG_MOUSE_CYCLE_COMPLETED
// are bracketed by the notifications.

//! \brief Sent before NotifyDependents from outside the recursion
/** that is, not during the traversal of reference targets */
#define NOTIFY_PRE_NOTIFYDEPENDENTS			0x0000006C 
//! \brief Sent after NotifyDependents from outside the recursion
/** That is, not during the traversal of reference targets */
#define NOTIFY_POST_NOTIFYDEPENDENTS		0x0000006D 

//[hutchij 10/26/02]
//! \brief Sent by Mtl::RefAdded(). CallParam is Mtl pointer.
#define	NOTIFY_MTL_REFADDED					0x0000006E 
//! \brief Sent by Mtl::FefDeleted(). CallParam is Mtl pointer.
#define NOTIFY_MTL_REFDELETED				0x0000006F 


//watje TIMERANGE CALLBACK
//watje time range call back for CS
//! \brief Sent after the animate time range has been changed
#define NOTIFY_TIMERANGE_CHANGE				0x00000070 

//aszabo|dec.04.02|
//! \brief Sent before a modifier is added to an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a struct{ INode* node; Modifier* mod; ModContext* mc;}. */
#define NOTIFY_PRE_MODIFIER_ADDED			0x00000071 
//! \brief Sent after a modifier is added to an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a struct{ INode* node; Modifier* mod; ModContext* mc;}. */
#define NOTIFY_POST_MODIFIER_ADDED			0x00000072 
//! \brief Sent before a modifier is deleted from an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a struct{ INode* node; Modifier* mod; ModContext* mc;}. */
#define NOTIFY_PRE_MODIFIER_DELETED			0x00000073 
//! \brief Sent after a modifier is deleted from an object.
/** The NotifyInfo structure pointer callParam is passed a 
pointer to a struct{ INode* node; Modifier* mod; ModContext* mc;}. */
#define NOTIFY_POST_MODIFIER_DELETED		0x00000074 

// See Node Property Change Notifications, below (codes 0x74 through 0x85)

// CA - 1/23/03
//! \brief Sent after all of the new objects for a reload have been created
/** But, before any objects have been deleted. */
#define NOTIFY_FILELINK_POST_RELOAD_PRE_PRUNE	0x00000085

// aszabo|jan.24.03|
//! \brief Sent before each set of clones is created.
/** For example, if there are N nodes cloned C times, the 
notification is sent C times. The CallParam for NOTIFY_PRE_NODES_CLONED is a pointer to the array of nodes 
that will be cloned (the original nodes): INodeTab* origNodes */
#define NOTIFY_PRE_NODES_CLONED				0x00000086 
//! \brief Sent after each set of clones is created.
/** For example, if there are N nodes cloned C times, the 
notification is sent C times. The CallParam for NOTIFY_POST_NODES_CLONED is a pointer to this struct: struct{ 
INodeTab* origNodes; INodeTab* clonedNodes; CloneType cloneType;} */
#define NOTIFY_POST_NODES_CLONED			0x00000087 
#define NOTIFY_POST_MERGE_PROCESS			0x00000088

// xavier robitaille | 03.02.07
//! \brief Sent before a system path changes.
/** Originally intended to notify the tool palette if the Catalogs dir changes from the Configure Path dialog. */
#define NOTIFY_SYSTEM_PRE_DIR_CHANGE		0x00000089 
//! \brief Sent after a system path has changed.
/** Originally intended to notify the tool palette if the Catalogs dir changes from the Configure Path dialog. */
#define NOTIFY_SYSTEM_POST_DIR_CHANGE		0x0000008A 

//! \brief Schematic view notification.
/** callParam is ptr to index of schematic view (int*) */
#define NOTIFY_SV_SELECTIONSET_CHANGED		0x0000008C 
//! \brief Schematic view notification
/** callParam is IGraphNode* */
#define NOTIFY_SV_DOUBLECLICK_GRAPHNODE		0x0000008D 

//! \brief Sent before the renderer changes
#define NOTIFY_PRE_RENDERER_CHANGE			0x0000008E 
//! \brief Sent after the renderer changes
#define NOTIFY_POST_RENDERER_CHANGE			0x0000008F 

//! \brief Sent before a schematic view layout change is made.
/** callParam is ptr to index of schematic view (int*) */
#define NOTIFY_SV_PRE_LAYOUT_CHANGE			0x00000090 
//! \brief Sent after a schematic view layout change is made.
/** callParam is ptr to index of schematic view (int*) */
#define NOTIFY_SV_POST_LAYOUT_CHANGE		0x00000091 

//! \brief Sent AFTER object categories were marked to be hidden/unhidden.
/** Clients registered for this notification can retrieve the categories whose hidden 
state have changed by retrieving the category flags by calling DWORD Interface::GetHideByCategoryFlags() */
#define NOTIFY_BY_CATEGORY_DISPLAY_FILTER_CHANGED	0x00000092 

//! \brief Sent AFTER custom display filters have been activated/deactivated
/** Results in changes to some objects hidden state. Clients registered for this 
notification can retrieve the active custom display filters by checking their On/Off 
state using BOOL Interface::GetDisplayFilter(int index) */
#define NOTIFY_CUSTOM_DISPLAY_FILTER_CHANGED	0x00000093 

//! \brief Sent after layer is added to layer manager
/** callParam is ptr to ILayer */
#define NOTIFY_LAYER_CREATED				0x00000094 
//! \brief Sent before layer is removed from layer manager; callParam is ptr to ILayer
#define NOTIFY_LAYER_DELETED				0x00000095 
//! \brief callParam is ptr to struct{ INode* node; ILayer* oldLayer; ILayer* newLayer;}
/** newLayer and oldLayer can be NULL when switching between layers, during create,
and when loading files Layers may not be present in layer manager when sent during file load/merge  */
#define NOTIFY_NODE_LAYER_CHANGED			0x00000096 
 
//! \brief Sent when a tabbed dialog is created.
/** callparam is point to dialogID (Class_ID) */
#define NOTIFY_TABBED_DIALOG_CREATED		0x00000097 
//! \brief Sent when a tabbed dialog is deleted.
/** callparam is point to dialogID (Class_ID) */
#define NOTIFY_TABBED_DIALOG_DELETED		0x00000098 

//! \brief Sent by BaseNode::SetName.
/** callParam is pointer to struct{ MCHAR* oldname; MCHAR* newname; INode* node} */
#define NOTIFY_NODE_NAME_SET				0x00000099 

//! \brief Sent by the Material Editor when the "use texture in hardware shader" button is pressed.
/* This allows the standard material to force a rebuild of the hardware shader.  
Param is a pointer to the material being effected. */
#define NOTIFY_HW_TEXTURE_CHANGED			0x0000009A 

//! \brief Sent by MAXScript during its initialization
/** Occurs immediately before it scans the registered plugin classes and 
wraps them in MAXClass values any runtime defined classes created in this callback will be detected by MXS any 
core interfaces installed in this callback will be detected by MXS */
#define NOTIFY_MXS_STARTUP					0x0000009B 

//! \brief Sent by MAXScript when it has completed its initialization 
#define NOTIFY_MXS_POST_STARTUP				0x0000009C 

//! \brief Sent before an action item is executed NotifyInfo::callParam is ActionItem*
#define NOTIFY_ACTION_ITEM_PRE_EXEC			0x0000009D 
//! \brief Sent after an action item is executed
#define NOTIFY_ACTION_ITEM_POST_EXEC		0x0000009E 
// NotifyInfo::callParam is ActionItem*

// CCJ - 12.7.2004
/** \name Scene State Notifications
All the SceneState notifications have CallParam as a MCHAR* with the SceneState name except 
for \ref NOTIFY_SCENESTATE_RENAME */
//@{
//! \brief Sent before a Scene State is saved.
#define NOTIFY_SCENESTATE_PRE_SAVE			0x0000009F 
//! \brief Sent after a Scene State is saved.
#define NOTIFY_SCENESTATE_POST_SAVE			0x000000A0 
//! \brief Sent before a Scene State is restored.
#define NOTIFY_SCENESTATE_PRE_RESTORE		0x000000A1 
//! \brief Sent after a Scene State is restored.
#define NOTIFY_SCENESTATE_POST_RESTORE		0x000000A2 
//! \brief Sent after a Scene State is deleted.
#define NOTIFY_SCENESTATE_DELETE			0x000000A3 
//! \ingroup NameChangeNotifications
//! \brief Sent after a Scene State is renamed.
/** CallParam is pointer to struct{ const MCHAR* oldname; const MCHAR* newname; } */
#define NOTIFY_SCENESTATE_RENAME			0x000000A4 
//@}

// NH 20-Dec-04
/** \name Undo/Redo Notifications */
//@{
//! \brief Sent before an Undo starts.
#define NOTIFY_SCENE_PRE_UNDO				0x000000A5 
//! \brief Sent before a Redo starts.
/** Allows a developer to get an accurate picture of what the undo/redo is doing, 
and not having to rely on calling RestoreOrRedoing().  This solves a problem of evaluating a node during an 
undo when the hold system was active, but wasn't initiated from either script or the UI.  An example is right-
clicking to cancel object creation */
#define NOTIFY_SCENE_PRE_REDO				0x000000A6 
//! \brief Sent to make the previous undo notification more explicit, and match the PRE_UNDO.
#define NOTIFY_SCENE_POST_UNDO NOTIFY_SCENE_UNDO 
//! \brief Sent to make the previous undo notification more explicit, and match the PRE_UNDO.
#define NOTIFY_SCENE_POST_REDO NOTIFY_SCENE_REDO 
//@}

// Added LAM 19-Jan-05
//! \brief Sent when MAXScript has been shut down.
/** No access to any MXS value should be made after this notification, including calls to event handlers */
#define NOTIFY_MXS_SHUTDOWN					0x000000A7 

//Added NH 21-Jan-05
//! \brief This is sent by the D3D GFX (Dx9) when a device reset is going to happen.
/** This can allow Hardware shader to release their resources allocated in the DEFAULT_POOL */
#define NOTIFY_D3D_PRE_DEVICE_RESET			0x000000A8 
//! \brief This is sent by the D3D GFX (Dx9) when a device reset has happened.
/** This can allow Hardware shader to release their resources allocated in the DEFAULT_POOL */
#define NOTIFY_D3D_POST_DEVICE_RESET		0x000000A9 

// JH 2-18-05
//! \brief Used to suspend material change tracking in VIZ
/** In VIZ, the tool palette system listens for additions and removals to the scene material 
library and tracks reference additions and removals to existing materials. Upon resumption, 
the scene material lib will be traversed and changes reflected in the palette. */
#define NOTIFY_TOOLPALETTE_MTL_SUSPEND		0x000000AA

//! \brief Used to resume material change tracking in VIZ
/** In VIZ, the tool palette system listens for additions and removals to the scene material 
library and tracks reference additions and removals to existing materials. Upon resumption, 
the scene material lib will be traversed and changes reflected in the palette. */
#define NOTIFY_TOOLPALETTE_MTL_RESUME		0x000000AB

// Added LAM 04-Mar-05
//! \brief Provides a notification that a ClassDesc is being replaced by another one.
/** This occurs when the dll containing a deferred loaded plugin is loaded. 
CallParam is pointer to struct{ const ClassDesc *oldClassDesc; const ClassDesc *newClassDesc; }. 
See struct ClassDescReplaced in plugapi.h */
#define NOTIFY_CLASSDESC_REPLACED			0x000000AC

// Added russom 08-Mar-05
/** \name File I/O Notifications 
All contain a CallParam that points to a struct{ int iProcessType; const MCHAR* filename; }.
See \ref notify_file_process_type */
//@{
#define NOTIFY_FILE_PRE_OPEN_PROCESS		0x000000AD
#define NOTIFY_FILE_POST_OPEN_PROCESS		0x0000008B
#define NOTIFY_FILE_PRE_SAVE_PROCESS		0x000000AE
#define NOTIFY_FILE_POST_SAVE_PROCESS		0x000000AF
//@}

// Added LAM 15-Mar-05
//! \brief Sent after a ClassDesc was successfully loaded from a plugin dll. 
/*!	\remarks For each non NULL class descriptor returned by a plugin's ClassDesc* LibClassDesc(int i)
	method, the system broadcasts this notification. The call parameter will be a pointer to the ClassDesc. 
	\see <a href="ms-its:3dsmaxsdk.chm::/start_dll_functions.html"> Required DLL Functions </a> */
#define NOTIFY_CLASSDESC_LOADED				0x000000B0

// Added MAB 7-June-05
//! \brief This provides a notification that a toolbar configuration is being loaded, prior to load.
#define NOTIFY_TOOLBARS_PRE_LOAD				0x000000B1
//! \brief This provides a notification that a toolbar configuration is being loaded, after the load.
#define NOTIFY_TOOLBARS_POST_LOAD				0x000000B2

/** \name Asset Tracking System Notifications
These notification bracket ATS traversal of the entire scene (or ReferenceTarget hierarchy, 
in the case of a general retarget action) and repath any assets that should be repathed.  
No parameters are sent with these notifications. */
//@{
//! \brief Sent before the ATS traversal starts
#define NOTIFY_ATS_PRE_REPATH_PHASE				0x000000B3
//! \brief Sent after the ATS traversal completes
#define NOTIFY_ATS_POST_REPATH_PHASE			0x000000B4
//@}

/*!	\name Bitmap Proxy System Notifications
Any specific operation during which proxies should be disable can be surrounded 
by these notifications. This method of disabling proxies is different from actually 
disabling the bitmap proxies through the proxy manager; these notifications do not 
actually result in re-loading of any bitmaps. Rather, the Bitmap Texture will only 
load the high-res image when it is asked to, which makes the process efficient.
\note One should ideally NOT broadcast these notifications. Instead, use class 
BitmapProxyTemporaryDisableGuard; it is safer as it handles multiple nested disables. */
//@{
#define NOTIFY_PROXY_TEMPORARY_DISABLE_START	0x000000B5
#define NOTIFY_PROXY_TEMPORARY_DISABLE_END		0x000000B6
//@}

// Added russom 11-Apr-06
/** Allows a plugin to query the system for the status of the specified file. */
/** The Callparam is a pointer to a struct { const MCHAR* szFilename; int iStatus; }.  Plugins that
register for this notification can add to the iStatus member as needed, but should not clear any bits
already in iStatus.  Valid status masks for iStatus are defined below in this file.  Look for
FILE_STATUS_xxxxx. */
#define NOTIFY_FILE_CHECK_STATUS				0x000000B7

//! \name Named Selection Sets Notifications.
//! \brief The following notifications will be broadcast whenever any operation has been
//!		   done to a named selection set.
//! \note These notifications only apply to named selection sets of objects but not sub-object ones.
//@{
//! \brief Sent when a Named selection set is created either in the UI, or via maxscript.
/*! the callParam is a pointer to the string(i.e., MCHAR*) of the name of the created named set.
*/
#define NOTIFY_NAMED_SEL_SET_CREATED			0x000000B8
//! \brief Sent when a Named selection set is deleted either in the UI, or via maxscript.
/*! the callParam is a pointer to the string(i.e., MCHAR*) of the name of the deleted named set.
*/
#define NOTIFY_NAMED_SEL_SET_DELETED			0x000000B9
//! \ingroup NameChangeNotifications
//! \brief Sent when a Named selection set name is changed, either in the old UI, or via maxscript.
/*! the callParam is a pointer to the struct NameChange of the changed named set.
*/
#define NOTIFY_NAMED_SEL_SET_RENAMED            0x000000BC
//! \brief The following notification will be sent when the node set of a named selection set begins 
//!		   to be changed. For e.g., add/remove nodes to/from a named selection set,or completely
//!		   replace the node set of a named selection set.
/*! the callParam is a pointer to the string(i.e., MCHAR*) of the name of the modified named set.
	\note When modifying a specified named selection set, it will first be deleted then created with
		  the same name when deleted. So both NOTIFY_NAMED_SEL_SET_CREATED and NOTIFY_NAMED_SEL_SET_DELETED
		  will be broadcast during this process. In order to stress that this is a modification to an 
		  exisiting named selection set rather than two irrelevant operations as deleting one set then 
		  creating another set, we send two additional notifications as NOTIFY_NAMED_SEL_SET_PRE_MODIFY
		  and NOTIFY_NAMED_SEL_SET_POST_MODIFY in the very begining and ending of a modification
		  to a named set. For users who needs to be imformed of a modification of a named set, they should
		  register to these two notifications and ignore only NOTIFY_NAMED_SEL_SET_DELETED and 
		  NOTIFY_NAMED_SEL_SET_CREATED(even this applies to the current state of the code) that is received 
		  between the pair of NOTIFY_NAMED_SEL_SET_PRE_MODIFY and NOTIFY_NAMED_SEL_SET_POST_MODIFY.
*/
#define NOTIFY_NAMED_SEL_SET_PRE_MODIFY			0x000000CA
//! \brief The following notification will be sent when the node set of a named selection set has been changed.
/*! the callParam is a pointer to the struct NameChange of the changed named set.
*/
#define NOTIFY_NAMED_SEL_SET_POST_MODIFY		0x000000CB
//@}

//! \brief Sent when the sub-object level changes in the modify panel
#define NOTIFY_MODPANEL_SUBOBJECTLEVEL_CHANGED  0x000000BA

// NH 13-May-2006

//! \brief Sent when a bitmap fails to load in the DirectX Shader Material
/** This is usually when DirectX does not support the file format. The developer 
can register for this notification so that they can convert the the format into a 
DirectX texture resource. The developer is responsible for maintaining this resource, 
and can register for NOTIFY_D3D_POST_DEVICE_RESET to release and rebuild the resource on Lost device 
situations The callParam is a struct{ MCHAR * fileName, MCHAR* paramName, Mtl* dxMaterial, bool forceUpdate} 
From the material the developer can access the IEffectParser and use LoadTexture using the paramName.  The 
forceUpdate will be set if this was an auto update of the bitmap from disk, or the user hit the reload button 
from the UI.  The developer will need to release and rebuild the bitmap under these situations. */
#define NOTIFY_FAILED_DIRECTX_MATERIAL_TEXTURE_LOAD		0x000000BB

//! \brief Sent just after NOTIFY_RENDER_PREEVAL.
/** The call param is a NotifyPreEval_FrameInfo*, defined at the bottom of this header file. Sent before the 
render start evaluating objects.
\note This message is sent ONLY when using the mental ray renderer. It is NOT sent when using the scanline 
renderer. */ 
#define NOTIFY_RENDER_PREEVAL_FRAMEINFO					0x000000BC

//! \brief Sent just after deleting all refs in existing scene.
/** This notification is sent after wiping the existing scene. Examples of when this occurs is 
immediately before loading in the new scene from a file, and when doing a file new */
#define NOTIFY_POST_SCENE_RESET							0x000000BD

//! \brief Sent just after animation layers are enabled on some nodes in the scene.
/** This notification is sent after layers are enabled, and new layer controller constructed.
The callParam is a pointer to the nodes which are having animation layers enabled. */
#define NOTIFY_ANIM_LAYERS_ENABLED							0x000000BE

//! \brief Sent just after animation layers are disabled on some nodes in the scene.
/** This notification is sent after layers are disabled on some nodes, and the layer ends up deleted.
The callParam is a pointer to the nodes which are having animation layers disabled. */
#define NOTIFY_ANIM_LAYERS_DISABLED							0x000000BF

//! \brief Sent just before an action item is overridden and IActionItemOverride::StartOveride is called.
/** The callParam is a pointer to the ActionItem. */
#define NOTIFY_ACTION_ITEM_PRE_START_OVERRIDE					0x000000C0

//! \brief Sent just after an action item is overridden and after IActionItemOverride::StartOveride is called.
/*! The callParam is a pointer to the ActionItem. */
#define NOTIFY_ACTION_ITEM_POST_START_OVERRIDE					0x000000C1

//! \brief Sent just before an action item finishes it's override and IActionItemOverride::EndOverride is called.
/** The callParam is a pointer to the ActionItem. */
#define NOTIFY_ACTION_ITEM_PRE_END_OVERRIDE					0x000000C2

//! \brief Sent just after an action item finishes it's override and after IActionItemOverride::EndOverride is called. */
/** The callParam is a pointer to the ActionItem. */
#define NOTIFY_ACTION_ITEM_POST_END_OVERRIDE					0x000000C3

//aszabo|dec.11.02|

/** \name Node Property Change Notifications
The callParam is a pointer to the list of nodes (INodeTab*) that is about to change or has changed.
\todo Add relevant details for individual defines */
//@{
#define NOTIFY_PRE_NODE_GENERAL_PROP_CHANGED	0x00000075 // Encompasses Render, Display, and Basic props below
#define NOTIFY_POST_NODE_GENERAL_PROP_CHANGED	0x00000076 // Encompasses Render, Display, and Basic props below
#define NOTIFY_PRE_NODE_GI_PROP_CHANGED			0x00000077
#define NOTIFY_POST_NODE_GI_PROP_CHANGED		0x00000078
#define NOTIFY_PRE_NODE_MENTALRAY_PROP_CHANGED	0x00000079
#define NOTIFY_POST_NODE_MENTALRAY_PROP_CHANGED	0x00000080
#define NOTIFY_PRE_NODE_BONE_PROP_CHANGED		0x00000081
#define NOTIFY_POST_NODE_BONE_PROP_CHANGED		0x00000082
#define NOTIFY_PRE_NODE_USER_PROP_CHANGED		0x00000083
#define NOTIFY_POST_NODE_USER_PROP_CHANGED		0x00000084

#define NOTIFY_PRE_NODE_RENDER_PROP_CHANGED		0x000000C4
#define NOTIFY_POST_NODE_RENDER_PROP_CHANGED	0x000000C5
#define NOTIFY_PRE_NODE_DISPLAY_PROP_CHANGED	0x000000C6
#define NOTIFY_POST_NODE_DISPLAY_PROP_CHANGED	0x000000C7
#define NOTIFY_PRE_NODE_BASIC_PROP_CHANGED		0x000000C8
#define NOTIFY_POST_NODE_BASIC_PROP_CHANGED		0x000000C9
//@}

//! \brief Sent when selection lock is triggered. */
/** The callParam is NULL. */
#define NOTIFY_SELECTION_LOCK					0x000000D0
//! \brief Sent when selection unlock is triggered. */
/** The callParam is NULL. */
#define NOTIFY_SELECTION_UNLOCK					0x000000D1

//! \brief Sent when an image viewer (including the rendered frame window) is opened, before it becomes visible. */
/** The callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed.
	The interface object may be used to add rollouts to the window in MaxScript. */
#define NOTIFY_PRE_IMAGE_VIEWER_DISPLAY			0x000000D2
//! \brief Sent when an image viewer (including the rendered frame window) is opened, after it becomes visible. */
/** The callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed. */
#define NOTIFY_POST_IMAGE_VIEWER_DISPLAY		0x000000D3
//! \brief Sent by an image viewer (or the rendered frame window) to request that its custom rollouts update their UI. */
/** The callParam is an FPValue, whose fpi member (of type FPInterface*) points to an interface to the VFB.
	The interface is intended for use only in MaxScript.  Note the interface object will be destroyed when the VFB is closed. */
#define NOTIFY_IMAGE_VIEWER_UPDATE				0x000000D4

/** \name Custom Attributes Notifications
These notifications are sent after a custom attribute is added or removed
from a custom attribute container.  
The callParam is a pointer to a NotifyCustomAttribute structure (see custattrib.h) */
//@{
//! \brief Sent after a custom attribute is added to an Animatable
#define NOTIFY_CUSTOM_ATTRIBUTES_ADDED			0x000000D5
//! \brief Sent after a custom attribute is removed from an Animatable
#define NOTIFY_CUSTOM_ATTRIBUTES_REMOVED		0x000000D6
//@}

//! \brief Sent after OS theme has been changed
#define NOTIFY_OS_THEME_CHANGED					0x000000D7

//! \brief Sent when the current active viewport is changed.  
/*! Typically this notification is sent when the user has switched viewports. 
 * To get notifications of any other type of viewport change, please use NOTIFY_VIEWPORT_CHANGE */
#define NOTIFY_ACTIVE_VIEWPORT_CHANGED			0x000000D8

//! \brief NOTIFY_PRE_MAXMAINWINDOW_SHOW is being sent when main window of 3ds Max is about to show.
#define NOTIFY_PRE_MAXMAINWINDOW_SHOW			0x000000DB

//! \brief NOTIFY_POST_MAXMAINWINDOW_SHOW is being sent immediately after the main window of 3ds Max is shown.
#define NOTIFY_POST_MAXMAINWINDOW_SHOW			0x000000DC

//! \brief Provides a notification that a new ClassDesc is being registered.
/** This occurs when dlls containing plugins are loaded, and when a ClassDesc is dynamically created and registered 
with 3ds Max (for example, when a scripted plugin is defined). When loading a dll containing plugins, for each plugin
you will get a NOTIFY_CLASSDESC_LOADED notification followed by a NOTIFY_CLASSDESC_ADDED notification. When a 
ClassDesc is dynamically created and registered with 3ds Max, you will get just a NOTIFY_CLASSDESC_ADDED notification.
CallParam is pointer to ClassDesc */
#define NOTIFY_CLASSDESC_ADDED			0x000000DD

//! \brief NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN is sent immediately before object instances are updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This notification is sent immediately before the instances are converted to 
the new definition.
The NotifyInfo structure pointer callParam is passed a pointer to a ObjectDefinitionChangeNotificationParam struct instance. 
Currently the only valid type value is kScriptedPlugin, which signifies a scripted plugin/custom attribute class description, 
and the definition is a pointer to ClassDesc2
A plugin may use this notification to, for example, control the rebuilding of rollout displays for instances of the plugins using
the definition. 
\sa REFMSG_OBJECT_DEFINITION_CHANGE_BEGIN, ObjectDefinitionChangeNotificationParam
*/
#define NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN			0x000000DE

//! \brief NOTIFY_OBJECT_DEFINITION_CHANGE_END is sent immediately after object instances are updated to a new object definition
/*! MAXScript allows scripted plugin and scripted custom attribute definitions to be updated by re-evaluating 
the definition script. The new definition can add or remove local variables, parameter blocks, parameter block
items, rollouts, and rollout controls. After the new definition is evaluated, existing plugin instances are 
converted to this new definition. This notification is sent immediately after the instances are converted to 
the new definition.
The NotifyInfo structure pointer callParam is passed a pointer to a ObjectDefinitionChangeNotificationParam struct instance. 
Currently the only valid type value is kScriptedPlugin, which signifies a scripted plugin/custom attribute class description, 
and the definition is a pointer to ClassDesc2
A plugin may use this notification to, for example, control the rebuilding of rollout displays for instances of the plugins using
the definition.
\sa REFMSG_OBJECT_DEFINITION_CHANGE_END, ObjectDefinitionChangeNotificationParam
*/
#define NOTIFY_OBJECT_DEFINITION_CHANGE_END			0x000000DF


//! \brief Sent when a MtlBase is about to show its UI with Associated parameter rollouts when being edited in a material editor. 
/*! CallParam is pointer to the MtlBase that is about to show its UI. */
#define NOTIFY_MTLBASE_PARAMDLG_PRE_OPEN  			0x000000E0

//! \brief Sent when a MtlBase has finished its editing in the material editor and the UI with Associated parameter rollouts have been closed.
/*! CallParam is pointer to the MtlBase that has closed its UI. */
#define NOTIFY_MTLBASE_PARAMDLG_POST_CLOSE  		0x000000E1

//! \brief Sent when the application theme is changed via IColorManager::SetAppFrameColorTheme().
/*!  CallParam is NULL. */
#define NOTIFY_APP_FRAME_THEME_CHANGED				0x000000E2


// Note #1: If you add more built-in notification codes, consider
//    increasing NUM_BUILTIN_NOTIFIERS in core\notify.cpp - currently 0x00E7

// Note #2: If you add more built-in notification codes, consider
//    adding them to MAXScript. See maxscrpt\maxcallbacks.cpp

// Start of messages for internal use only.
#define NOTIFY_INTERNAL_USE_START				0x70000000

/** \defgroup new_scene_options New Scene Options
Flag values in callParam for \ref NOTIFY_SYSTEM_PRE_NEW and \ref NOTIFY_SYSTEM_POST_NEW
indicating the type of new scene operation that was carried out.
*/
//@{
//! \brief All objects are deleted, including their animation data
#define PRE_NEW_NEW_ALL 0x1
//! \brief Objects are not deleted, hierarchical links between them and animation data are deleted
#define PRE_NEW_KEEP_OBJECTS 0x2
//! \brief Objects and hierarchical links between them are not deleted, animation data is deleted
#define PRE_NEW_KEEP_OBJECTS_AND_HIERARCHY 0x3
//@}

// Added russom 08-Mar-05
/** \defgroup notify_file_process_type File Process Type */
//@{
//* \brief iProcessType value for scene file save/load
/**	\see NOTIFY_FILE_PRE_OPEN_PROCESS */
#define FILE_PROCESS_SCENE					0x1
//* \brief iProcessType value for hold/fetch
/**	\see NOTIFY_FILE_PRE_OPEN_PROCESS */
#define FILE_PROCESS_HOLD_FETCH				0x2
//* \brief iProcessType value for auto backup
/**	\see NOTIFY_FILE_PRE_OPEN_PROCESS */
#define FILE_PROCESS_AUTOBAK				0x3
//@}

//  Added russom 11-Apr-06
/** \name File Attributes 
\todo Provide more file attribute defines */
//@{
//* \brief iStatus value for NOTIFY_FILE_CHECK_STATUS
/*	\see NOTIFY_FILE_CHECK_STATUS  */
#define FILE_STATUS_READONLY				0x1
//@}


//@}	END OF NotificationCodes System Notification Codes

// Notification information structure -- Passed to NOTIFYPROC to inform it what
// it's being notified about...
/*! 3ds Max supports a system where a plug-in can ask to receive a callback
when certain events occur. These are events such as the system unit 
settings changing, system time setting changing, or the user executing 
File/Reset, File/New, etc. \n\n
This structure is part of this system. It is available in release 2.0 
and later only. The Interface class has related methods for registering callbacks. \n\n
The plug-in creates a callback function to process the notification. 
The notification callback function (NOTIFYPROC) is defined as follows:\n
<code>typedef void (* NOTIFYPROC)(void *param, NotifyInfo *info);</code> \n
The NotifyInfo structure is passed to the <b>NOTIFYPROC</b> to inform 
it of what it's being notified about. \n\n
The sample code below shows how this system may be used.
\par Sample Code:
\code
// Declare the callback function
static void TimeUnitsChanged(void *param, NotifyInfo *info)
{
// Handle the units changing...
}
// Register the callback
RegisterNotification(TimeUnitsChanged,this, NOTIFY_TIMEUNITS_CHANGE);

// When done, unregister the callback
UnRegisterNotification(TimeUnitsChanged,this, NOTIFY_TIMEUNITS_CHANGE);
\endcode
\par See Functions:
RegisterNotification(NOTIFYPROC proc, void *param, int code); \n
UnRegisterNotification(NOTIFYPROC proc, void *param, int code); \n
BroadcastNotification(int code); \n
BroadcastNotification(int code, void *callParam); \n
UnRegisterNotification(NOTIFYPROC proc, void *param);
*/
struct NotifyInfo: public MaxHeapOperators {
	int intcode;
	/*! This pointer is available in release 3.0 and later only. \n
	This parameter can be passed in with the function BroadcastNotification(int code, void *callParam). */
	void *callParam;  // this param can be passed in with BroadcastNotification;
};

// The notification callback function
typedef void (* NOTIFYPROC)(void *param, NotifyInfo *info);

// Integer versions -- For pre-defined MAX codes
/*! \remarks This global function is called to establish the connection between the event and the callback.
\param proc The callback function called when the event occurs
\param param A pointer to a parameter which will be passed to the callback function
\param code Specifies which notification to register. See \ref NotificationCodes.
\return Value Nonzero if the event was registered; otherwise zero.
*/
CoreExport int RegisterNotification(NOTIFYPROC proc, void *param, int code);
/*! \remarks This global function is called to break the connection between the event
and the callback. After this function executes the callback is no
longer invoked when the event occurs.
\param proc The callback function called when the event occurs.
\param param This parameter must be identical to the param sent into RegisterNotification().
This function will only unregister a callback if this parameter equals
the param sent in to the RegisterNotification() function.
\param code Specifies which notification to unregister. See \ref NotificationCodes
\return Nonzero if the event was unregistered; otherwise zero. */
CoreExport int UnRegisterNotification(NOTIFYPROC proc, void *param, int code);
/*! \remarks Calling this global function causes the callback
corresponding to the specified code to be called.
\param code Specifies which notification to broadcast. See \ref NotificationCodes */
CoreExport void BroadcastNotification(int code);
/*! \remarks This global function is available in release 3.0 and later only. \n
This causes the callback corresponding to the specified code to be called 
and passes the specified void* parameter along to the callback.
\param code Specifies which notification to broadcast. See \ref NotificationCodes 
\param callParam This parameter is passed to the callback. See the code NOTIFY_BITMAP_CHANGED 
for an example of this in use. */
CoreExport void BroadcastNotification(int code, void *callParam);

// Unregister a callback from all codes
/*! \remarks This global function unregisters the callback from all codes
\param proc The callback function called when the event occurs.
\param param A pointer to a parameter which will be passed to the callback function.
\return Nonzero if the events were unregistered; otherwise zero.
*/
CoreExport int UnRegisterNotification(NOTIFYPROC proc, void *param);

class ClassDesc;

class CreateInstanceCallParam: public MaxHeapOperators {
public:
	void* instance;
	ClassDesc* cd;
	CreateInstanceCallParam (void* i, ClassDesc* cd) : instance(i), cd(cd) { }
}; 

//! The call parameter that accompanies the notification code
//! \ref NOTIFY_RENDER_PREEVAL_FRAMEINFO is a pointer to this 
//! struct, which contains information about the view and the 
//! frame to be rendered.
struct NotifyPreEval_FrameInfo : public MaxHeapOperators {
	//! Information about the view being rendered - 
	//! same as that passed to GeomObject::GetRenderMesh().
	View* view;
	//! Information about the frame to be rendered.
	FrameRendParams* frp;
	//! The view node, as passed to Renderer::Open(). May be NULL.
	INode* viewNode;
	//! The view parameters, as passed to Renderer::Render(). 
	//! Only valid if 'viewNode' is NULL.
	ViewParams* viewParams;
};

//! \brief The name change call parameter structure.
/*! The call parameter that accompanies any notification that involves
	a name change of an object(\ref NameChangeNotifications), which contains 
	information of both the old name and the new name of the changed 
	object.
*/
struct NameChange{ 
	//! \brief The old name of the changed object.
	MCHAR* oldname;
	//! \brief The new name that is assigned to the changed object.
	MCHAR* newname;
};

//! \brief The object redefinition call parameter structure.
/*! Instance of this structure is passed as call param on NOTIFY_OBJECT_DEFINITION_CHANGE_BEGIN/NOTIFY_OBJECT_DEFINITION_CHANGE_END
	broadcast notifications.
*/
struct ObjectDefinitionChangeNotificationParam : public MaxHeapOperators
{
	//! \brief The types of object definition redefinition.
	enum ObjectDefinitionType{ 
		//! \brief A scripted plugin/scripted custom attribute. The definition passed will be a ClassDesc2.
		kScriptedPlugin = 0 
	};
	//! \brief The type of object definition being redefined
	ObjectDefinitionType m_type;
	//! \brief A pointer whose type is dependent on the type of object definition being redefined
	void* m_definition;
	ObjectDefinitionChangeNotificationParam(ObjectDefinitionType type, void* definition) : m_type(type), m_definition(definition) {}
private:
	ObjectDefinitionChangeNotificationParam();
};


