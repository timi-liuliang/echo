/*=============================================================================
	Photon.cpp: Static lighting photon mapping implementation.
	Copyright 1998-2013 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "stdafx.h"
#include "LightingSystem.h"
#include "MonteCarlo.h"
#include "LightmassSwarm.h"

namespace Lightmass
{

/** Average fraction of emitted direct photons that get deposited on surfaces, used to presize gathered photon arrays. */
static const FLOAT DirectPhotonEfficiency = .3f;

/** Average fraction of emitted indirect photons that get deposited on surfaces, used to presize gathered photon arrays. */
static const FLOAT IndirectPhotonEfficiency = .1f;

// Number of parts that photon operating passes will be split into.
// Random number generators and other state will be seeded at the beginning of each work range, 
// To ensure deterministic behavior regardless of how many threads are processing and what order operations happen within each thread.
static const INT NumPhotonWorkRanges = 256;

extern INT WinCreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

/** Sets up photon mapping settings. */
void FStaticLightingSystem::InitializePhotonSettings()
{
	const FBoxSphereBounds SceneBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
	const FBoxSphereBounds ImportanceBounds = GetImportanceBounds();

	// Get direct photon counts from each light
#if LIGHTMASS_NOPROCESSING
	const INT MaxNumDirectPhotonsToEmit = 10;
#else
	// Maximum number of direct photons to emit, used to cap the memory and processing time for a given build
	//@todo - remove these clamps and come up with a robust solution for huge scenes
	const INT MaxNumDirectPhotonsToEmit = 40000000;
#endif
	NumDirectPhotonsToEmit = 0;
	Stats.NumFirstPassPhotonsRequested = 0;
	// Only emit direct photons if they will be used for lighting
	if (GeneralSettings.NumIndirectLightingBounces > 0 || PhotonMappingSettings.bOptimizeDirectLightingWithPhotons || PhotonMappingSettings.bUsePhotonsForDirectLighting)
	{
		for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
		{
			Stats.NumFirstPassPhotonsRequested += Lights(LightIndex)->GetNumDirectPhotons(PhotonMappingSettings.DirectPhotonDensity);
		}

		NumDirectPhotonsToEmit = Min<QWORD>(Stats.NumFirstPassPhotonsRequested, (QWORD)MaxNumDirectPhotonsToEmit);
		if (NumDirectPhotonsToEmit == MaxNumDirectPhotonsToEmit)
		{
			LogSolverMessage(FString::Printf(TEXT("Clamped the number of direct photons to emit to %.3f million, from %.3f million requested."), MaxNumDirectPhotonsToEmit / 1000000.0f, Stats.NumFirstPassPhotonsRequested / 1000000.0f));
		}
	}

	DirectIrradiancePhotonFraction = Clamp(Scene.PhotonMappingSettings.DirectIrradiancePhotonDensity / Scene.PhotonMappingSettings.DirectPhotonDensity, 0.0f, 1.0f);

	// Calculate numbers of photons to gather based on the scene using the given photon densities, the scene's surface area and the importance volume's surface area
	FLOAT SceneSurfaceAreaMillionUnits = Max(AggregateMesh.GetSurfaceArea() / 1000000.0f, DELTA);
	FLOAT SceneSurfaceAreaMillionUnitsEstimate = Max(4.0f * (FLOAT)PI * SceneBounds.SphereRadius * SceneBounds.SphereRadius / 1000000.0f, DELTA);
	FLOAT SceneSurfaceAreaMillionUnitsEstimateDiff = SceneSurfaceAreaMillionUnitsEstimate > DELTA ? ( SceneSurfaceAreaMillionUnits / SceneSurfaceAreaMillionUnitsEstimate * 100.0f ) : 0.0f;
	LogSolverMessage(FString::Printf(TEXT("Scene surface area calculated at %.3f million units (%.3f%% of the estimated %.3f million units)"), SceneSurfaceAreaMillionUnits, SceneSurfaceAreaMillionUnitsEstimateDiff, SceneSurfaceAreaMillionUnitsEstimate));

	FLOAT ImportanceSurfaceAreaMillionUnits = Max(AggregateMesh.GetSurfaceAreaWithinImportanceVolume() / 1000000.0f, DELTA);
	FLOAT ImportanceSurfaceAreaMillionUnitsEstimate = Max(4.0f * (FLOAT)PI * ImportanceBounds.SphereRadius * ImportanceBounds.SphereRadius / 1000000.0f, DELTA);
	FLOAT ImportanceSurfaceAreaMillionUnitsEstimateDiff = ImportanceSurfaceAreaMillionUnitsEstimate > DELTA ? ( ImportanceSurfaceAreaMillionUnits / ImportanceSurfaceAreaMillionUnitsEstimate * 100.0f ) : 0.0f;
	LogSolverMessage(FString::Printf(TEXT("Importance volume surface area calculated at %.3f million units (%.3f%% of the estimated %.3f million units)"), ImportanceSurfaceAreaMillionUnits, ImportanceSurfaceAreaMillionUnitsEstimateDiff, ImportanceSurfaceAreaMillionUnitsEstimate));

#if LIGHTMASS_NOPROCESSING
	const INT MaxNumIndirectPhotonPaths = 10;
#else
	const INT MaxNumIndirectPhotonPaths = 20000;
#endif
	// If the importance volume is valid, only gather enough indirect photon paths to meet IndirectPhotonPathDensity inside the importance volume
	if (!PhotonMappingSettings.bEmitPhotonsOutsideImportanceVolume && ImportanceBounds.SphereRadius > DELTA)
	{ 
		NumIndirectPhotonPaths = appTrunc(Scene.PhotonMappingSettings.IndirectPhotonPathDensity * ImportanceSurfaceAreaMillionUnits);
	}
	else if (ImportanceBounds.SphereRadius > DELTA)
	{
		NumIndirectPhotonPaths = appTrunc(Scene.PhotonMappingSettings.IndirectPhotonPathDensity * ImportanceSurfaceAreaMillionUnits
			+ Scene.PhotonMappingSettings.OutsideImportanceVolumeDensityScale * Scene.PhotonMappingSettings.IndirectPhotonPathDensity * SceneSurfaceAreaMillionUnits);
	}
	else
	{
		NumIndirectPhotonPaths = appTrunc(Scene.PhotonMappingSettings.IndirectPhotonPathDensity * SceneSurfaceAreaMillionUnits);
	}
	NumIndirectPhotonPaths = NumIndirectPhotonPaths == appTruncErrorCode ? MaxNumIndirectPhotonPaths : NumIndirectPhotonPaths;
	NumIndirectPhotonPaths = Min(NumIndirectPhotonPaths, MaxNumIndirectPhotonPaths);
	if (NumIndirectPhotonPaths == MaxNumIndirectPhotonPaths)
	{
		LogSolverMessage(FString::Printf(TEXT("Clamped the number of indirect photon paths to %u."), MaxNumIndirectPhotonPaths));
	}

#if LIGHTMASS_NOPROCESSING
	const INT MaxNumIndirectPhotons = 10;
#else
	const INT MaxNumIndirectPhotons = 40000000;
#endif
	Stats.NumSecondPassPhotonsRequested = 0;
	// If the importance volume is valid, only emit enough indirect photons to meet IndirectPhotonDensity inside the importance volume
	if (!PhotonMappingSettings.bEmitPhotonsOutsideImportanceVolume && ImportanceBounds.SphereRadius > DELTA)
	{
		Stats.NumSecondPassPhotonsRequested = Scene.PhotonMappingSettings.IndirectPhotonDensity * ImportanceSurfaceAreaMillionUnits;
	}
	else if (ImportanceBounds.SphereRadius > DELTA)
	{
		Stats.NumSecondPassPhotonsRequested = Scene.PhotonMappingSettings.IndirectPhotonDensity * ImportanceSurfaceAreaMillionUnits
			+ Scene.PhotonMappingSettings.OutsideImportanceVolumeDensityScale * Scene.PhotonMappingSettings.IndirectPhotonDensity * SceneSurfaceAreaMillionUnits;
	}
	else
	{
		Stats.NumSecondPassPhotonsRequested = Scene.PhotonMappingSettings.IndirectPhotonDensity * SceneSurfaceAreaMillionUnits;
	}
	NumIndirectPhotonsToEmit = Min<QWORD>(Stats.NumSecondPassPhotonsRequested, (QWORD)MaxNumIndirectPhotons);
	if (NumIndirectPhotonsToEmit == MaxNumIndirectPhotons)
	{
		LogSolverMessage(FString::Printf(TEXT("Clamped the number of indirect photons to emit to %.3f million, from %.3f million requested."), MaxNumIndirectPhotons / 1000000.0f, Stats.NumSecondPassPhotonsRequested / 1000000.0f));
	}

	IndirectIrradiancePhotonFraction = Clamp(Scene.PhotonMappingSettings.IndirectIrradiancePhotonDensity / Scene.PhotonMappingSettings.IndirectPhotonDensity, 0.0f, 1.0f);

	// If the importance volume is valid, only gather enough caustic photons to meet CausticPhotonDensity inside the importance volume
	if (!PhotonMappingSettings.bEmitPhotonsOutsideImportanceVolume && ImportanceBounds.SphereRadius > DELTA)
	{
		NumCausticPhotons = appTrunc(Scene.PhotonMappingSettings.CausticPhotonDensity * ImportanceSurfaceAreaMillionUnits);
	}
	else if (ImportanceBounds.SphereRadius > DELTA)
	{
		NumCausticPhotons = appTrunc(Scene.PhotonMappingSettings.CausticPhotonDensity * ImportanceSurfaceAreaMillionUnits
			+ Scene.PhotonMappingSettings.OutsideImportanceVolumeDensityScale * Scene.PhotonMappingSettings.CausticPhotonDensity * SceneSurfaceAreaMillionUnits);
	}
	else
	{
		NumCausticPhotons = appTrunc(Scene.PhotonMappingSettings.CausticPhotonDensity * SceneSurfaceAreaMillionUnits);
	}
	NumCausticPhotons = NumCausticPhotons == appTruncErrorCode ? MaxNumIndirectPhotons : NumCausticPhotons;
	NumCausticPhotons = Min(NumCausticPhotons, MaxNumIndirectPhotons);
}

/** Emits photons, builds data structures to accelerate photon map lookups, and does any other photon preprocessing required. */
void FStaticLightingSystem::EmitPhotons()
{
	const FBoxSphereBounds SceneSphereBounds = FBoxSphereBounds(AggregateMesh.GetBounds());
	FBoxSphereBounds ImportanceVolumeBounds = GetImportanceBounds();
	if (ImportanceVolumeBounds.SphereRadius < DELTA)
	{
		ImportanceVolumeBounds = SceneSphereBounds;
	}
	
	// Presize for the results from two emitting passes (direct photon emitting, then indirect)
	IrradiancePhotons.Empty(NumPhotonWorkRanges * 2);

	const DOUBLE StartEmitDirectTime = appSeconds();
	TArray<TArray<FIndirectPathRay> > IndirectPathRays;
	// Emit photons for the direct photon map, and gather rays which resulted in indirect or caustic photon paths.
	EmitDirectPhotons(ImportanceVolumeBounds, IndirectPathRays, IrradiancePhotons);

	const DOUBLE EndEmitDirectTime = appSeconds();
	Stats.EmitDirectPhotonsTime = EndEmitDirectTime - StartEmitDirectTime;
	LogSolverMessage(FString::Printf(TEXT("EmitDirectPhotons complete, %.3f million photons emitted in %.1f seconds"), Stats.NumFirstPassPhotonsEmitted / 1000000.0f, Stats.EmitDirectPhotonsTime));

	// Let the scene's lights cache information about the indirect path rays, 
	// Which will be used to accelerate light sampling using those paths when emitting indirect photons.
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		FLight* CurrentLight = Lights(LightIndex);
		CurrentLight->CachePathRays(IndirectPathRays(LightIndex));
	}
	const DOUBLE EndCachingIndirectPathsTime = appSeconds();
	Stats.CachingIndirectPhotonPathsTime = EndCachingIndirectPathsTime - EndEmitDirectTime;

	// Emit photons for the indirect and caustic photon maps, using the indirect photon paths to guide photon emission.
	EmitIndirectPhotons(ImportanceVolumeBounds, IndirectPathRays, IrradiancePhotons);
	const DOUBLE EndEmitIndirectTime = appSeconds();
	Stats.EmitIndirectPhotonsTime = EndEmitIndirectTime - EndCachingIndirectPathsTime;
	LogSolverMessage(FString::Printf(TEXT("EmitIndirectPhotons complete, %.3f million photons emitted in %.1f seconds"), Stats.NumSecondPassPhotonsEmitted / 1000000.0f, Stats.EmitIndirectPhotonsTime));

	if (PhotonMappingSettings.bUseIrradiancePhotons)
	{
		// Process all irradiance photons and mark ones that have direct photons nearby,
		// So that we can search for those with a smaller radius when using them for rendering.
		// This allows more accurate direct shadow transitions with irradiance photons.
		MarkIrradiancePhotons(ImportanceVolumeBounds, IrradiancePhotons);
		const DOUBLE EndMarkIrradiancePhotonsTime = appSeconds();
		Stats.IrradiancePhotonMarkingTime = EndMarkIrradiancePhotonsTime - EndEmitIndirectTime;
		LogSolverMessage(FString::Printf(TEXT("Marking Irradiance Photons complete, %.3f million photons marked in %.1f seconds"), Stats.NumIrradiancePhotons / 1000000.0f, Stats.IrradiancePhotonMarkingTime));

		if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
		{
			// Cache irradiance photons on surfaces, as an optimization for final gathering.
			// Final gather rays already know which surface they intersected, so we can do a constant time lookup to find
			// The nearest irradiance photon instead of doing a photon map gather at the end of each final gather ray.
			// As an additional benefit, only cached irradiance photons are actually used for rendering, so we only need to calculate irradiance for the used ones.
			CacheIrradiancePhotons();
			Stats.CacheIrradiancePhotonsTime = appSeconds() - EndMarkIrradiancePhotonsTime;
			LogSolverMessage(FString::Printf(TEXT("Caching Irradiance Photons complete, %.3f million cache samples in %.1f seconds"), Stats.NumCachedIrradianceSamples / 1000000.0f, Stats.CacheIrradiancePhotonsTime));
		}
		// Calculate irradiance for photons found by the caching on surfaces pass.
		// This is done as an optimization to final gathering, as described in the paper titled "Faster Photon Map Global Illumination"
		// The optimization is that irradiance is pre-calculated at a subset of the photons so that final gather rays can just lookup the nearest irradiance photon,
		// Instead of doing a photon map search to calculate irradiance.
		const DOUBLE StartCalculateIrradiancePhotonsTime = appSeconds();
		CalculateIrradiancePhotons(ImportanceVolumeBounds, IrradiancePhotons);
		Stats.IrradiancePhotonCalculatingTime = appSeconds() - StartCalculateIrradiancePhotonsTime;
		LogSolverMessage(FString::Printf(TEXT("Calculate Irradiance Photons complete, %.3f million irradiance calculations in %.1f seconds"), Stats.NumFoundIrradiancePhotons / 1000000.0f, Stats.IrradiancePhotonCalculatingTime));

	}

	// Verify that temporary photon memory has been freed
	check(DirectPhotonEmittingWorkRanges.Num() == 0
		&& DirectPhotonEmittingOutputs.Num() == 0
		&& IndirectPhotonEmittingWorkRanges.Num() == 0
		&& IndirectPhotonEmittingOutputs.Num() == 0
		&& IrradianceMarkWorkRanges.Num() == 0
		&& IrradianceCalculationWorkRanges.Num() == 0
		&& IrradiancePhotonCachingThreads.Num() == 0);
}

