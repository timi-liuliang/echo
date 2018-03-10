/*! \file IParticleChannelPoint3.h
    \brief Channel-generic interfaces for particle channels that store 3D vector data.
*/
/**********************************************************************
 *<
	CREATED BY: Oleg Bayborodin

	HISTORY: created 11-29-01

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"
// forward declarations
class Box3;

// generic particle channel "Point3"
// interface ID
#define PARTICLECHANNELPOINT3R_INTERFACE Interface_ID(0x74f93c03, 0x1eb34500)
#define PARTICLECHANNELPOINT3W_INTERFACE Interface_ID(0x74f93c03, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelPoint3RInterface(obj) ((IParticleChannelPoint3R*)obj->GetInterface(PARTICLECHANNELPOINT3R_INTERFACE))
//#define GetParticleChannelPoint3WInterface(obj) ((IParticleChannelPoint3W*)obj->GetInterface(PARTICLECHANNELPOINT3W_INTERFACE))


class IParticleChannelPoint3R : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetValue,
			kIsGlobal,
			kGetValueGlobal,
			kGetBoundingBox,
			kGetMaxLengthValue
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetValue, TYPE_POINT3_BR, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_POINT3_BR, GetValue);
	VFN_2(kGetBoundingBox, GetBoundingBox, TYPE_POINT3_BR, TYPE_POINT3_BR);
	FN_0(kGetMaxLengthValue, TYPE_FLOAT, GetMaxLengthValue);

	END_FUNCTION_MAP

	// get property for particle with index
	virtual const Point3&	GetValue(int index) const = 0;
	// verify if the channel is global
	virtual bool			IsGlobal() const = 0;
	// if channel is global returns the global value
	// if channel is not global returns value of the first particle
	virtual const Point3&	GetValue() const = 0;
	// returns bounding box for all particles
	virtual const Box3&		GetBoundingBox() const = 0;
	// FnPub alternative for the method above
	PFExport void			GetBoundingBox(Point3& corner1, Point3& corner2) const;
	// returns maximum length of 3D vector of all particles
	virtual float			GetMaxLengthValue() const = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELPOINT3R_INTERFACE); }
};

class IParticleChannelPoint3W : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
			kBuildBoundingBox,
			kUpdateMaxLengthValue
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_POINT3_BR);
	VFN_1(kSetValueGlobal, SetValue, TYPE_POINT3_BR);
	VFN_0(kBuildBoundingBox, BuildBoundingBox);
	VFN_0(kUpdateMaxLengthValue, UpdateMaxLengthValue);

	END_FUNCTION_MAP

	// set property for particle with index
	virtual void	SetValue(int index, const Point3& v) = 0;
	// set property for all particles at once thus making the channel global
	virtual void	SetValue(const Point3& v) = 0;
	// build bounding box for 3D vector data of all particles
	virtual void	BuildBoundingBox() = 0;
	// update maximum length value for 3d vector data of all particles
	virtual void	UpdateMaxLengthValue() = 0;

	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELPOINT3W_INTERFACE); }
};

