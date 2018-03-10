/*! \file IPViewManager.h
    \brief Interface for ParticleView manager and an access method
				 to get the PViewManager from the scene.
				 There is a single PViewManager in the scene that
				 gives us access to all Particle Views
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 01-11-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
// forward declarations
class ClassEntry;
class IParamBlock2;

// interface ID
#define PVIEWMANAGER_INTERFACE Interface_ID(0x74f93d07, 0x1eb34500) 

#define GetPViewManagerInterface(obj) ((IPViewManager*)obj->GetInterface(PVIEWMANAGER_INTERFACE)) 

class IPViewManager : public FPMixinInterface
{
public:

	// function IDs
	enum {	kOpenParticleView,
			kUpdateParticleViews,
			kPreUpdateParticleViews,
			kPostUpdateParticleViews,
			kUpdateOnHold,
			kNumActionClasses,
			kActionClass,
			kActionName,
			kNumPViewItemClasses,
			kPViewItemClass,
			kPViewItemName,
			kParticleViewOpenCloseToggle,
			kParticleFlowActivationToggle
	}; 

	BEGIN_FUNCTION_MAP
	FN_1(kOpenParticleView, TYPE_bool, OpenParticleView, TYPE_OBJECT);
	FN_1(kUpdateParticleViews, TYPE_bool, UpdateParticleViews, TYPE_bool);
	FN_1(kPreUpdateParticleViews, TYPE_bool, PreUpdateParticleViews, TYPE_bool);
	FN_1(kPostUpdateParticleViews, TYPE_bool, PostUpdateParticleViews, TYPE_bool);
	FN_0(kUpdateOnHold, TYPE_bool, UpdateOnHold);
	FN_0(kNumActionClasses, TYPE_INT, NumActionClasses );
	FN_1(kActionClass, TYPE_CLASS, ActionClass, TYPE_INT);
	FN_1(kActionName, TYPE_STRING, ActionName, TYPE_INT);
	FN_0(kNumPViewItemClasses, TYPE_INT, NumPViewItemClasses );
	FN_1(kPViewItemClass, TYPE_CLASS, PViewItemClass, TYPE_INT);
	FN_1(kPViewItemName, TYPE_STRING, PViewItemName, TYPE_INT);
	FN_0(kParticleViewOpenCloseToggle, TYPE_bool, ParticleViewOpenCloseToggle);
	FN_1(kParticleFlowActivationToggle, TYPE_bool, ParticleFlowActivationToggle, TYPE_bool);
	END_FUNCTION_MAP

    /** @defgroup IPViewManager IPViewManager.h
    *  @{
    */

    /*! \fn virtual bool	OpenParticleView(Object* engine) = 0;
    *  \brief Opens ParticleView for a given PFEngine.
	 If there are several ParticleViews with the given PFEngine,
	 a dialog pops up to make a choice.
	 \param engine: PFEngine to open particle view for
    */
	virtual bool	OpenParticleView(Object* engine) = 0;

    /*! \fn virtual bool	UpdateParticleViews(bool additionsOnly=false) = 0;
    *  \brief If nothing was delete from PView then additionsOnly is set to true
    */
	virtual bool	UpdateParticleViews(bool additionsOnly=false) = 0;

    /*! \fn virtual bool	PreUpdateParticleViews(bool additionsOnly=false) = 0;
    *  \brief If procedures between Pre.. and Post.. involve additions of new items only then
	 set "additionsOnly" to "true". The rollup panel won't be updated.
    */
	virtual bool	PreUpdateParticleViews(bool additionsOnly=false) = 0;

    /*! \fn virtual bool	PostUpdateParticleViews(bool additionsOnly=false) = 0;
    *  \brief If procedures between Pre.. and Post.. involve additions of new items only then
	 set "additionsOnly" to "true". The rollup panel won't be updated.
    */
	virtual bool	PostUpdateParticleViews(bool additionsOnly=false) = 0;

    /*! \fn virtual bool	CancelUpdateParticleViews() = 0;
    *  \brief Not supported for maxscript
    */
	virtual bool	CancelUpdateParticleViews() = 0;

    /*! \fn virtual bool	UpdateOnHold() = 0; 
    *  \brief Return true when in between PreUpdate and PostUpdate
    */
	virtual bool	UpdateOnHold() = 0;

    /*! \fn virtual void	UpdateParticleAmountShown() { ; }
    *  \brief Updates particle amount shown in PViews
    */
	virtual void	UpdateParticleAmountShown() { ; }

    /*! \fn virtual void	ActionProceedBegin(INode* actionNode) { ; }
    *  \brief Shows that an action starts the proceed
    */
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual void	ActionProceedBegin(INode* actionNode) { ; }

    /*! \fn virtual void	ActionProceedEnd(INode* actionNode) { ; }
    *  \brief Shows that an action ends the proceed
    */
	virtual void	ActionProceedEnd(INode* actionNode) { ; } 
	#pragma warning(pop)
    /*! \fn virtual bool	IsParticleViewInForeground() { return false; }
    *  \brief Check if any ParticleView is a foreground window
    */
	virtual bool	IsParticleViewInForeground() { return false; }

    /*! \fn PFExport static int NumActionClasses(); 
    *  \brief Public classes only.
    */
	PFExport static int NumActionClasses(); // public classes only

    /*! \fn PFExport static ClassDesc* ActionClass(int index);
    *  \brief 
    */
	PFExport static ClassDesc* ActionClass(int index);

    /*! \fn PFExport static MCHAR* ActionName(int index);
    *  \brief 
    */
	PFExport static MCHAR* ActionName(int index);

    /*! \fn PFExport static int NumPViewItemClasses();
    *  \brief Public classes only.
    */
	PFExport static int NumPViewItemClasses(); 

    /*! \fn PFExport static ClassDesc* PViewItemClass(int index);
    *  \brief 
    */
	PFExport static ClassDesc* PViewItemClass(int index);

    /*! \fn PFExport static MCHAR* PViewItemName(int index);
    *  \brief 
    */
	PFExport static MCHAR* PViewItemName(int index);

    /*! \fn virtual bool ParticleViewOpenCloseToggle() = 0;
    *  \brief Used for CUI shortcuts.
    */
	virtual bool ParticleViewOpenCloseToggle() = 0;

    /*! \fn virtual bool ParticleFlowActivationToggle(bool selectedOnly=false) = 0;
    *  \brief Used for CUI shortcuts.
    */
	virtual bool ParticleFlowActivationToggle(bool selectedOnly=false) = 0;

    /*! \fn virtual int GetVacantSpace() = 0;
    *  \brief Returns X coordinate of half-plane of space free from any PF item.
	 The method is usually used to calculate (x,y) location of newly created PF systems.
    */
	virtual int GetVacantSpace() = 0;

    /*! \fn virtual void KeepMaterial(Mtl* mtl) = 0;
    *  \brief The method is used to prevent automatic removal of non-used materials from the scene.
	The method is used by Shape Instance operator. The operator creates combined materials on-the-fly
	to be assigned to particles. When the reference object for the operator is changed, the material
	is changed. The old material could be deleted because it doesn't have any more references in the
	scene. This may create a problem for undo/redo operations. To prevent that this method creates 
	a reference dependency between PViewManager and the Material. Since PViewManager is not
	saved with the scene, the materials won't be saved either.
    */
	virtual void KeepMaterial(Mtl* mtl) = 0;

	/*! \fn PFExport static void RegisterModule(Class_ID moduleID);
	*  \brief The method is used by a PFlow module to register itself with the system.
	This way other PFlow modules can query whether a specific module is present/loaded
	with the system and act accordingly. Once a module is registered, it cannot be 
	unregistered.
	*/
	PFExport static void RegisterModule(Class_ID moduleID);

	/*! \fn PFExport static bool IsModuleRegistered(Class_ID moduleID);
	*  \brief Returns true if a PFlow module was registered.
	*/
	PFExport static bool IsModuleRegistered(Class_ID moduleID);

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PVIEWMANAGER_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PVIEWMANAGER_INTERFACE); }

    /*! \fn friend PFExport IPViewManager* GetPViewManager();
	friend PFExport void ReleasePViewManager();
    *  \brief Friend function declarations
    */
    friend PFExport IPViewManager* GetPViewManager();
	friend PFExport void ReleasePViewManager();
    