/** Emits direct photons and generates indirect photon paths. */
void FStaticLightingSystem::EmitDirectPhotons(
	const FBoxSphereBounds& ImportanceBounds, 
	TArray<TArray<FIndirectPathRay> >& IndirectPathRays,
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons)
{
	//GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing0, 0 ) );
	FSceneLightPowerDistribution LightDistribution;
	// Create a 1d Step Probability Density Function based on the number of direct photons each light wants to gather.
	LightDistribution.LightPDFs.Empty(Lights.Num());
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		const FLight* CurrentLight = Lights(LightIndex);
		const INT LightNumDirectPhotons = CurrentLight->GetNumDirectPhotons(PhotonMappingSettings.DirectPhotonDensity);
		LightDistribution.LightPDFs.AddItem(LightNumDirectPhotons);
	}
	if (Lights.Num() > 0)
	{
		// Compute the Cumulative Distribution Function for our step function of light powers
		CalculateStep1dCDF(LightDistribution.LightPDFs, LightDistribution.LightCDFs, LightDistribution.UnnormalizedIntegral);
	}

	IndirectPathRays.Empty(Lights.Num());
	IndirectPathRays.AddZeroed(Lights.Num());
	// Add irradiance photon array entries for all the work ranges that will be processed
	const INT IrradianceArrayStart = IrradiancePhotons.AddZeroed(NumPhotonWorkRanges);

	const FDirectPhotonEmittingInput Input(ImportanceBounds, LightDistribution);

	// Setup work ranges, which are sections of work that can be done in parallel.
	DirectPhotonEmittingWorkRanges.Empty(NumPhotonWorkRanges);
	for (INT RangeIndex = 0; RangeIndex < NumPhotonWorkRanges - 1; RangeIndex++)
	{
		DirectPhotonEmittingWorkRanges.AddItem(FDirectPhotonEmittingWorkRange(RangeIndex, NumDirectPhotonsToEmit / NumPhotonWorkRanges, NumIndirectPhotonPaths / NumPhotonWorkRanges));
	}
	// The last work range contains the remainders
	DirectPhotonEmittingWorkRanges.AddItem(FDirectPhotonEmittingWorkRange(
		NumPhotonWorkRanges - 1, 
		NumDirectPhotonsToEmit / NumPhotonWorkRanges + NumDirectPhotonsToEmit % NumPhotonWorkRanges, 
		NumIndirectPhotonPaths / NumPhotonWorkRanges + NumIndirectPhotonPaths % NumPhotonWorkRanges));

	DirectPhotonEmittingOutputs.Empty(NumPhotonWorkRanges);
	for (INT RangeIndex = 0; RangeIndex < NumPhotonWorkRanges; RangeIndex++)
	{
		// Initialize outputs with the appropriate irradiance photon array
		DirectPhotonEmittingOutputs.AddItem(FDirectPhotonEmittingOutput(&IrradiancePhotons(IrradianceArrayStart + RangeIndex)));
	}

	// Spawn threads to emit direct photons
	TIndirectArray<FDirectPhotonEmittingThreadRunnable> DirectPhotonEmittingThreads;
	for (INT ThreadIndex = 1; ThreadIndex < NumStaticLightingThreads; ThreadIndex++)
	{
		FDirectPhotonEmittingThreadRunnable* ThreadRunnable = new(DirectPhotonEmittingThreads) FDirectPhotonEmittingThreadRunnable(this, ThreadIndex, Input);
		const FString ThreadName = FString::Printf(TEXT("DirectPhotonEmittingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}

	const DOUBLE StartEmittingDirectPhotonsMainThread = appSeconds();

	// Add the photons into a spatial data structure to accelerate their searches later
	//@todo - should try a kd-tree instead as the distribution of photons is highly non-uniform
	DirectPhotonMap = FPhotonOctree(ImportanceBounds.Origin, ImportanceBounds.BoxExtent.GetMax());
	IrradiancePhotonMap = FIrradiancePhotonOctree(ImportanceBounds.Origin, ImportanceBounds.BoxExtent.GetMax());

	Stats.NumDirectPhotonsGathered = 0;
	Stats.NumDirectIrradiancePhotons = 0;
	INT NumIndirectPhotonPathsGathered = 0;
	INT NextOutputToProcess = 0;
	while (DirectPhotonEmittingWorkRangeIndex.GetValue() < DirectPhotonEmittingWorkRanges.Num()
		|| NextOutputToProcess < DirectPhotonEmittingOutputs.Num())
	{
		// Process one work range on the main thread
		EmitDirectPhotonsThreadLoop(Input, 0);

		LIGHTINGSTAT(FScopedRDTSCTimer MainThreadProcessTimer(Stats.ProcessDirectPhotonsThreadTime));
		// Process the outputs that have been completed by any thread
		// Outputs are collected from smallest to largest work range index so that the outputs will be deterministic.
		while (NextOutputToProcess < DirectPhotonEmittingOutputs.Num() 
			&& DirectPhotonEmittingOutputs(NextOutputToProcess).OutputComplete > 0)
		{
			const FDirectPhotonEmittingOutput& CurrentOutput = DirectPhotonEmittingOutputs(NextOutputToProcess);
			for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.DirectPhotons.Num(); PhotonIndex++)
			{
				// Add direct photons to the direct photon map, which is an octree for now
				DirectPhotonMap.AddElement(FPhotonElement(CurrentOutput.DirectPhotons(PhotonIndex)));
			}

			for (INT LightIndex = 0; LightIndex < CurrentOutput.IndirectPathRays.Num(); LightIndex++)
			{
				// Gather indirect path rays
				IndirectPathRays(LightIndex).Append(CurrentOutput.IndirectPathRays(LightIndex));
				NumIndirectPhotonPathsGathered += CurrentOutput.IndirectPathRays(LightIndex).Num();
			}

			if (PhotonMappingSettings.bUseIrradiancePhotons)
			{
				for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.IrradiancePhotons->Num(); PhotonIndex++)
				{
					// Add the irradiance photons to an octree
					IrradiancePhotonMap.AddElement(FIrradiancePhotonElement(PhotonIndex, *CurrentOutput.IrradiancePhotons));
				}
				Stats.NumIrradiancePhotons += CurrentOutput.IrradiancePhotons->Num();
				Stats.NumDirectIrradiancePhotons += CurrentOutput.IrradiancePhotons->Num();
			}
			
			Stats.NumFirstPassPhotonsEmitted += CurrentOutput.NumPhotonsEmitted;
			NumPhotonsEmittedDirect += CurrentOutput.NumPhotonsEmittedDirect;
			Stats.NumDirectPhotonsGathered += CurrentOutput.DirectPhotons.Num();
			NextOutputToProcess++;
			Stats.DirectPhotonsTracingThreadTime += CurrentOutput.DirectPhotonsTracingThreadTime;
			Stats.DirectPhotonsLightSamplingThreadTime += CurrentOutput.DirectPhotonsLightSamplingThreadTime;
			Stats.DirectCustomAttenuationThreadTime += CurrentOutput.DirectCustomAttenuationThreadTime;
		}
	}

	Stats.EmitDirectPhotonsThreadTime = appSeconds() - StartEmittingDirectPhotonsMainThread;

	// Wait until all worker threads have completed
	for (INT ThreadIndex = 0; ThreadIndex < DirectPhotonEmittingThreads.Num(); ThreadIndex++)
	{
		DirectPhotonEmittingThreads(ThreadIndex).Thread->WaitForCompletion();
		DirectPhotonEmittingThreads(ThreadIndex).CheckHealth();
		GThreadFactory->Destroy(DirectPhotonEmittingThreads(ThreadIndex).Thread);
		Stats.EmitDirectPhotonsThreadTime += DirectPhotonEmittingThreads(ThreadIndex).ExecutionTime;
	}

	if (NumIndirectPhotonPathsGathered != NumIndirectPhotonPaths && GeneralSettings.NumIndirectLightingBounces > 0)
	{
		LogSolverMessage(FString::Printf(TEXT("Couldn't gather the requested number of indirect photon paths! %u gathered"), NumIndirectPhotonPathsGathered));
	}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (PhotonMappingSettings.bVisualizePhotonPaths)
	{
		if (GeneralSettings.ViewSingleBounceNumber < 0
			|| PhotonMappingSettings.bUsePhotonsForDirectLighting && GeneralSettings.ViewSingleBounceNumber == 0 
			|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1)
		{
			if (PhotonMappingSettings.bUseIrradiancePhotons)
			{
				INT NumDirectIrradiancePhotons = 0;
				for (INT ArrayIndex = 0; ArrayIndex < IrradiancePhotons.Num(); ArrayIndex++)
				{
					NumDirectIrradiancePhotons += IrradiancePhotons(ArrayIndex).Num();
				}
				DebugOutput.IrradiancePhotons.Empty(NumDirectIrradiancePhotons);
				for (INT ArrayIndex = 0; ArrayIndex < IrradiancePhotons.Num(); ArrayIndex++)
				{
					for (INT i = 0; i < IrradiancePhotons(ArrayIndex).Num(); i++)
					{
						DebugOutput.IrradiancePhotons.AddItem(FDebugPhoton(0, IrradiancePhotons(ArrayIndex)(i).GetPosition(), IrradiancePhotons(ArrayIndex)(i).GetSurfaceNormal(), IrradiancePhotons(ArrayIndex)(i).GetSurfaceNormal()));
					}
				}
			}
			else
			{
				DebugOutput.DirectPhotons.Empty(Stats.NumDirectPhotonsGathered);
				for (INT OutputIndex = 0; OutputIndex < DirectPhotonEmittingOutputs.Num(); OutputIndex++)
				{
					const FDirectPhotonEmittingOutput& CurrentOutput = DirectPhotonEmittingOutputs(OutputIndex);
					for (INT i = 0; i < CurrentOutput.DirectPhotons.Num(); i++)
					{
						DebugOutput.DirectPhotons.AddItem(FDebugPhoton(CurrentOutput.DirectPhotons(i).GetId(), CurrentOutput.DirectPhotons(i).GetPosition(), CurrentOutput.DirectPhotons(i).GetIncidentDirection(), CurrentOutput.DirectPhotons(i).GetSurfaceNormal()));
					}
				}
			}
		}
		if (GeneralSettings.ViewSingleBounceNumber != 0)
		{
			DebugOutput.IndirectPhotonPaths.Empty(NumIndirectPhotonPathsGathered);
			for (INT LightIndex = 0; LightIndex < IndirectPathRays.Num(); LightIndex++)
			{
				for (INT RayIndex = 0; RayIndex < IndirectPathRays(LightIndex).Num(); RayIndex++)
				{
					DebugOutput.IndirectPhotonPaths.AddItem(FDebugStaticLightingRay(
						IndirectPathRays(LightIndex)(RayIndex).Start, 
						IndirectPathRays(LightIndex)(RayIndex).Start + IndirectPathRays(LightIndex)(RayIndex).UnitDirection * IndirectPathRays(LightIndex)(RayIndex).Length,
						TRUE));
				}
			}
		}
	}
#endif

	DirectPhotonEmittingWorkRanges.Empty();
	DirectPhotonEmittingOutputs.Empty();

	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing0, 0 ) );
}

DWORD FDirectPhotonEmittingThreadRunnable::Run()
{
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing0, ThreadIndex ) );

	const DOUBLE StartThreadTime = appSeconds();
#if _MSC_VER && !XBOX
	if(!appIsDebuggerPresent())
	{
		__try
		{
			System->EmitDirectPhotonsThreadLoop(Input, ThreadIndex);
		}
		__except( WinCreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = appGetError();

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = TRUE;
		}
	}
	else
#endif
	{
		System->EmitDirectPhotonsThreadLoop(Input, ThreadIndex);
	}
	ExecutionTime = appSeconds() - StartThreadTime;
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing0, ThreadIndex ) );
	return 0;
}

/** Entrypoint for all threads emitting direct photons. */
void FStaticLightingSystem::EmitDirectPhotonsThreadLoop(const FDirectPhotonEmittingInput& Input, INT ThreadIndex)
{
	while (TRUE)
	{
		// Atomically read and increment the next work range index to process.
		// In this way work ranges are processed on-demand, which ensures consistent end times between threads.
		// Processing from smallest to largest work range index since the main thread is processing outputs in the same order.
		const INT RangeIndex = DirectPhotonEmittingWorkRangeIndex.Increment() - 1;
		if (RangeIndex < DirectPhotonEmittingWorkRanges.Num())
		{
			EmitDirectPhotonsWorkRange(Input, DirectPhotonEmittingWorkRanges(RangeIndex), DirectPhotonEmittingOutputs(RangeIndex));
			if (ThreadIndex == 0)
			{
				// Break out of the loop on the main thread after one work range so that it can process any outputs that are ready
				break;
			}
		}
		else
		{
			// Processing has begun for all work ranges
			break;
		}
	}
}

