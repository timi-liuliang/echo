/*! \file IParticleChannelMap.h
    \brief Channel-generic interface for particle channels
				 that store data in "map" form
				 A map consists of Tab<UVVert> and Tab<TVFace> information
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 06-17-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
#include "IParticleChannelTabUVVert.h"
#include "IParticleChannelTabTVFace.h"

// interface for map channel; Mesh Map object may contain several map channels
// interface ID
#define PARTICLECHANNELMAPR_INTERFACE Interface_ID(0x74f93c16, 0x1eb34500)
#define PARTICLECHANNELMAPW_INTERFACE Interface_ID(0x74f93c16, 0x1eb34501)

#define GetParticleChannelMapRInterface(obj) ((IParticleChannelMapR*)obj->GetInterface(PARTICLECHANNELMAPR_INTERFACE))
#define GetParticleChannelMapWInterface(obj) ((IParticleChannelMapW*)obj->GetInterface(PARTICLECHANNELMAPW_INTERFACE))

class IParticleChannelMapR : public FPMixinInterface
{
public:
    /** @defgroup IParticleChannelMap IParticleChannelMap.h
    *  @{
    */

	/*! \fn bool		IsShared() const { return (IsUVVertShared() || IsTVFaceShared()); }
	*  \brief check out if some particles have shared mapping data. if it's true then there is no need to get a mapping for each particle
	*/
	bool		IsShared() const { return (IsUVVertShared() || IsTVFaceShared()); }

	/*! \fn virtual bool IsUVVertShared() const = 0;
	*  \brief check out if some particles have shared UVVert data. if it's true then there is no need to get a UVVert data for each particle
	*/
	virtual bool IsUVVertShared() const = 0;

	/*! \fn virtual bool IsTVFaceShared() const = 0;
	*  \brief check out if some particles have shared TVFace data. if it's true then there is no need to get a TVFace data for each particle
	*/
	virtual bool IsTVFaceShared() const = 0;

	/*! \fn virtual int GetUVVertCount() const = 0;
	*  \brief get total number of actual TabUVVert values in the channel
	*/
	virtual int GetUVVertCount() const = 0;

	/*! \fn virtual int GetUVVertIndex(int particleIndex) const = 0;
	*  \brief get the TabUVVert value index of a particle
	*/
	virtual int GetUVVertIndex(int particleIndex) const = 0;

	/*! \fn virtual const TabUVVert* GetUVVertByIndex(int valueIndex) const = 0;
	*  \brief get TabUVVert of the valueIndex-th value
	*/
	virtual const TabUVVert* GetUVVertByIndex(int valueIndex) const = 0;

	/*! \fn virtual const TabUVVert* GetUVVert(int particleIndex) const = 0;
	*  \brief get TabUVVert of a particle with the specified index
	*/
	virtual const TabUVVert* GetUVVert(int particleIndex) const = 0;

	/*! \fn virtual const TabUVVert* GetUVVert() const = 0;
	*  \brief get global TabUVVert value for all particles. the method returns the TabUVVert value of the first particle if it is local or shared
	*/
	virtual const TabUVVert* GetUVVert() const = 0;

	/*! \fn virtual int GetTVFaceCount() const = 0;
	*  \brief get total number of actual TabTVFace values in the channel
	*/
	virtual int GetTVFaceCount() const = 0;

	/*! \fn virtual int GetTVFaceIndex(int particleIndex) const = 0;
	*  \brief may return NULL. for different TabUVVert values it has different meanings.
	if TabUVVert has a single UVVert value then the particle has a planar TVFace data and all vertices of the particles 
	have the same UVVert value. if TabUVVert has the same number of UVVert values as the particle mesh vertices number
	then the particle has a planar TVFace data and there is one-to-one correspondence between UVVerts and vertices of the particle.
	get the TabTVFace value index of a particle.
	*/
	virtual int GetTVFaceIndex(int particleIndex) const = 0;

	/*! \fn virtual const TabTVFace* GetTVFaceByIndex(int valueIndex) const = 0;
	*  \brief get TabTVFace of the valueIndex-th value. See GetTVFaceIndex(int particleIndex).
	*/
	virtual const TabTVFace* GetTVFaceByIndex(int valueIndex) const = 0;

	/*! \fn virtual const TabTVFace* GetTVFace(int particleIndex) const = 0;
	*  \brief get TabTVFace of a particle with the specified index. See GetTVFaceIndex(int particleIndex).
	*/
	virtual const TabTVFace* GetTVFace(int particleIndex) const = 0;

	/*! \fn virtual const TabTVFace* GetTVFace() const = 0;
	*  \brief get global TabTVFace value for all particles. the method returns the TabTVFace value of the first particle if it is local or shared. See GetTVFaceIndex(int particleIndex).
	*/
	virtual const TabTVFace* GetTVFace() const = 0;

	/*! \fn virtual void Apply(Mesh* mesh, int particleIndex, int mp) const = 0;
	*  \brief apply content of the channel to the mesh [mesh] at mapping channel [mp]
	*/
	virtual void Apply(Mesh* mesh, int particleIndex, int mp) const = 0;

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMAPR_INTERFACE); }
	*  \brief
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMAPR_INTERFACE); }
};

