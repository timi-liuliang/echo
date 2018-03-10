	 /**********************************************************************
 
	FILE: IADTStyle.h

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
class IADTCategory;
class BitArray;

// class IADTStyle
//
// This interface models an ADT Style that can have components
// A number of ADT objects can be associated with the same style. These
// objects are reffered to as style instances. The components of these
// objects are associated with style components and are also reffered to as
// style instance.
//
// Once a style is created through the ADT Object Manager, instances of that 
// style can be created and components can be added to it
// See iADTObjMgr for more details.
class IADTStyle : public ReferenceTarget, public FPMixinInterface 
{
	public:
    using ReferenceTarget::GetInterface;
    typedef int InstIterator;
    
    // Access to the name of the style. The name is not case sensitive.
		virtual const MCHAR* GetName() const = 0;
		virtual void SetName(const MCHAR* pName) = 0;
		
		// Creates an instance of the desired style, by associating it with a node. 
		// If the node represents an instance of another style,
		// that association will be destroyed first, and then a new one created.
		// - INode& styleInst - The node that will represent an instance of this style
		// - const bool hasCompVisOverride - A value of 'true' means that this particular
		// style instance has the visibility of some of its components overriden, i.e.
		// set to a different value than the visibility defined by the style for those
		// components.
		// - Returns an iterator that can be used later to get to this particular instance
		// of this style. A negative value means failure, while a positive value means success
		virtual InstIterator SetInstance(INode& styleInst, const bool hasCompVisOverride) = 0;

		// Destroys the association between the given node and this style, and between 
		// the instances of the components of this style and their style components. 
		// To find out if a node is a style instance, call 
		// IADTStyle* IADTObjMgr::IsStyleInstance(INode& n) const;
		// - INode& n - Node representing a style instance
		// - Returns false if the node is not a style instance.
		virtual bool RemoveInstance(INode& n) = 0;
    
    // Adds a component to this style, and in the same time, creates an instance
    // of the style component. This instance becomes a component of the style instance 
    // represented by the iterator. If the style already has a component with the given 
    // name, a new one won't be created. If the given node is already an instance
    // of another style or style component, this operation changes that and makes
    // it an instance of this style component.
    // The style component names are case in-sensitive, thus if a component with 
    // this name exists aleady, an new one won't be created.
    // - const Iterator& i - This iterator represents the style instance whose 
		// component we are creating
    // - INode& styleCompInst - The node that will represent an instance of the 
    // style component
    // - const MCHAR* pCompName - The name of the style component. Cannot be NULL.
    // It's not case sensitive
    // - const bool mtlOverride - A value of 'true' means that the material of this
    // style component instance is different than that defined by the style component
    // - const bool keepRelTransf - A value of 'true' means that the transform 
    // of the style component instance relative to the style instance should not be
    // changed by this operation
    // - Returns a pointer to the style component
    virtual IADTStyle* SetComponent(const InstIterator& instIt, 
                              INode& styleCompInst, 
                              const MCHAR* pCompName, 
                              const bool hasMtlOverride, 
                              const bool keepRelTransf) = 0;

		// A style component can be removed from a style if that style component
		// has zero instances associated with it.
    // - const MCHAR* pCompName - The name of the style component. Cannot be NULL.
    // It's not case sensitive
    // - Returns false if the component cannot be found or it still has instances
		virtual bool RemoveComponent(const MCHAR* pCompName) = 0;
		
    // Returns NULL if the style component does not exist
		virtual IADTStyle* FindComponent(const MCHAR* pCompName) const = 0;
		
    // TODO: style component iteration
		virtual unsigned long NumComponents() const = 0;
		virtual unsigned long GetComponents(Tab<IADTStyle*>& components) const = 0;

		// Returns NULL if the Style doesn't belong to a category. 
		// This indicates an inconsistency in the scene
		virtual IADTCategory* GetCategory() const = 0;
    
    // If this style is a component of another style, it returns that style
    // owtherwise itself
		virtual IADTStyle* GetCompositeStyle() = 0;
		
  	// Returns the number of all nodes in the scene that represent instances of
  	// this style. 
		virtual unsigned long NumInstances() const = 0;
  	
		// Fills out the provided arrays with all the nodes in the scene that represent 
  	// instances of this style and their material override flags. Returns their count
		virtual unsigned long GetInstances(Tab<INode*>& instances, BitArray* pMtlOverrides = NULL) const = 0;
		
		// Searches the instances of this style for a match with the provided style 
		// instance. If it finds it, returns the iterator withis this style to it,
		// material and visibility override information for the style instance.
		// - INode& styleInst - The alleged style instance to search for
		// - bool* pMtlOverride - The material override flag for the instance
		// - Returns an iterator to the found style instance. If the provided style
		// instance is not an instance of this style, the iterator value will be
		// negative.
		virtual InstIterator  FindInstance(INode& styleInst, bool* pMtlOverride = NULL) const = 0;

		// Returns true if the given style instance needs a selection box displayed
		// around it when selected
		virtual bool IsSelectionBoxNeeded(INode& styleInst) const = 0;
		
		// Creates a name that is unique among the names of all style components and it's 
		// based on the given name. The unique name is seeded by passing in a seed-name
		// through the MSTR&
		// - MSTR& - Input\Output parameter. As input parameter, it represents a seed
		// for the the unique name. As output parameter, it holds the unique name that 
		// was generated
		virtual void MakeUniqueComponentName(MSTR& name) const = 0;

}; 



#define ADT_STYLE_INTERFACE Interface_ID(0x4cd242c8, 0x554201b3)
inline IADTStyle* GetADTStyleInterface(BaseInterface* baseIfc)	
{ 
	DbgAssert( baseIfc != NULL); 
	return static_cast<IADTStyle*>(baseIfc->GetInterface(ADT_STYLE_INTERFACE)); 
}

