/*! \file IParticleChannelNew.h
    \brief Channel-specific interfaces for ParticleChannelNew.
	The channel is used to mark particles that have just
	come to the current event (either via birth or a jump from
	another event 
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-03-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "..\ifnpub.h"

// standard particle channel "New"
// interface ID
#define PARTICLECHANNELNEWR_INTERFACE Interface_ID(0x74f93b01, 0x1eb34500) 
#define PARTICLECHANNELNEWW_INTERFACE Interface_ID(0x74f93b01, 0x1eb34501) 

#define GetParticleChannelNewRInterface(obj) ((IParticleChannelNewR*)obj->GetInterface(PARTICLECHANNELNEWR_INTERFACE)) 
#define GetParticleChannelNewWInterface(obj) ((IParticleChannelNewW*)obj->GetInterface(PARTICLECHANNELNEWW_INTERFACE)) 

// function IDs Read
enum {	particleChannelNew_isNew,
		particleChannelNew_isAllNew,
		particleChannelNew_isAllOld
}; 

// function IDs Write
enum {	particleChannelNew_setNew, 
		particleChannelNew_setOld,
		particleChannelNew_setAllNew,
		particleChannelNew_setAllOld
}; 

class IParticleChannelNewR : public FPMixinInterface
{

BEGIN_FUNCTION_MAP

FN_1(particleChannelNew_isNew, TYPE_bool, IsNew, TYPE_INT);
FN_0(particleChannelNew_isAllNew, TYPE_bool, IsAllNew);
FN_0(particleChannelNew_isAllOld, TYPE_bool, IsAllOld);

END_FUNCTION_MAP

public:

    /** @defgroup IParticleChannelNew IParticleChannelNew.h
    *  @{
    */

	// get particle property
	/*!  \fn virtual bool	IsNew(int index) const = 0;
	*  \brief Checks if particle with index is new
	*/
	virtual bool	IsNew(int index) const = 0; // is particle with the index is new

	/*!  \fn virtual bool	IsAllNew() const = 0;
	*  \brief Checks if all particles are new
	*/
	virtual bool	IsAllNew() const = 0; // all particles are new

	/*!  \fn virtual bool	IsAllOld() const = 0;
	*  \brief Checks if all particles are old
	*/
	virtual bool	IsAllOld() const = 0; // all particles are old

	/*!  \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELNEWR_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELNEWR_INTERFACE); }
};

class IParticleChannelNewW : public FPMixinInterface
{

BEGIN_FUNCTION_MAP

VFN_1(particleChannelNew_setNew, SetNew, TYPE_INT);
VFN_1(particleChannelNew_setOld, SetOld, TYPE_INT);
VFN_0(particleChannelNew_setAllNew, SetAllNew);
VFN_0(particleChannelNew_setAllOld, SetAllOld);

END_FUNCTION_MAP

public:

    /** @defgroup IParticleChannelNew IParticleChannelNew.h
    *  @{
    */

	// get/set particle property
	/*!  \fn virtual void	SetNew(int index) = 0;
	*  \brief Sets particle as new, returns true if success
	*/
	virtual void	SetNew(int index) = 0; 

	/*!  \fn virtual void	SetOld(int index) = 0;
	*  \brief Sets particle as old, returns true if success 
	*/
	virtual void	SetOld(int index) = 0; 

	/*!  \fn virtual void	SetAllNew() = 0;
	*  \brief Sets all particles as "new"
	*/
	virtual void	SetAllNew() = 0; 

	/*!  \fn virtual void	SetAllOld() = 0;
	*  \brief Sets all particles as "old"
	*/
	virtual void	SetAllOld() = 0; 

	/*!  \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELNEWW_INTERFACE); }
	*  \brief 
	*/

	/*!  \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELNEWW_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELNEWW_INTERFACE); }
};