/** Emits direct photons for a given work range. */
void FStaticLightingSystem::EmitDirectPhotonsWorkRange(
	const FDirectPhotonEmittingInput& Input, 
	FDirectPhotonEmittingWorkRange WorkRange, 
	FDirectPhotonEmittingOutput& Output)
{
	// No lights in the scene, so no photons to emit
	if (Lights.Num() == 0
		// No light power in the scene, so no photons to shoot
		|| Input.LightDistribution.UnnormalizedIntegral < DELTA)
	{
		// Indicate to the main thread that this output is ready for processing
		appInterlockedIncrement(&DirectPhotonEmittingOutputs(WorkRange.RangeIndex).OutputComplete);
		return;
	}

	Output.IndirectPathRays.Empty(Lights.Num());
	Output.IndirectPathRays.AddZeroed(Lights.Num());
	for (INT LightIndex = 0; LightIndex < Output.IndirectPathRays.Num(); LightIndex++)
	{
		Output.IndirectPathRays(LightIndex).Empty(WorkRange.TargetNumIndirectPhotonPaths);
	}
	if (PhotonMappingSettings.bUseIrradiancePhotons)
	{
		// Attempt to preallocate irradiance photons based on the percentage of photons that go into the irradiance photon map.
		// The actual number of irradiance photons is based on probability.
		Output.IrradiancePhotons->Empty(appTrunc(DirectIrradiancePhotonFraction * DirectPhotonEfficiency * WorkRange.NumDirectPhotonsToEmit));
	}

	FCoherentRayCache CoherentRayCache;
	// Initialize the random stream using the work range's index,
	// So that different numbers will be generated for each work range, 
	// While maintaining determinism regardless of the order that work ranges are processed.
	FRandomStream RandomStream(WorkRange.RangeIndex);

	// Array of rays from each light which resulted in an indirect path.
	// These are used in the second emitting pass to guide light sampling for indirect and caustic photons.
	Output.DirectPhotons.Empty(appTrunc(WorkRange.NumDirectPhotonsToEmit * DirectPhotonEfficiency));
	
	Output.NumPhotonsEmitted = 0;
	INT NumIndirectPathRaysGathered = 0;

	// Emit photons until we reach the limit for this work range,
	while (Output.NumPhotonsEmitted < WorkRange.NumDirectPhotonsToEmit 
		// Or we haven't found enough indirect photon paths yet.
		|| NumIndirectPathRaysGathered < WorkRange.TargetNumIndirectPhotonPaths)
	{
		Output.NumPhotonsEmitted++;

		// Once we have emitted enough direct photons,
		// Stop emitting photons if we are getting below 0.2% efficiency for indirect photon paths
		// This can happen if the scene is close to convex
		if (Output.NumPhotonsEmitted >= WorkRange.NumDirectPhotonsToEmit 
			&& NumIndirectPathRaysGathered < WorkRange.TargetNumIndirectPhotonPaths
			&& Output.NumPhotonsEmitted > WorkRange.TargetNumIndirectPhotonPaths * 500.0f)
		{
			break;
		}

		INT NumberOfPathVertices = 0;
		FLOAT LightPDF;
		FLOAT LightIndex;
		// Pick a light with probability proportional to the light's fraction of the direct photons being gathered for the whole scene
		Sample1dCDF(Input.LightDistribution.LightPDFs, Input.LightDistribution.LightCDFs, Input.LightDistribution.UnnormalizedIntegral, RandomStream, LightPDF, LightIndex);
		const INT QuantizedLightIndex = appTrunc(LightIndex * Input.LightDistribution.LightPDFs.Num());
		check(QuantizedLightIndex >= 0 && QuantizedLightIndex < Lights.Num());
		const FLight* Light = Lights(QuantizedLightIndex);

		FLightRay SampleRay;
		FVector4 LightSourceNormal;
		FVector2D LightSurfacePosition;
		FLOAT RayDirectionPDF;
		FLinearColor PathAlpha;
		{
			LIGHTINGSTAT(FScopedRDTSCTimer LightSampleTimer(Output.DirectPhotonsLightSamplingThreadTime));
			// Generate the first ray for a new path from the light's distribution of emitted light
			Light->SampleDirection(RandomStream, SampleRay, LightSourceNormal, LightSurfacePosition, RayDirectionPDF, PathAlpha);
		}
		// Update the path's throughput based on the probability of picking this light and this direction
		PathAlpha = PathAlpha / (LightPDF * RayDirectionPDF);
		if (PathAlpha.R <= 0.0f && PathAlpha.G <= 0.0f && PathAlpha.B <= 0.0f)
		{
			// Skip to next photon since the light doesn't emit any energy in this direction
			continue;
		}

		const FLOAT BeforeDirectTraceTime = CoherentRayCache.FirstHitRayTraceTime;
		// Find the first vertex of the photon path
		FLightRayIntersection PathIntersection;
		AggregateMesh.IntersectLightRay(SampleRay, TRUE, TRUE, FALSE, CoherentRayCache, PathIntersection);
		Output.DirectPhotonsTracingThreadTime += CoherentRayCache.FirstHitRayTraceTime - BeforeDirectTraceTime;

		const FVector4 WorldPathDirection = SampleRay.Direction.UnsafeNormal();
		// Register this photon path as long as it hit a frontface of something in the scene
		if (PathIntersection.bIntersects && (WorldPathDirection | PathIntersection.IntersectionVertex.WorldTangentZ) < 0.0f)
		{
			{
				LIGHTINGSTAT(FScopedRDTSCTimer CustomAttenuationTimer(Output.DirectCustomAttenuationThreadTime));
				// Allow the light to attenuate in a non-physically correct way
				PathAlpha *= Light->CustomAttenuation(PathIntersection.IntersectionVertex.WorldPosition, RandomStream);
			}
			
			// Apply transmission
			PathAlpha *= PathIntersection.Transmission;

			if (PathAlpha.R < DELTA && PathAlpha.G < DELTA && PathAlpha.B < DELTA)
			{
				// Skip to the next photon since the path contribution was completely filtered out by transmission or attenuation
				continue;
			}

			NumberOfPathVertices++;
			// Note: SampleRay.Start is offset from the actual start position, but not enough to matter for the algorthims which make use of the photon's traveled distance.
			const FLOAT RayLength = (SampleRay.Start - PathIntersection.IntersectionVertex.WorldPosition).Size();
			// Create a photon from this path vertex's information
			const FPhoton NewPhoton(Output.NumPhotonsEmitted, PathIntersection.IntersectionVertex.WorldPosition, RayLength, -WorldPathDirection, PathIntersection.IntersectionVertex.WorldTangentZ, PathAlpha);
			checkSlow(PathAlpha.AreFloatsValid());
			if (Output.NumPhotonsEmitted < WorkRange.NumDirectPhotonsToEmit
				// Only deposit photons inside the importance bounds
				&& Input.ImportanceBounds.GetBox().IsInside(PathIntersection.IntersectionVertex.WorldPosition))
			{
				Output.DirectPhotons.AddItem(NewPhoton);
				Output.NumPhotonsEmittedDirect = Output.NumPhotonsEmitted;
				if (PhotonMappingSettings.bUseIrradiancePhotons 
					// Create an irradiance photon for a fraction of the direct photons
					&& RandomStream.GetFraction() < DirectIrradiancePhotonFraction)
				{
					const FIrradiancePhoton NewIrradiancePhoton(PathIntersection.IntersectionVertex.WorldPosition, PathIntersection.IntersectionVertex.WorldTangentZ, TRUE);
					Output.IrradiancePhotons->AddItem(NewIrradiancePhoton);
				}
			}

			// Continue tracing this path if we don't have enough indirect photon paths yet
			if (NumIndirectPathRaysGathered < WorkRange.TargetNumIndirectPhotonPaths)
			{
				PathIntersection.IntersectionVertex.GenerateVertexTangents();
				FVector4 NewWorldPathDirection;
				FLOAT BRDFDirectionPDF;
				UBOOL bSpecularBounce;

				// Generate a new path direction from the BRDF
				const FLinearColor BRDF = PathIntersection.Mesh->SampleBRDF(
					PathIntersection.IntersectionVertex, 
					PathIntersection.ElementIndex,
					-WorldPathDirection, 
					NewWorldPathDirection, 
					BRDFDirectionPDF, 
					bSpecularBounce, 
					RandomStream);

				// Terminate if the path has lost all of its energy due to the surface's BRDF
				if (BRDF.Equals(FLinearColor::Black))
				{
					continue;
				}

				const FLOAT CosFactor = -WorldPathDirection | PathIntersection.IntersectionVertex.WorldTangentZ;
				checkSlow(CosFactor >= 0.0f && CosFactor <= 1.0f);

				const FVector4 RayStart = PathIntersection.IntersectionVertex.WorldPosition 
					+ NewWorldPathDirection * SceneConstants.VisibilityRayOffsetDistance 
					+ PathIntersection.IntersectionVertex.WorldTangentZ * SceneConstants.VisibilityNormalOffsetDistance;
				const FVector4 RayEnd = PathIntersection.IntersectionVertex.WorldPosition + NewWorldPathDirection * MaxRayDistance;

				const FLightRay IndirectSampleRay(
					RayStart,
					RayEnd,
					NULL,
					NULL
					); 

				const FLOAT BeforeIndirectTraceTime = CoherentRayCache.FirstHitRayTraceTime;
				FLightRayIntersection NewPathIntersection;
				AggregateMesh.IntersectLightRay(IndirectSampleRay, TRUE, FALSE, FALSE, CoherentRayCache, NewPathIntersection);
				Output.DirectPhotonsTracingThreadTime += CoherentRayCache.FirstHitRayTraceTime - BeforeIndirectTraceTime;

				if (NewPathIntersection.bIntersects && (NewWorldPathDirection | NewPathIntersection.IntersectionVertex.WorldTangentZ) < 0.0f)
				{
					// Store the original photon path which led to an indirect photon path,
					// This will be used in a second pass to guide indirect photon emission.
					Output.IndirectPathRays(QuantizedLightIndex).AddItem(FIndirectPathRay(SampleRay.Start, WorldPathDirection, LightSourceNormal, LightSurfacePosition, RayLength));
					NumIndirectPathRaysGathered++;
				}
			}
		}
	}
	// Indicate to the main thread that this output is ready for processing
	appInterlockedIncrement(&DirectPhotonEmittingOutputs(WorkRange.RangeIndex).OutputComplete);
}

/** Gathers indirect and caustic photons based on the indirect photon paths. */
void FStaticLightingSystem::EmitIndirectPhotons(
	const FBoxSphereBounds& ImportanceBounds,
	const TArray<TArray<FIndirectPathRay> >& IndirectPathRays, 
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons)
{
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing1, 0 ) );
	FSceneLightPowerDistribution LightDistribution;
	// Create a 1d Step Probability Density Function based on light powers,
	// So that lights are chosen with a probability proportional to their fraction of the total light power in the scene.
	LightDistribution.LightPDFs.Empty(Lights.Num());
	for (INT LightIndex = 0; LightIndex < Lights.Num(); LightIndex++)
	{
		const FLight* CurrentLight = Lights(LightIndex);
		LightDistribution.LightPDFs.AddItem(CurrentLight->Power());
	}

	if (Lights.Num() > 0)
	{
		// Compute the Cumulative Distribution Function for our step function of light powers
		CalculateStep1dCDF(LightDistribution.LightPDFs, LightDistribution.LightCDFs, LightDistribution.UnnormalizedIntegral);
	}
	// Add irradiance photon array entries for all the work ranges that will be processed
	const INT IndirectIrradianceArrayStart = IrradiancePhotons.AddZeroed(NumPhotonWorkRanges);
	const FIndirectPhotonEmittingInput Input(ImportanceBounds, LightDistribution, IndirectPathRays);

	// Setup work ranges, which are sections of work that can be done in parallel.
	IndirectPhotonEmittingWorkRanges.Empty(NumPhotonWorkRanges);
	for (INT RangeIndex = 0; RangeIndex < NumPhotonWorkRanges - 1; RangeIndex++)
	{
		IndirectPhotonEmittingWorkRanges.AddItem(FIndirectPhotonEmittingWorkRange(RangeIndex, NumIndirectPhotonsToEmit / NumPhotonWorkRanges, NumCausticPhotons / NumPhotonWorkRanges));
	}
	// The last work range will contain the remainder of photons
	IndirectPhotonEmittingWorkRanges.AddItem(FIndirectPhotonEmittingWorkRange(
		NumPhotonWorkRanges - 1, 
		NumIndirectPhotonsToEmit / NumPhotonWorkRanges + NumIndirectPhotonsToEmit % NumPhotonWorkRanges, 
		NumCausticPhotons / NumPhotonWorkRanges + NumCausticPhotons % NumPhotonWorkRanges));

	IndirectPhotonEmittingOutputs.Empty(NumPhotonWorkRanges);
	for (INT RangeIndex = 0; RangeIndex < NumPhotonWorkRanges; RangeIndex++)
	{
		// Initialize outputs with the appropriate irradiance photon array
		IndirectPhotonEmittingOutputs.AddItem(FIndirectPhotonEmittingOutput(&IrradiancePhotons(IndirectIrradianceArrayStart + RangeIndex)));
	}

	// Spawn threads to emit indirect photons
	TIndirectArray<FIndirectPhotonEmittingThreadRunnable> IndirectPhotonEmittingThreads;
	for (INT ThreadIndex = 1; ThreadIndex < NumStaticLightingThreads; ThreadIndex++)
	{
		FIndirectPhotonEmittingThreadRunnable* ThreadRunnable = new(IndirectPhotonEmittingThreads) FIndirectPhotonEmittingThreadRunnable(this, ThreadIndex, Input);
		const FString ThreadName = FString::Printf(TEXT("IndirectPhotonEmittingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}

	const DOUBLE StartEmittingIndirectPhotonsMainThread = appSeconds();

	// Add the photons into spatial data structures to accelerate their searches later
	FirstBouncePhotonMap = FPhotonOctree(ImportanceBounds.Origin, ImportanceBounds.BoxExtent.GetMax());
	SecondBouncePhotonMap = FPhotonOctree(ImportanceBounds.Origin, ImportanceBounds.BoxExtent.GetMax());
	CausticPhotonMap = FPhotonOctree(ImportanceBounds.Origin, ImportanceBounds.BoxExtent.GetMax());

	Stats.NumIndirectPhotonsGathered = 0;
	INT NumCausticPhotonsGathered = 0;
	INT NextOutputToProcess = 0;
	while (IndirectPhotonEmittingWorkRangeIndex.GetValue() < IndirectPhotonEmittingWorkRanges.Num()
		|| NextOutputToProcess < IndirectPhotonEmittingOutputs.Num())
	{
		// Process one work range on the main thread
		EmitIndirectPhotonsThreadLoop(Input, 0);

		LIGHTINGSTAT(FScopedRDTSCTimer MainThreadProcessTimer(Stats.ProcessIndirectPhotonsThreadTime));
		// Process the thread's outputs
		// Outputs are collected from smallest to largest work range index so that the outputs will be deterministic.
		while (NextOutputToProcess < IndirectPhotonEmittingOutputs.Num() 
			&& IndirectPhotonEmittingOutputs(NextOutputToProcess).OutputComplete > 0)
		{
			FIndirectPhotonEmittingOutput& CurrentOutput = IndirectPhotonEmittingOutputs(NextOutputToProcess);
			for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.FirstBouncePhotons.Num(); PhotonIndex++)
			{
				FirstBouncePhotonMap.AddElement(FPhotonElement(CurrentOutput.FirstBouncePhotons(PhotonIndex)));
			}

			for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.SecondBouncePhotons.Num(); PhotonIndex++)
			{
				SecondBouncePhotonMap.AddElement(FPhotonElement(CurrentOutput.SecondBouncePhotons(PhotonIndex)));
			}

			for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.CausticPhotons.Num(); PhotonIndex++)
			{
				CausticPhotonMap.AddElement(FPhotonElement(CurrentOutput.CausticPhotons(PhotonIndex)));
			}

			if (PhotonMappingSettings.bUseIrradiancePhotons)
			{
				for (INT PhotonIndex = 0; PhotonIndex < CurrentOutput.IrradiancePhotons->Num(); PhotonIndex++)
				{
					// Add the irradiance photons to an octree
					IrradiancePhotonMap.AddElement(FIrradiancePhotonElement(PhotonIndex, *CurrentOutput.IrradiancePhotons));
				}
				Stats.NumIrradiancePhotons += CurrentOutput.IrradiancePhotons->Num();
			}

			Stats.NumSecondPassPhotonsEmitted += CurrentOutput.NumPhotonsEmitted;
			Stats.LightSamplingThreadTime += CurrentOutput.LightSamplingThreadTime;
			Stats.IndirectCustomAttenuationThreadTime += CurrentOutput.IndirectCustomAttenuationThreadTime;
			Stats.IntersectLightRayThreadTime += CurrentOutput.IntersectLightRayThreadTime;
			Stats.PhotonBounceTracingThreadTime += CurrentOutput.PhotonBounceTracingThreadTime;
			NumPhotonsEmittedFirstBounce += CurrentOutput.NumPhotonsEmittedFirstBounce;
			NumPhotonsEmittedSecondBounce += CurrentOutput.NumPhotonsEmittedSecondBounce;
			NumPhotonsEmittedCaustic += CurrentOutput.NumPhotonsEmittedCaustic;

			Stats.NumIndirectPhotonsGathered += CurrentOutput.FirstBouncePhotons.Num() + CurrentOutput.SecondBouncePhotons.Num();
			NumCausticPhotonsGathered += CurrentOutput.CausticPhotons.Num();
			NextOutputToProcess++;
			CurrentOutput.FirstBouncePhotons.Empty();
			CurrentOutput.SecondBouncePhotons.Empty();
			CurrentOutput.CausticPhotons.Empty();
		}
	}

	Stats.EmitIndirectPhotonsThreadTime = appSeconds() - StartEmittingIndirectPhotonsMainThread;

	// Wait until all worker threads have completed
	for (INT ThreadIndex = 0; ThreadIndex < IndirectPhotonEmittingThreads.Num(); ThreadIndex++)
	{
		IndirectPhotonEmittingThreads(ThreadIndex).Thread->WaitForCompletion();
		IndirectPhotonEmittingThreads(ThreadIndex).CheckHealth();
		GThreadFactory->Destroy(IndirectPhotonEmittingThreads(ThreadIndex).Thread);
		Stats.EmitIndirectPhotonsThreadTime += IndirectPhotonEmittingThreads(ThreadIndex).ExecutionTime;
	}

	if (NumCausticPhotonsGathered != NumCausticPhotons && GeneralSettings.NumIndirectLightingBounces > 0)
	{
		LogSolverMessage(FString::Printf(TEXT("Couldn't gather the requested number of caustic photons! %u gathered"), NumCausticPhotonsGathered));
	}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (PhotonMappingSettings.bVisualizePhotonPaths
		&& PhotonMappingSettings.bUseIrradiancePhotons 
		&& GeneralSettings.ViewSingleBounceNumber != 0)
	{
		INT NumIndirectIrradiancePhotons = 0;
		for (INT RangeIndex = NumPhotonWorkRanges; RangeIndex < IrradiancePhotons.Num(); RangeIndex++)
		{
			NumIndirectIrradiancePhotons += IrradiancePhotons(RangeIndex).Num();
		}
		DebugOutput.IrradiancePhotons.Empty(NumIndirectIrradiancePhotons);
		for (INT RangeIndex = NumPhotonWorkRanges; RangeIndex < IrradiancePhotons.Num(); RangeIndex++)
		{
			for (INT i = 0; i < IrradiancePhotons(RangeIndex).Num(); i++)
			{
				DebugOutput.IrradiancePhotons.AddItem(FDebugPhoton(0, IrradiancePhotons(RangeIndex)(i).GetPosition(), IrradiancePhotons(RangeIndex)(i).GetSurfaceNormal(), IrradiancePhotons(RangeIndex)(i).GetSurfaceNormal()));
			}
		}
	}
