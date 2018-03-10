/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#ifndef GU_HEIGHTFIELD_DATA_H
#define GU_HEIGHTFIELD_DATA_H

#include "PxSimpleTypes.h"
#include "PxHeightFieldFlag.h"
#include "PxHeightFieldSample.h"
#include "PxBounds3.h"
#include "PxMetaData.h"

namespace physx
{

// CA: New tiled memory layout on PS3
#ifdef __CELLOS_LV2__
#define HF_TILED_MEMORY_LAYOUT 1
#else
#define HF_TILED_MEMORY_LAYOUT 0
#endif
#define HF_TILE_SIZE_U (4)	// PT: WARNING: if you change this value, you must also change it in ConvX
#define HF_TILE_SIZE_V (4)	// PT: WARNING: if you change this value, you must also change it in ConvX

namespace Gu
{

#if defined(PX_VC) 
    #pragma warning(push)
	#pragma warning( disable : 4251 ) // class needs to have dll-interface to be used by clients of class
#endif
struct PX_PHYSX_COMMON_API HeightFieldData
{
// PX_SERIALIZATION
	PX_FORCE_INLINE				HeightFieldData()									{}
	PX_FORCE_INLINE				HeightFieldData(const PxEMPTY&) :	flags(PxEmpty)	{}
//~PX_SERIALIZATION

	//properties
						PxU32						rows;					// PT: WARNING: don't change this member's name (used in ConvX)
						PxU32						columns;				// PT: WARNING: don't change this member's name (used in ConvX)
						PxReal						rowLimit;				// PT: to avoid runtime int-to-float conversions on Xbox
						PxReal						colLimit;				// PT: to avoid runtime int-to-float conversions on Xbox
						PxReal						nbColumns;				// PT: to avoid runtime int-to-float conversions on Xbox
						PxHeightFieldSample*		samples;				// PT: WARNING: don't change this member's name (used in ConvX)
						PxReal						thickness;
						PxReal						convexEdgeThreshold;

						PxHeightFieldFlags			flags;

						PxHeightFieldFormat::Enum	format;
						PxBounds3					mAABB;				

//#ifdef HF_TILED_MEMORY_LAYOUT
	// CA:  New tiled memory layout on PS3
						PxU32						rowsPadded;				// PT: WARNING: don't change this member's name (used in ConvX)
						PxU32						columnsPadded;			// PT: WARNING: don't change this member's name (used in ConvX)
						PxU32						tilesU;					// PT: WARNING: don't change this member's name (used in ConvX)
						PxU32						tilesV;					// PT: WARNING: don't change this member's name (used in ConvX)
//#endif
};
#if defined(PX_VC) 
     #pragma warning(pop) 
#endif

} // namespace Gu

}

#endif
