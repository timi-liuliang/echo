//*****************************************************************************
// Copyright 1994, 2009 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise accompanies 
// this software in either electronic or hard copy form.   
//
//*****************************************************************************
//
//	FILE: hold.h
//	DESCRIPTION:	Describes the hold system objects 
//	CREATED BY:	Dan Silva
//
//*****************************************************************************

#pragma once

#include "maxheap.h"
#include "baseinterface.h"
#include "strclass.h"

#define RESTOBJ_DOES_REFERENCE_POINTER		1 // cmd ID for RestoreObj::Execute
#define RESTOBJ_GOING_TO_DELETE_POINTER		2 // cmd ID for RestoreObj::Execute
#define REFTARG_AUTO_DELETED				10 // cmd ID for HoldStore::Execute

#define HOLD_SYSTEM_EMPTY					1 // cmd ID for Hold::Execute - return 1 if hold system is definitely empty
#define HOLD_POINTER_IS_MANAGER				2 // cmd ID for Hold::Execute - return 1 if arg1 is a pointer to the undo manager
#define HOLD_SUPERLEVEL						3 // cmd ID for Hold::Execute - return Hold.superLevel

class HoldStore;
/*! \sa  Class Hold, <a href="ms-its:3dsmaxsdk.chm::/undo_redo.html">Undo and Redo</a>.\n\n
\par Description:
This class is the restore object used in the undo / redo system of 3ds Max.
 */
class RestoreObj : public InterfaceServer {
	friend class HoldStore;
	friend class GenUndoObject;
	friend class CheckForFlush;
		RestoreObj *next,*prev;
	public:
		RestoreObj() { next = prev = NULL; }
		virtual ~RestoreObj() {};
		/*! \remarks The developer implements this method to restore the state of the
		database to as it was when <b>theHold.Put()</b> was called with this
		restore object.\n\n
		Regardless of the value of the isUndo parameter, the method must
		restore the state of the scene to the one it was in when theHold.Put()
		was called with this restore object.\n\n
		RestoreObj::Restore and RestoreObj::Redo may be called several times in
		a row: these methods must protect against  any problems that may arise
		because of this, such as trying to undo multiple times in a row what
		has been undone already.
		\param isUndo Nonzero if <b>Restore()</b> is being called in response to the Undo
		command; otherwise zero. If <b>isUndo</b> is nonzero, the developer
		needs to save whatever data they need to allow the user to redo the
		operation. */
		virtual void Restore(int isUndo)=0;
		/*! \remarks This method is called when the user selects the Redo command. The
		developer should restore the database to the state prior to the last
		Undo command. */
		virtual void Redo()=0;
		/*! \remarks Returns the size of the restore object in bytes. This size does not
		need to be exact but should be close. This is used to make sure all the
		accumulated restore objects do not grow beyond a manageable size.
		\par Default Implementation:
		<b>{ return 1; }</b>
		\return  The size of the restore object in bytes. */
		virtual int Size() { return sizeof(RestoreObj); }
		/*! \remarks This method is called when <b>theHold.Accept()</b> or
		<b>theHold.Cancel()</b> is called. This means the restore object is no
		longer held, it was either tossed out or sent to the undo system. The
		developer may then call <b>ClearAFlag(A_HELD)</b> to indicate the
		restore object is no longer being held. */
		virtual void EndHold() { }
		/*! \remarks This method is used internally to 3ds Max in debugging only. It is used
		to display a symbolic name for the restore object.
		\return  The name of the restore object.
		\par Default Implementation:
		<b>{ return MSTR(_M("---")); }</b> */
		virtual MSTR Description() { return MSTR(_M("---")); }
		/*! \remarks This is a general purpose function that allows the API to be extended
		in the future. The 3ds Max development team can assign new <b>cmd</b>
		numbers and continue to add functionality to this class without having
		to 'break' the API.\n\n
		This is reserved for future use.
		\par Default Implementation:
		<b>{return -1;}</b> */
		virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0)
		{
			UNUSED_PARAM(cmd);
			UNUSED_PARAM(arg1);
			UNUSED_PARAM(arg2);
			UNUSED_PARAM(arg3);
			return -1;
		}
	};

