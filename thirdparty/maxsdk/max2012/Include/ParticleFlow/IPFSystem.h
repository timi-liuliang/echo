/*! \file IPFSystem.h
    \brief Interface for Event-Driven Particle System. PF System has a system-specific parameters like a 
				 maximum number of particles in the system, integration
				 step size etc. Operators and Tests may request values
				 of those parameters through the interface.

*/
/**********************************************************************
 *<

	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-22-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "IPViewManager.h"
#include "..\object.h"

// interface ID
#define PFSYSTEM_INTERFACE Interface_ID(0x74f93d00, 0x1eb34700) 
#define GetPFSystemInterface(obj) ((IPFSystem*)((GetPFObject(obj))->GetInterface(PFSYSTEM_INTERFACE))) 


class IPFSystem : public FPMixinInterface
{
public:

	// function IDs
	enum {	kGetMultiplier,
			kGetBornAllowance,
			kHasEmitter,
			kGetEmitterType,
			kGetEmitterDimensions,
			kGetEmitterGeometry,
			kIsEmitterGeometryAnimated,
			kSetRenderState,
			kIsRenderState,
			kGetIntegrationStep,
			kGetUpdateType,
			kNumParticlesSelected,
			kGetSelectedParticleID,
			kIsParticleSelected,
			kNumActionListsSelected,
			kGetSelectedActionList,
			kIsActionListSelected,
			kIsRunningScript,
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetMultiplier, TYPE_FLOAT, GetMultiplier, TYPE_TIMEVALUE );
	FN_0(kGetBornAllowance, TYPE_INT, GetBornAllowance );
	FN_0(kHasEmitter, TYPE_bool, HasEmitter);
	FN_1(kGetEmitterType, TYPE_INT, GetEmitterType, TYPE_TIMEVALUE );
	VFN_2(kGetEmitterDimensions, GetEmitterDimensions, TYPE_TIMEVALUE, TYPE_FLOAT_TAB_BR );
	FN_1(kGetEmitterGeometry, TYPE_MESH, GetEmitterGeometry, TYPE_TIMEVALUE);
	FN_0(kIsEmitterGeometryAnimated, TYPE_bool, IsEmitterGeometryAnimated);
	VFN_1(kSetRenderState, SetRenderState, TYPE_bool);
	FN_0(kIsRenderState, TYPE_bool, IsRenderState);
	FN_0(kGetIntegrationStep, TYPE_TIMEVALUE, GetIntegrationStep);
	FN_0(kGetUpdateType, TYPE_INT, GetUpdateType);
	FN_0(kNumParticlesSelected, TYPE_INT, NumParticlesSelected);
	FN_1(kGetSelectedParticleID, TYPE_INDEX, GetSelectedParticleID, TYPE_INDEX);
	FN_1(kIsParticleSelected, TYPE_bool, IsParticleSelected, TYPE_INDEX);
	FN_0(kNumActionListsSelected, TYPE_INT, NumActionListsSelected);
	FN_1(kGetSelectedActionList, TYPE_INODE, GetSelectedActionList, TYPE_INDEX);
	FN_1(kIsActionListSelected, TYPE_bool, IsActionListSelected, TYPE_INODE);
	FN_0(kIsRunningScript, TYPE_bool, IsRunningScript);

	END_FUNCTION_MAP


    /** @defgroup IPFSystem IPFSystem.h
    *  @{
    */

    /*! \fn virtual float GetMultiplier(TimeValue time) = 0;
    *  \brief Returns multiplier coefficient to generate particles. Value 1.0f means that
	 an operator produces particles at 100% rate. Value 0.5f means that an operator
	 produces only half of particles that it supposed to be generating, i.e. if
	 a Birth operator is set to produce 1000 particles total, then under multiplier = 0.5f
	 the operator produces only 500 particles.
    */
	virtual float GetMultiplier(TimeValue time) = 0;

    /*! \fn virtual int GetBornAllowance() = 0;
    *  \brief Returns number of particles allowed to be generated at current time. Particle system has
	 an upper limit of a total number of particles in the system. At each moment of time
	 particle system is able to calculate the current number of particle in the system and 
	 how many particles are allowed to be generated to not exceed the limit threshold. Birth and
	 Spawn Operators use this method to find out how many particles they are allowed to generate.

    */
	virtual int GetBornAllowance() = 0;

    /*! \fn virtual bool HasEmitter() { return false; }
    *  \brief PF System has an icon that can be used as an emitter for particles. For Operators to use
	 the icon as an emitter the following five methods should be supported. If PF System doesn't
	 support emitter icon then it returns "false" in the first method. The default implementation 
	 of the following five methods is if PF System doesn't support icon emission.
    */
	virtual bool HasEmitter() { return false; }
	// emitter types
	enum {	kEmitterType_none = -1,
			kEmitterType_rectangle = 0,
			kEmitterType_circle,
			kEmitterType_box,
			kEmitterType_sphere,
			kEmitterType_mesh
	};
