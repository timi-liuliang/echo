/*=============================================================================
	Cache.cpp: Lighting cache implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightingSystem.h"

namespace Lightmass
{

FLightingCacheBase::FLightingCacheBase(const FStaticLightingSystem& InSystem, INT InBounceNumber) :
	InterpolationAngleNormalization(1.0f / appSqrt(1.0f - appCos(InSystem.IrradianceCachingSettings.InterpolationMaxAngle * (FLOAT)PI / 180.0f))),
	InterpolationAngleNormalizationSmooth(1.0f / appSqrt(1.0f - appCos(InSystem.IrradianceCachingSettings.AngleSmoothFactor * InSystem.IrradianceCachingSettings.InterpolationMaxAngle * (FLOAT)PI / 180.0f))),
	MinCosPointBehindPlane(appCos((InSystem.IrradianceCachingSettings.PointBehindRecordMaxAngle + 90.0f) * (FLOAT)PI / 180.0f)),
	DistanceSmoothFactor(InSystem.IrradianceCachingSettings.DistanceSmoothFactor),
	bUseIrradianceGradients(InSystem.IrradianceCachingSettings.bUseIrradianceGradients),
	bShowGradientsOnly(InSystem.IrradianceCachingSettings.bShowGradientsOnly),
	bVisualizeIrradianceSamples(InSystem.IrradianceCachingSettings.bVisualizeIrradianceSamples),
	BounceNumber(InBounceNumber),
	NextRecordId(0),
	System(InSystem)
{}

} //namespace Lightmass
