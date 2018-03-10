/*! \file PFSimpleAction.h
    \brief PF SimpleAction (abstract class) header
				 Features that are common for a simplest form of
				 PF Action. SimpleAction doesn't have a
				 visual representation in viewports therefore it
				 cannot depend on its own icon to determine parameters
				 of the operation. All parameters are defined through
				 ParamBlock2
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 05-28-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "IPFAction.h"
#include "IPViewItem.h"
#include "IPFIntegrator.h"
#include "IPFSystem.h"
#include "OneClickCreateCallBack.h"
#include "PFClassIDs.h"
#include "IPViewManager.h"
#include "RandObjLinker.h"
#include "..\iparamb2.h"

class PFSimpleAction:	public HelperObject,
						public IPFAction,
						public IPViewItem
{
public:
	PFExport virtual ~PFSimpleAction();

	// From InterfaceServer
	PFExport virtual BaseInterface* GetInterface(Interface_ID id);

	// From Animatable
	PFExport virtual void DeleteThis();
	virtual int NumSubs() { return 1; } // the paramBlock
	PFExport virtual Animatable* SubAnim(int i);
	PFExport virtual MSTR SubAnimName(int i);
	PFExport BOOL SelectSubAnim(int subNum);
	PFExport virtual ParamDimension* GetParamDimension(int i);
	PFExport virtual int SubNumToRefNum(int subNum);
	virtual BOOL CanCopyAnim() { return FALSE; }
	PFExport virtual int HasSubElements(int type);
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual BOOL CanDeleteSubAnim(int i) { return FALSE; }
	virtual BOOL IsAnimated() { return _pblock()->IsAnimated(); }
	virtual void FreeCaches() { ; }
	virtual int NumParamBlocks() { return 1; } // the paramBlock
	PFExport virtual IParamBlock2* GetParamBlock(int i);
	PFExport virtual IParamBlock2* GetParamBlockByID(short id);

	// From ReferenceMaker
	virtual int NumRefs() { return 1; }
	PFExport virtual RefTargetHandle GetReference(int i);
protected:
	PFExport virtual void SetReference(int i, RefTargetHandle rtarg);
public:
	PFExport virtual RefResult NotifyRefChanged(Interval changeInt,RefTargetHandle hTarget,PartID& partID, RefMessage message);
	PFExport virtual int RemapRefOnLoad(int iref);
	PFExport virtual IOResult Save(ISave *isave);
	PFExport virtual IOResult Load(ILoad *iload);

	// From BaseObject
	virtual CreateMouseCallBack* GetCreateMouseCallBack() { return OneClickCreateCallBack::Instance(); }
	virtual BOOL OKToChangeTopology(MSTR &modName) { return FALSE; }

	// From Object
	virtual void InitNodeName(MSTR& s) { s = GetObjectName(); }
	virtual ObjectState Eval(TimeValue t) { return ObjectState(this); }

	// --- These methods MUST be implemented by the derived class --- //
	// --- These methods have a default virtual implementation to --- //
	// --- ease PFExport implementation of Clone(...) method     --- //

	// constructor: inherited as a base class constructor
	PFSimpleAction()
	{
		_pblock() = NULL;
		RegisterParticleFlowNotification();
	}
	// implement it like this
	//	{
	//		GetClassDesc()->MakeAutoParamBlocks(this);
	//	}
	//

	// From Animatable
	virtual void GetClassName(MSTR& s) { ; }
	virtual Class_ID ClassID() { return Class_ID(0,0); }
	virtual void BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev) = 0;
	// implement it like this
	//	{
	//		GetClassDesc()->BeginEditParams(ip, this, flags, prev);
	//	}
	virtual void EndEditParams(IObjParam *ip,	ULONG flags,Animatable *next) = 0;
	//	{
	//		GetClassDesc()->EndEditParams(ip, this, flags, next );
	//	}

	// From ReferenceTarget
	virtual RefTargetHandle Clone(RemapDir &remap) = 0;
	// implement it like this
	//	{
	//		PFOperatorXXX* newOp = new PFOperatorXXX();
	//		newOp->ReplaceReference(0, remap.CloneRef(pblock()));
	//		BaseClone(this, newOp, remap);
	//		return newOp;
	//	}

	// From BaseObject
	virtual MCHAR *GetObjectName() = 0;

	// From IPFAction interface
	PFExport virtual bool	Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes);
	PFExport virtual bool	Release(IObject* pCont);

	// SimpleOperator doesn't support randomness
	// If you need randomness for the operator please override
	// the following four methods
	virtual bool	SupportRand() const	{ return false; }
	virtual int		GetRand()			{ return 0; }
	virtual void	SetRand(int seed)	{ ; }
	#pragma warning(pop)
	PFExport virtual int NewRand();

	virtual const ParticleChannelMask& ChannelsUsed(const Interval& time) const = 0;
	virtual const Interval ActivityInterval() const = 0;

	PFExport virtual IObject* GetCurrentState(IObject* pContainer);
	PFExport virtual void SetCurrentState(IObject* actionState, IObject* pContainer);

	// From IPViewItem interface
	virtual int NumPViewParamBlocks() const { return 1; } // the paramBlock
	PFExport virtual IParamBlock2* GetPViewParamBlock(int i) const;

	// supply ClassDesc descriptor for the concrete implementation of the operator
	virtual ClassDesc* GetClassDesc() const = 0;

public:
		PFExport void RefreshUI(WPARAM message, IParamMap2* map=NULL) const;
		PFExport void UpdatePViewUI(ParamID updateID) const;

		// const access to class members
		IParamBlock2*			pblock()		const	{ return m_pblock; }
		const RandObjLinker&	randLinker()	const	{ return m_randLinker; }

		// access to class members
		IParamBlock2*&	_pblock()			{ return m_pblock; }
		RandObjLinker&	_randLinker()		{ return m_randLinker; }

protected:
		// parameters
		IParamBlock2*	m_pblock;

		// to keep track of client particle systems
		// the test may serve several particle systems at once
		// each particle system has its own randomization scheme
		RandObjLinker	m_randLinker;
};

