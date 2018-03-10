/**********************************************************************
 
	FILE: INodeExposure.h

	DESCRIPTION:  Public interface for determining if a node should be
					exposed in misc. UIs (SelectObjects, TrackView etc).

	CREATED BY: Oleg Bayborodin

	HISTORY: - created March 21, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "inode.h"

#define NODEEXPOSURE_INTERFACE Interface_ID(0x1b00412c, 0x6ebf48f2)
#define NODEEXPOSURE_INTERFACE_TOAPPEND Interface_ID(0x1b00412c, 0x6ebf48f3)


/*! class INodeExposure: public FPMixinInterface
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This interface provides the ability for a node to define whether it is visible
in any of max's dialog boxes. This interface will be extended and used by more
of 3ds Max's core utilities, but <b>currently ONLY TrackView and the Select
Object/HideObject dialog box use this interface</b>. By default this interface
is not available through the default nodes, it needs to be added.\n\n
To get a pointer to this interface from a node the following code can be
used.
\code
INodeExposure* iNE = (INodeExposure*)node->GetInterface(NODEEXPOSURE_INTERFACE_TOAPPEND)
\endcode 
\n
This will add a new INodeExposure interface to the node if it is not present.
The next time you use this technique it will only return the interface and not
create another new interface.\n\n

\par Data Members:
<b>enum { </b>\n\n
<b>  kSelectObjects, kSchematicView, kMaxscript,</b>\n\n
<b>  kMerge, kMergeAnimation, kReplace,</b>\n\n
<b>  kKeyEditor, kCurveEditor, kRangeEditor,</b>\n\n
<b> };</b>\n\n
This enum provides access to the different supported UI elements. It is used
with various methods of the class to get/set the UI flags.   */
class INodeExposure : public FPMixinInterface {
public:

enum {	
		kSelectObjects,	kSchematicView,		kMaxscript,
		kMerge,			kMergeAnimation,	kReplace, 
		kKeyEditor,		kCurveEditor,		kRangeEditor, // three components for TrackView editors
		kUI_num // always last in the enum
	};

	// Utilities:
	static INodeExposure* GetINodeExposure(INode& n) {
		return static_cast<INodeExposure*>(n.GetInterface(NODEEXPOSURE_INTERFACE)); }

	static INodeExposure* AppendINodeExposure(INode& n) {
		return static_cast<INodeExposure*>(n.GetInterface(NODEEXPOSURE_INTERFACE_TOAPPEND)); }

	// exposure in a specific UI
	/*! \remarks This will return the exposure state of the UI element being
	queried\n\n

	\par Parameters:
	<b>int ui</b>\n\n
	The UI flag to query. This should be a value from the UI enum - See data
	members section\n\n
	  */
	virtual bool IsExposed(int ui) const =0;
	// set exposure in all UIs
	/*! \remarks This will set the state of all the UI elements to the state
	passed into the method\n\n

	\par Parameters:
	<b>bool state</b>\n\n
	The state to set the nodes UI exposure\n\n
	  */
	virtual void SetExposed(bool state) =0;
	// set exposure in a specific UI
	/*! \remarks This will set the state of the individual UI element to the
	state passed into the method\n\n

	\par Parameters:
	<b>bool state</b>\n\n
	The state to set the nodes UI exposure\n\n
	<b>int ui</b>\n\n
	The UI element to set */
	virtual void SetExposed(bool state, int ui) =0;

	// Derived methods. They are all inline'd at the end of the file.
	/*! \remarks This will return true if it is visible in Selected
	Objects/HideObjects Dialog box otherwise false\n\n
	  */
	bool IsExposedInSelectObjects() const;
	bool IsExposedInSchematicView() const;
	bool IsExposedInMaxscript() const;
	bool IsExposedInMerge() const;
	bool IsExposedInMergeAnimation() const;
	bool IsExposedInReplace() const;
	/*! \remarks This will return true if it is visible in TrackView or false
	if it is not. This will return TRUE is the node is exposed in <b>ANY</b> of
	the TrackView states, kKeyEditor, kCurveEditor and kRangeEditor. It will
	return FALSE if <b>ALL</b> are set to false.\n\n
	  */
	bool IsExposedInTrackView() const;
	/*! \remarks Specifies whether the node is visible in the Key Editor of
	TrackView */
	bool IsExposedInKeyEditor() const;
	/*! \remarks Specifies whether the node is visible in the Function Curve
	Editor of Track %View */
	bool IsExposedInCurveEditor() const;
	/*! \remarks Specifies whether the node is visible in the Key Range Editor
	of Track %View */
	bool IsExposedInRangeEditor() const;
	/*! \remarks This will set the flag for the exposure in Selected
	Objects/HideObjects dialog box\n\n

	\par Parameters:
	<b>bool state</b>\n\n
	The value to set the flag\n\n
	  */
	void SetExposedInSelectObjects(bool state);
	void SetExposedInSchematicView(bool state);
	void SetExposedInMaxscript(bool state);
	void SetExposedInMerge(bool state);
	void SetExposedInMergeAnimation(bool state);
	void SetExposedInReplace(bool state);
	/*! \remarks This allows the state of the TrackView exposure flag to be
	set by the user. This will set the flag for all three TrackView flags. See
	comment in IsExposedInTrackView().\n\n
	  */
	void SetExposedInTrackView(bool state);
	/*! \remarks This allows the state of the Key Editor of Trackview exposure
	flag to be set by the user.\n\n
	  */
	void SetExposedInKeyEditor(bool state);
	/*! \remarks This allows the state of the Function Curve Editor of Track
	view exposure flag to be set by the user.\n\n
	  */
	void SetExposedInCurveEditor(bool state);
	/*! \remarks This allows the state of the Key Range Editor of Track view
	exposure flag to be set by the user.*/
	void SetExposedInRangeEditor(bool state);
	