#endif
	
	IndirectPhotonEmittingWorkRanges.Empty();
	IndirectPhotonEmittingOutputs.Empty();
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing1, 0 ) );
}

DWORD FIndirectPhotonEmittingThreadRunnable::Run()
{
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing1, ThreadIndex ) );
	const DOUBLE StartThreadTime = appSeconds();
#if _MSC_VER && !XBOX
	if(!appIsDebuggerPresent())
	{
		__try
		{
			System->EmitIndirectPhotonsThreadLoop(Input, ThreadIndex);
		}
		__except( WinCreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = appGetError();

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = TRUE;
		}
	}
	else
#endif
	{
		System->EmitIndirectPhotonsThreadLoop(Input, ThreadIndex);
	}
	const DOUBLE EndThreadTime = appSeconds();
	EndTime = EndThreadTime - GStartupTime;
	ExecutionTime = EndThreadTime - StartThreadTime;

	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing1, ThreadIndex ) );
	return 0;
}

/** Entrypoint for all threads emitting indirect photons. */
void FStaticLightingSystem::EmitIndirectPhotonsThreadLoop(const FIndirectPhotonEmittingInput& Input, INT ThreadIndex)
{
	while (TRUE)
	{
		// Atomically read and increment the next work range index to process.
		// In this way work ranges are processed on-demand, which ensures consistent end times between threads.
		// Processing from smallest to largest work range index since the main thread is processing outputs in the same order.
		const INT RangeIndex = IndirectPhotonEmittingWorkRangeIndex.Increment() - 1;
		if (RangeIndex < IndirectPhotonEmittingWorkRanges.Num())
		{
			EmitIndirectPhotonsWorkRange(Input, IndirectPhotonEmittingWorkRanges(RangeIndex), IndirectPhotonEmittingOutputs(RangeIndex));
			if (ThreadIndex == 0)
			{
				// Break out of the loop on the main thread after one work range so that it can process any outputs that are ready
				break;
			}
		}
		else
		{
			// Processing has begun for all work ranges
			break;
		}
	}
}

/** Emits indirect photons for a given work range. */
void FStaticLightingSystem::EmitIndirectPhotonsWorkRange(
	const FIndirectPhotonEmittingInput& Input, 
	FIndirectPhotonEmittingWorkRange WorkRange, 
	FIndirectPhotonEmittingOutput& Output)
{
	if (Input.IndirectPathRays.Num() == 0 || Input.LightDistribution.UnnormalizedIntegral < DELTA)
	{
		// No lights in the scene, so no photons to emit
		// Indicate to the main thread that this output is ready for processing
		appInterlockedIncrement(&IndirectPhotonEmittingOutputs(WorkRange.RangeIndex).OutputComplete);
		return;
	}

	//@todo - re-evaluate these sizes
	Output.FirstBouncePhotons.Empty(appTrunc(WorkRange.NumIndirectPhotonsToEmit * .6f * IndirectPhotonEfficiency));
	Output.SecondBouncePhotons.Empty(appTrunc(WorkRange.NumIndirectPhotonsToEmit * .4f * IndirectPhotonEfficiency));
	Output.CausticPhotons.Empty(WorkRange.TargetNumCausticPhotons);
	if (PhotonMappingSettings.bUseIrradiancePhotons)
	{
		// Attempt to preallocate irradiance photons based on the percentage of photons that go into the irradiance photon map.
		// The actual number of irradiance photons is based on probability.
		Output.IrradiancePhotons->Empty(appTrunc(IndirectIrradiancePhotonFraction * IndirectPhotonEfficiency * WorkRange.NumIndirectPhotonsToEmit));
	}

	FCoherentRayCache CoherentRayCache;
	// Seed the random number generator at the beginning of each work range, so we get deterministic results regardless of the number of threads being used.
	FRandomStream RandomStream(WorkRange.RangeIndex);

	const UBOOL bIndirectPhotonsNeeded = WorkRange.NumIndirectPhotonsToEmit > 0 && GeneralSettings.NumIndirectLightingBounces > 0;
	const UBOOL bCausticPhotonsNeeded = WorkRange.TargetNumCausticPhotons > 0 && GeneralSettings.NumIndirectLightingBounces > 0;

	Output.NumPhotonsEmitted = 0;

	// Emit photons until we reach the limit for this work range,
	while (bIndirectPhotonsNeeded && Output.NumPhotonsEmitted < WorkRange.NumIndirectPhotonsToEmit
		// Or if caustic photons are needed and we don't have enough.
		|| bCausticPhotonsNeeded && Output.CausticPhotons.Num() != WorkRange.TargetNumCausticPhotons)
	{
		// Once we have enough indirect photons,
		// Stop emitting photons if we are getting below 0.2% efficiency for caustic photons
		// This can happen if the scene is full of purely diffuse surfaces
		if (Output.NumPhotonsEmitted >= WorkRange.NumIndirectPhotonsToEmit
			&& bCausticPhotonsNeeded 
			&& Output.CausticPhotons.Num() != WorkRange.TargetNumCausticPhotons
			&& Output.NumPhotonsEmitted > WorkRange.TargetNumCausticPhotons * 500.0f)
		{
			break;
		}

		Output.NumPhotonsEmitted++;

		INT NumberOfPathVertices = 0;

		FLightRay SampleRay;
		FLinearColor PathAlpha;
		const FLight* Light = NULL;
		{
			LIGHTINGSTAT(FScopedRDTSCTimer SampleLightTimer(Output.LightSamplingThreadTime));
			FLOAT LightPDF;
			FLOAT LightIndex;
			// Pick a light with probability proportional to the light's fraction of the scene's light power
			Sample1dCDF(Input.LightDistribution.LightPDFs, Input.LightDistribution.LightCDFs, Input.LightDistribution.UnnormalizedIntegral, RandomStream, LightPDF, LightIndex);
			const INT QuantizedLightIndex = appTrunc(LightIndex * Input.LightDistribution.LightPDFs.Num());
			check(QuantizedLightIndex >= 0 && QuantizedLightIndex < Lights.Num());
			Light = Lights(QuantizedLightIndex);

			FLOAT RayDirectionPDF;
			if (Input.IndirectPathRays(QuantizedLightIndex).Num() > 0)
			{
				// Use the indirect path rays to sample the light
				Light->SampleDirection(
					Input.IndirectPathRays(QuantizedLightIndex), 
					RandomStream, 
					SampleRay, 
					RayDirectionPDF, 
					PathAlpha);
			}
			else
			{
				FVector4 LightSourceNormal;
				FVector2D LightSurfacePosition;
				// No indirect path rays from this light, sample it uniformly
				Light->SampleDirection(RandomStream, SampleRay, LightSourceNormal, LightSurfacePosition, RayDirectionPDF, PathAlpha);
			}
			// Update the path's throughput based on the probability of picking this light and this direction
			PathAlpha = PathAlpha / (LightPDF * RayDirectionPDF);
			checkSlow(PathAlpha.AreFloatsValid());
			if (PathAlpha.R < DELTA && PathAlpha.G < DELTA && PathAlpha.B < DELTA)
			{
				// Skip to the next photon since the light doesn't emit any energy in this direction
				continue;
			}

			// Clip the end of the photon path to the importance volume, or skip if the photon path does not intersect the importance volume at all.
			FVector4 ClippedStart, ClippedEnd;
			if (!ClipLineWithBox(Input.ImportanceBounds.GetBox(), SampleRay.Start, SampleRay.End, ClippedStart, ClippedEnd))
			{
				continue;
			}
			SampleRay.End = ClippedEnd;
		}

		UBOOL bCausticPath = FALSE;
		// Find the first vertex of the photon path
		FLightRayIntersection PathIntersection;
		const FLOAT BeforeLightRayTime = CoherentRayCache.FirstHitRayTraceTime;
		AggregateMesh.IntersectLightRay(SampleRay, TRUE, TRUE, FALSE, CoherentRayCache, PathIntersection);
		Output.IntersectLightRayThreadTime += CoherentRayCache.FirstHitRayTraceTime - BeforeLightRayTime;

		LIGHTINGSTAT(FScopedRDTSCTimer PhotonTracingTimer(Output.PhotonBounceTracingThreadTime));
		FVector4 WorldPathDirection = SampleRay.Direction.UnsafeNormal();
		// Continue tracing this photon path as long as it hit a frontface of something in the scene
		while (PathIntersection.bIntersects && (WorldPathDirection | PathIntersection.IntersectionVertex.WorldTangentZ) < 0.0f)
		{
			if (NumberOfPathVertices == 0)
			{
				LIGHTINGSTAT(FScopedRDTSCTimer CustomAttenuationTimer(Output.IndirectCustomAttenuationThreadTime));
				// Allow the light to attenuate in a non-physically correct way
				PathAlpha *= Light->CustomAttenuation(PathIntersection.IntersectionVertex.WorldPosition, RandomStream);
			}

			// Apply transmission
			PathAlpha *= PathIntersection.Transmission;

			if (PathAlpha.R < DELTA && PathAlpha.G < DELTA && PathAlpha.B < DELTA)
			{
				// Skip to the next photon since the light was completely filtered out by transmission or attenuation
				break;
			}

			NumberOfPathVertices++;

			// Note: SampleRay.Start is offset from the actual start position, but not enough to matter for the algorthims which make use of the photon's traveled distance.
			const FLOAT RayLength = (SampleRay.Start - PathIntersection.IntersectionVertex.WorldPosition).Size();
			// Create a photon from this path vertex's information
			const FPhoton NewPhoton(Output.NumPhotonsEmitted, PathIntersection.IntersectionVertex.WorldPosition, RayLength, -WorldPathDirection, PathIntersection.IntersectionVertex.WorldTangentZ, PathAlpha);
			checkSlow(PathAlpha.AreFloatsValid());
			// Only deposit photons inside the importance bounds
			if (Input.ImportanceBounds.GetBox().IsInside(PathIntersection.IntersectionVertex.WorldPosition))
			{
				if (bCausticPath 
					&& Output.CausticPhotons.Num() < WorkRange.TargetNumCausticPhotons 
					// Only record caustic photons for the first bounce, because bounces after that are too splotchy
					//@todo - make this a configurable setting
					&& NumberOfPathVertices == 2)
				{
					Output.CausticPhotons.AddItem(NewPhoton);
					Output.NumPhotonsEmittedCaustic = Output.NumPhotonsEmitted;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
					if (PhotonMappingSettings.bVisualizePhotonPaths 
						&& !PhotonMappingSettings.bUseIrradiancePhotons
						&& GeneralSettings.ViewSingleBounceNumber != 0)
					{
						FScopeLock DebugOutputLock(&DebugOutputSync);
						if (DebugOutput.IndirectPhotons.Num() == 0)
						{
							DebugOutput.IndirectPhotons.Empty(appTrunc(NumIndirectPhotonsToEmit * IndirectPhotonEfficiency));
						}
						DebugOutput.IndirectPhotons.AddItem(FDebugPhoton(NewPhoton.GetId(), NewPhoton.GetPosition(), SampleRay.Start - NewPhoton.GetPosition(), NewPhoton.GetSurfaceNormal()));
					}
#endif
				}
				else if (!bCausticPath)
				{
					// Only deposit a photon for non-caustic paths if it is not a direct lighting path, and we still need to gather more indirect photons
					if (NumberOfPathVertices > 1 && Output.NumPhotonsEmitted < WorkRange.NumIndirectPhotonsToEmit)
					{
						UBOOL bShouldCreateIrradiancePhoton = FALSE;
						if (NumberOfPathVertices == 2)
						{ 
							// This is a first bounce photon
							Output.FirstBouncePhotons.AddItem(NewPhoton);
							Output.NumPhotonsEmittedFirstBounce = Output.NumPhotonsEmitted;
							// Only allow creating an irradiance photon if one or more indirect bounces are required
							// The final gather is the first bounce when enabled
							bShouldCreateIrradiancePhoton = 
								PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 1
								|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0;
						}
						else
						{
							Output.SecondBouncePhotons.AddItem(NewPhoton);
							Output.NumPhotonsEmittedSecondBounce = Output.NumPhotonsEmitted;
							// Only allow creating an irradiance photon if two or more indirect bounces are required
							// The final gather is the first bounce when enabled
							bShouldCreateIrradiancePhoton = 
								PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 2
								|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 1;
						}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
						if (PhotonMappingSettings.bVisualizePhotonPaths
							&& !PhotonMappingSettings.bUseIrradiancePhotons
							&& (GeneralSettings.ViewSingleBounceNumber < 0
							|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber > 1 
							|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber > 0))
						{
							FScopeLock DebugOutputLock(&DebugOutputSync);
							if (DebugOutput.IndirectPhotons.Num() == 0)
							{
								DebugOutput.IndirectPhotons.Empty(appTrunc(NumIndirectPhotonsToEmit * IndirectPhotonEfficiency));
							}
							DebugOutput.IndirectPhotons.AddItem(FDebugPhoton(NewPhoton.GetId(), NewPhoton.GetPosition(), SampleRay.Start - NewPhoton.GetPosition(), NewPhoton.GetSurfaceNormal()));
						}
#endif
						// Create an irradiance photon for a fraction of the deposited photons
						if (PhotonMappingSettings.bUseIrradiancePhotons 
							&& bShouldCreateIrradiancePhoton
							&& RandomStream.GetFraction() < IndirectIrradiancePhotonFraction)
						{
							const FIrradiancePhoton NewIrradiancePhoton(NewPhoton.GetPosition(), PathIntersection.IntersectionVertex.WorldTangentZ, FALSE);
							Output.IrradiancePhotons->AddItem(NewIrradiancePhoton);
						}
					}
				}
			}

			if (NumberOfPathVertices > GeneralSettings.NumIndirectLightingBounces)
			{
				// Stop tracing this photon due to bounce number
				break;
			}

			PathIntersection.IntersectionVertex.GenerateVertexTangents();
			FVector4 NewWorldPathDirection;
			FLOAT BRDFDirectionPDF;
			UBOOL bSpecularBounce;

			// Generate a new path direction from the BRDF
			const FLinearColor BRDF = PathIntersection.Mesh->SampleBRDF(
				PathIntersection.IntersectionVertex, 
				PathIntersection.ElementIndex,
				-WorldPathDirection, 
				NewWorldPathDirection, 
				BRDFDirectionPDF, 
				bSpecularBounce, 
				RandomStream);

			// Only a specular bounce on the first path intersection can turn a path into a caustic path
			if (bSpecularBounce && NumberOfPathVertices == 1)
			{
				bCausticPath = TRUE;
			}
			// The first non-caustic bounce turns this path into a normal path
			else if (!bSpecularBounce)
			{
				bCausticPath = FALSE;
			}

			// Terminate if the path has lost all of its energy due to the surface's BRDF
			if (BRDF.Equals(FLinearColor::Black)
				// Terminate if indirect photons are completed (so only caustic photons can be deposited at this point)
				// And the path is not a caustic path, so it will never go into a caustic photon map.
				|| Output.NumPhotonsEmitted >= WorkRange.NumIndirectPhotonsToEmit && !bCausticPath)
			{
				break;
			}

			const FLOAT CosFactor = -WorldPathDirection | PathIntersection.IntersectionVertex.WorldTangentZ;
			checkSlow(CosFactor >= 0.0f && CosFactor <= 1.0f);
			if (NumberOfPathVertices == 1)
			{
				// On the first bounce, re-weight the photon's throughput instead of using Russian Roulette to maintain equal weights,
				// Because NumEmitted/NumDeposited efficiency is more important to first bounce photons than having equal weight,
				// Since they are used for importance sampling the final gather.
				// Re-weight the throughput based on the probability of surviving.
				PathAlpha = PathAlpha * BRDF * CosFactor / BRDFDirectionPDF;
			}
			else
			{
				const FLinearColor NewPathAlpha = PathAlpha * BRDF * CosFactor / BRDFDirectionPDF;
				// On second and up bounces, terminate the path with probability proportional to the ratio between the new throughput and the old
				// This results in a smaller number of photons after surface reflections, but they have the same weight as before the reflection,
				// Which is desirable to reduce noise from the photon maps, at the cost of lower NumEmitted/NumDeposited efficiency.
				// See the "Extended Photon Map Implementation" paper for details.

				// Note: to be physically correct this probability should be clamped to [0,1], however this produces photons with extremely large weights,
				// So instead we maintain a constant photon weight after the surface interaction,
				// At the cost of introducing bias and leaking energy for bounces where BRDF * CosFactor / BRDFDirectionPDF > 1.
				const FLOAT ContinueProbability = NewPathAlpha.LinearRGBToXYZ().G / PathAlpha.LinearRGBToXYZ().G;
				const FLOAT RandomFloat = RandomStream.GetFraction();
				if (RandomFloat > ContinueProbability)
				{
					// Terminate due to Russian Roulette
					break;
				}
				PathAlpha = NewPathAlpha / ContinueProbability;
			}

			checkSlow(PathAlpha.AreFloatsValid());

			const FVector4 RayStart = PathIntersection.IntersectionVertex.WorldPosition 
				+ NewWorldPathDirection * SceneConstants.VisibilityRayOffsetDistance 
				+ PathIntersection.IntersectionVertex.WorldTangentZ * SceneConstants.VisibilityNormalOffsetDistance;
			FVector4 RayEnd = PathIntersection.IntersectionVertex.WorldPosition + NewWorldPathDirection * MaxRayDistance;

			// Clip photon path end points to the importance volume, so we do not bother tracing rays outside the area that photons can be deposited.
			// If the photon path does not intersect the importance volume at all, it did not originate from inside the volume, so skip to the next photon.
			FVector4 ClippedStart, ClippedEnd;
			if (!ClipLineWithBox(Input.ImportanceBounds.GetBox(), RayStart, RayEnd, ClippedStart, ClippedEnd))
			{
				break;
			}
			RayEnd = ClippedEnd;

			SampleRay = FLightRay(
				RayStart,
				RayEnd,
				NULL,
				NULL
				); 

			// Trace a ray to determine the next vertex of the photon's path.
			AggregateMesh.IntersectLightRay(SampleRay, TRUE, TRUE, FALSE, CoherentRayCache, PathIntersection);
			WorldPathDirection = NewWorldPathDirection;
		}
	}
	// Indicate to the main thread that this output is ready for processing
	appInterlockedIncrement(&IndirectPhotonEmittingOutputs(WorkRange.RangeIndex).OutputComplete);
}

