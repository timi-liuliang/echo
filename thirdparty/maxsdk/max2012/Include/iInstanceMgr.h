//*****************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
// DESCRIPTION: Interface for working with instances of scene entities
// AUTHOR: Attila Szabo - created Aug.26.2002
//*****************************************************************************/
#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "GetCOREInterface.h"

// --- Forward declaration
class INode;
class INodeTab;
class Mtl;

// --- Interface IDs
#define INSTANCE_MGR_INTERFACE Interface_ID(0x5ecd70b5, 0x59092257)

/** \defgroup InstanceMgrMakeUniqueOptions Instance Manager Make Unique Options
 * These options control whether a group of instanced objects are made unique 
 * with respect to each other or made unique with respect to the remaining 
 * instanced objects. These options are only relevant when more than one node is
 * being processed.
*/
//@{
/*! \brief Prompt user to choose how instanced elements are to be made unique */
#define INSTANCE_MGR_MAKE_UNIQUE_PROMPT 0 
/*! \brief Makes instanced elements unique with respect to each other */
#define INSTANCE_MGR_MAKE_UNIQUE_INDIVIDUAL 1 
/*! \brief Makes instanced elements unique with respect to all other instances of those elements */
#define INSTANCE_MGR_MAKE_UNIQUE_GROUP 2 
//@}

/*! \brief Manages instances of objects, modifiers, transformation controllers
 * Use this interface to work with object instances in a Max scene.
 * There are several criteria based on which two or more objects must be considered 
 * instances of each other. This interface abstracts the details of dealing with them
 * and thus help client code focus on the specific tasks it needs to perform on object 
 * instances, rather than on the logic of finding them.
 * Two or more objects are considered instances of each other if:
 * - They are regular Max instances or references (their nodes reference the same DerivedObject)
 * - Are part of the same style (this applies to Max objects resulted from importing 
 * a DWG drawing that contains style information. See IADTObjMgr for more information on styles.)
 * - Represent instances of the same block (this applies to Max objects resulted from 
 * importing a DWG drawing that contains block instances. See IBlockMgr for more 
 * information on blocks.)
 * 
 * \see IADTObjMgr, IBlockMgr
*/
class IInstanceMgr : public FPStaticInterface
{
	public:
		/*! \brief Retrieves instances and references of an object.
		 * Instances of styles and blocks are collected only if IADTObjMgr and IBlockMgr 
		 * interfaces are available.
		 * \param [in] source The node of the object whose instances are to be retrieved. 
		 * For an object with multiple instances, any of the nodes representing an instance 
		 * can be specified here as a parameter.
		 * \param [in,out] instances Collection of nodes representing the instances of source
		 * The collection is cleared before this method starts filling it in. The source node 
		 * is also included in this collection since every object can be thought of as having 
		 * at least one instance.
		 * \return The number of instances found. There's always at least 1 instance found
		*/
		virtual unsigned long GetInstances(INode& source, INodeTab& instances) const = 0;

		/*! \brief Sets the provided material on all instances of a given object.
		 * \param [in] source The node of the object whose instances should be set up with 
		 * the specified material. For an object with multiple instances, any of the nodes 
		 * representing an instance can be specified as this parameter.
		 * \param [in] newMtl	The material to be set. If NULL, all instances of the source
		 * object will have their materials removed.
		 * \return The number of instances found. 
		*/
		virtual unsigned long SetMtlOnInstances(INode& source, Mtl* newMtl) = 0;
		
	/*! \name Automatic material propagation to instances of objects
	 * When automatic material propagation to object instances is On, materials 
	 * assigned to an object via all means (UI, maxscript, SDK) will be automatically 
	 * propagated to all instances and references of that object. No user confirmation 
	 * is be required.
	 * When Off, material assignments to an object won't get propagated to the 
	 * object's instances and references. The material of an instance can be still 
	 * propagated by calling the SetMtlOnInstances method.
	 *
	 * The initial value of automatic material propagation can be set through the 
	 * AutomaticMtlPropagation setting, in the InstanceMgr section of the 
	 * application's .ini file. When the application closes, the current value of 
	 * this settings saved in the .ini file by the system.
	 */
	//@{
		/*! \brief Retrieves state of automatic material propagation flag
		 * \return true if automatic material propagation is On, false otherwise.
		*/
		virtual bool GetAutoMtlPropagation() const = 0;

