/*=============================================================================
	BuildOptions.h: Lighting options definitions.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#pragma once

namespace Lightmass
{

enum ELightingSolverType
{
	/** Default direct lighting solver with no global/bounced lighting */
	LightSolver_Direct=0,
	/** GI solver type */
	LightSolver_Bouncy
};

/**
 * A set of parameters specifying how static lighting is rebuilt.
 */
class FLightingBuildOptions
{
public:

	FLightingBuildOptions()
	:	bPerformFullQualityBuild(TRUE)
	,	LightSolverType(LightSolver_Direct)
	{}

	/** Whether to use fast or pretty lighting build.								*/
	UBOOL					bPerformFullQualityBuild;
	/** Current light solver type desired */
	ELightingSolverType		LightSolverType;
};

} //namespace Lightmass
