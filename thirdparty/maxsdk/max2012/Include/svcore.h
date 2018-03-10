//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
#pragma once
#include <WTypes.h>
#include "maxheap.h"

/** The Schematic View traversal status */
enum SvTraverseStatus
{
	SVT_PROCEED,		//!< The traversal should proceed.
	SVT_EXISTS,			//!< This is not currently being used.
	SVT_DO_NOT_PROCEED	//!< The traversal should not proceed.
};

/** The Schematic View reference types */
enum SvReferenceType
{
	REFTYPE_CHILD,
	REFTYPE_SUBANIM,
	REFTYPE_PLUGIN
};

/** The Schematic View relationship types */
enum SvRelationshipType
{
	RELTYPE_INSTANCE,
	RELTYPE_PARAMWIRE,
	RELTYPE_CONSTRAINT,
	RELTYPE_LIGHT,
	RELTYPE_MODIFIER,
	RELTYPE_CONTROLLER,
	RELTYPE_OTHER
};


/// \defgroup SV_Filter_bits Schematic View Filter bits
//@{
static const DWORD SV_FILTER_SELOBJECTS     = (1<<0);	//!< Show only Selected objects.
static const DWORD SV_FILTER_OBJECTMODS     = (1<<1);	//!< Show Modified Objects.
static const DWORD SV_FILTER_BASEPARAMS     = (1<<2);	//!< Show Base Objects.
static const DWORD SV_FILTER_MATPARAMS      = (1<<3);	//!< Show Materials.
static const DWORD SV_FILTER_GEOM           = (1<<4);	//!< Show geometry nodes.
static const DWORD SV_FILTER_SHAPES         = (1<<5);	//!< Show shape nodes.
static const DWORD SV_FILTER_LIGHTS         = (1<<6);	//!< Show light nodes.
static const DWORD SV_FILTER_CAMERAS        = (1<<7);	//!< Show camera nodes.
static const DWORD SV_FILTER_HELPERS        = (1<<8);	//!< Show helper nodes.
static const DWORD SV_FILTER_WARPS          = (1<<9);	//!< Show space warps.
static const DWORD SV_FILTER_VISIBLE_OBJS   = (1<<10);	//!< Show only Visible objects.
static const DWORD SV_FILTER_CONTROLLERS    = (1<<11);	//!< Show controllers.
static const DWORD SV_FILTER_ANIMATEDONLY   = (1<<12);	//!< Show only animated nodes
static const DWORD SV_FILTER_MAPS           = (1<<13);	//!< \deprecated Show maps.
static const DWORD SV_FILTER_BONES          = (1<<14);	//!< \deprecated Show bone base objects.
static const DWORD SV_FILTER_BONEOBJECTS    = (1<<15);	//!< Show bone base objects
static const DWORD SV_FILTER_PB_PARAMS      = (1<<16);
static const DWORD SV_FILTER_PRS_POS        = (1<<17);
static const DWORD SV_FILTER_PRS_ROT        = (1<<18);
static const DWORD SV_FILTER_PRS_SCALE      = (1<<19);
static const DWORD SV_FILTER_MASTER_POINT   = (1<<20);
static const DWORD SV_FILTER_SKIN_DETAILS   = (1<<21);
//@}


/** \defgroup Schematic_view_colors Schematic view colors
 *  \internal The listing in "Viewport Drawing Color Indices" might apply to these constants as well. needs to be verified. */
