/*! \file IPFAction.h
    \brief Action-generic interface IPFAction.
				This is a part of every PF Operator & Test.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-12-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "ParticleChannelMask.h"
#include "..\ifnpub.h"
#include "..\object.h"
// forward declarations
class IParamBlock2;

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFACTION_INTERFACE Interface_ID(0x74f93d00, 0x1eb34500) 
#define GetPFActionInterface(obj) ((IPFAction*)((GetPFObject(obj))->GetInterface(PFACTION_INTERFACE))) 

// The propID is used by a particle group to define "final update time"
// in particle actions; particle group sets object property with this id
// to define the time value; later a particle action (operator or test)
// can find out what is the final time of a particle system evaluation
// and skip some steps in order to improve performance
#define PROPID_FINALUPDATETIME	0x00635690

class IPFAction : public FPMixinInterface
{
public:

	// function IDs
	enum {	kInit, 
			kRelease,
			kChannelsUsed, // reserved for future use
			kActivityInterval,
			kIsFertile,
			kIsNonExecutable,
			kSupportRand,
			kGetRand,
			kSetRand,
			kNewRand,
			kIsMaterialHolder,
			kGetMaterial,
			kSetMaterial,
			kSupportScriptWiring,
			kGetUseScriptWiring,
			kSetUseScriptWiring
	}; 

	// Function Map for Function Publish System 
	//***********************************
	#pragma warning(push)
	#pragma warning(disable:4239 4238)
	BEGIN_FUNCTION_MAP

		FN_5(kInit, TYPE_bool, Init, TYPE_IOBJECT, TYPE_OBJECT, TYPE_INODE, TYPE_OBJECT_TAB_BR, TYPE_INODE_TAB_BR);
		FN_1(kRelease, TYPE_bool, Release, TYPE_IOBJECT);
		VFN_2(kChannelsUsed, ChannelsUsed, TYPE_INTERVAL_BR, TYPE_FPVALUE); // reserved for future use
		FN_0(kActivityInterval, TYPE_INTERVAL_BV, ActivityInterval);
		FN_0(kIsFertile, TYPE_bool, IsFertile);
		FN_0(kIsNonExecutable, TYPE_bool, IsNonExecutable);
		FN_0(kSupportRand, TYPE_bool, SupportRand);
		FN_0(kGetRand, TYPE_INT, GetRand);
		VFN_1(kSetRand, SetRand, TYPE_INT);
		FN_0(kNewRand, TYPE_INT, NewRand);
		FN_0(kIsMaterialHolder, TYPE_bool, IsMaterialHolder);
		FN_0(kGetMaterial, TYPE_MTL, GetMaterial);
		FN_1(kSetMaterial, TYPE_bool, SetMaterial, TYPE_MTL);
		FN_0(kSupportScriptWiring, TYPE_bool, SupportScriptWiring);
		FN_0(kGetUseScriptWiring, TYPE_bool, GetUseScriptWiring);
		VFN_1(kSetUseScriptWiring, SetUseScriptWiring, TYPE_bool);

	END_FUNCTION_MAP
	#pragma warning(pop)
    	/** @defgroup IPFAction IPFAction.h
    	*  @{
    	*/

    	/*! \fn virtual bool	Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes) = 0;
    	*  \brief Returns true if the operation has been proceeded successfully. The method is called when the ActionList 
	   is established the first time, when there is a first particle that enters the ActionList and there is a need to proceed particles.
	/param pCont:	particle container with particle data
	/param pSystem: the particle system that generates the particle stream; 
					the same operator may get calls from different particle systems; if the 
					result varies according to the particle system that generates
					particles, then this parameter is useful.
	/param pNode:	INode of the particle system that generates the particles.
	/param actions: Tab<Object*> of Actions in the current ActionList.
					The stack will be used to proceed particles.
					The stack is mutable: an Action may change the stack (to remove
					or add some Actions) to fine-tune the order of execution or
					add some extra actions. The next parameter should sync with
					the mutation.
	/param actionNodes: Tab<INode*> of action nodes.
        */
	virtual bool	Init(IObject* pCont, Object* pSystem, INode* node, Tab<Object*>& actions, Tab<INode*>& actionNodes) = 0;

    	/*! \fn virtual bool	Release(IObject* pCont) = 0;
    	*  \brief See Init().
        */
	virtual bool	Release(IObject* pCont) = 0;

    	/*! \fn virtual const ParticleChannelMask& ChannelsUsed(const Interval& time) const = 0;
    	*  \brief See Init().
        */
	virtual const ParticleChannelMask& ChannelsUsed(const Interval& time) const = 0;

    	/*! \fn PFExport void	ChannelsUsed(const Interval& time, FPValue* channels) const;
    	*  \brief "Function publishing" alternative for ChannelsUsed method. Reserved for future use.
        */
	PFExport void	ChannelsUsed(const Interval& time, FPValue* channels) const;

    	/*! \fn virtual const Interval ActivityInterval() const = 0;
    	*  \brief Casting the FPValue->ptr parameter to Tab<Interface_ID>* as in PFExport void ChannelsUsed(...);
	an Action may have a time interval when the Action is active. Outside this interval
	the Action does nothing, therefore there is no need to proceed particles outside
	the activity interval.
        */
	virtual const Interval ActivityInterval() const = 0;

    	/*! \fn virtual bool IsFertile() const { return false; }
    	*  \brief Birth Action is a special type of actions since it can create particles from scratch.
	All other type of actions proceed existing particles. Only Action that can create
	absolutely new particles (if number of particles in the container is zero) should
	override this method returning true.
        */
	virtual bool IsFertile() const { return false; }

    	/*! \fn virtual bool IsNonExecutable() const { return false; }
    	*  \brief There is a special case of non-executable actions. They are present in the action
	schematics but don't affect the particle behavior. Example: Comments operator
        */
	virtual bool IsNonExecutable() const { return false; }

    	/*! \fn virtual bool Has3dIcon() const { return false; }
    	*  \brief An action may have 3d icon representation in viewport.
	The 3d icon can be used as a reference for some of the action parameters.
        */
	virtual bool Has3dIcon() const { return false; }

    	/*! \fn virtual int IsColorCoordinated() const { return -1; }
    	*  \brief If an operator/test has a 3D icon the icon can be automatically color matched
	to the color of the display operator in the same event. For the automatic
	color match the operator/test should return 1 in this method.
	If returns 0 then the action gets the default color for operator/test.
	If returns -1 then the action doesn't need any color coordination from Particle Flow.
        */
	virtual int IsColorCoordinated() const { return -1; }

    	/*! \fn virtual bool	SupportRand() const = 0;
    	*  \brief An Action may carry some chaos/unpredictibility in its processing. This 
	method supports chaos randomness.
        */
	virtual bool	SupportRand() const = 0;

    	/*! \fn virtual	int		GetRand() = 0;
    	*  \brief For chaos/unpredictibility action, gets rand value.
        */
	virtual	int		GetRand() = 0;

    	/*! \fn virtual void	SetRand(int seed) = 0;
    	*  \brief For chaos/unpredictibility action, sets rand value.
        */
	virtual void	SetRand(int seed) = 0;

    	/*! \fn PFExport int	NewRand();
    	*  \brief For chaos/unpredictibility action, sets and returns new rand value. 
        */
	PFExport int	NewRand(); 

    	/*! \fn PFExport static int NewRand(IParamBlock2* pblock, ParamID randomSeedParamID);
    	*  \brief Alternative for PFExport int NewRand(); but not in FnPub interface
        */
	PFExport static int NewRand(IParamBlock2* pblock, ParamID randomSeedParamID);

    	/*! \fn virtual bool	IsMaterialHolder() const	{ return false; }
    	*  \brief If the Action may potentically change particle material, then return true
        */
	virtual bool	IsMaterialHolder() const	{ return false; }

    	/*! \fn virtual Mtl*	GetMaterial()				{ return NULL; }
    	*  \brief Returns a material for particles
        */
	virtual Mtl*	GetMaterial()				{ return NULL; }
