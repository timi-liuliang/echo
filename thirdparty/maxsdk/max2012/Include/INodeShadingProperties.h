/**********************************************************************
*<
FILE:			INodeShadingProperties.h

DESCRIPTION:	Core interface to control node based viewport shading properties. 
				This is only useful for lights

CREATED BY:		Neil Hazzard

HISTORY:		created March 8th, 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/

#pragma once

#include "ifnpub.h"

#define NODESHADINGPROPERTIES_INTERFACE Interface_ID(0x7acd13f1, 0x2b2a128d)

//! A node based interface to support new Viewport Rendering features
/*! The new interface for 3ds max 10 provides access to the new viewport rendering features. This 
includes the ability for any number of lights to illuminate the scene.  A light can now cast shadows in the 
viewport and provide illumination which are properties stored at the node level.  The IViewportShadingMgr 
interacts with this interface to give the user some control over these light node properties.  These properties
only have an effect if Viewport Shading is turned on.
The developer can access this interface directly from the node.
\code
INodeShadingProperties * shadeProp = static_cast<INodeShadingProperties*>(node->GetInterface(NODESHADINGPROPERTIES_INTERFACE));
\endcode
*/
class INodeShadingProperties : public FPMixinInterface
{
	public:
		/*! Retrieves the shadow casting flag.
		\return TRUE if it is shadow casting, FALSE if this is turned off
		*/
		virtual bool GetViewportShadowCasting() = 0;

		/*! Set the shading casting flag
		\param bSet Pass TRUE to turn on shadow casting, FALSE to turn it off.
		*/
		virtual void SetViewportShadowCasting(bool bSet) =0;

		/*! Retrieves the illumination flag.
		\return TRUE if it is turned on for illumination, FALSE if this is turned off
		*/
		virtual bool GetViewportIlluminate() =0;

		/*! Set the illuminate flag
		\param bSet Pass TRUE to turn on illumination, FALSE to turn it off.
		*/
		virtual void SetViewportIlluminate(bool bSet) =0;

		//! Retrieves the locked state of the light.
		/*! A locked light means that it is not affected by changes of other properties.  For example
		if the user locks the sun light, it will not be affected by a group "turn of shadows".
		\return The locked lighting flag
		*/
		virtual bool GetLockedViewportIllumination() =0;

		//! Sets the locked flag for the light node
		/*! A locked light means that it is not affected by changes of other properties.  For example
		if the user locks the sun light, it will not be affected by a group "turn of shadows".
		\param bSet TRUE to turn it on, FALSE to turn it off
		*/
		virtual void SetLockedViewportIllumination(bool bSet) =0;

};