/*! \sa  Class RestoreObj, <a href="ms-its:3dsmaxsdk.chm::/undo_redo.html">Undo and Redo</a>.\n\n
\par Description:
The undo / redo system of 3ds Max uses a global instance of this class called
<b>theHold</b>. Developers call methods of <b>theHold</b> object to participate
in the undo / redo system.  */
class Hold : public BaseInterfaceServer {
	HoldStore *holdStore, *holdList;
	int undoEnabled;
	int superLevel;
	int putCount;
	HoldStore *ResetStore();
	void Init();
	void AddOpToUndoMgr(MCHAR *name,int id);
	public:
		CoreExport Hold();
		CoreExport ~Hold();
		/*! \remarks Implemented by the System.\n\n
		The developer calls this method to register a new restore object with
		the system. Once this object is registered the developer should set the
		<b>A_HELD</b> flag of <b>Animatable</b>, i.e. call
		<b>SetAFlag(A_HELD)</b>. This indicates that a restore object is
		registered with the system.
		\param rob The restore object to register.
		\par Example:
		Example code from <b>EDMREST.CPP</b>\n\n
		<b>if ( theHold.Holding() ) {</b>\n\n
		<b> theHold.Put(new MeshSelectRestore(meshData,this));</b>\n\n
		<b> }</b> */
		CoreExport void Put(RestoreObj *rob);
		/*! \remarks Implemented by the System.\n\n
		When a developer is about to modify the database they should check to
		see if <b>theHold</b> is 'holding'. This indicates that the
		<b>Begin()</b> method has been called. This signifies the beginning of
		a potential undo/redo operation. If <b>theHold</b> is not holding, they
		should call <b>Begin()</b>. After <b>Begin()</b> has been called the
		system is ready to accept restore objects.\n\n
		In certain cases the system may already be 'holding' when the plug-in
		is about to begin its modification to the database. For example
		controllers would normally not call <b>Begin()</b> because it usually
		has been called already. A procedural object will normally call
		<b>Begin()</b> because there is no other part of the system that may
		alter a procedural object so <b>Begin()</b> would not have been called.
		\par Example:
		<b>theHold.Begin();</b> */
		CoreExport void Begin();
		/*! \remarks Implemented by the System.\n\n
		This is used internally. It temporarily suspends holding. */
		CoreExport void Suspend();	  // temporarly suspend holding
		CoreExport int IsSuspended();
		/*! \remarks Implemented by the System.\n\n
		This is used internally. It resumes holding if it was suspended. */
		CoreExport void Resume();    // resume holding
		/*! \remarks Implemented by the System.\n\n
		This indicates if <b>theHold.Begin()</b> has been called. Any operation
		that modifies the database checks to see if <b>theHold</b> is currently
		in a holding state. If the undo system is 'holding' it is ready to
		accept restore objects. For more details see the Advanced Topics
		section on <a href="ms-its:3dsmaxsdk.chm::/undo_redo.html">Undo /
		Redo</a>.
		\return  Nonzero if <b>theHold</b> is holding; otherwise 0.
		\par Example:
\code	
		 if ( theHold.Holding() ) {
		  ...
		  } 
\endcode */
		CoreExport int	Holding();  // are we holding?
		/*! \remarks Implemented by the System.\n\n
		Returns nonzero if the system is restoring and zero if not.
		\param isUndo This parameter is updated to indicate if the restore is coming from an
		undo. It's assigned nonzero if it is; zero if not. */
		CoreExport int  Restoring(int& isUndo);
		/*! \remarks Implemented by the System.\n\n
		Returns nonzero if the system is redoing and zero if not. */
		CoreExport int  Redoing();
		/*! \remarks Implemented by the System.\n\n
		Returns nonzero if the system is restoring or redoing and zero if not.
		*/
		CoreExport int  RestoreOrRedoing();

		/*! \remarks Implemented by the System.\n\n
		This is used internally. Plug-In developers should not call this
		method. Prevents Undo when <b>Accept()</b> is called. */
		CoreExport void DisableUndo();  // prevents Undo when Accept is called.
		/*! \remarks Implemented by the System.\n\n
		This is used internally. Plug-In developers should not call this
		method. Allows Undo when <b>Accept()</b> is called. */
		CoreExport void EnableUndo();
		/*! \remarks Implemented by the System.\n\n
		This is used internally. Plug-In developers should not call this
		method. Returns state set by DisableUndo/EnableUndo. */
		CoreExport BOOL IsUndoDisabled();

		/*! \remarks Implemented by the System.\n\n
		Returns current nesting level of Begin() calls. */
		CoreExport int GetBeginDepth();

		/*! \remarks Implemented by the System.\n\n
		This method will call Restore() on all the restore objects registered
		since the last <b>Begin()</b>. This restores the database to the state
		it was in when <b>Begin()</b> was called. The restore objects are NOT
		deleted. */
		CoreExport void Restore();  // Restore changes from holdStore. 
		/*! \remarks Implemented by the System.\n\n
		This tosses out the restore objects since the last <b>Begin()</b> but
		still continues holding.\n\n
		Group several Begin-End lists into a single Super-group. */
		CoreExport void Release();  // Tosses out holdStore. 