#pragma warning(push)
#pragma warning(disable:4100)
    	/*! \fn virtual bool	SetMaterial(Mtl* mtl)		{ return false; }
    	*  \brief Return true if material was set successfully
        */
	virtual bool	SetMaterial(Mtl* mtl)		{ return false; }

    	/*! \fn virtual bool SupportScriptWiring() const { return false; }
    	*  \brief An Action may have parameters controlled by script channels.
	How the script wiring is set up is controlled by a separate rollup
	with script-wiring pblock. The rollup is visible if "Use Script Wiring"
	option in the right-click menu is set to ON. The option is visible
	only for actions that support script wiring.
        */
	virtual bool SupportScriptWiring() const { return false; }

    	/*! \fn virtual bool GetUseScriptWiring() const { return false; }
    	*  \brief Supports script wiring. See SupportScriptWiring().
        */
	virtual bool GetUseScriptWiring() const { return false; }

    	/*! \fn virtual void SetUseScriptWiring(bool useScriptWiring) { ; }
    	*  \brief Supports script wiring. See SupportScriptWiring().
        */
	virtual void SetUseScriptWiring(bool useScriptWiring) { ; }

    	/*! \fn virtual bool IsEmitterTMDependent() const { return false; }
    	*  \brief An Action may or may not be dependent on the particle system emitter.
	When the emitter is invalidated, a particle group has to decide if it needs
	to invalidate itself. To make the decision, the group checks all the actions
	if any of them are dependent on the emitter change. There are two categories of 
	dependency: the transformation matrix of the emitter (TM), and object parameters
	of the emitter (props). This method checks the TM category.
        */
	virtual bool IsEmitterTMDependent() const { return false; }

    	/*! \fn virtual bool IsEmitterPropDependent() const { return false; }
    	*  \brief Checks the props category for dependency. See IsEmitterTMDependent().
      */
	virtual bool IsEmitterPropDependent() const { return false; }

    	/*! \fn virtual IObject* GetCurrentState(IObject* pContainer) { return NULL; }
    	*  \brief Returns action state (see IPFActionState.h for details).
	You have to delete the IObject by DeleteIObject method
	when the state is no longer needed.
        */
	virtual IObject* GetCurrentState(IObject* pContainer) { return NULL; }

    	/*! \fn virtual void SetCurrentState(IObject* actionState, IObject* pContainer) { ; }
    	*  \brief Sets the state of the action to the given state
      */
	virtual void SetCurrentState(IObject* actionState, IObject* pContainer) { ; }