/** Iterates through all irradiance photons, searches for nearby direct photons, and marks the irradiance photon has having direct photon influence if necessary. */
void FStaticLightingSystem::MarkIrradiancePhotons(const FBoxSphereBounds& ImportanceBounds, TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons)
{
	check(PhotonMappingSettings.bUseIrradiancePhotons);
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing2, 0 ) );

	// Setup work ranges for processing the irradiance photons
	IrradianceMarkWorkRanges.Empty(IrradiancePhotons.Num());
	for (INT WorkRange = 0; WorkRange < IrradiancePhotons.Num(); WorkRange++)
	{
		IrradianceMarkWorkRanges.AddItem(FIrradianceMarkingWorkRange(WorkRange, WorkRange));
	}

	TIndirectArray<FIrradiancePhotonMarkingThreadRunnable> IrradiancePhotonMarkingThreads;
	IrradiancePhotonMarkingThreads.Empty(NumStaticLightingThreads);
	for(INT ThreadIndex = 1; ThreadIndex < NumStaticLightingThreads; ThreadIndex++)
	{
		FIrradiancePhotonMarkingThreadRunnable* ThreadRunnable = new(IrradiancePhotonMarkingThreads) FIrradiancePhotonMarkingThreadRunnable(this, ThreadIndex, IrradiancePhotons);
		const FString ThreadName = FString::Printf(TEXT("IrradiancePhotonMarkingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}

	const DOUBLE MainThreadStartTime = appSeconds();

	MarkIrradiancePhotonsThreadLoop(0, IrradiancePhotons);
	
	Stats.IrradiancePhotonMarkingThreadTime = appSeconds() - MainThreadStartTime;

	// Stop the static lighting threads.
	for(INT ThreadIndex = 0;ThreadIndex < IrradiancePhotonMarkingThreads.Num();ThreadIndex++)
	{
		// Wait for the thread to exit.
		IrradiancePhotonMarkingThreads(ThreadIndex).Thread->WaitForCompletion();

		// Check that it didn't terminate with an error.
		IrradiancePhotonMarkingThreads(ThreadIndex).CheckHealth();

		// Destroy the thread.
		GThreadFactory->Destroy(IrradiancePhotonMarkingThreads(ThreadIndex).Thread);

		// Accumulate each thread's execution time and stats
		Stats.IrradiancePhotonMarkingThreadTime += IrradiancePhotonMarkingThreads(ThreadIndex).ExecutionTime;
	}

	IrradianceMarkWorkRanges.Empty();

	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing2, 0 ) );
}

DWORD FIrradiancePhotonMarkingThreadRunnable::Run()
{
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing2, ThreadIndex ) );
	const DOUBLE StartThreadTime = appSeconds();
#if _MSC_VER && !XBOX
	if(!appIsDebuggerPresent())
	{
		__try
		{
			System->MarkIrradiancePhotonsThreadLoop(ThreadIndex, IrradiancePhotons);
		}
		__except( WinCreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = appGetError();

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = TRUE;
		}
	}
	else
#endif
	{
		System->MarkIrradiancePhotonsThreadLoop(ThreadIndex, IrradiancePhotons);
	}
	const DOUBLE EndThreadTime = appSeconds();
	EndTime = EndThreadTime - GStartupTime;
	ExecutionTime = EndThreadTime - StartThreadTime;
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing2, ThreadIndex ) );
	return 0;
}

/** Entry point for all threads marking irradiance photons. */
void FStaticLightingSystem::MarkIrradiancePhotonsThreadLoop(
	INT ThreadIndex, 
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons)
{
	while (TRUE)
	{
		// Atomically read and increment the next work range index to process.
		// In this way work ranges are processed on-demand, which ensures consistent end times between threads.
		const INT RangeIndex = IrradianceMarkWorkRangeIndex.Increment() - 1;
		if (RangeIndex < IrradianceMarkWorkRanges.Num())
		{
			MarkIrradiancePhotonsWorkRange(IrradiancePhotons, IrradianceMarkWorkRanges(RangeIndex));
		}
		else
		{
			// Processing has begun for all work ranges
			break;
		}
	}
}

/** Marks irradiance photons specified by a single work range. */
void FStaticLightingSystem::MarkIrradiancePhotonsWorkRange(
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
	FIrradianceMarkingWorkRange WorkRange)
{
	// Temporary array that is reused for all photon searches by this thread, to reduce allocations
	TArray<FPhoton> TempFoundPhotons;
	TArray<FIrradiancePhoton>& CurrentArray = IrradiancePhotons(WorkRange.IrradiancePhotonArrayIndex);
	for (INT PhotonIndex = 0; PhotonIndex < CurrentArray.Num(); PhotonIndex++)
	{
		FIrradiancePhoton& CurrentIrradiancePhoton = CurrentArray(PhotonIndex);

		// Only add direct contribution if we are final gathering and at least one bounce is required,
		if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0
			// Or if photon mapping is being used for direct lighting.
			|| PhotonMappingSettings.bUsePhotonsForDirectLighting)
		{
			// Find a nearby direct photon
			const UBOOL bHasDirectContribution = FindAnyNearbyPhoton(DirectPhotonMap, CurrentIrradiancePhoton.GetPosition(), PhotonMappingSettings.DirectPhotonSearchDistance, FALSE);
			if (bHasDirectContribution)
			{
				// Mark the irradiance photon has having direct contribution, which will be used to reduce the search radius for this irradiance photon,
				// In order to get more accurate direct shadow transitions using the photon map.
				CurrentIrradiancePhoton.SetHasDirectContribution();
			}
		}
	}
}

