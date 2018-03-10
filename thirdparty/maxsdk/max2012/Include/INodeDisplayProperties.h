/**********************************************************************
*<
FILE:			INodeDisplayProperties.h

DESCRIPTION:	Declare Mixin interface class INodeDisplayProperties

CREATED BY:		Peter Watje

HISTORY:		created Jan 20, 2007

*>	Copyright (c) 2007, All Rights Reserved.
**********************************************************************/
#pragma once

#include "ifnpub.h"

//Forward decls
class INodeDisplayProperties;

// The interface ID for class INodeLayerProperties
#define NODE_DISPLAYPROPERTIES_INTERFACE Interface_ID(0x44ef2511, 0x6b071e99)

// Provides access to the nodes Max 10 display properties
/*! class INodeDisplayProperties : public FPMixinInterface\n\n

\par Description:
<b>This class is only available in release 10 or later.</b>\n\n
This class defines an interface for accessing a node's new display properties dealing with
degradation and bundling.\n\n
An instance of this interface can be retrieved using the following line of code
(assuming 'node' is of type INode*):
\code
static_cast<INodeDisplayProperties*>(node->GetInterface(NODEDISPLAYPROPERTIES_INTERFACE))


// Provides access to the nodes layer and bylayer bits
These gets/set the degradation bit which allows the user to set a node never to degrade
virtual	BOOL	GetAdaptivedegradationNeverDegrade	()		=	0;
virtual void	SetAdaptivedegradationNeverDegrade	(BOOL)	=	0;

\endcode  */
class INodeDisplayProperties : public FPMixinInterface
{
public:

	//set and get access to the bylayer bit groups
	//access to the individual bits does not seem necessary 
	virtual	BOOL	GetAdaptivedegradationNeverDegrade	()		=	0;
	virtual void	SetAdaptivedegradationNeverDegrade	(BOOL on)	=	0;
};
