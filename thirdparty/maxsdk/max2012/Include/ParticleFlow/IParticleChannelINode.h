/*! \file IParticleChannelINode.h
    \brief	Channel-generic interfaces for particle channels that store pointers on INode objects.
*/
/**********************************************************************
 *<
	CREATED BY:		Oleg Bayborodin

	HISTORY:		created 07-08-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "INodePtr"
// interface ID
#define PARTICLECHANNELINODER_INTERFACE Interface_ID(0x74f93c09, 0x1eb34500)
#define PARTICLECHANNELINODEW_INTERFACE Interface_ID(0x74f93c09, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelINodeRInterface(obj) ((IParticleChannelINodeR*)obj->GetInterface(PARTICLECHANNELINODER_INTERFACE))
//#define GetParticleChannelINodeWInterface(obj) ((IParticleChannelINodeW*)obj->GetInterface(PARTICLECHANNELINODEW_INTERFACE))


class IParticleChannelINodeR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetValue,
			kIsGlobal,
			kGetValueGlobal
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetValue, TYPE_INODE, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_INODE, GetValue);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelINode IParticleChannelINode.h
    *  @{
    */

    	/*! \fn virtual INode*		GetValue(int index) const = 0;
    	*  \brief get property for particle with index
      */
	virtual INode*		GetValue(int index) const = 0;

    	/*! \fn virtual bool	IsGlobal() const = 0;
    	*  \brief verify if the channel is global
      */
	virtual bool	IsGlobal() const = 0;

    	/*! \fn virtual INode*		GetValue() const = 0;
    	*  \brief if channel is global returns the global value. if channel is not global returns value of the first particle.
      */
	virtual INode*		GetValue() const = 0;

    	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINODER_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINODER_INTERFACE); }
};

class IParticleChannelINodeW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_INODE);
	VFN_1(kSetValueGlobal, SetValue, TYPE_INODE);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelINode IParticleChannelINode.h
    *  @{
    */

    	/*! \fn virtual void	SetValue(int index, INode* value) = 0;
    	*  \brief set property for particle with index
      */
	virtual void	SetValue(int index, INode* value) = 0;

    	/*! \fn virtual void	SetValue(INode* value) = 0;
    	*  \brief set property for all particles at once thus making the channel global
      */
	virtual void	SetValue(INode* value) = 0;

    	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINODEW_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELINODEW_INTERFACE); }
};

