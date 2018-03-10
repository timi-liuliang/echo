 /**********************************************************************
 
	FILE: IPipelineClient.h

	DESCRIPTION:  Geometry pipeline client API

	CREATED BY: Attila Szabo, Discreet

	HISTORY: [attilas|19.09.2000]


 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "baseinterface.h"
#include "channels.h"	

// GUID that identifies this ifc (interface)
#define PIPELINECLIENT_INTERFACE Interface_ID(0x62383d51, 0x2d0f7d6a)

// This interface should be implemented by objects that flow up the 
// geometry pipeline and have data members that belong to the pipeline
// channels (geometry, topology, texmap, etc)
// ... in other words by objects that are part of the data flow evaluation 
// of the Max stack. 
class IPipelineClient : public BaseInterface
{
	public:
		// --- IPipelineClient methods
		virtual void	ShallowCopy( IPipelineClient* from, ChannelMask channels ) = 0;
		virtual void	DeepCopy( IPipelineClient* from, ChannelMask channels ) = 0;
		virtual void	NewAndCopyChannels( ChannelMask channels ) = 0;
		virtual void	FreeChannels( ChannelMask channels, int zeroOthers = 1 ) = 0;
		virtual void	ZeroChannels( ChannelMask channels ) = 0;
		virtual void	AppendAllChannels( IPipelineClient* from ) = 0;

		// --- from BaseInterface
		virtual Interface_ID	GetID() { return PIPELINECLIENT_INTERFACE; }
};

