/**********************************************************************
 *<
	FILE: IRefHierarchy.h

	DESCRIPTION: Interface for accessing reference hierarchy related info

	CREATED BY: Attila Szabo

	HISTORY: Created Nov/27/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"

// This interface is supposed to group reference hierarchy related methods
/*! \sa  Class ReferenceTarget, Class FPStaticInterface\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This interface groups reference hierarchy related methods.\n\n
The global method <b>IRefHierarchy* GetRefHierarchyInterface()</b> will return
a pointer to the reference hierarchy interface class.  */
class IRefHierarchy : public FPStaticInterface 
{
	public:
	
		// function IDs 
		enum 
		{ 
			fnIdIsRefTargetInstanced,
		};

		// This method can be used to find out if an object is instanced 
		// (instanced pipeline). 
		// It Checks if the Derived Object is instanced. If it is, the pipeline 
		// part below and including that derived object is instanced.
		// If the argument is NULL, returns FALSE
		/*! \remarks This method can be used to find out if an object is
		instanced (instanced pipeline). It Checks if the Derived Object is
		instanced. If it is, the pipeline part below and including that derived
		object is instanced.If the argument is NULL, returns FALSE
		\par Parameters:
		<b>ReferenceTarget* refTarget</b>\n\n
		A pointer to a reference target to check for instantiation.
		\return  TRUE if the reference target is instanced, otherwise FALSE. */
		virtual BOOL IsRefTargetInstanced( ReferenceTarget* refTarget ) = 0;
};


#define REFHIERARCHY_INTERFACE Interface_ID(0x296e2793, 0x247d12e4)
inline IRefHierarchy* GetRefHierarchyInterface() 
{ 
	return static_cast<IRefHierarchy*>(GetCOREInterface(REFHIERARCHY_INTERFACE)); 
}