		/*! \brief Sets automatic material propagation flag
		 * \param [in] autoPropagate pass true to turn On automatic material propagation, 
		 * false otherwise.
		 * \return The previous state of the automatic material propagation flag.
		*/
		virtual bool SetAutoMtlPropagation(bool autoPropagate) = 0;
	//@}		

	/** \name Making instances unique 
	 * Methods for making base objects, derived objects, individual modifier and 
	 * transform controllers of a given set of nodes unique.
	*/
	//@{
		/*! \brief Determines whether the given objects can be made unique.
		 * \param [in] tabNodes The nodes representing the objects to be made unique.
		 * \return true if at least one object can be made unique.
		*/
		virtual bool CanMakeObjectsUnique(INodeTab& tabNodes) const = 0;

		/*! \brief Makes the given objects unique.
		 * \param [in] tabNodes The nodes representing the objects to be made unique.
		 * \param [in] iMultiNodeOption Specifies the \ref InstanceMgrMakeUniqueOptions "method" 
		 * to use when making objects unique. 
		 * \return true if at least one object was made unique
		*/
		virtual bool MakeObjectsUnique(
			INodeTab& tabNodes, 
			int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;

		/*! \brief Determines whether the given modifiers can be made unique.
		 * \param [in] tabNodes The nodes the modifiers are being applied to. 
		 * \param [in] tabMods The modifiers to be made unique.
		 * \return true if at least one modifier can be made unique.
		*/
		virtual bool CanMakeModifiersUnique(
			INodeTab& tabNodes, 
			Tab<ReferenceTarget*>& tabMods) const = 0;

		/*! \brief Makes the given modifiers unique.
		 * \param [in] tabNodes The nodes the modifiers are applied to. 
		 * \param [in] tabMods The modifiers to be made unique.
		 * \param [in] iMultiNodeOption Specifies the \ref InstanceMgrMakeUniqueOptions "method" 
		 * to use when making modifiers unique. 
		 * \return true if at least one modifier was made unique
		*/
		virtual bool MakeModifiersUnique(
			INodeTab& tabNodes, 
			Tab<ReferenceTarget*>& tabMods, 
			int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;

		/*! \brief Determines whether the given transform controllers can be made unique.
		 * \param [in] tabNodes The nodes the controller are applied to. 
		 * \param [in] tabConts The controllers to be made unique.
		 * \return true if at least one controller can be made unique.
		*/
		virtual bool CanMakeControllersUnique(
			INodeTab& tabNodes, 
			Tab<ReferenceTarget*>& tabConts) const = 0;
		
		/*! \brief Makes the given transform controllers unique.
		 * \param [in] tabNodes The nodes the controllers are applied to. 
		 * \param [in] tabConts The controllers to be made unique.
		 * \param [in] iMultiNodeOption Specifies the \ref InstanceMgrMakeUniqueOptions "method" 
		 * to use when making controllers unique. 
		 * \return true if at least one controller was made unique
		*/
		virtual bool MakeControllersUnique(
			INodeTab& tabNodes, 
			Tab<ReferenceTarget*>& tabConts, 
			int iMultiNodeOption = INSTANCE_MGR_MAKE_UNIQUE_PROMPT) const = 0;
	//@}		

	/** \name File IO 
	 * Methods for persisting the state of the instance manager.
	 */
	//@{		
		/*! \brief Saves instance manager state. This method is called by the system.
		*/
		virtual IOResult Save(ISave* isave) const = 0;

		/*! \brief Loads instance manager state. This method is called by the system.
		*/
		virtual IOResult Load(ILoad* iload) = 0;
	//@}

		/*! \brief Retrieves the single instance of the instance manager
		*/
		static IInstanceMgr* GetInstanceMgr()	{
			return static_cast<IInstanceMgr*>(GetCOREInterface(INSTANCE_MGR_INTERFACE));
		}
}; 