//@{
static const int SV_UICLR_WINBK               = 0;	
static const int SV_UICLR_NODEBK              = 1;
static const int SV_UICLR_SELNODEBK           = 2;
static const int SV_UICLR_NODE_HIGHLIGHT      = 3;
static const int SV_UICLR_MATERIAL_HIGHLIGHT  = 4;
static const int SV_UICLR_MODIFIER_HIGHLIGHT  = 5;
static const int SV_UICLR_PLUGIN_HIGHLIGHT    = 6;
static const int SV_UICLR_SUBANIM_LINE        = 7;
static const int SV_UICLR_CHILD_LINE          = 8;
static const int SV_UICLR_FRAME               = 9;
static const int SV_UICLR_SELTEXT             = 10;
static const int SV_UICLR_TEXT                = 11;
static const int SV_UICLR_FOCUS               = 12;
static const int SV_UICLR_MARQUIS             = 13;
static const int SV_UICLR_COLLAPSEARROW       = 14;
static const int SV_UICLR_GEOMOBJECT_BK       = 15;
static const int SV_UICLR_LIGHT_BK            = 16;
static const int SV_UICLR_CAMERA_BK           = 17;
static const int SV_UICLR_SHAPE_BK            = 18;
static const int SV_UICLR_HELPER_BK           = 19;
static const int SV_UICLR_SYSTEM_BK           = 20;
static const int SV_UICLR_CONTROLLER_BK       = 21;
static const int SV_UICLR_MODIFIER_BK         = 22;
static const int SV_UICLR_MATERIAL_BK         = 23;
static const int SV_UICLR_MAP_BK              = 24;
static const int SV_UICLR_GRID                = 25;
static const int SV_UICLR_REL_INSTANCE        = 26;
static const int SV_UICLR_REL_CONSTRAINT      = 27;
static const int SV_UICLR_REL_PARAMWIRE       = 28;
static const int SV_UICLR_REL_LIGHT           = 29;
static const int SV_UICLR_REL_MODIFIER        = 30;
static const int SV_UICLR_REL_CONTROLLER      = 31;
static const int SV_UICLR_REL_OTHER           = 32;
static const int SV_UICLR_SPACEWARP_BK        = 33;
static const int SV_UICLR_BASEOBJECT_BK       = 34;

static const int SV_UICLR_CACHE_SIZE          = 35;
//@}

/** Magic value returned from Animatable::SvGetSwatchColor()
 * to indicate that no swatch is to be drawn... */
static const int SV_NO_SWATCH							= 0xFFFFFFFF;

/** \defgroup Flags_for_AddAnimatable___and_SvTravereseAnimGraph__ Flags for AddAnimatable() and SvTravereseAnimGraph()
 * These flags can be passed to IGraphObjectManager::AddAnimatable()
 * and Animatable::SvTraverseAnimGraph() */
//@{
/** If set, newly created node will be in the hidden state.  If the node
 * already exists in the graph, the flag is ignored. */
static const DWORD SV_INITIALLY_HIDDEN          = 0x00000001;

/** If set, shared instances of an animatable will produce multiple
 * graph nodes in the schematic view instead of a single shared graph node. */
static const DWORD SV_DUPLICATE_INSTANCES       = 0x00000002;

/** If set, the newly created children of the newly created node
 * will be in the hidden state.  If the node already exists in the graph,
 * the flag is ignored.  Children of this node that already exist in
 * the graph will not have their visibility state changed.*/
static const DWORD SV_INITIALLY_CLOSED          = 0x00000004;
//@}

/** \defgroup Flags_for_PushLevel___ Flags for PushLevel() 
 * These flags can be passed to IGraphObjectManager::PushLevel()*/
//@{
/** This id, when passed to IGraphObjectManager::PushLevel(), indicates that no id is to be associated
 * with Animatable being pushed onto the stack. */
static const int SV_NO_ID                       = 0x80000000;
//@}

class IGraphNode;
class Animatable;
class IGraphObjectManager;

