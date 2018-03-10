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

#include "PxPhysicsAPI.h"
#include "extensions/PxExtensionsAPI.h"
#include "PxVehicleMetaDataObjects.h"
#include "PxExtensionMetaDataObjects.h"

namespace physx
{
	inline void SetMFrictionVsSlipGraph( PxVehicleTireData* inTireData, PxU32 idx1, PxU32 idx2, PxReal val ) { inTireData->mFrictionVsSlipGraph[idx1][idx2] = val; }
	inline PxReal GetMFrictionVsSlipGraph( const PxVehicleTireData* inTireData, PxU32 idx1, PxU32 idx2 ) 
	{ 
		return inTireData->mFrictionVsSlipGraph[idx1][idx2]; 
	}
	PX_PHYSX_CORE_API MFrictionVsSlipGraphProperty::MFrictionVsSlipGraphProperty()
									: PxExtendedDualIndexedPropertyInfo<PxVehiclePropertyInfoName::PxVehicleTireData_MFrictionVsSlipGraph
																, PxVehicleTireData
																, PxU32
																, PxU32
																, PxReal> ( "MFrictionVsSlipGraph", SetMFrictionVsSlipGraph, GetMFrictionVsSlipGraph, 3, 2 )
	{

	}
		
	inline PxU32 GetNbWheels( const PxVehicleWheels* inStats ) { return inStats->mWheelsSimData.getNbWheels(); }

	inline PxU32 GetNbTorqueCurvePair( const PxVehicleEngineData* inStats ) { return inStats->mTorqueCurve.getNbDataPairs(); }
		
    
	inline PxReal getXTorqueCurvePair( const PxVehicleEngineData* inStats, PxU32 index)
	{ 
		return inStats->mTorqueCurve.getX(index);
	}
	inline PxReal getYTorqueCurvePair( const PxVehicleEngineData* inStats, PxU32 index)
	{ 
		return inStats->mTorqueCurve.getY(index);
	}
	
	void addTorqueCurvePair(PxVehicleEngineData* inStats, const PxReal x, const PxReal y) 
	{ 
		inStats->mTorqueCurve.addPair(x, y); 
	}
	
	void clearTorqueCurvePair(PxVehicleEngineData* inStats) 
	{ 
		inStats->mTorqueCurve.clear(); 
	}

	PX_PHYSX_CORE_API MTorqueCurveProperty::MTorqueCurveProperty()
		: PxFixedSizeLookupTablePropertyInfo<PxVehiclePropertyInfoName::PxVehicleEngineData_MTorqueCurve
				, PxVehicleEngineData
				, PxU32
				, PxReal>("MTorqueCurve", getXTorqueCurvePair, getYTorqueCurvePair, GetNbTorqueCurvePair, addTorqueCurvePair, clearTorqueCurvePair)
	{
	}


}