#pragma warning(push)
#pragma warning(disable:4100)
    /*! \fn virtual int GetEmitterType(TimeValue time) { return kEmitterType_none; }
    *  \brief
	*/
	virtual int GetEmitterType(TimeValue time) { return kEmitterType_none; }

    /*! \fn virtual void GetEmitterDimensions(TimeValue time, Tab<float>& dimensions) { }
    *  \brief Returns spacial dimensions of the emitter icon. It may have less or more than three
	 as emitter type requires. For example, sphere emitter type has only one dimension.
    */
	virtual void GetEmitterDimensions(TimeValue time, Tab<float>& dimensions) { }

    /*! \fn virtual Mesh* GetEmitterGeometry(TimeValue time) { return NULL; }
    *  \brief Returns emitter geometry in form of mesh.
    */
		virtual Mesh* GetEmitterGeometry(TimeValue time) { return NULL; }

    /*! \fn virtual bool IsEmitterGeometryAnimated() { return false; }
    *  \brief If the emitter mesh is geometry-animated, return "true" in the following method.
    */
		virtual bool IsEmitterGeometryAnimated() { return false; }

    /*! \fn virtual void SetRenderState(bool renderState=true) = 0;
    *  \brief PF System has two states: render and viewport. At any given moment PF System
	 is exclusively in one state or the other. PF System may have different parameters
	 during render and viewport (i.e. Multiplier value, or emitter geometry etc.)
	 therefore it's important to know what state the PF System is in. It's also
	 possible to alterate the state of PF System. PF System alterates its state
	 automatically when the rendering starts and when the rendering ends.
    */
	virtual void SetRenderState(bool renderState=true) = 0;

    /*! \fn virtual bool IsRenderState() const = 0;
    *  \brief Works with SetRenderState.
    */
	virtual bool IsRenderState() const = 0;

    /*! \fn virtual TimeValue GetIntegrationStep() const = 0;
    *  \brief PF System has an integration step parameter that defines granularity in proceeding
	 particles in time. An PF System may have different integration steps for viewports
	 and for render. 
    */
	virtual TimeValue GetIntegrationStep() const = 0;

    /*! \fn virtual int GetUpdateType() const { return GetPFUpdateType(); } // uses global UpdateType method from IPViewManager
    *  \brief PF system has different modes for update. 
	 Forward mode: particles aren't updated right away. At playback time the particle history is
	 not recalculated. Only new events that will happen with the particles have new settings
	 Complete mode: the whole particle animation is recalculated. 
    */
	enum { kPFUpdateType_complete, kPFUpdateType_forward };
	virtual int GetUpdateType() const { return GetPFUpdateType(); } // uses global UpdateType method from IPViewManager

    /*! \fn virtual int NumParticlesSelected() const { return 0; }
    *  \brief PF system may select sub-components. There are two types of sub-components: particles and
	 action lists. Returns amount of selected particles.
    */
	virtual int NumParticlesSelected() const { return 0; }

    /*! \fn virtual int GetSelectedParticleID(int i) const { return 0; }
    *  \brief Returns born ID of the i-th selected particle
    */
	virtual int GetSelectedParticleID(int i) const { return 0; }

    /*! \fn virtual bool IsParticleSelected(int id) const { return false; }
    *  \brief Verifies if a particle with the given born ID is selected or not
    */
	virtual bool IsParticleSelected(int id) const { return false; }

    /*! \fn virtual int NumActionListsSelected() const { return 0; }
    *  \brief Returns amount of selected action lists
    */
	virtual int NumActionListsSelected() const { return 0; }

    /*! \fn virtual INode* GetSelectedActionList(int i) const { return NULL; }
    *  \brief Returns node of the i-th selected action list
    */
	virtual INode* GetSelectedActionList(int i) const { return NULL; }

    /*! \fn virtual bool IsActionListSelected(INode* inode) const { return false; }
    *  \brief Verifies if an action list is selected or not
    */
	virtual bool IsActionListSelected(INode* inode) const { return false; }
#pragma warning(pop)
    /*! \fn virtual bool IsRunningScript() const { return false; }
    *  \brief Returns true if the particle system is in the process of runnint Every Step or Final Step scripts
    */
	virtual bool IsRunningScript() const { return false; }

    /*! \fn virtual bool IsCopyClone() const { return true; }
    *  \brief For internal use.
    */
	virtual bool IsCopyClone() const { return true; }

    /*! \fn virtual void ResetCopyClone() { ; }
    *  \brief Works with IsCopyClone.
    */
	virtual void ResetCopyClone() { ; }

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFSYSTEM_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFSYSTEM_INTERFACE); }

    /*@}*/

};

inline IPFSystem* PFSystemInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFSystemInterface(obj));
};

inline IPFSystem* PFSystemInterface(INode* node) {
	return ((node == NULL) ? NULL : PFSystemInterface(node->GetObjectRef()));
};

