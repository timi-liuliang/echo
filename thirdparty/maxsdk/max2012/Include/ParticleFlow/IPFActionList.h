/*! \file IPFActionList.h
    \brief Interface for PF ActionList
				 An ActionList is a list of actions combined together
				 to control properties of group of particles
				 PF System also implements this interface to keep 
				 track of all global Actions for the particle system.
				Part of public SDK
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-15-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "IPFAction.h"

PFExport Object* GetPFObject(Object* obj);
// interface ID
#define PFACTIONLIST_INTERFACE Interface_ID(0x74f93d04, 0x1eb34500) 
#define GetPFActionListInterface(obj) ((IPFActionList*)((GetPFObject(obj))->GetInterface(PFACTIONLIST_INTERFACE))) 

class IPFActionList : public FPMixinInterface
{
public:
	// function IDs
	enum {	kGetNumActions,		kGetAction,			kAppendAction,
			kInsertAction,		kRemoveAction,		kHasAction,
			kIsAction,			kIsOperator,		kIsTest,
			kIsActivated,		kActivate,			kIsActionActive,	
			kActivateAction,	kGetPViewLocation,	kSetPViewLocation,
			kGetPViewZOrder,	kSetPViewZOrder,	kGetListWidth,		
			kSetListWidth,		kGetPViewRightBoundary,
			kIsCollapsed,		kCollapse,			kExpand,
			kHasUpStream,		kGetSelected,		kSetSelected,		
			kAcceptFertile
	}; 

	// Function Map for Function Publish System 
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_0(	kGetNumActions,		TYPE_INT,	NumActions	);
	FN_1(	kGetAction,			TYPE_INODE, GetAction,		TYPE_INDEX);
	FN_1(	kAppendAction,		TYPE_bool,	AppendAction,	TYPE_INODE);
	FN_2(	kInsertAction,		TYPE_bool,	InsertAction,	TYPE_INODE, TYPE_INDEX);
	FN_1(	kRemoveAction,		TYPE_bool,	RemoveAction,	TYPE_INDEX);
	FN_2(	kHasAction,			TYPE_bool,	HasAction,		TYPE_INODE, TYPE_INDEX_BR);
	FN_1(	kIsAction,			TYPE_bool,	IsAction,		TYPE_INODE);
	FN_1(	kIsOperator,		TYPE_bool,	IsOperator,		TYPE_INODE);
	FN_1(	kIsTest,			TYPE_bool,	IsTest,			TYPE_INODE);
	FN_0(	kIsActivated,		TYPE_bool,	IsActivated		);
	VFN_1(	kActivate,						Activate,		TYPE_bool);
	FN_1(	kIsActionActive,	TYPE_INT,	IsActionActive, TYPE_INDEX);
	VFN_2(	kActivateAction,				ActivateAction, TYPE_INDEX, TYPE_INT);
	VFN_2(	kGetPViewLocation,				GetPViewLocation, TYPE_INT_BR, TYPE_INT_BR);
	VFN_2(	kSetPViewLocation,				SetPViewLocation, TYPE_INT, TYPE_INT);
	FN_0(	kGetPViewZOrder,	TYPE_INT,	GetPViewZOrder );
	VFN_1(	kSetPViewZOrder,				SetPViewZOrder,	TYPE_INT);
	FN_0(	kGetListWidth,		TYPE_INT,	GetListWidth );
	VFN_1(	kSetListWidth,					SetListWidth,	TYPE_INT);
	FN_0(	kGetPViewRightBoundary, TYPE_INT, GetPViewRightBoundary );
	FN_0(	kIsCollapsed,		TYPE_bool,	IsCollapsed );
	VFN_0(	kCollapse,						Collapse );
	VFN_0(	kExpand,						Expand );
	FN_0(	kHasUpStream,		TYPE_bool,	HasUpStream );
	FN_0(	kGetSelected,		TYPE_INT,	GetSelected );
	VFN_1(	kSetSelected,					SetSelected, TYPE_INT);
	FN_0(	kAcceptFertile,		TYPE_bool,	AcceptFertile );

	END_FUNCTION_MAP

      /** @defgroup IPFActionList IPFActionList.h
      *  @{
      */

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONLIST_INTERFACE); }
      *  \brief Must implement GetDescByID method 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONLIST_INTERFACE); }
	//End of Function Publishing system code 
	//***********************************

      /*! \fn virtual int		NumActions() const = 0;
      *  \brief 
      */
	virtual int		NumActions() const = 0;

      /*! \fn virtual INode*	GetAction(int index) const = 0;
      *  \brief 
      */
	virtual INode*	GetAction(int index) const = 0;

      /*! \fn virtual bool	AppendAction(INode* action) = 0;
      *  \brief 
      */
	virtual bool	AppendAction(INode* action) = 0;

      /*! \fn virtual bool	InsertAction(INode* action, int indexAt) = 0;
      *  \brief 
      */
	virtual bool	InsertAction(INode* action, int indexAt) = 0;

      /*! \fn virtual bool	RemoveAction(int index) = 0;
      *  \brief 
      */
	virtual bool	RemoveAction(int index) = 0;

      /*! \fn virtual bool	HasAction(INode* action, int& index) const = 0;
      *  \brief 
      */
	virtual bool	HasAction(INode* action, int& index) const = 0;

      /*! \fn PFExport static bool IsAction(INode* action);
      *  \brief 
      */
	PFExport static bool IsAction(INode* action);

      /*! \fn PFExport static bool IsOperator(INode* action);
      *  \brief 
      */
	PFExport static bool IsOperator(INode* action);

      /*! \fn PFExport static bool IsTest(INode* action);
      *  \brief 
      */
	PFExport static bool IsTest(INode* action);

      /*! \fn virtual bool	IsActivated() const = 0;
      *  \brief 
      */
	virtual bool	IsActivated() const = 0;

      /*! \fn virtual void	Activate(bool active) = 0;
      *  \brief Activate/deactivate all the actions in the list
      */
	virtual void	Activate(bool active) = 0;

      /*! \fn virtual int		IsActionActive(int index) const = 0;
      *  \brief Activate/deactivate all the actions in the list. 1: active; 0: non-active for operators and "always false" for tests; 
	-1: "always true" for tests
      */
	virtual int		IsActionActive(int index) const = 0;

      /*! \fn virtual void	ActivateAction(int index, int active=1) = 0;
      *  \brief Activate/deactivate all the actions in the list
      */
	virtual void	ActivateAction(int index, int active=1) = 0;

	// default spacing and location on Particle View
	enum {	kListWidth=180 }; // default actionList width

      /*! \fn virtual void	GetPViewLocation(int& x, int& y) = 0;
      *  \brief 
      */
	virtual void	GetPViewLocation(int& x, int& y) = 0;