/*@}*/

protected:
	PFExport IPViewManager();
	PFExport virtual ~IPViewManager();

	virtual void releaseManager() = 0;

	static IPViewManager* m_manager; // the one and only PViewManager in the scene
	static Tab<ClassEntry*> m_actionClasses;
	static Tab<ClassEntry*> m_pViewItemClasses;
	static Tab<Class_ID> m_registeredModules;
};

#define PFTOOLSBOX1_MODULE_CLASSID			Class_ID(0x13410824, 0x88cd3e26)
#define PFTOOLSBOX1CHANNELS_MODULE_CLASSID	Class_ID(0x00358976, 0xf7f26da4)


/** @defgroup IPViewManager IPViewManager.h
*  @{
*/

/*! \fn PFExport IPViewManager* GetPViewManager();
*  \brief Gets PViewManager from the scene
*/
PFExport IPViewManager* GetPViewManager();

/*! \fn PFExport   INode *CreatePFNode( Object *obj);
*  \brief Creates an INode used by ParticleFlow objects (actions, engines etc.)
   The ParticleFlow INode has some properties that are different from a regular INode.
*/
PFExport   INode *CreatePFNode( Object *obj);

/*! \fn PFExport   void DeletePFNode(INode* node);
*  \brief Deletes a PF-type INode. The ParticleFlow INode has some properties that are
   different from a regular INode. Also, deleting a node by the methods cleans up
   the scene from associated elements, for example deleting a PF emitter also 
   removes all global actions and exclusive actionLists downstream.
   There won't be any harm done if a regular INode is deleted by the method.
   The plug-in is able to classify the given node to choose the appropriate way of action.
*/
PFExport   void DeletePFNode(INode* node);

