/*=============================================================================
	LightmassPublic.h: Main header file for external users of Lightmass to include
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

#include "PublicTypes.h"

namespace Lightmass
{
	// Remap the definitions in PublicStructs.h to the Base versions in PublicTypes.h
	typedef FGuidBase FGuid;
	typedef FMatrixBase FMatrix;
	typedef FVector4Base FVector4;
	typedef FVector4Base FPlane;
	typedef FVector2DBase FVector2D;
	typedef FColorBase FColor;
	typedef FPackedNormalBase FPackedNormal;
	typedef FBoxBase FBox;
	typedef FIntPointBase FIntPoint;
	typedef FFloat16Base FFloat16;
	typedef FQuantizedSHVectorBase FQuantizedSHVector;
	typedef FQuantizedSHVectorRGBBase FQuantizedSHVectorRGB;
}

#include "PublicStructs.h"
