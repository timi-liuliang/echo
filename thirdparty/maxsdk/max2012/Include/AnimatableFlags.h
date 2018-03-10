//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once


#define ANIMTYPE_NODE		1
#define ANIMTYPE_ROOTNODE	3
#define ANIMTYPE_CONTROL	2

// The maximum number of track views that can be opened. Each
// animatable stores 3 bits for each track to identify it's open/close
// state and selection state.
#define MAX_TRACK_VIEWS	16

#define ALL_TRACK_VIEWS	0xffff

// The maximum number of track view selection sets
#define MAX_TRACKVIEW_SELSETS	16

//! \brief 'type' argument to the OpenTreeEntry, CloseTreeEntry, IsTreeEntryOpen methods
/*! 
\see Animatable::OpenTreeEntry(int type, DWORD tv)
\see Animatable::CloseTreeEntry(int type, DWORD tv)
\see Animatable::IsTreeEntryOpen(int type, DWORD tv)
*/
#define TRACKVIEW_NODE	0  //!< operate on the tree based on the node parent/child relationship
#define TRACKVIEW_ANIM	1  //!< operate on the tree based on the animatable/subanim relationship

/*! \defgroup AnimatableFlags Animatable Flags
Animatable flags are generally meant to be used in enumerations of Animatable 
instances where each instance must be visited only once. The Animatable flags are
in general not persisted, nor copied between instances of class Animatable.
Many of these flags are for internal use only. 

Unique Animatable bit flag values can be requested via Animatable::RequestFlagBit(). 
Plugins are encouraged to use this mechanism over the older <A href="#Animatable_Work_Flags">General Animatable work flags</A>, 
or the <A href="#Animatable_Plugin_Flags">Animatable plugin flags</A>.
 */
//!@{

// BITS 5-11, and 19 are reserved for specific sub-class use.

/*! \name  Atmospheric flags */
//!@{
//! \brief The atmosphere effect is disabled
#define A_ATMOS_DISABLED		(1<<5)
//! \todo Describe this define
#define A_ATMOS_OBJECTXREF		(1<<6)
//! \todo Describe this define
#define A_ATMOS_SCENEXREF		(1<<7)
//!@}

/*! \name Tone Operator flags */
//!@{
//! \brief The exposure control is disabled
#define A_TONEOP_DISABLED		(1<<5)
//! \brief The exposure control proceses the background.
#define A_TONEOP_PROCESS_BG		(1<<6)
//! \todo Describe this define
#define A_TONEOP_INDIRECT_ONLY	(1<<7)
//!@}

/*! \name Object flags */
//!@{
//! \brief The object is being created. It doesn't want to snap to itself.
#define A_OBJ_CREATING			(1<<5)		
#ifdef _OSNAP
/*! \brief Persists throughout the whole creation process as opposed to A_OBJ_CREATING 
which gets cleared as as the object is added to the scene. */
#define A_OBJ_LONG_CREATE		(1<<6)		
#endif
#define A_OBJ_BEING_EDITED		(1<<7)
//!@}

/*! \name Modifier flags */
//!@{
//! \brief The modifier is disabled
#define A_MOD_DISABLED			(1<<5)
//! \brief The modifer is deleted
#define A_MOD_BEING_EDITED		(1<<6)
//! \brief No longer used - use sub-ob selection
#define A_MOD_USE_SEL			(1<<7)
//! \brief Modifier is disabled in viewports only
#define A_MOD_DISABLED_INVIEWS	(1<<8)
//! \brief Modifier is disabled in renderer only
#define A_MOD_DISABLED_INRENDER	(1<<9)
//!@}

/*! \name Modapp flags */
//!@{
//! \brief Used internally
#define A_MODAPP_DISABLED		(1<<5)
//! \brief parent node is selected
#define A_MODAPP_SELECTED		(1<<6)
//! \brief Used internally
#define A_MODAPP_DISPLAY_ACTIVE (1<<7)
//! \brief Used internally
#define A_MODAPP_DYNAMIC_BOX  	(1<<8)
//! \brief Render begin turns this on and render end turns it off
#define A_MODAPP_RENDERING		(1<<9)
//!@}

