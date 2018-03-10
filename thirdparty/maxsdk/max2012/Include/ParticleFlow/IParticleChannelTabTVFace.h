/*! \file IParticleChannelTabTVFace.h
    \brief Channel-generic interface for particle channels
				 that store data in Tab<TVFace> form
				 The channel can be local (each particle has its own Tab<TVFace>)
				 global (all particles have the same Tab<TVFace>) and shared
				 (particles are sharing a limited set of Tab<TVFace>)
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
#include "..\mesh.h"

#define TabTVFace Tab<TVFace>

// standard particle channel "Tab<TVFace>"
// interface ID
#define PARTICLECHANNELTABTVFACER_INTERFACE Interface_ID(0x74f93c14, 0x1eb34500)
#define PARTICLECHANNELTABTVFACEW_INTERFACE Interface_ID(0x74f93c14, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelTabTVFaceRInterface(obj) ((IParticleChannelTabTVFaceR*)obj->GetInterface(PARTICLECHANNELTABTVFACER_INTERFACE))
//#define GetParticleChannelTabTVFaceWInterface(obj) ((IParticleChannelTabTVFaceW*)obj->GetInterface(PARTICLECHANNELTABTVFACEW_INTERFACE))

class IParticleChannelTabTVFaceR : public FPMixinInterface
{
public:

      /** @defgroup IParticleChannelTabTVFace IParticleChannelTabTVFace.h
      *  @{
      */

      /*! \fn virtual bool		IsShared() const = 0;
      *  \brief Check out if some particles have shared Tab<TVFace>. If it's true then there is no need to get a Tab for each particle.
      */
	virtual bool		IsShared() const = 0;

      /*! \fn virtual int			GetValueCount() const = 0;
      *  \brief Get total number of actual Tabs (values) in the channel
      */
	virtual int			GetValueCount() const = 0;

      /*! \fn virtual int			GetValueIndex(int particleIndex) const = 0;
      *  \brief Get the value index of a particle
      */
	virtual int			GetValueIndex(int particleIndex) const = 0;

      /*! \fn virtual const TabTVFace* GetValueByIndex(int valueIndex) const = 0;
      *  \brief Get Tab of the valueIndex-th value
      */
	virtual const TabTVFace* GetValueByIndex(int valueIndex) const = 0;

      /*! \fn virtual const TabTVFace* GetValue(int particleIndex) const = 0;
      *  \brief Get Tab for particle with index
      */
	virtual const TabTVFace* GetValue(int particleIndex) const = 0;

      /*! \fn virtual const TabTVFace* GetValue() const = 0;
      *  \brief Get global Tab<TVFace> for all particles. The method returns the Tab of the first particle if it is local or shared.
      */
	virtual const TabTVFace* GetValue() const = 0;

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABTVFACER_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABTVFACER_INTERFACE); }
};

class IParticleChannelTabTVFaceW : public FPMixinInterface
{
public:

      /** @defgroup IParticleChannelTabTVFace IParticleChannelTabTVFace.h
      *  @{
      */

      /*! \fn virtual bool	SetValue(int particleIndex, const TabTVFace* value) = 0;
      *  \brief Copies Tab to be a local value for particle with index "particleIndex". Returns true if successful.
      */
	virtual bool	SetValue(int particleIndex, const TabTVFace* value) = 0;

      /*! \fn virtual bool	SetValue(Tab<int>& particleIndices, const TabTVFace* value) = 0;
      *  \brief Copies Tab to be a shared value with indices in "particleIndices". Returns true if successful.
      */
	virtual bool	SetValue(Tab<int>& particleIndices, const TabTVFace* value) = 0;

      /*! \fn virtual bool	SetValue(const TabTVFace* value) = 0;
      *  \brief Copies Tab to be a global values for all particles. Returns true if successful.
      */
	virtual bool	SetValue(const TabTVFace* value) = 0;

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

      /*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABTVFACEW_INTERFACE); }
      *  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELTABTVFACEW_INTERFACE); }
};

/*@}*/
