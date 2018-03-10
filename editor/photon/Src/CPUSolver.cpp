/*=============================================================================
	CPUSolver.cpp: CPU solver implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "Scene.h"
#include "Importer.h"
#include "Exporter.h"
#include "LightmassSwarm.h"
#include "CPUSolver.h"
#include "LightingSystem.h"


namespace Lightmass
{

/** Global statistics */
FGlobalStatistics GStatistics;

/** Global Swarm instance. */
FLightmassSwarm* GSwarm = NULL;

/** Whether we should report detailed stats back to UE3. */
UBOOL GReportDetailedStats = FALSE;

/** Whether Lightmass is running in debug mode (-debug), using a hardcoded job and not requesting tasks from Swarm. */
UBOOL GDebugMode = FALSE;

/** How many tasks to prefetch per worker thread.(ÁõÁÁ) */ 
FLOAT GNumTasksPerThreadPrefetch = 100000.0f;

/** Report statistics back to UE3. */
void ReportStatistics()
{
	if ( GReportDetailedStats )
	{
		DOUBLE RequestTime = GStatistics.ThreadStatistics.RequestTime + GStatistics.ThreadStatistics.RequestTimeoutTime;
		DOUBLE TrackedTime = RequestTime + GStatistics.ThreadStatistics.VertexMappingTime +
			GStatistics.ThreadStatistics.TextureMappingTime + GStatistics.ThreadStatistics.ExportTime;
		DOUBLE UnTrackedTime = GStatistics.ThreadStatistics.TotalTime - TrackedTime;

		// Send back detailed information to the UE3 log.
		GSwarm->SendTextMessage(
			TEXT("Lightmass on %s: %s total, %s importing, %s setup, %s photons, %s processing, %s extra exporting [%d/%d mappings].\n")
			TEXT("  Threads: %d threads, %.0f total thread seconds (out of %.0f available)\n")
			TEXT("  - %6.2f%% %7.1fs   Requesting tasks\n")
			TEXT("  ---> %6.2f%% %7.1fs   Requesting tasks from Swarm\n")
			TEXT("  - %6.2f%% %7.1fs   Processing vertex mappings\n")
			TEXT("  - %6.2f%% %7.1fs   Processing texture mappings\n")
			TEXT("  - %6.2f%% %7.1fs   Exporting %d mappings\n")
			TEXT("  - %6.2f%% %7.1fs   Untracked thread time\n")
			TEXT("\n")
			TEXT("  Read amount: %3.2fMB (%.3f sec, %d calls)\n")
			TEXT("  Write amount: %3.2fMB (%.3f sec, %d calls)\n")
			, appComputerName()
			, *appPrettyTime(GStatistics.TotalTimeEnd - GStatistics.TotalTimeStart)
			, *appPrettyTime(GStatistics.ImportTimeEnd - GStatistics.ImportTimeStart)
			, *appPrettyTime(GStatistics.SceneSetupTime)
			, *appPrettyTime(GStatistics.PhotonsEnd - GStatistics.PhotonsStart)
			, *appPrettyTime(GStatistics.WorkTimeEnd - GStatistics.WorkTimeStart)
			, *appPrettyTime(GStatistics.ExtraExportTime)
			, GStatistics.ThreadStatistics.NumVertexMappings + GStatistics.ThreadStatistics.NumTextureMappings
			, GStatistics.NumTotalMappings
			, GStatistics.NumThreads
			, GStatistics.ThreadStatistics.TotalTime
			, (GStatistics.WorkTimeEnd - GStatistics.WorkTimeStart) * GStatistics.NumThreads
			, RequestTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, RequestTime
			, GStatistics.ThreadStatistics.SwarmRequestTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, GStatistics.ThreadStatistics.SwarmRequestTime
			, GStatistics.ThreadStatistics.VertexMappingTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, GStatistics.ThreadStatistics.VertexMappingTime
			, GStatistics.ThreadStatistics.TextureMappingTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, GStatistics.ThreadStatistics.TextureMappingTime
			, GStatistics.ThreadStatistics.ExportTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, GStatistics.ThreadStatistics.ExportTime
			, GStatistics.NumExportedMappings
			, UnTrackedTime / GStatistics.ThreadStatistics.TotalTime * 100.0
			, UnTrackedTime
			, (DOUBLE)GSwarm->GetTotalBytesRead() / 1000.0 / 1000.0
			, GSwarm->GetTotalSecondsRead()
			, GSwarm->GetTotalNumReads()
			, (DOUBLE)GSwarm->GetTotalBytesWritten() / 1000.0 / 1000.0
			, GSwarm->GetTotalSecondsWritten()
			, GSwarm->GetTotalNumWrites()
			);

		if ( GDebugMode == FALSE )
		{
			debugf( TEXT("Time in SendMessage() = %s"), *appPrettyTime(GStatistics.SendMessageTime) );
			debugf( TEXT("Task request roundtrip = %s"), *appPrettyTime(FTiming::GetAverageTiming()) );
		}
	}
	else
	{
		// Send back timing information to the UE3 log.
		GSwarm->SendTextMessage(
			TEXT("Lightmass on %s: %s total, %s importing, %s setup, %s photons, %s processing, %s extra exporting [%d/%d mappings]. Threads: %s total, %s processing.")
			, appComputerName()
			, *appPrettyTime(GStatistics.TotalTimeEnd - GStatistics.TotalTimeStart)
			, *appPrettyTime(GStatistics.ImportTimeEnd - GStatistics.ImportTimeStart)
			, *appPrettyTime(GStatistics.SceneSetupTime)
			, *appPrettyTime(GStatistics.PhotonsEnd - GStatistics.PhotonsStart)
			, *appPrettyTime(GStatistics.WorkTimeEnd - GStatistics.WorkTimeStart)
			, *appPrettyTime(GStatistics.ExtraExportTime)
			, GStatistics.ThreadStatistics.NumVertexMappings + GStatistics.ThreadStatistics.NumTextureMappings
			, GStatistics.NumTotalMappings
			, *appPrettyTime(GStatistics.ThreadStatistics.TotalTime)
			, *appPrettyTime(GStatistics.ThreadStatistics.VertexMappingTime + GStatistics.ThreadStatistics.TextureMappingTime)
			);
	}
}