/*! \fn PFExport bool AppendPFAction(INode* actionList, INode* action);
*  \brief There is a special relationship between an action list and an action.
   The following four methods ensure the consistency in this relationship.
   Since PF particle system has a list of global action, the system can also
   be considered as an action list.
*/
PFExport bool AppendPFAction(INode* actionList, INode* action);

/*! \fn PFExport bool InsertPFAction(INode* actionList, INode* action, int indexAt);
*  \brief Ensures consistency in relationship between action list and action. See AppendPFAction();
*/
PFExport bool InsertPFAction(INode* actionList, INode* action, int indexAt);

/*! \fn PFExport bool RemovePFAction(INode* actionList, int index);
*  \brief Ensures consistency in relationship between action list and action. See AppendPFAction();
*/
PFExport bool RemovePFAction(INode* actionList, int index);

/*! \fn PFExport bool RemovePFAction(INode* actionList, INode* action);
*  \brief Ensures consistency in relationship between action list and action. See AppendPFAction();
*/
PFExport bool RemovePFAction(INode* actionList, INode* action);

/*! \fn PFExport int GetPViewVacantSpace(INode* excludeNode=NULL);
*  \brief Returns X coordinate of half-plane of space free from any PF item.
   The method is usually used to calculate (x,y) location of newly created PF systems.
   If given node is not NULL, then the node is excluded from consideration while looking
   for free PView space.
*/
PFExport int GetPViewVacantSpace(INode* excludeNode=NULL);

/*! \fn PFExport int GetPViewMaximumZOrderIndex(INode* excludeNode=NULL);
*  \brief All items in PView are shown according to their Z order index.
   Returns the highest Z order index amongst all particle systems and action lists.
*/
PFExport int GetPViewMaximumZOrderIndex(INode* excludeNode=NULL);

/*! \fn PFExport bool CompactPViewZOrder();
*  \brief The Z order sequence may have some "holes". The method makes the sequence to
   be more compact by eliminating these "holes".
   Returns true if there were "holes"; and false if it is already compact.
*/
PFExport bool CompactPViewZOrder();