#pragma warning(pop)
    	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTION_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTION_INTERFACE); }

};

// Codes for ClassDesc::Execute(...) method
// The Execute method is used to extract action description from the ClassDesc
enum {	kPF_GetActionDescription = 781,
// defines an action name to be shown in the action depot area in PView
// if the method is not supported then the class name of the action is used
		kPF_GetActionName,
// defines if the action is shown in ParticleView Action Depot window
// if the execute is not implemented then it is assumed that the action is
// public
		kPF_PViewPublic,
// defines a category for the action to be placed in the Action Depot
// the call is reserved for future use. If the execute is not implemented
// then the category is assigned according to the action type (operator or test)
		kPF_PViewCategory,
// defines if the action is "fertile". It means it is able to generate particles
// from nothing. If the execute is not implemented then it is assumed
// that the action is not "fertile".
		kPF_IsFertile,
// defines if the action is "non-executable". Non-executable action doesn't
// affect particle behavior. The action is present in the action schematics
// but its presence doesn't change how the particles are shown or rendered.
// If the execute is not implemented then it is assumed that the action is
// executable.
		kPF_IsNonExecutable,
// returns bitmap with icon for the depot window of ParticleView
// if the execute is not implemented then a default icon is used
		kPF_PViewDepotIcon 
	};

/*! \fn inline IPFAction* PFActionInterface(Object* obj) 
*  \brief 
*/
inline IPFAction* PFActionInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFActionInterface(obj));
};

/*! \fn inline IPFAction* PFActionInterface(INode* node)
*  \brief 
*/
inline IPFAction* PFActionInterface(INode* node) {
	return ((node == NULL) ? NULL : PFActionInterface(node->GetObjectRef()));
};

/*! \fn PFExport int IsPFActionActive(INode* actionNode);
*  \brief An action may be turned ON, OFF, True and False in Particle View.
An operator has two states: ON (1) and OFF (0).
A test has three states: ON (1), False (0) and True (-1).
The method returns the state of the action.
*/
PFExport int IsPFActionActive(INode* actionNode);

/*@}*/