/** This is the callback object used to perform the [de]selection via
 * Animatable:: SvGetMultiSelectCallback().
 * Schematic view supports multiple selection. When the user selects a set of
 * objects in the schematic view and then "transfers" that selection set to the
 * rest of max (either by having "synchronize selection" on or by manually moving
 * the selection out), there are a number of ambiguities that can arise. For
 * example, some of the objects in the schematic view cannot be selected in the
 * viewports, material editor, or modifier stack. Another example: the material
 * editor only supports one active material/map but many materials and maps can be
 * selected simultaneously in the schematic view. The "MultiSelectCallback" system
 * exists order to handle these cases and to handle selection synchronization
 * between SV and future editors in 3ds Max. When the schematic view attempts to
 * synchronize selection by moving the SV selection set to the "outside" world, it
 * follows this procedure:\n\n
 * 1. First SV calls SvGetMultiSelectCallback() on all the visible SV nodes to
 * "collect" MultiSelectCallback objects. Objects that want to synchronize their
 * selection state with the schematic view (not a common or trivial operation --
 * this is really more associated with adding a new editor in 3ds Max rather than
 * adding new plug-in) return a pointer to a static instance of a
 * MultiSelectCallback derived object. There is only one instance of a
 * MultiSelectCallback per editor. Furthermore, if an editor displays objects of
 * many classes, all the classes should override SvGetMultiSelectCallback() to
 * return the same MultiSelectCallback instance. This implies that, as far as the
 * schematic view is concerned, there is never more than one primary editor class
 * associated with any particular object class (currently, viewports for nodes,
 * material editor for materials and maps and the modifier panel for
 * modifiers).
 * For example, here is the code in BaseNode that returns the MultiSelectCallback
 * instance for nodes (this is the MultiSelectCallback used for viewports):
\code
class BaseNodeMSelCB : public MultiSelectCallback
{
	private:
		bool clear;
		BaseNodeTab selNodeTab;
		BaseNodeTab deselNodeTab;

		int Priority() { return 1000; }
		void Begin(IGraphObjectManager *gom, bool clear);
		void Select(IGraphObjectManager *gom, IGraphNode *gNode, bool isSelected);
		void End(IGraphObjectManager *gom);
};
static BaseNodeMSelCB baseNodeMSelCB;
MultiSelectCallback* BaseNode::SvGetMultiSelectCallback(IGraphObjectManager *gom, IGraphNode *gNode)
{
	return &baseNodeMSelCB;
}
\endcode 
2. For each selection class (unique MultiSelectCallback instance), the
schematic views calls "Begin()". This is the spot where any "pre-selection"
preparation takes place. The order that the MultiSelectCallback instances are
called in is determined by their priority. The priority is returned by the
"Priority()" method. MultiSelectCallback's with a higher priority (lower value)
are called before those with a lower priority (higher value). For example, here
is the Begin associated with the viewports:
\code
void BaseNodeMSelCB::Begin(IGraphObjectManager *gom, bool clear)
{
	this->clear = clear;
//
// If the "clear" bool is true, the current viewport selection set is cleared...
//
		if (clear)
		GetActiveSelSet()->Clear(FALSE);
//
// Some housekeeping in preparation for the select...
//
	selNodeTab.Resize(0);
	deselNodeTab.Resize(0);
}
\endcode 
 * 3. For each of objects in the schematic view whose selection state is changing,
 * the object's MultiSelectCallback instance is retrieved (again) and the "Select"
 * method is called. Here is where the actual selection/de-selection work can take
 * place. I say "can" because, in practice, the select method usually just
 * collects all the objects to be selected and all the objects to be deselected
 * into lists which are then processed in the "End()" method. This is simply
 * for performance -- it is often more efficient to set the selection state of a
 * group of objects all at once. Here's the "Select()" method from
 * BaseNode:
\code
void Select(IGraphObjectManager *gom, IGraphNode *gNode, bool isSelected)
{
	BaseNode *baseNode = (BaseNode *) gNode->GetAnim();

	if (isSelected) {
		if (!baseNode->IsRootNode() && !baseNode->IsFrozen()
			&& !baseNode->IsHidden())
			selNodeTab.AppendNode(baseNode, FALSE);
	}
	else {
		if (baseNode->Selected())
			deselNodeTab.AppendNode(baseNode, FALSE);
	}
}
\endcode 
 * 4. Finally, for each selection class (unique MultiSelectCallback instance), the
 * schematic views calls "End()". This is the spot where any "post-selection"
 * operations take place. For example, here is the "End()" for the BaseNode
 * (viewports):
\code
void End(IGraphObjectManager *gom)
{
	if (selNodeTab.Count() > 0 || deselNodeTab.Count() > 0) {
		theHold.Begin();
		if (selNodeTab.Count() > 0)
			GetActiveSelSet()->SelectMultiple(selNodeTab, FALSE);

		if (deselNodeTab.Count() > 0)
			GetActiveSelSet()->DeselectMultiple(deselNodeTab, FALSE);

		theHold.Accept(getResMgr().getString(IDS_SV_SELECT, appInst));
		RedrawViewports(GetCOREInterface()->GetTime(), VP_DONT_SIMPLIFY);
	}
	else {
		if (clear)
			RedrawViewports(GetCOREInterface()->GetTime(), VP_DONT_SIMPLIFY);
	}
}
\endcode  
 * \see Animatable*/
