//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        referenceMgr.h
// DESCRIPTION: manages references to any maxsdk ReferenceTarget
// AUTHOR:      michael malone (mjm)
// HISTORY:     created February 01, 2000
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include "ref.h"

class RefTarget: public MaxHeapOperators
{
protected:
	RefTargetHandle mRef;
	Interval mValid;
	bool mHeld;

public:
	RefTarget() : mRef(NULL), mHeld(false) { }
	RefTarget(RefTargetHandle ref, const Interval& valid = NEVER) : mRef(ref), mHeld(false) { mValid = valid; }
	RefTarget(const RefTarget& refTarget) : mRef(refTarget.mRef), mHeld(false) { mValid = refTarget.mValid; }
	virtual ~RefTarget() {;}
	virtual RefTarget& operator=(const RefTarget& refTarget) { mRef = refTarget.mRef; mValid = refTarget.mValid; return *this; }

	RefTargetHandle GetRefTargetHandle() const { return mRef; }
	void SetRefTargetHandle(RefTargetHandle ref) { mRef = ref; }
	const Interval& GetValidity() const { return mValid; }
	void SetValidity(const Interval& valid) { mValid = valid; }
	bool IsHeld() const { return mHeld; }
	void SetHeld(bool held) { mHeld = held; }
}; 

// forward declaration
template <class T> class RefMgr;

// template type must be class RefTarget (or derived from it)
// We handle undo and redo special in the RefMgr class. This is because
// We need to save the object that we are storing along with the
// reference. We override DeleteReference to add a special undo
// record to the undo stack, and we also add an undo record at the
template <class T> class RefMgrAddDeleteRestore : public RestoreObj, public ReferenceMaker
{
	RefMgr<T>*			mMgr;			// The RefMgr that we are restoring
	T*					mRefTarg;		// The data that needs to be restored
										// This is NULL if the reference needs removing
	RefTargetHandle		mRef;			// The ReferenceTarget we are restoring
	BOOL				undone;			// True if undone

public:
	RefMgrAddDeleteRestore(RefMgr<T>* mgr, int which, RefTargetHandle ref)
		: mMgr(NULL), mRefTarg(which < 0 ? NULL : mgr->GetRefTarget(which)), mRef(ref), undone(FALSE)
	{
		ReplaceReference(0, mgr);
		if (mRefTarg != NULL)
			mRefTarg->SetHeld(true);
	}
		
	~RefMgrAddDeleteRestore()
	{
		if (mRefTarg != NULL) {
			if (mMgr != NULL)
				mMgr->RemoveRef(mRefTarg);
			delete mRefTarg;
		}
		DeleteAllRefs();
	}

	virtual Class_ID ClassID() { return Class_ID(0x102f266c, 0x23492667); }

// Restore handles both undo and redo. It swaps mRefTarg with any
// reference already in the RefMgr.
// Really should implement both Restore and Redo, but doing so breaks 
// compatibility. So Restore needs to check to see whether it is really doing
// a Restore or a Redo.
// In some cases, Restore can be called twice in a row, with no
// interleaving Redo(). For example, theHold.Restore();theHold.Cancel();
// No know case of Redo being called twice in a row, but play it safe.
// member variable 'undone' used as interlock
	void Restore(int isUndo)
	{
		int isUndo2;
		BOOL isInRestore = theHold.Restoring(isUndo2);
		if (isInRestore)  // really doing a restore
		{
			if (undone) return;
			undone = TRUE;
		}
		else  // really doing a redo
		{
			if (!undone) return;
			undone = FALSE;
		}
		if (mMgr != NULL) {
			if (mRefTarg == NULL) {
				int i = mMgr->FindRefTargetHandleIndex(mRef);
				if (i >= 0) {
					mRefTarg = mMgr->mRefTargetPtrs[i];
					mRefTarg->SetHeld(true);
					mMgr->mRefTargetPtrs.Delete(i, 1);
				}
			} else {
				mMgr->RemoveRef(mRefTarg);
				mRefTarg->SetRefTargetHandle(mRef);
				mMgr->mRefTargetPtrs.Append(1, &mRefTarg, 3);
				mRefTarg->SetHeld(false);
				mRefTarg = NULL;
			}
		}
	}
				
	void Redo() { Restore(false); }

	MSTR Description() { return _M("RefMgrAddDeleteRestore"); }

	virtual RefResult NotifyRefChanged(
		Interval			changeInt,
		RefTargetHandle		hTarget,
		PartID&				partID,
		RefMessage			message
	)
	{
		switch (message) {
		case REFMSG_TARGET_DELETED:
			mMgr = NULL;
			break;
		}

		return REF_SUCCEED;
	}
	virtual int NumRefs() { return 1; }
	virtual RefTargetHandle GetReference(int i) { return i == 0 ? mMgr : NULL; }
protected:
	virtual void SetReference(int i, RefTargetHandle rtarg)
	{
		if (i == 0)
			mMgr = static_cast<RefMgr<T>*>(rtarg);
	}
public:
	virtual BOOL CanTransferReference(int i) { return FALSE; }
};