/*! \fn PFExport void RegisterParticleFlowNotification();
*  \brief ParticleFlow has to track creation of new PF items.
   For ParticleFlow Shell: the system creates a default configuration and sets PView location.
   For ActionList: the system sets PView location.
   For Action: the system creates an ActionList that includes the Action, and sets PView location.
   To avoid this extra effort from the system side, use Suspend/Resume methods.
*/
PFExport void RegisterParticleFlowNotification();

/*! \fn PFExport void SuspendParticleFlowNotification();
*  \brief Avoids extra effort for tracking. See RegisterParticleFlowNotification();
*/
PFExport void SuspendParticleFlowNotification();

/*! \fn PFExport void ResumeParticleFlowNotification();
*  \brief Avoids extra effort for tracking. See RegisterParticleFlowNotification();
*/
PFExport void ResumeParticleFlowNotification();

/*! \fn PFExport bool IsParticleFlowNotificationSuspended();
*  \brief Avoids extra effort for tracking. See RegisterParticleFlowNotification();
*/
PFExport bool IsParticleFlowNotificationSuspended();

/*! \fn PFExport void ReleasePViewManager();
*  \brief The method is used to release PViewManager when a scene is closed.
   For internal use only.
*/
PFExport void ReleasePViewManager();

/*! \fn PFExport int GetPFInt(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);
*  \brief GetPFInt, GetPFFloat and GetPFTimeValue reinforce value boundaries set for parameter values.
   Can be used instead of pblock()->GetInt and pblock()->GetFloat to be
   sure that the result values are stay inside the parameter boundaries no matter what.
*/
PFExport int GetPFInt(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);

/*! \fn PFExport float GetPFFloat(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);
*  \brief See GetPFInt();
*/
PFExport float GetPFFloat(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);

/*! \fn PFExport TimeValue GetPFTimeValue(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);
*  \brief See GetPFInt();
*/
PFExport TimeValue GetPFTimeValue(IParamBlock2* pblock, ParamID id, TimeValue t=0, int tabIndex=0);

/*! \fn PFExport bool ValidateParticleFlows();
*  \brief For internal use. The method is used to make all ParticleFlow elements valid.
   The method traverses the scene to collect all ParticleFlow elements
   then it removes the invalid ones and fixes elements that are fixable.
   Returns true is the scene was valid, and false if the scene was invalid.
*/
PFExport bool ValidateParticleFlows();

enum { kPFDefaultDisplay_global, kPFDefaultDisplay_local };

/*! \fn PFExport int GetPFDefaultDisplay();
*  \brief 
*/
PFExport int GetPFDefaultDisplay();

/*! \fn PFExport void SetPFDefaultDisplay(int type);
*  \brief 
*/
PFExport void SetPFDefaultDisplay(int type);

/*! \fn PFExport void InitPFDefaultDisplay(int type);
*  \brief 
*/
PFExport void InitPFDefaultDisplay(int type);

enum { kPFActionOrder_globalsFirst, kPFActionOrder_localsFirst };

/*! \fn PFExport int GetPFActionOrder();
*  \brief 
*/
PFExport int GetPFActionOrder();

/*! \fn PFExport void SetPFActionOrder(int order);
*  \brief 
*/
PFExport void SetPFActionOrder(int order);

/*! \fn PFExport void InitPFActionOrder(int order);
*  \brief 
*/
PFExport void InitPFActionOrder(int order);

// PF particle system has different modes for update. 
// Forward mode: particles aren't updated right away. At playback time the particle history is
// not recalculated. Only new events that will happen with the particles have new settings
// Complete mode: the whole particle animation is recalculated. 
enum { kPFUpdateType_complete, kPFUpdateType_forward };

/*! \fn PFExport int GetPFUpdateType();
*  \brief 
*/
PFExport int GetPFUpdateType();

/*! \fn PFExport void SetPFUpdateType(int type);
*  \brief 
*/
PFExport void SetPFUpdateType(int type);

/*! \fn PFExport void InitPFUpdateType(int type);
*  \brief 
*/
PFExport void InitPFUpdateType(int type);

