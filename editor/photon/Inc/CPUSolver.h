/*=============================================================================
	CPUSolver.h: CPU solver interface
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/


#pragma once

#ifndef CPUSOLVER_H
#define CPUSOLVER_H

namespace Lightmass
{

/**
 * Entry point for starting the static lighting process
 * 
 * @param SceneFile		File of the scene to process
 * @param NumThreads	Number of concurrent threads to use for lighting building
 * @param bDumpTextures	If TRUE, 2d lightmaps will be dumped to 
 */
int BuildStaticLighting(const FString& SceneFile, INT NumThreads, UBOOL bDumpTextures);

/** Helper struct that contain global statistics for the Lightmass thread execution. */
struct FThreadStatistics
{
	FThreadStatistics()
	:	TotalTime(0.0)
	,	RequestTime(0.0)
	,	ExportTime(0.0)
	,	VertexMappingTime(0.0)
	,	TextureMappingTime(0.0)
	,	RequestTimeoutTime(0.0)
	,	SwarmRequestTime(0.0)
	,	NumVertexMappings(0)
	,	NumTextureMappings(0)
	{
	}

	void operator+=( const FThreadStatistics& Other )
	{
		TotalTime += Other.TotalTime;
		RequestTime += Other.RequestTime;
		ExportTime += Other.ExportTime;
		VertexMappingTime += Other.VertexMappingTime;
		TextureMappingTime += Other.TextureMappingTime;
		RequestTimeoutTime += Other.RequestTimeoutTime;
		SwarmRequestTime += Other.SwarmRequestTime;
		NumVertexMappings += Other.NumVertexMappings;
		NumTextureMappings += Other.NumTextureMappings;
	}

	DOUBLE	TotalTime;
	DOUBLE	RequestTime;
	DOUBLE	ExportTime;
	DOUBLE	VertexMappingTime;
	DOUBLE	TextureMappingTime;
	DOUBLE	RequestTimeoutTime;
	DOUBLE	SwarmRequestTime;
	INT		NumVertexMappings;
	INT		NumTextureMappings;
};

/** Helper struct that contain global statistics for the Lightmass execution. */
struct FGlobalStatistics
{
	FGlobalStatistics()
	:	NumThreads(0)
	,	NumThreadsFinished(0)
	,	NumTotalMappings(0)
	,	NumExportedMappings(0)
	,	TotalTimeStart(0.0)
	,	TotalTimeEnd(0.0)
	,	ImportTimeStart(0.0)
	,	ImportTimeEnd(0.0)
	,	PhotonsStart(0.0)
	,	PhotonsEnd(0.0)
	,	WorkTimeStart(0.0)
	,	WorkTimeEnd(0.0)
	,	ExtraExportTime(0.0)
	,	SendMessageTime(0.0)
	,	SceneSetupTime(0.0)
	{
	}
	FThreadStatistics	ThreadStatistics;
	INT					NumThreads;
	volatile INT		NumThreadsFinished;		// Incremented by each thread when they finish.
	INT					NumTotalMappings;
	INT					NumExportedMappings;	// Only incremented while threads are still running.
	DOUBLE				TotalTimeStart;
	DOUBLE				TotalTimeEnd;
	DOUBLE				ImportTimeStart;
	DOUBLE				ImportTimeEnd;
	DOUBLE				PhotonsStart;
	DOUBLE				PhotonsEnd;
	DOUBLE				WorkTimeStart;
	DOUBLE				WorkTimeEnd;
	DOUBLE				ExtraExportTime;
	/** Time spent in SendMessage(), in seconds. */
	DOUBLE				SendMessageTime;
	/** Time spent setting up the scene, in seconds. */
	DOUBLE				SceneSetupTime;
};

/** Global statistics */
extern FGlobalStatistics GStatistics;

/** Global Swarm instance. */
extern class FLightmassSwarm* GSwarm;

/** Whether we should report detailed stats back to UE3. */
extern UBOOL GReportDetailedStats;

/** Whether Lightmass is running in debug mode (-debug), using a hardcoded job and not requesting tasks from Swarm. */
extern UBOOL GDebugMode;

} //namespace Lightmass

#endif