/** Calculates irradiance for photons randomly chosen to precalculate irradiance. */
void FStaticLightingSystem::CalculateIrradiancePhotons(const FBoxSphereBounds& ImportanceBounds, TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons)
{
	check(PhotonMappingSettings.bUseIrradiancePhotons);
	//@todo - add a preparing stage for the swarm visualizer
	//GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing2, 0 ) );

	if (!PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces)
	{
		// Without bCacheIrradiancePhotonsOnSurfaces, treat all irradiance photons as found since we'll have to calculate irradiance for all of them.
		Stats.NumFoundIrradiancePhotons = Stats.NumIrradiancePhotons;
	}

	if (PhotonMappingSettings.bVisualizeIrradiancePhotonCalculation && Scene.DebugMapping)
	{
		FLOAT ClosestIrradiancePhotonDistSq = FLT_MAX;
		// Skip direct irradiance photons if viewing indirect bounces
		const INT ArrayStart = GeneralSettings.ViewSingleBounceNumber > 0 ? NumPhotonWorkRanges : 0;
		// Skip indirect irradiance photons if viewing direct only
		const INT ArrayEnd = GeneralSettings.ViewSingleBounceNumber == 0 ? NumPhotonWorkRanges : IrradiancePhotons.Num();
		for (INT ArrayIndex = ArrayStart; ArrayIndex < ArrayEnd; ArrayIndex++)
		{
			for (INT PhotonIndex = 0; PhotonIndex < IrradiancePhotons(ArrayIndex).Num(); PhotonIndex++)
			{
				const FIrradiancePhoton& CurrentPhoton = IrradiancePhotons(ArrayIndex)(PhotonIndex);
				const FLOAT CurrentDistSquared = (CurrentPhoton.GetPosition() - Scene.DebugInput.Position).SizeSquared();
				if ((!PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces || CurrentPhoton.IsUsed())
					&& CurrentDistSquared < ClosestIrradiancePhotonDistSq)
				{
					// Debug the closest irradiance photon to the selected position.
					// NOTE: This is not necessarily the photon that will get cached for the selected texel!
					// It's not easy to figure out which photon will get cached at this point in the lighting process, so we use the closest instead.
					//@todo - if bCacheIrradiancePhotonsOnSurfaces is enabled, we can figure out exactly which photon will be used by the selected texel or vertex.
					ClosestIrradiancePhotonDistSq = CurrentDistSquared;
					DebugIrradiancePhotonCalculationArrayIndex = ArrayIndex;
					DebugIrradiancePhotonCalculationPhotonIndex = PhotonIndex;
				}
			}
		}
	}

	// Setup work ranges for processing the irradiance photons
	IrradianceCalculationWorkRanges.Empty(IrradiancePhotons.Num());
	for (INT WorkRange = 0; WorkRange < IrradiancePhotons.Num(); WorkRange++)
	{
		IrradianceCalculationWorkRanges.AddItem(FIrradianceCalculatingWorkRange(WorkRange, WorkRange));
	}

	TIndirectArray<FIrradiancePhotonCalculatingThreadRunnable> IrradiancePhotonThreads;
	IrradiancePhotonThreads.Empty(NumStaticLightingThreads);
	for(INT ThreadIndex = 1; ThreadIndex < NumStaticLightingThreads; ThreadIndex++)
	{
		FIrradiancePhotonCalculatingThreadRunnable* ThreadRunnable = new(IrradiancePhotonThreads) FIrradiancePhotonCalculatingThreadRunnable(this, ThreadIndex, IrradiancePhotons);
		const FString ThreadName = FString::Printf(TEXT("IrradiancePhotonCalculatingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}

	const DOUBLE MainThreadStartTime = appSeconds();

	FCalculateIrradiancePhotonStats MainThreadStats;
	CalculateIrradiancePhotonsThreadLoop(0, IrradiancePhotons, MainThreadStats);
	
	Stats.IrradiancePhotonCalculatingThreadTime = appSeconds() - MainThreadStartTime;
	Stats.CalculateIrradiancePhotonStats = MainThreadStats;

	// Stop the static lighting threads.
	for(INT ThreadIndex = 0;ThreadIndex < IrradiancePhotonThreads.Num();ThreadIndex++)
	{
		// Wait for the thread to exit.
		IrradiancePhotonThreads(ThreadIndex).Thread->WaitForCompletion();

		// Check that it didn't terminate with an error.
		IrradiancePhotonThreads(ThreadIndex).CheckHealth();

		// Destroy the thread.
		GThreadFactory->Destroy(IrradiancePhotonThreads(ThreadIndex).Thread);

		// Accumulate each thread's execution time and stats
		Stats.IrradiancePhotonCalculatingThreadTime += IrradiancePhotonThreads(ThreadIndex).ExecutionTime;
		Stats.CalculateIrradiancePhotonStats += IrradiancePhotonThreads(ThreadIndex).Stats;
	}

	IrradianceCalculationWorkRanges.Empty();

	if (!PhotonMappingSettings.bOptimizeDirectLightingWithPhotons)
	{
		// Release all of the direct photon map memory if we are not going to need it later
		DirectPhotonMap.Destroy();
	}
	// Release all of the second bounce photon map memory since it will not be used again
	SecondBouncePhotonMap.Destroy();
}

DWORD FIrradiancePhotonCalculatingThreadRunnable::Run()
{
	const DOUBLE StartThreadTime = appSeconds();
#if _MSC_VER && !XBOX
	if(!appIsDebuggerPresent())
	{
		__try
		{
			System->CalculateIrradiancePhotonsThreadLoop(ThreadIndex, IrradiancePhotons, Stats);
		}
		__except( WinCreateMiniDump( GetExceptionInformation() ) )
		{
			ErrorMessage = appGetError();

			// Use a memory barrier to ensure that the main thread sees the write to ErrorMessage before
			// the write to bTerminatedByError.
			appMemoryBarrier();

			bTerminatedByError = TRUE;
		}
	}
	else
#endif
	{
		System->CalculateIrradiancePhotonsThreadLoop(ThreadIndex, IrradiancePhotons, Stats);
	}
	const DOUBLE EndThreadTime = appSeconds();
	EndTime = EndThreadTime - GStartupTime;
	ExecutionTime = EndThreadTime - StartThreadTime;
	return 0;
}

/** Main loop that all threads access to calculate irradiance photons. */
void FStaticLightingSystem::CalculateIrradiancePhotonsThreadLoop(
	INT ThreadIndex, 
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
	FCalculateIrradiancePhotonStats& Stats)
{
	while (TRUE)
	{
		// Atomically read and increment the next work range index to process.
		// In this way work ranges are processed on-demand, which ensures consistent end times between threads.
		// Processing from smallest to largest work range index since the main thread is processing outputs in the same order.
		const INT RangeIndex = IrradianceCalcWorkRangeIndex.Increment() - 1;
		if (RangeIndex < IrradianceCalculationWorkRanges.Num())
		{
			CalculateIrradiancePhotonsWorkRange(IrradiancePhotons, IrradianceCalculationWorkRanges(RangeIndex), Stats);
		}
		else
		{
			// Processing has begun for all work ranges
			break;
		}
	}
}

/** Calculates irradiance for the photons specified by a single work range. */
void FStaticLightingSystem::CalculateIrradiancePhotonsWorkRange(
	TArray<TArray<FIrradiancePhoton>>& IrradiancePhotons, 
	FIrradianceCalculatingWorkRange WorkRange,
	FCalculateIrradiancePhotonStats& Stats)
{
	// Temporary array that is reused for all photon searches by this thread, to reduce allocations
	TArray<FPhoton> TempFoundPhotons;
	TArray<FIrradiancePhoton>& CurrentArray = IrradiancePhotons(WorkRange.IrradiancePhotonArrayIndex);
	for (INT PhotonIndex = 0; PhotonIndex < CurrentArray.Num(); PhotonIndex++)
	{
		FIrradiancePhoton& CurrentIrradiancePhoton = CurrentArray(PhotonIndex);
		// If we already cached irradiance photons on surfaces, only calculate irradiance for photons which actually got found.
		if (PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces && !CurrentIrradiancePhoton.IsUsed())
		{
			continue;
		}

		const UBOOL bDebugThisPhoton = PhotonMappingSettings.bVisualizeIrradiancePhotonCalculation
			&& DebugIrradiancePhotonCalculationArrayIndex == WorkRange.IrradiancePhotonArrayIndex 
			&& DebugIrradiancePhotonCalculationPhotonIndex == PhotonIndex;

		FLinearColor AccumulatedIrradiance(FLinearColor::Black);
		// Only add direct contribution if we are final gathering and at least one bounce is required,
		if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0
			// Or if photon mapping is being used for direct lighting.
			|| PhotonMappingSettings.bUsePhotonsForDirectLighting)
		{
			const FLinearColor DirectPhotonIrradiance = CalculatePhotonIrradiance(
				DirectPhotonMap, 
				NumPhotonsEmittedDirect, 
				PhotonMappingSettings.NumIrradianceCalculationPhotons, 
				PhotonMappingSettings.DirectPhotonSearchDistance, 
				CurrentIrradiancePhoton,
				bDebugThisPhoton && GeneralSettings.ViewSingleBounceNumber == 0,
				TempFoundPhotons,
				Stats);

			checkSlow(DirectPhotonIrradiance.AreFloatsValid());

			// Only add direct contribution if it should be viewed given ViewSingleBounceNumber
			if (GeneralSettings.ViewSingleBounceNumber < 0
				|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1
				|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 0
				|| PhotonMappingSettings.bUsePhotonsForDirectLighting && GeneralSettings.ViewSingleBounceNumber == 0)
			{
				AccumulatedIrradiance = DirectPhotonIrradiance;
			}
		}

		if (GeneralSettings.NumIndirectLightingBounces > 0)
		{
			// Always add caustic photon contribution, even if we are not final gathering.
			// This results in poor caustics when not final gathering, but allows us to debug irradiance photons more accurately,
			// Since they will always be calculated in the same way regardless of whether we are final gathering.
			const FLinearColor CausticPhotonIrradiance = CalculatePhotonIrradiance(
				CausticPhotonMap, 
				NumPhotonsEmittedCaustic, 
				PhotonMappingSettings.NumIrradianceCalculationPhotons, 
				PhotonMappingSettings.CausticPhotonSearchDistance, 
				CurrentIrradiancePhoton,
				FALSE,
				TempFoundPhotons,
				Stats);

			checkSlow(CausticPhotonIrradiance.AreFloatsValid());

			if (GeneralSettings.ViewSingleBounceNumber < 0 || GeneralSettings.ViewSingleBounceNumber == 1)
			{
				AccumulatedIrradiance += CausticPhotonIrradiance;
			}
		}

		// If we are final gathering, first bounce photons are actually the second lighting bounce since the final gather is the first bounce
		if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 1
			|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 0)
		{
			const FLinearColor FirstBouncePhotonIrradiance = CalculatePhotonIrradiance(
				FirstBouncePhotonMap, 
				NumPhotonsEmittedFirstBounce, 
				PhotonMappingSettings.NumIrradianceCalculationPhotons, 
				PhotonMappingSettings.IndirectPhotonSearchDistance, 
				CurrentIrradiancePhoton,
				bDebugThisPhoton && GeneralSettings.ViewSingleBounceNumber == 1,
				TempFoundPhotons,
				Stats);

			checkSlow(FirstBouncePhotonIrradiance.AreFloatsValid());

			// Only add first bounce contribution if it should be viewed given ViewSingleBounceNumber
			if (GeneralSettings.ViewSingleBounceNumber < 0 
				|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 2
				|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 1)
			{
				AccumulatedIrradiance += FirstBouncePhotonIrradiance;
			}

			// If we are final gathering, second bounce photons are actually the third lighting bounce since the final gather is the first bounce
			if (PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 2
				|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.NumIndirectLightingBounces > 1)
			{
				const FLinearColor SecondBouncePhotonIrradiance = CalculatePhotonIrradiance(
					SecondBouncePhotonMap, 
					NumPhotonsEmittedSecondBounce, 
					PhotonMappingSettings.NumIrradianceCalculationPhotons, 
					PhotonMappingSettings.IndirectPhotonSearchDistance, 
					CurrentIrradiancePhoton,
					bDebugThisPhoton && GeneralSettings.ViewSingleBounceNumber > 1,
					TempFoundPhotons,
					Stats);

				checkSlow(SecondBouncePhotonIrradiance.AreFloatsValid());

				// Only add second and up bounce contribution if it should be viewed given ViewSingleBounceNumber
				if (GeneralSettings.ViewSingleBounceNumber < 0 
					|| PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 3
					|| !PhotonMappingSettings.bUseFinalGathering && GeneralSettings.ViewSingleBounceNumber == 2)
				{
					AccumulatedIrradiance += SecondBouncePhotonIrradiance;
				}
			}
		}
		CurrentIrradiancePhoton.SetIrradiance(AccumulatedIrradiance);
	}
}

/** Cache irradiance photons on surfaces. */
void FStaticLightingSystem::CacheIrradiancePhotons()
{
	check(PhotonMappingSettings.bCacheIrradiancePhotonsOnSurfaces);
	for(INT ThreadIndex = 1; ThreadIndex < NumStaticLightingThreads; ThreadIndex++)
	{
		FMappingProcessingThreadRunnable* ThreadRunnable = new(IrradiancePhotonCachingThreads) FMappingProcessingThreadRunnable(this, ThreadIndex, StaticLightingTask_CacheIrradiancePhotons);
		const FString ThreadName = FString::Printf(TEXT("IrradiancePhotonCachingThread%u"), ThreadIndex);
		ThreadRunnable->Thread = GThreadFactory->CreateThread(ThreadRunnable, TCHAR_TO_ANSI(*ThreadName), 0, 0, 0, TPri_Normal);
	}

	// Start the static lighting thread loop on the main thread, too.
	// Once it returns, all static lighting mappings have begun processing.
	CacheIrradiancePhotonsThreadLoop(0, TRUE);

	// Stop the static lighting threads.
	for(INT ThreadIndex = 0;ThreadIndex < IrradiancePhotonCachingThreads.Num();ThreadIndex++)
	{
		// Wait for the thread to exit.
		IrradiancePhotonCachingThreads(ThreadIndex).Thread->WaitForCompletion();
		// Check that it didn't terminate with an error.
		IrradiancePhotonCachingThreads(ThreadIndex).CheckHealth();

		// Destroy the thread.
		GThreadFactory->Destroy(IrradiancePhotonCachingThreads(ThreadIndex).Thread);
	}
	IrradiancePhotonCachingThreads.Empty();
	IrradiancePhotonMap.Destroy();
}

/** Main loop that all threads access to cache irradiance photons. */
void FStaticLightingSystem::CacheIrradiancePhotonsThreadLoop(INT ThreadIndex, UBOOL bIsMainThread)
{
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing3, ThreadIndex ) );
	UBOOL bIsDone = FALSE;
	while (!bIsDone)
	{
		// Atomically read and increment the next mapping index to process.
		const INT MappingIndex = NextMappingToCacheIrradiancePhotonsOn.Increment() - 1;

		if (MappingIndex < AllMappings.Num())
		{
			// If this is the main thread, update progress and apply completed static lighting.
			if (bIsMainThread)
			{
				// Check the health of all static lighting threads.
				for (INT ThreadIndex = 0; ThreadIndex < IrradiancePhotonCachingThreads.Num(); ThreadIndex++)
				{
					IrradiancePhotonCachingThreads(ThreadIndex).CheckHealth();
				}
			}

			FStaticLightingTextureMapping* TextureMapping = AllMappings(MappingIndex)->GetTextureMapping();
			FStaticLightingVertexMapping* VertexMapping = AllMappings(MappingIndex)->GetVertexMapping();
			if (TextureMapping)
			{
				CacheIrradiancePhotonsTextureMapping(TextureMapping);
			}
			else if (VertexMapping)
			{
				CacheIrradiancePhotonsVertexMapping(VertexMapping);
			}
		}
		else
		{
			// Processing has begun for all mappings.
			bIsDone = TRUE;
		}
	}
	GSwarm->SendMessage( NSwarm::FTimingMessage( NSwarm::PROGSTATE_Preparing3, ThreadIndex ) );
}

/** Returns TRUE if a photon was found within MaxPhotonSearchDistance. */
UBOOL FStaticLightingSystem::FindAnyNearbyPhoton(
	const FPhotonOctree& PhotonMap, 
	const FVector4& SearchPosition, 
	FLOAT MaxPhotonSearchDistance,
	UBOOL bDebugThisLookup) const
{
	appInterlockedIncrement(&Stats.NumPhotonGathers);

	const FBox SearchBox = FBox::BuildAABB(SearchPosition, FVector4(MaxPhotonSearchDistance, MaxPhotonSearchDistance, MaxPhotonSearchDistance));
	for (FPhotonOctree::TConstIterator<> OctreeIt(PhotonMap); OctreeIt.HasPendingNodes(); OctreeIt.Advance())
	{
		const FPhotonOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
		const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();

		// Push children onto the iterator stack if they intersect the query box
		if (!CurrentNode.IsLeaf())
		{
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				if (CurrentNode.HasChild(ChildRef))
				{
					const FOctreeNodeContext ChildContext = CurrentContext.GetChildContext(ChildRef);
					if (ChildContext.Bounds.GetBox().Intersect(SearchBox))
					{
						OctreeIt.PushChild(ChildRef);
					}
				}
			}
		}

		// Iterate over all photons in the nodes intersecting the query box
		for (FPhotonOctree::ElementConstIt MeshIt(CurrentNode.GetConstElementIt()); MeshIt; ++MeshIt)
		{
			const FPhotonElement& PhotonElement = *MeshIt;
			const FLOAT DistanceSquared = (PhotonElement.Photon.GetPosition() - SearchPosition).SizeSquared();
			// Only searching for photons closer than the max distance
			if (DistanceSquared < MaxPhotonSearchDistance * MaxPhotonSearchDistance)
			{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugThisLookup 
					&& PhotonMappingSettings.bVisualizePhotonGathers
					&& &PhotonMap == &DirectPhotonMap)
				{
					DebugOutput.bDirectPhotonValid = TRUE;
					DebugOutput.GatheredDirectPhoton = FDebugPhoton(PhotonElement.Photon.GetId(), PhotonElement.Photon.GetPosition(), PhotonElement.Photon.GetIncidentDirection(), PhotonElement.Photon.GetSurfaceNormal());
				}
#endif
				return TRUE;
			}
		}
	}
	return FALSE;
}

/** 
 * Searches the given photon map for the nearest NumPhotonsToFind photons to SearchPosition using an iterative process, 
 * Unless the start and max search distances are the same, in which case all photons in that distance will be returned.
 * The iterative search starts at StartPhotonSearchDistance and doubles the search distance until enough photons are found or the distance is greater than MaxPhotonSearchDistance.
 * @return - the furthest found photon's distance squared from SearchPosition, unless the start and max search distances are the same,
 *		in which case Square(MaxPhotonSearchDistance) will be returned.
 */