// template type must be class RefTarget (or derived from it)
template <class T> class RefMgr : public ReferenceTarget
{
	friend class RefMgrAddDeleteRestore<T>;

protected:
	typedef RefResult (* TNotifyCB)(void *cbParam, Interval changeInt, T* pRefTarget, RefTargetHandle hTarger, PartID& partID, RefMessage message);

	Tab<T*> mRefTargetPtrs;
	TNotifyCB mNotifyCB;
	void *mNotifyCBParam;

public:
	RefMgr() : mNotifyCB(NULL), mNotifyCBParam(NULL) { }

	virtual ~RefMgr() { DeleteAllRefs(); }

	virtual Class_ID ClassID() { return Class_ID(0x7eff2f08, 0x25707aa2); }

	virtual void DeleteThis() 
	{ 
		delete this; 
	}

	virtual void Init(TNotifyCB notifyCB=NULL, void *notifyCBParam=NULL)
	{
		mRefTargetPtrs.ZeroCount();
		mNotifyCB = notifyCB;
		mNotifyCBParam = notifyCBParam;
	}

	virtual int Count() { return mRefTargetPtrs.Count(); }

	virtual int FindRefTargetHandleIndex(RefTargetHandle rtarg)
	{
		int n = mRefTargetPtrs.Count();
		for (int i=0; i<n; i++)
		{
			if (mRefTargetPtrs[i]->GetRefTargetHandle() == rtarg)
				return i;
		}
		return -1;
	}

	virtual int FindRefTargetIndex(T *pRefTarget)
	{
		int n = mRefTargetPtrs.Count();
		for (int i=0; i<n; i++)
		{
			if (mRefTargetPtrs[i]->GetRefTargetHandle() == pRefTarget->GetRefTargetHandle())
				return i;
		}
		return -1;
	}

	virtual T* FindRefTarget(RefTargetHandle rtarg)
	{
		int index = FindRefTargetHandleIndex(rtarg);
		return (index == -1) ? NULL : mRefTargetPtrs[index];
	}

	virtual T* GetRefTarget(int i)
	{
		if ( i < mRefTargetPtrs.Count() )
			return mRefTargetPtrs[i];
		else
			return NULL;
	}

	virtual bool AddUndo(int which, RefTargetHandle ref)
	{
		bool undo = false;
		if (!theHold.RestoreOrRedoing()) {
			int resumeCount = 0;

			while (theHold.IsSuspended()) {
				theHold.Resume();
				++resumeCount;
			}

			undo = theHold.Holding() != 0;
			if (undo)
				theHold.Put(new RefMgrAddDeleteRestore<T>(this, which, ref));
			
			while (--resumeCount >= 0)
				theHold.Suspend();
		}
		return undo;
	}

	virtual bool AddRef(T* pRefTarget)
	{
		// make sure rtarg is valid and hasn't already been referenced
		RefTargetHandle pRefTargHandle = pRefTarget->GetRefTargetHandle();
		if ( (pRefTargHandle != NULL) && (FindRefTargetIndex(pRefTarget) == -1) )
		{
			pRefTarget->SetRefTargetHandle(NULL);
			mRefTargetPtrs.Append(1, &pRefTarget);
			HoldSuspend hs;
			if ( ReplaceReference (mRefTargetPtrs.Count()-1, pRefTargHandle) != REF_SUCCEED )
			{
				pRefTarget->SetRefTargetHandle(pRefTargHandle);
				mRefTargetPtrs.Delete(mRefTargetPtrs.Count()-1, 1);
				return false;
			}
			hs.Resume();
			NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);

			// Handle undo if we need to
			AddUndo(-1, pRefTarget->GetRefTargetHandle());
			return true;
		}
		else
			return false;
	}

