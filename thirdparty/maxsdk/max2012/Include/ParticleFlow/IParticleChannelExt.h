/*! \file IParticleChannelExt.h
    \brief Channel-generic interfaces ParticleChannelExt
				This is a part of a particle channel that
				has extended interface for advanced funtionality
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 04-15-2007

 *>	Copyright (c) Orbaz Technologies, Inc. 2007, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "..\ifnpub.h"

// interface ID
#define PARTICLECHANNELEXT_INTERFACE Interface_ID(0x12dd1705, 0x1eb34400)

#define GetParticleChannelExtInterface(obj) ((IParticleChannelExt*)obj->GetInterface(PARTICLECHANNELEXT_INTERFACE))


class IParticleChannelExt : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kCopy
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kCopy, TYPE_bool, Copy, TYPE_IOBJECT);

	END_FUNCTION_MAP

	virtual bool	Append(IObject* obj) = 0; 


      /** @defgroup IParticleChannelExt IParticleChannelExt.h
      *  @{
      */

      /*! \fn virtual bool	Copy(IObject* obj) = 0; 
      *  \brief Copy all particle data from another channel of the same type.
	   Returns true if the operation is successful.
      */
	virtual bool	Copy(IObject* obj) = 0; 

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELEXT_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELEXT_INTERFACE); }
};