// names of actions in PView may have a dynamic suffix to reflect the most important settings of the action
enum { kPFDynamicNames_no, kPFDynamicNames_yes };

/*! \fn PFExport int GetPFDynamicNames();
*  \brief Names of actions in PView may have a dynamic suffix to reflect the most important settings of the action.
*/
PFExport int GetPFDynamicNames();

/*! \fn PFExport void SetPFDynamicNames(int type);
*  \brief Names of actions in PView may have a dynamic suffix to reflect the most important settings of the action.
*/
PFExport void SetPFDynamicNames(int type);

/*! \fn PFExport void InitPFDynamicNames(int type);
*  \brief Names of actions in PView may have a dynamic suffix to reflect the most important settings of the action.
*/
PFExport void InitPFDynamicNames(int type);

/*! \fn PFExport void DisableParticleInvalidation();
*  \brief Particle system and operators/tests may have reference object (i.e. define an emitting area).
   An operator may require TM of a reference object at different moments. If the reference object
   has a complex controller, the action of requesting the state of the reference object, if the time
   is different from the current time, may inflict REFMSG_CHANGE message thus provoking invalidation
   of the whole particle system. To prevent that, at the moment of acquiring the reference object TMs, 
   the operator should disable particle invalidation, and then enable the invalidation when it is done
   with the reference object.
*/
PFExport void DisableParticleInvalidation();

/*! \fn PFExport void EnableParticleInvalidation();
*  \brief See DisableParticleInvalidation();
*/
PFExport void EnableParticleInvalidation();

/*! \fn PFExport bool IsAbleInvalidateParticles();
*  \brief See DisableParticleInvalidation();
*/
PFExport bool IsAbleInvalidateParticles();

/*! \fn PFExport void SetIgnoreEmitterTMChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void SetIgnoreEmitterTMChange();

/*! \fn PFExport void ClearIgnoreEmitterTMChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void ClearIgnoreEmitterTMChange();

/*! \fn PFExport bool IsIgnoringEmitterTMChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport bool IsIgnoringEmitterTMChange();

/*! \fn PFExport void SetIgnoreEmitterPropChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void SetIgnoreEmitterPropChange();

/*! \fn PFExport void ClearIgnoreEmitterPropChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void ClearIgnoreEmitterPropChange();

/*! \fn PFExport bool IsIgnoringEmitterPropChange();
*  \brief For more selective tuning in notification change
*/
PFExport bool IsIgnoringEmitterPropChange();

/*! \fn PFExport void SetIgnoreRefNodeChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void SetIgnoreRefNodeChange();

/*! \fn PFExport void ClearIgnoreRefNodeChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport void ClearIgnoreRefNodeChange();

/*! \fn PFExport bool IsIgnoringRefNodeChange();
*  \brief See DisableParticleInvalidation();
*/
PFExport bool IsIgnoringRefNodeChange();

/*! \fn PFExport void SetPFProceedStatus();
*  \brief When a particle system is in the process of calculation/evaluation, it is not advisable to invalidate
   particle containers. This method sets the proceed status.
*/
PFExport void SetPFProceedStatus();

/*! \fn PFExport void ClearPFProceedStatus();
*  \brief When a particle system is in the process of calculation/evaluation, it is not advisable to invalidate
   particle containers. This method clears the proceed status.
*/
PFExport void ClearPFProceedStatus();

/*! \fn PFExport bool IsPFProceeding();
*  \brief When a particle system is in the process of calculation/evaluation, it is not advisable to invalidate
   particle containers. This method checks if PF is proceeding.
*/
PFExport bool IsPFProceeding(); 

/*! \fn PFExport Object* GetPFObject(Object* obj);
*  \brief ParticleFlow relays on presence of specific interfaces in PF objects. Sometimes the objects are
   hidden behind layers of XRef objects and WSM modifiers. To reach the real PF object underneath,
   use this method.
*/
PFExport Object* GetPFObject(Object* obj);

// constants for particle view spacement and location
enum { kPViewActionListHorSpace=20 };

/*@}*/