/** Transfers back the current log file to the instigator. */
void ReportLogFile()
{
	GLog->Flush();

	// Get the log filename and replace ".log" with "_Result.log"
	FString LogFilename = GLog->GetLogFilename();
	FString ChannelName = LogFilename.Left( LogFilename.Len() - 4 ) + TEXT("_Result.log");

	HANDLE File = CreateFile(*LogFilename, GENERIC_READ, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	BOOL bIsOk = FALSE;
	if ( File != INVALID_HANDLE_VALUE )
	{
		LARGE_INTEGER Filesize;
		GetFileSizeEx( File, &Filesize );

		INT Channel = GSwarm->OpenChannel( *ChannelName, NSwarm::SWARM_JOB_CHANNEL_WRITE, TRUE );
		if ( Channel >= 0 )
		{
			static char Buffer[4096];
			unsigned long  NumBytesRead = 0;
			bIsOk = TRUE;
			while ( Filesize.QuadPart > 0 && ReadFile( File, Buffer, 4096, &NumBytesRead, NULL ) )
			{
				GSwarm->Write( Buffer, NumBytesRead );
				Filesize.QuadPart -= NumBytesRead;
			}
			GSwarm->PopChannel( TRUE );
		}
		CloseHandle( File );
	}
	if ( !bIsOk )
	{
		debugf(TEXT("Failed to send back log file through Swarm!"));
	}
}

/**
 * Entry point for starting the static lighting process
 * 
 * @param SceneFile		File of the scene to process
 * @param NumThreads	Number of concurrent threads to use for lighting building
 * @param bDumpTextures	If TRUE, 2d lightmaps will be dumped to 
 */
INT BuildStaticLighting(const FString& SceneFile, INT NumThreads, UBOOL bDumpTextures)
{
	// Place a marker in the memory profile data.
	GMalloc->SnapshotMemory();

	debugf(TEXT("Building static lighting..."));

	// time it
	DOUBLE SetupTimeStart = appSeconds();

	// Start initializing GCPUFrequency.
	StartInitCPUFrequency();

	// Startup Swarm.
	GStatistics.ImportTimeStart = appSeconds();
	GSwarm = new FLightmassSwarm( /*NSwarm::FSwarmInterface::Get(), SceneGuid,*/ appTrunc(GNumTasksPerThreadPrefetch*NumThreads) );
	//GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_BeginJob, -1 ) );

	//FLightmassImporter Importer( SceneFile);
	FScene Scene;
	if( !Scene.Load( SceneFile))
	{
		debugf(TEXT("Failed to import scene file"));
		exit( 1 );
	}
	GStatistics.ImportTimeEnd = appSeconds();

	// Finish initializing GCPUFrequency.
	FinishInitCPUFrequency();

	// setup the desired lighting options
	FLightingBuildOptions LightingOptions;

	FLightmassSolverExporter Exporter( GSwarm, Scene, bDumpTextures );


	// Place a marker in the memory profile data.
	GMalloc->SnapshotMemory();

	DOUBLE LightTimeStart = appSeconds();

	// Create the global lighting system to kick off the processing
	FStaticLightingSystem LightingSystem(LightingOptions, Scene, Exporter, NumThreads );	

	GStatistics.TotalTimeEnd = appSeconds();

	// Place a marker in the memory profile data.
	GMalloc->SnapshotMemory();

	// Report back statistics over Swarm.
	ReportStatistics();

	DOUBLE EndTime = appSeconds();

	debugf(TEXT("Lighting complete [Startup = %s, Lighting = %s]"), *appPrettyTime(LightTimeStart - SetupTimeStart), *appPrettyTime(EndTime - LightTimeStart));

	if ( GReportDetailedStats )
	{
		extern volatile QWORD GKDOPParentNodesTraversed;
		extern volatile QWORD GKDOPLeafNodesTraversed;
		extern volatile QWORD GKDOPTrianglesTraversed;
		extern volatile QWORD GKDOPTrianglesTraversedReal;
		DOUBLE KDOPTrianglesTraversedRealPercent = (1.0 - ((GKDOPTrianglesTraversed - GKDOPTrianglesTraversedReal) / (GKDOPTrianglesTraversed * 100.0)));
		debugf(TEXT("kDOP traversals (in millions): %.3g parents, %.3g leaves, %.3g triangles (%.3g, %.3g%%, real triangles)."),
			GKDOPParentNodesTraversed / 1000000.0,
			GKDOPLeafNodesTraversed / 1000000.0,
			GKDOPTrianglesTraversed / 1000000.0,
			GKDOPTrianglesTraversedReal / 1000000.0,
			KDOPTrianglesTraversedRealPercent );
	}

	// Transfer back the log to the instigator.
	if( FALSE)
		ReportLogFile();

	// Shutdown Swarm
	FLightmassSwarm* Swarm = GSwarm;
	GSwarm = NULL;
	delete Swarm;

	// Write out memory profiling data to the .mprof file.
	GMalloc->WriteProfilingData();

	return Scene.Num;
}

} //namespace Lightmass
