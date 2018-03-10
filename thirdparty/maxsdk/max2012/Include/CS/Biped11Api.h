//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface for Biped for Max 11
// AUTHOR: Susan Amkraut - created August 31, 2007
//***************************************************************************/

#pragma once

#include "biped10Api.h"


/*! \defgroup BipedInterface11 Biped Interface 11
Max 11 Biped export interface
*/

//! \brief Interface into the biped master controller for Max 11.
/*! This contains functions supporting new biped features in Max 11.
*/
class IBipMaster11: public IBipMaster10 
{
	public:

	/************ Access to the Biped Interface ***************/
	/*! \name Biped Interface Access
	*/
//@{
		static const ULONG I_BIPMASTER11 = 0x9171;    //!< Biped interface identifier

		//! \brief Gets the Biped interface
		//! \param[in]  anim - the Animatable whose Biped interface is returned
		//! \return the Biped interface
		BIPExport static IBipMaster11* GetBipMaster11Interface(Animatable& anim); //!< Gets the Biped interface
//@}*/

	/************ Triangle Neck ***************/
	/*! \name Triangle Neck
	If Triangle Neck is true, the clavicles are parented to the top spine link.
	If Triangle Neck is false, the clavicles are parented to the neck base.
	*/
//@{		
		//! \brief Queries the value of Triangle Neck
		//! \return true if Triangular Neck is on, false if not
		virtual const BOOL	GetTriangleNeck()=0;

		//! \brief Sets Triangle Neck
		//! \param[in]  bEnabled - sets Triangle Neck to the value of bEnabled
		virtual void	SetTriangleNeck(BOOL bEnabled)=0;
//@}
		
	/************ ForeFeet ***************/
	/*! \name ForeFeet
	If ForeFeet is true, the the fingers are shaped and attached like toes, and viewport manipulation of IK hands is like feet.
	If ForeFeet is false, the fingers and hands appear and behave as usual.
	*/
//@{		
		//! \brief Queries the value of ForeFeet
		//! \return true if ForeFeet is on, false if not
		virtual const BOOL	GetForeFeet()=0;

		//! \brief Sets ForeFeet
		//! \pre must be in figure mode
		//! \param[in]  bEnabled - sets ForeFeet to the value of bEnabled
		virtual void	SetForeFeet(BOOL bEnabled)=0;
//@}
		
	/************ Mirror In Place ***************/
	/*! \name Mirror In Place
	*/
//@{		
		//! \brief Mirrors the biped animation in place
		/*! The position and orientation of the center of mass at the current frame is maintained.
		\pre cannot be in figure mode, motion flow mode, or mixer mode
		\pre cannot be editing a biped layer
		\return true if the operation was successful, false if the operation was unsuccessful
		*/
		virtual bool	MirrorInPlace()=0;
//@}
};

