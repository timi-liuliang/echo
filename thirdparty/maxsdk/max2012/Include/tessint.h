/**********************************************************************
 *<
	FILE: tessint.h

	DESCRIPTION: include file for gap integration

	CREATED BY: Charles Thaeler

	HISTORY: created 10 Jan 1997

 *>	Copyright (c) 1996, All Rights Reserved.
 **********************************************************************/

#pragma once

#ifdef BLD_TESS
#define TExport __declspec(dllexport)
#else
#define TExport __declspec(dllimport)
#endif

#include "maxheap.h"
#include "maxtess.h"
#include "UVWMapper.h"

// forward declarations
class GmSurface;
class View;
class Mtl;
class Mesh;

// This class describes the parameters for a projective mapper.  They
// are set when a UVW mapper modifer is applied.
// SteveA 6/98: I moved the guts of this into "UVWMapper" in the mesh library.
class UVWMapperDesc : public UVWMapper
{
public:
	int     channel;

	UVWMapperDesc() {channel=0;}
	UVWMapperDesc(int type, float utile, float vtile, float wtile,
		int uflip, int vflip, int wflip, int cap,
		const Matrix3 &tm,int channel);
	UVWMapperDesc(UVWMapperDesc& m)
		: UVWMapper (m)
	{
		this->channel = m.channel;
	}

	void ApplyMapper(Mesh* pMesh);
	void InvalidateMapping(Mesh* pMesh);
	IOResult Load(ILoad* iload);
	IOResult Save(ISave* isave);
};

struct SurfTabEntry: public MaxHeapOperators {
	GmSurface *gmsurf;
	UVWMapperDesc* mpChannel1Mapper;
	UVWMapperDesc* mpChannel2Mapper;

	SurfTabEntry() {
		gmsurf = NULL;
		mpChannel1Mapper = NULL;
		mpChannel2Mapper = NULL;
	}
};

typedef Tab<SurfTabEntry> SurfTab;


enum SurfaceType
{
	BEZIER_PATCH,
	GMSURFACE,
	MAX_MESH,
	MODEL_OP
};

TExport MCHAR* GapVersion(void);
TExport int GapTessellate(void *surf, SurfaceType type, Matrix3 *otm, Mesh *mesh,
							TessApprox *tess, TessApprox *disp, 
							View *view, Mtl* mtl, BOOL dumpMiFile, BOOL splitmesh);
