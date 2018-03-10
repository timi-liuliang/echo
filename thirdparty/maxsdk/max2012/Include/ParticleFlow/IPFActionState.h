/*! \file IPFActionState.h
    \brief Action-generic interface IPFActionState for action state.
				 An action may have history-dependent parameters that need
				 to be restored if the state of the whole particle system
				 needs to be saved/restored. For example, random generator
				 that is used by an action, is a history-dependent item.
				 An action may have several action states at the same time,
				 each of them reflecting a state for a specific particle
				 container.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-25-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\ifnpub.h"

// interface ID
#define PFACTIONSTATE_INTERFACE Interface_ID(0x23a31c4e, 0x6a2b2c68)

#define GetPFActionStateInterface(obj) (IPFActionState*)(obj->GetInterface(PFACTIONSTATE_INTERFACE))

class IPFActionState : public FPMixinInterface
{
public:
	// chunk IDs for saving and loading
	// developer: you may have your own chunk IDs
	// they are factored here for convenience
	enum {	kChunkActionHandle	= 4100,
			kChunkRandGen		= 4200,
			kChunkRandGen2		= 4210,
			kChunkRandGen3		= 4220,
			kChunkRandGen4		= 4230,
			kChunkRandGen5		= 4240,
			kChunkData			= 4300,
			kChunkData2			= 4310,
			kChunkData3			= 4320,
			kChunkData4			= 4330,
			kChunkData5			= 4340,
	};

public:

    /** @defgroup IPFActionState IPFActionState.h 
    *  @{
    */


    /*! \fn virtual Class_ID GetClassID() = 0;
    *  \brief The method returns the unique ID for the action state class.
	 The ID is used for constructing the action state when loading since
	 the action state class is not inherited from class Animatable.
    */
	virtual Class_ID GetClassID() = 0;

    /*! \fn virtual ULONG GetActionHandle() const = 0;
    *  \brief Returns node handle of the action with the action state
    */
	virtual ULONG GetActionHandle() const = 0;

    /*! \fn virtual void SetActionHandle(ULONG actionHandle) = 0;
    *  \brief Defines node handle of the action with the action state
    */
	virtual void SetActionHandle(ULONG actionHandle) = 0;

    /*! \fn virtual IOResult Save(ISave* isave) const = 0;
    *  \brief Saving content of the action state to file
    */
	virtual IOResult Save(ISave* isave) const = 0;

    /*! \fn virtual IOResult Load(ILoad* iload) = 0;
    *  \brief Loading content of the action state from file
    */
	virtual IOResult Load(ILoad* iload) = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONSTATE_INTERFACE); 
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PFACTIONSTATE_INTERFACE); 

/*@}*/
}
};



