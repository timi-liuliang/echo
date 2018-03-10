	 /**********************************************************************
 
	FILE: IADTCategory.h

	DESCRIPTION:  Public intefaces that defines the concept of ADT Styles

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Sep 29, 2002

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "ref.h"
#include "iFnPub.h"
#include "maxtypes.h"


// Forward declarations
class IADTStyle;

// This interface models an ADT Object category that has styles
// Categories can be created and destroyed via the ADT Object Manager 
// Styles are added and deleted to\from categories by the ADT Object Manager
// methods in order to ensure consistency of the data.
//
// Categories should be created and destroyed through the ADT Object Manager 
// See IADTStyle* GetStyle(const MCHAR* pCategName, const MCHAR* pStyleName);
// An ADT Category is destroyed by the object manager when there are no styles
// that is references. See iADTObjMgr for more details
class IADTCategory : public ReferenceTarget, public FPMixinInterface 
{
	public:
 	  using ReferenceTarget::GetInterface;
    
    // Access to name. The name is not case sensitive.
		virtual const MCHAR* GetName() const = 0;
		virtual void SetName(const MCHAR* pName) = 0;

    // Returns NULL if the style does not exist
		virtual IADTStyle* FindStyle(const MCHAR* pStyleName) const = 0;
		
    // TODO: styles iteration
		virtual unsigned long NumStyles() const = 0;
		virtual unsigned long GetStyles(Tab<IADTStyle*>& styles) const = 0;

		// Returns the number of all nodes in the scene that represent instances of
  	// all the styles that belong to this category. 
    virtual unsigned long NumInstances() const = 0;

		// Fills out the provided array with all the nodes in the scene that represent 
  	// instances of all the styles that belong to this category. Returns their count
		virtual unsigned long GetInstances(Tab<INode*>& instances) const = 0;

		// Creates a name that is unique among the style names and it's 
		// based on the given name. The unique name is seeded by passing in a seed-name
		// through the MSTR&
		// - MSTR& - Input\Output parameter. As input parameter, it represents a seed
		// for the the unique name. As output parameter, it holds the unique name that 
		// was generated
		virtual void MakeUniqueStyleName(MSTR& name) const = 0;
}; 

#define ADT_CATEGORY_INTERFACE Interface_ID(0x41030580, 0x7bce1c46)
inline IADTCategory* GetADTCategoryInterface(BaseInterface* baseIfc)	
{ 
	DbgAssert( baseIfc != NULL); 
	return static_cast<IADTCategory*>(baseIfc->GetInterface(ADT_CATEGORY_INTERFACE)); 
}