	virtual IOResult	Save(ISave*) const =0;
	virtual IOResult	Load(ILoad*) =0;
	virtual void	 	Copy(INodeExposure*) =0;

	// makes exposure parameters to be permanent (not mutable)
	virtual void BakeExposure() =0;
	// defines if exposure is baked
	virtual bool IsExposureBaked() =0;

	// Used by implementation class:
	virtual const void* ObjectOf(void*) const =0;
	virtual		  void*	ObjectOf(void*) =0;

	// Due to FPMixinInterface:
	FPInterfaceDesc* GetDesc() { return GetDescByID(NODEEXPOSURE_INTERFACE); }

	// Function publishings:
	//
	enum FuncID {
		kIsExposed, kSetExposedInAll, kSetExposed, kBakeExposure
	};

BEGIN_FUNCTION_MAP
	FN_1(kIsExposed,	TYPE_bool,	IsExposed,	TYPE_INT);
	VFN_1(kSetExposedInAll,			SetExposed, TYPE_bool);
	VFN_2(kSetExposed,				SetExposed, TYPE_bool, TYPE_INT);
	VFN_0(kBakeExposure,			BakeExposure );
END_FUNCTION_MAP

};

// inlines for derived methods
//
inline bool INodeExposure::IsExposedInSelectObjects() const
{
	return IsExposed(kSelectObjects);
}

inline bool INodeExposure::IsExposedInSchematicView() const
{
	return IsExposed(kSchematicView);
}

inline bool INodeExposure::IsExposedInMaxscript() const
{
	return IsExposed(kMaxscript);
}

inline bool INodeExposure::IsExposedInMerge() const
{
	return IsExposed(kMerge);
}

inline bool INodeExposure::IsExposedInMergeAnimation() const
{
	return IsExposed(kMergeAnimation);
}

inline bool INodeExposure::IsExposedInReplace() const
{
	return IsExposed(kReplace);
}

inline bool INodeExposure::IsExposedInTrackView() const
{
	return (IsExposed(kKeyEditor) || IsExposed(kCurveEditor) || IsExposed(kRangeEditor));
}

inline bool INodeExposure::IsExposedInKeyEditor() const
{
	return IsExposed(kKeyEditor);
}

inline bool INodeExposure::IsExposedInCurveEditor() const
{
	return IsExposed(kCurveEditor);
}

inline bool INodeExposure::IsExposedInRangeEditor() const
{
	return IsExposed(kRangeEditor);
}

inline void INodeExposure::SetExposedInSelectObjects(bool state)
{
	SetExposed(state, kSelectObjects);
}

inline void INodeExposure::SetExposedInSchematicView(bool state)
{
	SetExposed(state, kSchematicView);
}

inline void INodeExposure::SetExposedInMaxscript(bool state)
{
	SetExposed(state, kMaxscript);
}

inline void INodeExposure::SetExposedInMerge(bool state)
{
	SetExposed(state, kMerge);
}

inline void INodeExposure::SetExposedInMergeAnimation(bool state)
{
	SetExposed(state, kMergeAnimation);
}

inline void INodeExposure::SetExposedInReplace(bool state)
{
	SetExposed(state, kReplace);
}

inline void INodeExposure::SetExposedInTrackView(bool state)
{
	SetExposed(state, kKeyEditor);
	SetExposed(state, kCurveEditor);
	SetExposed(state, kRangeEditor);
}

inline void INodeExposure::SetExposedInKeyEditor(bool state)
{
	SetExposed(state, kKeyEditor);
}

inline void INodeExposure::SetExposedInCurveEditor(bool state)
{
	SetExposed(state, kCurveEditor);
}

inline void INodeExposure::SetExposedInRangeEditor(bool state)
{
	SetExposed(state, kRangeEditor);
}