// mjm - begin - 06.20.00
	virtual bool RemoveRef(RefTargetHandle rtarg)
	{
		return RemoveRef( FindRefTargetHandleIndex(rtarg) );
	}

	virtual bool RemoveRef(T* pRefTarget)
	{
		return RemoveRef( FindRefTargetIndex(pRefTarget) );
	}

	virtual bool RemoveRef(int index)
	{
		if ( (index >= 0) && ( index < Count() ) )
		{
			DeleteReference(index);
			T* targ = mRefTargetPtrs[index];
			if (targ != NULL && !targ->IsHeld())
			{
				delete targ;
			}
			mRefTargetPtrs.Delete(index, 1);
			NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
			return true;
		}
		return false;
	}
// mjm - end

	virtual void Clear()
	{
		DeleteAllRefsFromMe();
		for ( int i = mRefTargetPtrs.Count(); --i >= 0; ) {
			T* targ = mRefTargetPtrs[i];
			if (targ != NULL && !targ->IsHeld())
			{
				delete targ;
			}
		}
		mRefTargetPtrs.ZeroCount();
		NotifyDependents(FOREVER, 0, REFMSG_SUBANIM_STRUCTURE_CHANGED);
	}


	// from class ReferenceMaker
	virtual int NumRefs() { return Count(); }

	// In order to make undo work we need to keep an undo record
	// for the T* object we keep with the reference.
	virtual RefResult DeleteReference(int which) {
		RefTargetHandle ref = GetReference(which);
		RefResult ret = ReferenceTarget::DeleteReference(which);
		if (ret == REF_SUCCEED) {
			AddUndo(which, ref);
		}
		return ret;
	}

	virtual ReferenceTarget* GetReference(int i)
	{
		if ( i < mRefTargetPtrs.Count() )
			return mRefTargetPtrs[i]->GetRefTargetHandle();
		else
			return NULL;
	}

protected:
	virtual void SetReference(int i, RefTargetHandle rtarg)
	{
		// Ignore SetReference during redo or undo, this is because the
		// RefMgrAddDeleteRestore will fix up everything.
		if (!theHold.RestoreOrRedoing()) {
			int count = mRefTargetPtrs.Count();
			assert(i < count || rtarg == NULL);
			if ( i < count )
				mRefTargetPtrs[i]->SetRefTargetHandle(rtarg);
		}
	}
public:
	virtual RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message)
	{
		RefResult refResult(REF_SUCCEED);
		if (mNotifyCB)
		{
			T* pRefTarget = FindRefTarget(hTarget);
			refResult = mNotifyCB(mNotifyCBParam, changeInt, pRefTarget, hTarget, partID,  message);
		}

		switch (message)
		{
			case REFMSG_TARGET_DELETED:
			{
// mjm - begin - 6.20.00
				int i = FindRefTargetHandleIndex(hTarget);
				assert(i >= 0);
				if (i >= 0)
				{
					// We need to handle undo
					if (!AddUndo(i, hTarget))
					{
						delete mRefTargetPtrs[i];
					}
					mRefTargetPtrs.Delete(i, 1);
				}
// mjm - end
				break;
			}
		}
		return refResult;
	}
};