#pragma warning(push)
#pragma warning(disable:4100)
      /*! \fn virtual void	SetPViewLocation(int x, int y) { ; }
      *  \brief 
      */
	virtual void	SetPViewLocation(int x, int y) { ; }

      /*! \fn virtual int		GetPViewZOrder() { return 0; }
      *  \brief 
      */
	virtual int		GetPViewZOrder() { return 0; }

      /*! \fn virtual void	SetPViewZOrder(int z) { ; }
      *  \brief 
      */
	virtual void	SetPViewZOrder(int z) { ; }

      /*! \fn virtual int		GetListWidth() { return kListWidth; }
      *  \brief 
      */
	virtual int		GetListWidth() { return kListWidth; }

      /*! \fn virtual void	SetListWidth(int w) { ; }
      *  \brief 
      */
	virtual void	SetListWidth(int w) { ; }

      /*! \fn PFExport int	GetPViewRightBoundary();
      *  \brief Returns x-coord of the right side of the action list
      */
	PFExport int	GetPViewRightBoundary();

      /*! \fn virtual bool	IsCollapsed() { return false; }
      *  \brief Action list may be shown in a "collapsed" state in ParticleView
      */
	virtual bool	IsCollapsed() { return false; }

      /*! \fn virtual void	Collapse() { ; }
      *  \brief Action list may be shown in a "collapsed" state in ParticleView
      */
	virtual void	Collapse() { ; }

      /*! \fn virtual void	Expand() { ; }
      *  \brief Action list may be shown in a "collapsed" state in ParticleView
      */
	virtual void	Expand() { ; }

      /*! \fn virtual	bool	HasUpStream() { return false; }
      *  \brief Indicates if there are PFSystems or ActionLists that direct particle flow
	   in this action list. If returns false then the action list won't produce particles.
      */
	virtual	bool	HasUpStream() { return false; }

      /*! \fn virtual int		GetSelected() const { return 0; }
      *  \brief ActionList has its own methods for selecting. Selected ActionList has a white boundary
	   in the ParticleView. If the corresponding particle system has Event-Based Selection ON,
	   the the corresponding particles are shown as selected.
      */
	virtual int		GetSelected() const { return 0; }

      /*! \fn virtual void	SetSelected(int onOff) { ; }
      *  \brief See virtual int GetSelected().
      */
	virtual void	SetSelected(int onOff) { ; }
#pragma warning(pop)
      /*! \fn virtual bool	AcceptFertile() { return false; }
      *  \brief ActionList has some restrictions on whether it is able to add a "fertile" action.
	   ActionList may have only single "fertile" action in the list of actions.
	   Also, ActionList may have a "fertile" action only if no test uses this
	   actionList as a next event.
      */
	virtual bool	AcceptFertile() { return false; }

      /*! \fn virtual void	UpdateMaterial() { ; }
      *  \brief The method is used to initiate material update in the associated particle group.
      */
	virtual void	UpdateMaterial() { ; }
};

/*! \fn PFExport IPFActionList* GetParentPFActionList(INode* actionNode);
*  \brief Returns interface of the parent actionList of the given action
*/
PFExport IPFActionList* GetParentPFActionList(INode* actionNode);

inline IPFActionList* PFActionListInterface(Object* obj) {
	return ((obj == NULL) ? NULL : GetPFActionListInterface(obj));
};

inline IPFActionList* PFActionListInterface(INode* node) {
	return ((node == NULL) ? NULL : PFActionListInterface(node->GetObjectRef()));
};


