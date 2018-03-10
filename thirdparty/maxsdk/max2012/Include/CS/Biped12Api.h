//**************************************************************************/
// Copyright (c) 1998-2008 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface for Biped for Max 12
// AUTHOR: Susan Amkraut - created August 28, 2008
//***************************************************************************/

#pragma once

#include "biped11Api.h"


/*! \defgroup BipedInterface12 Biped Interface 12
Max 12 Biped export interface
*/

//! \brief Interface into the biped master controller.
/*! This contains functions supporting new biped features.
*/
class IBipMaster12: public IBipMaster11 
{
	public:

	/************ Access to the Biped Interface ***************/
	/*! \name Biped Interface Access
	*/
//@{
		static const ULONG I_BIPMASTER12 = 0x9172;    //!< Biped interface identifier

		//! \brief Gets the Biped interface
		//! \param[in]  anim - the Animatable whose Biped interface is returned
		//! \return the Biped interface
		BIPExport static IBipMaster12* GetBipMaster12Interface(Animatable& anim);
//@}*/

	/************ Knuckles ***************/
	/*! \name Knuckles
	When Knuckles is set to true, the hand is shrunk, the fingers have 4 links, splay out from the hand, and ThumbMinusOne is set to true.
	When Knuckles is set to false, the fingers and hands appear as in 3ds Max 2009, and ThumbMinusOne is set to false.
	*/
//@{		
		//! \brief Queries the value of Knuckles
		//! \return true if Knuckles is on, false if not
		virtual BOOL	GetKnuckles() const = 0;

		//! \brief Sets Knuckles
		//! \pre must be in figure mode
		//! \pre ForeFeet must be false
		//! \param[in]  bEnabled - sets Knuckles to the value of bEnabled
		virtual void	SetKnuckles(BOOL bEnabled)=0;
//@}

		
	/************ ShortThumb  ***************/
	/*! \name ShortThumb
	If ShortThumb is true, the thumb has one less link than the other fingers, except when there is only one link per finger.
	If ShortThumb is false, the thumb has the same number of links as the fingers.
	*/
//@{		
		//! \brief Queries the value of ShortThumb
		//! \return true if ShortThumb is on, false if not
		virtual BOOL	GetShortThumb() const = 0;

		//! \brief Sets ShortThumb
		//! \pre must be in figure mode
		//! \pre Knuckles must be true
		//! \param[in]  bEnabled - sets ShortThumb to the value of bEnabled
		virtual void	SetShortThumb(BOOL bEnabled)=0;
//@}

		
	/************ Layer Retargeting  ***************/
	/*! \name Layer Retargeting
	The following functions are used for retargeting biped layer animation, either to the base layer or to a reference biped.
	*/
//@{
		//! \brief Gets the layer retarget state for a limb
		//! \param[in]  keytrack - KEY_LARM or KEY_RARM or KEY_LLEG or KEY_RLEG
		//! \return true if the limb's retarget state is on, false if not
		virtual bool	GetLimbRetargetState(int keytrack)=0;

		//! \brief Sets the layer retarget state for a limb
		//! \param[in]  keytrack - KEY_LARM or KEY_RARM or KEY_LLEG or KEY_RLEG
		//! \param[in]  bEnabled - sets the limb retarget state to the value of bEnabled.
		//! \see RetargetToBaseLayer(bool IK_Only)
		//! \see RetargetToReferenceBiped(bool IK_Only)
		virtual void	SetLimbRetargetState(int keytrack, bool bEnabled)=0;

		//! \brief Gets the retarget reference biped
		//! \return the retarget reference biped if there is one, NULL if not
		virtual INode	*GetRetargetReferenceBiped()=0;
		
		//! \brief Sets the retarget reference biped
		//! \param[in]  node - sets the reference biped to the node, which must be any node from a different biped.
		//! \see RetargetToReferenceBiped(bool IK_Only)
		//! \return true if the reference biped was set correctly, false if not
		virtual bool	SetRetargetReferenceBiped(INode *node)=0;

		//! \brief Retargets the current biped layer to its base layer.
		/*! For every limb whose retarget state is active, calculates the hand/foot position at every key,
		maintaining IK constraints to the base layer.
		\pre A layer must be active
		\see SetLayerActive(int index, bool onOff)
		\param[in]  IK_Only - When IK_Only is on, the biped's constrained hands and feet are retargeted only during the frames on which they are IK controlled.
		When IK_Only is off, the hands and feet are retargeted during both IK and FK keys.
		*/
		virtual void	RetargetToBaseLayer(bool IK_Only)=0;

		//! \brief Retargets the current biped layer to the reference biped.
		/*! For every limb whose retarget state is active, calculates the hand/foot position at every key,
		maintaining IK constraints to the retarget biped.
		\pre A retarget biped must be specified.
		\see SetRetargetReferenceBiped()
		\param[in]  IK_Only - When IK_Only is on, the biped's constrained hands and feet are retargeted only during the frames on which they are IK controlled.
		When IK_Only is off, the hands and feet are retargeted during both IK and FK keys.
		*/
		virtual void	RetargetToReferenceBiped(bool IK_Only)=0;
//@}
};