		// 3 ways to terminate the Begin()...
		/*! \remarks Implemented by the System.\n\n
		This method is used internally to 3ds Max and should not be called by a
		plug-in developer. It leaves the database in its modified state but
		throws out the restore object. */
		CoreExport void End();  // toss holdStore.
		/*! \remarks Implemented by the System.\n\n
		Leaves the database in its modified state and registers an undo object
		with the undo system. This will allow the user to undo the operation.
		\param nameID The resource ID of the string to appear in the Edit menu next to Undo
		or Redo.
		\par Example:
		<b>theHold.Accept(IDS_MOVE);</b> */
		CoreExport void Accept(int nameID); // record Undo (if enabled), End();
		/*! \remarks Implemented by the System.\n\n
		Leaves the database in its modified state and registers an undo object
		with the undo system. This will allow the user to undo the operation.
		\param name The string to appear in the Edit menu next to Undo or Redo. */
		CoreExport void Accept(MCHAR *name);
		/*! \remarks Implemented by the System.\n\n
		Restores the database to its previous state and throws out the restore
		object. This cancels the operation. */
		CoreExport void Cancel();   // Restore changes, End() 

		//		
		// Group several Begin-End lists into a single Super-group.
		/*! \remarks Implemented by the System.\n\n
		Normally this is NOT needed but in special cases this can be useful.
		This allows a developer to group a set of Begin()/Accept() sequences to
		be undone in one operation.\n\n
		Consider the case of the user using the Shift-Move command to create a
		new node in the scene. There are two parts to this process. First the
		node must be cloned and second the position must be tracked as the user
		moves the mouse to place the new node in the scene. Naturally if the
		user wanted to Undo this operation, they would expect a single
		selection of the Undo command would accomplish it. However the process
		was not a single operation. There was the initial cloning of the node,
		and then the iterative process of moving the node in the scene,
		restoring its position, moving it again, restoring it again, etc. Cases
		like this are handled using methods of <b>theHold</b> named
		<b>SuperBegin(), SuperAccept()</b> and <b>SuperCancel()</b>. These
		allow the developer to group several restore objects together so that
		they may be undone via a single selection of Undo. Note that in this
		example it is only necessary to use <b>SuperBegin()</b> and
		<b>SuperAccept()</b> because the move was restoring interactively.
		Normally a developer does NOT need to use these methods even if they
		have several operations that modify the database. The undo system will
		automatically register all the restore objects needed as part of the
		undo object when <b>theHold.Accept()</b> is called and these may be
		undone using a single UNDO command.
		\par Sample Code:
		See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
		CoreExport void SuperBegin();
		/*! \remarks Implemented by the System.\n\n
		When a developer has used <b>SuperBegin()</b>, this method is used to
		Accept. This leaves the database in its modified state and registers
		the restore object with the undo system. This will allow the user to
		undo the operation.
		\param nameID The resource ID of the string to appear in the Edit menu next to Undo or Redo.
		\par Sample Code:
		See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
		CoreExport void SuperAccept(int nameID);
		/*! \remarks Implemented by the System.\n\n
		When a developer has used <b>SuperBegin()</b>, this method is used to
		Accept. This leaves the database in its modified state and registers
		the restore object with the undo system. This will allow the user to
		undo the operation.
		\param name The string to appear in the Edit menu next to Undo or Redo. */
		CoreExport void SuperAccept(MCHAR *name);
		/*! \remarks Implemented by the System.\n\n
		When a developer has used <b>SuperBegin()</b>, this method is used to
		Cancel. This restores the database to its previous state and throws out
		the restore object. This cancels the operation.
		\par Sample Code:
		See the sample program <b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>. */
		CoreExport void SuperCancel();
		/*! \remarks Implemented by the System.\n\n
		Returns current nesting level of SuperBegin() calls. */
		CoreExport int GetSuperBeginDepth();


		// Get the number of times Put() has been called in the current session of MAX
		/*! \remarks Returns the number of times <b>Put()</b> has been called in the current
		session of 3ds Max. */
		CoreExport int GetGlobalPutCount();

		CoreExport INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0);

		/*! \brief Returns the (approximate) memory usage of this hold.

		The memory returned is the sum of all RestoreObj's Size() methods, so the accuracy
		of this method depends on the accuracy of the restore objects.

		\see RestoreObj::Size
		\return Size, in bytes, of the restore objects held by this hold.
		!*/
		CoreExport DWORD_PTR Size() const;

	protected:
		friend HoldStore;
		unsigned flags;
		enum {
			kInRestore,
			kInUndo,
			kInRedo
		};
	};



extern CoreExport Hold theHold;

CoreExport void EnableUndoDebugPrintout(BOOL onoff);

// A class to help control the hold system. Create an instance of this class, and when it is
// destructed any hold suspends are resumed. Makes it safe in case a throw occurs after the suspend,
// but before the resume
class HoldSuspend: public MaxHeapOperators {
public:
	HoldSuspend(BOOL suspendNow = TRUE) : suspendCount(0) {
		if (suspendNow) {
			this->Suspend();
		}
	}
	~HoldSuspend()	{
		while(suspendCount > 0) {
			this->Resume();
		}
	}
	void Suspend() {
		if (suspendCount == 0) 
			theHold.Suspend ();
		suspendCount++;
	}
	void Resume() {
		if (suspendCount == 1) 
			theHold.Resume();
		if (suspendCount > 0) 
			suspendCount--;
	}

private:
	int	suspendCount;

};					  