FLOAT FStaticLightingSystem::FindNearbyPhotonsIterative(
	const FPhotonOctree& PhotonMap, 
	const FVector4& SearchPosition, 
	const FVector4& SearchNormal, 
	INT NumPhotonsToFind,
	FLOAT StartPhotonSearchDistance, 
	FLOAT MaxPhotonSearchDistance,
	UBOOL bDebugSearchResults,
	UBOOL bDebugSearchProcess,
	TArray<FPhoton>& FoundPhotons,
	FFindNearbyPhotonStats& SearchStats) const
{
	appInterlockedIncrement(&Stats.NumPhotonGathers);
	SearchStats.NumIterativePhotonMapSearches++;
	// Only enforce the search number if the start and max distances are not equal
	const UBOOL bEnforceSearchNumber = !appIsNearlyEqual(StartPhotonSearchDistance, MaxPhotonSearchDistance);
	FLOAT SearchDistance = StartPhotonSearchDistance;
	FLOAT FurthestPhotonDistanceSquared = 0.0f;

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugSearchProcess)
	{
		// Verify that only one search is debugged
		// This will not always catch multiple searches due to re-entrance by multiple threads
		checkSlow(DebugOutput.GatheredPhotonNodes.Num() == 0);
	}
#endif

	// Continue searching until we have found enough photons or have exceeded the max search distance
	while (FoundPhotons.Num() < NumPhotonsToFind && SearchDistance <= MaxPhotonSearchDistance)
	{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
		if (bDebugSearchProcess)
		{
			// Only capture the nodes visited on the last iteration
			DebugOutput.GatheredPhotonNodes.Empty();
		}
#endif
		FurthestPhotonDistanceSquared = SearchDistance * SearchDistance;
		// Presize to avoid unnecessary allocations
		// Empty last search iteration's results so we don't have to use AddUniqueItem
		FoundPhotons.Empty(Max(NumPhotonsToFind, FoundPhotons.Num() + FoundPhotons.GetSlack()));
		const FBox SearchBox = FBox::BuildAABB(SearchPosition, FVector4(SearchDistance, SearchDistance, SearchDistance));
		for (FPhotonOctree::TConstIterator<TInlineAllocator<600>> OctreeIt(PhotonMap); OctreeIt.HasPendingNodes(); OctreeIt.Advance())
		{
			const FPhotonOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
			const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();
			{
				LIGHTINGSTAT(FScopedRDTSCTimer PushingChildrenTimer(SearchStats.PushingOctreeChildrenThreadTime));
				// Push children onto the iterator stack if they intersect the query box
				if (!CurrentNode.IsLeaf())
				{
					FOREACH_OCTREE_CHILD_NODE(ChildRef)
					{
						if (CurrentNode.HasChild(ChildRef))
						{
							const FOctreeNodeContext ChildContext = CurrentContext.GetChildContext(ChildRef);
							if (ChildContext.Bounds.GetBox().Intersect(SearchBox))
							{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
								if (bDebugSearchProcess)
								{
									DebugOutput.GatheredPhotonNodes.AddItem(FDebugOctreeNode(ChildContext.Bounds.Center, ChildContext.Bounds.Extent));
								}
#endif
								OctreeIt.PushChild(ChildRef);
							}
						}
					}
				}
			}

			LIGHTINGSTAT(FScopedRDTSCTimer ProcessingElementsTimer(SearchStats.ProcessingOctreeElementsThreadTime));
			// Iterate over all photons in the nodes intersecting the query box
			for (FPhotonOctree::ElementConstIt MeshIt(CurrentNode.GetConstElementIt()); MeshIt; ++MeshIt)
			{
				const FPhotonElement& PhotonElement = *MeshIt;
				const FLOAT DistanceSquared = (PhotonElement.Photon.GetPosition() - SearchPosition).SizeSquared();
				const FLOAT CosNormalTheta = SearchNormal | PhotonElement.Photon.GetSurfaceNormal();
				const FLOAT CosIncidentDirectionTheta = SearchNormal | PhotonElement.Photon.GetIncidentDirection();
				// Only searching for photons closer than the max distance
				if (DistanceSquared < FurthestPhotonDistanceSquared
					// Whose normal is within the specified angle from the search normal
					&& CosNormalTheta > PhotonMappingSettings.PhotonSearchAngleThreshold
					// And whose incident direction is in the same hemisphere as the search normal.
					&& CosIncidentDirectionTheta > 0.0f)
				{
					if (bEnforceSearchNumber)
					{
						if (FoundPhotons.Num() < NumPhotonsToFind)
						{
							FoundPhotons.AddItem(PhotonElement.Photon);
						}
						else
						{
							checkSlow(FoundPhotons.Num() == NumPhotonsToFind);
							FLOAT FurthestFoundPhotonDistSq = 0;
							INT FurthestFoundPhotonIndex = -1;

							// Find the furthest photon
							// This could be accelerated with a heap instead of doing an O(n) search
							LIGHTINGSTAT(FScopedRDTSCTimer FindingFurthestTimer(SearchStats.FindingFurthestPhotonThreadTime));
							for (INT PhotonIndex = 0; PhotonIndex < FoundPhotons.Num(); PhotonIndex++)
							{
								const FLOAT CurrentDistanceSquared = (FoundPhotons(PhotonIndex).GetPosition() - SearchPosition).SizeSquared();
								if (CurrentDistanceSquared > FurthestFoundPhotonDistSq)
								{
									FurthestFoundPhotonDistSq = CurrentDistanceSquared;
									FurthestFoundPhotonIndex = PhotonIndex;
								}
							}
							checkSlow(FurthestFoundPhotonIndex >= 0);
							FurthestPhotonDistanceSquared = FurthestFoundPhotonDistSq;
							if (DistanceSquared < FurthestFoundPhotonDistSq)
							{
								// Replace the furthest photon with the new photon since the new photon is closer
								FoundPhotons(FurthestFoundPhotonIndex) = PhotonElement.Photon;
							}
						}
					}
					else
					{
						FoundPhotons.AddItem(PhotonElement.Photon);
					}
				}
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugSearchProcess)
				{
					DebugOutput.IrradiancePhotons.AddItem(FDebugPhoton(PhotonElement.Photon.GetId(), PhotonElement.Photon.GetPosition(), PhotonElement.Photon.GetIncidentDirection(), PhotonElement.Photon.GetSurfaceNormal()));
				}
#endif
			}
		}
		// Double the search radius for each iteration
		SearchDistance *= 2.0f;
		SearchStats.NumSearchIterations++;
	}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugSearchProcess || bDebugSearchResults && PhotonMappingSettings.bVisualizePhotonGathers)
	{
		if (&PhotonMap == &CausticPhotonMap)
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredCausticPhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
		// Assuming that only importance photons are debugged and enforce the search number
		else if (bDebugSearchResults && bEnforceSearchNumber)
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredImportancePhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
		else
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredPhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
	}
#endif
	return FurthestPhotonDistanceSquared;
}

struct FOctreeNodeRefAndDistance
{
	FOctreeChildNodeRef NodeRef;
	FLOAT DistanceSquared;

	FORCEINLINE FOctreeNodeRefAndDistance(FOctreeChildNodeRef InNodeRef, FLOAT InDistanceSquared) :
		NodeRef(InNodeRef),
		DistanceSquared(InDistanceSquared)
	{}
};

/** Used to sort FOctreeNodeRefAndDistances from smallest DistanceSquared to largest. */
IMPLEMENT_COMPARE_CONSTREF( FOctreeNodeRefAndDistance, PhotonMapping, { return Sgn( A.DistanceSquared - B.DistanceSquared ); } )

/** 
 * Searches the given photon map for the nearest NumPhotonsToFind photons to SearchPosition by sorting octree nodes nearest to furthest.
 * @return - the furthest found photon's distance squared from SearchPosition.
 */
FLOAT FStaticLightingSystem::FindNearbyPhotonsSorted(
	const FPhotonOctree& PhotonMap, 
	const FVector4& SearchPosition, 
	const FVector4& SearchNormal, 
	INT NumPhotonsToFind, 
	FLOAT MaxPhotonSearchDistance,
	UBOOL bDebugSearchResults,
	UBOOL bDebugSearchProcess,
	TArray<FPhoton>& FoundPhotons,
	FFindNearbyPhotonStats& SearchStats) const
{
	appInterlockedIncrement(&Stats.NumPhotonGathers);
	FLOAT FurthestPhotonDistanceSquared = MaxPhotonSearchDistance * MaxPhotonSearchDistance;

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugSearchProcess)
	{
		// Verify that only one search is debugged
		// This will not always catch multiple searches due to re-entrance by multiple threads
		checkSlow(DebugOutput.GatheredPhotonNodes.Num() == 0);
	}
#endif

	// Presize to avoid unnecessary allocations
	FoundPhotons.Empty(Max(NumPhotonsToFind, FoundPhotons.Num() + FoundPhotons.GetSlack()));
	for (FPhotonOctree::TConstIterator<TInlineAllocator<600>> OctreeIt(PhotonMap); OctreeIt.HasPendingNodes(); OctreeIt.Advance())
	{
		SearchStats.NumOctreeNodesVisited++;
		const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();
		const FLOAT ClosestNodePointDistanceSquared = (CurrentContext.Bounds.Center - SearchPosition).SizeSquared() - CurrentContext.Bounds.Extent.SizeSquared();
		if (ClosestNodePointDistanceSquared > FurthestPhotonDistanceSquared && !CurrentContext.Bounds.GetBox().IsInside(SearchPosition))
		{
			// Skip nodes that don't contain the search position and whose closest point is further than FurthestPhotonDistanceSquared
			// This check was already done before pushing the node, but FurthestPhotonDistanceSquared may have been reduced since then
			//@todo - can we skip all remaining nodes too?  Nodes are pushed from closest to furthest.
			continue;
		}
		
		const FPhotonOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
		{
			LIGHTINGSTAT(FScopedRDTSCTimer ProcessingElementsTimer(SearchStats.ProcessingOctreeElementsThreadTime));
			// Iterate over all photons in the nodes intersecting the query box
			for (FPhotonOctree::ElementConstIt MeshIt(CurrentNode.GetConstElementIt()); MeshIt; ++MeshIt)
			{
				SearchStats.NumElementsTested++;
				const FPhotonElement& PhotonElement = *MeshIt;
				const FLOAT DistanceSquared = (PhotonElement.Photon.GetPosition() - SearchPosition).SizeSquared();
				const FLOAT CosNormalTheta = SearchNormal | PhotonElement.Photon.GetSurfaceNormal();
				const FLOAT CosIncidentDirectionTheta = SearchNormal | PhotonElement.Photon.GetIncidentDirection();
				// Only searching for photons closer than the max distance
				if (DistanceSquared < FurthestPhotonDistanceSquared
					// Whose normal is within the specified angle from the search normal
					&& CosNormalTheta > PhotonMappingSettings.PhotonSearchAngleThreshold
					// And whose incident direction is in the same hemisphere as the search normal.
					&& CosIncidentDirectionTheta > 0.0f)
				{
					SearchStats.NumElementsAccepted++;
					if (FoundPhotons.Num() < NumPhotonsToFind)
					{
						FoundPhotons.AddItem(PhotonElement.Photon);
					}
					else
					{
						checkSlow(FoundPhotons.Num() == NumPhotonsToFind);
						FLOAT FurthestFoundPhotonDistSq = 0;
						INT FurthestFoundPhotonIndex = -1;

						// Find the furthest photon
						// This could be accelerated with a heap instead of doing an O(n) search
						LIGHTINGSTAT(FScopedRDTSCTimer FindingFurthestTimer(SearchStats.FindingFurthestPhotonThreadTime));
						for (INT PhotonIndex = FoundPhotons.Num() - 1; PhotonIndex >= 0; PhotonIndex--)
						{
							const FLOAT CurrentDistanceSquared = (FoundPhotons(PhotonIndex).GetPosition() - SearchPosition).SizeSquared();
							if (CurrentDistanceSquared > FurthestFoundPhotonDistSq)
							{
								FurthestFoundPhotonDistSq = CurrentDistanceSquared;
								FurthestFoundPhotonIndex = PhotonIndex;
							}
						}
						
						checkSlow(FurthestFoundPhotonIndex >= 0);
						FurthestPhotonDistanceSquared = FurthestFoundPhotonDistSq;
						if (DistanceSquared < FurthestFoundPhotonDistSq)
						{
							// Replace the furthest photon with the new photon since the new photon is closer
							FoundPhotons(FurthestFoundPhotonIndex) = PhotonElement.Photon;
						}
					}
				}
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
				if (bDebugSearchProcess)
				{
					DebugOutput.IrradiancePhotons.AddItem(FDebugPhoton(PhotonElement.Photon.GetId(), PhotonElement.Photon.GetPosition(), PhotonElement.Photon.GetIncidentDirection(), PhotonElement.Photon.GetSurfaceNormal()));
				}
#endif
			}
		}
		
		LIGHTINGSTAT(FScopedRDTSCTimer PushingChildrenTimer(SearchStats.PushingOctreeChildrenThreadTime));
		// Push children onto the iterator stack if they intersect the query box
		if (!CurrentNode.IsLeaf())
		{
			TArray<FOctreeNodeRefAndDistance, TInlineAllocator<8> > ChildrenInRange;
			UBOOL bAllNodesZeroDistance = TRUE;
			FOREACH_OCTREE_CHILD_NODE(ChildRef)
			{
				if (CurrentNode.HasChild(ChildRef))
				{
					SearchStats.NumOctreeNodesTested++;
					const FOctreeNodeContext ChildContext = CurrentContext.GetChildContext(ChildRef);
					const UBOOL ChildContainsSearchPosition = ChildContext.Bounds.GetBox().IsInside(SearchPosition);
					const FLOAT ClosestChildPointDistanceSquared = ChildContainsSearchPosition ? 
						0 : 
						Max((ChildContext.Bounds.Center - SearchPosition).SizeSquared() - ChildContext.Bounds.Extent.SizeSquared(), 0.0f);

					// Only visit nodes that either contain the search position or whose closest point is closer than FurthestPhotonDistanceSquared
					if (ClosestChildPointDistanceSquared <= FurthestPhotonDistanceSquared)
					{
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
						if (bDebugSearchProcess)
						{
							DebugOutput.GatheredPhotonNodes.AddItem(FDebugOctreeNode(ChildContext.Bounds.Center, ChildContext.Bounds.Extent));
						}
#endif
						bAllNodesZeroDistance = bAllNodesZeroDistance && ClosestChildPointDistanceSquared < DELTA;
						ChildrenInRange.AddItem(FOctreeNodeRefAndDistance(ChildRef, ClosestChildPointDistanceSquared));
					}
				}
			}

			if (!bAllNodesZeroDistance && ChildrenInRange.Num() > 1)
			{
				// Sort the nodes from closest to furthest
				Sort<FOctreeNodeRefAndDistance,COMPARE_CONSTREF_CLASS(FOctreeNodeRefAndDistance,PhotonMapping)>(ChildrenInRange.GetData(), ChildrenInRange.Num());
			}

			for (INT NodeIndex = 0; NodeIndex < ChildrenInRange.Num(); NodeIndex++)
			{
				OctreeIt.PushChild(ChildrenInRange(NodeIndex).NodeRef);
			}
		}
	}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugSearchProcess || bDebugSearchResults && PhotonMappingSettings.bVisualizePhotonGathers)
	{
		if (&PhotonMap == &CausticPhotonMap)
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredCausticPhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
		// Assuming that only importance photons are debugged
		else if (bDebugSearchResults)
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredImportancePhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
		else
		{
			for (INT i = 0; i < FoundPhotons.Num(); i++)
			{
				DebugOutput.GatheredPhotons.AddItem(FDebugPhoton(FoundPhotons(i).GetId(), FoundPhotons(i).GetPosition(), FoundPhotons(i).GetIncidentDirection(), FoundPhotons(i).GetSurfaceNormal()));
			}
		}
	}