//! \brief When the last modifier is deleted form this derived object, don't delete the derived object
#define A_DERIVEDOBJ_DONTDELETE	(1<<9)

/*! \name Control flags */
//!@{
//! \brief Used internally
#define A_ORT_MASK				7
//! \brief Uses bits 5,6 and 7 to store ORT
#define A_ORT_BEFORESHIFT		5
//! \brief Uses bits 8,9 and 10 to store ORT
#define A_ORT_AFTERSHIFT		8
//! \brief Used internally
#define A_CTRL_DISABLED			(1<<11)
//! \brief Indicates that the ORT is disabled
#define A_ORT_DISABLED			(1<<19)
//!@}

/*! \name Inode flags */
//!@{
//! \brief  Terminates the top of an IK chain
#define A_INODE_IK_TERMINATOR	(1<<5)
//! \brief The position is pinned
#define A_INODE_IK_POS_PINNED	(1<<6)
//! \brief The rotation is pinned
#define A_INODE_IK_ROT_PINNED	(1<<7)
#ifdef _OSNAP
#define A_INODE_CLONE_TARGET	(1<<8)
#endif
//! \brief Used internally only
#define A_INODE_IN_UPDATE		(1<<9)
//! \brief Flag is set if it's updating it's TM. Don't Call GetNodeTM if it is.
#define A_INODE_IN_UPDATE_TM	(1<<10)
//!@}

//! \brief  Needed for CompositeBase and its children
/*! \note CompositeBase derives from ShapeObject) */
#define A_COMPONENT_LOCKED		(1<<19)

//! \brief Don't call RescaleWorldUnits on sub-controllers
#define A_TVNODE_DONTRESACLECONTROLLERS (1 << 5)

/*! \name Flags for Hold and Restore logic 
For "lazy holding" to avoid multiple holding. */
//!@{
//! \brief Typically a plug-in would not hold unless this flag was not set. 
/*! Then set it once it has held something, then clear it once EndHold() is 
called on the RestoreObj. This will keep it from putting multiple restore 
objects in one cycle. See Undo/Redo for more details. */
#define A_HELD				(1<<12)
//! \brief Similar to A_HELD except is used by controllers
#define A_SET				(1<<13)
//! \brief Used internally
/*! Deleted but kept around for UNDO */
#define A_IS_DELETED		(1<<14)
//! \brief Used internally
/*! To prevent AutoDelete from being re-entered. */
#define A_BEING_AUTO_DELETED  (1<<15)
//!@}

//! \brief Reserved for future internal use.
#define A_RESERVED_B16	 				(1<<16)

//! \brief Used internally
/*! Used by FileLink for replacing Acad controllers, and is used on controllers. 
\note Nothing else should use it. */
#define A_CHANGE_PARENTS_DONE			(1<<17)

//! \brief Used internally 
/*! Used to flag that at least one reference has been set on a refmaker, and that
the refmaker's references were checked at that time to make sure they were all NULL. */
#define A_REFMAKER_REFS_CHECKED			(1<<18)

/*! \name Reserved for superclass use */
//!@{
#define A_SUPERCLASS1		(1<<20)
#define A_SUPERCLASS2		(1<<21)
//!@} 

/*!  <A name="Animatable_Plugin_Flags"></A>  \name Reserved for use by plugins
These flags are not persisted with the max file, thus they should only be used as 
temporary storage. It is highly recommended that plugin code clears the flag it 
wishes to use before starting to set it on Animatable instances.*/
//!@{
#define A_PLUGIN1			(1<<22)
#define A_PLUGIN2			(1<<23)
#define A_PLUGIN3			(1<<24)
#define A_PLUGIN4			(1<<25)
//!@}

//! \brief Used to test for a dependency
#define A_DEPENDENCY_TEST	(1<<26)

