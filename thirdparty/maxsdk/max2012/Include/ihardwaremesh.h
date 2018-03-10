/**********************************************************************
 *<
	FILE: IHardwareMesh.h

	DESCRIPTION: Hardware Mesh Extension Interface

	CREATED BY: Norbert Jeske

	HISTORY: Created 10/10/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "baseinterface.h"

#define IHARDWARE_MESH_INTERFACE_ID Interface_ID(0x6215327f, 0x6fb14d7a)

class IHardwareMesh : public BaseInterface
{
public:
	virtual Interface_ID	GetID() { return IHARDWARE_MESH_INTERFACE_ID; }

	// Interface Lifetime
	virtual LifetimeType	LifetimeControl() { return noRelease; }
};

