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
// DESCRIPTION: Interface for Biped for Max 9
// AUTHOR: Michael Zyracki - created March 16, 2006
//***************************************************************************/

#pragma once

#include "biped8Api.h"


/*! \defgroup BipedInterface9 Biped Interface 9
Max 9 Biped export interface
*/
//@{
#define I_BIPMASTER9	0x9168    //!< Biped interface identifier
#define GetBipMaster9Interface(anim) ((IBipMaster9*)(anim)->GetInterface(I_BIPMASTER9))	//!< Gets the Biped interface
//@}

//! \brief Interface into the biped master controller for Max 9.
/*! This contains functions supporting new biped features in Max 9, in particularly 
new functionality for setting large number of keys onto a biped at once, e.g. by loadin 
an XAF file.
*/
class IBipMaster9: public IBipMaster8 
{
	public:

	/*! \name Setting Blocks of Biped keys.

	The following functions are used for setting a collection of biped keys all at once.  Due to how biped works internally,
	the best way to set the keys on the biped is a pose per each frame, instead of setting keys over each body part individually.
	Since it's easier for many systems to set a key over each part, one at a time, we've added these functions that basically
	cache the values and then set them per pose per frame.  This also allows us to perform some optimizations to speed up these
	operations which can be slow due to the number of change notifications.   The workflow is thus, call StartSettingBipedKeys, set
	the world and/or local values for the limbs using SetBipedTM or SetBipedLocalRotation, and then call 	StopSettingBipedKeys,
	which will then go through and set the keys pose per frame.  Note that these functions are used when loading an XAF file onto a
	biped.
	*/

	//@{

	//! \brief Function for specifying that we will be setting a group of biped keys.
	
	//! Calling this function means that biped will soon get a large number of keys set on different biped body parts, usually via a
	//! SetNodeTM function call.  Internally, this will allow biped to correctly set the keys based upon biped peculiarities, for example
	//! we need to set the biped up all at once per each frame that has a key (per pose), and we need to handle shared keys caused by 
	//! unseperated tracks.
	virtual void StartSettingBipedKeys() = 0;


	//! \brief Function for specifying that we will are done setting a group of biped keys.

	//! Calling this function means that the biped is done setting keys on it, and we can now go ahead and key the biped
	//! based upon all of the values that were set after the previous StartSettingBipedKeys call.  Note that if no StartSettingBipedKeys
	//! function was called, then nothing happens.
	virtual void StopSettingBipedKeys() = 0;

	//! \brief Function for setting world transforms on a biped.
	
	//! This function will set the world transform on a biped, similarly to INode::GetNodeTM.  This function however will cache values based
	//! upon biped body part, when called within StartSettingBipedSetKeys/StopSettingBipedKeys().  This allows us to correctly and optimally
	//! set up a bipeds animation pose per pose.
	//! \param[in] t The time at which to set the pose.
	//! \param[in] id The internal 'id' of the biped node.  You get this by calling IBipMaster::GetIdLink.
	//! \param[in] id The internal 'link' of the biped node.  You get this by calling IBipMaster::GetIdLink.
	virtual void SetBipedTM(TimeValue t,Matrix3 &mat,int id, int link) = 0;

	//! \brief Function for setting local biped rotation values.

	//! This function will set the a local rotation on the biped, in max's local coordinate space (not biped's own internal local
	//! coordinate space.)  Also this function works within StartSettingBipedKeys and StopSettingBipedKeys for correctly setting the biped
	//! rotations locally, pose per pose.
	//! \param[in] t The time to set the rotation.
	//! \param[in] q The local rotation.
	//! \param[in] id The internal 'id' of the biped node.  You get this by calling IBipMaster::GetIdLink.
	//! \param[in] link The internal 'link' of the biped node.  You get this by calling IBipMaster::GetIdLink.
	virtual void SetBipedLocalRotation(TimeValue t,Quat &q, int id, int link) = 0;
	//@}
};