class IParticleChannelMapW : public FPMixinInterface
{
public:
    /** @defgroup IChannelContainer IChannelContainer.h
    *  @{
    */

	/*! \fn virtual bool SetUVVert(int particleIndex, const UVVert& value) = 0;
	*  \brief  sets all vertices of the particle have the same UVVert value. returns true if successful.
	*/
	virtual bool SetUVVert(int particleIndex, const UVVert& value) = 0;

	/*! \fn virtual bool SetUVVert(int particleIndex, const TabUVVert* tab) = 0;
	*  \brief copies UVVert data to be texture mapping for the particle. returns true if successful.
	*/
	virtual bool SetUVVert(int particleIndex, const TabUVVert* tab) = 0;

	/*! \fn  virtual bool SetUVVert(Tab<int>& particleIndices, const UVVert& value) = 0;
	*  \brief copies UVVert data to be a shared value with indices in "particleIndeces"
	*/
	virtual bool SetUVVert(Tab<int>& particleIndices, const UVVert& value) = 0;

	/*! \fn virtual bool SetUVVert(Tab<int>& particleIndices, const TabUVVert* tab) = 0;
	*  \brief copies UVVert data to be a shared value with indices in "particleIndeces"
	*/
	virtual bool SetUVVert(Tab<int>& particleIndices, const TabUVVert* tab) = 0;

	/*! \fn virtual bool SetUVVert(const UVVert& value) = 0;
	*  \brief copies UVVert data to a global value for all particles
	*/
	virtual bool SetUVVert(const UVVert& value) = 0;

	/*! \fn virtual bool SetUVVert(const TabUVVert* tab) = 0;
	*  \brief copies UVVert data to a global value for all particles
	*/
	virtual bool SetUVVert(const TabUVVert* tab) = 0;

	/*! \fn virtual bool	CopyUVVert(int fromParticle, int toParticle) = 0;
	*  \brief copy UVVert data from fromParticle to toParticle
 	*/
	virtual bool	CopyUVVert(int fromParticle, int toParticle) = 0;

	/*! \fn virtual bool	CopyUVVert(int fromParticle, Tab<int>& toParticles) = 0;
	*  \brief copy UVVert data from fromParticle to toParticles
	*/
	virtual bool	CopyUVVert(int fromParticle, Tab<int>& toParticles) = 0;

	/*! \fn virtual bool	CopyUVVert(int fromParticle) = 0;
	*  \brief copy UVVert data from fromParticle to all particles
	*/
	virtual bool	CopyUVVert(int fromParticle) = 0;

	/*! \fn virtual bool SetTVFace(int particleIndex, const TabTVFace* tab) = 0;
	*  \brief copies TVFace data to be texture facing for the particle. returns true if successful.
	*/
	virtual bool SetTVFace(int particleIndex, const TabTVFace* tab) = 0;

	/*! \fn virtual bool SetTVFace(Tab<int>& particleIndices, const TabTVFace* tab) = 0;
	*  \brief copies TVFace data to be a shared value with indices in "particleIndeces"
	*/
	virtual bool SetTVFace(Tab<int>& particleIndices, const TabTVFace* tab) = 0;

