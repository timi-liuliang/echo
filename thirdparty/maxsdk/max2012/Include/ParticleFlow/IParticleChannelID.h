/*! \file IParticleChannelID.h
    \brief Channel-specific interfaces for ParticleChannelID. The channel is used to identify particles.
				 Particle ID consists of two integers: particleIndex and
				 particleBorn. ParticleIndex gives relative correspondense
				 to the whole amount of particles. If PF ParticleAmountMultiplier
				 is set to 100% then the given particleIndex are successive 
				 ordinal numbers. If it's set to 50% then the given particle indeces
				 are 0, 2, 4, 6 etc. If it is a Birth by Total Number then the
				 last particle born has an index of the total number whatever
				 multiplier is. ParticleBorn number always are successive
				 ordinal numbers when particles were born: 0, 1, 2, 3 etc.
				 If ParticleAmountMultiplier equals 100% then for a 
				 particular particle particleIndex and particleBorn are the
				 same number. If ParticleAmountMultiplier is greater then
				 100% then you may have several particles with the same
				 particleIndex. It is recommended to link non-random properties to
				 particleIndex and random properties to particleBorn.

*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 10-04-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "..\maxheap.h"
#include "..\ifnpub.h"


// standard particle channel "particleID"
// interface ID
#define PARTICLECHANNELIDR_INTERFACE Interface_ID(0x74f93b02, 0x1eb34500) 
#define PARTICLECHANNELIDW_INTERFACE Interface_ID(0x74f93b02, 0x1eb34501) 

#define GetParticleChannelIDRInterface(obj) ((IParticleChannelIDR*)obj->GetInterface(PARTICLECHANNELIDR_INTERFACE)) 
#define GetParticleChannelIDWInterface(obj) ((IParticleChannelIDW*)obj->GetInterface(PARTICLECHANNELIDW_INTERFACE)) 

// function IDs Read
enum {	particleChannelID_getID, 
		particleChannelID_getParticleIndex,
		particleChannelID_getParticleBorn
}; 

// function IDs Write
enum {	particleChannelID_setID
}; 

struct ParticleID: public MaxHeapOperators {
	int index;
	int born;
};

class IParticleChannelIDR : public FPMixinInterface
{

BEGIN_FUNCTION_MAP

VFN_3(particleChannelID_getID, GetID, TYPE_INT, TYPE_INT_BR, TYPE_INT_BR);
FN_1(particleChannelID_getParticleIndex, TYPE_INT, GetParticleIndex, TYPE_INT);
FN_1(particleChannelID_getParticleBorn, TYPE_INT, GetParticleBorn, TYPE_INT);

END_FUNCTION_MAP

public:

    /** @defgroup IParticleChannelID IParticleChannelID.h
    *  @{
    */

    /*! \fn virtual void	GetID(int index, int& particleIndex, int& particleBorn) const = 0;
    *  \brief 
    */
	virtual void	GetID(int index, int& particleIndex, int& particleBorn) const = 0;

    /*! \fn virtual int		GetParticleIndex(int index) const = 0; 
    *  \brief Returns particleIndex
    */
	virtual int		GetParticleIndex(int index) const = 0; // returns particleIndex

    /*! \fn virtual int		GetParticleBorn(int index) const = 0; 
    *  \brief Returns particleBorn
    */
	virtual int		GetParticleBorn(int index) const = 0; // returns particleBorn

    /*! \fn virtual const ParticleID& GetID(int index) const = 0; 
    *  \brief Methods not in the mapped interface
    */
	virtual const ParticleID& GetID(int index) const = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELIDR_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELIDR_INTERFACE); }
};

class IParticleChannelIDW : public FPMixinInterface
{

BEGIN_FUNCTION_MAP

VFN_3(particleChannelID_setID, SetID, TYPE_INT, TYPE_INT, TYPE_INT);

END_FUNCTION_MAP

public:

    /** @defgroup IParticleChannelID IParticleChannelID.h
    *  @{
    */

    /*! \fn virtual void SetID(int index, int particleIndex, int particleBorn) = 0;
    *  \brief Set particle property
    */
	virtual void SetID(int index, int particleIndex, int particleBorn) = 0;

    /*! \fn virtual void SetID(int index, const ParticleID& id) = 0; 
    *  \brief Methods not in the mapped interface
    */
	virtual void SetID(int index, const ParticleID& id) = 0;

    /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELIDW_INTERFACE); }
    *  \brief 
    */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELIDW_INTERFACE); }
};

/*@}*/

