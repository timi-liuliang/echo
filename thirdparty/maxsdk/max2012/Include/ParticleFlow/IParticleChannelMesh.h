/*! \file IParticleChannelMesh.h
    \brief Channel-generic interface for particle channels
				 that store data in "mesh" form
				 The channel can be local (each particle has its own mesh)
				 global (all particles have the same mesh) and shared
				 (particles are sharing a limited set of meshes)
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-28-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
// forward declarations
class Box3;

// standard particle channel "Mesh"
// interface ID
#define PARTICLECHANNELMESHR_INTERFACE Interface_ID(0x74f93c11, 0x1eb34500)
#define PARTICLECHANNELMESHW_INTERFACE Interface_ID(0x74f93c11, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelMeshRInterface(obj) ((IParticleChannelMeshR*)obj->GetInterface(PARTICLECHANNELMESHR_INTERFACE))
//#define GetParticleChannelMeshWInterface(obj) ((IParticleChannelMeshW*)obj->GetInterface(PARTICLECHANNELMESHW_INTERFACE))


class IParticleChannelMeshR : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kIsShared,
			kGetValueCount,
			kGetValueIndex,
			kGetValueByIndex,
			kGetValue,
			kGetValueFirst,
			kGetMaxBoundingBox
	};

	BEGIN_FUNCTION_MAP

	FN_0(kIsShared, TYPE_bool, IsShared);
	FN_0(kGetValueCount, TYPE_INT, GetValueCount);
	FN_1(kGetValueIndex, TYPE_INT, GetValueIndex, TYPE_INT);
	FN_1(kGetValueByIndex, TYPE_MESH, GetValueByIndex, TYPE_INT);
	FN_1(kGetValue, TYPE_MESH, GetValue, TYPE_INT);
	FN_0(kGetValueFirst, TYPE_MESH, GetValue);
	VFN_2(kGetMaxBoundingBox, GetMaxBoundingBox, TYPE_POINT3_BR, TYPE_POINT3_BR);

	END_FUNCTION_MAP

	// check out if some particles have shared mesh
	// if it's true then there is no need to get a shape for each particle
	virtual bool		IsShared() const = 0;
	// get total number of actual meshes (values) in the channel
	virtual int			GetValueCount() const = 0;
	// get the value index of a particle
	virtual int			GetValueIndex(int particleIndex) const = 0;
	// get shape of the valueIndex-th value
	virtual const Mesh* GetValueByIndex(int valueIndex) const = 0;
	// get shape for particle with index
	virtual const Mesh*	GetValue(int particleIndex) const = 0;
	// get global shape for all particles
	// the method returns the mesh of the first particle if it is local or shared
	virtual const Mesh*	GetValue() const = 0;
	// returns maximal bounding box
	virtual const Box3&		GetMaxBoundingBox() const = 0;
	// FnPub alternative for the method above
	PFExport void			GetMaxBoundingBox(Point3& corner1, Point3& corner2) const;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHR_INTERFACE); }
};

class IParticleChannelMeshW : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueMany,
			kSetValueAll,
			kCopyValue,
			kCopyValueMany,
			kCopyValueAll,
			kBuildMaxBoundingBox
	};

	BEGIN_FUNCTION_MAP

	FN_2(kSetValue, TYPE_bool, SetValue, TYPE_INT, TYPE_MESH);
	FN_2(kSetValueMany, TYPE_bool, SetValue, TYPE_INT_TAB_BR, TYPE_MESH);
	FN_1(kSetValueAll, TYPE_bool, SetValue, TYPE_MESH);
	FN_2(kCopyValue, TYPE_bool, CopyValue, TYPE_INT, TYPE_INT);
	FN_2(kCopyValueMany, TYPE_bool, CopyValue, TYPE_INT, TYPE_INT_TAB_BR);
	FN_1(kCopyValueAll, TYPE_bool, CopyValue, TYPE_INT);
	VFN_0(kBuildMaxBoundingBox, BuildMaxBoundingBox);

	END_FUNCTION_MAP

	// copies mesh to be a local value for particle with index "particleIndex"
	// returns true if successful
	virtual bool	SetValue(int particleIndex, Mesh* mesh) = 0;
	// copies mesh to be a shared value with indices in "particleIndices"
	// returns true if successful
	virtual bool	SetValue(Tab<int>& particleIndices, Mesh* mesh) = 0;
	// copies mesh to be a global values for all particles
	// returns true if successful
	virtual bool	SetValue(Mesh* mesh) = 0;
	// copy mesh value from fromParticle to toParticle
	virtual bool	CopyValue(int fromParticle, int toParticle) = 0;
	// copy mesh value from fromParticle to toParticles
	virtual bool	CopyValue(int fromParticle, Tab<int>& toParticles) = 0;
	// copy mesh value from fromParticle to all particles
	virtual bool	CopyValue(int fromParticle) = 0;
	// build maximal bounding box for the set of all shapes
	virtual void	BuildMaxBoundingBox() = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMESHW_INTERFACE); }
};