	/*! \fn virtual bool SetTVFace(const TabTVFace* tab) = 0;
	*  \brief copies TVFace data to a global value for all particles
	*/
	virtual bool SetTVFace(const TabTVFace* tab) = 0;

	/*! \fn virtual bool	CopyTVFace(int fromParticle, int toParticle) = 0;
	*  \brief copy TVFace data from fromParticle to toParticle
	*/
	virtual bool	CopyTVFace(int fromParticle, int toParticle) = 0;

	/*! \fn virtual bool	CopyTVFace(int fromParticle, Tab<int>& toParticles) = 0;
	*  \brief copy TVFace data from fromParticle to toParticles
	*/
	virtual bool	CopyTVFace(int fromParticle, Tab<int>& toParticles) = 0;

	/*! \fn virtual bool	CopyTVFace(int fromParticle) = 0;
	*  \brief copy TVFace data from fromParticle to all particles
	*/
	virtual bool	CopyTVFace(int fromParticle) = 0;

	/*! \fn virtual IObject* GetUVVertChannel() const = 0;
	*  \brief returns an object of TabUVVert channel
	*/
	virtual IObject* GetUVVertChannel() const = 0;

	/*! \fn virtual IObject* GetTVFaceChannel() const = 0;
	*  \brief returns an object of TabTVFace channel
	*/
	virtual IObject* GetTVFaceChannel() const = 0;

	/*! \fn bool SetMap(int particleIndex, const UVVert& value)
	*  \brief sets all vertices of the particle have the same map value. returns true if successful.
	*/
	bool SetMap(int particleIndex, const UVVert& value)
	{ return (SetUVVert(particleIndex, value) && SetTVFace(NULL)); }

	/*! \fn bool SetMap(int particleIndex, const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	*  \brief copies map data to be texture mapping for the particle. returns true if successful.
	*/
	bool SetMap(int particleIndex, const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	{ return (SetUVVert(particleIndex, tabUVVert) && SetTVFace(particleIndex, tabTVFace)); }

	/*! \fn bool SetMap(Tab<int>& particleIndices, const UVVert& value)
	*  \brief copies map data to be a shared value with indices in "particleIndeces"
	*/
	bool SetMap(Tab<int>& particleIndices, const UVVert& value)
	{ return (SetUVVert(particleIndices, value) && SetTVFace(particleIndices, NULL)); }

	/*! \fn  bool SetMap(Tab<int>& particleIndices, const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	*  \brief copies map data to be a shared value with indices in "particleIndeces"
	*/
	bool SetMap(Tab<int>& particleIndices, const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	{ return (SetUVVert(particleIndices, tabUVVert) && SetTVFace(particleIndices, tabTVFace)); }

	/*! \fn bool SetMap(const UVVert& value)
	*  \brief copies map data to a global value for all particles
	*/
	bool SetMap(const UVVert& value)
	{ return (SetUVVert(value) && SetTVFace(NULL)); }

	/*! \fn  bool SetMap(const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	*  \brief copies map data to a global value for all particles
	*/
	bool SetMap(const TabUVVert* tabUVVert, const TabTVFace* tabTVFace=NULL)
	{ return (SetUVVert(tabUVVert) && SetTVFace(tabTVFace)); }

	/*! \fn bool	CopyMap(int fromParticle, int toParticle)
	*  \brief copy map data from fromParticle to toParticle
	*/
	bool	CopyMap(int fromParticle, int toParticle)
	{ return (CopyUVVert(fromParticle, toParticle) && CopyTVFace(fromParticle, toParticle)); }

	/*! \fn bool	CopyMap(int fromParticle, Tab<int>& toParticles)
	*  \brief copy map data from fromParticle to toParticles
	*/
	bool	CopyMap(int fromParticle, Tab<int>& toParticles)
	{ return (CopyUVVert(fromParticle, toParticles) && CopyTVFace(fromParticle, toParticles)); }

	/*! \fn bool	CopyMap(int fromParticle)
	*  \brief copy map data from fromParticle to all particles
	*/
	bool	CopyMap(int fromParticle)
	{ return (CopyUVVert(fromParticle) && CopyTVFace(fromParticle)); }

	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMAPW_INTERFACE); }
	*  \brief 
	*/
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMAPW_INTERFACE); }
};