//! \brief Ref target isn't deleted when dependents goes to 0 if this flag is set.
/*! Setting this flag will keep an item from being deleted when you delete a reference to it. 
For example, if you need to swap references for two items. For instance, say you have two 
nodes and two objects and you want to swap the object reference of the nodes. If you simply 
call ReplaceReference() on one node with the other node's object, the old object will get 
deleted because nothing else is referencing it anymore. By setting this flag temporarily 
you can keep it from being deleted and perform the swap. */
#define A_LOCK_TARGET		(1<<27)

/*! \name General work flags 
<A name="Animatable_Work_Flags"></A>
These flags can be used by both 3ds Max and plugins. These flags are not persisted 
with the max file, thus they should only be used as temporary storage. It is highly 
recommended that plugin code clears the work flag it wishes to use before starting 
to set it on Animatable instances.*/
//!@{
#define A_WORK1				(1<<28)
#define A_WORK2				(1<<29)
#define A_WORK3				(1<<30)
#define A_WORK4				(1<<31)
//!@}

/*! \name Values for Animatable extended flags.
Reserved for future internal use 
\see Animatable::SetAFlagEx(DWORD mask)
\see Animatable::ClearAFlagEx(DWORD mask)
\see Animatable::TestAFlagEx(DWORD mask)   */
//!@{
#define A_EX_RESERVED_B00	 		(1<<0)
#define A_EX_RESERVED_B01	 		(1<<1)
#define A_EX_RESERVED_B02	 		(1<<2)
#define A_EX_RESERVED_B03	 		(1<<3)
#define A_EX_RESERVED_B04	 		(1<<4)
#define A_EX_RESERVED_B05	 		(1<<5)
#define A_EX_RESERVED_B06	 		(1<<6)
#define A_EX_RESERVED_B07	 		(1<<7)
#define A_EX_RESERVED_B08	 		(1<<8)
#define A_EX_RESERVED_B09	 		(1<<9)
#define A_EX_RESERVED_B10	 		(1<<10)
#define A_EX_RESERVED_B11	 		(1<<11)
#define A_EX_RESERVED_B12	 		(1<<12)
#define A_EX_RESERVED_B13	 		(1<<13)
#define A_EX_RESERVED_B14	 		(1<<14)
#define A_EX_RESERVED_B15	 		(1<<15)
#define A_EX_RESERVED_B16	 		(1<<16)
#define A_EX_RESERVED_B17	 		(1<<17)
#define A_EX_RESERVED_B18	 		(1<<18)
#define A_EX_RESERVED_B19	 		(1<<19)
#define A_EX_RESERVED_B20	 		(1<<20)
#define A_EX_RESERVED_B21	 		(1<<21)
#define A_EX_RESERVED_B22	 		(1<<22)
#define A_EX_RESERVED_B23	 		(1<<23)
#define A_EX_RESERVED_B24	 		(1<<24)
#define A_EX_RESERVED_B25	 		(1<<25)
#define A_EX_RESERVED_B26	 		(1<<26)
#define A_EX_RESERVED_B27	 		(1<<27)
#define A_EX_RESERVED_B28	 		(1<<28)
#define A_EX_RESERVED_B29	 		(1<<29)
#define A_EX_RESERVED_B30	 		(1<<30)
#define A_EX_RESERVED_B31	 		(1<<31)
//!@}

/*! \name Values for Animatable::aflag
The following flags are bits of the aflag data member of class Animatable. \n
See methods Animatable::ClearAFlag(), Animatable::SetAFlag() and Animatable::TestAFlag() 
to work with these flags. */
//!\{
//! \brief Used Internally
#define A_EVALUATING 			1
//! \brief Used Internally
#define A_NOTIFYDEP  			(1<<1)
//! \brief Used Internally
#define A_DEPENDENTS_BEING_ENUMERATED	(1<<2)
//! \brief Reserved for future internal use
#define A_RESERVED_B03			(1<<3) 
//! \brief Used Internally
#define A_OBJECT_REDUCED		(1<<4)
//!\}

