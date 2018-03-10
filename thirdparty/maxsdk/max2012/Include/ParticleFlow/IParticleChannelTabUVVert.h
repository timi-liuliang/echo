/*! \file IParticleChannelTabUVVert.h
    \brief Channel-generic interface for particle channels
				 that store data in Tab<UVVert> form
				 The channel can be local (each particle has its own Tab<UVVert>)
				 global (all particles have the same Tab<UVVert>) and shared
				 (particles are sharing a limited set of Tab<UVVert>)
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-19-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "..\tab.h"
#include "..\point3.h"

#define TabUVVert Tab<UVVert>

// standard particle channel "Tab<UVVert>"
// interface ID
#define PARTICLECHANNELTABUVVERTR_INTERFACE Interface_ID(0x74f93c14, 0x1eb34500)
#define PARTICLECHANNELTABUVVERTW_INTERFACE Interface_ID(0x74f93c14, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelTabUVVertRInterface(obj) ((IParticleChannelTabUVVertR*)obj->GetInterface(PARTICLECHANNELTABUVVERTR_INTERFACE))
//#define GetParticleChannelTabUVVertWInterface(obj) ((IParticleChannelTabUVVertW*)obj->GetInterface(PARTICLECHANNELTABUVVERTW_INTERFACE))

class IParticleChannelTabUVVertR : public FPMixinInterface
{
public:
      /** @defgroup IParticleChannelTabUVVert IParticleChannelTabUVVert.h
      *  @{
      */

      /*! \fn virtual bool		IsShared() const = 0;
      *  \brief Check out if some particles have shared Tab<UVVert>.
	If it's true, then there is no need to get a Tab for each particle.
      */
	virtual bool		IsShared() const = 0;

      /*! \fn virtual int			GetValueCount() const = 0;
      *  \brief Get total number of actual Tabs (values) in the channel.
      */
	virtual int			GetValueCount() const = 0;

      /*! \fn virtual int			GetValueIndex(int particleIndex) const = 0;
      *  \brief Get the value index of a particle.
      */
	virtual int			GetValueIndex(int particleIndex) const = 0;

      /*! \fn virtual const TabUVVert* GetValueByIndex(int valueIndex) const = 0;
      *  \brief Get Tab of the valueIndex-th value
	virtual const TabUVVert* GetValueByIndex(int valueIndex) const = 0;
      */

      /*! \fn virtual const TabUVVert* GetValue(int particleIndex) const = 0;
      *  \brief Get Tab for particle with index
      */
	virtual const TabUVVert* GetValue(int particleIndex) const = 0;

      /*! \fn virtual const TabUVVert* GetValue() const = 0;
      *  \brief Get global Tab<UVVert> for all particles.
	The method returns the Tab of the first particle if it is local or shared.
      */
	virtual const TabUVVert* GetValue() const = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABUVVERTR_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABUVVERTR_INTERFACE); }
};

class IParticleChannelTabUVVertW : public FPMixinInterface
{
public:
      /** @defgroup IParticleChannelTabUVVert IParticleChannelTabUVVert.h
      *  @{
      */

      /*! \fn virtual bool	SetValue(int particleIndex, const TabUVVert* value) = 0;
      *  \brief Copies Tab to be a local value for particle with index "particleIndex".
	Returns true if successful.
      */
	virtual bool	SetValue(int particleIndex, const TabUVVert* value) = 0;

      /*! \fn virtual bool	SetValue(int particleIndex, UVVert value) = 0;
      *  \brief Creates a Tab of size one from the UVVert value and
	copies the Tab to be a local value for particle with index "particleIndex".
	Returns true if successful
      */
	virtual bool	SetValue(int particleIndex, UVVert value) = 0;

      /*! \fn virtual bool	SetValue(Tab<int>& particleIndices, const TabUVVert* value) = 0;
      *  \brief Copies Tab to be a shared value with indices in "particleIndices".
	Returns true if successful.
      */
	virtual bool	SetValue(Tab<int>& particleIndices, const TabUVVert* value) = 0;

      /*! \fn virtual bool	SetValue(Tab<int>& particleIndices, UVVert value) = 0;
      *  \brief Creates a Tab of size one from the UVVert value and
	copies the Tab to be a shared value with indices in "particleIndices".
	Returns true if successful.
      */
	virtual bool	SetValue(Tab<int>& particleIndices, UVVert value) = 0;

      /*! \fn virtual bool	SetValue(const TabUVVert* value) = 0;
      *  \brief Copies Tab to be a global values for all particles.
	Returns true if successful
      */
	virtual bool	SetValue(const TabUVVert* value) = 0;

      /*! \fn virtual bool	SetValue(UVVert value) = 0;
      *  \brief Creates a Tab of size one from the UVVert value and
	copies the Tab to be a global values for all particles.
	Returns true if successful.
      */
	virtual bool	SetValue(UVVert value) = 0;

      /*! \fn virtual bool	CopyValue(int fromParticle, int toParticle) = 0;
      *  \brief Copy tab value from fromParticle to toParticle
      */
	virtual bool	CopyValue(int fromParticle, int toParticle) = 0;

      /*! \fn virtual bool	CopyValue(int fromParticle, Tab<int>& toParticles) = 0;
      *  \brief Copy tab value from fromParticle to toParticles
      */
	virtual bool	CopyValue(int fromParticle, Tab<int>& toParticles) = 0;

      /*! \fn virtual bool	CopyValue(int fromParticle) = 0;
      *  \brief Copy tab value from fromParticle to all particles
      */
	virtual bool	CopyValue(int fromParticle) = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABUVVERTW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABUVVERTW_INTERFACE); }

/*@}*/

};