class MultiSelectCallback: public MaxHeapOperators
	{
	public:
		virtual ~MultiSelectCallback() {}
	/** Returns the priority of the callback.
	 * MultiSelectCallback's with a higher priority (lower value) are called before those with a lower
	 * with a higher priority (lower value) are called before those with a lower
	 * priority (higher value). */
	virtual int Priority() = 0;		// Used for sorting select order.
	/** Called to begin the multi-selection process. 
	 * This is the spot where any "pre-selection" operations take place.
	 * \param gom - Points to the schematic view window manager.
	 * \param clear - true to clear the previous selection; false to leave intact. */
	virtual void Begin(IGraphObjectManager *gom, bool clear) = 0;
	/** Selects or de-selects the node passed.
	 * \param gom Points to the schematic view window manager.
	 * \param gNode Points to the node in schematic view.
	 * \param isSelected true if select; false if deselect. */
	virtual void Select(IGraphObjectManager *gom, IGraphNode *gNode, bool isSelected) = 0;
	/** Called when done. 
	 *! This is the spot where any "post-selection"	operations take place.
	 * \param gom Points to the schematic view window manager. */
	virtual void End(IGraphObjectManager *gom) = 0;
	};

/** Represents a node pointer and currently has no methods. 
 * \see IGraphObjectManager, IGraphNode, Object */
class IGraphRef: public MaxHeapOperators
	{
	public:
	};

/** Represents a node in the schematic view graph and provides a few
 * methods for querying information about the node.  
 * \see IGraphObjectManager, Object, Animatable*/
class IGraphNode: public MaxHeapOperators
	{
	public:

	/**Returns the Animatable associated with this node. */
	virtual Animatable *GetAnim() = 0;

	/** Returns the "primary parent" of this node. 
	 * Nodes can have multiple parents (objects referencing this node) so this function is not
	 * strictly accurate. That said, many nodes have the concept of an owner node,
	 * which is what this function returns. */
	virtual IGraphNode *GetParentNode() = 0;

	/** Returns the "owner" of this node. 
	 * Some nodes have multiple owners. When this is the case, this function returns the "first" owner (the
	 * object that first added this node to the schematic view). */
	virtual Animatable *GetOwner() = 0;

	/** Returns the ID of this node.
	 * When nodes are added to the schematic view (via the IGraphObjectManager::AddAnimatable()
	 * method), an integer is provided. This value is not used internally by the
	 * schematic view. Rather, it is available to implementers of the
	 * Animatable::Sv*() methods to aid in identifying the node. */
	virtual int GetID() = 0;
	};

/** A small container type class that associates graph nodes with traversal status. 
 * \see IGraphNode, IGraphObjectManager, Animatable
*/
class SvGraphNodeReference: public MaxHeapOperators
	{
	public:

	/** Points to the interface for the node in the schematic view */
	IGraphNode *gNode;
	/** The traversal status.*/
	SvTraverseStatus stat;

	/** Constructor */
	SvGraphNodeReference()
		{
		gNode= NULL;
		stat = SVT_DO_NOT_PROCEED;
		}
	};

/** Represents an instance of a schematic view window and
 * provides methods for adding nodes and node pointers, refreshing the schematic
 * view, accessing filter bits and updating and controlling the various editors
 * within 3ds Max in ways that are not surfaced in the general interface.  
 * \see IGraphNode, IGraphRef, Animatable */
