/*! \file IPViewItemCreator.h
    \brief Interface for PViewItemCreator. A ClassDesc2 should implement
				 the interface to be able to create PViewItems in the Pview.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-11-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// interface ID
#define PVIEWITEMCREATOR_INTERFACE Interface_ID(0x74f93d0b, 0x1eb34500)

#define GetPViewItemCreatorInterface(obj) (IPViewItemCreator*)(obj->GetInterface(PVIEWITEMCREATOR_INTERFACE))

class IPViewItemCreator : public FPMixinInterface
{
public:

	/** @defgroup IPViewItemCreator IPViewItemCreator.h 
	*  @{
	*/

	/*! \fn virtual bool CreateItem(int pviewX, int pviewY, Tab<INode*> *anchorNodes, Tab<INode*> *showNodes) { return false; }
	*  \brief A PView item may have its own creation routine.
	If so, then the item is given an opportunity to create itself when the item is created in Pview.
	sourceNodes is a list of anchor nodes for PViews, showNodes is a list of all
	additional nodes that are added to PView to show.
	*/
#pragma warning(push)
#pragma warning(disable:4100)
	virtual bool CreateItem(int pviewX, int pviewY, Tab<INode*>* anchorNodes, Tab<INode*>* showNodes) { return false; }
#pragma warning(pop)
	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PVIEWITEMCREATOR_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PVIEWITEMCREATOR_INTERFACE); }
};

/*@}*/

