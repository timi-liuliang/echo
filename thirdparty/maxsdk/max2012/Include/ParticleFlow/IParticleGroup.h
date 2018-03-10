/*! \file IParticleGroup.h
    \brief Interface for ParticleGroup object.
				 ParticleGroup is a geometry object that is created
				 for each interrelated pair of particle system and
				 action list.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 12-10-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\object.h"

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PARTICLEGROUP_INTERFACE Interface_ID(0x2c712d9f, 0x7bc54cb0) 
#define GetParticleGroupInterface(obj) ((IParticleGroup*)((GetPFObject(obj))->GetInterface(PARTICLEGROUP_INTERFACE))) 

class IParticleGroup : public FPMixinInterface
{

public:
    /** @defgroup IParticleGroup IParticleGroup.h
    *  @{
    */

	/*! \fn virtual INode* GetParticleSystem() const = 0;
      \brief get a master particle system for the particle group
      */
	virtual INode* GetParticleSystem() const = 0;


	/*! \fn virtual void SetParticleSystem(INode* pSystem) = 0;
      \brief set a reference on a master particle system
      */
	virtual void SetParticleSystem(INode* pSystem) = 0;

	/*! \fn virtualINode* GetActionList() const = 0;
      \brief get a master action list for the particle group
      */
	virtual INode* GetActionList() const = 0;

	/*! \fn virtual void SetActionList(INode* aList) = 0;
      \brief set a reference on a master action list
      */
	virtual void SetActionList(INode* aList) = 0;

	/*! \fn virtual int GetActiveStatus() const = 0;
      \brief get activity status of the group; the group can be put into "idle" mode
	// if returns zero then the group is in the "idle" mode
      */
	virtual int GetActiveStatus() const = 0;

	/*! \fn virtual void SetActiveStatus(int status) = 0;
      \brief set activity status of the group
      */
	virtual void SetActiveStatus(int status) = 0;

	/*! \fn virtual int UpdateActiveStatus() = 0;
      \brief updates activity status of the group taking into consideration
	schematics of the master particle system and master action list
	if the group is "fertile" but the master particle system does not
	have direct association with the master action list then the group
	is put into "idle" mode
	returns activity status to be set
      */
	virtual int UpdateActiveStatus() = 0;

	/*! \fn virtual void UpdateActionSet() = 0;
      \brief updates set of actions in the particle group
	the set is composed from global actions from the master particle system and
	local actions from the associated master actionList
      */
	virtual void UpdateActionSet() = 0;

	/*! \fn virtual Mtl* GetMaterial() const = 0;
      \brief get material from the list of actions
	the last action that has a material is the one
      */
	virtual Mtl* GetMaterial() const = 0;

	/*! \fn virtual DWORD GetWireColor() const = 0;
      \brief get wire color from the list of actions
	the last action that has a PFViewport interface is the one
      */
	virtual DWORD GetWireColor() const = 0;

	/*! \fn virtual IObject* GetParticleContainer() const = 0;
      \brief get a particle container for the particle group
      */
	virtual IObject* GetParticleContainer() const = 0;

	/*! \fn virtual const Interval& GetValidityInterval() const = 0;
      \brief get validity interval for the particle group
      */
	virtual const Interval& GetValidityInterval() const = 0;

	/*! \fn virtual void SetValidityInterval(Interval& validInterval) = 0;
      \brief set validity interval for the particle group
      */
	virtual void SetValidityInterval(Interval& validInterval) = 0;

	/*! \fn virtual void Init(TimeValue time) = 0;
      \brief set initial state for the particle group with time
      */
	virtual void Init(TimeValue time) = 0;

	/*! \fn virtual TimeValue GetFertilityTime() = 0;
      \brief returns fertility time of the particle group: the first time moment
	when the particle group can generate particles from scratch
	if the group is not active then it is not "fertile"
      */
	virtual TimeValue GetFertilityTime() = 0;

	/*! \fn virtual bool Update(TimeValue t) = 0;
      \brief advance particles in the group to the given time
      */
	virtual bool Update(TimeValue t) = 0;

	/*! \fn virtual bool PreUpdate(TimeValue t) = 0;
      \brief the following method allows to apply the actions Proceed method as a pre procedure before
	any modification to the particle container has been made. It is not recommended to change
	the particle container in this method. It is mostly to inquiring particle properties.
      */
	virtual bool PreUpdate(TimeValue t) = 0;

	/*! \fn virtual bool PostUpdate(TimeValue t) = 0;
      \brief the following method allows to apply the actions Proceed method as a post procedure when the
	amount of particles in all particle groups has been established and there won't be any
	particle transfers from one particle group to another
      */
	virtual bool PostUpdate(TimeValue t) = 0;

	/*! \fn virtual bool TransferSurplusContainer(IObject*& pCont, INode*& actionListTo) = 0;
      \brief returns a ptr at particleContainer to be transferred to another Event
	the particle group looses this ptr completely
	if the group doesn't have surplus containers then it returns false
      */
	virtual bool TransferSurplusContainer(IObject*& pCont, INode*& actionListTo) = 0;

	/*! \fn virtual bool AppendSurplusContainer(IObject* pCont) = 0;
      \brief appends the given container to the container of the group but first
	the given container is advanced in time to be in sync with the container of the group
      */
	virtual bool AppendSurplusContainer(IObject* pCont) = 0;

	/*! \fn virtual bool IsSync(TimeValue& time) = 0;
      \brief returns true if content of the particle group is synchronized in time
	if group is in "sync" then it doesn't have surplus containers
	if false then the method returns the closest sync time for the particle group
	it could be a time of the closest cache moment. The closest time is always
	smaller (earlier) then the request time.
      */
	virtual bool IsSync(TimeValue& time) = 0;

	/*! \fn virtual bool InstantUpdate(TimeValue t) = 0;
      \brief set particles in the group to the given time
	it should not involve any history dependent operations
	if the method is called it implies that the group is able to make
	an instant jump to the given time thus there is a cache for that time
      */
	virtual bool InstantUpdate(TimeValue t) = 0;

	/*! \fn virtual bool SetFinalUpdateTime(TimeValue t) = 0;
      \brief inform the group what would be a final time for the series of update calls
      */
	virtual bool SetFinalUpdateTime(TimeValue t) = 0;

	/*! \fn virtual void SetHandle(ULONG handle) = 0;
      \brief Informs the particle group about the handle of the particle group node
	the handle is later used to uniquely identify particle container for cache purposes
      */
	virtual void SetHandle(ULONG handle) = 0;


	/*! \fn virtual void InvalidateContainer(int type=invalidateCurrent) = 0;
      \brief clear particle content in the particle group
	then enum should be in sync with IPFCache
      */
	enum { invalidateCurrent=0, invalidateViewport=1, invalidateRender=2, invalidateBoth=3};
	virtual void InvalidateContainer(int type=invalidateCurrent) = 0;

	/*! \fn virtual void InvalidateCaches(int type=invalidateCurrent) = 0;
      \brief clear caches in the particle group
      */
	virtual void InvalidateCaches(int type=invalidateCurrent) = 0;


	/*! \fn virtual bool HasDelayedInvalidation() const = 0;
      \brief in normal circumstances when a particle group receives a change notification message from
	the encapsulated operator/test the particle group should invalidate itself. However if
	the particle system is in the stage of proceeding then it is not possible to invalidate
	particle group right away. The particle group can know the status of the particle system
	by using global IsPFProceeding method (IPViewManager.h). Then if the system has the proceed
	status then the particle group should delay the invalidation.
      */
	virtual bool HasDelayedInvalidation() const = 0;
 
      /*! \fn virtual bool DoDelayedInvalidation() = 0;
      \brief check if the group has a delayed invalidation
      */
	virtual bool DoDelayedInvalidation() = 0;

	/*! \fn virtual void InvalidateActions() = 0;
      \brief Particle Group stores pointers at IPFAction interfaces for efficiency. Those pointers
	  are maintained automatically. In some circumstances (during Replace) it is necessary to flush
	  those interfaces because they may become invalid.
      */
	virtual void InvalidateActions() = 0;
 
       /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLEGROUP_INTERFACE); }
      \brief perform invalidation that was delayed
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLEGROUP_INTERFACE); }
};

inline IParticleGroup* ParticleGroupInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetParticleGroupInterface(obj));
};

inline IParticleGroup* ParticleGroupInterface(INode* node) {
	return ((node == NULL) ? NULL : ParticleGroupInterface(node->GetObjectRef()));
};


