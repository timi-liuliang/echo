/**********************************************************************
 *<
	FILE: sceneapi.h

	DESCRIPTION: Scene interface

	CREATED BY:	Rolf Berteig

	HISTORY: Created 13 January 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include "maxtypes.h"

class BaseObject;
class ITreeEnumProc;

/*! \sa  Class ITreeEnumProc, Class INode.\n\n
\par Description:
Methods of this class may be used to enumerate the scene and to flag certain
nodes in the scene. Nodes chosen by the plug-in may be flagged using the
<b>EnumTree()</b> method. Selected nodes may be flagged using
<b>FlagFGSelected()</b>. Animated nodes may be flagged using
<b>FlagFGAnimated()</b> and dependent nodes may be flagged using
<b>FlagFGDependent()</b>.  */
class IScene: public MaxHeapOperators {		
	public:
		/*! \remarks Implemented by the System.\n\n
		This may be called to enumerate every INode in the scene. The callback
		may flag any of these nodes (using <b>INode::FlagForeground()</b>).
		\param proc This callback object is called once for each INode in the scene.
		\return  Nonzero if the process was aborted by the callback
		(<b>TREE_ABORT</b>); otherwise 0. */
		virtual int EnumTree( ITreeEnumProc* proc )=0;
		/*! \remarks Implemented by the System.\n\n
		Flags all selected nodes in the scene.
		\param t The time to flag the nodes. */
		virtual void FlagFGSelected( TimeValue t )=0;
		/*! \remarks Implemented by the System.\n\n
		Flags all animated nodes in the scene.
		\param t The time to flag the nodes. */
		virtual void FlagFGAnimated( TimeValue t )=0;
		/*! \remarks Implemented by the System.\n\n
		Flags nodes that are dependent on the given object.
		\param t The time to flag the nodes.
		\param obj The object whose dependent nodes should be flagged. */
		virtual void FlagFGDependent( TimeValue t, BaseObject *obj )=0;
	};


// The purpose of this callback is to call FlagForeground() for
// any nodes in the scene that are supposed to be in the foreground.
class ChangeForegroundCallback: public MaxHeapOperators {
	public:
		virtual ~ChangeForegroundCallback() {;}
		virtual BOOL IsValid()=0;
		virtual void Invalidate()=0;
		virtual void Validate()=0;
		virtual void callback(TimeValue t,IScene *scene)=0;
	};

