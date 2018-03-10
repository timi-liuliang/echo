/**********************************************************************
FILE: ITrackSet.h

DESCRIPTION:	
	Interfaces for track sets

HISTORY:
	leonarni|Feb.23.2005 - creation

	Copyright (c) 2005, All Rights Reserved.
**********************************************************************/

#pragma once

#include "iFnPub.h"
// forward declarations
class Animatable;
class INode;

//! Id for IAnimTrack interface
#define IID_ANIM_TRACK Interface_ID(0x481f0fff, 0x520478f5)

//! \brief Interface of a parameter track object, a member of a track set.
/*!
	The IAnimTrack interface provides access to a member of a track set and   
	represents one track in the track view, for instance, the X position of a   
	node.  A track is defined by a node and then a path of subanims, from the   
	node down to a leaf controller.  For instance the track for the X position    
	of a node would keep the subAnims of the PRS control, then of the indePos 
	and of the X position.  At the moment only tracks that are under a node 
	can belong to a track set.
*/
class IAnimTrack: public FPMixinInterface
{
public:
	// Function Publishing System
	enum {
		get_node,
		sub_anim_count,
		get_sub_anim,
		get_sub_anim_index,
		get_path_name,
		set
	};

public:
	//! Returns the node to which the parameter track belongs 
	//! \return - the node to which the parameter belongs
	virtual INode* GetNode() const = 0;
	//! Returns the number of subanims for this track, from the node to 
	//! the leaf controller
	//! \return - the number of subanims in this track
	virtual int SubAnimCount() const = 0;
	//! Returns the desired subanim.  For instance GetSubAnim(0) on the 
	//! track "Cone01\Transform\Position\X Position" would return the PRS 
	//! controller under Cone01
	//! \param[in] index - zero-based index of the subanim
	virtual Animatable* GetSubAnim(int index) const = 0;
	//! Returns the desired subanim index.  For instance GetSubAnim(0) on the 
	//! track "Cone01\Transform\Position\X Position" would return the subanim 
	//! index of the PRS controller under Cone01
	//! \param[in] index - zero-based index of the subanim
	virtual int GetSubAnimIndex(int index) const = 0;
	//! Returns a string describing the track, e.g. "Cone01\Transform\Position\X Position"
	//! \return - a string describing the track
	virtual const MSTR& GetPathName() const = 0;
	//! Sets the animation track from a string.  The string will be parsed
	//! as a node followed with a backslash-separated list of subanims
	//! \param[in] pathname - string describing the track
	//! \see IAnimTrack::GetSubAnim()
	virtual void Set(MCHAR* pathname) = 0;

	//! Retrieves the leaf controller of a track 
	//! \return - Pointer to the controller or NULL
	virtual Control* GetLeafController() const = 0;

	using FPMixinInterface::GetInterface;
	//! Retrieves the IAnimTrack interface from an object 
	//! \param[in] is - Pointer to an object
	//! \return - Pointer to the IAnimTrack interface of the object or NULL
	//! if the object does not support the animation track interface
	static IAnimTrack* GetInterface(InterfaceServer& is) {
		return static_cast<IAnimTrack*>(is.GetInterface(IID_ANIM_TRACK));
	};

	// From FPMixinInterface
	FPInterfaceDesc* GetDesc();		// Must implement
};