class IGraphObjectManager: public MaxHeapOperators
	{
	public:

	/** Push a level onto the animatable ownership stack.
	 * During traversal of the Animatable graph via
	 * SvTraverseAnimGraph(), this method (along with PopLevel())
	 * should be called appropriately to maintain an ownership stack. This is
	 * required by the schematic view when nodes are added to the graph with the
	 * SV_DUPLICATE_INSTANCES flag set. Note: In 3ds Max 3.0,
	 * SV_DUPLICATE_INSTANCES is always on (i.e., the flag is ignored).
	 * Because of this, PushLevel() and PopLevel() should always be
	 * called in SvTraverseAnimGraph().
	 * See the sample code in Animatable::SvStdTraverseAnimGraph() for an
	 * example of this.
	 * \param anim This is the Animatable that you are, in all likelihood, about to add to the
	 * graph via the AddAnimatable() call.
	 * \param id (default to SV_NO_ID) This is also the same "id" you'd pass into AddAnimatable(). The "id"
	 * is only required in cases where it's impossible for the schematic view to
	 * distinguish between two (or more) children in the tree that have the same
	 * Animatable but represent different sub-anims. For example, a box has both
	 * its width and height set to the same controller (instanced). In the
	 * schematic view, this is still shown in tree form so we need the ID to
	 * distinguish between the "width" and "height" children. */
	virtual void PushLevel(Animatable *anim, int id = SV_NO_ID) = 0;
	
	/** Pops a level off the animatable ownership stack.
	* \see PushLevel() */
	virtual void PopLevel() = 0;

	/** Adds an Animatable to the schematic view. 
	 * Note that "owner" and "id" are actually arbitrary - they are used to provide context for this
	 * Animatable. This means that the plug-in developer can set them to any
	 * value. They are not used internally by the schematic view except in the
	 * "Sv*" methods (which the developer can override). So, when you add an
	 * Animatable to the schematic view, you would typically add the owner
	 * (parent) Animatable as well as the sub-anim id. This allows you to, for
	 * example, easily return the name of the object when queried by the
	 * SvGetName() call (whose default implementation is shown below):

	\code
	MSTR Animatable::SvGetName(IGraphObjectManager *gom, IGraphNode *gNode, bool isBeingEdited)
	{
		Animatable *owner;
		int subNum;
		MSTR name;
		owner = gNode->GetOwner();
		subNum = gNode->GetID();
		name = owner->SubAnimName(subNum);
	
		return name;
	}
	\endcode

	* \param anim Points to the animatable to add.
	* \param owner Points to the owner of "anim" above (typically).
	* \param id When nodes are added to the schematic view via this method this integer is
	* provided. This value is not used internally by the schematic view. Rather,
	* it is available to implementers of the Animatable::Sv*() methods to
	* aid in identifying the node.
	* \param flags This flag is some combination of the bit flags in \ref Flags_for_AddAnimatable___and_SvTravereseAnimGraph__ 
	* \return  A SvGraphNodeReference object. */
	virtual SvGraphNodeReference AddAnimatable(Animatable *anim, Animatable *owner, int id, DWORD flags = 0) = 0;

	/** This method adds a reference from the specified "maker" node to the specified "target" node.
	 * \param maker Points to the 'maker' node in schematic view.
	 * \param target Points to the 'target' node in schematic view.
	 * \param type One of the following enum values:
	 * \li REFTYPE_CHILD
	 * \li REFTYPE_SUBANIM
	 * \li REFTYPE_PLUGIN
	 * \return  A pointer to an IGraphRef object.
	 * \par Sample Code:
	\code
	SvGraphNodeReference Control::SvTraverseAnimGraph(IGraphObjectManager *gom, Animatable *owner, int id, DWORD flags)
	{
		int i;
		SvGraphNodeReference nodeRef;
		SvGraphNodeReference childNodeRef;
	
	//
	// Test filter to see if "Controllers" are active.
	// Bail out if they're off (being filtered out)...
	//
		if (!gom->TestFilter(SV_FILTER_CONTROLLERS))
			return SvGraphNodeReference();
	
	//
	// Push this level in the tree. Note that the sub-anim id is passed
	// in here because it's possible that the same instance of this control
	// may exist in multiple tracks of "owner".
	//
		gom->PushLevel(this, id);
	
	//
	// Some flags are set here pertaining to the control being added.
	// Note that the flags are also propagated down the tree
	// by passing them on to SubAnim(i)->SvTraverseAnimGraph(gom, this, i, flags);
	// SV_DUPLICATE_INSTANCES tells the schematic view not to
	// represent multiple instances with a single node. Instead they
	// are represented by multiple nodes in the schematic view
	// with the "triangle thingy" attached to the side to indicate
	// shared instances. This flag is ignored in R3 because
	// this mode of operation is globally enabled
	// SV_INITIALLY_HIDDEN tells the schematic view that this
	// control's node is to be initially displayed in the closed state.
	// Note that this has no effect if the node already exists
	// in the graph -- it only applies to newly added nodes.
	//
		flags |= SV_DUPLICATE_INSTANCES | SV_INITIALLY_HIDDEN;
	
	//
	// The control is added to the schematic view...
	//
		nodeRef = gom->AddAnimatable(this, owner, id, flags);
		if (nodeRef.stat == SVT_PROCEED) {
	//
	// This control's sub-anims are iterated over...
	//
			for (i = 0; i < NumSubs(); i++) {
				if (SubAnim(i)) {
	//
	// SvTraverseAnimGraph() is recursively called to add this sub-anim (and all its descendants) to the graph...
	//
					childNodeRef = SubAnim(i)->SvTraverseAnimGraph(gom, this, i, flags);
	
	//
	// Now a link (node pointer) is created in the schematic between the control (nodeRef.gNode) and its child sub-anim (childNodeRef.gNode)....
	//
					if (childNodeRef.stat != SVT_DO_NOT_PROCEED)
						gom->AddReference(nodeRef.gNode, childNodeRef.gNode, REFTYPE_SUBANIM);
				}
			}
		}
	
	//
	// The tree level is popped. Note: a PopLevel() call must always be	paired with a PushLevel() call!
	//
		gom->PopLevel();
	
		return nodeRef;
	}
	\endcode  */
	virtual IGraphRef *AddReference(IGraphNode *maker, IGraphNode *target, SvReferenceType type) = 0;

	/** This method adds a relationship from the specified "maker" node to the specified "target" node.
	 * \param maker Points to the 'maker' node in schematic view.
	 * \param target Points to the 'target' node in schematic view.
	 * \param id An arbitrary ID that the Animatable can use later to identify itself.
	 * \param type the type of the relationship
	 * \return A pointer to an IGraphRef object.*/
	virtual IGraphRef *AddRelationship( IGraphNode *maker, Animatable *anim, int id, SvRelationshipType type) = 0;

	/** Pops up the property editor dialog on the selected nodes in the schematic view. */
	virtual void SvEditSelectedNodeProperties() = 0;

	/** Selects the given node in the material editor. 
	 * Does nothing if "gNode" does not represent a material or map.
	 * \param gNode Points to the node in schematic view. */
	virtual void SvSelectInMaterialEditor(IGraphNode *gNode) = 0;

	/** Selects the given node in the modifier panel. 
	 * Does nothing if "gNode" does not represent an object.
	 * \param gNode Points to the node in schematic view. */
	virtual void SvSetCurEditObject(IGraphNode *gNode) = 0;

	/** Returns true if the given node is current
	 * in the modifier panel */
	virtual bool SvIsCurEditObject(IGraphNode *gNode) = 0;

	virtual bool ApplyModifier(IGraphNode *gModNode, IGraphNode *gParentNode) = 0;
	virtual bool DeleteModifier(IGraphNode *gNode) = 0;

	/** Invokes the copy/instance controller dialog within schematic view.
	 * The controller referenced by gSrcNode will be either copied or instanced into gDestNode.
	 * This is typically called from within a link callback:
	\code
	bool myControl::SvLinkChild(IGraphObjectManager *gom, IGraphNode *gNodeThis, IGraphNode *gNodeChild)
	{
		if( SvCanConcludeLink( gom, gNodeThis, gNodeChild ) )
			return gom->ApplyController(gNodeChild, gNodeThis);

		return false;
	}
	\endcode 
	 * \param gSrcNode The node that supplies the Matrix3 controller to apply.
	 * \param gDestNode The node to apply the controller to. 
	 * \return true if the controller is successfully applied, false if not. */
	virtual bool ApplyController(IGraphNode *gSrcNode, IGraphNode *gDestNode) = 0;

	/** Invalidates the schematic view window.*/
	virtual void SvInvalidateView() = 0;

	/** Invalidates a node in the schematic view window.
	 * \param gNode - Points to the node in schematic view. */
	virtual void SvInvalidateNode(IGraphNode *gNode) = 0;

	/** Forces the material editor to update. */
	virtual void SvUpdateMaterialEditor() = 0;

	/** Forces the modifier panel to update. */
	virtual void SvUpdateModifierPanel() = 0;

	/** Sets the specified filter bits.
	 * \param mask See \ref SV_Filter_bits */
	virtual void SetFilter(DWORD mask) = 0;
	
	/** Clears the specified filter bits.
	 * \param mask See \ref SV_Filter_bits */
	virtual void ClearFilter(DWORD mask) = 0;
	
	/** Tets the specified filter bits. Returns true if set;
	 * otherwise false.
	 * \param mask See \ref SV_Filter_bits */
	virtual bool TestFilter(DWORD mask) = 0;

	/** Get a SV UI color given a color index. */
	virtual COLORREF SvGetUIColor(int colorIndex) = 0;

	/** Get HWnd for Schematic View window. */
	virtual HWND GetHWnd() = 0;
	};


