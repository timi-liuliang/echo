/**********************************************************************
 *<
	FILE:			INodeLayerProperties.h

	DESCRIPTION:	Declare Mixin interface class INodeLayerProperties
											 
	CREATED BY:		John Hutchinson

	HISTORY:		created Aug 6, 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"

//Forward decls
class ILayerProperties;

// The interface ID for class INodeLayerProperties
#define NODELAYERPROPERTIES_INTERFACE Interface_ID(0x44e025f8, 0x6b071e44)

// Provides access to the nodes layer and bylayer bits
/*! class INodeLayerProperties : public FPMixinInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This class defines an interface for accessing a node's global illumination
properties.\n\n
An instance of this interface can be retrieved using the following line of code
(assuming 'node' is of type INode*):
\code
static_cast<INodeGIProperties*>(node->GetInterface(NODELAYERPROPERTIES_INTERFACE))
#define NODELAYERPROPERTIES_INTERFACE Interface_ID(0x44e025f8,0x6b071e44)

// Provides access to the nodes layer and bylayer bits
virtual ILayerProperties* getLayer (void) = 0;
virtual void setLayer (FPInterface *) = 0;

virtual BOOL getDisplayByLayer () = 0;
virtual void setDisplayByLayer (BOOL) = 0;
virtual BOOL getRenderByLayer () = 0;
virtual void setRenderByLayer (BOOL) = 0;
virtual BOOL getMotionByLayer () = 0;
virtual void setMotionByLayer (BOOL) = 0;
virtual BOOL getColorByLayer () = 0;
virtual void setColorByLayer (BOOL) = 0;
virtual BOOL getGlobalIlluminationByLayer () = 0;
virtual void setGlobalIlluminationByLayer (BOOL) = 0;
\endcode  */
class INodeLayerProperties : public FPMixinInterface
{
public:
	//set and get access to the nodes layer
	virtual ILayerProperties*	getLayer	(void)			=	0;
	virtual void				setLayer	(FPInterface *)	=	0;

	//set and get access to the bylayer bit groups
	//access to the individual bits does not seem necessary 
	virtual	BOOL	getDisplayByLayer	()		=	0;
	virtual void	setDisplayByLayer	(BOOL)	=	0;
	virtual BOOL	getRenderByLayer	()		=	0;
	virtual void	setRenderByLayer	(BOOL)	=	0;
	virtual BOOL	getMotionByLayer	()		=	0;
	virtual void	setMotionByLayer	(BOOL)	=	0;
	virtual BOOL	getColorByLayer	()		=	0;
	virtual void	setColorByLayer	(BOOL)	=	0;
	virtual BOOL	getGlobalIlluminationByLayer	()		=	0;
	virtual void	setGlobalIlluminationByLayer	(BOOL)	=	0;
};
