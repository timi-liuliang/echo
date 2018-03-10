/*==============================================================================
Copyright 2010 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement provided at the time of installation or download,
or which otherwise accompanies this software in either electronic or hard copy form.   
*****************************/
// DESCRIPTION: Interface to AutoTangent Manager
// AUTHOR: Michael Zyracki created 2010
//***************************************************************************/
#pragma once

#include "ifnpub.h"
#include "Control.h"

/** The unique interface ID for the Auto Tangent Manager */
#define IAUTOTANGENTMAN_INTERFACE Interface_ID(0x13383fe1, 0x326579c6)

/** Manages controller auto-tangent related options.
This interface class allows for querying controllers for their auto-tangent support 
and the auto-tangent algorithm they use.
3ds Max 2012 introduces an improved auto tangent algorithm which is consistent 
with the ones used in other Autodesk products. IAutoTangentMan allows to determine 
whether or not the legacy or the new auto tangent algorithm is used by 3ds Max 
controllers that implement support for auto tangents, such as the Bezier float 
and Bezier point3 controllers.
Client code can get access to IAutoTangentMan using the following code: 
\code
static_cast<IAutoTangentMan*>( GetCOREInterface(IAUTOTANGENTMAN_INTERFACE ) );
\endcode
*/
class IAutoTangentMan: public FPStaticInterface
{
public:
	/** The auto-tangent algorithms supported by 3ds Max. */
	enum AutoTangentAlgorithm
	{ 
		eNotSupported = -1, //!< The specified controller doesn't support auto tangents
		eLegacyAlgorithm=0, //!< The pre-3ds Max 2012 auto tangent algorithm 
		eUnifiedAlgorithm=1 //!< The auto-tangent algorithm being used across Autodesk products
	};
 
	/** Returns whether a particular controller supports auto-tangents.
	\param[in] controller The controller to check
	\return Returns true if the controller supports auto-tangents, for example a Bezier controller. */
	virtual bool SupportsAutoTangent(Control *controller)const = 0;

	/** Gets the auto-tangent algorithm used by a given controller.
	\param[in] controller  The controller to query
	\return Returns the auto-tangent algorithm being used. If the controller doesn't 
	support auto tangents then eNotSupported is returned. */
	virtual AutoTangentAlgorithm GetAutoTangentAlgorithm(Control *controller)const = 0;

	/** Sets the auto-tangent algorithm to be used by a given controllers.
	\param[in] controllers The list of controllers.
	\param[in] algorithm  The auto-tangent algorithm to set. eNotSuported is not a 
	valid input.  Note that if the controller doesn't support the auto tangent 
	algorithm, no algorithm is set on that controller, effectively setting its type 
	as eNotSupported. */
	virtual void SetAutoTangentAlgorithm(AutoTangentAlgorithm algorithm,Tab<Control*> &controllers)=0;

	/** Gets the default auto-tangent algorithm that will be used when new controllers 
	that support auto tangents are created.
	\return Returns the default auto-tangent algorithm. */
	virtual AutoTangentAlgorithm GetDefaultControllerAutoTangentAlgorithm()const = 0;

	/** Sets the default auto-tangent algorithm that will be used when new controllers 
	that support auto tangents are created. 
	\param[in] algorithm  The default auto-tangent algorithm to be used. eNotSupported is not a valid input. */
	virtual void SetDefaultControllerAutoTangentAlgorithm(AutoTangentAlgorithm algorithm) = 0;
};

