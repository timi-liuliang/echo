 /**********************************************************************
 
	FILE: IADTObjMgr.h

	DESCRIPTION:  Public interface for working with (file)linked ADT Objects 

	CREATED BY: Attila Szabo, Discreet

	HISTORY: - created Sep.25.02

 *>	Copyright (c) 1998-2002, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "ref.h"
#include "iFnPub.h"
#include "maxtypes.h"
#include "GetCOREInterface.h"

// --- Forward declaration
class IADTCategory;
class IADTStyle;
class INode;
class RemapDir;

// --- Interface IDs
#define ADT_OBJ_MGR_INTERFACE Interface_ID(0x3d8514f1, 0x7c9252af)

// class IADTObjMgr
//
// This interface allows different parts of the system (max) and plugins
// to create, destroy and work with ADT Objects.
//
// ADT objects have categories and styles associated with them. 
// Examples of categories are wall, window, etc. Each category has a number 
// of styles. Styles have components. Each style component defines the dimensions
// of some part of the ADT object, display and rendering properties.
//
// Although in ADT objects don't have components, only styles have, when transferred
// into max (or derived products), the ADT objects will have components the user
// can select and work with. These components will correspond to the components of
// the style that is associated with the ADT object and we sometimes reffer to them
// as style instances
//
// This is how client code can create style instances (that is, associate category, 
// style and style component information with nodes):
// (i) Make sure that desired category and style exist. Call:
//    IADTStyle* IADTObjMgr::GetStyle(const MCHAR* pCategName, const MCHAR* pStyleName)
// (ii) Create an instance of the desired style.  
//    IADTStyle::Iterator IADTStyle::SetInstance(INode& styleInst, bool hasCompVisOverrides);
// (iii) Add style component instances to the style instance created above.
//    bool IADTStyle::SetComponent(IADTStyle::Iterator i, INode& styleCompInst, 
//                              const MCHAR* pCompName, bool mtlOverride, 
//                              bool keepRelativeTransform);
//
// Style instances are destroyed when the nodes associated with a style and style
// component are deleted, rather than by deleting a style or a category.
// For more deatils see iADTObjMgr.h, iADTStyle.h, iADTCategory.h
class IADTObjMgr : public ReferenceTarget, public FPStaticInterface
{
	public:
    	  
		// Makes sure the given category and the given style within that category 
		// exist. If the category and\or the style exist, simply returns a pointer
		// to the style - it doesn't create a new one. The resulting style can be 
		// used by client code to create instances of that style (ADT objects that
		// are associated with that style). The category and style names cannot 
		// be NULL and are case in-sensitive.
		// - const MCHAR* pCategName - The name of ADT object category (wall, window, etc)
		// - const MCHAR* pStyleName - The name of ADT Style. This cannot be NULL.
    // - Returns NULL if it doesn't succeed, otherwise the style within the given category
	  virtual IADTStyle* GetStyle(const MCHAR* pCategName, const MCHAR* pStyleName) = 0;
	
    // Checks whether a node is associated with an ADT Style. If yes, it means
    // it's a style instance and the method returns a pointer to the instanced 
    // style or style component.
    // - INode& n - the node we want to know if it's a style instance
		virtual IADTStyle* IsInstance(INode& n) const = 0;
		
		// Returns the node that represents the top of an ADT object. In other words,
		// if parameter n is a component of an ADT object, this method returns the node
		// that is an instance of the style whose component n is.
		// If n is not a style instance, return NULL.
		virtual INode* GetInstanceTop(INode& n) const = 0;

  	// Returns the number of all nodes in the scene that represent style instances 
  	// (this number doesn't include style component instances) 
    virtual unsigned long NumInstances() const = 0;
  	
		// Fills out the provided array with all the nodes in the scene that represent 
  	// style instances. Returns their count
		virtual unsigned long GetInstances(Tab<INode*>& instances) const = 0;
		
    // Returns true if the node is a style instance and if that style has 
		// more then one instance 
		virtual bool CanMakeUnique(INode& node) const = 0;

		// Makes unique the given style instance. This means, that after this operation 
		// the style instance (node) will be associate with a unique style, but it will
		// maintain its material override and visibility override flags.
		// When called with a node that is a style composite instance, the components of that 
		// style instance are also made unique (the child nodes that are style instances)
		// To find out if a style instance can be made unique, call IADTObjMgr::CanMakeUnique
		// NOTE: this method does not make unique the objects referenced by the node;
		// it only makes unique the style makes the new style reference the given node.
    // A node should be always referenced by only one style - a node can be the instance of
    // one style only.
		// - INode& - the Node that represents the style instance. 
		// - RemapDir* - allows for making style instances unique with respect to 
		// each other or not
		// - Returns a pointer to the style of the unique style instance or NULL if the
		// operation failed
		virtual IADTStyle* MakeUnique(INode& node, RemapDir* pRemapDir) const = 0;

		// Creates a name that is unique among the names of all categories and it's 
		// based on the given name. The unique name is seeded by passing in a seed-name
		// through the MSTR&
		// - MSTR& - Input\Output parameter. As input parameter, it represents a seed
		// for the the unique name. As output parameter, it holds the unique name that 
		// was generated
		virtual void MakeUniqueCategoryName(MSTR& name) const = 0;

		// TOOD: Category iteration
		virtual unsigned long NumCategories() const = 0;

		// Retrieves all Categories within the current drawing
		virtual unsigned long GetCategories(Tab<IADTCategory*>& categories) const = 0;
		
		// Retrieves a category given a category name
		virtual IADTCategory* FindCategory(const MCHAR* pCategName) const = 0;

		// For each node in the given array that represents a style instance, extends 
		// the current selection set with the nodes that represent instances of components
		// of that style
		virtual void ExpandSelection(INodeTab& nodes) const = 0;

    // This is how client code can get to the ADT Object Manager
		static IADTObjMgr* GetADTObjMgr()
		{
			return static_cast<IADTObjMgr*>(GetCOREInterface(ADT_OBJ_MGR_INTERFACE));
		}
}; 