#endif
	return FurthestPhotonDistanceSquared;
}

/** Comparison class to sort an array of photons into ascending order of distance to ComparePosition. */
class FCompareNearestIrradiancePhotons
{
public:
	FCompareNearestIrradiancePhotons(const FVector4& InPosition) :
		ComparePosition(InPosition)
	{}

	inline INT Compare(const FIrradiancePhoton* A, const FIrradiancePhoton* B) const
	{
		const FLOAT DistanceSquaredA = (A->GetPosition() - ComparePosition).SizeSquared();
		const FLOAT DistanceSquaredB = (B->GetPosition() - ComparePosition).SizeSquared();
		return DistanceSquaredA > DistanceSquaredB ? 1 : -1;
	}

private:
	const FVector4 ComparePosition;
};

/** Finds the nearest irradiance photon, if one exists. */
FIrradiancePhoton* FStaticLightingSystem::FindNearestIrradiancePhoton(
	const FStaticLightingVertex& Vertex, 
	FStaticLightingMappingContext& MappingContext, 
	TArray<FIrradiancePhoton*>& TempIrradiancePhotons,
	UBOOL bVisibleOnly, 
	UBOOL bDebugThisLookup) const
{
	MappingContext.Stats.NumIrradiancePhotonMapSearches++;

	FIrradiancePhoton* ClosestPhoton = NULL;
	// Traverse the octree with the maximum distance required
	const FLOAT SearchDistance = Max(PhotonMappingSettings.DirectPhotonSearchDistance, PhotonMappingSettings.IndirectPhotonSearchDistance);
	FLOAT ClosestDistanceSquared = Square(SearchDistance);

	// Empty the temporary array without reallocating
	TempIrradiancePhotons.Empty(TempIrradiancePhotons.Num() + TempIrradiancePhotons.GetSlack());
	const FBox SearchBox = FBox::BuildAABB(Vertex.WorldPosition, FVector4(SearchDistance, SearchDistance, SearchDistance));
	{
		LIGHTINGSTAT(FScopedRDTSCTimer OctreeTraversal(MappingContext.Stats.IrradiancePhotonOctreeTraversalTime));
		for (FIrradiancePhotonOctree::TConstIterator<> OctreeIt(IrradiancePhotonMap); OctreeIt.HasPendingNodes(); OctreeIt.Advance())
		{
			const FIrradiancePhotonOctree::FNode& CurrentNode = OctreeIt.GetCurrentNode();
			const FOctreeNodeContext& CurrentContext = OctreeIt.GetCurrentContext();

			// Push children onto the iterator stack if they intersect the query box
			if (!CurrentNode.IsLeaf())
			{
				FOREACH_OCTREE_CHILD_NODE(ChildRef)
				{
					if (CurrentNode.HasChild(ChildRef))
					{
						const FOctreeNodeContext ChildContext = CurrentContext.GetChildContext(ChildRef);
						if (ChildContext.Bounds.GetBox().Intersect(SearchBox))
						{
							OctreeIt.PushChild(ChildRef);
						}
					}
				}
			}

			// Iterate over all photons in the nodes intersecting the query box
			for (FIrradiancePhotonOctree::ElementIt MeshIt(CurrentNode.GetElementIt()); MeshIt; ++MeshIt)
			{
				FIrradiancePhotonElement& PhotonElement = *MeshIt;
				FIrradiancePhoton& CurrentPhoton = PhotonElement.GetPhoton();
				const FVector4 PhotonToVertexVector = Vertex.WorldPosition - CurrentPhoton.GetPosition();
				const FLOAT DistanceSquared = PhotonToVertexVector.SizeSquared();
				const FLOAT CosTheta = Vertex.WorldTangentZ | CurrentPhoton.GetSurfaceNormal();

				// Only searching for irradiance photons with normals similar to the search normal
				if (CosTheta > PhotonMappingSettings.PhotonSearchAngleThreshold
					// And closer to the search position than the max search distance.
					&& (CurrentPhoton.HasDirectContribution() && (DistanceSquared < Square(PhotonMappingSettings.DirectPhotonSearchDistance))
					|| !CurrentPhoton.HasDirectContribution() && (DistanceSquared < Square(PhotonMappingSettings.IndirectPhotonSearchDistance))))
				{
					// Only accept irradiance photons within an angle of the plane defined by the vertex normal
					// This avoids expensive visibility traces to photons that are probably not on the same surface
					const FLOAT DirectionDotNormal = CurrentPhoton.GetSurfaceNormal() | PhotonToVertexVector.SafeNormal();
					if (Abs(DirectionDotNormal) < PhotonMappingSettings.MinCosIrradiancePhotonSearchCone)
					{
						if (bVisibleOnly)
						{
							// Store the photon for later, which is faster than tracing a ray here since this may not be the closest photon
							TempIrradiancePhotons.AddItem(&CurrentPhoton);
						}
						else if (DistanceSquared < ClosestDistanceSquared)
						{
							// Only accept the closest photon if visibility is not required
							ClosestPhoton = &CurrentPhoton;
							ClosestDistanceSquared = DistanceSquared;
						}
					}
				}
			}
		}
	}

	if (bVisibleOnly)
	{
		// Sort the photons so the closest photon is in the beginning of the array
		FCompareNearestIrradiancePhotons CompareClassInstance(Vertex.WorldPosition);
		SortWithLocalDataComparison(TempIrradiancePhotons.GetData(), TempIrradiancePhotons.Num(), CompareClassInstance);

		// Trace a ray from the vertex to each irradiance photon until a visible one is found, starting with the closest
		for (INT PhotonIndex = 0; PhotonIndex < TempIrradiancePhotons.Num(); PhotonIndex++)
		{
			FIrradiancePhoton* CurrentPhoton = TempIrradiancePhotons(PhotonIndex);
			const FVector4 VertexToPhoton = CurrentPhoton->GetPosition() - Vertex.WorldPosition;
			const FLightRay VertexToPhotonRay(
				Vertex.WorldPosition + VertexToPhoton.SafeNormal() * SceneConstants.VisibilityRayOffsetDistance + Vertex.WorldTangentZ * SceneConstants.VisibilityNormalOffsetDistance,
				CurrentPhoton->GetPosition() + CurrentPhoton->GetSurfaceNormal() * SceneConstants.VisibilityNormalOffsetDistance,
				NULL,
				NULL
				);

			MappingContext.Stats.NumIrradiancePhotonSearchRays++;
			const FLOAT PreviousShadowTraceTime = MappingContext.RayCache.BooleanRayTraceTime;
			// Check the line segment for intersection with the static lighting meshes.
			FLightRayIntersection Intersection;
			AggregateMesh.IntersectLightRay(VertexToPhotonRay, FALSE, FALSE, FALSE, MappingContext.RayCache, Intersection);
			MappingContext.Stats.IrradiancePhotonSearchRayTime += MappingContext.RayCache.BooleanRayTraceTime - PreviousShadowTraceTime;
#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
			if (bDebugThisLookup && PhotonMappingSettings.bVisualizePhotonGathers)
			{
				FDebugStaticLightingRay DebugRay(VertexToPhotonRay.Start, VertexToPhotonRay.End, Intersection.bIntersects);
				if (Intersection.bIntersects)
				{
					DebugRay.End = Intersection.IntersectionVertex.WorldPosition;
				}
				DebugOutput.ShadowRays.AddItem(DebugRay);
			}
#endif
			if (!Intersection.bIntersects)
			{
				// Break on the first visible photon
				ClosestPhoton = CurrentPhoton;
				break;
			}
		}
	}

#if ALLOW_LIGHTMAP_SAMPLE_DEBUGGING
	if (bDebugThisLookup && ClosestPhoton != NULL && PhotonMappingSettings.bVisualizePhotonGathers)
	{
		DebugOutput.GatheredPhotons.AddItem(FDebugPhoton(0, ClosestPhoton->GetPosition(), ClosestPhoton->GetSurfaceNormal(), ClosestPhoton->GetSurfaceNormal()));
	}
#endif

	return ClosestPhoton;
}
	
/** Calculates the irradiance for an irradiance photon */
FLinearColor FStaticLightingSystem::CalculatePhotonIrradiance(
	const FPhotonOctree& PhotonMap,
	INT NumPhotonsEmitted, 
	INT NumPhotonsToFind,
	FLOAT SearchDistance,
	const FIrradiancePhoton& IrradiancePhoton,
	UBOOL bDebugThisCalculation,
	TArray<FPhoton>& TempFoundPhotons,
	FCalculateIrradiancePhotonStats& Stats) const
{
	// Empty TempFoundPhotons without causing any allocations / frees
	TempFoundPhotons.Empty(TempFoundPhotons.Num() + TempFoundPhotons.GetSlack());

	const FLOAT MaxFoundDistanceSquared = FindNearbyPhotonsSorted(
		PhotonMap, 
		IrradiancePhoton.GetPosition(), 
		IrradiancePhoton.GetSurfaceNormal(), 
		NumPhotonsToFind,
		SearchDistance, 
		FALSE,
		bDebugThisCalculation, 
		TempFoundPhotons, 
		Stats);

	FLinearColor PhotonIrradiance(FLinearColor::Black);
	if (TempFoundPhotons.Num() > 0)
	{
		LIGHTINGSTAT(FScopedRDTSCTimer CalculateIrradianceTimer(Stats.CalculateIrradianceThreadTime));
		const FLOAT MaxFoundDistance = appSqrt(MaxFoundDistanceSquared);
		// Estimate the photon density using a cone filter, from the paper "Global Illumination using Photon Maps"
		const FLOAT DiskArea = (FLOAT)PI * MaxFoundDistanceSquared;
		const FLOAT ConeFilterNormalizeConstant = 1.0f - 2.0f / (3.0f * PhotonMappingSettings.ConeFilterConstant);
		const FLOAT ConstantWeight = 1.0f / (ConeFilterNormalizeConstant * NumPhotonsEmitted * DiskArea);
		for (INT PhotonIndex = 0; PhotonIndex < TempFoundPhotons.Num(); PhotonIndex++)
		{
			const FPhoton& CurrentPhoton = TempFoundPhotons(PhotonIndex);
			if ((IrradiancePhoton.GetSurfaceNormal() | CurrentPhoton.GetIncidentDirection()) > 0.0f)
			{
				const FLOAT PhotonDistance = (CurrentPhoton.GetPosition() - IrradiancePhoton.GetPosition()).Size();
				const FLOAT ConeWeight = Max(1.0f - PhotonDistance / (PhotonMappingSettings.ConeFilterConstant * MaxFoundDistance), 0.0f);
				PhotonIrradiance += CurrentPhoton.GetPower() * ConeWeight * ConstantWeight;
			}
		}
	}
	return PhotonIrradiance;
}

/** Calculates incident radiance at a vertex from the given photon map. */
FGatheredLightSample FStaticLightingSystem::CalculatePhotonIncidentRadiance(
	const FPhotonOctree& PhotonMap,
	INT NumPhotonsEmitted, 
	FLOAT SearchDistance,
	const FStaticLightingVertex& Vertex,
	UBOOL bDebugThisDensityEstimation) const
{
	TArray<FPhoton> FoundPhotons;
	FFindNearbyPhotonStats DummyStats;
	FindNearbyPhotonsIterative(PhotonMap, Vertex.WorldPosition, Vertex.WorldTangentZ, 1, SearchDistance, SearchDistance, bDebugThisDensityEstimation, FALSE, FoundPhotons, DummyStats);

	FGatheredLightSample PhotonIncidentRadiance;
	if (FoundPhotons.Num() > 0)
	{
		// Estimate the photon density using a cone filter, from the paper "Global Illumination using Photon Maps"
		const FLOAT DiskArea = (FLOAT)PI * SearchDistance * SearchDistance;
		const FLOAT ConeFilterNormalizeConstant = 1.0f - 2.0f / (3.0f * PhotonMappingSettings.ConeFilterConstant);
		const FLOAT ConstantWeight = 1.0f / (ConeFilterNormalizeConstant * NumPhotonsEmitted * DiskArea);
		for (INT PhotonIndex = 0; PhotonIndex < FoundPhotons.Num(); PhotonIndex++)
		{
			const FPhoton& CurrentPhoton = FoundPhotons(PhotonIndex);
			const FVector4 TangentPathDirection = Vertex.TransformWorldVectorToTangent(CurrentPhoton.GetIncidentDirection());
			if (TangentPathDirection.Z > 0)
			{
				const FLOAT PhotonDistance = (CurrentPhoton.GetPosition() - Vertex.WorldPosition).Size();
				const FLOAT ConeWeight = Max(1.0f - PhotonDistance / (PhotonMappingSettings.ConeFilterConstant * SearchDistance), 0.0f);
				PhotonIncidentRadiance.AddWeighted(FGatheredLightSample::PointLight(CurrentPhoton.GetPower(), TangentPathDirection), ConeWeight * ConstantWeight);
			}
		}
	}
	
	return PhotonIncidentRadiance;
}

/** Calculates exitant radiance at a vertex from the given photon map. */
FLinearColor FStaticLightingSystem::CalculatePhotonExitantRadiance(
	const FPhotonOctree& PhotonMap,
	INT NumPhotonsEmitted, 
	FLOAT SearchDistance,
	const FStaticLightingMesh* Mesh,
	const FStaticLightingVertex& Vertex,
	INT ElementIndex,
	const FVector4& OutgoingDirection,
	UBOOL bDebugThisDensityEstimation) const
{
	TArray<FPhoton> FoundPhotons;
	FFindNearbyPhotonStats DummyStats;
	FindNearbyPhotonsIterative(PhotonMap, Vertex.WorldPosition, Vertex.WorldTangentZ, 1, SearchDistance, SearchDistance, bDebugThisDensityEstimation, FALSE, FoundPhotons, DummyStats);

	FLinearColor AccumulatedRadiance(FLinearColor::Black);
	if (FoundPhotons.Num() > 0)
	{
		// Estimate the photon density using a cone filter, from the paper "Global Illumination using Photon Maps"
		const FLOAT DiskArea = (FLOAT)PI * SearchDistance * SearchDistance;
		const FLOAT ConeFilterNormalizeConstant = 1.0f - 2.0f / (3.0f * PhotonMappingSettings.ConeFilterConstant);
		const FLOAT ConstantWeight = 1.0f / (ConeFilterNormalizeConstant * NumPhotonsEmitted * DiskArea);
		for (INT PhotonIndex = 0; PhotonIndex < FoundPhotons.Num(); PhotonIndex++)
		{
			const FPhoton& CurrentPhoton = FoundPhotons(PhotonIndex);
			if ((Vertex.WorldTangentZ | CurrentPhoton.GetIncidentDirection()) > 0.0f)
			{
				const FLOAT PhotonDistance = (CurrentPhoton.GetPosition() - Vertex.WorldPosition).Size();
				const FLOAT ConeWeight = Max(1.0f - PhotonDistance / (PhotonMappingSettings.ConeFilterConstant * SearchDistance), 0.0f);
				const FLinearColor BRDF = Mesh->EvaluateBRDF(Vertex, ElementIndex, CurrentPhoton.GetIncidentDirection(), OutgoingDirection);
				AccumulatedRadiance += CurrentPhoton.GetPower() * ConeWeight * ConstantWeight * BRDF;
			}
		}
	}
	return AccumulatedRadiance;
}


}