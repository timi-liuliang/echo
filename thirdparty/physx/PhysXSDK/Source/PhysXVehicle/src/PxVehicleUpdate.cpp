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

#include "PxVehicleUpdate.h"
#include "PxVehicleSDK.h"
#include "PxVehicleSuspWheelTire4.h"
#include "PxVehicleDrive4W.h"
#include "PxVehicleDriveNW.h"
#include "PxVehicleDriveTank.h"
#include "PxVehicleNoDrive.h"
#include "PxVehicleSuspLimitConstraintShader.h"
#include "PxVehicleDefaults.h"
#include "PxVehicleTireFriction.h"
#include "PxVehicleUtilTelemetry.h"
#include "PxVehicleLinearMath.h"
#include "PxQuat.h"
#include "PxShape.h"
#include "PxRigidDynamic.h"
#include "PxBatchQuery.h"
#include "PxHeightField.h"
#include "PxTriangleMesh.h"
#include "PxHeightFieldGeometry.h"
#include "PxTriangleMeshGeometry.h"
#include "PxConstraint.h"
#include "PxMaterial.h"
#include "PxTolerancesScale.h"
#include "PxRigidBodyExt.h"
#include "PsFoundation.h"
#include "PsUtilities.h"
#include "CmBitMap.h"
#include "PsTime.h"
#include "PxSceneLock.h"

#ifdef PX_PROFILE
#include "PxProfileEventNames.h"
#include "PxProfileZone.h"
#include "PxProfileZoneManager.h"
#endif

#if defined (PX_PSP2)
#include <stdint.h> // intptr_t
#endif

using namespace physx;


//TODO: lsd - handle case where wheels are spinning in different directions.
//TODO: ackermann - use faster approximate functions for PxTan/PxATan because the results don't need to be too accurate here.
//TODO: tire lat slip - do we really use PxAbs(vz) as denominator, that's not in the paper?
//TODO: toe vs jounce table.
//TODO: pneumatic trail.
//TODO: we probably need to have a graphics jounce and a physics jounce and 
//TODO: expose sticky friction values in api.
//TODO: blend the graphics jounce towards the physics jounce to avoid graphical pops at kerbs etc.
//TODO: better graph of friction vs slip.  Need to account for negative slip and positive slip differences.

namespace physx
{

////////////////////////////////////////////////////////////////////////////
//Implementation of public api function PxVehicleSetBasisVectors
////////////////////////////////////////////////////////////////////////////

PxVec3 gRight(1.f,0,0);
PxVec3 gUp(0,1.f,0);
PxVec3 gForward(0,0,1.f);

void PxVehicleSetBasisVectors(const PxVec3& up, const PxVec3& forward)
{
	gRight=up.cross(forward);
	gUp=up;
	gForward=forward;
}

////////////////////////////////////////////////////////////////////////////
//Implementation of public api function PxVehicleSetUpdateMode
////////////////////////////////////////////////////////////////////////////

bool gApplyForces=false;

void PxVehicleSetUpdateMode(PxVehicleUpdateMode::Enum vehicleUpdateMode)
{
	switch(vehicleUpdateMode)
	{
	case PxVehicleUpdateMode::eVELOCITY_CHANGE:
		gApplyForces=false;
		break;
	case PxVehicleUpdateMode::eACCELERATION:
		gApplyForces=true;
		break;
	default:
		gApplyForces=false;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////
//Called from PxVehicleInitSDK/PxCloseVehicleSDK
////////////////////////////////////////////////////////////////////////////



//***********************

PxF32 gThresholdForwardSpeedForWheelAngleIntegration=0;
PxF32 gRecipThresholdForwardSpeedForWheelAngleIntegration=0;
PxF32 gMinLatSpeedForTireModel=0;
PxF32 gStickyTireFrictionThresholdSpeed=0;
PxF32 gToleranceScaleLength=0;
PxF32 gMinimumSlipThreshold=0;

void setVehicleToleranceScale(const PxTolerancesScale& ts)
{
	gThresholdForwardSpeedForWheelAngleIntegration=5.0f*ts.length;
	gRecipThresholdForwardSpeedForWheelAngleIntegration=1.0f/gThresholdForwardSpeedForWheelAngleIntegration;

	gMinLatSpeedForTireModel=1.0f*ts.length;

	gStickyTireFrictionThresholdSpeed=0.2f*ts.length;

	gToleranceScaleLength=ts.length;

	gMinimumSlipThreshold = 1e-5f;
}

void resetVehicleToleranceScale()
{
	gThresholdForwardSpeedForWheelAngleIntegration=0;
	gRecipThresholdForwardSpeedForWheelAngleIntegration=0;

	gMinLatSpeedForTireModel=0;

	gStickyTireFrictionThresholdSpeed=0;

	gToleranceScaleLength=0;

	gMinimumSlipThreshold=0;
}

//***********************

const PxSerializationRegistry* gSerializationRegistry=NULL;

void setSerializationRegistryPtr(const PxSerializationRegistry* sr)
{
	gSerializationRegistry = sr;
}

const PxSerializationRegistry* resetSerializationRegistryPtr()
{
	const PxSerializationRegistry* tmp = gSerializationRegistry;
	gSerializationRegistry = NULL;
	return tmp;
}

//***********************

#ifdef PX_PROFILE

enum
{
	ePROFILE_RAYCASTS=0,
	ePROFILE_UPDATES,
	ePROFILE_POSTUPDATES,
	eMAX_NUM_PROFILES
};

char gVehicleProfileZoneEventNames[eMAX_NUM_PROFILES][64] =
{
	"PxVehicleSuspensionRaycasts", 
	"PxVehicleUpdates",
	"PxVehiclePostUpdates"
};

PxProfileEventName gProfileEventNames[eMAX_NUM_PROFILES] =
{
	PxProfileEventName(gVehicleProfileZoneEventNames[ePROFILE_RAYCASTS], ePROFILE_RAYCASTS),
	PxProfileEventName(gVehicleProfileZoneEventNames[ePROFILE_UPDATES], ePROFILE_UPDATES),
	PxProfileEventName(gVehicleProfileZoneEventNames[ePROFILE_POSTUPDATES], ePROFILE_POSTUPDATES)
};

class VehicleEventNameProvider : public PxProfileNameProvider
{
public:

	VehicleEventNameProvider()
	{
	}

	virtual PxProfileNames getProfileNames() const {return PxProfileNames(eMAX_NUM_PROFILES, gProfileEventNames);}

private:

};
VehicleEventNameProvider gEventNameProvider;

PxProfileZone* gVehicleProfileZone = NULL;

void setupVehicleProfileZones(PxFoundation& foundation, PxProfileZoneManager* profileZoneManager)
{
	if(profileZoneManager)
	{
		gVehicleProfileZone =  &PxProfileZone::createProfileZone(&foundation.getAllocator(), "PxVehicleProfiler", gEventNameProvider);
		profileZoneManager->addProfileZone(*gVehicleProfileZone);
	}
}

void releaseVehicleProfileZones()
{	
	if(gVehicleProfileZone)
	{
		PxProfileZoneManager* profileZoneManager = gVehicleProfileZone->getProfileZoneManager();
		profileZoneManager->removeProfileZone(*gVehicleProfileZone);
		gVehicleProfileZone->release();
		gVehicleProfileZone  = NULL;
	}
}

#endif //PX_PROFILE

////////////////////////////////////////////////////////////////////////////
//Profile macros
////////////////////////////////////////////////////////////////////////////

#ifdef PX_PROFILE

#define START_PROFILER(id)	{if(gVehicleProfileZone) gVehicleProfileZone->startEvent(id, 0);}
#define STOP_PROFILER(id) {if(gVehicleProfileZone) gVehicleProfileZone->stopEvent(id, 0);}

#else

#define START_PROFILER(id)	
#define STOP_PROFILER(id) 

#endif

////////////////////////////////////////////////////////////////////////////
//Global values used to trigger and control sticky tire friction constraints.
////////////////////////////////////////////////////////////////////////////

const PxF32 gStickyTireFrictionForwardDamping=0.01f;
const PxF32 gStickyTireFrictionSideDamping=0.1f;
const PxF32 gLowForwardSpeedThresholdTime=1.0f;
const PxF32 gLowSideSpeedThresholdTime=1.0f;

////////////////////////////////////////////////////////////////////////////
//Global values used to control max iteration count if estimate mode is chosen
////////////////////////////////////////////////////////////////////////////

const PxF32 gSolverTolerance = 1e-10f;

////////////////////////////////////////////////////////////////////////////
//Compute the sprung masses that satisfy the centre of mass and sprung mass coords.
////////////////////////////////////////////////////////////////////////////

#define DETERMINANT_THRESHOLD (1e-6f)

void computeSprungMasses(const PxU32 numSprungMasses, const PxVec3* sprungMassCoordinates, const PxVec3& centreOfMass, const PxReal totalMass, const PxU32 gravityDirection, PxReal* sprungMasses)
{
#ifdef PX_CHECKED
	PX_CHECK_AND_RETURN(numSprungMasses > 0,  "PxVehicleComputeSprungMasses: numSprungMasses must be greater than zero");
	PX_CHECK_AND_RETURN(numSprungMasses <= PX_MAX_NB_WHEELS, "PxVehicleComputeSprungMasses: numSprungMasses must be less than or equal to 20");
	for(PxU32 i=0;i<numSprungMasses;i++)
	{
		PX_CHECK_AND_RETURN(sprungMassCoordinates[i].isFinite(), "PxVehicleComputeSprungMasses: sprungMassCoordinates must all be valid coordinates");
	}
	PX_CHECK_AND_RETURN(totalMass > 0.0f, "PxVehicleComputeSprungMasses: totalMass must be greater than zero");
	PX_CHECK_AND_RETURN(gravityDirection<=2, "PxVehicleComputeSprungMasses: gravityDirection must be 0 or 1 or 2");
	PX_CHECK_AND_RETURN(sprungMasses, "PxVehicleComputeSprungMasses: sprungMasses must be a non-null pointer");
#endif

	if(1==numSprungMasses)
	{
		sprungMasses[0]=totalMass;
	}
	else if(2==numSprungMasses)
	{
		PxVec3 v=sprungMassCoordinates[0];
		v[gravityDirection]=0;
		PxVec3 w=sprungMassCoordinates[1]-sprungMassCoordinates[0];
		w[gravityDirection]=0;
		w.normalize();

		PxVec3 cm=centreOfMass;
		cm[gravityDirection]=0;
		PxF32 t=w.dot(cm-v);
		PxVec3 p=v+w*t;

		PxVec3 x0=sprungMassCoordinates[0];
		x0[gravityDirection]=0;
		PxVec3 x1=sprungMassCoordinates[1];
		x1[gravityDirection]=0;
		const PxF32 r0=(x0-p).dot(w);
		const PxF32 r1=(x1-p).dot(w);

		PX_CHECK_AND_RETURN(PxAbs(r0-r1) > DETERMINANT_THRESHOLD, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");

		const PxF32 m0=totalMass*r1/(r1-r0);
		const PxF32 m1=totalMass-m0;

		sprungMasses[0]=m0;
		sprungMasses[1]=m1;
	}
	else if(3==numSprungMasses)
	{
		const PxU32 d0=(gravityDirection+1)%3;
		const PxU32 d1=(gravityDirection+2)%3;

		MatrixNN A(3);
		VectorN b(3);
		A.set(0,0,sprungMassCoordinates[0][d0]);
		A.set(0,1,sprungMassCoordinates[1][d0]);
		A.set(0,2,sprungMassCoordinates[2][d0]);
		A.set(1,0,sprungMassCoordinates[0][d1]);
		A.set(1,1,sprungMassCoordinates[1][d1]);
		A.set(1,2,sprungMassCoordinates[2][d1]);
		A.set(2,0,1.f);
		A.set(2,1,1.f);
		A.set(2,2,1.f);
		b[0]=totalMass*centreOfMass[d0];
		b[1]=totalMass*centreOfMass[d1];
		b[2]=totalMass;

		VectorN result(3);
		MatrixNNLUSolver solver;
		solver.decomposeLU(A);
		PX_CHECK_AND_RETURN(PxAbs(solver.getDet()) > DETERMINANT_THRESHOLD, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");
		solver.solve(b,result);

		sprungMasses[0]=result[0];
		sprungMasses[1]=result[1];
		sprungMasses[2]=result[2];
	}
	else if(numSprungMasses>=4)
	{
		const PxU32 d0=(gravityDirection+1)%3;
		const PxU32 d1=(gravityDirection+2)%3;

		const PxF32 mbar = totalMass/(numSprungMasses*1.0f);

		//See http://en.wikipedia.org/wiki/Lagrange_multiplier
		//particularly the section on multiple constraints.

		//3 Constraint equations.
		//g0 = sum_ xi*mi=xcm	
		//g1 = sum_ zi*mi=zcm	
		//g2 = sum_ mi = totalMass		
		//Minimisation function to achieve solution with minimum mass variance.
		//f = sum_ (mi - mave)^2 
		//Lagrange terms (N equations, N+3 unknowns)
		//2*mi  - xi*lambda0 - zi*lambda1 - 1*lambda2 = 2*mave

		MatrixNN A(numSprungMasses+3);
		VectorN b(numSprungMasses+3);

		//g0, g1, g2
		for(PxU32 i=0;i<numSprungMasses;i++)
		{
			A.set(0,i,sprungMassCoordinates[i][d0]);	//g0
			A.set(1,i,sprungMassCoordinates[i][d1]);	//g1
			A.set(2,i,1.0f);							//g2
		}
		for(PxU32 i=numSprungMasses;i<numSprungMasses+3;i++)
		{
			A.set(0,i,0);								//g0 independent of lambda0,lambda1,lambda2
			A.set(1,i,0);								//g1 independent of lambda0,lambda1,lambda2
			A.set(2,i,0);								//g2 independent of lambda0,lambda1,lambda2
		}
		b[0] = totalMass*(centreOfMass[d0]);			//g0
		b[1] = totalMass*(centreOfMass[d1]);			//g1
		b[2] = totalMass;								//g2

		//Lagrange terms.
		for(PxU32 i=0;i<numSprungMasses;i++)
		{
			//Off-diagonal terms from the derivative of f
			for(PxU32 j=0;j<numSprungMasses;j++)
			{
				A.set(i+3,j,0);
			}
			//Diagonal term from the derivative of f
			A.set(i+3,i,2.f);

			//Derivative of g
			A.set(i+3,numSprungMasses+0,sprungMassCoordinates[i][d0]);
			A.set(i+3,numSprungMasses+1,sprungMassCoordinates[i][d1]);
			A.set(i+3,numSprungMasses+2,1.0f);

			//rhs.
			b[i+3] = 2*mbar;
		}

		//Solve Ax=b
		VectorN result(numSprungMasses+3);
		MatrixNNLUSolver solver;
		solver.decomposeLU(A);
		solver.solve(b,result);
		PX_CHECK_AND_RETURN(PxAbs(solver.getDet()) > DETERMINANT_THRESHOLD, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");

		for(PxU32 i=0;i<numSprungMasses;i++)
		{
			sprungMasses[i]=result[i];
		}
	}

#ifdef PX_CHECKED
	PxVec3 cm(0,0,0);
	PxF32 m = 0;
	for(PxU32 i=0;i<numSprungMasses;i++)
	{
		PX_CHECK_AND_RETURN(sprungMasses[i] >= 0, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");
		cm += sprungMassCoordinates[i]*sprungMasses[i];
		m += sprungMasses[i];
	}
	cm *= (1.0f/totalMass);
	PX_CHECK_AND_RETURN((PxAbs(totalMass - m)/totalMass)  < 1e-3f, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");
	PxVec3 diff = cm - centreOfMass;
	diff[gravityDirection]=0;
	const PxF32 diffMag = diff.magnitude();
	PX_CHECK_AND_RETURN(numSprungMasses <=2 || diffMag < 1e-3f, "PxVehicleComputeSprungMasses: Unable to determine sprung masses.  Please check the values in sprungMassCoordinates.");
#endif
}

////////////////////////////////////////////////////////////////////////////
//Work out if all wheels are in the air.
////////////////////////////////////////////////////////////////////////////

bool PxVehicleIsInAir(const PxVehicleWheelQueryResult& vehWheelQueryResults)
{
	if(!vehWheelQueryResults.wheelQueryResults)
	{
		return true;
	}

	for(PxU32 i=0;i<vehWheelQueryResults.nbWheelQueryResults;i++)
	{
		if(!vehWheelQueryResults.wheelQueryResults[i].isInAir)
		{
			return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
//Enable a 4W vehicle in either tadpole or delta configuration.
////////////////////////////////////////////////////////////////////////////

void computeDirection(PxU32& rightDirection, PxU32& upDirection)
{
	//Work out the up and right vectors.
	rightDirection = 0xffffffff;
	if(gRight == PxVec3(1.f,0,0) || gRight == PxVec3(-1.f,0,0))
	{
		rightDirection = 0;
	}
	else if(gRight == PxVec3(0,1.f,0) || gRight == PxVec3(0,-1.f,0))
	{
		rightDirection = 1;
	}
	else if(gRight == PxVec3(0,0,1.f) || gRight == PxVec3(0,0,-1.f))
	{
		rightDirection = 2;
	}
	upDirection = 0xffffffff;
	if(gUp== PxVec3(1.f,0,0) || gUp == PxVec3(-1.f,0,0))
	{
		upDirection = 0;
	}
	else if(gUp == PxVec3(0,1.f,0) || gUp == PxVec3(0,-1.f,0))
	{
		upDirection = 1;
	}
	else if(gUp == PxVec3(0,0,1.f) || gUp == PxVec3(0,0,-1.f))
	{
		upDirection = 2;
	}
}

void enable3WMode(const PxU32 rightDirection, const PxU32 upDirection, const bool removeFrontWheel, PxVehicleWheelsSimData& wheelsSimData, PxVehicleWheelsDynData& wheelsDynData, PxVehicleDriveSimData4W& driveSimData)
{
	PX_ASSERT(rightDirection < 3);
	PX_ASSERT(upDirection < 3);

	const PxU32 wheelToRemove = removeFrontWheel ? PxVehicleDrive4WWheelOrder::eFRONT_LEFT : PxVehicleDrive4WWheelOrder::eREAR_LEFT;
	const PxU32 wheelToModify =  removeFrontWheel ? PxVehicleDrive4WWheelOrder::eFRONT_RIGHT : PxVehicleDrive4WWheelOrder::eREAR_RIGHT;

	//Disable the wheel.
	wheelsSimData.disableWheel(wheelToRemove);

	//Make sure the wheel's corresponding PxShape does not get posed again.
	wheelsSimData.setWheelShapeMapping(wheelToRemove, -1);

	//Set the angular speed to 0.0f
	wheelsDynData.setWheelRotationSpeed(wheelToRemove, 0.0f);

	//Disable Ackermann steering.
	//If the front wheel is to be removed and the front wheels can steer then disable Ackermann correction.
	//If the rear wheel is to be removed and the rear wheels can steer then disable Ackermann correction.
	if(removeFrontWheel && 
		(wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).mMaxSteer!=0.0f ||
		wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).mMaxSteer!=0.0f))
	{
		PxVehicleAckermannGeometryData ackermannData=driveSimData.getAckermannGeometryData();
		ackermannData.mAccuracy=0.0f;
		driveSimData.setAckermannGeometryData(ackermannData);
	}
	if(!removeFrontWheel && 
		(wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).mMaxSteer!=0.0f ||
		wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eREAR_LEFT).mMaxSteer!=0.0f))
	{
		PxVehicleAckermannGeometryData ackermannData=driveSimData.getAckermannGeometryData();
		ackermannData.mAccuracy=0.0f;
		driveSimData.setAckermannGeometryData(ackermannData);
	}

	//We need to set up the differential to make sure that no drive torque is delivered to the disabled wheel.
	PxVehicleDifferential4WData diffData =driveSimData.getDiffData();
	if(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT==wheelToModify)	
	{
		diffData.mFrontBias=PX_MAX_F32;
		diffData.mFrontLeftRightSplit=0.0f;
	}
	else
	{
		diffData.mRearBias=PX_MAX_F32;
		diffData.mRearLeftRightSplit=0.0f;
	}
	driveSimData.setDiffData(diffData);

	//Now reposition the disabled wheel so that it lies at the center of its axle.
	//The following assumes that the front and rear axles lie along the x-axis.
	{
		PxVec3 wheelCentreOffset=wheelsSimData.getWheelCentreOffset(wheelToModify);
		wheelCentreOffset[rightDirection]=0.0f;
		wheelsSimData.setWheelCentreOffset(wheelToModify,wheelCentreOffset);

		PxVec3 suspOffset=wheelsSimData.getSuspForceAppPointOffset(wheelToModify);
		suspOffset[rightDirection]=0;
		wheelsSimData.setSuspForceAppPointOffset(wheelToModify,suspOffset);

		PxVec3 tireOffset=wheelsSimData.getTireForceAppPointOffset(wheelToModify);
		tireOffset[rightDirection]=0;
		wheelsSimData.setTireForceAppPointOffset(wheelToModify,tireOffset);
	}

	//Redistribute the mass supported by 4 wheels among the 3 remaining enabled wheels.
	//Compute the total mass supported by all 4 wheels.
	const PxF32 totalMass = 
		wheelsSimData.getSuspensionData(PxVehicleDrive4WWheelOrder::eFRONT_LEFT).mSprungMass +
		wheelsSimData.getSuspensionData(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT).mSprungMass +
		wheelsSimData.getSuspensionData(PxVehicleDrive4WWheelOrder::eREAR_LEFT).mSprungMass +
		wheelsSimData.getSuspensionData(PxVehicleDrive4WWheelOrder::eREAR_RIGHT).mSprungMass;
	//Get the wheel cm offsets of the 3 enabled wheels.
	PxVec3 cmOffsets[3]=
	{
		wheelsSimData.getWheelCentreOffset((wheelToRemove+1) % 4),
		wheelsSimData.getWheelCentreOffset((wheelToRemove+2) % 4),
		wheelsSimData.getWheelCentreOffset((wheelToRemove+3) % 4),
	};
	//Re-compute the sprung masses.
	PxF32 sprungMasses[3];
	computeSprungMasses(3, cmOffsets, PxVec3(0,0,0), totalMass, upDirection, sprungMasses);

	//Now set the new sprung masses.
	//Do this in a way that preserves the natural frequency and damping ratio of the spring.
	for(PxU32 i=0;i<3;i++)
	{
		PxVehicleSuspensionData suspData = wheelsSimData.getSuspensionData((wheelToRemove+1+i) % 4);

		const PxF32 oldSprungMass = suspData.mSprungMass;
		const PxF32 oldStrength = suspData.mSpringStrength;
		const PxF32 oldDampingRate = suspData.mSpringDamperRate;
		const PxF32 oldNaturalFrequency = PxSqrt(oldStrength/oldSprungMass);

		const PxF32 newSprungMass = sprungMasses[i];
		const PxF32 newStrength = oldNaturalFrequency*oldNaturalFrequency*newSprungMass;
		const PxF32 newDampingRate = oldDampingRate;

		suspData.mSprungMass = newSprungMass;
		suspData.mSpringStrength = newStrength;
		suspData.mSpringDamperRate = newDampingRate;
		wheelsSimData.setSuspensionData((wheelToRemove+1+i) % 4, suspData);
	}
}


////////////////////////////////////////////////////////////////////////////
//Maximum number of allowed blocks of 4 wheels
////////////////////////////////////////////////////////////////////////////

#define PX_MAX_NB_SUSPWHEELTIRE4 (PX_MAX_NB_WHEELS >>2)


////////////////////////////////////////////////////////////////////////////
//Pointers to telemetry data.  
//Set to NULL if no telemetry data is to be recorded during a vehicle update.
//Functions used throughout vehicle update to record specific vehicle data.
////////////////////////////////////////////////////////////////////////////

#if PX_DEBUG_VEHICLE_ON

//Render data.
PxVec3* gCarTireForceAppPoints=NULL;
PxVec3* gCarSuspForceAppPoints=NULL;

//Graph data
PxF32* gCarWheelGraphData[PX_MAX_NB_WHEELS]={NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};
PxF32* gCarEngineGraphData=NULL;

PX_FORCE_INLINE void updateGraphDataInternalWheelDynamics(const PxU32 startIndex, const PxF32* carWheelSpeeds)
{
	//Grab the internal rotation speeds for graphing before we update them.
	if(gCarWheelGraphData[startIndex])
	{
		for(PxU32 i=0;i<4;i++)
		{
			PX_ASSERT((startIndex+i) < PX_MAX_NB_WHEELS);
			PX_ASSERT(gCarWheelGraphData[startIndex+i]);
			gCarWheelGraphData[startIndex+i][PxVehicleWheelGraphChannel::eWHEEL_OMEGA]=carWheelSpeeds[i];
		}
	}
}

PX_FORCE_INLINE void updateGraphDataInternalEngineDynamics(const PxF32 carEngineSpeed)
{
	if(gCarEngineGraphData)
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eENGINE_REVS]=carEngineSpeed;
}

PX_FORCE_INLINE void updateGraphDataControlInputs(const PxF32 accel, const PxF32 brake, const PxF32 handbrake, const PxF32 steerLeft, const PxF32 steerRight)
{
	if(gCarEngineGraphData)
	{
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eACCEL_CONTROL]=accel;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eBRAKE_CONTROL]=brake;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eHANDBRAKE_CONTROL]=handbrake;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eSTEER_LEFT_CONTROL]=steerLeft;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eSTEER_RIGHT_CONTROL]=steerRight;
	}
}
PX_FORCE_INLINE void updateGraphDataGearRatio(const PxF32 G)
{
	if(gCarEngineGraphData)
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eGEAR_RATIO]=G;
}
PX_FORCE_INLINE void updateGraphDataEngineDriveTorque(const PxF32 engineDriveTorque)
{
	if(gCarEngineGraphData)
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eENGINE_DRIVE_TORQUE]=engineDriveTorque;
}
PX_FORCE_INLINE void updateGraphDataClutchSlip(const PxF32* wheelSpeeds, const PxF32* aveWheelSpeedContributions, const PxF32 engineSpeed, const PxF32 G)
{
	if(gCarEngineGraphData)
	{
		PxF32 averageWheelSpeed=0;
		for(PxU32 i=0;i<4;i++)
		{
			averageWheelSpeed+=wheelSpeeds[i]*aveWheelSpeedContributions[i];
		}
		averageWheelSpeed*=G;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eCLUTCH_SLIP]=averageWheelSpeed-engineSpeed;
	}
}
PX_FORCE_INLINE void updateGraphDataClutchSlipNW(const PxU32 numWheels4, const PxVehicleWheels4DynData* wheelsDynData, const PxF32* aveWheelSpeedContributions, const PxF32 engineSpeed, const PxF32 G)
{
	if(gCarEngineGraphData)
	{
		PxF32 averageWheelSpeed=0;
		for(PxU32 i=0;i<numWheels4;i++)
		{
			averageWheelSpeed+=wheelsDynData[i].mWheelSpeeds[0]*aveWheelSpeedContributions[4*i+0];
			averageWheelSpeed+=wheelsDynData[i].mWheelSpeeds[1]*aveWheelSpeedContributions[4*i+1];
			averageWheelSpeed+=wheelsDynData[i].mWheelSpeeds[2]*aveWheelSpeedContributions[4*i+2];
			averageWheelSpeed+=wheelsDynData[i].mWheelSpeeds[3]*aveWheelSpeedContributions[4*i+3];
		}
		averageWheelSpeed*=G;
		gCarEngineGraphData[PxVehicleDriveGraphChannel::eCLUTCH_SLIP]=averageWheelSpeed-engineSpeed;
	}
}

PX_FORCE_INLINE void zeroGraphDataWheels(const PxU32 startIndex, const PxU32 type)
{
	if(gCarWheelGraphData[0])
	{
		for(PxU32 i=0;i<4;i++)
		{
			PX_ASSERT((startIndex+i) < PX_MAX_NB_WHEELS);
			PX_ASSERT(gCarWheelGraphData[startIndex+i]);
			gCarWheelGraphData[startIndex+i][type]=0.0f;
		}
	}
}
PX_FORCE_INLINE void updateGraphDataSuspJounce(const PxU32 startIndex, const PxU32 wheel, const PxF32 jounce)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eJOUNCE]=jounce;
	}
}
PX_FORCE_INLINE void updateGraphDataSuspForce(const PxU32 startIndex, const PxU32 wheel, const PxF32 springForce)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eSUSPFORCE]=springForce;
	}
}
PX_FORCE_INLINE void updateGraphDataTireLoad(const PxU32 startIndex, const PxU32 wheel, const PxF32 filteredTireLoad)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eTIRELOAD]=filteredTireLoad;
	}
}
PX_FORCE_INLINE void updateGraphDataNormTireLoad(const PxU32 startIndex, const PxU32 wheel, const PxF32 filteredNormalisedTireLoad)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eNORMALIZED_TIRELOAD]=filteredNormalisedTireLoad;
	}
}
PX_FORCE_INLINE void updateGraphDataNormLongTireForce(const PxU32 startIndex, const PxU32 wheel, const PxF32 normForce)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eNORM_TIRE_LONG_FORCE]=normForce;
	}
}
PX_FORCE_INLINE void updateGraphDataNormLatTireForce(const PxU32 startIndex, const PxU32 wheel, const PxF32 normForce)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eNORM_TIRE_LAT_FORCE]=normForce;
	}
}
PX_FORCE_INLINE void updateGraphDataLatTireSlip(const PxU32 startIndex, const PxU32 wheel, const PxF32 latSlip)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eTIRE_LAT_SLIP]=latSlip;
	}
}
PX_FORCE_INLINE void updateGraphDataLongTireSlip(const PxU32 startIndex, const PxU32 wheel, const PxF32 longSlip)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eTIRE_LONG_SLIP]=longSlip;
	}
}
PX_FORCE_INLINE void updateGraphDataTireFriction(const PxU32 startIndex, const PxU32 wheel, const PxF32 friction)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eTIRE_FRICTION]=friction;
	}
}
PX_FORCE_INLINE void updateGraphDataNormTireAligningMoment(const PxU32 startIndex, const PxU32 wheel, const PxF32 normAlignMoment)
{
	if(gCarWheelGraphData[0])
	{
		PX_ASSERT((startIndex+wheel) < PX_MAX_NB_WHEELS);
		PX_ASSERT(gCarWheelGraphData[startIndex+wheel]);
		gCarWheelGraphData[startIndex+wheel][PxVehicleWheelGraphChannel::eNORM_TIRE_ALIGNING_MOMENT]=normAlignMoment;
	}
}

#endif //DEBUG_VEHICLE_ON


////////////////////////////////////////////////////////////////////////////
//Profile data structures.
////////////////////////////////////////////////////////////////////////////


#define PX_VEHICLE_PROFILE 0

enum
{
	TIMER_ADMIN=0,
	TIMER_GRAPHS,
	TIMER_COMPONENTS_UPDATE,
	TIMER_WHEELS,
	TIMER_INTERNAL_DYNAMICS_SOLVER,
	TIMER_POSTUPDATE1,
	TIMER_POSTUPDATE2,
	TIMER_POSTUPDATE3,
	TIMER_ALL,
	TIMER_RAYCASTS,
	MAX_NB_TIMERS
};

#if PX_VEHICLE_PROFILE

bool gTimerStates[MAX_NB_TIMERS]={false,false,false,false,false,false,false,false,false,false};
PxU64 gTimers[MAX_NB_TIMERS]={0,0,0,0,0,0,0,0,0,0};
PxU64 gStartTimers[MAX_NB_TIMERS]={0,0,0,0,0,0,0,0,0,0};
PxU64 gEndTimers[MAX_NB_TIMERS]={0,0,0,0,0,0,0,0,0,0};
PxU32 gTimerCount=0;
physx::Ps::Time gTimer;

PX_FORCE_INLINE void START_TIMER(const PxU32 id)
{
	PX_ASSERT(!gTimerStates[id]);
	gStartTimers[id]=gTimer.getCurrentCounterValue();
	gTimerStates[id]=true;
}

PX_FORCE_INLINE void END_TIMER(const PxU32 id)
{
	PX_ASSERT(gTimerStates[id]);
	gTimerStates[id]=false;
	gEndTimers[id]=gTimer.getCurrentCounterValue();
	gTimers[id]+=(gEndTimers[id]-gStartTimers[id]);
}

PX_FORCE_INLINE PxF32 getTimerFraction(const PxU32 id)
{
	return gTimers[id]/(1.0f*gTimers[TIMER_ALL]);
}

PX_FORCE_INLINE PxReal getTimerInMilliseconds(const PxU32 id)
{
	const PxU64 time=gTimers[id];
	const PxU64 timein10sOfNs = gTimer.getBootCounterFrequency().toTensOfNanos(time);
	return (timein10sOfNs/(gTimerCount*100*1.0f));
}

#else

PX_FORCE_INLINE void START_TIMER(const PxU32)
{
}

PX_FORCE_INLINE void END_TIMER(const PxU32)
{
}

#endif


////////////////////////////////////////////////////////////////////////////
//Hash table of PxMaterial pointers used to associate each PxMaterial pointer
//with a unique PxDrivableSurfaceType.  PxDrivableSurfaceType is just an integer
//representing an id but introducing this type allows different PxMaterial pointers
//to be associated with the same surface type.  The friction of a specific tire
//touching a specific PxMaterial is found from a 2D table using the integers for
//the tire type (stored in the tire) and drivable surface type (from the hash table).
//It would be great to use PsHashSet for the hash table of PxMaterials but
//PsHashSet will never, ever work on spu so this will need to do instead.
//Perf isn't really critical so this will do in the meantime.
//It is probably wasteful to compute the hash table each update
//but this is really not an expensive operation so keeping the api as 
//simple as possible wins out at the cost of a relatively very small number of wasted cycles.
////////////////////////////////////////////////////////////////////////////

class VehicleSurfaceTypeHashTable
{
public:

	VehicleSurfaceTypeHashTable(const PxVehicleDrivableSurfaceToTireFrictionPairs& pairs)
		: mNbEntries(pairs.mNbSurfaceTypes),
 	      mMaterials(pairs.mDrivableSurfaceMaterials),
	      mDrivableSurfaceTypes(pairs.mDrivableSurfaceTypes)
	{
		for(PxU32 i=0;i<eHASH_SIZE;i++)
		{
			mHeadIds[i]=PX_MAX_U32;
		}
		for(PxU32 i=0;i<eMAX_NB_KEYS;i++)
		{
			mNextIds[i]=PX_MAX_U32;
		}

		if(mNbEntries>0)
		{
			//Compute the number of bits to right-shift that gives the maximum number of unique hashes.
			//Keep searching until we find either a set of completely unique hashes or a peak count of unique hashes.
			PxU32 prevShift=0;
			PxU32 shift=2;
			PxU32 prevNumUniqueHashes=0;
			PxU32 currNumUniqueHashes=0;
			while( ((currNumUniqueHashes=computeNumUniqueHashes(shift)) > prevNumUniqueHashes) && currNumUniqueHashes!=mNbEntries)
			{
				prevNumUniqueHashes=currNumUniqueHashes;
				prevShift=shift;
				shift = (shift << 1);
			}
			if(currNumUniqueHashes!=mNbEntries)
			{
				//Stopped searching because we have gone past the peak number of unqiue hashes.
				mShift = prevShift;
			}
			else
			{
				//Stopped searching because we found a unique hash for each key.
				mShift = shift;
			}

			//Compute the hash values with the optimum shift.
			for(PxU32 i=0;i<mNbEntries;i++)
			{
				const PxMaterial* const material=mMaterials[i];
				const PxU32 hash=computeHash(material,mShift);
				if(PX_MAX_U32==mHeadIds[hash])
				{
					mNextIds[i]=PX_MAX_U32;
					mHeadIds[hash]=i;
				}
				else
				{
					mNextIds[i]=mHeadIds[hash];
					mHeadIds[hash]=i;
				}
			}
		}
	}
	~VehicleSurfaceTypeHashTable()
	{
	}

	PX_FORCE_INLINE PxU32 get(const PxMaterial* const key) const 
	{
		PX_ASSERT(key);
		const PxU32 hash=computeHash(key, mShift);
		PxU32 id=mHeadIds[hash];
		while(PX_MAX_U32!=id)
		{
			const PxMaterial* const mat=mMaterials[id];
			if(key==mat)
			{
				return mDrivableSurfaceTypes[id].mType;
			}
			id=mNextIds[id];
		}

		return 0;
	}

private:

	PxU32 mNbEntries; 
	const PxMaterial* const* mMaterials;
	const PxVehicleDrivableSurfaceType* mDrivableSurfaceTypes;

	static PX_FORCE_INLINE PxU32 computeHash(const PxMaterial* const key, const PxU32 shift) 
	{
		const uintptr_t ptr = (((uintptr_t) key) >> shift);
		const uintptr_t hash = (ptr & (eHASH_SIZE-1));
		return (PxU32)hash;
	}

	PxU32 computeNumUniqueHashes(const PxU32 shift) const
	{
		PxU32 words[eHASH_SIZE >>5];
		PxU8* bitmapBuffer[sizeof(Cm::BitMap)];
		Cm::BitMap* bitmap=(Cm::BitMap*)bitmapBuffer;
		bitmap->setWords(words, eHASH_SIZE >>5);

		PxU32 numUniqueHashes=0;
		PxMemZero(words, sizeof(PxU32)*(eHASH_SIZE >>5));
		for(PxU32 i=0;i<mNbEntries;i++)
		{
			const PxMaterial* const material=mMaterials[i];
			const PxU32 hash=computeHash(material, shift);
			if(!bitmap->test(hash))
			{
				bitmap->set(hash);
				numUniqueHashes++;
			}
		}
		return numUniqueHashes;
	}

	enum
	{
		eHASH_SIZE=PxVehicleDrivableSurfaceToTireFrictionPairs::eMAX_NB_SURFACE_TYPES
	};
	PxU32 mHeadIds[eHASH_SIZE];
	enum
	{
		eMAX_NB_KEYS=PxVehicleDrivableSurfaceToTireFrictionPairs::eMAX_NB_SURFACE_TYPES
	};
	PxU32 mNextIds[eMAX_NB_KEYS];

	PxU32 mShift;
};


////////////////////////////////////////////////////////////////////////////
//Compute the suspension line raycast start point and direction.
////////////////////////////////////////////////////////////////////////////

PX_INLINE void computeSuspensionRaycast
(const PxTransform& carChassisTrnsfm, const PxVec3& bodySpaceWheelCentreOffset, const PxVec3& bodySpaceSuspTravelDir, const PxF32 radius, const PxF32 maxBounce,
 PxVec3& suspLineStart, PxVec3& suspLineDir)
{
	//Direction of raycast.
	suspLineDir=carChassisTrnsfm.rotate(bodySpaceSuspTravelDir);

	//Position at top of wheel at maximum compression.
	suspLineStart=carChassisTrnsfm.transform(bodySpaceWheelCentreOffset);
	suspLineStart-=suspLineDir*(radius+maxBounce);
}


////////////////////////////////////////////////////////////////////////////
//Functions used to integrate rigid body transform and velocity.
//The sub-step system divides a specified timestep into N equal sub-steps
//and integrates the velocity and transform each sub-step.
//After all sub-steps are complete the velocity required to move the 
//associated PxRigidBody from the start transform to the transform at the end 
//of the timestep is computed and set.  If the update mode is chosen to be 
//acceleration then the acceleration is computed/set that will move the rigid body 
//from the start to end transform.  The PxRigidBody never actually has its transform 
//updated and only has its velocity or acceleration set at the very end of the timestep.
////////////////////////////////////////////////////////////////////////////


PX_INLINE void transformInertiaTensor(const PxVec3& invD, const PxMat33& M, PxMat33& mIInv)
{
	const float	axx = invD.x*M(0,0), axy = invD.x*M(1,0), axz = invD.x*M(2,0);
	const float	byx = invD.y*M(0,1), byy = invD.y*M(1,1), byz = invD.y*M(2,1);
	const float	czx = invD.z*M(0,2), czy = invD.z*M(1,2), czz = invD.z*M(2,2);

	mIInv(0,0) = axx*M(0,0) + byx*M(0,1) + czx*M(0,2);
	mIInv(1,1) = axy*M(1,0) + byy*M(1,1) + czy*M(1,2);
	mIInv(2,2) = axz*M(2,0) + byz*M(2,1) + czz*M(2,2);

	mIInv(0,1) = mIInv(1,0)	= axx*M(1,0) + byx*M(1,1) + czx*M(1,2);
	mIInv(0,2) = mIInv(2,0)	= axx*M(2,0) + byx*M(2,1) + czx*M(2,2);
	mIInv(1,2) = mIInv(2,1)	= axy*M(2,0) + byy*M(2,1) + czy*M(2,2);
}


PX_INLINE void integrateBody
(const PxF32 inverseMass, const PxVec3& invInertia, const PxVec3& force, const PxVec3& torque, const PxF32 dt,
 PxVec3& v, PxVec3& w, PxTransform& t)
{
	//Integrate linear velocity.
	v+=force*(inverseMass*dt);

	//Integrate angular velocity.
	PxMat33 inverseInertia;
	transformInertiaTensor(invInertia, PxMat33(t.q), inverseInertia);
	w+=inverseInertia*(torque*dt);

	//Integrate position.
	t.p+=v*dt;

	//Integrate quaternion.
	PxQuat wq(w.x,w.y,w.z,0.0f);
	PxQuat q=t.q;
	PxQuat qdot=wq*q*(dt*0.5f);
	q+=qdot;
	q.normalize();
	t.q=q;
}

/*
PX_INLINE void computeVelocity(const PxTransform& t1, const PxTransform& t2, const PxF32 invDT, PxVec3& v, PxVec3& w)
{
	//Linear velocity.
	v = (t2.p - t1.p)*invDT;

	//Angular velocity.
	PxQuat qw = (t2.q - t1.q)*t1.q.getConjugate()*(2.0f*invDT);
	w.x=qw.x;
	w.y=qw.y;
	w.z=qw.z;
}
*/


/////////////////////////////////////////////////////////////////////////////////////////
//Use fsel to compute the sign of a float: +1 for positive values, -1 for negative values
/////////////////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE PxF32 computeSign(const PxF32 f)
{
	return physx::intrinsics::fsel(f, physx::intrinsics::fsel(-f, 0.0f, 1.0f), -1.0f); 
}


/////////////////////////////////////////////////////////////////////////////////////////
//Get the accel/brake/handbrake as floats in range (0,1) from the inputs stored in the vehicle.
//Equivalent for tank involving thrustleft/thrustright and brakeleft/brakeright.
/////////////////////////////////////////////////////////////////////////////////////////


PX_FORCE_INLINE void getVehicle4WControlValues(const PxVehicleDriveDynData& driveDynData, PxF32& accel, PxF32& brake, PxF32& handbrake, PxF32& steerLeft, PxF32& steerRight)
{
	accel=driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL];
	brake=driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE];
	handbrake=driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE];
	steerLeft=driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT];
	steerRight=driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT];
}

PX_FORCE_INLINE void getVehicleNWControlValues(const PxVehicleDriveDynData& driveDynData, PxF32& accel, PxF32& brake, PxF32& handbrake, PxF32& steerLeft, PxF32& steerRight)
{
	accel=driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL];
	brake=driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_BRAKE];
	handbrake=driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_HANDBRAKE];
	steerLeft=driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_LEFT];
	steerRight=driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_RIGHT];
}

PX_FORCE_INLINE void getTankControlValues(const PxVehicleDriveDynData& driveDynData, PxF32& accel, PxF32& brakeLeft, PxF32& brakeRight, PxF32& thrustLeft, PxF32& thrustRight)
{
	accel=driveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_ACCEL];
	brakeLeft=driveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_LEFT];
	brakeRight=driveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_RIGHT];
	thrustLeft=driveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT];
	thrustRight=driveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT];
}


////////////////////////////////////////////////////////////////////////////
//Process autobox to initiate automatic gear changes.
//The autobox can be turned off and simulated externally by setting 
//the target gear as required prior to calling PxVehicleUpdates.
////////////////////////////////////////////////////////////////////////////


PX_FORCE_INLINE PxF32 processAutoBox(const PxU32 accelIndex, const PxF32 timestep, const PxVehicleDriveSimData& vehCoreSimData, PxVehicleDriveDynData& vehDynData)
{
	PX_ASSERT(vehDynData.getUseAutoGears());

	PxF32 autoboxCompensatedAnalogAccel = vehDynData.mControlAnalogVals[accelIndex];

	//If still undergoing a gear change triggered by the autobox 
	//then turn off the accelerator pedal.  This happens in autoboxes
	//to stop the driver revving the engine crazily then damaging the 
	//clutch when the clutch re-engages at the end of the gear change.
	const PxU32 currentGear=vehDynData.getCurrentGear();
	const PxU32 targetGear=vehDynData.getTargetGear();
	if(targetGear!=currentGear && PxVehicleGearsData::eNEUTRAL==currentGear)
	{
		autoboxCompensatedAnalogAccel = 0;
	}

	//Only process the autobox if no gear change is underway and the time passed since the last autobox
	//gear change is greater than the autobox latency.
	PxF32 autoBoxSwitchTime=vehDynData.getAutoBoxSwitchTime();
	const PxF32 autoBoxLatencyTime=vehCoreSimData.getAutoBoxData().mDownRatios[PxVehicleGearsData::eREVERSE];
	if(targetGear==currentGear && autoBoxSwitchTime > autoBoxLatencyTime)
	{
		//Work out if the autobox wants to switch up or down.
		const PxF32 normalisedEngineOmega=vehDynData.getEngineRotationSpeed()*vehCoreSimData.getEngineData().getRecipMaxOmega();
		const PxVehicleAutoBoxData& autoBoxData=vehCoreSimData.getAutoBoxData();

		bool gearUp=false;
		if(normalisedEngineOmega > autoBoxData.mUpRatios[currentGear] && PxVehicleGearsData::eREVERSE!=currentGear)
		{
			//If revs too high and not in reverse and not undergoing a gear change then switch up. 
			gearUp=true;
		}

		bool gearDown=false;
		if(normalisedEngineOmega < autoBoxData.mDownRatios[currentGear] && currentGear > PxVehicleGearsData::eFIRST)
		{
			//If revs too low and in gear greater than first and not undergoing a gear change then change down.
			gearDown=true;
		}

		//Start the gear change and reset the time since the last autobox gear change.
		if(gearUp || gearDown)
		{
			vehDynData.setGearUp(gearUp);
			vehDynData.setGearDown(gearDown);
			vehDynData.setAutoBoxSwitchTime(0.f);
		}
	}
	else
	{
		autoBoxSwitchTime+=timestep;
		vehDynData.setAutoBoxSwitchTime(autoBoxSwitchTime);
	}

	return autoboxCompensatedAnalogAccel;
}


////////////////////////////////////////////////////////////////////////////
//Process gear changes.
//If target gear not equal to current gear then a gear change needs to start.
//The gear change process begins by switching immediately in neutral and 
//staying there for a specified time.  The process ends by setting current 
//gear equal to target gear when the gear switch time has passed. 
//This can be bypassed by always forcing target gear = current gear and then 
//externally managing gear changes prior to calling PxVehicleUpdates.
////////////////////////////////////////////////////////////////////////////

void processGears(const PxF32 timestep, const PxVehicleGearsData& gears, PxVehicleDriveDynData& car)
{
	//const PxVehicleGearsData& gears=car.mVehicleSimData.getGearsData();

	//Process the gears.
	if(car.getGearUp() && gears.mNbRatios-1!=car.getCurrentGear() && car.getCurrentGear()==car.getTargetGear())
	{
		//Car wants to go up a gear and can go up a gear and not already undergoing a gear change.
		if(PxVehicleGearsData::eREVERSE==car.getCurrentGear())
		{
			//In reverse so switch to first through neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(PxVehicleGearsData::eFIRST);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
		else if(PxVehicleGearsData::eNEUTRAL==car.getCurrentGear())
		{
			//In neutral so switch to first and stay in neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(PxVehicleGearsData::eFIRST);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
		else
		{
			//Switch up a gear through neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(car.getCurrentGear() + 1);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
	}
	if(car.getGearDown() && PxVehicleGearsData::eREVERSE!=car.getCurrentGear() && car.getCurrentGear()==car.getTargetGear())
	{
		//Car wants to go down a gear and can go down a gear and not already undergoing a gear change
		if(PxVehicleGearsData::eFIRST==car.getCurrentGear())
		{
			//In first so switch to reverse through neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(PxVehicleGearsData::eREVERSE);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
		else if(PxVehicleGearsData::eNEUTRAL==car.getCurrentGear())
		{
			//In neutral so switch to reverse and stay in neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(PxVehicleGearsData::eREVERSE);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
		else
		{
			//Switch down a gear through neutral.
			car.setGearSwitchTime(0);
			car.setTargetGear(car.getCurrentGear() - 1);
			car.setCurrentGear(PxVehicleGearsData::eNEUTRAL);
		}
	}
	if(car.getCurrentGear()!=car.getTargetGear())
	{
		if(car.getGearSwitchTime()>gears.mSwitchTime)
		{
			car.setCurrentGear(car.getTargetGear());
			car.setGearSwitchTime(0);
			car.setGearDown(false);
			car.setGearUp(false);
		}
		else
		{
			car.setGearSwitchTime(car.getGearSwitchTime() + timestep);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//Helper functions to compute 
//1. the gear ratio from the current gear.
//2. the drive torque from the state of the accelerator pedal and torque curve of available torque against engine speed.
//3. engine damping rate (a blend between a rate when not accelerating and a rate when fully accelerating).
//4. clutch strength (rate at which clutch will regulate difference between engine and averaged wheel speed).
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE PxF32 computeGearRatio(const PxVehicleGearsData& gearsData, const PxU32 currentGear)
{
	const PxF32 gearRatio=gearsData.mRatios[currentGear]*gearsData.mFinalRatio;
	return gearRatio;
}

PX_FORCE_INLINE PxF32 computeEngineDriveTorque(const PxVehicleEngineData& engineData, const PxF32 omega, const PxF32 accel)
{
	const PxF32 engineDriveTorque=accel*engineData.mPeakTorque*engineData.mTorqueCurve.getYVal(omega*engineData.getRecipMaxOmega());
	return engineDriveTorque;
}

PX_FORCE_INLINE PxF32 computeEngineDampingRate(const PxVehicleEngineData& engineData, const PxU32 gear, const PxF32 accel)
{
	const PxF32 fullThrottleDamping = engineData.mDampingRateFullThrottle;
	const PxF32 zeroThrottleDamping = (PxVehicleGearsData::eNEUTRAL!=gear) ? engineData.mDampingRateZeroThrottleClutchEngaged : engineData.mDampingRateZeroThrottleClutchDisengaged;
	const PxF32 engineDamping = zeroThrottleDamping + (fullThrottleDamping-zeroThrottleDamping)*accel;
	return engineDamping;
}

PX_FORCE_INLINE PxF32 computeClutchStrength(const PxVehicleClutchData& clutchData, const PxU32 currentGear)
{
	return ((PxVehicleGearsData::eNEUTRAL!=currentGear) ? clutchData.mStrength : 0.0f);
}


////////////////////////////////////////////////////////////////////////////
//Limited slip differential.
//Split the available drive torque as a fraction of the total between up to 4 driven wheels.
//Compute the fraction that each wheel contributes to the averages wheel speed at the clutch.
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE void splitTorque
(const PxF32 w1, const PxF32 w2, const PxF32 diffBias, const PxF32 defaultSplitRatio,
 PxF32* t1, PxF32* t2)
{
	PX_ASSERT(computeSign(w1)==computeSign(w2) && 0.0f!=computeSign(w1));
	const PxF32 w1Abs=PxAbs(w1);
	const PxF32 w2Abs=PxAbs(w2);
	const PxF32 omegaMax=PxMax(w1Abs,w2Abs);
	const PxF32 omegaMin=PxMin(w1Abs,w2Abs);
	const PxF32 delta=omegaMax-diffBias*omegaMin;
	const PxF32 deltaTorque=physx::intrinsics::fsel(delta, delta/omegaMax , 0.0f);
	const PxF32 f1=physx::intrinsics::fsel(w1Abs-w2Abs, defaultSplitRatio*(1.0f-deltaTorque), defaultSplitRatio*(1.0f+deltaTorque));
	const PxF32 f2=physx::intrinsics::fsel(w1Abs-w2Abs, (1.0f-defaultSplitRatio)*(1.0f+deltaTorque), (1.0f-defaultSplitRatio)*(1.0f-deltaTorque));
	const PxF32 denom=1.0f/(f1+f2);
	*t1=f1*denom;
	*t2=f2*denom;
	PX_ASSERT((*t1 + *t2) >=0.999f && (*t1 + *t2) <=1.001f);  
}

PX_FORCE_INLINE void computeDiffTorqueRatios
(const PxVehicleDifferential4WData& diffData, const PxF32 handbrake, const PxF32* PX_RESTRICT wheelOmegas, PxF32* PX_RESTRICT diffTorqueRatios)
{
	//If the handbrake is on only deliver torque to the front wheels.
	PxU32 type=diffData.mType;
	if(handbrake>0)
	{
		switch(diffData.mType)
		{
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD:
			type=PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD;
			break;
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD:
			type=PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD;
			break;
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD:
		case PxVehicleDifferential4WData::eMAX_NB_DIFF_TYPES:
		default:
			break;
		}
	}

	const PxF32 wfl=wheelOmegas[PxVehicleDrive4WWheelOrder::eFRONT_LEFT];
	const PxF32 wfr=wheelOmegas[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT];
	const PxF32 wrl=wheelOmegas[PxVehicleDrive4WWheelOrder::eREAR_LEFT];
	const PxF32 wrr=wheelOmegas[PxVehicleDrive4WWheelOrder::eREAR_RIGHT];

	const PxF32 centreBias=diffData.mCentreBias;
	const PxF32 frontBias=diffData.mFrontBias;
	const PxF32 rearBias=diffData.mRearBias;

	const PxF32 frontRearSplit=diffData.mFrontRearSplit;
	const PxF32 frontLeftRightSplit=diffData.mFrontLeftRightSplit;
	const PxF32 rearLeftRightSplit=diffData.mRearLeftRightSplit;

	const PxF32 oneMinusFrontRearSplit=1.0f-diffData.mFrontRearSplit;
	const PxF32 oneMinusFrontLeftRightSplit=1.0f-diffData.mFrontLeftRightSplit;
	const PxF32 oneMinusRearLeftRightSplit=1.0f-diffData.mRearLeftRightSplit;

	const PxF32 swfl=computeSign(wfl);

	//Split a torque of 1 between front and rear.
	//Then split that torque between left and right.
	PxF32 torqueFrontLeft=0;
	PxF32 torqueFrontRight=0;
	PxF32 torqueRearLeft=0;
	PxF32 torqueRearRight=0;
	switch(type)
	{
	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD:
		if(0.0f!=swfl && swfl==computeSign(wfr) && swfl==computeSign(wrl) && swfl==computeSign(wrr))
		{
			PxF32 torqueFront,torqueRear;
			const PxF32 omegaFront=PxAbs(wfl+wfr);
			const PxF32 omegaRear=PxAbs(wrl+wrr);
			splitTorque(omegaFront,omegaRear,centreBias,frontRearSplit,&torqueFront,&torqueRear);
			splitTorque(wfl,wfr,frontBias,frontLeftRightSplit,&torqueFrontLeft,&torqueFrontRight);
			splitTorque(wrl,wrr,rearBias,rearLeftRightSplit,&torqueRearLeft,&torqueRearRight);
			torqueFrontLeft*=torqueFront;
			torqueFrontRight*=torqueFront;
			torqueRearLeft*=torqueRear;
			torqueRearRight*=torqueRear;
		}
		else
		{
			//TODO: need to handle this case.
			torqueFrontLeft=frontRearSplit*frontLeftRightSplit;
			torqueFrontRight=frontRearSplit*oneMinusFrontLeftRightSplit;
			torqueRearLeft=oneMinusFrontRearSplit*rearLeftRightSplit;
			torqueRearRight=oneMinusFrontRearSplit*oneMinusRearLeftRightSplit;
		}
		break;

	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD:
		if(0.0f!=swfl && swfl==computeSign(wfr))
		{
			splitTorque(wfl,wfr,frontBias,frontLeftRightSplit,&torqueFrontLeft,&torqueFrontRight);
		}
		else
		{
			torqueFrontLeft=frontLeftRightSplit;
			torqueFrontRight=oneMinusFrontLeftRightSplit;
		}
		break;

	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD:

		if(0.0f!=computeSign(wrl) && computeSign(wrl)==computeSign(wrr))
		{
			splitTorque(wrl,wrr,rearBias,rearLeftRightSplit,&torqueRearLeft,&torqueRearRight);
		}
		else
		{
			torqueRearLeft=rearLeftRightSplit;
			torqueRearRight=oneMinusRearLeftRightSplit;
		}
		break;

	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD:
		torqueFrontLeft=frontRearSplit*frontLeftRightSplit;
		torqueFrontRight=frontRearSplit*oneMinusFrontLeftRightSplit;
		torqueRearLeft=oneMinusFrontRearSplit*rearLeftRightSplit;
		torqueRearRight=oneMinusFrontRearSplit*oneMinusRearLeftRightSplit;
		break;

	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD:
		torqueFrontLeft=frontLeftRightSplit;
		torqueFrontRight=oneMinusFrontLeftRightSplit;
		break;

	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD:
		torqueRearLeft=rearLeftRightSplit;
		torqueRearRight=oneMinusRearLeftRightSplit;
		break;

	default:
		PX_ASSERT(false);
		break;
	}

	diffTorqueRatios[PxVehicleDrive4WWheelOrder::eFRONT_LEFT]=torqueFrontLeft;
	diffTorqueRatios[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT]=torqueFrontRight;
	diffTorqueRatios[PxVehicleDrive4WWheelOrder::eREAR_LEFT]=torqueRearLeft;
	diffTorqueRatios[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]=torqueRearRight;

	PX_ASSERT(((torqueFrontLeft+torqueFrontRight+torqueRearLeft+torqueRearRight) >= 0.999f) && ((torqueFrontLeft+torqueFrontRight+torqueRearLeft+torqueRearRight) <= 1.001f));
}

PX_FORCE_INLINE void computeDiffAveWheelSpeedContributions
(const PxVehicleDifferential4WData& diffData, const PxF32 handbrake, PxF32* PX_RESTRICT diffAveWheelSpeedContributions)
{
	PxU32 type=diffData.mType;

	//If the handbrake is on only deliver torque to the front wheels.
	if(handbrake>0)
	{
		switch(diffData.mType)
		{
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD:
			type=PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD;
			break;
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD:
			type=PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD;
			break;
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD:
		case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD:
		case PxVehicleDifferential4WData::eMAX_NB_DIFF_TYPES:
		default:
			break;
		}
	}

	const PxF32 frontRearSplit=diffData.mFrontRearSplit;
	const PxF32 frontLeftRightSplit=diffData.mFrontLeftRightSplit;
	const PxF32 rearLeftRightSplit=diffData.mRearLeftRightSplit;

	const PxF32 oneMinusFrontRearSplit=1.0f-diffData.mFrontRearSplit;
	const PxF32 oneMinusFrontLeftRightSplit=1.0f-diffData.mFrontLeftRightSplit;
	const PxF32 oneMinusRearLeftRightSplit=1.0f-diffData.mRearLeftRightSplit;

	switch(type)
	{
	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_4WD:
	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_4WD:		
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_LEFT]=frontRearSplit*frontLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT]=frontRearSplit*oneMinusFrontLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_LEFT]=oneMinusFrontRearSplit*rearLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]=oneMinusFrontRearSplit*oneMinusRearLeftRightSplit;
		break;
	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_FRONTWD:
	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_FRONTWD:
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_LEFT]=frontLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT]=oneMinusFrontLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_LEFT]=0.0f;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]=0.0f;
		break;
	case PxVehicleDifferential4WData::eDIFF_TYPE_LS_REARWD:
	case PxVehicleDifferential4WData::eDIFF_TYPE_OPEN_REARWD:
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_LEFT]=0.0f;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT]=0.0f;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_LEFT]=rearLeftRightSplit;
		diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]=oneMinusRearLeftRightSplit;
		break;
	default:
		PX_ASSERT(false);
		break;
	}

	PX_ASSERT((diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_LEFT] + 
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] + 
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_LEFT] +
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]) >= 0.999f &&
			   (diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_LEFT] + 
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT] + 
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_LEFT] +
			   diffAveWheelSpeedContributions[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]) <= 1.001f);
}


///////////////////////////////////////////////////////
//Tank differential
///////////////////////////////////////////////////////

void computeTankDiff
(const PxF32 thrustLeft, const PxF32 thrustRight, 
 const PxU32 numActiveWheels, const bool* activeWheelStates, 
 PxF32* aveWheelSpeedContributions, PxF32* diffTorqueRatios, PxF32* wheelGearings)
{
	const PxF32 thrustLeftAbs=PxAbs(thrustLeft);
	const PxF32 thrustRightAbs=PxAbs(thrustRight);

	//Work out now many left wheels are enabled.
	PxF32 numLeftWheels=0.0f;
	for(PxU32 i=0;i<numActiveWheels;i+=2)
	{
		if(activeWheelStates[i])
		{
			numLeftWheels+=1.0f;
		}
	}
	const PxF32 invNumEnabledWheelsLeft = numLeftWheels > 0 ? 1.0f/numLeftWheels : 0.0f;

	//Work out now many right wheels are enabled.
	PxF32 numRightWheels=0.0f;
	for(PxU32 i=1;i<numActiveWheels;i+=2)
	{
		if(activeWheelStates[i])
		{
			numRightWheels+=1.0f;
		}
	}
	const PxF32 invNumEnabledWheelsRight = numRightWheels > 0 ? 1.0f/numRightWheels : 0.0f;

	//Split the diff torque between left and right.
	PxF32 diffTorqueRatioLeft=0.5f;
	PxF32 diffTorqueRatioRight=0.5f;
	if((thrustLeftAbs + thrustRightAbs) > 1e-3f)
	{
		const PxF32 thrustDiff = 0.5f*(thrustLeftAbs - thrustRightAbs)/(thrustLeftAbs + thrustRightAbs);
		diffTorqueRatioLeft += thrustDiff;
		diffTorqueRatioRight -= thrustDiff;

	}
	diffTorqueRatioLeft *= invNumEnabledWheelsLeft;
	diffTorqueRatioRight *= invNumEnabledWheelsRight;

	//Compute the per wheel gearing.
	PxF32 wheelGearingLeft=1.0f;
	PxF32 wheelGearingRight=1.0f;
	if((thrustLeftAbs + thrustRightAbs) > 1e-3f)
	{
		wheelGearingLeft=computeSign(thrustLeft);
		wheelGearingRight=computeSign(thrustRight);
	}

	//Compute the contribution of each wheel to the average speed at the clutch.
	const PxF32 aveWheelSpeedContributionLeft = 0.5f*invNumEnabledWheelsLeft;
	const PxF32 aveWheelSpeedContributionRight = 0.5f*invNumEnabledWheelsRight;

	//Set all the left wheels.
	for(PxU32 i=0;i<numActiveWheels;i+=2)
	{
		if(activeWheelStates[i])
		{
			aveWheelSpeedContributions[i]=aveWheelSpeedContributionLeft;
			diffTorqueRatios[i]=diffTorqueRatioLeft;
			wheelGearings[i]=wheelGearingLeft;
		}
	}
	//Set all the right wheels.
	for(PxU32 i=1;i<numActiveWheels;i+=2)
	{
		if(activeWheelStates[i])
		{
			aveWheelSpeedContributions[i]=aveWheelSpeedContributionRight;
			diffTorqueRatios[i]=diffTorqueRatioRight;
			wheelGearings[i]=wheelGearingRight;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//Compute a per-wheel accelerator pedal value.
//These values are to blend the denominator normalised longitudinal slip at low speed 
//between a low value for wheels under drive torque and a high value for wheels with no 
//drive torque.  
//Using a high value allows the vehicle to come to rest smoothly.
//Using a low value gives better thrust.
////////////////////////////////////////////////////////////////////////////

void computeIsAccelApplied(const PxF32* aveWheelSpeedContributions, bool* isAccelApplied)
{
	isAccelApplied[0] = aveWheelSpeedContributions[0] != 0.0f ? true : false;
	isAccelApplied[1] = aveWheelSpeedContributions[1] != 0.0f ? true : false;
	isAccelApplied[2] = aveWheelSpeedContributions[2] != 0.0f ? true : false;
	isAccelApplied[3] = aveWheelSpeedContributions[3] != 0.0f ? true : false;
}

////////////////////////////////////////////////////////////////////////////
//Ackermann correction to steer angles.
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE void computeAckermannSteerAngles
(const PxF32 steer, const PxF32 steerGain, 
 const PxF32 ackermannAccuracy, const PxF32 width, const PxF32 axleSeparation,  
 PxF32* PX_RESTRICT leftAckermannSteerAngle, PxF32* PX_RESTRICT rightAckermannSteerAngle)
{
	PX_ASSERT(steer>=-1.01f && steer<=1.01f);
	PX_ASSERT(steerGain<PxPi);

	const PxF32 steerAngle=steer*steerGain;

	if(0==steerAngle)
	{
		*leftAckermannSteerAngle=0;
		*rightAckermannSteerAngle=0;
		return;
	}

	//Work out the ackermann steer for +ve steer then swap and negate the steer angles if the steer is -ve.
	//TODO: use faster approximate functions for PxTan/PxATan because the results don't need to be too accurate here.
	const PxF32 rightSteerAngle=PxAbs(steerAngle);
	const PxF32 dz=axleSeparation;
	const PxF32 dx=width + dz/PxTan(rightSteerAngle);
	const PxF32 leftSteerAnglePerfect=PxAtan(dz/dx);
	const PxF32 leftSteerAngle=rightSteerAngle + ackermannAccuracy*(leftSteerAnglePerfect-rightSteerAngle);
	*rightAckermannSteerAngle=physx::intrinsics::fsel(steerAngle, rightSteerAngle, -leftSteerAngle);
	*leftAckermannSteerAngle=physx::intrinsics::fsel(steerAngle, leftSteerAngle, -rightSteerAngle);
}

PX_FORCE_INLINE void computeAckermannCorrectedSteerAngles
(const PxVehicleDriveSimData4W& driveSimData, const PxVehicleWheels4SimData& wheelsSimData, const PxF32 steer, 
 PxF32* PX_RESTRICT steerAngles)
{
	const PxVehicleAckermannGeometryData& ackermannData=driveSimData.getAckermannGeometryData();
	const PxF32 ackermannAccuracy=ackermannData.mAccuracy;
	const PxF32 axleSeparation=ackermannData.mAxleSeparation;

	{
		const PxVehicleWheelData& wheelDataFL=wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eFRONT_LEFT);
		const PxVehicleWheelData& wheelDataFR=wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT);
		const PxF32 steerGainFront=PxMax(wheelDataFL.mMaxSteer,wheelDataFR.mMaxSteer);
		const PxF32 frontWidth=ackermannData.mFrontWidth;
		PxF32 frontLeftSteer,frontRightSteer;
		computeAckermannSteerAngles(steer,steerGainFront,ackermannAccuracy,frontWidth,axleSeparation,&frontLeftSteer,&frontRightSteer);
		steerAngles[PxVehicleDrive4WWheelOrder::eFRONT_LEFT]=wheelDataFL.mToeAngle+frontLeftSteer;
		steerAngles[PxVehicleDrive4WWheelOrder::eFRONT_RIGHT]=wheelDataFR.mToeAngle+frontRightSteer;
	}

	{
		const PxVehicleWheelData& wheelDataRL=wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eREAR_LEFT);
		const PxVehicleWheelData& wheelDataRR=wheelsSimData.getWheelData(PxVehicleDrive4WWheelOrder::eREAR_RIGHT);
		const PxF32 steerGainRear=PxMax(wheelDataRL.mMaxSteer,wheelDataRR.mMaxSteer);
		const PxF32 rearWidth=ackermannData.mRearWidth;
		PxF32 rearLeftSteer,rearRightSteer;
		computeAckermannSteerAngles(steer,steerGainRear,ackermannAccuracy,rearWidth,axleSeparation,&rearLeftSteer,&rearRightSteer);
		steerAngles[PxVehicleDrive4WWheelOrder::eREAR_LEFT]=wheelDataRL.mToeAngle-rearLeftSteer;
		steerAngles[PxVehicleDrive4WWheelOrder::eREAR_RIGHT]=wheelDataRR.mToeAngle-rearRightSteer;
	}
}

////////////////////////////////////////////////////////////////////////////
//Compute the wheel active states
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE void computeWheelActiveStates(const PxU32 startId, PxU32* bitmapBuffer, bool* activeStates)
{
	PX_ASSERT(!activeStates[0] && !activeStates[1] && !activeStates[2] && !activeStates[3]);

	Cm::BitMap bm;
	bm.setWords(bitmapBuffer, ((PX_MAX_NB_WHEELS + 31) & ~31) >> 5);

	if(bm.test(startId + 0))
	{
		activeStates[0]=true;
	}
	if(bm.test(startId + 1))
	{
		activeStates[1]=true;
	}
	if(bm.test(startId + 2))
	{
		activeStates[2]=true;
	}
	if(bm.test(startId + 3))
	{
		activeStates[3]=true;
	}
}


////////////////////////////////////////////////////////////////////////////
//Compute the brake and handbrake torques for different vehicle types.
//Also compute a boolean for each tire to know if the brake is applied or not.
//Can't use a single function for all types because not all vehicle types have 
//handbrakes and the brake control mechanism is different for different vehicle
//types.
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE void computeNoDriveBrakeTorques
(const PxVehicleWheelData* PX_RESTRICT wheelDatas, const PxF32* PX_RESTRICT wheelOmegas,  const PxF32* PX_RESTRICT rawBrakeTroques, 
 PxF32* PX_RESTRICT brakeTorques, bool* PX_RESTRICT isBrakeApplied)
{
	PX_UNUSED(wheelDatas);

	const PxF32 sign0=computeSign(wheelOmegas[0]); 
	brakeTorques[0]=(-sign0*rawBrakeTroques[0]);
	isBrakeApplied[0]=(rawBrakeTroques[0]!=0);

	const PxF32 sign1=computeSign(wheelOmegas[1]); 
	brakeTorques[1]=(-sign1*rawBrakeTroques[1]);
	isBrakeApplied[1]=(rawBrakeTroques[1]!=0);

	const PxF32 sign2=computeSign(wheelOmegas[2]); 
	brakeTorques[2]=(-sign2*rawBrakeTroques[2]);
	isBrakeApplied[2]=(rawBrakeTroques[2]!=0);

	const PxF32 sign3=computeSign(wheelOmegas[3]); 
	brakeTorques[3]=(-sign3*rawBrakeTroques[3]);
	isBrakeApplied[3]=(rawBrakeTroques[3]!=0);
}

PX_FORCE_INLINE void computeBrakeAndHandBrakeTorques
(const PxVehicleWheelData* PX_RESTRICT wheelDatas, const PxF32* PX_RESTRICT wheelOmegas, const PxF32 brake, const PxF32 handbrake, 
 PxF32* PX_RESTRICT brakeTorques, bool* isBrakeApplied)
{
	//At zero speed offer no brake torque allowed.

	const PxF32 sign0=computeSign(wheelOmegas[0]); 
	brakeTorques[0]=(-brake*sign0*wheelDatas[0].mMaxBrakeTorque-handbrake*sign0*wheelDatas[0].mMaxHandBrakeTorque);
	isBrakeApplied[0]=((brake*wheelDatas[0].mMaxBrakeTorque+handbrake*wheelDatas[0].mMaxHandBrakeTorque)!=0);

	const PxF32 sign1=computeSign(wheelOmegas[1]); 
	brakeTorques[1]=(-brake*sign1*wheelDatas[1].mMaxBrakeTorque-handbrake*sign1*wheelDatas[1].mMaxHandBrakeTorque);
	isBrakeApplied[1]=((brake*wheelDatas[1].mMaxBrakeTorque+handbrake*wheelDatas[1].mMaxHandBrakeTorque)!=0);

	const PxF32 sign2=computeSign(wheelOmegas[2]); 
	brakeTorques[2]=(-brake*sign2*wheelDatas[2].mMaxBrakeTorque-handbrake*sign2*wheelDatas[2].mMaxHandBrakeTorque);
	isBrakeApplied[2]=((brake*wheelDatas[2].mMaxBrakeTorque+handbrake*wheelDatas[2].mMaxHandBrakeTorque)!=0);

	const PxF32 sign3=computeSign(wheelOmegas[3]); 
	brakeTorques[3]=(-brake*sign3*wheelDatas[3].mMaxBrakeTorque-handbrake*sign3*wheelDatas[3].mMaxHandBrakeTorque);
	isBrakeApplied[3]=((brake*wheelDatas[3].mMaxBrakeTorque+handbrake*wheelDatas[3].mMaxHandBrakeTorque)!=0);
}

PX_FORCE_INLINE void computeTankBrakeTorques
(const PxVehicleWheelData* PX_RESTRICT wheelDatas, const PxF32* PX_RESTRICT wheelOmegas, const PxF32 brakeLeft, const PxF32 brakeRight, 
 PxF32* PX_RESTRICT brakeTorques, bool* isBrakeApplied)
{
	//At zero speed offer no brake torque allowed.

	const PxF32 sign0=computeSign(wheelOmegas[0]); 
	brakeTorques[0]=(-brakeLeft*sign0*wheelDatas[0].mMaxBrakeTorque);
	isBrakeApplied[0]=((brakeLeft*wheelDatas[0].mMaxBrakeTorque)!=0);

	const PxF32 sign1=computeSign(wheelOmegas[1]); 
	brakeTorques[1]=(-brakeRight*sign1*wheelDatas[1].mMaxBrakeTorque);
	isBrakeApplied[1]=((brakeRight*wheelDatas[1].mMaxBrakeTorque)!=0);

	const PxF32 sign2=computeSign(wheelOmegas[2]); 
	brakeTorques[2]=(-brakeLeft*sign2*wheelDatas[2].mMaxBrakeTorque);
	isBrakeApplied[2]=((brakeLeft*wheelDatas[2].mMaxBrakeTorque)!=0);

	const PxF32 sign3=computeSign(wheelOmegas[3]); 
	brakeTorques[3]=(-brakeRight*sign3*wheelDatas[3].mMaxBrakeTorque);
	isBrakeApplied[3]=((brakeRight*wheelDatas[3].mMaxBrakeTorque)!=0);
}


////////////////////////////////////////////////////////////////////////////
//Functions to compute inputs to tire force calculation.
//1.  Filter the normalised tire load to smooth any spikes in load.
//2.  Compute the tire lat and long directions in the ground plane.
//3.  Compute the tire lat and long slips.
//4.  Compute the friction from a graph of friction vs slip.
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE PxF32 computeFilteredNormalisedTireLoad(const PxVehicleTireLoadFilterData& filterData, const PxF32 normalisedLoad)
{
	if(normalisedLoad <= filterData.mMinNormalisedLoad)
	{
		return filterData.mMinFilteredNormalisedLoad;
	}
	else if(normalisedLoad >= filterData.mMaxNormalisedLoad)
	{
		return filterData.mMaxFilteredNormalisedLoad;
	}
	else
	{
		const PxF32 x=normalisedLoad;
		const PxF32 xmin=filterData.mMinNormalisedLoad;
		const PxF32 ymin=filterData.mMinFilteredNormalisedLoad;
		const PxF32 ymax=filterData.mMaxFilteredNormalisedLoad;
		const PxF32 recipXmaxMinusXMin=filterData.getDenominator();
		return (ymin + (x-xmin)*(ymax-ymin)*recipXmaxMinusXMin);
	}
}

PX_FORCE_INLINE void computeTireDirs(const PxVec3& chassisLatDir, const PxVec3& hitNorm, const PxF32 wheelSteerAngle, PxVec3& tireLongDir, PxVec3& tireLatDir)
{
	PX_ASSERT(chassisLatDir.magnitude()>0.999f && chassisLatDir.magnitude()<1.001f);
	PX_ASSERT(hitNorm.magnitude()>0.999f && hitNorm.magnitude()<1.001f);

	//Compute the tire axes in the ground plane.
	PxVec3 tzRaw=chassisLatDir.cross(hitNorm);
	PxVec3 txRaw=hitNorm.cross(tzRaw);
	tzRaw.normalize();
	txRaw.normalize();
	//Rotate the tire using the steer angle.
	const PxF32 cosWheelSteer=PxCos(wheelSteerAngle);
	const PxF32 sinWheelSteer=PxSin(wheelSteerAngle);
	const PxVec3 tz=tzRaw*cosWheelSteer + txRaw*sinWheelSteer;
	const PxVec3 tx=txRaw*cosWheelSteer - tzRaw*sinWheelSteer;
	tireLongDir=tz;
	tireLatDir=tx;
}

PX_FORCE_INLINE void computeTireSlips
(const PxF32 longSpeed, const PxF32 latSpeed, const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 maxDenominator,
 const bool isAccelApplied, const bool isBrakeApplied, 
 const bool isTank, 
 PxF32& longSlip, PxF32& latSlip)
{
	PX_ASSERT(maxDenominator>=0.0f);

	const PxF32 longSpeedAbs=PxAbs(longSpeed);
	const PxF32 wheelSpeed=wheelOmega*wheelRadius;
	const PxF32 wheelSpeedAbs=PxAbs(wheelSpeed);

	//Lateral slip is easy.
	latSlip = PxAtan(latSpeed/(longSpeedAbs+gMinLatSpeedForTireModel));//TODO: do we really use PxAbs(vz) as denominator?

	//If nothing is moving just avoid a divide by zero and set the long slip to zero.
	if(longSpeed==0 && wheelOmega==0)
	{
		longSlip=0.0f;
		return;
	}

	//Longitudinal slip is a bit harder because we can end up wtih zero on the denominator.
	if(isTank)
	{
		if(isBrakeApplied || isAccelApplied)
		{
			//Wheel experiencing an applied torque.
			//Use the raw denominator value plus an offset to avoid anything approaching a divide by zero.
			//When accelerating from rest the small denominator will generate really quite large 
			//slip values, which will, in turn, generate large longitudinal forces. With large 
			//time-steps this might lead to a temporary oscillation in longSlip direction and an 
			//oscillation in wheel speed direction.  The amplitude of the oscillation should be low
			//unless the timestep is really large.
			//There's not really an obvious solution to this without setting the denominator offset higher 
			//(or decreasing the timestep). Setting the denominator higher affects handling everywhere so 
			//settling for a potential temporary oscillation is probably the least worst compromise.
			//Note that we always use longSpeedAbs as denominator because in order to turn on the spot the 
			//tank needs to get strong longitudinal force when it isn't moving but the wheels are slipping.
			longSlip = (wheelSpeed - longSpeed)/(longSpeedAbs + 0.1f*gToleranceScaleLength);
		}
		else
		{
			//Wheel not experiencing an applied torque.
			//If the denominator becomes too small then the longSlip becomes large and the longitudinal force
			//can overshoot zero at large timesteps.  This can be really noticeable so it's harder to justify 
			//not taking action.  Further, the car isn't being actually driven so there is a strong case to fiddle
			//with the denominator because it doesn't really affect active handling.
			//Don't let the denominator fall below a user-specified value.  This can be tuned upwards until the  
			//oscillation in the sign of longSlip disappears. 
			longSlip = (wheelSpeed - longSpeed)/(PxMax(maxDenominator, PxMax(longSpeedAbs,wheelSpeedAbs)));
		}
	}
	else
	{
		if(isBrakeApplied || isAccelApplied)
		{
			//Wheel experiencing an applied torque.
			//Use the raw denominator value plus an offset to avoid anything approaching a divide by zero.
			//When accelerating from rest the small denominator will generate really quite large 
			//slip values, which will, in turn, generate large longitudinal forces. With large 
			//time-steps this might lead to a temporary oscillation in longSlip direction and an 
			//oscillation in wheel speed direction.  The amplitude of the oscillation should be low
			//unless the timestep is really large.
			//There's not really an obvious solution to this without setting the denominator offset higher 
			//(or decreasing the timestep). Setting the denominator higher affects handling everywhere so 
			//settling for a potential temporary oscillation is probably the least worst compromise.
			longSlip = (wheelSpeed - longSpeed)/(PxMax(longSpeedAbs,wheelSpeedAbs)+0.1f*gToleranceScaleLength);
		}
		else
		{
			//Wheel not experiencing an applied torque.
			//If the denominator becomes too small then the longSlip becomes large and the longitudinal force
			//can overshoot zero at large timesteps.  This can be really noticeable so it's harder to justify 
			//not taking action.  Further, the car isn't being actually driven so there is a strong case to fiddle
			//with the denominator because it doesn't really affect active handling.
			//Don't let the denominator fall below a user-specified value.  This can be tuned upwards until the  
			//oscillation in the sign of longSlip disappears. 
			longSlip = (wheelSpeed - longSpeed)/(PxMax(maxDenominator,PxMax(longSpeedAbs,wheelSpeedAbs)));
		}
	}
}

PX_FORCE_INLINE void computeTireFriction(const PxVehicleTireData& tireData, const PxF32 longSlip, const PxF32 frictionMultiplier, PxF32& friction)
{
	const PxF32 x0=tireData.mFrictionVsSlipGraph[0][0];
	const PxF32 y0=tireData.mFrictionVsSlipGraph[0][1];
	const PxF32 x1=tireData.mFrictionVsSlipGraph[1][0];
	const PxF32 y1=tireData.mFrictionVsSlipGraph[1][1];
	const PxF32 x2=tireData.mFrictionVsSlipGraph[2][0];
	const PxF32 y2=tireData.mFrictionVsSlipGraph[2][1];
	const PxF32 recipx1Minusx0=tireData.getFrictionVsSlipGraphRecipx1Minusx0();
	const PxF32 recipx2Minusx1=tireData.getFrictionVsSlipGraphRecipx2Minusx1();
	const PxF32 longSlipAbs=PxAbs(longSlip);
	PxF32 mu;
	if(longSlipAbs<x1)
	{
		mu=y0 + (y1-y0)*(longSlipAbs-x0)*recipx1Minusx0;
	}
	else if(longSlipAbs<x2)
	{
		mu=y1 + (y2-y1)*(longSlipAbs-x1)*recipx2Minusx1;
	}
	else
	{
		mu=y2;
	}
	PX_ASSERT(mu>=0);
	friction=mu*frictionMultiplier;
}

////////////////////////////////////////////////////////////////////////////
//Sticky tire constraints.
//Increment a timer each update that a tire has a very low longitudinal speed.
//Activate a sticky constraint when the tire has had an unbroken low long speed 
//for at least a threshold time.
//The longer the sticky constraint is active, the slower the target constraint speed 
//along the long dir.  Quickly tends towards zero.
//When the sticky constraint is activated set the long slip to zero and let
//the sticky constraint take over.
////////////////////////////////////////////////////////////////////////////

PX_FORCE_INLINE void updateLowForwardSpeedTimer
(const PxF32 longSpeed, const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 recipWheelRadius,  const bool isIntentionToAccelerate,
 const PxF32 timestep, PxF32& lowForwardSpeedTime)
{
	PX_UNUSED(wheelRadius);
	PX_UNUSED(recipWheelRadius);

	//If the tire is rotating slowly and the forward speed is slow then increment the slow forward speed timer.
	//If the intention of the driver is to accelerate the vehicle then reset the timer because the intention has been signalled NOT to bring 
	//the wheel to rest.
	PxF32 longSpeedAbs=PxAbs(longSpeed);
	if((longSpeedAbs<gStickyTireFrictionThresholdSpeed) && (PxAbs(wheelOmega)< gStickyTireFrictionThresholdSpeed*recipWheelRadius) && !isIntentionToAccelerate)
	{
		lowForwardSpeedTime+=timestep;		
	}
	else
	{
		lowForwardSpeedTime=0;
	}
}

PX_FORCE_INLINE void updateLowSideSpeedTimer
(const PxF32 latSpeed, const bool isIntentionToAccelerate, const PxF32 timestep, PxF32& lowSideSpeedTime)
{
	//If the side speed is slow then increment the slow side speed timer.
	//If the intention of the driver is to accelerate the vehicle then reset the timer because the intention has been signalled NOT to bring 
	//the wheel to rest.
	PxF32 latSpeedAbs=PxAbs(latSpeed);
	if((latSpeedAbs<gStickyTireFrictionThresholdSpeed) && !isIntentionToAccelerate)
	{
		lowSideSpeedTime+=timestep;		
	}
	else
	{
		lowSideSpeedTime=0;
	}
}


PX_FORCE_INLINE void activateStickyFrictionForwardConstraint
(const PxF32 longSpeed, const PxF32 wheelOmega, const PxF32 lowForwardSpeedTime, const bool isIntentionToAccelerate,
 bool& stickyTireActiveFlag, PxF32& stickyTireTargetSpeed)
{
	 //Setup the sticky friction constraint to bring the vehicle to rest at the tire contact point.
	 //The idea here is to resolve the singularity of the tire long slip at low vz by replacing the long force with a velocity constraint.
	 //Only do this if we can guarantee that the intention is to bring the car to rest (no accel pedal applied).
	 //Smoothly reduce error to zero to avoid bringing car immediately to rest.  This avoids graphical glitchiness.
	 //We're going to replace the longitudinal tire force with the sticky friction so set the long slip to zero to ensure zero long force.
	 //Apply sticky friction to this tire if 
	 //(1) the wheel is locked (this means the brake/handbrake must be on) and the forward speed at the tire contact point is vanishingly small and
	 //    the drive of vehicle has no intention to accelerate the vehicle.
	 //(2) the accumulated time of low forward speed is greater than a threshold.
	 PxF32 longSpeedAbs=PxAbs(longSpeed);
	 stickyTireActiveFlag=false;
	 stickyTireTargetSpeed=0.0f;
	 if((longSpeedAbs < gStickyTireFrictionThresholdSpeed && 0.0f==wheelOmega && !isIntentionToAccelerate) || lowForwardSpeedTime>gLowForwardSpeedThresholdTime)
	 {
		 stickyTireActiveFlag=true;
		 stickyTireTargetSpeed=longSpeed*gStickyTireFrictionForwardDamping;
	 }
}

PX_FORCE_INLINE void activateStickyFrictionSideConstraint
(const PxF32 latSpeed, const PxF32 lowSpeedForwardTimer, const PxF32 lowSideSpeedTimer, const bool isIntentionToAccelerate,
 bool& stickyTireActiveFlag, PxF32& stickyTireTargetSpeed)
{
	PX_UNUSED(latSpeed);
	PX_UNUSED(isIntentionToAccelerate);

	//Setup the sticky friction constraint to bring the vehicle to rest at the tire contact point.
	//Only do this if we can guarantee that the intention is to bring the car to rest (no accel pedal applied).
	//Smoothly reduce error to zero to avoid bringing car immediately to rest.  This avoids graphical glitchiness.
	//We're going to replace the lateral tire force with the sticky friction so set the lat slip to zero to ensure zero lat force.
	//Apply sticky friction to this tire if 
	//(1) the low forward speed timer is > 0.
	//(2) the accumulated time of low forward speed is greater than a threshold.
	stickyTireActiveFlag=false;
	stickyTireTargetSpeed=0.0f;
	if((lowSpeedForwardTimer > 0) && lowSideSpeedTimer>gLowSideSpeedThresholdTime)
	{
		stickyTireActiveFlag=true;
		stickyTireTargetSpeed=latSpeed*gStickyTireFrictionSideDamping;
	}
}



////////////////////////////////////////////////////////////////////////////
//Default tire force shader function.
//Taken from Michigan tire model.
//Computes tire long and lat forces plus the aligning moment arising from 
//the lat force and the torque to apply back to the wheel arising from the 
//long force (application of Newton's 3rd law).
////////////////////////////////////////////////////////////////////////////


#define ONE_TWENTYSEVENTH 0.037037f
#define ONE_THIRD 0.33333f
PX_FORCE_INLINE PxF32 smoothingFunction1(const PxF32 K)
{
	//Equation 20 in CarSimEd manual Appendix F.
	//Looks a bit like a curve of sqrt(x) for 0<x<1 but reaching 1.0 on y-axis at K=3. 
	PX_ASSERT(K>=0.0f);
	return PxMin(1.0f, K - ONE_THIRD*K*K + ONE_TWENTYSEVENTH*K*K*K);
}
PX_FORCE_INLINE PxF32 smoothingFunction2(const PxF32 K)
{
	//Equation 21 in CarSimEd manual Appendix F.
	//Rises to a peak at K=0.75 and falls back to zero by K=3
	PX_ASSERT(K>=0.0f);
	return (K - K*K + ONE_THIRD*K*K*K - ONE_TWENTYSEVENTH*K*K*K*K);
}

void PxVehicleComputeTireForceDefault
(const void* tireShaderData, 
 const PxF32 tireFriction,
 const PxF32 longSlipUnClamped, const PxF32 latSlipUnClamped, const PxF32 camberUnclamped,
 const PxF32 wheelOmega, const PxF32 wheelRadius, const PxF32 recipWheelRadius,
 const PxF32 restTireLoad, const PxF32 normalisedTireLoad, const PxF32 tireLoad,
 const PxF32 gravity, const PxF32 recipGravity,
 PxF32& wheelTorque, PxF32& tireLongForceMag, PxF32& tireLatForceMag, PxF32& tireAlignMoment)
{
	PX_UNUSED(wheelOmega);
	PX_UNUSED(recipWheelRadius);

	const PxVehicleTireData& tireData=*((PxVehicleTireData*)tireShaderData);

	PX_ASSERT(tireFriction>0);
	PX_ASSERT(tireLoad>0);

	wheelTorque=0.0f;
	tireLongForceMag=0.0f;
	tireLatForceMag=0.0f;
	tireAlignMoment=0.0f;

	//Clamp the slips to a minimum value.
	const PxF32 latSlip = PxAbs(latSlipUnClamped) >= gMinimumSlipThreshold ? latSlipUnClamped : 0.0f;
	const PxF32 longSlip = PxAbs(longSlipUnClamped) >= gMinimumSlipThreshold ? longSlipUnClamped : 0.0f;
	const PxF32 camber =  PxAbs(camberUnclamped) >= gMinimumSlipThreshold ? camberUnclamped : 0.0f;

	//If long slip/lat slip/camber are all zero than there will be zero tire force.
	if((0==latSlip)&&(0==longSlip)&&(0==camber))
	{
		return;
	}

	//Compute the lateral stiffness
	const PxF32 latStiff=restTireLoad*tireData.mLatStiffY*smoothingFunction1(normalisedTireLoad*3.0f/tireData.mLatStiffX);

	//Get the longitudinal stiffness
	const PxF32 longStiff=tireData.mLongitudinalStiffnessPerUnitGravity*gravity;
	const PxF32 recipLongStiff=tireData.getRecipLongitudinalStiffnessPerUnitGravity()*recipGravity;

	//Get the camber stiffness.
	const PxF32 camberStiff=tireData.mCamberStiffnessPerUnitGravity*gravity;

	//Carry on and compute the forces.
	const PxF32 TEff = PxTan(latSlip - camber*camberStiff/latStiff);
	const PxF32 K = PxSqrt(latStiff*TEff*latStiff*TEff + longStiff*longSlip*longStiff*longSlip) /(tireFriction*tireLoad);
	//const PxF32 KAbs=PxAbs(K);
	PxF32 FBar = smoothingFunction1(K);//K - ONE_THIRD*PxAbs(K)*K + ONE_TWENTYSEVENTH*K*K*K;
	PxF32 MBar = smoothingFunction2(K); //K - KAbs*K + ONE_THIRD*K*K*K - ONE_TWENTYSEVENTH*KAbs*K*K*K;
	//Mbar = PxMin(Mbar, 1.0f);
	PxF32 nu=1;
	if(K <= 2.0f*PxPi)
	{
		const PxF32 latOverlLong=latStiff*recipLongStiff;
		nu = 0.5f*(1.0f + latOverlLong - (1.0f - latOverlLong)*PxCos(K*0.5f));
	}
	const PxF32 FZero = tireFriction*tireLoad / (PxSqrt(longSlip*longSlip + nu*TEff*nu*TEff));
	const PxF32 fz = longSlip*FBar*FZero;
	const PxF32 fx = -nu*TEff*FBar*FZero;
	//TODO: pneumatic trail.
	const PxF32 pneumaticTrail=1.0f;
	const PxF32	fMy= nu * pneumaticTrail * TEff * MBar * FZero;

	//We can add the torque to the wheel.
	wheelTorque=-fz*wheelRadius;
	tireLongForceMag=fz;
	tireLatForceMag=fx;
	tireAlignMoment=fMy;
}

////////////////////////////////////////////////////////////////////////////
//Structures used to process blocks of 4 wheels:  process the raycast result,
//compute the suspension and tire force, store a number of report variables 
//such as tire slip, hit shape, hit material, friction etc.
////////////////////////////////////////////////////////////////////////////

class PxVehicleTireForceCalculator4
{
public:

	const void* mShaderData[4];
	PxVehicleComputeTireForce mShader;
private:
};

//This data structure is passed to processSuspTireWheels
//and represents the data that is logically constant across all sub-steps of each dt update.
struct ProcessSuspWheelTireConstData
{
	//We are integrating dt over N sub-steps.  
	//timeFraction is 1/N.
	PxF32 timeFraction;				
	//We are integrating dt over N sub-steps.  
	//subTimeStep is dt/N.
	PxF32 subTimeStep;		
	PxF32 recipSubTimeStep;

	//Gravitational acceleration vector
	PxVec3 gravity;					
	//Length of gravitational acceleration vector (saves a square root each time we need it)
	PxF32 gravityMagnitude;			
	//Reciprocal length of gravitational acceleration vector (saves a square root and divide each time we need it).
	PxF32 recipGravityMagnitude;	

	//True for tanks, false for all other vehicle types.
	//Used when computing the longitudinal and lateral slips.
	bool isTank;		

	//Minimum denominator allowed in longitudinal slip computation.
	PxF32 minLongSlipDenominator;

	//Pointer to physx actor that represents the vehicle.
	const PxRigidDynamic* vehActor;	

	//Pointer to table of friction values for each combination of material and tire type.
	const PxVehicleDrivableSurfaceToTireFrictionPairs* frictionPairs;	
};

//This data structure is passed to processSuspTireWheels
//and represents the data that is physically constant across each sub-steps of each dt update.
struct ProcessSuspWheelTireInputData
{
public:

	//True if the driver intends to pass drive torque to any wheel of the vehicle, 
	//even if none of the wheels in the block of 4 wheels processed in processSuspTireWheels are given drive torque.
	//False if the driver does not intend the vehicle to accelerate.
	//If the player intends to accelerate then no wheel will be given a sticky tire constraint.
	//This data is actually logically constant.
	bool isIntentionToAccelerate;

	//True if a wheel has a non-zero diff torque, false if a wheel has zero diff torque.
	//This data is actually logically constant.
	const bool* isAccelApplied;

	//True if a wheel has a non-zero brake torque, false if a wheel has zero brake torque.
	//This data is actually logically constant.
	const bool* isBrakeApplied; 

	//Steer angles of each wheel in radians.
	//This data is actually logically constant.
	const PxF32* steerAngles;

	//True if the wheel is not disabled, false if wheel is disabled.
	//This data is actually logically constant.
	bool* activeWheelStates;

	//Properties of the rigid body - transform.
	//This data is actually logically constant.
	PxTransform carChassisTrnsfm;
	//Properties of the rigid body - linear velocity.
	//This data is actually logically constant.
	PxVec3 carChassisLinVel;
	//Properties of the rigid body - angular velocity
	//This data is actually logically constant.
	PxVec3 carChassisAngVel;

	//Simulation data for the 4 wheels being processed in processSuspTireWheels
	//This data is actually logically constant.
	const PxVehicleWheels4SimData* vehWheels4SimData;

	//Dynamics data for the 4 wheels being processed in processSuspTireWheels
	//This data is a mixture of logically and physically constant.
	//We could update some of the data in vehWheels4DynData in processSuspTireWheels
	//but we choose to do it after.  By specifying the non-constant data members explicitly
	//in ProcessSuspWheelTireOutputData we are able to more easily keep a track of the 
	//constant and non-constant data members.  After processSuspTireWheels is complete 
	//we explicitly transfer the updated data in ProcessSuspWheelTireOutputData to vehWheels4DynData.
	//Examples are low long and lat forward speed timers.
	const PxVehicleWheels4DynData* vehWheels4DynData;

	//Shaders to calculate the tire forces.
	//This data is actually logically constant.
	const PxVehicleTireForceCalculator4* vehWheels4TireForceCalculator;

	//Filter function to filter tire load.
	//This data is actually logically constant.
	const PxVehicleTireLoadFilterData* vehWheels4TireLoadFilterData;

	//How many of the 4 wheels are real wheels (eg a 6-wheeled car has a 
	//block of 4 wheels then a 2nd block of 4 wheels with only 2 active wheels)
	//This data is actually logically constant.
	PxU32 numActiveWheels;
};

struct ProcessSuspWheelTireOutputData
{
public:

	ProcessSuspWheelTireOutputData()
	{
		PxMemZero(this, sizeof(ProcessSuspWheelTireOutputData)); 
		for(PxU32 i=0;i<4;i++)
		{
			isInAir[i]=true;
			tireSurfaceTypes[i]=PxU32(PxVehicleDrivableSurfaceType::eSURFACE_TYPE_UNKNOWN);
		}
	}


	////////////////////////////////////////////////////////////////////////////////////////////
	//The following data is stored so that it may be later passed to PxVehicleWheelQueryResult
	/////////////////////////////////////////////////////////////////////////////////////////////

	//Raycast start [most recent raycast start coord or (0,0,0) if using a cached raycast]
	PxVec3 suspLineStarts[4];
	//Raycast start [most recent raycast direction or (0,0,0) if using a cached raycast]
	PxVec3 suspLineDirs[4];
	//Raycast start [most recent raycast length or 0 if using a cached raycast]
	PxF32 suspLineLengths[4];
	//False if wheel cannot touch the ground.
	bool isInAir[4];
	//Actor hit by most recent raycast, NULL if using a cached raycast.
	PxActor* tireContactActors[4];
	//Shape hit by most recent raycast, NULL if using a cached raycast.
	PxShape* tireContactShapes[4];
	//Material hit by most recent raycast, NULL if using a cached raycast.
	PxMaterial* tireSurfaceMaterials[4];
	//Surface type of material hit by most recent raycast, eSURFACE_TYPE_UNKNOWN if using a cached raycast.
	PxU32 tireSurfaceTypes[4];
	//Contact point of raycast against either fresh contact plane from fresh raycast or cached contact plane.
	PxVec3 tireContactPoints[4];
	//Contact normal of raycast against either fresh contact plane from fresh raycast or cached contact plane.
	PxVec3 tireContactNormals[4];
	//Friction experienced by tire (value from friction table for surface/tire type combos multiplied by friction vs slip graph)
	PxF32 frictions[4];
	//Jounce experienced by suspension against fresh or cached contact plane.
	PxF32 jounces[4];
	//Suspension force to be applied to rigid body.
	PxF32 suspensionSpringForces[4];
	//Longitudinal direction of tire in the ground contact plane.
	PxVec3 tireLongitudinalDirs[4];
	//Lateral direction of tire in the ground contact plane.
	PxVec3 tireLateralDirs[4];
	//Longitudinal slip.
	PxF32 longSlips[4];
	//Lateral slip.
	PxF32 latSlips[4];

	//Forward speed of rigid body along tire longitudinal direction at tire base.
	//Used later to blend the integrated wheel rotation angle between rolling speed and computed speed
	//when the wheel rotation speeds become unreliable at low forward speeds.
	PxF32 forwardSpeeds[4];

	//Torque to be applied to wheel as 1d rigid body.  Taken from the longitudinal tire force.
	//(Newton's 3rd law means the longitudinal tire force must have an equal and opposite force).
	//(The lateral tire force is assumed to be absorbed by the suspension geometry).
	PxF32 tireTorques[4];

	//Force to be applied to rigid body (accumulated across all 4 wheels/tires/suspensions).
	PxVec3 chassisForce;
	//Torque to be applied to rigid body (accumulated across all 4 wheels/tires/suspensions).
	PxVec3 chassisTorque;

	//Updated time spend at low forward speed.  
	//Needs copied back to vehWheels4DynData
	PxF32 newLowForwardSpeedTimers[4];
	//Updated time spend at low lateral speed.  
	//Needs copied back to vehWheels4DynData
	PxF32 newLowSideSpeedTimers[4];

	//Constraint data for sticky tire constraints and suspension limit constraints.
	//Needs copied back to vehWheels4DynData
	PxVehicleConstraintShader::VehicleConstraintData vehConstraintData;

	//Store the details of the raycast hit results so that they may be re-used 
	//next update in the event that no raycast is performed.
	//If no raycast was performed then the cached values are just re-copied here
	//so that they can be recycled without having to do further tests on whether
	//raycasts were performed or not.
	//Needs copied back to vehWheels4DynData after the last call to processSuspTireWheels.
	//The union of cached hit data and susp raycast data means we don't want to overwrite the 
	//raycast data until we don't need it any more.
	PxU32 cachedHitCounts[4];
	PxVec4 cachedHitPlanes[4];
	PxF32 cachedHitDistances[4];
	PxF32 cachedFrictionMultipliers[4];

	//Store the details of the force applied to any dynamic actor hit by wheel raycasts.
	PxRigidDynamic* hitActors[4];
	PxVec3 hitActorForces[4];
	PxVec3 hitActorForcePositions[4];
};



////////////////////////////////////////////////////////////////////////////
//Monster function to 
//1.  compute the tire/susp forces
//2.  compute the torque to apply to the 1D rigid body wheel arising from the long tire force
//3.  process the sticky tire friction constraints 
//    (monitor and increment the low long + lat speed timers, compute data for the sticky tire constraint if necessary)
//4.  process the suspension limit constraints
//    (monitor the suspension jounce versus the suspension travel limit, compute the data for the suspension limit constraint if necessary).
//5.  record the contact plane so that it may be re-used in future updates in the absence of fresh raycasts.
//6.  record telemetry data (if necessary) and record data for reporting such as hit material, hit normal etc.
////////////////////////////////////////////////////////////////////////////

void processSuspTireWheels
(const PxU32 startWheelIndex, 
 const ProcessSuspWheelTireConstData& constData, const ProcessSuspWheelTireInputData& inputData, 
 ProcessSuspWheelTireOutputData& outputData)
{
	PX_SIMD_GUARD; //tzRaw.normalize(); in computeTireDirs threw a denorm exception on osx
	
#if PX_DEBUG_VEHICLE_ON
	PX_ASSERT(0==(startWheelIndex & 3));
#endif

#if PX_DEBUG_VEHICLE_ON
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eJOUNCE);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eSUSPFORCE);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eTIRELOAD);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eNORMALIZED_TIRELOAD);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eNORM_TIRE_LONG_FORCE);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eNORM_TIRE_LAT_FORCE);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eTIRE_LONG_SLIP);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eTIRE_LAT_SLIP);
	zeroGraphDataWheels(startWheelIndex,PxVehicleWheelGraphChannel::eTIRE_FRICTION);
#endif

	//Unpack the logically constant data.
	const PxVec3& gravity=constData.gravity;
	const PxF32 timeFraction=constData.timeFraction;
	const PxF32 timeStep=constData.subTimeStep;
	const PxF32 recipTimeStep=constData.recipSubTimeStep;
	const PxF32 recipGravityMagnitude=constData.recipGravityMagnitude;
	const PxF32 gravityMagnitude=constData.gravityMagnitude;
	const bool isTank=constData.isTank;
	const PxF32 minLongSlipDenominator=constData.minLongSlipDenominator;

	//Unpack the input data (physically constant data).
	const PxVehicleWheels4SimData& wheelsSimData=*inputData.vehWheels4SimData;
	const PxVehicleWheels4DynData& wheelsDynData=*inputData.vehWheels4DynData;
	const PxVehicleTireForceCalculator4& tireForceCalculator=*inputData.vehWheels4TireForceCalculator;
	const PxVehicleTireLoadFilterData& tireLoadFilterData=*inputData.vehWheels4TireLoadFilterData;
	//More constant data describing the 4 wheels under consideration.
	const PxF32* PX_RESTRICT tireRestLoads=wheelsSimData.getTireRestLoadsArray();
	const PxF32* PX_RESTRICT recipTireRestLoads=wheelsSimData.getRecipTireRestLoadsArray();
	//Compute the right direction for later.
	const PxTransform& carChassisTrnsfm=inputData.carChassisTrnsfm;
	const PxVec3 latDir=inputData.carChassisTrnsfm.rotate(gRight);
	//Unpack the linear and angular velocity of the rigid body.
	const PxVec3& carChassisLinVel=inputData.carChassisLinVel;
	const PxVec3& carChassisAngVel=inputData.carChassisAngVel;
	//Inputs (accel, steer, brake).
	const bool isIntentionToAccelerate=inputData.isIntentionToAccelerate;
	const PxF32* steerAngles=inputData.steerAngles;
	const bool* isBrakeApplied=inputData.isBrakeApplied;
	const bool* isAccelApplied=inputData.isAccelApplied;
	//Disabled/enabled wheel states.
	const bool* activeWheelStates=inputData.activeWheelStates;
	//Current low forward/side speed timers.  Note that the updated timers
	//are stored in newLowForwardSpeedTimers and newLowSideSpeedTimers.
	const PxF32* PX_RESTRICT lowForwardSpeedTimers=wheelsDynData.mTireLowForwardSpeedTimers;
	const PxF32* PX_RESTRICT lowSideSpeedTimers=wheelsDynData.mTireLowSideSpeedTimers;
	//Susp jounces from previous call to processSuspTireWheels.
	const PxF32* PX_RESTRICT prevJounces=wheelsDynData.mJounces;

	//Unpack the output data (the data we are going to compute).
	//Start with the data stored for reporting to PxVehicleWheelQueryResult.
	//PxVec3* suspLineStarts=outputData.suspLineStarts;
	//PxVec3* suspLineDirs=outputData.suspLineDirs;
	//PxF32* suspLineLengths=outputData.suspLineLengths;
	bool* isInAirs=outputData.isInAir;	
	PxActor** tireContactActors=outputData.tireContactActors;
	PxShape** tireContactShapes=outputData.tireContactShapes;
	PxMaterial** tireSurfaceMaterials=outputData.tireSurfaceMaterials;
	PxU32* tireSurfaceTypes=outputData.tireSurfaceTypes;
	PxVec3* tireContactPoints=outputData.tireContactPoints;
	PxVec3* tireContactNormals=outputData.tireContactNormals;
	PxF32* frictions=outputData.frictions;
	PxF32* jounces=outputData.jounces;
	PxF32* suspensionSpringForces=outputData.suspensionSpringForces;
	PxVec3* tireLongitudinalDirs=outputData.tireLongitudinalDirs;
	PxVec3* tireLateralDirs=outputData.tireLateralDirs;
	PxF32* longSlips=outputData.longSlips;
	PxF32* latSlips=outputData.latSlips;
	//Now unpack the forward speeds that are used later to blend the integrated wheel 
	//rotation angle between rolling speed and computed speed when the wheel rotation 
	//speeds become unreliable at low forward speeds.
	PxF32* forwardSpeeds=outputData.forwardSpeeds;
	//Unpack the real outputs of this function (wheel torques to apply to 1d rigid body wheel and forces/torques
	//to apply to 3d rigid body chassis).
	PxF32* tireTorques=outputData.tireTorques;
	PxVec3& chassisForce=outputData.chassisForce;
	PxVec3& chassisTorque=outputData.chassisTorque;
	//Unpack the low speed timers that will be computed.
	PxF32* newLowForwardSpeedTimers=outputData.newLowForwardSpeedTimers;
	PxF32* newLowSideSpeedTimers=outputData.newLowSideSpeedTimers;
	//Unpack the constraint data for suspensions limit and sticky tire constraints.
	//Susp limits.
	bool* suspLimitActiveFlags=outputData.vehConstraintData.mSuspLimitData.mActiveFlags;
	PxVec3* suspLimitDirs=outputData.vehConstraintData.mSuspLimitData.mDirs;
	PxVec3* suspLimitCMOffsets=outputData.vehConstraintData.mSuspLimitData.mCMOffsets;
	PxF32* suspLimitErrors=outputData.vehConstraintData.mSuspLimitData.mErrors;
	//Longitudinal sticky tires.
	bool* stickyTireForwardActiveFlags=outputData.vehConstraintData.mStickyTireForwardData.mActiveFlags;
	PxVec3* stickyTireForwardDirs=outputData.vehConstraintData.mStickyTireForwardData.mDirs;
	PxVec3* stickyTireForwardCMOffsets=outputData.vehConstraintData.mStickyTireForwardData.mCMOffsets;
	PxF32* stickyTireForwardTargetSpeeds=outputData.vehConstraintData.mStickyTireForwardData.mTargetSpeeds;
	//Lateral sticky tires.
	bool* stickyTireSideActiveFlags=outputData.vehConstraintData.mStickyTireSideData.mActiveFlags;
	PxVec3* stickyTireSideDirs=outputData.vehConstraintData.mStickyTireSideData.mDirs;
	PxVec3* stickyTireSideCMOffsets=outputData.vehConstraintData.mStickyTireSideData.mCMOffsets;
	PxF32* stickyTireSideTargetSpeeds=outputData.vehConstraintData.mStickyTireSideData.mTargetSpeeds;
	//Hit data.  Store the contact data so it can be reused.
	PxU32* cachedHitCounts=outputData.cachedHitCounts;
	PxVec4* cachedHitPlanes=outputData.cachedHitPlanes;
	PxF32* cachedHitDistances=outputData.cachedHitDistances;
	PxF32* cachedFrictionMultipliers=outputData.cachedFrictionMultipliers;
	//Hit actor data.
	PxRigidDynamic** hitActors=outputData.hitActors;
	PxVec3* hitActorForces=outputData.hitActorForces;
	PxVec3* hitActorForcePositions=outputData.hitActorForcePositions;

	//Compute all the hit data (counts, distances, planes, frictions, actors, shapes, materials etc etc).
	//If we just did a raycast then we need to compute all this from the hit reports.
	//If we are using cached raycast results then just copy the cached hit result data.
	PxU32 hitCounts4[4];
	PxF32 hitDistances4[4];
	PxVec4 hitPlanes4[4];
	PxF32 hitFrictionMultipliers4[4];
	PxShape* hitContactShapes4[4];
	PxRigidActor* hitContactActors4[4];
	PxMaterial* hitContactMaterials4[4];
	PxU32 hitSurfaceTypes4[4];
	PxVec3 hitContactPoints4[4];
	PxVec3 hitContactNormals4[4];
	const PxRaycastQueryResult* PX_RESTRICT sqResults=inputData.vehWheels4DynData->mSqResults;
	if(sqResults)
	{
		for(PxU32 i=0;i<inputData.numActiveWheels;i++)
		{
			const PxU32 hitCount=PxU32(sqResults[i].hasBlock);
			if(hitCount)
			{
				const PxRaycastHit& hit=sqResults[i].block;

				//Test that the hit actor isn't the vehicle itself.
				PX_CHECK_AND_RETURN(constData.vehActor != hit.actor, "Vehicle raycast has hit itself.  Please check the filter data to avoid this.");

				//Hit count.
				hitCounts4[i]=PxU32(sqResults[i].hasBlock);

				//Hit distance.
				hitDistances4[i]=hit.distance;

				//Hit plane.
				const PxVec3 hitPos=hit.position;
				const PxVec3 hitNorm=hit.normal;
				const PxF32 hitD=-hitNorm.dot(hitPos);
				hitPlanes4[i]=PxVec4(hitNorm, hitD);

				//Hit friction.
				PxU32 surfaceType=0;
				PxMaterial* material = NULL;
				{
					//Only get the material if the raycast started outside the hit shape.
					material = (hit.distance != 0.0f) ? hit.shape->getMaterialFromInternalFaceIndex(hit.faceIndex) : NULL; 
				}
				//Hash table for quick lookup of drivable surface type from material.
				const PxVehicleDrivableSurfaceToTireFrictionPairs* PX_RESTRICT frictionPairs=constData.frictionPairs;
				VehicleSurfaceTypeHashTable surfaceTypeHashTable(*constData.frictionPairs);
				if(NULL!=material)
				{
					surfaceType=surfaceTypeHashTable.get(material);
				}
				const PxVehicleTireData& tire=inputData.vehWheels4SimData->getTireData(i);
				const PxF32 frictionMultiplier=frictionPairs->getTypePairFriction(surfaceType,tire.mType);
				PX_ASSERT(frictionMultiplier>=0);
				hitFrictionMultipliers4[i]=frictionMultiplier;

				//Hit report.
				hitContactShapes4[i]=hit.shape;
				hitContactActors4[i]=hit.actor;
				hitContactMaterials4[i]=material;
				hitSurfaceTypes4[i]=surfaceType;
				hitContactPoints4[i]=hit.position;
				hitContactNormals4[i]=hit.normal;

				//When we're finished here we need to copy this back to the vehicle.
				cachedHitCounts[i]=hitCount;
				cachedHitPlanes[i]=PxVec4(hitNorm, hitD);
				cachedHitDistances[i]=hit.distance;
				cachedFrictionMultipliers[i]=frictionMultiplier;
			}
			else
			{
				hitCounts4[i]=0;
				hitDistances4[i]=0;
				hitPlanes4[i]=PxVec4(0,0,0,0);
				hitFrictionMultipliers4[i]=0;
				hitContactShapes4[i]=NULL;
				hitContactActors4[i]=NULL;
				hitContactMaterials4[i]=NULL;
				hitSurfaceTypes4[i]=PxU32(PxVehicleDrivableSurfaceType::eSURFACE_TYPE_UNKNOWN);
				hitContactPoints4[i]=PxVec3(0,0,0);
				hitContactNormals4[i]=PxVec3(0,0,0);

				//When we're finished here we need to copy this back to the vehicle.
				cachedHitCounts[i]=0;
				cachedHitPlanes[i]=PxVec4(0,0,0,0);
				cachedHitDistances[i]=0;
				cachedFrictionMultipliers[i]=0;
			}
		}
	}
	else
	{
		//If we have no sq results then we must have a cached raycast hit result.
		const PxVehicleWheels4DynData::CachedSuspLineRaycastHitResult& cachedHitResult = 
			(const PxVehicleWheels4DynData::CachedSuspLineRaycastHitResult&)inputData.vehWheels4DynData->mRaycastsOrCachedHitResults;

		for(PxU32 i=0;i<inputData.numActiveWheels;i++)
		{
			hitCounts4[i]=cachedHitResult.mCounts[i];
			hitDistances4[i]=cachedHitResult.mDistances[i];
			hitPlanes4[i]=cachedHitResult.mPlanes[i];
			hitFrictionMultipliers4[i]=cachedHitResult.mFrictionMultipliers[i];
			hitContactShapes4[i]=NULL;
			hitContactActors4[i]=NULL;
			hitContactMaterials4[i]=NULL;
			hitSurfaceTypes4[i]=PxU32(PxVehicleDrivableSurfaceType::eSURFACE_TYPE_UNKNOWN);
			hitContactPoints4[i]=PxVec3(0,0,0);
			hitContactNormals4[i]=PxVec3(0,0,0);

			//When we're finished here we need to copy this back to the vehicle.
			cachedHitCounts[i]=cachedHitResult.mCounts[i];
			cachedHitPlanes[i]=cachedHitResult.mPlanes[i];
			cachedHitDistances[i]=cachedHitResult.mDistances[i];
			cachedFrictionMultipliers[i]=cachedHitResult.mFrictionMultipliers[i];
		}
	}

	
	//Iterate over all 4 wheels.
	for(PxU32 i=0;i<4;i++)
	{
		//Constant data of the ith wheel.
		const PxVehicleWheelData& wheel=wheelsSimData.getWheelData(i);
		const PxVehicleSuspensionData& susp=wheelsSimData.getSuspensionData(i);
		const PxVehicleTireData& tire=wheelsSimData.getTireData(i);
		const PxVec3& bodySpaceWheelCentreOffset=wheelsSimData.getWheelCentreOffset(i);
		const PxVec3& bodySpaceSuspTravelDir=wheelsSimData.getSuspTravelDirection(i);

		//Take a copy of the low forward/side speed timer of the ith wheel (time spent at low forward/side speed)
		//Do this so we can quickly tell if the low/side forward speed timer changes.
		newLowForwardSpeedTimers[i]=lowForwardSpeedTimers[i];
		newLowSideSpeedTimers[i]=lowSideSpeedTimers[i];

		//Reset the graph of the jounce to max droop.
		//This will get updated as we learn more about the suspension.
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataSuspJounce(startWheelIndex, i,-susp.mMaxDroop);
#endif

		//Reset the jounce to max droop.
		//This will get updated as we learn more about the suspension and tire.
		PxF32 jounce=-susp.mMaxDroop;
		jounces[i]=jounce;

		//Deactivate the sticky tire and susp limit constraint.
		//These will get updated as we learn more about the suspension and tire.
		suspLimitActiveFlags[i]=false;
		suspLimitErrors[i]=0.0f;
		stickyTireForwardActiveFlags[i]=false;
		stickyTireForwardTargetSpeeds[i]=0;
		stickyTireSideActiveFlags[i]=false;
		stickyTireSideTargetSpeeds[i]=0;

		//The vehicle is in the air until we know otherwise.
		isInAirs[i]=true;

		//If there has been a hit then compute the suspension force and tire load.
		//Ignore the hit if the raycast starts inside the hit shape (eg wheel completely underneath surface of a heightfield).
		const bool activeWheelState=activeWheelStates[i];
		const PxU32 numHits=hitCounts4[i];
		const PxF32 hitDistance=hitDistances4[i];
		PxVec3 v;
		PxVec3 w;
		const PxVec3 hitNorm(hitPlanes4[i].x, hitPlanes4[i].y,hitPlanes4[i].z);
		computeSuspensionRaycast(carChassisTrnsfm,bodySpaceWheelCentreOffset,bodySpaceSuspTravelDir,wheel.mRadius,susp.mMaxCompression,v,w);
		if(activeWheelState && numHits>0 && hitDistance!=0.0f && hitNorm.dot(w)<0.0f)
		{
			//Get the friction multiplier from the combination of surface type and tire type.
			const PxF32 frictionMultiplier=hitFrictionMultipliers4[i];
			PX_ASSERT(frictionMultiplier>=0);

			//Work out the point on the susp line that touches the intersection plane.
			//n.(v+wt)+d=0 where n,d describe the plane; v,w describe the susp ray; t is the point on the susp line.
			//t=-(n.v + d)/n.w
			const PxF32 hitD = hitPlanes4[i].w;
			const PxVec3& n=hitNorm;
			const PxF32 d=hitD;
			const PxF32 T=-(n.dot(v) + d)/(n.dot(w));

			//The rest pos of the susp line is 2*radius + maxBounce.
			const PxF32 restT=2.0f*wheel.mRadius+susp.mMaxCompression;

			//Compute the spring compression ie the difference between T and restT.
			//+ve means that the spring is compressed
			//-ve means that the spring is elongated.
			PxF32 dx=restT-T;

			//If the spring is elongated past its max droop then the wheel isn't touching the ground.
			//In this case the spring offers zero force and provides no support for the chassis/sprung mass.
			//Only carry on computing the spring force if the wheel is touching the ground.
			PX_ASSERT(susp.mMaxCompression>=0);
			PX_ASSERT(susp.mMaxDroop>=0);
			if(dx > -susp.mMaxDroop)
			{
				//We can record the hit shape, hit actor, hit material, hit surface type, hit point, and hit normal now because we've got a hit.
				tireContactShapes[i]=hitContactShapes4[i];
				tireContactActors[i]=hitContactActors4[i];
				tireSurfaceMaterials[i]=hitContactMaterials4[i];
				tireSurfaceTypes[i]=hitSurfaceTypes4[i];
				tireContactPoints[i]=hitContactPoints4[i];
				tireContactNormals[i]=hitContactNormals4[i];

				//We know that the vehicle is not in the air.
				isInAirs[i]=false;

				//Clamp the spring compression so that it is never greater than the max bounce.
				//Apply the susp limit constraint if the spring compression is greater than the max bounce.
				suspLimitErrors[i] = dx - susp.mMaxCompression;
				suspLimitActiveFlags[i] = (dx > susp.mMaxCompression);
				suspLimitCMOffsets[i] = bodySpaceWheelCentreOffset;
				suspLimitDirs[i] = 	bodySpaceSuspTravelDir;
				jounce=PxMin(dx,susp.mMaxCompression);

				//Store the jounce (having a local copy avoids lhs).
				jounces[i]=jounce;

				//Store the jounce in the graph.
#if PX_DEBUG_VEHICLE_ON
				updateGraphDataSuspJounce(startWheelIndex, i,jounce);
#endif

				//Compute the speed of the rigid body along the suspension travel dir at the 
				//bottom of the wheel.
				const PxVec3 wheelBottomPos=v+w*(restT - jounce);
				const PxVec3 r=wheelBottomPos-carChassisTrnsfm.p;
				PxVec3 wheelBottomVel=carChassisLinVel;
				wheelBottomVel+=carChassisAngVel.cross(r);

				PxRigidDynamic* dynamicHitActor=NULL;
				PxVec3 hitActorVelocity(0,0,0);
				if(hitContactActors4[i] && ((dynamicHitActor = hitContactActors4[i]->is<PxRigidDynamic>()) != NULL))
				{
					hitActorVelocity = PxRigidBodyExt::getVelocityAtPos(*dynamicHitActor,wheelBottomPos);
					wheelBottomVel -= hitActorVelocity;
				}

				//Compute the jounce speed. 
				const PxF32 jounceSpeed =  PX_MAX_F32 != prevJounces[i] ? (jounce - prevJounces[i])*recipTimeStep : 0.0f;
				//jounceSpeed=wheelBottomVel.dot(w);

				//Compute the spring force.
				const PxF32 wDotGrav=PxMax(0.0f, w.dot(gravity));				
				PxF32 springForce=susp.mSprungMass*wDotGrav;
				springForce=susp.mSprungMass*wDotGrav;					//gravity acting along spring direction
				springForce+=susp.mSpringStrength*jounce;				//linear spring
				springForce=PxMax(springForce,0.0f);
				springForce+=susp.mSpringDamperRate*jounceSpeed;		//damping

				//Store the spring force now (having a local copy avoids lhs).
				suspensionSpringForces[i]=springForce;

				//Store the spring force in the graph.
#if PX_DEBUG_VEHICLE_ON
				updateGraphDataSuspForce(startWheelIndex,i,springForce);
#endif

				//Chassis force in opposite direction to spring travel direction.
				springForce*=-1.0f;
				const PxVec3 springForceW=w*springForce;

				//Only consider component of force that is along the contact normal.
				const PxVec3 springForceJ=hitNorm*(hitNorm.dot(springForceW));

				//Torque from spring force.
				const PxVec3 suspForceCMOffset = carChassisTrnsfm.rotate(wheelsSimData.getSuspForceAppPointOffset(i));
				const PxVec3 springTorqueJ=suspForceCMOffset.cross(springForceJ);

				//Add the suspension force/torque to the chassis force/torque.
				chassisForce+=springForceJ;
				chassisTorque+=springTorqueJ;

				//Now compute the tire load.
				//Add on the tire mass gravity force.
				PxF32 tireLoad=springForce*w.dot(hitNorm);
				const PxF32 gravityDotHitNorm=PxMin(gravity.dot(hitNorm), 0.0f);
				tireLoad -= wheel.mMass*gravityDotHitNorm;

				//Apply the opposite force to the hit object.
				if(dynamicHitActor)
				{
					if(!(dynamicHitActor->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC))
					{
						hitActors[i]=dynamicHitActor;
						hitActorForces[i]=hitNorm*(-tireLoad)*timeFraction;
						hitActorForcePositions[i]=hitContactPoints4[i];
					}
				}

				//Normalize the tire load 
				//Now work out the normalized tire load.
				const PxF32 normalisedTireLoad=tireLoad*recipGravityMagnitude*recipTireRestLoads[i];
				//Filter the normalized tire load and compute the filtered tire load too.
				const PxF32 filteredNormalisedTireLoad=computeFilteredNormalisedTireLoad(tireLoadFilterData,normalisedTireLoad);
				const PxF32 filteredTireLoad=filteredNormalisedTireLoad*gravityMagnitude*tireRestLoads[i];

#if PX_DEBUG_VEHICLE_ON
				updateGraphDataTireLoad(startWheelIndex,i,filteredTireLoad);
				updateGraphDataNormTireLoad(startWheelIndex,i,filteredNormalisedTireLoad);
#endif

				//Compute the lateral and longitudinal tire axes in the ground plane.
				PxVec3 tireLongDir;
				PxVec3 tireLatDir;
				computeTireDirs(latDir,hitNorm,steerAngles[i],tireLongDir,tireLatDir);

				//Store the tire long and lat dirs now (having a local copy avoids lhs).
				tireLongitudinalDirs[i]= tireLongDir;
				tireLateralDirs[i]=tireLatDir;

				//Now compute the speeds along each of the tire axes.
				const PxF32 tireLongSpeed=wheelBottomVel.dot(tireLongDir);
				const PxF32 tireLatSpeed=wheelBottomVel.dot(tireLatDir);

				//Store the forward speed (having a local copy avoids lhs).
				forwardSpeeds[i]=tireLongSpeed;

				//Now compute the slips along each axes.
				const bool hasAccel=isAccelApplied[i];
				const bool hasBrake=isBrakeApplied[i];
				const PxF32 wheelOmega=wheelsDynData.mWheelSpeeds[i];
				const PxF32 wheelRadius=wheel.mRadius;
				PxF32 longSlip;
				PxF32 latSlip;
				computeTireSlips
					(tireLongSpeed,tireLatSpeed,wheelOmega,wheelRadius,minLongSlipDenominator,
					 hasAccel,hasBrake,
					 isTank,
					 longSlip,latSlip);

				//Store the lat and long slip (having local copies avoids lhs).
				longSlips[i]=longSlip;
				latSlips[i]=latSlip;

				//Camber angle.
				PxF32 camber=susp.mCamberAtRest;
				if(jounce>0)
				{
					camber += jounce*susp.mCamberAtMaxCompression*susp.getRecipMaxCompression();
				}
				else
				{
					camber -= jounce*susp.mCamberAtMaxDroop*susp.getRecipMaxDroop();
				}

				//Compute the friction that will be experienced by the tire.
				PxF32 friction;
				computeTireFriction(tire,longSlip,frictionMultiplier,friction);

				//Store the friction (having a local copy avoids lhs).
				frictions[i]=friction;

				if(filteredTireLoad*frictionMultiplier>0)
				{
					//Either tire forces or sticky tire friction constraint will be applied here.
					const PxVec3 tireForceCMOffset = carChassisTrnsfm.rotate(wheelsSimData.getTireForceAppPointOffset(i));

					PxF32 newLowForwardSpeedTimer;
					{
						//check the accel value here
						//Update low forward speed timer.
						const PxF32 recipWheelRadius=wheel.getRecipRadius();
						newLowForwardSpeedTimer=newLowForwardSpeedTimers[i];
						updateLowForwardSpeedTimer(tireLongSpeed,wheelOmega,wheelRadius,recipWheelRadius,isIntentionToAccelerate,timeStep,newLowForwardSpeedTimer);

						//Activate sticky tire forward friction constraint if required.
						//If sticky tire friction is active then set the longitudinal slip to zero because 
						//the sticky tire constraint will take care of the longitudinal component of motion.
						bool stickyTireForwardActiveFlag=false;
						PxF32 stickyTireForwardTargetSpeed=0.0f;
						activateStickyFrictionForwardConstraint(tireLongSpeed,wheelOmega,newLowForwardSpeedTimer,isIntentionToAccelerate,stickyTireForwardActiveFlag,stickyTireForwardTargetSpeed);
						stickyTireForwardTargetSpeed += hitActorVelocity.dot(tireLongDir);

						//Store the sticky tire data (having local copies avoids lhs). 
						newLowForwardSpeedTimers[i] = newLowForwardSpeedTimer;
						stickyTireForwardActiveFlags[i]=stickyTireForwardActiveFlag;
						stickyTireForwardTargetSpeeds[i]=stickyTireForwardTargetSpeed;
						stickyTireForwardDirs[i]=tireLongDir;
						stickyTireForwardCMOffsets[i]=tireForceCMOffset;

						//Deactivate the long slip if sticky tire constraint is active.
						longSlip=(!stickyTireForwardActiveFlag ? longSlip : 0.0f); 

						//Store the long slip (having local copies avoids lhs). 
						longSlips[i]=longSlip;
					}

					PxF32 newLowSideSpeedTimer;
					{
						//check the accel value here
						//Update low side speed timer.
						newLowSideSpeedTimer=newLowSideSpeedTimers[i];
						updateLowSideSpeedTimer(tireLatSpeed,isIntentionToAccelerate,timeStep,newLowSideSpeedTimer);

						//Activate sticky tire side friction constraint if required.
						//If sticky tire friction is active then set the lateral slip to zero because 
						//the sticky tire constraint will take care of the lateral component of motion.
						bool stickyTireSideActiveFlag=false;
						PxF32 stickyTireSideTargetSpeed=0.0f;
						activateStickyFrictionSideConstraint(tireLatSpeed,newLowForwardSpeedTimer,newLowSideSpeedTimer,isIntentionToAccelerate,stickyTireSideActiveFlag,stickyTireSideTargetSpeed);
						stickyTireSideTargetSpeed += hitActorVelocity.dot(tireLatDir);

						//Store the sticky tire data (having local copies avoids lhs). 
						newLowSideSpeedTimers[i] = newLowSideSpeedTimer;
						stickyTireSideActiveFlags[i]=stickyTireSideActiveFlag;
						stickyTireSideTargetSpeeds[i]=stickyTireSideTargetSpeed;
						stickyTireSideDirs[i]=tireLatDir;
						stickyTireSideCMOffsets[i]=tireForceCMOffset;

						//Deactivate the lat slip if sticky tire constraint is active.
						latSlip=(!stickyTireSideActiveFlag ? latSlip : 0.0f); 

						//Store the long slip (having local copies avoids lhs). 
						latSlips[i]=latSlip;
					}

					//Compute the various tire torques.
					PxF32 wheelTorque=0;
					PxF32 tireLongForceMag=0;
					PxF32 tireLatForceMag=0;
					PxF32 tireAlignMoment=0;
					const PxF32 restTireLoad=gravityMagnitude*tireRestLoads[i];
					const PxF32 recipWheelRadius=wheel.getRecipRadius();
					tireForceCalculator.mShader(
						tireForceCalculator.mShaderData[i],
						friction,
						longSlip,latSlip,camber,
						wheelOmega,wheelRadius,recipWheelRadius,
						restTireLoad,filteredNormalisedTireLoad,filteredTireLoad,
						gravityMagnitude, recipGravityMagnitude,
						wheelTorque,tireLongForceMag,tireLatForceMag,tireAlignMoment);

					//Store the tire torque ((having a local copy avoids lhs).
					tireTorques[i]=wheelTorque;

					//Apply the torque to the chassis.
					//Compute the tire force to apply to the chassis.
					const PxVec3 tireLongForce=tireLongDir*tireLongForceMag;
					const PxVec3 tireLatForce=tireLatDir*tireLatForceMag;
					const PxVec3 tireForce=tireLongForce+tireLatForce;
					//Compute the torque to apply to the chassis.
					const PxVec3 tireTorque=tireForceCMOffset.cross(tireForce);
					//Add all the forces/torques together.
					chassisForce+=tireForce;
					chassisTorque+=tireTorque;

					//Graph all the data we just computed.
#if PX_DEBUG_VEHICLE_ON
					if(gCarTireForceAppPoints)
						gCarTireForceAppPoints[i]=carChassisTrnsfm.p + tireForceCMOffset;
					if(gCarSuspForceAppPoints)
						gCarSuspForceAppPoints[i]=carChassisTrnsfm.p + suspForceCMOffset;

					if(gCarWheelGraphData[0])
					{
						updateGraphDataNormLongTireForce(startWheelIndex, i, PxAbs(tireLongForceMag)*normalisedTireLoad/tireLoad);
						updateGraphDataNormLatTireForce(startWheelIndex, i, PxAbs(tireLatForceMag)*normalisedTireLoad/tireLoad);
						updateGraphDataNormTireAligningMoment(startWheelIndex, i, tireAlignMoment*normalisedTireLoad/tireLoad);
						updateGraphDataLongTireSlip(startWheelIndex, i,longSlips[i]);
						updateGraphDataLatTireSlip(startWheelIndex, i,latSlips[i]);
						updateGraphDataTireFriction(startWheelIndex, i,frictions[i]);
					}
#endif
				}//filteredTireLoad*frictionMultiplier>0
			}//if(dx > -susp.mMaxCompression)
		}//if(numHits>0)
	}//i
}

////////////////////////////////////////////////////////////////////////////
//Set the low long speed timers computed in processSuspTireWheels
//Call immediately after completing processSuspTireWheels.
////////////////////////////////////////////////////////////////////////////

void updateLowSpeedTimers(const PxF32* PX_RESTRICT newLowSpeedTimers, PxF32* PX_RESTRICT lowSpeedTimers)
{
	for(PxU32 i=0;i<4;i++)
	{
		lowSpeedTimers[i]=(newLowSpeedTimers[i]!=lowSpeedTimers[i] ? newLowSpeedTimers[i] : 0.0f);
	}
}

////////////////////////////////////////////////////////////////////////////
//Set the jounce values computed in processSuspTireWheels
//Call immediately after completing processSuspTireWheels.
////////////////////////////////////////////////////////////////////////////
void updateJounces(const PxF32* PX_RESTRICT jounces, PxF32* PX_RESTRICT prevJounces)
{
	for(PxU32 i=0;i<4;i++)
	{
		prevJounces[i] = jounces[i];
	}
}

///////////////////////////////////////////////////////////////////////////////
//Set the hit plane, hit distance and hit friction multplier computed in processSuspTireWheels
//Call immediately after completing processSuspTireWheels.
////////////////////////////////////////////////////////////////////////////

void updateCachedHitData
(const PxU32* PX_RESTRICT cachedHitCounts, const PxVec4* PX_RESTRICT cachedHitPlanes, const PxF32* PX_RESTRICT cachedHitDistances, const PxF32* PX_RESTRICT cachedFrictionMultipliers, 
 PxVehicleWheels4DynData* wheels4DynData)
{
	if(wheels4DynData->mSqResults)
	{
		wheels4DynData->mHasCachedRaycastHitPlane = true;
	}

	PxVehicleWheels4DynData::CachedSuspLineRaycastHitResult* cachedRaycastHitResults = 
		(PxVehicleWheels4DynData::CachedSuspLineRaycastHitResult*)wheels4DynData->mRaycastsOrCachedHitResults;

	for(PxU32 i=0;i<4;i++)
	{
		cachedRaycastHitResults->mCounts[i]=Ps::to16(cachedHitCounts[i]);
		cachedRaycastHitResults->mPlanes[i]=cachedHitPlanes[i];
		cachedRaycastHitResults->mDistances[i]=cachedHitDistances[i];
		cachedRaycastHitResults->mFrictionMultipliers[i]=cachedFrictionMultipliers[i];
	}
}



////////////////////////////////////////////////////////////////////////////
//Solve the system of engine speed + wheel rotation speeds using an implicit integrator.
//The following functions only compute the speed of wheels connected to the diff.
//Worth going to the length of the implicit integrator because after gear changes
//the difference in speed at the clutch can be hard to integrate.
//Separate functions for 4W, NW and tank because the differential works in slightly 
//different ways.  With driveNW we end up with (N+1)*(N+1) problem, with drive4W we end up 
//with 5*5 and with tanks we end up with just 3*3. Tanks use the method of least squares
//to apply the rule that all left/right wheels have the same speed. 
//Remember that the following functions don't integrate wheels not connected to the diff
//so these need integrated separately.
////////////////////////////////////////////////////////////////////////////

#ifdef PX_CHECKED
bool isValid(const MatrixNN& A, const VectorN& b, const VectorN& result) 
{
	PX_ASSERT(A.getSize()==b.getSize());
	PX_ASSERT(A.getSize()==result.getSize());
	const PxU32 size=A.getSize();

	//r=A*result-b
	VectorN r(size);
	for(PxU32 i=0;i<size;i++)
	{
		r[i]=-b[i];
		for(PxU32 j=0;j<size;j++)
		{
			r[i]+=A.get(i,j)*result[j];
		}
	}

	PxF32 rLength=0;
	PxF32 bLength=0;
	for(PxU32 i=0;i<size;i++)
	{
		rLength+=r[i]*r[i];
		bLength+=b[i]*b[i];
	}
	const PxF32 error=PxSqrt(rLength/(bLength+1e-5f));
	return (error<1e-5f);
}
#endif


struct ImplicitSolverInput
{
	//dt/numSubSteps
	PxF32 subTimeStep;

	//Brake control value in range (0,1)
	PxF32 brake;

	//Handbrake control value in range (0,1)
	PxF32 handBrake;

	//Clutch strength
	PxF32 K;

	//Gear ratio.
	PxF32 G;

	PxVehicleClutchAccuracyMode::Enum accuracyMode;
	PxU32 maxNumIterations;

	//Engine drive torque 
	PxF32 engineDriveTorque;

	//Engine damping rate.
	PxF32 engineDampingRate;

	//Fraction of available clutch torque to be delivered to each wheel.
	const PxF32* diffTorqueRatios;

	//Fractional contribution of each wheel to average wheel speed at clutch.
	const PxF32* aveWheelSpeedContributions;

	//Braking torque at each wheel (inlcudes handbrake torque).
	const PxF32* brakeTorques;

	//True per wheel brakeTorques[i] > 0, false if brakeTorques[i]==0
	const bool* isBrakeApplied;

	//Tire torques to apply to each 1d rigid body wheel.
	const PxF32* tireTorques;

	//Sim and dyn data.
	PxU32 numWheels4;
	PxU32 numActiveWheels;
	const PxVehicleWheels4SimData* wheels4SimData;
	const PxVehicleDriveSimData* driveSimData;
};

struct ImplicitSolverOutput
{
	PxVehicleWheels4DynData* wheelsDynData;
	PxVehicleDriveDynData* driveDynData;
};

void solveDrive4WInternaDynamicsEnginePlusDrivenWheels
(const ImplicitSolverInput& input, ImplicitSolverOutput* output)
{
 	const PxF32 subTimestep = input.subTimeStep;
	const PxF32 K = input.K;
	const PxF32 G = input.G; 
	const PxVehicleClutchAccuracyMode::Enum accuracyMode = input.accuracyMode;
	const PxU32 maxIterations = input.maxNumIterations;
	const PxF32 engineDriveTorque = input.engineDriveTorque;
	const PxF32 engineDampingRate = input.engineDampingRate;
	const PxF32* PX_RESTRICT diffTorqueRatios = input.diffTorqueRatios;
	const PxF32* PX_RESTRICT aveWheelSpeedContributions = input.aveWheelSpeedContributions; 
	const PxF32* PX_RESTRICT brakeTorques = input.brakeTorques;
	const bool* PX_RESTRICT isBrakeApplied = input.isBrakeApplied;
	const PxF32* PX_RESTRICT tireTorques = input.tireTorques;
	const PxVehicleWheels4SimData& wheels4SimData = *input.wheels4SimData;
	const PxVehicleDriveSimData4W& driveSimData = *(PxVehicleDriveSimData4W*)input.driveSimData;

	PxVehicleDriveDynData* driveDynData = output->driveDynData;
	PxVehicleWheels4DynData* wheels4DynData = output->wheelsDynData;

	const PxF32 KG=K*G;
	const PxF32 KGG=K*G*G;

	MatrixNN A(4+1);
	VectorN b(4+1);
	VectorN result(4+1);

	const PxVehicleEngineData& engineData=driveSimData.getEngineData();

	const PxF32* PX_RESTRICT wheelSpeeds=wheels4DynData->mWheelSpeeds;
	const PxF32 engineOmega=driveDynData->getEngineRotationSpeed();

	//
	//torque at clutch:  
	//tc = K*{G*[alpha0*w0 + alpha1*w1 + alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}
	//where 
	//(i)   G is the gearing ratio, 
	//(ii)  alphai is the fractional contribution of the ith wheel to the average wheel speed at the clutch (alpha(i) is zero for undriven wheels)
	//(iii) wi is the angular speed of the ith wheel
	//(iv)  K is the clutch strength 
	//(v)   wEng is the angular speed of the engine

	//torque applied to ith wheel is 
	//ti = G*gammai*tc + bt(i) + tt(i) 
	//where
	//gammai is the fractional proportion of the clutch torque that the differential delivers to the ith wheel
	//bt(i) is the brake torque applied to the ith wheel
	//tt(i) is the tire torque applied to the ith wheel

	//acceleration applied to ith wheel is 
	//ai = G*gammai*K*{G*[alpha0*w0 + alpha1*w1 alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}/Ii + (bt(i) + tt(i))/Ii
	//wheer Ii is the moi of the ith wheel

	//express ai as 
	//ai = [wi(t+dt) - wi(t)]/dt
	//and rearrange
	//wi(t+dt) - wi(t)] = dt*G*gammai*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1)(t+dt)] - wEng(t+dt)}/Ii + dt*(bt(i) + tt(i))/Ii

	//Do the same for tEng (torque applied to engine)
	//tEng  = -tc + engineDriveTorque
	//where engineDriveTorque is the drive torque applied to the engine
	//Assuming the engine has unit mass then
	//wEng(t+dt) -wEng(t) = -dt*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1(t+dt))] - wEng(t+dt)}/Ieng + dt*engineDriveTorque]/IEng

	//Introduce the vector w=(w0,w1,w2....w(N-1), wEng)
	//and re-express as a matrix after collecting all unknowns at (t+dt) and knowns at time t.
	//A*w(t+dt)=b(t);

	//Wheels.
	{
		for(PxU32 i=0;i<4;i++)
		{
			const PxF32 dt=subTimestep*wheels4SimData.getWheelData(i).getRecipMOI();
			const PxF32 R=diffTorqueRatios[i];
			const PxF32 dtKGGR=dt*KGG*R;
			A.set(i,0,dtKGGR*aveWheelSpeedContributions[0]);
			A.set(i,1,dtKGGR*aveWheelSpeedContributions[1]);
			A.set(i,2,dtKGGR*aveWheelSpeedContributions[2]);
			A.set(i,3,dtKGGR*aveWheelSpeedContributions[3]);
			A.set(i,i,1.0f+dtKGGR*aveWheelSpeedContributions[i]+dt*wheels4SimData.getWheelData(i).mDampingRate);
			A.set(i,4,-dt*KG*R);
			b[i] = wheelSpeeds[i] + dt*(brakeTorques[i]+tireTorques[i]);
			result[i] = wheelSpeeds[i];
		}
	}

	//Engine.
	{
		const PxF32 dt=subTimestep*driveSimData.getEngineData().getRecipMOI();
		const PxF32 dtKG=dt*K*G;
		A.set(4,0,-dtKG*aveWheelSpeedContributions[0]);
		A.set(4,1,-dtKG*aveWheelSpeedContributions[1]);
		A.set(4,2,-dtKG*aveWheelSpeedContributions[2]);
		A.set(4,3,-dtKG*aveWheelSpeedContributions[3]);
		A.set(4,4,1.0f + dt*(K+engineDampingRate));
		b[4] = engineOmega + dt*engineDriveTorque;
		result[4] = engineOmega;
	}

	//Solve Aw=b
	if(PxVehicleClutchAccuracyMode::eBEST_POSSIBLE == accuracyMode)
	{
		MatrixNNLUSolver solver;
		solver.decomposeLU(A);
		solver.solve(b,result);
		PX_WARN_ONCE(!isValid(A,b,result), "Unable to compute new PxVehicleDrive4W internal rotation speeds.  Please check vehicle sim data, especially clutch strength; engine moi and damping; wheel moi and damping");
	}
	else
	{
		MatrixNGaussSeidelSolver solver;
		solver.solve(maxIterations, gSolverTolerance, A, b, result);
	}

	//Check for sanity in the resultant internal rotation speeds.
	//If the brakes are on and the wheels have switched direction then lock them at zero.
	//A consequence of this quick fix is that locked wheels remain locked until the brake is entirely released.
	//This isn't strictly mathematically or physically correct - a more accurate solution would either formulate the 
	//brake as a lcp problem or repeatedly solve with constraints that locked wheels remain at zero rotation speed.
	//The physically correct solution will certainly be more expensive so let's live with the restriction that 
	//locked wheels remain locked until the brake is released. 
	//newOmega=result[i], oldOmega=wheelSpeeds[i], if newOmega*oldOmega<=0 and isBrakeApplied then lock wheel.
	result[0]=(isBrakeApplied[0] && (wheelSpeeds[0]*result[0]<=0)) ? 0.0f : result[0];
	result[1]=(isBrakeApplied[1] && (wheelSpeeds[1]*result[1]<=0)) ? 0.0f : result[1];
	result[2]=(isBrakeApplied[2] && (wheelSpeeds[2]*result[2]<=0)) ? 0.0f : result[2];
	result[3]=(isBrakeApplied[3] && (wheelSpeeds[3]*result[3]<=0)) ? 0.0f : result[3];
	//Clamp the engine revs.
	//Again, this is not physically or mathematically correct but the loss in behaviour will be hard to notice.
	//The alternative would be to add constraints to the solver, which would be much more expensive.
	result[4]=PxClamp(result[4],0.0f,engineData.mMaxOmega);

	//Copy back to the car's internal rotation speeds.
	wheels4DynData->mWheelSpeeds[0]=result[0];
	wheels4DynData->mWheelSpeeds[1]=result[1];
	wheels4DynData->mWheelSpeeds[2]=result[2];
	wheels4DynData->mWheelSpeeds[3]=result[3];
	driveDynData->setEngineRotationSpeed(result[4]);
}

void solveDriveNWInternalDynamicsEnginePlusDrivenWheels
(const ImplicitSolverInput& input, ImplicitSolverOutput* output)
{  
	 const PxF32 subTimestep = input.subTimeStep;
	 //const PxF32 brake = input.brake;
	 //const PxF32 handbrake = input.handBrake;
	 const PxF32 K = input.K;
	 const PxF32 G = input.G;
	 const PxVehicleClutchAccuracyMode::Enum accuracyMode = input.accuracyMode;
	 const PxU32 maxIterations = input.maxNumIterations;
	 const PxF32 engineDriveTorque = input.engineDriveTorque;
	 const PxF32 engineDampingRate = input.engineDampingRate;
	 const PxF32* PX_RESTRICT diffTorqueRatios = input.diffTorqueRatios;
	 const PxF32* PX_RESTRICT aveWheelSpeedContributions = input.aveWheelSpeedContributions;
	 const PxF32* PX_RESTRICT brakeTorques = input.brakeTorques;
	 const bool* PX_RESTRICT isBrakeApplied = input.isBrakeApplied;
	 const PxF32* PX_RESTRICT tireTorques = input.tireTorques;
	 //const PxU32 numWheels4 = input.numWheels4;
	 const PxU32 numActiveWheels = input.numActiveWheels;
	 const PxVehicleWheels4SimData* PX_RESTRICT wheels4SimDatas = input.wheels4SimData;
	 const PxVehicleDriveSimDataNW& driveSimData = *(PxVehicleDriveSimDataNW*)input.driveSimData;

	 PxVehicleDriveDynData* driveDynData = output->driveDynData;
	 PxVehicleWheels4DynData* wheels4DynDatas = output->wheelsDynData;

	const PxF32 KG=K*G;
	const PxF32 KGG=K*G*G;

	MatrixNN A(numActiveWheels+1);
	VectorN b(numActiveWheels+1);
	VectorN result(numActiveWheels+1);

	const PxVehicleEngineData& engineData=driveSimData.getEngineData();
	const PxF32 engineOmega=driveDynData->getEngineRotationSpeed();

	//
	//torque at clutch:  
	//tc = K*{G*[alpha0*w0 + alpha1*w1 + alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}
	//where 
	//(i)   G is the gearing ratio, 
	//(ii)  alphai is the fractional contribution of the ith wheel to the average wheel speed at the clutch (alpha(i) is zero for undriven wheels)
	//(iii) wi is the angular speed of the ith wheel
	//(iv)  K is the clutch strength 
	//(v)   wEng is the angular speed of the engine

	//torque applied to ith wheel is 
	//ti = G*gammai*tc + bt(i) + tt(i) 
	//where
	//gammai is the fractional proportion of the clutch torque that the differential delivers to the ith wheel
	//bt(i) is the brake torque applied to the ith wheel
	//tt(i) is the tire torque applied to the ith wheel

	//acceleration applied to ith wheel is 
	//ai = G*gammai*K*{G*[alpha0*w0 + alpha1*w1 alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}/Ii + (bt(i) + tt(i))/Ii
	//wheer Ii is the moi of the ith wheel.

	//express ai as 
	//ai = [wi(t+dt) - wi(t)]/dt
	//and rearrange
	//wi(t+dt) - wi(t)] = dt*G*gammai*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1)(t+dt)] - wEng(t+dt)}/Ii + dt*(bt(i) + tt(i))/Ii

	//Do the same for tEng (torque applied to engine)
	//tEng  = -tc + engineDriveTorque
	//where engineDriveTorque is the drive torque applied to the engine
	//Assuming the engine has unit mass then
	//wEng(t+dt) -wEng(t) = -dt*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1(t+dt))] - wEng(t+dt)}/Ieng + dt*engineDriveTorque/Ieng

	//Introduce the vector w=(w0,w1,w2....w(N-1), wEng)
	//and re-express as a matrix after collecting all unknowns at (t+dt) and knowns at time t.
	//A*w(t+dt)=b(t);

	//Wheels.
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		const PxF32 dt=subTimestep*wheels4SimDatas[i>>2].getWheelData(i&3).getRecipMOI();
		const PxF32 R=diffTorqueRatios[i];
		const PxF32 dtKGGR=dt*KGG*R;

		for(PxU32 j=0;j<numActiveWheels;j++)
		{
			A.set(i,j,dtKGGR*aveWheelSpeedContributions[j]);
		}

		A.set(i,i,1.0f+dtKGGR*aveWheelSpeedContributions[i]+dt*wheels4SimDatas[i>>2].getWheelData(i&3).mDampingRate);
		A.set(i,numActiveWheels,-dt*KG*R);
		b[i] = wheels4DynDatas[i>>2].mWheelSpeeds[i&3] + dt*(brakeTorques[i]+tireTorques[i]);
		result[i] = wheels4DynDatas[i>>2].mWheelSpeeds[i&3];
	}

	//Engine.
	{
		const PxF32 dt=subTimestep*driveSimData.getEngineData().getRecipMOI();
		const PxF32 dtKG=dt*K*G;
		for(PxU32 i=0;i<numActiveWheels;i++)
		{
			A.set(numActiveWheels,i,-dtKG*aveWheelSpeedContributions[i]);
		}
		A.set(numActiveWheels,numActiveWheels,1.0f + dt*(K+engineDampingRate));
		b[numActiveWheels] = engineOmega + dt*engineDriveTorque;
		result[numActiveWheels] = engineOmega;
	}

	//Solve Aw=b
	if(PxVehicleClutchAccuracyMode::eBEST_POSSIBLE == accuracyMode)
	{
		MatrixNNLUSolver solver;
		solver.decomposeLU(A);
		solver.solve(b,result);
		PX_WARN_ONCE(!isValid(A,b,result), "Unable to compute new PxVehicleDriveNW internal rotation speeds.  Please check vehicle sim data, especially clutch strength; engine moi and damping; wheel moi and damping");
	}
	else
	{
		MatrixNGaussSeidelSolver solver;
		solver.solve(maxIterations, gSolverTolerance, A, b, result);
	}

	//Check for sanity in the resultant internal rotation speeds.
	//If the brakes are on and the wheels have switched direction then lock them at zero.
	//A consequence of this quick fix is that locked wheels remain locked until the brake is entirely released.
	//This isn't strictly mathematically or physically correct - a more accurate solution would either formulate the 
	//brake as a lcp problem or repeatedly solve with constraints that locked wheels remain at zero rotation speed.
	//The physically correct solution will certainly be more expensive so let's live with the restriction that 
	//locked wheels remain locked until the brake is released. 
	//newOmega=result[i], oldOmega=wheelSpeeds[i], if newOmega*oldOmega<=0 and isBrakeApplied then lock wheel.
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		result[i]=(isBrakeApplied[i] && (wheels4DynDatas[i>>2].mWheelSpeeds[i&3]*result[i]<=0)) ? 0.0f : result[i];
	}
	//Clamp the engine revs.
	//Again, this is not physically or mathematically correct but the loss in behaviour will be hard to notice.
	result[numActiveWheels]=PxClamp(result[numActiveWheels],0.0f,engineData.mMaxOmega);

	//Copy back to the car's internal rotation speeds.
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		wheels4DynDatas[i>>2].mWheelSpeeds[i&3]=result[i];
	}
	driveDynData->setEngineRotationSpeed(result[numActiveWheels]);
}


void solveTankInternaDynamicsEnginePlusDrivenWheels
(const ImplicitSolverInput& input, const bool* PX_RESTRICT activeWheelStates, const PxF32* PX_RESTRICT wheelGearings, ImplicitSolverOutput* output)
{
	PX_SIMD_GUARD; // denormal exception triggered at oldOmega*newOmega on osx
	const PxF32 subTimestep = input.subTimeStep;
	const PxF32 K = input.K;
	const PxF32 G = input.G; 
	const PxF32 engineDriveTorque = input.engineDriveTorque;
	const PxF32 engineDampingRate = input.engineDampingRate;
	const PxF32* PX_RESTRICT diffTorqueRatios = input.diffTorqueRatios;
	const PxF32* PX_RESTRICT aveWheelSpeedContributions = input.aveWheelSpeedContributions; 
	const PxF32* PX_RESTRICT brakeTorques = input.brakeTorques;
	const bool* PX_RESTRICT isBrakeApplied = input.isBrakeApplied;
	const PxF32* PX_RESTRICT tireTorques = input.tireTorques;
	const PxU32 numWheels4 = input.numWheels4;
	const PxU32 numActiveWheels = input.numActiveWheels;
	const PxVehicleWheels4SimData* PX_RESTRICT wheels4SimDatas = input.wheels4SimData;
	const PxVehicleDriveSimData& driveSimData = *input.driveSimData;

	PxVehicleWheels4DynData* PX_RESTRICT wheels4DynDatas = output->wheelsDynData; 
	PxVehicleDriveDynData* driveDynData = output->driveDynData;

	const PxF32 KG=K*G;
	const PxF32 KGG=K*G*G;

	//Rearrange data in a single array rather than scattered in blocks of 4.
	//This makes it easier later on.
	PxF32 recipMOI[PX_MAX_NB_WHEELS];
	PxF32 dampingRates[PX_MAX_NB_WHEELS];
	PxF32 wheelSpeeds[PX_MAX_NB_WHEELS];
	PxF32 wheelRecipRadii[PX_MAX_NB_WHEELS];

	for(PxU32 i=0;i<numWheels4-1;i++)
	{
		const PxVehicleWheelData& wheelData0=wheels4SimDatas[i].getWheelData(0);
		const PxVehicleWheelData& wheelData1=wheels4SimDatas[i].getWheelData(1);
		const PxVehicleWheelData& wheelData2=wheels4SimDatas[i].getWheelData(2);
		const PxVehicleWheelData& wheelData3=wheels4SimDatas[i].getWheelData(3);

		recipMOI[4*i+0]=wheelData0.getRecipMOI();
		recipMOI[4*i+1]=wheelData1.getRecipMOI();
		recipMOI[4*i+2]=wheelData2.getRecipMOI();
		recipMOI[4*i+3]=wheelData3.getRecipMOI();

		dampingRates[4*i+0]=wheelData0.mDampingRate;
		dampingRates[4*i+1]=wheelData1.mDampingRate;
		dampingRates[4*i+2]=wheelData2.mDampingRate;
		dampingRates[4*i+3]=wheelData3.mDampingRate;

		wheelRecipRadii[4*i+0]=wheelData0.getRecipRadius();
		wheelRecipRadii[4*i+1]=wheelData1.getRecipRadius();
		wheelRecipRadii[4*i+2]=wheelData2.getRecipRadius();
		wheelRecipRadii[4*i+3]=wheelData3.getRecipRadius();

		const PxVehicleWheels4DynData& suspWheelTire4=wheels4DynDatas[i];
		wheelSpeeds[4*i+0]=suspWheelTire4.mWheelSpeeds[0];
		wheelSpeeds[4*i+1]=suspWheelTire4.mWheelSpeeds[1];
		wheelSpeeds[4*i+2]=suspWheelTire4.mWheelSpeeds[2];
		wheelSpeeds[4*i+3]=suspWheelTire4.mWheelSpeeds[3];
	}
	const PxU32 numInLastBlock = 4 - (4*numWheels4 - numActiveWheels);
	for(PxU32 i=0;i<numInLastBlock;i++)
	{
		const PxVehicleWheelData& wheelData=wheels4SimDatas[numWheels4-1].getWheelData(i);
		recipMOI[4*(numWheels4-1)+i]=wheelData.getRecipMOI();
		dampingRates[4*(numWheels4-1)+i]=wheelData.mDampingRate;
		wheelRecipRadii[4*(numWheels4-1)+i]=wheelData.getRecipRadius();

		const PxVehicleWheels4DynData& suspWheelTire4=wheels4DynDatas[numWheels4-1];
		wheelSpeeds[4*(numWheels4-1)+i]=suspWheelTire4.mWheelSpeeds[i];
	}
	const PxF32 wheelRadius0=wheels4SimDatas[0].getWheelData(0).mRadius;
	const PxF32 wheelRadius1=wheels4SimDatas[0].getWheelData(1).mRadius;

	//
	//torque at clutch:  
	//tc = K*{G*[alpha0*w0 + alpha1*w1 + alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}
	//where 
	//(i)   G is the gearing ratio, 
	//(ii)  alphai is the fractional contribution of the ith wheel to the average wheel speed at the clutch (alpha(i) is zero for undriven wheels)
	//(iii) wi is the angular speed of the ith wheel
	//(iv)  K is the clutch strength 
	//(v)   wEng is the angular speed of the engine

	//torque applied to ith wheel is 
	//ti = G*gammai*tc + bt(i) + tt(i) 
	//where
	//gammai is the fractional proportion of the clutch torque that the differential delivers to the ith wheel
	//bt(i) is the brake torque applied to the ith wheel
	//tt(i) is the tire torque applied to the ith wheel

	//acceleration applied to ith wheel is 
	//ai = G*gammai*K*{G*[alpha0*w0 + alpha1*w1 alpha2*w2 + ..... alpha(N-1)*w(N-1)] - wEng}/Ii + (bt(i) + tt(i))/Ii
	//wheer Ii is the moi of the ith wheel.

	//express ai as 
	//ai = [wi(t+dt) - wi(t)]/dt
	//and rearrange
	//wi(t+dt) - wi(t)] = dt*G*gammai*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1)(t+dt)] - wEng(t+dt)}/Ii + dt*(bt(i) + tt(i))/Ii

	//Do the same for tEng (torque applied to engine)
	//tEng  = -tc + engineDriveTorque
	//where engineDriveTorque is the drive torque applied to the engine
	//Assuming the engine has unit mass then
	//wEng(t+dt) -wEng(t) = -dt*K*{G*[alpha0*w0(t+dt) + alpha1*w1(t+dt) + alpha2*w2(t+dt) + ..... alpha(N-1)*w(N-1(t+dt))] - wEng(t+dt)}/Ieng + dt*engineDriveTorque/Ieng

	//Introduce the vector w=(w0,w1,w2....w(N-1), wEng)
	//and re-express as a matrix after collecting all unknowns at (t+dt) and knowns at time t.
	//M*w(t+dt)=b(t);

	//Matrix M and rhs vector b that we use to solve Mw=b.
	MatrixNN M(numActiveWheels+1);
	VectorN b(numActiveWheels+1);

	//Wheels.
	{
		for(PxU32 i=0;i<numActiveWheels;i++)
		{
			const PxF32 dt=subTimestep*recipMOI[i];
			const PxF32 R=diffTorqueRatios[i];
			const PxF32 g=wheelGearings[i];
			const PxF32 dtKGGRg=dt*KGG*R*g;
			for(PxU32 j=0;j<numActiveWheels;j++)
			{
				M.set(i,j,dtKGGRg*aveWheelSpeedContributions[j]*wheelGearings[j]);
			}
			M.set(i,i,1.0f+dtKGGRg*aveWheelSpeedContributions[i]*wheelGearings[i]+dt*dampingRates[i]);
			M.set(i,numActiveWheels,-dt*KG*R*g);
			b[i] = wheelSpeeds[i] + dt*(brakeTorques[i]+tireTorques[i]);
		}
	}

	//Engine.
	{
		const PxF32 engineOmega=driveDynData->getEngineRotationSpeed();

		const PxF32 dt=subTimestep*driveSimData.getEngineData().getRecipMOI();
		const PxF32 dtKG=dt*K*G;
		for(PxU32 i=0;i<numActiveWheels;i++)
		{
			M.set(numActiveWheels,i,-dtKG*aveWheelSpeedContributions[i]*wheelGearings[i]);
		}
		M.set(numActiveWheels,numActiveWheels,1.0f + dt*(K+engineDampingRate));
		b[numActiveWheels] = engineOmega + dt*engineDriveTorque;
	}

	//Now apply the constraints that all the odd numbers are equal and all the even numbers are equal.
	//ie w2,w4,w6 are all equal to w0 and w3,w5,w7 are all equal to w1.
	//That leaves (4*N+1) equations but only 3 unknowns: two wheels speeds and the engine speed.
	//Substitute these extra constraints into the matrix.
	MatrixNN A(numActiveWheels+1);
	for(PxU32 i=0;i<numActiveWheels+1;i++)
	{
		PxF32 sum0=M.get(i,0+0);
		PxF32 sum1=M.get(i,0+1);
		for(PxU32 j=2;j<numActiveWheels;j+=2)
		{
			sum0+=M.get(i,j+0)*wheelRadius0*wheelRecipRadii[j+0];
			sum1+=M.get(i,j+1)*wheelRadius1*wheelRecipRadii[j+1];
		}
		A.set(i,0,sum0);
		A.set(i,1,sum1);
		A.set(i,2,M.get(i,numActiveWheels));
	}
	
	//We have an over-determined problem because of the extra constraints 
	//on equal wheel speeds. Solve using the least squares method as in
	//http://s-mat-pcs.oulu.fi/~mpa/matreng/ematr5_5.htm

	//Compute A^T*A
	//No longer using M.
	MatrixNN& ATA = M;
	ATA.setSize(3);
	for(PxU32 i=0;i<3;i++)
	{
		for(PxU32 j=0;j<3;j++)
		{
			PxF32 sum=0.0f;
			for(PxU32 k=0;k<numActiveWheels+1;k++)
			{
				//sum+=AT.get(i,k)*A.get(k,j);
				sum+=A.get(k,i)*A.get(k,j);
			}
			ATA.set(i,j,sum);
		}
	}

	//Compute A^T*b;
	VectorN ATb(3);
	for(PxU32 i=0;i<3;i++)
	{
		PxF32 sum=0;
		for(PxU32 j=0;j<numActiveWheels+1;j++)
		{
			//sum+=AT.get(i,j)*b[j];
			sum+=A.get(j,i)*b[j];
		}
		ATb[i]=sum;
	}

	//Solve (A^T*A)*x = A^T*b
	VectorN result(3);
	Matrix33Solver solver;
	bool successfulSolver = solver.solve(ATA, ATb, result);
	if(!successfulSolver)
	{
		PX_WARN_ONCE(true, "Unable to compute new PxVehicleDriveTank internal rotation speeds.  Please check vehicle sim data, especially clutch strength; engine moi and damping; wheel moi and damping");
		return;
	}

	//Clamp the engine revs between zero and maxOmega
	const PxF32 maxEngineOmega=driveSimData.getEngineData().mMaxOmega;
	const PxF32 newEngineOmega=PxClamp(result[2],0.0f,maxEngineOmega);

	//Apply the constraints on each of the equal wheel speeds.
	PxF32 wheelSpeedResults[PX_MAX_NB_WHEELS];
	wheelSpeedResults[0]=result[0];
	wheelSpeedResults[1]=result[1];
	for(PxU32 i=2;i<numActiveWheels;i+=2)
	{
		wheelSpeedResults[i+0]=result[0];
		wheelSpeedResults[i+1]=result[1];
	}
	
	//Check for sanity in the resultant internal rotation speeds.
	//If the brakes are on and the wheels have switched direction then lock them at zero.
	//A consequence of this quick fix is that locked wheels remain locked until the brake is entirely released.
	//This isn't strictly mathematically or physically correct - a more accurate solution would either formulate the 
	//brake as a lcp problem or repeatedly solve with constraints that locked wheels remain at zero rotation speed.
	//The physically correct solution will certainly be more expensive so let's live with the restriction that 
	//locked wheels remain locked until the brake is released. 
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		const PxF32 oldOmega=wheelSpeeds[i];
		const PxF32 newOmega=wheelSpeedResults[i];
		const bool hasBrake=isBrakeApplied[i];
		if(hasBrake && (oldOmega*newOmega <= 0))
		{
			wheelSpeedResults[i]=0.0f;
		}
	}


	//Copy back to the car's internal rotation speeds.
	for(PxU32 i=0;i<numWheels4-1;i++)
	{
		wheels4DynDatas[i].mWheelSpeeds[0] = activeWheelStates[4*i+0] ? wheelSpeedResults[4*i+0] : 0.0f;
		wheels4DynDatas[i].mWheelSpeeds[1] = activeWheelStates[4*i+1] ? wheelSpeedResults[4*i+1] : 0.0f;
		wheels4DynDatas[i].mWheelSpeeds[2] = activeWheelStates[4*i+2] ? wheelSpeedResults[4*i+2] : 0.0f;
		wheels4DynDatas[i].mWheelSpeeds[3] = activeWheelStates[4*i+3] ? wheelSpeedResults[4*i+3] : 0.0f;
	}
	for(PxU32 i=0;i<numInLastBlock;i++)
	{
		wheels4DynDatas[numWheels4-1].mWheelSpeeds[i] = activeWheelStates[4*(numWheels4-1)+i] ?  wheelSpeedResults[4*(numWheels4-1)+i] : 0.0f;
	}
	driveDynData->setEngineRotationSpeed(newEngineOmega);
}

////////////////////////////////////////////////////////////////////////////
//Integrate wheel rotation speeds of wheels not connected to the differential.
//Obviously, no wheels in a PxVehicleNoDrive are connected to a diff so all require 
//direct integration.
//Only the first 4 wheels of a PxVehicleDrive4W are connected to the diff so 
//any extra wheels need direct integration.
//All tank wheels are connected to the diff so none need integrated in a separate pass.
//What about undriven wheels in a PxVehicleDriveNW?  This vehicle type treats all
//wheels as being connected to the diff but sets the diff contribution to zero for 
//all undriven wheels.  No wheels from a PxVehicleNW need integrated in a separate pass.
////////////////////////////////////////////////////////////////////////////

void integrateNoDriveWheelSpeeds
(const PxF32 subTimestep, 
 const PxF32* PX_RESTRICT brakeTorques, const bool* PX_RESTRICT isBrakeApplied, const PxF32* driveTorques, const PxF32* PX_RESTRICT tireTorques, const PxF32* PX_RESTRICT dampingRates,
 const PxVehicleWheels4SimData& vehSuspWheelTire4SimData, PxVehicleWheels4DynData& vehSuspWheelTire4)
{
	//w(t+dt) = w(t) + (1/inertia)*(brakeTorque + driveTorque + tireTorque)*dt - (1/inertia)*damping*w(t)*dt )	(1)
	//Apply implicit trick and rearrange.
	//w(t+dt)[1 + (1/inertia)*damping*dt] = w(t) + (1/inertia)*(brakeTorque + driveTorque + tireTorque)*dt		(2)

	//Introduce (1/inertia)*dt to avoid duplication in (2)
	PxF32 subTimeSteps[4] = 
	{
		subTimestep*vehSuspWheelTire4SimData.getWheelData(0).getRecipMOI(),
		subTimestep*vehSuspWheelTire4SimData.getWheelData(1).getRecipMOI(),
		subTimestep*vehSuspWheelTire4SimData.getWheelData(2).getRecipMOI(),
		subTimestep*vehSuspWheelTire4SimData.getWheelData(3).getRecipMOI()
	};

	//Integrate.
	//w += torque*dt/inertia - damping*dt*w
	//Use implicit integrate trick and rearrange
	//w(t+dt)  = [w(t) + torque*dt/inertia]/[1 + damping*dt]
	const PxF32* PX_RESTRICT wheelSpeeds=vehSuspWheelTire4.mWheelSpeeds;
	PxF32 result[4]=
	{
		(wheelSpeeds[0] + subTimeSteps[0]*(tireTorques[0] + driveTorques[0] + brakeTorques[0]))/(1.0f + dampingRates[0]*subTimeSteps[0]),
		(wheelSpeeds[1] + subTimeSteps[1]*(tireTorques[1] + driveTorques[1] + brakeTorques[1]))/(1.0f + dampingRates[1]*subTimeSteps[1]),
		(wheelSpeeds[2] + subTimeSteps[2]*(tireTorques[2] + driveTorques[2] + brakeTorques[2]))/(1.0f + dampingRates[2]*subTimeSteps[2]),
		(wheelSpeeds[3] + subTimeSteps[3]*(tireTorques[3] + driveTorques[3] + brakeTorques[3]))/(1.0f + dampingRates[3]*subTimeSteps[3]),
	};

	//Check for sanity in the resultant internal rotation speeds.
	//If the brakes are on and the wheels have switched direction then lock them at zero.
	//newOmega=result[i], oldOmega=wheelSpeeds[i], if newOmega*oldOmega<=0 and isBrakeApplied then lock wheel.
	result[0]=(isBrakeApplied[0] && (wheelSpeeds[0]*result[0]<=0)) ? 0.0f : result[0];
	result[1]=(isBrakeApplied[1] && (wheelSpeeds[1]*result[1]<=0)) ? 0.0f : result[1];
	result[2]=(isBrakeApplied[2] && (wheelSpeeds[2]*result[2]<=0)) ? 0.0f : result[2];
	result[3]=(isBrakeApplied[3] && (wheelSpeeds[3]*result[3]<=0)) ? 0.0f : result[3];

	//Copy back to the car's internal rotation speeds.
	vehSuspWheelTire4.mWheelSpeeds[0]=result[0];
	vehSuspWheelTire4.mWheelSpeeds[1]=result[1];
	vehSuspWheelTire4.mWheelSpeeds[2]=result[2];
	vehSuspWheelTire4.mWheelSpeeds[3]=result[3];
}

void integrateUndriveWheelRotationSpeeds
(const PxF32 subTimestep, 
 const PxF32 brake, const PxF32 handbrake, const PxF32* PX_RESTRICT tireTorques, const PxF32* PX_RESTRICT brakeTorques, 
 const PxVehicleWheels4SimData& vehSuspWheelTire4SimData, PxVehicleWheels4DynData& vehSuspWheelTire4)
{
	for(PxU32 i=0;i<4;i++)
	{
		//Compute the new angular speed of the wheel.
		const PxF32 oldOmega=vehSuspWheelTire4.mWheelSpeeds[i];
		const PxF32 dtI = subTimestep*vehSuspWheelTire4SimData.getWheelData(i).getRecipMOI();
		const PxF32 gamma = vehSuspWheelTire4SimData.getWheelData(i).mDampingRate;
		const PxF32 newOmega=(oldOmega+dtI*(tireTorques[i]+brakeTorques[i]))/(1.0f + gamma*dtI);

		//Has the brake been applied?  It's hard to tell from brakeTorques[j] because that 
		//will be zero if the wheel is locked. Work it out from the brake and handbrake data.
		const PxF32 brakeGain=vehSuspWheelTire4SimData.getWheelData(i).mMaxBrakeTorque;
		const PxF32 handbrakeGain=vehSuspWheelTire4SimData.getWheelData(i).mMaxHandBrakeTorque;

		//Work out if the wheel should be locked.
		const bool brakeApplied=((brake*brakeGain + handbrake*handbrakeGain)!=0.0f);
		const bool wheelReversed=(oldOmega*newOmega <=0);
		const bool wheelLocked=(brakeApplied && wheelReversed);

		//Lock the wheel or apply its new angular speed.
		if(!wheelLocked)
		{
			vehSuspWheelTire4.mWheelSpeeds[i]=newOmega;
		}
		else
		{
			vehSuspWheelTire4.mWheelSpeeds[i]=0.0f;
		}
	}
}


////////////////////////////////////////////////////////////////////////////
//Pose the wheels.
//First integrate the wheel rotation angles and clamp them to a range (-10*pi, 10*pi)
//PxVehicleNoDrive has a different way of telling if a wheel is driven by a drive torque so has a separate function.
//Use the wheel steer/rotation/camber angle and suspension jounce to compute the local transform of each wheel.
////////////////////////////////////////////////////////////////////////////

void integrateWheelRotationAngles
(const PxF32 timestep,
 const PxF32 K, const PxF32 G, const PxF32 engineDriveTorque,
 const PxF32* PX_RESTRICT jounces, const PxF32* PX_RESTRICT diffTorqueRatios, const PxF32* PX_RESTRICT forwardSpeeds, const bool* isBrakeApplied,
 const PxVehicleDriveSimData& vehCoreSimData, const PxVehicleWheels4SimData& vehSuspWheelTire4SimData,
 PxVehicleDriveDynData& vehCore, PxVehicleWheels4DynData& vehSuspWheelTire4)
{
	PX_SIMD_GUARD; //denorm exception on newRotAngle=wheelRotationAngles[j]+wheelOmega*timestep; on osx
	
	PX_UNUSED(vehCore);
	PX_UNUSED(vehCoreSimData);

	const PxF32 KG=K*G;

	PxF32* PX_RESTRICT wheelSpeeds=vehSuspWheelTire4.mWheelSpeeds;
	PxF32* PX_RESTRICT wheelRotationAngles=vehSuspWheelTire4.mWheelRotationAngles;
	PxF32* PX_RESTRICT correctedWheelSpeeds = vehSuspWheelTire4.mCorrectedWheelSpeeds;

	for(PxU32 j=0;j<4;j++)
	{
		//At low vehicle forward speeds we have some numerical difficulties getting the 
		//wheel rotation speeds to be correct due to the tire model's difficulties at low vz.
		//The solution is to blend between the rolling speed at the wheel and the wheel's actual rotation speed.
		//If the wheel is 
		//(i)   in the air or, 
		//(ii)  under braking torque or, 
		//(iii) driven by the engine through the gears and diff
		//then always use the wheel's actual rotation speed.
		//Just to be clear, this means we will blend when the wheel
		//(i)   is on the ground and 
		//(ii)  has no brake applied and
		//(iii) has no drive torque applied from the clutch and
		//(iv)  is at low forward speed
		PxF32 wheelOmega=wheelSpeeds[j];
		if(jounces[j] > -vehSuspWheelTire4SimData.getSuspensionData(j).mMaxDroop &&	//(i)   wheel touching ground
			false==isBrakeApplied[j] &&												//(ii)  no brake applied
			0.0f==diffTorqueRatios[j]*KG*engineDriveTorque &&						//(iii) no drive torque applied
			PxAbs(forwardSpeeds[j])<gThresholdForwardSpeedForWheelAngleIntegration)	//(iv)  low speed
		{
			const PxF32 recipWheelRadius=vehSuspWheelTire4SimData.getWheelData(j).getRecipRadius();
			const PxF32 alpha=PxAbs(forwardSpeeds[j])*gRecipThresholdForwardSpeedForWheelAngleIntegration;
			wheelOmega = (forwardSpeeds[j]*recipWheelRadius)*(1.0f-alpha) + wheelOmega*alpha;
		}

		PxF32 newRotAngle=wheelRotationAngles[j]+wheelOmega*timestep;
		//Clamp the wheel rotation angle to a range (-10*pi,10*pi) to stop it getting crazily big.
		newRotAngle=physx::intrinsics::fsel(newRotAngle-10*PxPi, newRotAngle-10*PxPi, physx::intrinsics::fsel(-newRotAngle-10*PxPi, newRotAngle + 10*PxPi, newRotAngle));
		wheelRotationAngles[j]=newRotAngle;
		correctedWheelSpeeds[j]=wheelOmega;
	}
}

void integrateNoDriveWheelRotationAngles
(const PxF32 timestep,
 const PxF32* PX_RESTRICT driveTorques,
 const PxF32* PX_RESTRICT jounces, const PxF32* PX_RESTRICT forwardSpeeds, const bool* isBrakeApplied,
 const PxVehicleWheels4SimData& vehSuspWheelTire4SimData,
 PxVehicleWheels4DynData& vehSuspWheelTire4)
{
	PxF32* PX_RESTRICT wheelSpeeds=vehSuspWheelTire4.mWheelSpeeds;
	PxF32* PX_RESTRICT wheelRotationAngles=vehSuspWheelTire4.mWheelRotationAngles;
	PxF32* PX_RESTRICT correctedWheelSpeeds=vehSuspWheelTire4.mCorrectedWheelSpeeds;

	for(PxU32 j=0;j<4;j++)
	{
		//At low vehicle forward speeds we have some numerical difficulties getting the 
		//wheel rotation speeds to be correct due to the tire model's difficulties at low vz.
		//The solution is to blend between the rolling speed at the wheel and the wheel's actual rotation speed.
		//If the wheel is 
		//(i)   in the air or, 
		//(ii)  under braking torque or, 
		//(iii) driven by a drive torque
		//then always use the wheel's actual rotation speed.
		//Just to be clear, this means we will blend when the wheel
		//(i)   is on the ground and
		//(ii)  has no brake applied and
		//(iii) has no drive torque and
		//(iv)  is at low forward speed
		PxF32 wheelOmega=wheelSpeeds[j];
		if(jounces[j] > -vehSuspWheelTire4SimData.getSuspensionData(j).mMaxDroop &&	//(i)   wheel touching ground
			false==isBrakeApplied[j] &&												//(ii)  no brake applied
			0.0f==driveTorques[j] &&												//(iii) no drive torque applied
			PxAbs(forwardSpeeds[j])<gThresholdForwardSpeedForWheelAngleIntegration)	//(iv)  low speed
		{
			const PxF32 recipWheelRadius=vehSuspWheelTire4SimData.getWheelData(j).getRecipRadius();
			const PxF32 alpha=PxAbs(forwardSpeeds[j])*gRecipThresholdForwardSpeedForWheelAngleIntegration;
			wheelOmega = (forwardSpeeds[j]*recipWheelRadius)*(1.0f-alpha) + wheelOmega*alpha;

			//TODO: maybe just set the car wheel omega to the blended value?
			//Not sure about this bit.  
			//Turned this off because it added energy to the car at very small timesteps.
			//wheelSpeeds[j]=wheelOmega;
		}

		PxF32 newRotAngle=wheelRotationAngles[j]+wheelOmega*timestep;
		//Clamp the wheel rotation angle to a range (-10*pi,10*pi) to stop it getting crazily big.
		newRotAngle=physx::intrinsics::fsel(newRotAngle-10*PxPi, newRotAngle-10*PxPi, physx::intrinsics::fsel(-newRotAngle-10*PxPi, newRotAngle + 10*PxPi, newRotAngle));
		wheelRotationAngles[j]=newRotAngle;
		correctedWheelSpeeds[j]=wheelOmega;
	}
}

void computeWheelLocalPoses
(const PxVehicleWheels4SimData& wheelsSimData,
 const PxVehicleWheels4DynData& wheelsDynData, 
 const PxWheelQueryResult* wheelQueryResults,
 const PxU32 numWheelsToPose,
 const PxTransform& vehChassisCMLocalPose,
 PxTransform* localPoses)
{
	const PxF32* PX_RESTRICT rotAngles=wheelsDynData.mWheelRotationAngles;

	const PxVec3 cmOffset=vehChassisCMLocalPose.p;

	const PxVec3 forward = gRight.cross(gUp);

	for(PxU32 i=0;i<numWheelsToPose;i++)
	{
		const PxF32 jounce=wheelQueryResults[i].suspJounce;

		//Compute the camber angle.
		const PxVehicleSuspensionData& suspData=wheelsSimData.getSuspensionData(i);
		PxF32 camberAngle=suspData.mCamberAtRest;
		if(jounce > 0.0f)
		{
			camberAngle += jounce*suspData.mCamberAtMaxCompression*suspData.getRecipMaxCompression();
		}
		else
		{
			camberAngle -= jounce*suspData.mCamberAtMaxDroop*suspData.getRecipMaxDroop();
		}

		//Compute the transform of the wheel shapes. 
		const PxVec3 pos=cmOffset+wheelsSimData.getWheelCentreOffset(i)-wheelsSimData.getSuspTravelDirection(i)*jounce;
		const PxQuat quat(wheelQueryResults[i].steerAngle, gUp);
		const PxQuat quat2(camberAngle, quat.rotate(forward));
		const PxQuat quat3=quat2*quat;
		const PxQuat quat4(rotAngles[i],quat3.rotate(gRight));
		const PxTransform t(pos,quat4*quat3);
		localPoses[i] = t;
	}
}

void poseWheels
(const PxVehicleWheels4SimData& wheelsSimData,
 const PxTransform* localPoses,
 const PxU32 numWheelsToPose,
 PxRigidDynamic* vehActor)
{
	PxShape* shapeBuffer[128];
	vehActor->getShapes(shapeBuffer,128,0);

	for(PxU32 i=0;i<numWheelsToPose;i++)
	{
		const PxI32 shapeIndex = wheelsSimData.getWheelShapeMapping(i);
		if(shapeIndex != -1)
		{
			PxShape* currShape = NULL;
			if(shapeIndex < 128)
			{
				currShape = shapeBuffer[shapeIndex];
			}
			else
			{
				PxShape* shapeBuffer2[1];
				vehActor->getShapes(shapeBuffer2,1,(PxU32)shapeIndex);
				currShape = shapeBuffer2[0];
			}
			PX_ASSERT(currShape);
			currShape->setLocalPose(localPoses[i]);
		}
	}
}



////////////////////////////////////////////////////////////////////////////
//Update each vehicle type with a special function
////////////////////////////////////////////////////////////////////////////

class PxVehicleUpdate
{
public:

#if PX_DEBUG_VEHICLE_ON
	static void updateSingleVehicleAndStoreTelemetryData(
		const PxF32 timestep, const PxVec3& gravity, const PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
		PxVehicleWheels* focusVehicle, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleTelemetryData& telemetryData);
#endif

	static void update(
		const PxF32 timestep, const PxVec3& gravity, const PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
		const PxU32 numVehicles, PxVehicleWheels** vehicles, PxVehicleWheelQueryResult* wheelQueryResults, PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates);

	static void updatePost(
		const PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates, const PxU32 numVehicles, PxVehicleWheels** vehicles);

	static void suspensionRaycasts(
		PxBatchQuery* batchQuery, 
		const PxU32 numVehicles, PxVehicleWheels** vehicles, const PxU32 numSceneQueryResults, PxRaycastQueryResult* sceneQueryResults,
		const bool* vehiclesToRaycast);

	static void updateDrive4W(
		const PxF32 timestep, 
		const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
		const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
		PxVehicleDrive4W* vehDrive4W, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates);

	static void updateDriveNW(
		const PxF32 timestep, 
		const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
		const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
		PxVehicleDriveNW* vehDriveNW, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates);

	static void updateTank(
		const PxF32 timestep, 
		const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
		const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
		PxVehicleDriveTank* vehDriveTank, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates);

	static void updateNoDrive(
		const PxF32 timestep, 
		const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
		const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
		PxVehicleNoDrive* vehDriveTank, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates);

	static PxU32 computeNumberOfSubsteps(const PxVehicleWheelsSimData& wheelsSimData, const PxVec3& linVel, const PxTransform& globalPose, const PxVec3& forward)
	{
		const PxVec3 z=globalPose.q.rotate(forward);
		const PxF32 vz=PxAbs(linVel.dot(z));
		const PxF32 thresholdVz=wheelsSimData.mThresholdLongitudinalSpeed;
		const PxU32 lowCount=wheelsSimData.mLowForwardSpeedSubStepCount;
		const PxU32 highCount=wheelsSimData.mHighForwardSpeedSubStepCount;
		const PxU32 count=(vz<thresholdVz ? lowCount : highCount);
		return count;
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleWheelsDynData& wheels)
	{
		const PxU32 nbWheels4 = (wheels.mNbActiveWheels + 3) >> 2;
		PxVehicleWheels4DynData* wheels4 = wheels.getWheel4DynData();
		for(PxU32 i = 0; i < nbWheels4; i++)
		{
			wheels4[i].setInternalDynamicsToZero();
		}
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleDriveDynData& drive)
	{
		drive.setEngineRotationSpeed(0.0f);
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleNoDrive* veh)
	{
		setInternalDynamicsToZero(veh->mWheelsDynData);
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleDrive4W* veh)
	{
		setInternalDynamicsToZero(veh->mWheelsDynData);
		setInternalDynamicsToZero(veh->mDriveDynData);
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleDriveNW* veh)
	{
		veh->mDriveDynData.setEngineRotationSpeed(0.0f);
		setInternalDynamicsToZero(veh->mWheelsDynData);
		setInternalDynamicsToZero(veh->mDriveDynData);
	}

	PX_INLINE static void setInternalDynamicsToZero(PxVehicleDriveTank* veh)
	{
		setInternalDynamicsToZero(veh->mWheelsDynData);
		setInternalDynamicsToZero(veh->mDriveDynData);
	}


	PX_INLINE static bool isOnDynamicActor(const PxVehicleWheelsSimData& wheelsSimData, const PxVehicleWheelsDynData& wheelsDynData)
	{
		const PxU32 numWheels4 = wheelsSimData.mNbWheels4;
		const PxVehicleWheels4DynData* PX_RESTRICT wheels4DynDatas = wheelsDynData.mWheels4DynData;

		for(PxU32 i=0;i<numWheels4;i++)
		{
			for(PxU32 j=0;j<4;j++)
			{
				if(!wheelsSimData.getIsWheelDisabled(4*i + j))
				{
					const PxU32 hitCount=PxU32(wheels4DynDatas[i].mSqResults[j].hasBlock);
					const PxRaycastHit& hit=wheels4DynDatas[i].mSqResults[j].block;
					if(hitCount && hit.actor && hit.actor->is<PxRigidDynamic>())
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	PX_INLINE static void storeRaycasts(const PxVehicleWheels4DynData& dynData, PxWheelQueryResult* wheelQueryResults)
	{
		if(dynData.mSqResults)
		{
			for(PxU32 i=0;i<4;i++)
			{
				const PxVehicleWheels4DynData::SuspLineRaycast& raycast = 
					(const PxVehicleWheels4DynData::SuspLineRaycast&)dynData.mRaycastsOrCachedHitResults;

				wheelQueryResults[i].suspLineStart=raycast.mStarts[i];
				wheelQueryResults[i].suspLineDir=raycast.mDirs[i];
				wheelQueryResults[i].suspLineLength=raycast.mLengths[i];
			}
		}
		else
		{
			for(PxU32 i=0;i<4;i++)
			{
				wheelQueryResults[i].suspLineStart=PxVec3(0,0,0);
				wheelQueryResults[i].suspLineDir=PxVec3(0,0,0);
				wheelQueryResults[i].suspLineLength=0;
			}
		}
	}

	PX_INLINE static void storeSuspWheelTireResults
		(const ProcessSuspWheelTireOutputData& outputData, const PxF32* steerAngles, PxWheelQueryResult* wheelQueryResults, const PxU32 numWheels)
	{
		for(PxU32 i=0;i<numWheels;i++)
		{
			wheelQueryResults[i].isInAir=outputData.isInAir[i];
			wheelQueryResults[i].tireContactActor=outputData.tireContactActors[i];
			wheelQueryResults[i].tireContactShape=outputData.tireContactShapes[i];
			wheelQueryResults[i].tireSurfaceMaterial=outputData.tireSurfaceMaterials[i];
			wheelQueryResults[i].tireSurfaceType=outputData.tireSurfaceTypes[i];
			wheelQueryResults[i].tireContactPoint=outputData.tireContactPoints[i];
			wheelQueryResults[i].tireContactNormal=outputData.tireContactNormals[i];
			wheelQueryResults[i].tireFriction=outputData.frictions[i];
			wheelQueryResults[i].suspJounce=outputData.jounces[i];
			wheelQueryResults[i].suspSpringForce=outputData.suspensionSpringForces[i];
			wheelQueryResults[i].tireLongitudinalDir=outputData.tireLongitudinalDirs[i];
			wheelQueryResults[i].tireLateralDir=outputData.tireLateralDirs[i];
			wheelQueryResults[i].longitudinalSlip=outputData.longSlips[i];
			wheelQueryResults[i].lateralSlip=outputData.latSlips[i];
			wheelQueryResults[i].steerAngle=steerAngles[i];
		}
	}

	PX_INLINE static void storeHitActorForces(const ProcessSuspWheelTireOutputData& outputData, PxVehicleWheelConcurrentUpdateData* wheelConcurrentUpdates, const PxU32 numWheels)
	{
		for(PxU32 i=0;i<numWheels;i++)
		{
			PX_ASSERT(!wheelConcurrentUpdates[i].hitActor || !outputData.hitActors[i] || wheelConcurrentUpdates[i].hitActor==outputData.hitActors[i]);
			wheelConcurrentUpdates[i].hitActor=outputData.hitActors[i];
			wheelConcurrentUpdates[i].hitActorForce+=outputData.hitActorForces[i];
			wheelConcurrentUpdates[i].hitActorForcePosition=outputData.hitActorForcePositions[i];
		}
	}


	static void shiftOrigin(const PxVec3& shift, const PxU32 numVehicles, PxVehicleWheels** vehicles);
};

void PxVehicleUpdate::updateDrive4W(
const PxF32 timestep, 
const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
PxVehicleDrive4W* vehDrive4W, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates)
{
	PX_SIMD_GUARD; // denorm exception in transformInertiaTensor() on osx

	START_TIMER(TIMER_ADMIN);

	PX_CHECK_AND_RETURN(
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL]>-0.01f && 
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL]<1.01f, 
		"Illegal vehicle control value - accel must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE]>-0.01f && 
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_BRAKE]<1.01f, 
		"Illegal vehicle control value - brake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE]>-0.01f && 
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_HANDBRAKE]<1.01f, 
		"Illegal vehicle control value - handbrake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT]>-1.01f && 
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT]<1.01f, 
		"Illegal vehicle control value - left steer must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT]>-1.01f && 
		vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT]<1.01f, 
		"Illegal vehicle control value - right steer must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		PxAbs(vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT]-
			  vehDrive4W->mDriveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT])<1.01f, 
		"Illegal vehicle control value - right steer value minus left steer value must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		!(vehDrive4W->getRigidDynamicActor()->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC),
		"Attempting to update a drive4W with a kinematic actor - this isn't allowed");
	PX_CHECK_AND_RETURN(
		NULL==vehWheelQueryResults || vehWheelQueryResults->nbWheelQueryResults >= vehDrive4W->mWheelsSimData.getNbWheels(), 
		"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");
	PX_CHECK_AND_RETURN(
		NULL==vehConcurrentUpdates || vehConcurrentUpdates->nbConcurrentWheelUpdates >= vehDrive4W->mWheelsSimData.getNbWheels(), 
		"vehConcurrentUpdates->nbConcurrentWheelUpdates must always be greater than or equal to number of wheels in corresponding vehicle");

#ifdef PX_CHECKED
	{
		//Check that the sense of left/right and forward/rear is true.
		const PxVec3 fl=vehDrive4W->mWheelsSimData.mWheels4SimData[0].getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_LEFT);
		const PxVec3 fr=vehDrive4W->mWheelsSimData.mWheels4SimData[0].getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eFRONT_RIGHT);
		const PxVec3 rl=vehDrive4W->mWheelsSimData.mWheels4SimData[0].getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_LEFT);
		const PxVec3 rr=vehDrive4W->mWheelsSimData.mWheels4SimData[0].getWheelCentreOffset(PxVehicleDrive4WWheelOrder::eREAR_RIGHT);
		const PxVec3 right=gRight;
		const PxF32 s0=computeSign((fr-fl).dot(right));
		const PxF32 s1=computeSign((rr-rl).dot(right));
		PX_CHECK_AND_RETURN(0==s0 || 0==s1 || s0==s1, "PxVehicle4W does not obey the rule that the eFRONT_RIGHT/eREAR_RIGHT wheels are to the right of the eFRONT_LEFT/eREAR_LEFT wheels");
	}
#endif

	END_TIMER(TIMER_ADMIN);
	START_TIMER(TIMER_GRAPHS);

#if PX_DEBUG_VEHICLE_ON
	for(PxU32 i=0;i<vehDrive4W->mWheelsSimData.mNbWheels4;i++)
	{
		updateGraphDataInternalWheelDynamics(4*i,vehDrive4W->mWheelsDynData.mWheels4DynData[i].mWheelSpeeds);
	}
	updateGraphDataInternalEngineDynamics(vehDrive4W->mDriveDynData.getEngineRotationSpeed());
#endif

	END_TIMER(TIMER_GRAPHS);
	START_TIMER(TIMER_ADMIN);

	//Unpack the vehicle.
	//Unpack the 4W simulation and instanced dynamics components.
	const PxVehicleWheels4SimData* wheels4SimDatas=vehDrive4W->mWheelsSimData.mWheels4SimData;
	const PxVehicleTireLoadFilterData& tireLoadFilterData=vehDrive4W->mWheelsSimData.mNormalisedLoadFilter;
	PxVehicleWheels4DynData* wheels4DynDatas=vehDrive4W->mWheelsDynData.mWheels4DynData;
	const PxU32 numWheels4=vehDrive4W->mWheelsSimData.mNbWheels4;
	const PxU32 numActiveWheels=vehDrive4W->mWheelsSimData.mNbActiveWheels;
	const PxU32 numActiveWheelsInLast4=4-(4*numWheels4 - numActiveWheels);
	const PxVehicleDriveSimData4W driveSimData=vehDrive4W->mDriveSimData;
	PxVehicleDriveDynData& driveDynData=vehDrive4W->mDriveDynData;
	PxRigidDynamic* vehActor=vehDrive4W->mActor;

	//We need to store that data we are going to write to actors so we can do this at the end in one go with fewer write locks.
	PxVehicleWheelConcurrentUpdateData wheelConcurrentUpdates[PX_MAX_NB_WHEELS];
	PxVehicleConcurrentUpdateData vehicleConcurrentUpdates;
	vehicleConcurrentUpdates.nbConcurrentWheelUpdates = numActiveWheels;
	vehicleConcurrentUpdates.concurrentWheelUpdates = wheelConcurrentUpdates;

	//Test if a non-zero drive torque was applied or if a non-zero steer angle was applied.
	bool finiteInputApplied=false;
	if(0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT) || 
	   0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT) ||
	   0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL) ||
	   driveDynData.getGearDown() || driveDynData.getGearUp())
	{
		finiteInputApplied=true;
	}

	//Awake or sleep.
	if(vehActor->isSleeping())
	{
		if(finiteInputApplied)
		{
			//Driving inputs so we need the actor to start moving.
			vehicleConcurrentUpdates.wakeup = true;
		}
		else if(isOnDynamicActor(vehDrive4W->mWheelsSimData, vehDrive4W->mWheelsDynData))
		{
			//Driving on dynamic so we need to keep moving.
			vehicleConcurrentUpdates.wakeup = true;
		}
		else
		{
			//No driving inputs and the actor is asleep.
			//Set internal dynamics to zero.
			setInternalDynamicsToZero(vehDrive4W);
			if(vehConcurrentUpdates) vehConcurrentUpdates->staySleeping = true;
			return;
		}
	}

	//In each block of 4 wheels record how many wheels are active.
	PxU32 numActiveWheelsPerBlock4[PX_MAX_NB_SUSPWHEELTIRE4]={0,0,0,0,0};
	numActiveWheelsPerBlock4[0]=PxMin(numActiveWheels,(PxU32)4);
	for(PxU32 i=1;i<numWheels4-1;i++)
	{
		numActiveWheelsPerBlock4[i]=4;
	}
	numActiveWheelsPerBlock4[numWheels4-1]=numActiveWheelsInLast4;
	PX_ASSERT(numActiveWheels == numActiveWheelsPerBlock4[0] + numActiveWheelsPerBlock4[1] + numActiveWheelsPerBlock4[2] + numActiveWheelsPerBlock4[3] + numActiveWheelsPerBlock4[4]); 

	//Organise the shader data in blocks of 4.
	PxVehicleTireForceCalculator4 tires4ForceCalculators[PX_MAX_NB_SUSPWHEELTIRE4];
	for(PxU32 i=0;i<numWheels4;i++)
	{
		tires4ForceCalculators[i].mShaderData[0]=vehDrive4W->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+0];
		tires4ForceCalculators[i].mShaderData[1]=vehDrive4W->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+1];
		tires4ForceCalculators[i].mShaderData[2]=vehDrive4W->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+2];
		tires4ForceCalculators[i].mShaderData[3]=vehDrive4W->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+3];
		tires4ForceCalculators[i].mShader=vehDrive4W->mWheelsDynData.mTireForceCalculators->mShader;
	}

	//Mark the constraints as dirty to force them to be updated in the sdk.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		wheels4DynDatas[i].getVehicletConstraintShader().mConstraint->markDirty();
	}

	//We need to store data to pose the wheels at the end.
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];

	END_TIMER(TIMER_ADMIN);
	START_TIMER(TIMER_COMPONENTS_UPDATE);

	//Center of mass local pose.
	PxTransform carChassisCMLocalPose;
	//Compute the transform of the center of mass.
	PxTransform origCarChassisTransform;
	PxTransform carChassisTransform;
	//Inverse mass and inertia to apply the tire/suspension forces as impulses.
	PxF32 inverseChassisMass;
	PxVec3 inverseInertia;
	//Linear and angular velocity.
	PxVec3 carChassisLinVel;
	PxVec3 carChassisAngVel;
	{
		carChassisCMLocalPose = vehActor->getCMassLocalPose();
		origCarChassisTransform = vehActor->getGlobalPose().transform(carChassisCMLocalPose);
		carChassisTransform = origCarChassisTransform;
		const PxF32 chassisMass = vehActor->getMass();
		inverseChassisMass = 1.0f/chassisMass;
		inverseInertia = vehActor->getMassSpaceInvInertiaTensor();
		carChassisLinVel = vehActor->getLinearVelocity();
		carChassisAngVel = vehActor->getAngularVelocity();
	}

	//Update the auto-box and decide whether to change gear up or down.
	PxF32 autoboxCompensatedAnalogAccel = driveDynData.mControlAnalogVals[PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL];
	if(driveDynData.getUseAutoGears())
	{
		autoboxCompensatedAnalogAccel = processAutoBox(PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL,timestep,driveSimData,driveDynData);
	}

	//Process gear-up/gear-down commands.
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		processGears(timestep,gearsData,driveDynData);
	}

	//Clutch strength;
	PxF32 K;
	{
		const PxVehicleClutchData& clutchData=driveSimData.getClutchData();
		const PxU32 currentGear=driveDynData.getCurrentGear();
		K=computeClutchStrength(clutchData, currentGear);
	}

	//Clutch accuracy
	PxVehicleClutchAccuracyMode::Enum clutchAccuracyMode;
	PxU32 clutchMaxIterations;
	{
		const PxVehicleClutchData& clutchData=driveSimData.getClutchData();
		clutchAccuracyMode = clutchData.mAccuracyMode;
		clutchMaxIterations = clutchData.mEstimateIterations;
	}

	//Gear ratio.
	PxF32 G;
	PxU32 currentGear;
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		currentGear=driveDynData.getCurrentGear();
		G=computeGearRatio(gearsData,currentGear);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataGearRatio(G);
#endif
	}

	//Retrieve control values from vehicle controls.
	PxF32 accel,brake,handbrake,steerLeft,steerRight;
	PxF32 steer;
	bool isIntentionToAccelerate;
	{
		getVehicle4WControlValues(driveDynData,accel,brake,handbrake,steerLeft,steerRight);
		steer=steerRight-steerLeft;
		accel=autoboxCompensatedAnalogAccel;
		isIntentionToAccelerate = (accel>0.0f && 0.0f==brake && 0.0f==handbrake && PxVehicleGearsData::eNEUTRAL != currentGear);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataControlInputs(accel,brake,handbrake,steerLeft,steerRight);
#endif
	}

	//Active wheels (wheels which have not been disabled).
	bool activeWheelStates[4]={false,false,false,false};
	{
		computeWheelActiveStates(4*0, vehDrive4W->mWheelsSimData.mActiveWheelsBitmapBuffer, activeWheelStates);
	}

	//Get the drive wheels (the first 4 wheels are the drive wheels).
	const PxVehicleWheels4SimData& wheels4SimData=wheels4SimDatas[0];
	PxVehicleWheels4DynData& wheels4DynData=wheels4DynDatas[0];
	const PxVehicleTireForceCalculator4& tires4ForceCalculator=tires4ForceCalculators[0];

	//Contribution of each driven wheel to average wheel speed at clutch.
	//With 4 driven wheels the average wheel speed at clutch is 
	//wAve = alpha0*w0 + alpha1*w1 + alpha2*w2 + alpha3*w3.
	//This next bit of code computes alpha0,alpha1,alpha2,alpha3.
	//For rear wheel drive alpha0=alpha1=0
	//For front wheel drive alpha2=alpha3=0
	PxF32 aveWheelSpeedContributions[4]={0.0f,0.0f,0.0f,0.0f};
	{
		const PxVehicleDifferential4WData& diffData=driveSimData.getDiffData();
		computeDiffAveWheelSpeedContributions(diffData,handbrake,aveWheelSpeedContributions);

#if PX_DEBUG_VEHICLE_ON
		updateGraphDataClutchSlip(wheels4DynData.mWheelSpeeds,aveWheelSpeedContributions,driveDynData.getEngineRotationSpeed(),G);
#endif

		PX_CHECK_AND_RETURN(
			(activeWheelStates[0] || 0.0f==aveWheelSpeedContributions[0]) &&
			(activeWheelStates[1] || 0.0f==aveWheelSpeedContributions[1]) &&
			(activeWheelStates[2] || 0.0f==aveWheelSpeedContributions[2]) &&
			(activeWheelStates[3] || 0.0f==aveWheelSpeedContributions[3]),
			"PxVehicleDifferential4WData must be configured so that no torque is delivered to a disabled wheel");
	}

	//Compute a per-wheel accelerator pedal value.
	bool isAccelApplied[4]={false,false,false,false};
	if(isIntentionToAccelerate)
	{
		PX_ASSERT(accel>0);
		computeIsAccelApplied(aveWheelSpeedContributions, isAccelApplied);
	}

	//Ackermann-corrected steering angles.
	//http://en.wikipedia.org/wiki/Ackermann_steering_geometry
	PxF32 steerAngles[4]={0.0f,0.0f,0.0f,0.0f};
	{
		computeAckermannCorrectedSteerAngles(driveSimData,wheels4SimData,steer,steerAngles);
	}

	END_TIMER(TIMER_COMPONENTS_UPDATE);
	START_TIMER(TIMER_ADMIN);

	//Store the susp line raycast data.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		storeRaycasts(wheels4DynDatas[i], &wheelQueryResults[4*i]);
	}

	//Ready to do the update.
	PxVec3 carChassisLinVelOrig=carChassisLinVel;
	PxVec3 carChassisAngVelOrig=carChassisAngVel;
	const PxU32 numSubSteps=computeNumberOfSubsteps(vehDrive4W->mWheelsSimData,carChassisLinVel,carChassisTransform,gForward);
	const PxF32 timeFraction=1.0f/(1.0f*numSubSteps);
	const PxF32 subTimestep=timestep*timeFraction;
	const PxF32 recipSubTimeStep=1.0f/subTimestep;
	const PxF32 recipTimestep=1.0f/timestep;
	const PxF32 minLongSlipDenominator=vehDrive4W->mWheelsSimData.mMinLongSlipDenominator;
	ProcessSuspWheelTireConstData constData={timeFraction, subTimestep, recipSubTimeStep, gravity, gravityMagnitude, recipGravityMagnitude, false, minLongSlipDenominator, vehActor, &drivableSurfaceToTireFrictionPairs};

	END_TIMER(TIMER_ADMIN);

	for(PxU32 k=0;k<numSubSteps;k++)
	{
		//Set the force and torque for the current update to zero.
		PxVec3 chassisForce(0,0,0);
		PxVec3 chassisTorque(0,0,0);

		START_TIMER(TIMER_COMPONENTS_UPDATE);

		//Update the drive/steer wheels and engine.
		{
			//Compute the brake torques.
			PxF32 brakeTorques[4]={0.0f,0.0f,0.0f,0.0f};
			bool isBrakeApplied[4]={false,false,false,false};
			computeBrakeAndHandBrakeTorques
				(&wheels4SimData.getWheelData(0),wheels4DynData.mWheelSpeeds,brake,handbrake,
				 brakeTorques,isBrakeApplied);

			END_TIMER(TIMER_COMPONENTS_UPDATE);
			START_TIMER(TIMER_WHEELS);

			//Compute jounces, slips, tire forces, suspension forces etc.
			ProcessSuspWheelTireInputData inputData=
			{
				isIntentionToAccelerate, isAccelApplied, isBrakeApplied, steerAngles, activeWheelStates,
				carChassisTransform, carChassisLinVel, carChassisAngVel,
				&wheels4SimData, &wheels4DynData, &tires4ForceCalculator, &tireLoadFilterData, numActiveWheelsPerBlock4[0]
			};
			ProcessSuspWheelTireOutputData outputData;
			processSuspTireWheels(0, constData, inputData, outputData);
			updateLowSpeedTimers(outputData.newLowForwardSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowForwardSpeedTimers);
			updateLowSpeedTimers(outputData.newLowSideSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowSideSpeedTimers);
			updateJounces(outputData.jounces, (PxF32*)inputData.vehWheels4DynData->mJounces);
			if((numSubSteps-1) == k)
			{
				updateCachedHitData(outputData.cachedHitCounts, outputData.cachedHitPlanes, outputData.cachedHitDistances, outputData.cachedFrictionMultipliers, &wheels4DynData);
			}
			chassisForce+=outputData.chassisForce;
			chassisTorque+=outputData.chassisTorque;
			if(0 == k)
			{
				wheels4DynData.mVehicleConstraints->mData=outputData.vehConstraintData;
			}
			storeSuspWheelTireResults(outputData, inputData.steerAngles, &wheelQueryResults[4*0], numActiveWheelsPerBlock4[0]);
			storeHitActorForces(outputData, &vehicleConcurrentUpdates.concurrentWheelUpdates[4*0], numActiveWheelsPerBlock4[0]);

			END_TIMER(TIMER_WHEELS);
			START_TIMER(TIMER_INTERNAL_DYNAMICS_SOLVER);

			//Diff torque ratios needed (how we split the torque between the drive wheels).
			//The sum of the torque ratios is always 1.0f.
			//The drive torque delivered to each wheel is the total available drive torque multiplied by the 
			//diff torque ratio for each wheel.
			PxF32 diffTorqueRatios[4]={0.0f,0.0f,0.0f,0.0f};
			computeDiffTorqueRatios(driveSimData.getDiffData(),handbrake,wheels4DynData.mWheelSpeeds,diffTorqueRatios);

			PX_CHECK_AND_RETURN(
				(activeWheelStates[0] || 0.0f==diffTorqueRatios[0]) &&
				(activeWheelStates[1] || 0.0f==diffTorqueRatios[1]) &&
				(activeWheelStates[2] || 0.0f==diffTorqueRatios[2]) &&
				(activeWheelStates[3] || 0.0f==diffTorqueRatios[3]),
				"PxVehicleDifferential4WData must be configured so that no torque is delivered to a disabled wheel");

			PxF32 engineDriveTorque;
			{
				const PxVehicleEngineData& engineData=driveSimData.getEngineData();
				const PxF32 engineOmega=driveDynData.getEngineRotationSpeed();
				engineDriveTorque=computeEngineDriveTorque(engineData,engineOmega,accel);
	#if PX_DEBUG_VEHICLE_ON
				updateGraphDataEngineDriveTorque(engineDriveTorque);
	#endif
			}

			PxF32 engineDampingRate;
			{
				const PxVehicleEngineData& engineData=driveSimData.getEngineData();
				engineDampingRate=computeEngineDampingRate(engineData,currentGear,accel);
			}

			//Update the wheel and engine speeds - 5x5 matrix coupling engine and wheels.
			ImplicitSolverInput implicitSolverInput=
			{
				subTimestep, 
				brake, handbrake,
				K, G,
				clutchAccuracyMode, clutchMaxIterations,
				engineDriveTorque, engineDampingRate,
				diffTorqueRatios, aveWheelSpeedContributions, 
				brakeTorques, isBrakeApplied, outputData.tireTorques,
				1, 4,
				&wheels4SimData, &driveSimData
			};
			ImplicitSolverOutput implicitSolverOutput=
			{
				&wheels4DynData, &driveDynData
			};
			solveDrive4WInternaDynamicsEnginePlusDrivenWheels(implicitSolverInput, &implicitSolverOutput);

			END_TIMER(TIMER_INTERNAL_DYNAMICS_SOLVER);
			START_TIMER(TIMER_POSTUPDATE1);

			//Integrate wheel rotation angle (theta += omega*dt)
			integrateWheelRotationAngles
				(subTimestep,
				K,G,engineDriveTorque,
				outputData.jounces,diffTorqueRatios,outputData.forwardSpeeds,isBrakeApplied,
				driveSimData,wheels4SimData,
				driveDynData,wheels4DynData);
		}

		END_TIMER(TIMER_POSTUPDATE1);

		//////////////////////////////////////////////////////////////////////////
		//susp and tire forces from extra wheels (non-driven wheels)
		//////////////////////////////////////////////////////////////////////////
		for(PxU32 j=1;j<numWheels4;j++)
		{
			//Only the driven wheels can steer but the non-drive wheels can still have a toe angle.
			const PxVehicleWheelData& wheelData0=wheels4SimDatas[j].getWheelData(0);
			const PxVehicleWheelData& wheelData1=wheels4SimDatas[j].getWheelData(1);
			const PxVehicleWheelData& wheelData2=wheels4SimDatas[j].getWheelData(2);
			const PxVehicleWheelData& wheelData3=wheels4SimDatas[j].getWheelData(3);
			const PxF32 toe0=wheelData0.mToeAngle;
			const PxF32 toe1=wheelData1.mToeAngle;
			const PxF32 toe2=wheelData2.mToeAngle;
			const PxF32 toe3=wheelData3.mToeAngle;
			PxF32 extraWheelSteerAngles[4]={toe0,toe1,toe2,toe3};

			//Only the driven wheels are connected to the diff.
			PxF32 extraWheelsDiffTorqueRatios[4]={0.0f,0.0f,0.0f,0.0f};
			bool extraIsAccelApplied[4]={false,false,false,false};

			//The extra wheels do have brakes.
			PxF32 extraWheelBrakeTorques[4]={0.0f,0.0f,0.0f,0.0f};
			bool extraIsBrakeApplied[4]={false,false,false,false};
			computeBrakeAndHandBrakeTorques
				(&wheels4SimDatas[j].getWheelData(0),wheels4DynDatas[j].mWheelSpeeds,brake,handbrake,
				extraWheelBrakeTorques,extraIsBrakeApplied);

			//The extra wheels can be disabled or enabled.
			bool extraWheelActiveStates[4]={false,false,false,false};
			computeWheelActiveStates(4*j, vehDrive4W->mWheelsSimData.mActiveWheelsBitmapBuffer, extraWheelActiveStates);

			ProcessSuspWheelTireInputData extraInputData=
			{
				isIntentionToAccelerate, extraIsAccelApplied, extraIsBrakeApplied, extraWheelSteerAngles, extraWheelActiveStates,
				carChassisTransform,carChassisLinVel,carChassisAngVel,
				&wheels4SimDatas[j],&wheels4DynDatas[j],&tires4ForceCalculators[j],&tireLoadFilterData,numActiveWheelsPerBlock4[j],
			};
			ProcessSuspWheelTireOutputData extraOutputData;
			processSuspTireWheels(4*j, constData, extraInputData, extraOutputData);
			updateLowSpeedTimers(extraOutputData.newLowForwardSpeedTimers, (PxF32*)extraInputData.vehWheels4DynData->mTireLowForwardSpeedTimers);
			updateLowSpeedTimers(extraOutputData.newLowSideSpeedTimers, (PxF32*)extraInputData.vehWheels4DynData->mTireLowSideSpeedTimers);
			updateJounces(extraOutputData.jounces, (PxF32*)extraInputData.vehWheels4DynData->mJounces);
			if((numSubSteps-1) == k)
			{
				updateCachedHitData(extraOutputData.cachedHitCounts, extraOutputData.cachedHitPlanes, extraOutputData.cachedHitDistances, extraOutputData.cachedFrictionMultipliers, &wheels4DynDatas[j]);
			}
			chassisForce+=extraOutputData.chassisForce;
			chassisTorque+=extraOutputData.chassisTorque;
			if(0 == k)
			{
				wheels4DynDatas[j].mVehicleConstraints->mData=extraOutputData.vehConstraintData;
			}
			storeSuspWheelTireResults(extraOutputData, extraInputData.steerAngles, &wheelQueryResults[4*j], numActiveWheelsPerBlock4[j]);
			storeHitActorForces(extraOutputData, &vehicleConcurrentUpdates.concurrentWheelUpdates[4*j], numActiveWheelsPerBlock4[j]);

			//Integrate the tire torques (omega += (tireTorque + brakeTorque)*dt)
			integrateUndriveWheelRotationSpeeds(subTimestep, brake, handbrake, extraOutputData.tireTorques, extraWheelBrakeTorques, wheels4SimDatas[j], wheels4DynDatas[j]);

			//Integrate wheel rotation angle (theta += omega*dt)
			integrateWheelRotationAngles
				(subTimestep,
				0,0,0,
				extraOutputData.jounces,extraWheelsDiffTorqueRatios,extraOutputData.forwardSpeeds,extraIsBrakeApplied,
				driveSimData,wheels4SimDatas[j],
				driveDynData,wheels4DynDatas[j]);
		}

		START_TIMER(TIMER_POSTUPDATE2);

		integrateBody(inverseChassisMass,inverseInertia,chassisForce,chassisTorque,subTimestep,carChassisLinVel,carChassisAngVel,carChassisTransform);

		END_TIMER(TIMER_POSTUPDATE2);
	}

	START_TIMER(TIMER_POSTUPDATE3);

	//Set the new chassis linear/angular velocity.
	if(!gApplyForces)
	{
		vehicleConcurrentUpdates.linearMomentumChange = carChassisLinVel;
		vehicleConcurrentUpdates.angularMomentumChange = carChassisAngVel;
	}
	else
	{
		//integration steps are: 
		//v = v0 + a*dt	(1)
		//x = x0 + v*dt	(2)
		//Sub (2) into (1.
		//x = x0 + v0*dt + a*dt*dt;
		//Rearrange for a
		//a = (x -x0 - v0*dt)/(dt*dt) = [(x-x0)/dt - v0/dt]
		//Rearrange again with v = (x-x0)/dt
		//a = (v - v0)/dt
		vehicleConcurrentUpdates.linearMomentumChange = (carChassisLinVel-carChassisLinVelOrig)*recipTimestep;;
		vehicleConcurrentUpdates.angularMomentumChange = (carChassisAngVel-carChassisAngVelOrig)*recipTimestep;;
	}

	//Compute and pose the wheels from jounces, rotations angles, and steer angles.
	PxTransform localPoses0[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
	computeWheelLocalPoses(wheels4SimDatas[0],wheels4DynDatas[0],&wheelQueryResults[4*0],numActiveWheelsPerBlock4[0],carChassisCMLocalPose,localPoses0);
	//Copy the poses to the wheelQueryResults
	wheelQueryResults[4*0 + 0].localPose = localPoses0[0];
	wheelQueryResults[4*0 + 1].localPose = localPoses0[1];
	wheelQueryResults[4*0 + 2].localPose = localPoses0[2];
	wheelQueryResults[4*0 + 3].localPose = localPoses0[3];
	//Copy the poses to the concurrent update data.
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 0].localPose = localPoses0[0];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 1].localPose = localPoses0[1];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 2].localPose = localPoses0[2];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 3].localPose = localPoses0[3];
	for(PxU32 i=1;i<numWheels4;i++)
	{
		PxTransform localPoses[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
		computeWheelLocalPoses(wheels4SimDatas[i],wheels4DynDatas[i],&wheelQueryResults[4*i],numActiveWheelsPerBlock4[i],carChassisCMLocalPose,localPoses);
		//Copy the poses to the wheelQueryResults
		wheelQueryResults[4*i + 0].localPose = localPoses[0];
		wheelQueryResults[4*i + 1].localPose = localPoses[1];
		wheelQueryResults[4*i + 2].localPose = localPoses[2];
		wheelQueryResults[4*i + 3].localPose = localPoses[3];
		//Copy the poses to the concurrent update data.
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 0].localPose = localPoses[0];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 1].localPose = localPoses[1];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 2].localPose = localPoses[2];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 3].localPose = localPoses[3];
	}

	if(vehWheelQueryResults && vehWheelQueryResults->wheelQueryResults)
	{
		PxMemCopy(vehWheelQueryResults->wheelQueryResults, wheelQueryResults, sizeof(PxWheelQueryResult)*numActiveWheels);
	}

	if(vehConcurrentUpdates)
	{
		//Copy across to input data structure so that writes can be applied later.
		PxMemCopy(vehConcurrentUpdates->concurrentWheelUpdates, vehicleConcurrentUpdates.concurrentWheelUpdates, sizeof(PxVehicleWheelConcurrentUpdateData)*numActiveWheels);
		vehConcurrentUpdates->linearMomentumChange = vehicleConcurrentUpdates.linearMomentumChange;
		vehConcurrentUpdates->angularMomentumChange = vehicleConcurrentUpdates.angularMomentumChange;
		vehConcurrentUpdates->staySleeping = vehicleConcurrentUpdates.staySleeping;
		vehConcurrentUpdates->wakeup = vehicleConcurrentUpdates.wakeup;
	}
	else
	{
		//Apply the writes immediately.
		PxVehicleWheels* vehWheels[1]={vehDrive4W};
		PxVehiclePostUpdates(&vehicleConcurrentUpdates, 1, vehWheels);
	}

	END_TIMER(TIMER_POSTUPDATE3);
}

void PxVehicleUpdate::updateDriveNW
(const PxF32 timestep, 
 const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude,
 const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
 PxVehicleDriveNW* vehDriveNW, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates)
{
	PX_SIMD_GUARD; // denorm exception triggered in transformInertiaTensor() on osx

	START_TIMER(TIMER_ADMIN);

	PX_CHECK_AND_RETURN(
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL]>-0.01f && 
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL]<1.01f, 
		"Illegal vehicle control value - accel must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_BRAKE]>-0.01f && 
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_BRAKE]<1.01f, 
		"Illegal vehicle control value - brake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_HANDBRAKE]>-0.01f && 
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_HANDBRAKE]<1.01f, 
		"Illegal vehicle control value - handbrake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_LEFT]>-1.01f && 
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_LEFT]<1.01f, 
		"Illegal vehicle control value - left steer must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_RIGHT]>-1.01f && 
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_RIGHT]<1.01f, 
		"Illegal vehicle control value - right steer must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		PxAbs(vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_RIGHT]-
		vehDriveNW->mDriveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_STEER_LEFT])<1.01f, 
		"Illegal vehicle control value - right steer value minus left steer value must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		!(vehDriveNW->getRigidDynamicActor()->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC),
		"Attempting to update a drive4W with a kinematic actor - this isn't allowed");
	PX_CHECK_AND_RETURN(
		NULL==vehWheelQueryResults || vehWheelQueryResults->nbWheelQueryResults >= vehDriveNW->mWheelsSimData.getNbWheels(), 
		"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");

#ifdef PX_CHECKED
	for(PxU32 i=0;i<vehDriveNW->mWheelsSimData.getNbWheels();i++)
	{
		PX_CHECK_AND_RETURN(!vehDriveNW->mWheelsSimData.getIsWheelDisabled(i) || !vehDriveNW->mDriveSimData.getDiffData().getIsDrivenWheel(i), 
			"PxVehicleDifferentialNWData must be configured so that no torque is delivered to a disabled wheel");
	}
#endif

	END_TIMER(TIMER_ADMIN);
	START_TIMER(TIMER_GRAPHS);

#if PX_DEBUG_VEHICLE_ON
	for(PxU32 i=0;i<vehDriveNW->mWheelsSimData.mNbWheels4;i++)
	{
		updateGraphDataInternalWheelDynamics(4*i,vehDriveNW->mWheelsDynData.mWheels4DynData[i].mWheelSpeeds);
	}
	updateGraphDataInternalEngineDynamics(vehDriveNW->mDriveDynData.getEngineRotationSpeed());
#endif

	END_TIMER(TIMER_GRAPHS);
	START_TIMER(TIMER_ADMIN);

	//Unpack the vehicle.
	//Unpack the NW simulation and instanced dynamics components.
	const PxVehicleWheels4SimData* wheels4SimDatas=vehDriveNW->mWheelsSimData.mWheels4SimData;
	const PxVehicleTireLoadFilterData& tireLoadFilterData=vehDriveNW->mWheelsSimData.mNormalisedLoadFilter;
	PxVehicleWheels4DynData* wheels4DynDatas=vehDriveNW->mWheelsDynData.mWheels4DynData;
	const PxU32 numWheels4=vehDriveNW->mWheelsSimData.mNbWheels4;
	const PxU32 numActiveWheels=vehDriveNW->mWheelsSimData.mNbActiveWheels;
	const PxU32 numActiveWheelsInLast4=4-(4*numWheels4 - numActiveWheels);
	const PxVehicleDriveSimDataNW driveSimData=vehDriveNW->mDriveSimData;
	PxVehicleDriveDynData& driveDynData=vehDriveNW->mDriveDynData;
	PxRigidDynamic* vehActor=vehDriveNW->mActor;

	//We need to store that data we are going to write to actors so we can do this at the end in one go with fewer write locks.
	PxVehicleWheelConcurrentUpdateData wheelConcurrentUpdates[PX_MAX_NB_WHEELS];
	PxVehicleConcurrentUpdateData vehicleConcurrentUpdates;
	vehicleConcurrentUpdates.nbConcurrentWheelUpdates = numActiveWheels;
	vehicleConcurrentUpdates.concurrentWheelUpdates = wheelConcurrentUpdates;

	//In each block of 4 wheels record how many wheels are active.
	PxU32 numActiveWheelsPerBlock4[PX_MAX_NB_SUSPWHEELTIRE4]={0,0,0,0,0};
	numActiveWheelsPerBlock4[0]=PxMin(numActiveWheels,(PxU32)4);
	for(PxU32 i=1;i<numWheels4-1;i++)
	{
		numActiveWheelsPerBlock4[i]=4;
	}
	numActiveWheelsPerBlock4[numWheels4-1]=numActiveWheelsInLast4;
	PX_ASSERT(numActiveWheels == numActiveWheelsPerBlock4[0] + numActiveWheelsPerBlock4[1] + numActiveWheelsPerBlock4[2] + numActiveWheelsPerBlock4[3] + numActiveWheelsPerBlock4[4]); 


	//Test if a non-zero drive torque was applied or if a non-zero steer angle was applied.
	bool finiteInputApplied=false;
	if(0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_STEER_LEFT) || 
		0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_STEER_RIGHT) ||
		0!=driveDynData.getAnalogInput(PxVehicleDrive4WControl::eANALOG_INPUT_ACCEL) ||
		driveDynData.getGearDown() || driveDynData.getGearUp())
	{
		finiteInputApplied=true;
	}

	//Awake or sleep.
	{
		if(vehActor->isSleeping())
		{
			if(finiteInputApplied)
			{
				//Driving inputs so we need the actor to start moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else if(isOnDynamicActor(vehDriveNW->mWheelsSimData, vehDriveNW->mWheelsDynData))
			{
				//Driving on dynamic so we need to keep moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else
			{
				//No driving inputs and the actor is asleep.
				//Set internal dynamics to sleep.
				setInternalDynamicsToZero(vehDriveNW);
				if(vehConcurrentUpdates) vehConcurrentUpdates->staySleeping = true;
				return;
			}
		}
	}

	//Organise the shader data in blocks of 4.
	PxVehicleTireForceCalculator4 tires4ForceCalculators[PX_MAX_NB_SUSPWHEELTIRE4];
	for(PxU32 i=0;i<numWheels4;i++)
	{
		tires4ForceCalculators[i].mShaderData[0]=vehDriveNW->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+0];
		tires4ForceCalculators[i].mShaderData[1]=vehDriveNW->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+1];
		tires4ForceCalculators[i].mShaderData[2]=vehDriveNW->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+2];
		tires4ForceCalculators[i].mShaderData[3]=vehDriveNW->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+3];
		tires4ForceCalculators[i].mShader=vehDriveNW->mWheelsDynData.mTireForceCalculators->mShader;
	}

	//Mark the constraints as dirty to force them to be updated in the sdk.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		wheels4DynDatas[i].getVehicletConstraintShader().mConstraint->markDirty();
	}

	//Need to store report data to pose the wheels.
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];

	END_TIMER(TIMER_ADMIN);
	START_TIMER(TIMER_COMPONENTS_UPDATE);

	//Center of mass local pose.
	PxTransform carChassisCMLocalPose;
	//Compute the transform of the center of mass.
	PxTransform origCarChassisTransform;
	PxTransform carChassisTransform;
	//Inverse mass and inertia to apply the tire/suspension forces as impulses.
	PxF32 inverseChassisMass;
	PxVec3 inverseInertia;
	//Linear and angular velocity.
	PxVec3 carChassisLinVel;
	PxVec3 carChassisAngVel;
	{
		carChassisCMLocalPose = vehActor->getCMassLocalPose();
		origCarChassisTransform = vehActor->getGlobalPose().transform(carChassisCMLocalPose);
		carChassisTransform = origCarChassisTransform;
		const PxF32 chassisMass = vehActor->getMass();
		inverseChassisMass = 1.0f/chassisMass;
		inverseInertia = vehActor->getMassSpaceInvInertiaTensor();
		carChassisLinVel = vehActor->getLinearVelocity();
		carChassisAngVel = vehActor->getAngularVelocity();
	}

	//Update the auto-box and decide whether to change gear up or down.
	PxF32 autoboxCompensatedAnalogAccel = driveDynData.mControlAnalogVals[PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL];
	if(driveDynData.getUseAutoGears())
	{
		autoboxCompensatedAnalogAccel = processAutoBox(PxVehicleDriveNWControl::eANALOG_INPUT_ACCEL,timestep,driveSimData,driveDynData);
	}

	//Process gear-up/gear-down commands.
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		processGears(timestep,gearsData,driveDynData);
	}

	//Clutch strength.
	PxF32 K;
	{
		const PxVehicleClutchData& clutchData=driveSimData.getClutchData();
		const PxU32 currentGear=driveDynData.getCurrentGear();
		K=computeClutchStrength(clutchData, currentGear);
	}

	//Clutch accuracy.
	PxVehicleClutchAccuracyMode::Enum clutchAccuracyMode;
	PxU32 clutchMaxIterations;
	{
		const PxVehicleClutchData& clutchData=driveSimData.getClutchData();
		clutchAccuracyMode=clutchData.mAccuracyMode;
		clutchMaxIterations=clutchData.mEstimateIterations;
	}

	//Gear ratio.
	PxF32 G;
	PxU32 currentGear;
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		currentGear=driveDynData.getCurrentGear();
		G=computeGearRatio(gearsData,currentGear);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataGearRatio(G);
#endif
	}

	//Retrieve control values from vehicle controls.
	PxF32 accel,brake,handbrake,steerLeft,steerRight;
	PxF32 steer;
	bool isIntentionToAccelerate;
	{
		getVehicleNWControlValues(driveDynData,accel,brake,handbrake,steerLeft,steerRight);
		steer=steerRight-steerLeft;
		accel=autoboxCompensatedAnalogAccel;
		isIntentionToAccelerate = (accel>0.0f && 0.0f==brake && 0.0f==handbrake && PxVehicleGearsData::eNEUTRAL != currentGear);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataControlInputs(accel,brake,handbrake,steerLeft,steerRight);
#endif
	}

	//Compute the wheels that are disabled or enabled.
	bool activeWheelStates[PX_MAX_NB_WHEELS];
	PxMemSet(activeWheelStates, 0, sizeof(bool)*PX_MAX_NB_WHEELS);
	for(PxU32 i=0;i<numWheels4;i++)
	{
		computeWheelActiveStates(4*i, vehDriveNW->mWheelsSimData.mActiveWheelsBitmapBuffer, &activeWheelStates[4*i]);
	}

	//Contribution of each driven wheel to average wheel speed at clutch.
	//For NW drive equal torque split is supported.
	const PxVehicleDifferentialNWData diffData=driveSimData.getDiffData();
	const PxF32 invNumDrivenWheels=diffData.mInvNbDrivenWheels;
	PxF32 aveWheelSpeedContributions[PX_MAX_NB_WHEELS];
	PxMemSet(aveWheelSpeedContributions, 0, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		aveWheelSpeedContributions[i] = diffData.getIsDrivenWheel(i) ? invNumDrivenWheels : 0;
	}
#if PX_DEBUG_VEHICLE_ON
	updateGraphDataClutchSlipNW(numWheels4,wheels4DynDatas,aveWheelSpeedContributions,driveDynData.getEngineRotationSpeed(),G);
#endif

	//Compute a per-wheel accelerator pedal value.
	bool isAccelApplied[PX_MAX_NB_WHEELS];
	PxMemSet(isAccelApplied, 0, sizeof(bool)*PX_MAX_NB_WHEELS);
	if(isIntentionToAccelerate)
	{
		PX_ASSERT(accel>0);
		for(PxU32 i=0;i<numWheels4;i++)
		{
			computeIsAccelApplied(&aveWheelSpeedContributions[4*i],&isAccelApplied[4*i]);
		}
	}

	//Steer angles.
	PxF32 steerAngles[PX_MAX_NB_WHEELS];
	PxMemSet(steerAngles, 0, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		const PxVehicleWheelData& wheelData=vehDriveNW->mWheelsSimData.getWheelData(i);
		const PxF32 steerGain=wheelData.mMaxSteer;
		const PxF32 toe=wheelData.mToeAngle;
		steerAngles[i]=steerGain*steer + toe;
	}

	//Diff torque ratios needed (how we split the torque between the drive wheels).
	//The sum of the torque ratios is always 1.0f.
	//The drive torque delivered to each wheel is the total available drive torque multiplied by the 
	//diff torque ratio for each wheel.
	PxF32 diffTorqueRatios[PX_MAX_NB_WHEELS];
	PxMemSet(diffTorqueRatios, 0, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		diffTorqueRatios[i] = diffData.getIsDrivenWheel(i) ? invNumDrivenWheels : 0.0f;
	}

	END_TIMER(TIMER_COMPONENTS_UPDATE);
	START_TIMER(TIMER_ADMIN);

	//Store the susp line raycast data.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		storeRaycasts(wheels4DynDatas[i], &wheelQueryResults[4*i]);
	}

	//Ready to do the update.
	PxVec3 carChassisLinVelOrig=carChassisLinVel;
	PxVec3 carChassisAngVelOrig=carChassisAngVel;
	const PxU32 numSubSteps=computeNumberOfSubsteps(vehDriveNW->mWheelsSimData,carChassisLinVel,carChassisTransform,gForward);
	const PxF32 timeFraction=1.0f/(1.0f*numSubSteps);
	const PxF32 subTimestep=timestep*timeFraction;
	const PxF32 recipSubTimeStep=1.0f/subTimestep;
	const PxF32 recipTimestep=1.0f/timestep;
	const PxF32 minLongSlipDenominator=vehDriveNW->mWheelsSimData.mMinLongSlipDenominator;
	ProcessSuspWheelTireConstData constData={timeFraction, subTimestep, recipSubTimeStep, gravity, gravityMagnitude, recipGravityMagnitude, false, minLongSlipDenominator, vehActor, &drivableSurfaceToTireFrictionPairs};
	
	END_TIMER(TIMER_ADMIN);

	for(PxU32 k=0;k<numSubSteps;k++)
	{
		//Set the force and torque for the current update to zero.
		PxVec3 chassisForce(0,0,0);
		PxVec3 chassisTorque(0,0,0);

		START_TIMER(TIMER_COMPONENTS_UPDATE);

		PxF32 brakeTorques[PX_MAX_NB_WHEELS];
		bool isBrakeApplied[PX_MAX_NB_WHEELS];
		PxMemSet(brakeTorques, 0, sizeof(PxF32)*PX_MAX_NB_WHEELS);
		PxMemSet(isBrakeApplied, false, sizeof(bool)*PX_MAX_NB_WHEELS);
		for(PxU32 i=0;i<numWheels4;i++)
		{
			computeBrakeAndHandBrakeTorques(&wheels4SimDatas[i].getWheelData(0),wheels4DynDatas[i].mWheelSpeeds,brake,handbrake,&brakeTorques[4*i],&isBrakeApplied[4*i]);
		}

		END_TIMER(TIMER_COMPONENTS_UPDATE);
		START_TIMER(TIMER_WHEELS);

		ProcessSuspWheelTireOutputData outputData[PX_MAX_NB_SUSPWHEELTIRE4];
		for(PxU32 i=0;i<numWheels4;i++)
		{
			ProcessSuspWheelTireInputData inputData=
			{
				isIntentionToAccelerate, &isAccelApplied[4*i], &isBrakeApplied[4*i], &steerAngles[4*i], &activeWheelStates[4*i],
				carChassisTransform, carChassisLinVel, carChassisAngVel,
				&wheels4SimDatas[i], &wheels4DynDatas[i], &tires4ForceCalculators[i], &tireLoadFilterData, numActiveWheelsPerBlock4[i]
			};
			processSuspTireWheels(4*i, constData, inputData, outputData[i]); 
			updateLowSpeedTimers(outputData[i].newLowForwardSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowForwardSpeedTimers);
			updateLowSpeedTimers(outputData[i].newLowSideSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowSideSpeedTimers);
			updateJounces(outputData[i].jounces, (PxF32*)inputData.vehWheels4DynData->mJounces);
			if((numSubSteps-1) == k)
			{
				updateCachedHitData(outputData[i].cachedHitCounts, outputData[i].cachedHitPlanes, outputData[i].cachedHitDistances, outputData[i].cachedFrictionMultipliers, &wheels4DynDatas[i]);
			}
			chassisForce+=outputData[i].chassisForce;
			chassisTorque+=outputData[i].chassisTorque;
			if(0 == k)
			{
				wheels4DynDatas[i].mVehicleConstraints->mData=outputData[i].vehConstraintData;
			}
			storeSuspWheelTireResults(outputData[i], inputData.steerAngles, &wheelQueryResults[4*i], numActiveWheelsPerBlock4[i]);
			storeHitActorForces(outputData[i], &vehicleConcurrentUpdates.concurrentWheelUpdates[4*i], numActiveWheelsPerBlock4[i]);
		}

		//Store the tire torques in a single array.
		PxF32 tireTorques[PX_MAX_NB_WHEELS];
		for(PxU32 i=0;i<numWheels4;i++)
		{
			tireTorques[4*i+0]=outputData[i].tireTorques[0];
			tireTorques[4*i+1]=outputData[i].tireTorques[1];
			tireTorques[4*i+2]=outputData[i].tireTorques[2];
			tireTorques[4*i+3]=outputData[i].tireTorques[3];
		}

		END_TIMER(TIMER_WHEELS);
		START_TIMER(TIMER_INTERNAL_DYNAMICS_SOLVER);

		PxF32 engineDriveTorque;
		{
			const PxVehicleEngineData& engineData=driveSimData.getEngineData();
			const PxF32 engineOmega=driveDynData.getEngineRotationSpeed();
			engineDriveTorque=computeEngineDriveTorque(engineData,engineOmega,accel);
#if PX_DEBUG_VEHICLE_ON
			updateGraphDataEngineDriveTorque(engineDriveTorque);
#endif
		}

		PxF32 engineDampingRate;
		{
			const PxVehicleEngineData& engineData=driveSimData.getEngineData();
			engineDampingRate=computeEngineDampingRate(engineData,currentGear,accel);
		}

		//Update the wheel and engine speeds - (N+1)*(N+1) matrix coupling engine and wheels.
		ImplicitSolverInput implicitSolverInput=
		{
			subTimestep, 
			brake, handbrake,
			K, G,
			clutchAccuracyMode, clutchMaxIterations,
			engineDriveTorque, engineDampingRate,
			diffTorqueRatios, aveWheelSpeedContributions, 
			brakeTorques, isBrakeApplied, tireTorques,
			numWheels4, numActiveWheels,
			wheels4SimDatas, &driveSimData
		};
		ImplicitSolverOutput implicitSolverOutput=
		{
			wheels4DynDatas, &driveDynData
		};
		solveDriveNWInternalDynamicsEnginePlusDrivenWheels(implicitSolverInput, &implicitSolverOutput);

		END_TIMER(TIMER_INTERNAL_DYNAMICS_SOLVER);
		START_TIMER(TIMER_POSTUPDATE1);

		//Integrate wheel rotation angle (theta += omega*dt)
		for(PxU32 i=0;i<numWheels4;i++)
		{
			integrateWheelRotationAngles
				(subTimestep,
				 K,G,engineDriveTorque,
				 outputData[i].jounces,&diffTorqueRatios[4*i],outputData[i].forwardSpeeds,&isBrakeApplied[4*i],
				 driveSimData,wheels4SimDatas[i],
				 driveDynData,wheels4DynDatas[i]);
		}

		END_TIMER(TIMER_POSTUPDATE1);
		START_TIMER(TIMER_POSTUPDATE2);

		//Integrate the chassis velocity by applying the accumulated force and torque.
		integrateBody(inverseChassisMass,inverseInertia,chassisForce,chassisTorque,subTimestep,carChassisLinVel,carChassisAngVel,carChassisTransform);		
		
		END_TIMER(TIMER_POSTUPDATE2);
	}

	//Set the new chassis linear/angular velocity.
	if(!gApplyForces)
	{
		vehicleConcurrentUpdates.linearMomentumChange = carChassisLinVel;
		vehicleConcurrentUpdates.angularMomentumChange = carChassisAngVel;
	}
	else
	{
		//integration steps are: 
		//v = v0 + a*dt	(1)
		//x = x0 + v*dt	(2)
		//Sub (2) into (1.
		//x = x0 + v0*dt + a*dt*dt;
		//Rearrange for a
		//a = (x -x0 - v0*dt)/(dt*dt) = [(x-x0)/dt - v0/dt]
		//Rearrange again with v = (x-x0)/dt
		//a = (v - v0)/dt
		vehicleConcurrentUpdates.linearMomentumChange = (carChassisLinVel-carChassisLinVelOrig)*recipTimestep;
		vehicleConcurrentUpdates.angularMomentumChange = (carChassisAngVel-carChassisAngVelOrig)*recipTimestep;
	}

	START_TIMER(TIMER_POSTUPDATE3);

	//Pose the wheels from jounces, rotations angles, and steer angles.
	PxTransform localPoses0[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
	computeWheelLocalPoses(wheels4SimDatas[0],wheels4DynDatas[0],&wheelQueryResults[4*0],numActiveWheelsPerBlock4[0],carChassisCMLocalPose,localPoses0);
	wheelQueryResults[4*0 + 0].localPose = localPoses0[0];
	wheelQueryResults[4*0 + 1].localPose = localPoses0[1];
	wheelQueryResults[4*0 + 2].localPose = localPoses0[2];
	wheelQueryResults[4*0 + 3].localPose = localPoses0[3];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 0].localPose = localPoses0[0];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 1].localPose = localPoses0[1];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 2].localPose = localPoses0[2];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 3].localPose = localPoses0[3];
	for(PxU32 i=1;i<numWheels4;i++)
	{
		PxTransform localPoses[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
		computeWheelLocalPoses(wheels4SimDatas[i],wheels4DynDatas[i],&wheelQueryResults[4*i],numActiveWheelsPerBlock4[i],carChassisCMLocalPose,localPoses);
		wheelQueryResults[4*i + 0].localPose = localPoses[0];
		wheelQueryResults[4*i + 1].localPose = localPoses[1];
		wheelQueryResults[4*i + 2].localPose = localPoses[2];
		wheelQueryResults[4*i + 3].localPose = localPoses[3];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 0].localPose = localPoses[0];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 1].localPose = localPoses[1];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 2].localPose = localPoses[2];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 3].localPose = localPoses[3];
	}

	if(vehWheelQueryResults && vehWheelQueryResults->wheelQueryResults)
	{
		PxMemCopy(vehWheelQueryResults->wheelQueryResults, wheelQueryResults, sizeof(PxWheelQueryResult)*numActiveWheels);
	}

	if(vehConcurrentUpdates)
	{
		//Copy across to input data structure so that writes can be applied later.
		PxMemCopy(vehConcurrentUpdates->concurrentWheelUpdates, vehicleConcurrentUpdates.concurrentWheelUpdates, sizeof(PxVehicleWheelConcurrentUpdateData)*numActiveWheels);
		vehConcurrentUpdates->linearMomentumChange = vehicleConcurrentUpdates.linearMomentumChange;
		vehConcurrentUpdates->angularMomentumChange = vehicleConcurrentUpdates.angularMomentumChange;
		vehConcurrentUpdates->staySleeping = vehicleConcurrentUpdates.staySleeping;
		vehConcurrentUpdates->wakeup = vehicleConcurrentUpdates.wakeup;
	}
	else
	{
		//Apply the writes immediately.
		PxVehicleWheels* vehWheels[1]={vehDriveNW};
		PxVehiclePostUpdates(&vehicleConcurrentUpdates, 1, vehWheels);
	}

	END_TIMER(TIMER_POSTUPDATE3);
}

void PxVehicleUpdate::updateTank
(const PxF32 timestep, 
 const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude, 
 const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
 PxVehicleDriveTank* vehDriveTank, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates)
{
	PX_SIMD_GUARD; // denorm exception in transformInertiaTensor()
	
	PX_CHECK_AND_RETURN(
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_ACCEL]>-0.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_ACCEL]<1.01f, 
		"Illegal tank control value - accel must be in range (0,1)" );
	PX_CHECK_AND_RETURN(
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_LEFT]>-0.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_LEFT]<1.01f, 
		"Illegal tank control value - left brake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_RIGHT]>-0.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_RIGHT]<1.01f, 
		"Illegal tank control right value - right brake must be in range (0,1)");
	PX_CHECK_AND_RETURN(
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT]>-1.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT]<1.01f, 
		"Illegal tank control value - left thrust must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT]>-1.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT]<1.01f, 
		"Illegal tank control value - right thrust must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		PxVehicleDriveTankControlModel::eSPECIAL==vehDriveTank->mDriveModel ||
		(vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT]>-0.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT]<1.01f), 
		"Illegal tank control value - left thrust must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		PxVehicleDriveTankControlModel::eSPECIAL==vehDriveTank->mDriveModel ||
		(vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT]>-0.01f && 
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT]<1.01f), 
		"Illegal tank control value - right thrust must be in range (-1,1)");
	PX_CHECK_AND_RETURN(
		PxVehicleDriveTankControlModel::eSPECIAL==vehDriveTank->mDriveModel ||
		0.0f==
			vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT]*
			vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_LEFT],
		"Illegal tank control value - thrust left and brake left simultaneously non-zero in standard drive mode");
	PX_CHECK_AND_RETURN(
		PxVehicleDriveTankControlModel::eSPECIAL==vehDriveTank->mDriveModel ||
		0.0f==
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT]*
		vehDriveTank->mDriveDynData.mControlAnalogVals[PxVehicleDriveTankControl::eANALOG_INPUT_BRAKE_RIGHT],
		"Illegal tank control value - thrust right and brake right simultaneously non-zero in standard drive mode");
	PX_CHECK_AND_RETURN(
		!(vehDriveTank->getRigidDynamicActor()->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC),
		"Attempting to update a tank with a kinematic actor - this isn't allowed");
	PX_CHECK_AND_RETURN(
		NULL==vehWheelQueryResults || vehWheelQueryResults->nbWheelQueryResults >= vehDriveTank->mWheelsSimData.getNbWheels(), 
		"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");

#ifdef PX_CHECKED
	{
		PxVec3 fl=vehDriveTank->mWheelsSimData.getWheelCentreOffset(PxVehicleDriveTankWheelOrder::eFRONT_LEFT);
		PxVec3 fr=vehDriveTank->mWheelsSimData.getWheelCentreOffset(PxVehicleDriveTankWheelOrder::eFRONT_RIGHT);
		const PxVec3 right=gRight;
		const PxF32 s0=computeSign((fr-fl).dot(right));
		for(PxU32 i=PxVehicleDriveTankWheelOrder::e1ST_FROM_FRONT_LEFT;i<vehDriveTank->mWheelsSimData.getNbWheels();i+=2)
		{
			PxVec3 rl=vehDriveTank->mWheelsSimData.getWheelCentreOffset(i);
			PxVec3 rr=vehDriveTank->mWheelsSimData.getWheelCentreOffset(i+1);
			const PxF32 t0=computeSign((rr-rl).dot(right));
			PX_CHECK_AND_RETURN(s0==t0 || 0==s0 || 0==t0, "Tank wheels must be ordered with odd wheels on one side and even wheels on the other side");
		}
	}
#endif


#if PX_DEBUG_VEHICLE_ON
	for(PxU32 i=0;i<vehDriveTank->mWheelsSimData.mNbWheels4;i++)
	{
		updateGraphDataInternalWheelDynamics(4*i,vehDriveTank->mWheelsDynData.mWheels4DynData[i].mWheelSpeeds);
	}
	updateGraphDataInternalEngineDynamics(vehDriveTank->mDriveDynData.getEngineRotationSpeed());
#endif

	//Unpack the tank simulation and instanced dynamics components.
	const PxVehicleWheels4SimData* wheels4SimDatas=vehDriveTank->mWheelsSimData.mWheels4SimData;
	const PxVehicleTireLoadFilterData& tireLoadFilterData=vehDriveTank->mWheelsSimData.mNormalisedLoadFilter;
	PxVehicleWheels4DynData* wheels4DynDatas=vehDriveTank->mWheelsDynData.mWheels4DynData;
	const PxU32 numWheels4=vehDriveTank->mWheelsSimData.mNbWheels4;
	const PxU32 numActiveWheels=vehDriveTank->mWheelsSimData.mNbActiveWheels;
	const PxU32 numActiveWheelsInLast4=4-(4*numWheels4-numActiveWheels);
	const PxVehicleDriveSimData driveSimData=vehDriveTank->mDriveSimData;
	PxVehicleDriveDynData& driveDynData=vehDriveTank->mDriveDynData;
	PxRigidDynamic* vehActor=vehDriveTank->mActor;

	//We need to store that data we are going to write to actors so we can do this at the end in one go with fewer write locks.
	PxVehicleWheelConcurrentUpdateData wheelConcurrentUpdates[PX_MAX_NB_WHEELS];
	PxVehicleConcurrentUpdateData vehicleConcurrentUpdates;
	vehicleConcurrentUpdates.nbConcurrentWheelUpdates = numActiveWheels;
	vehicleConcurrentUpdates.concurrentWheelUpdates = wheelConcurrentUpdates;

	//Test if a non-zero drive torque was applied or if a non-zero steer angle was applied.
	bool finiteInputApplied=false;
	if(0!=driveDynData.getAnalogInput(PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_LEFT) || 
		0!=driveDynData.getAnalogInput(PxVehicleDriveTankControl::eANALOG_INPUT_THRUST_RIGHT) ||
		0!=driveDynData.getAnalogInput(PxVehicleDriveTankControl::eANALOG_INPUT_ACCEL) ||
		driveDynData.getGearDown() || driveDynData.getGearUp())
	{
		finiteInputApplied=true;
	}

	//Awake or sleep.
	{
		if(vehActor->isSleeping())
		{
			if(finiteInputApplied)
			{
				//Driving inputs so we need the actor to start moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else if(isOnDynamicActor(vehDriveTank->mWheelsSimData, vehDriveTank->mWheelsDynData))
			{
				//Driving on dynamic so we need to keep moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else
			{
				//No driving inputs and the actor is asleep.
				//Set internal dynamics to sleep.
				setInternalDynamicsToZero(vehDriveTank);
				if(vehConcurrentUpdates) vehConcurrentUpdates->staySleeping = true;
				return;
			}
		}
	}

	//In each block of 4 wheels record how many wheels are active.
	PxU32 numActiveWheelsPerBlock4[PX_MAX_NB_SUSPWHEELTIRE4]={0,0,0,0,0};
	numActiveWheelsPerBlock4[0]=PxMin(numActiveWheels,(PxU32)4);
	for(PxU32 i=1;i<numWheels4-1;i++)
	{
		numActiveWheelsPerBlock4[i]=4;
	}
	numActiveWheelsPerBlock4[numWheels4-1]=numActiveWheelsInLast4;
	PX_ASSERT(numActiveWheels == numActiveWheelsPerBlock4[0] + numActiveWheelsPerBlock4[1] + numActiveWheelsPerBlock4[2] + numActiveWheelsPerBlock4[3] + numActiveWheelsPerBlock4[4]); 

	//Organise the shader data in blocks of 4.
	PxVehicleTireForceCalculator4 tires4ForceCalculators[PX_MAX_NB_SUSPWHEELTIRE4];
	for(PxU32 i=0;i<numWheels4;i++)
	{
		tires4ForceCalculators[i].mShaderData[0]=vehDriveTank->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+0];
		tires4ForceCalculators[i].mShaderData[1]=vehDriveTank->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+1];
		tires4ForceCalculators[i].mShaderData[2]=vehDriveTank->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+2];
		tires4ForceCalculators[i].mShaderData[3]=vehDriveTank->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+3];
		tires4ForceCalculators[i].mShader=vehDriveTank->mWheelsDynData.mTireForceCalculators->mShader;
	}

	//Mark the suspension/tire constraints as dirty to force them to be updated in the sdk.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		wheels4DynDatas[i].getVehicletConstraintShader().mConstraint->markDirty();
	}

	//Need to store report data to pose the wheels.
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];


	//Center of mass local pose.
	PxTransform carChassisCMLocalPose;
	//Compute the transform of the center of mass.
	PxTransform origCarChassisTransform;
	PxTransform carChassisTransform;
	//Inverse mass and inertia to apply the tire/suspension forces as impulses.
	PxF32 inverseChassisMass;
	PxVec3 inverseInertia;
	//Linear and angular velocity.
	PxVec3 carChassisLinVel;
	PxVec3 carChassisAngVel;
	{
		carChassisCMLocalPose = vehActor->getCMassLocalPose();
		origCarChassisTransform = vehActor->getGlobalPose().transform(vehActor->getCMassLocalPose());
		carChassisTransform = origCarChassisTransform;
		const PxF32 chassisMass = vehActor->getMass();
		inverseChassisMass = 1.0f/chassisMass;
		inverseInertia = vehActor->getMassSpaceInvInertiaTensor();
		carChassisLinVel = vehActor->getLinearVelocity();
		carChassisAngVel = vehActor->getAngularVelocity();
	}

	//Retrieve control values from vehicle controls.
	PxF32 accel,brakeLeft,brakeRight,thrustLeft,thrustRight;
	{
		getTankControlValues(driveDynData,accel,brakeLeft,brakeRight,thrustLeft,thrustRight);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataControlInputs(accel,brakeLeft,brakeRight,thrustLeft,thrustRight);
#endif
	}

	//Update the auto-box and decide whether to change gear up or down.
	//If the tank is supposed to turn sharply don't process the auto-box.
	bool useAutoGears;
	if(vehDriveTank->getDriveModel()==PxVehicleDriveTankControlModel::eSPECIAL)
	{
		useAutoGears = driveDynData.getUseAutoGears() ? ((((thrustRight*thrustLeft) >= 0.0f) || (0.0f==thrustLeft && 0.0f==thrustRight)) ? true : false) : false;
	}
	else
	{
		useAutoGears = driveDynData.getUseAutoGears() ? (thrustRight*brakeLeft>0 || thrustLeft*brakeRight>0 ? false : true) : false; 
	}
	if(useAutoGears)
	{
		processAutoBox(PxVehicleDriveTankControl::eANALOG_INPUT_ACCEL,timestep,driveSimData,driveDynData);
	}

	//Process gear-up/gear-down commands.
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		processGears(timestep,gearsData,driveDynData);
	}

	//Clutch strength;
	PxF32 K;
	{
		const PxVehicleClutchData& clutchData=driveSimData.getClutchData();
		const PxU32 currentGear=driveDynData.getCurrentGear();
		K=computeClutchStrength(clutchData, currentGear);
	}

	//Gear ratio.
	PxF32 G;
	PxU32 currentGear;
	{
		const PxVehicleGearsData& gearsData=driveSimData.getGearsData();
		currentGear=driveDynData.getCurrentGear();
		G=computeGearRatio(gearsData,currentGear);
#if PX_DEBUG_VEHICLE_ON
		updateGraphDataGearRatio(G);
#endif
	}

	bool isIntentionToAccelerate;
	{
		const PxF32 thrustLeftAbs=PxAbs(thrustLeft);
		const PxF32 thrustRightAbs=PxAbs(thrustRight);
		isIntentionToAccelerate = (accel*(thrustLeftAbs+thrustRightAbs)>0 && PxVehicleGearsData::eNEUTRAL != currentGear);
	}


	//Compute the wheels that are enabled/disabled.
	bool activeWheelStates[PX_MAX_NB_WHEELS];
	PxMemZero(activeWheelStates, sizeof(bool)*PX_MAX_NB_WHEELS);
	for(PxU32 i=0;i<numWheels4;i++)
	{
		computeWheelActiveStates(4*i, vehDriveTank->mWheelsSimData.mActiveWheelsBitmapBuffer, &activeWheelStates[4*i]);
	}

	//Set up contribution of each wheel to the average wheel speed at the clutch 
	//Set up the torque ratio delivered by the diff to each wheel.
	//Set the sign of the gearing applied to the left and right wheels.
	PxF32 aveWheelSpeedContributions[PX_MAX_NB_WHEELS];
	PxF32 diffTorqueRatios[PX_MAX_NB_WHEELS];
	PxF32 wheelGearings[PX_MAX_NB_WHEELS];
	PxMemZero(aveWheelSpeedContributions, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	PxMemZero(diffTorqueRatios, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	PxMemZero(wheelGearings, sizeof(PxF32)*PX_MAX_NB_WHEELS);
	computeTankDiff
		(thrustLeft, thrustRight, 
		 numActiveWheels, activeWheelStates,
		 aveWheelSpeedContributions, diffTorqueRatios, wheelGearings);

	//Compute an accelerator pedal value per wheel.
	bool isAccelApplied[PX_MAX_NB_WHEELS];
	PxMemZero(isAccelApplied, sizeof(bool)*PX_MAX_NB_WHEELS);
	if(isIntentionToAccelerate)
	{
		PX_ASSERT(accel>0);
		for(PxU32 i=0;i<numWheels4;i++)
		{
			computeIsAccelApplied(&aveWheelSpeedContributions[4*i], &isAccelApplied[4*i]);
		}
	}


#if PX_DEBUG_VEHICLE_ON
	updateGraphDataClutchSlip(wheels4DynDatas[0].mWheelSpeeds,aveWheelSpeedContributions,driveDynData.getEngineRotationSpeed(),G);
#endif

	//Ackermann-corrected steer angles 
	//For tanks this is always zero because they turn by torque delivery rather than a steering mechanism.
	PxF32 steerAngles[PX_MAX_NB_WHEELS];
	PxMemZero(steerAngles, sizeof(PxF32)*PX_MAX_NB_WHEELS);

	//Store the susp line raycast data.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		storeRaycasts(wheels4DynDatas[i], &wheelQueryResults[4*i]);
	}

	//Ready to do the update.
	PxVec3 carChassisLinVelOrig=carChassisLinVel;
	PxVec3 carChassisAngVelOrig=carChassisAngVel;
	const PxU32 numSubSteps=computeNumberOfSubsteps(vehDriveTank->mWheelsSimData,carChassisLinVel,carChassisTransform,gForward);
	const PxF32 timeFraction=1.0f/(1.0f*numSubSteps);
	const PxF32 subTimestep=timestep*timeFraction;
	const PxF32 recipSubTimeStep=1.0f/subTimestep;
	const PxF32 recipTimestep=1.0f/timestep;
	const PxF32 minLongSlipDenominator=vehDriveTank->mWheelsSimData.mMinLongSlipDenominator;
	ProcessSuspWheelTireConstData constData={timeFraction, subTimestep, recipSubTimeStep, gravity, gravityMagnitude, recipGravityMagnitude, true, minLongSlipDenominator, vehActor, &drivableSurfaceToTireFrictionPairs};

	for(PxU32 k=0;k<numSubSteps;k++)
	{
		//Set the force and torque for the current update to zero.
		PxVec3 chassisForce(0,0,0);
		PxVec3 chassisTorque(0,0,0);

		//Compute the brake torques.
		PxF32 brakeTorques[PX_MAX_NB_WHEELS];
		bool isBrakeApplied[PX_MAX_NB_WHEELS];
		PxMemZero(brakeTorques, sizeof(PxF32)*PX_MAX_NB_WHEELS);
		PxMemZero(isBrakeApplied, sizeof(bool)*PX_MAX_NB_WHEELS);
		for(PxU32 i=0;i<numWheels4;i++)
		{
			computeTankBrakeTorques
				(&wheels4SimDatas[i].getWheelData(0),wheels4DynDatas[i].mWheelSpeeds,brakeLeft,brakeRight,
				&brakeTorques[i*4],&isBrakeApplied[i*4]);
		}

		//Compute jounces, slips, tire forces, suspension forces etc.
		ProcessSuspWheelTireOutputData outputData[PX_MAX_NB_SUSPWHEELTIRE4];
		for(PxU32 i=0;i<numWheels4;i++)
		{
			ProcessSuspWheelTireInputData inputData=
			{
				isIntentionToAccelerate, &isAccelApplied[i*4], &isBrakeApplied[i*4], &steerAngles[i*4], &activeWheelStates[4*i],
				carChassisTransform, carChassisLinVel, carChassisAngVel,
				&wheels4SimDatas[i], &wheels4DynDatas[i], &tires4ForceCalculators[i], &tireLoadFilterData, numActiveWheelsPerBlock4[i],
			};
			processSuspTireWheels(i*4, constData, inputData, outputData[i]);
			updateLowSpeedTimers(outputData[i].newLowForwardSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowForwardSpeedTimers);
			updateLowSpeedTimers(outputData[i].newLowSideSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowSideSpeedTimers);
			updateJounces(outputData[i].jounces, (PxF32*)inputData.vehWheels4DynData->mJounces);
			if((numSubSteps-1) == k)
			{
				updateCachedHitData(outputData[i].cachedHitCounts, outputData[i].cachedHitPlanes, outputData[i].cachedHitDistances, outputData[i].cachedFrictionMultipliers, &wheels4DynDatas[i]);
			}
			chassisForce+=outputData[i].chassisForce;
			chassisTorque+=outputData[i].chassisTorque;	
			if(0 == k)
			{
				wheels4DynDatas[i].mVehicleConstraints->mData=outputData[i].vehConstraintData;
			}
			storeSuspWheelTireResults(outputData[i], inputData.steerAngles, &wheelQueryResults[4*i], numActiveWheelsPerBlock4[i]);
			storeHitActorForces(outputData[i], &vehicleConcurrentUpdates.concurrentWheelUpdates[4*i], numActiveWheelsPerBlock4[i]);
		}

		//Copy the tire torques to a single array.
		PxF32 tireTorques[PX_MAX_NB_WHEELS];
		for(PxU32 i=0;i<numWheels4;i++)
		{
			tireTorques[4*i+0]=outputData[i].tireTorques[0];
			tireTorques[4*i+1]=outputData[i].tireTorques[1];
			tireTorques[4*i+2]=outputData[i].tireTorques[2];
			tireTorques[4*i+3]=outputData[i].tireTorques[3];
		}

		PxF32 engineDriveTorque;
		{
			const PxVehicleEngineData& engineData=driveSimData.getEngineData();
			const PxF32 engineOmega=driveDynData.getEngineRotationSpeed();
			engineDriveTorque=computeEngineDriveTorque(engineData,engineOmega,accel);
#if PX_DEBUG_VEHICLE_ON
			updateGraphDataEngineDriveTorque(engineDriveTorque);
#endif
		}

		PxF32 engineDampingRate;
		{
			const PxVehicleEngineData& engineData=driveSimData.getEngineData();
			engineDampingRate=computeEngineDampingRate(engineData,currentGear,accel);
		}

		//Update the wheel and engine speeds - 5x5 matrix coupling engine and wheels.
		ImplicitSolverInput implicitSolverInput = 
		{
			subTimestep, 
			0.0f, 0.0f,
			K, G,
			PxVehicleClutchAccuracyMode::eBEST_POSSIBLE, 0,
			engineDriveTorque, engineDampingRate,
			diffTorqueRatios, aveWheelSpeedContributions, 
			brakeTorques, isBrakeApplied, tireTorques,
			numWheels4, numActiveWheels,
			wheels4SimDatas, &driveSimData
		};
		ImplicitSolverOutput implicitSolverOutput = 
		{
			wheels4DynDatas, &driveDynData
		};
		solveTankInternaDynamicsEnginePlusDrivenWheels(implicitSolverInput, activeWheelStates, wheelGearings, &implicitSolverOutput);

		//Integrate wheel rotation angle (theta += omega*dt)
		for(PxU32 i=0;i<numWheels4;i++)
		{
			integrateWheelRotationAngles
				(subTimestep,
				K,G,engineDriveTorque,
				outputData[i].jounces,diffTorqueRatios,outputData[i].forwardSpeeds,isBrakeApplied,
				driveSimData,wheels4SimDatas[i],
				driveDynData,wheels4DynDatas[i]);
		}

		//Integrate the chassis velocity by applying the accumulated force and torque.
		integrateBody(inverseChassisMass,inverseInertia,chassisForce,chassisTorque,subTimestep,carChassisLinVel,carChassisAngVel,carChassisTransform);
	}

	//Set the new chassis linear/angular velocity.
	if(!gApplyForces)
	{
		vehicleConcurrentUpdates.linearMomentumChange = carChassisLinVel;
		vehicleConcurrentUpdates.angularMomentumChange = carChassisAngVel;
	}
	else
	{
		//integration steps are: 
		//v = v0 + a*dt	(1)
		//x = x0 + v*dt	(2)
		//Sub (2) into (1.
		//x = x0 + v0*dt + a*dt*dt;
		//Rearrange for a
		//a = (x -x0 - v0*dt)/(dt*dt) = [(x-x0)/dt - v0/dt]
		//Rearrange again with v = (x-x0)/dt
		//a = (v - v0)/dt
		vehicleConcurrentUpdates.linearMomentumChange = (carChassisLinVel-carChassisLinVelOrig)*recipTimestep;
		vehicleConcurrentUpdates.angularMomentumChange = (carChassisAngVel-carChassisAngVelOrig)*recipTimestep;
	}

	//Pose the wheels from jounces, rotations angles, and steer angles.
	PxTransform localPoses0[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
	computeWheelLocalPoses(wheels4SimDatas[0],wheels4DynDatas[0],&wheelQueryResults[4*0],numActiveWheelsPerBlock4[0],carChassisCMLocalPose,localPoses0);
	wheelQueryResults[4*0 + 0].localPose = localPoses0[0];
	wheelQueryResults[4*0 + 1].localPose = localPoses0[1];
	wheelQueryResults[4*0 + 2].localPose = localPoses0[2];
	wheelQueryResults[4*0 + 3].localPose = localPoses0[3];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 0].localPose = localPoses0[0];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 1].localPose = localPoses0[1];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 2].localPose = localPoses0[2];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 3].localPose = localPoses0[3];
	for(PxU32 i=1;i<numWheels4;i++)
	{
		PxTransform localPoses[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
		computeWheelLocalPoses(wheels4SimDatas[i],wheels4DynDatas[i],&wheelQueryResults[4*i],numActiveWheelsPerBlock4[i],carChassisCMLocalPose,localPoses);
		wheelQueryResults[4*i + 0].localPose = localPoses[0];
		wheelQueryResults[4*i + 1].localPose = localPoses[1];
		wheelQueryResults[4*i + 2].localPose = localPoses[2];
		wheelQueryResults[4*i + 3].localPose = localPoses[3];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 0].localPose = localPoses[0];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 1].localPose = localPoses[1];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 2].localPose = localPoses[2];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 3].localPose = localPoses[3];
	}

	if(vehWheelQueryResults && vehWheelQueryResults->wheelQueryResults)
	{
		PxMemCopy(vehWheelQueryResults->wheelQueryResults, wheelQueryResults, sizeof(PxWheelQueryResult)*numActiveWheels);
	}

	if(vehConcurrentUpdates)
	{
		//Copy across to input data structure so that writes can be applied later.
		PxMemCopy(vehConcurrentUpdates->concurrentWheelUpdates, vehicleConcurrentUpdates.concurrentWheelUpdates, sizeof(PxVehicleWheelConcurrentUpdateData)*numActiveWheels);
		vehConcurrentUpdates->linearMomentumChange = vehicleConcurrentUpdates.linearMomentumChange;
		vehConcurrentUpdates->angularMomentumChange = vehicleConcurrentUpdates.angularMomentumChange;
		vehConcurrentUpdates->staySleeping = vehicleConcurrentUpdates.staySleeping;
		vehConcurrentUpdates->wakeup = vehicleConcurrentUpdates.wakeup;
	}
	else
	{
		//Apply the writes immediately.
		PxVehicleWheels* vehWheels[1]={vehDriveTank};
		PxVehiclePostUpdates(&vehicleConcurrentUpdates, 1, vehWheels);
	}
}

void PxVehicleUpdate::updateNoDrive
(const PxF32 timestep, 
 const PxVec3& gravity, const PxF32 gravityMagnitude, const PxF32 recipGravityMagnitude,
 const PxVehicleDrivableSurfaceToTireFrictionPairs& drivableSurfaceToTireFrictionPairs,
 PxVehicleNoDrive* vehNoDrive, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleConcurrentUpdateData* vehConcurrentUpdates)
{
	PX_SIMD_GUARD; // denorm exception in transformInertiaTensor() on osx

	PX_CHECK_AND_RETURN(
		!(vehNoDrive->getRigidDynamicActor()->getRigidDynamicFlags() & PxRigidDynamicFlag::eKINEMATIC),
		"Attempting to update a PxVehicleNoDrive with a kinematic actor - this isn't allowed");

	PX_CHECK_AND_RETURN(
		NULL==vehWheelQueryResults || vehWheelQueryResults->nbWheelQueryResults >= vehNoDrive->mWheelsSimData.getNbWheels(), 
		"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");

#ifdef PX_CHECKED
	for(PxU32 i=0;i<vehNoDrive->mWheelsSimData.getNbWheels();i++)
	{
		PX_CHECK_AND_RETURN( 
			!vehNoDrive->mWheelsSimData.getIsWheelDisabled(i) || 0==vehNoDrive->getDriveTorque(i),
			"Disabled wheels should have zero drive torque applied to them.");
	}
#endif

#if PX_DEBUG_VEHICLE_ON
	for(PxU32 i=0;i<vehNoDrive->mWheelsSimData.mNbWheels4;i++)
	{
		updateGraphDataInternalWheelDynamics(4*i,vehNoDrive->mWheelsDynData.mWheels4DynData[i].mWheelSpeeds);
	}
#endif

	//Unpack the tank simulation and instanced dynamics components.
	const PxVehicleWheels4SimData* wheels4SimDatas=vehNoDrive->mWheelsSimData.mWheels4SimData;
	const PxVehicleTireLoadFilterData& tireLoadFilterData=vehNoDrive->mWheelsSimData.mNormalisedLoadFilter;
	PxVehicleWheels4DynData* wheels4DynDatas=vehNoDrive->mWheelsDynData.mWheels4DynData;
	const PxU32 numWheels4=vehNoDrive->mWheelsSimData.mNbWheels4;
	const PxU32 numActiveWheels=vehNoDrive->mWheelsSimData.mNbActiveWheels;
	const PxU32 numActiveWheelsInLast4=4-(4*numWheels4-numActiveWheels);
	PxRigidDynamic* vehActor=vehNoDrive->mActor;

	//We need to store that data we are going to write to actors so we can do this at the end in one go with fewer write locks.
	PxVehicleWheelConcurrentUpdateData wheelConcurrentUpdates[PX_MAX_NB_WHEELS];
	PxVehicleConcurrentUpdateData vehicleConcurrentUpdates;
	vehicleConcurrentUpdates.nbConcurrentWheelUpdates = numActiveWheels;
	vehicleConcurrentUpdates.concurrentWheelUpdates = wheelConcurrentUpdates;

	//Test if a non-zero drive torque was applied or if a non-zero steer angle was applied.
	bool finiteInputApplied=false;
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		if(vehNoDrive->getDriveTorque(i) != 0.0f)
		{
			finiteInputApplied=true;
			break;
		}
		if(vehNoDrive->getSteerAngle(i)!=0.0f)
		{
			finiteInputApplied=true;
			break;
		}
	}

	//Wake or sleep.
	{
		if(vehActor->isSleeping())
		{
			if(finiteInputApplied)
			{
				//Driving inputs so we need the actor to start moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else if(isOnDynamicActor(vehNoDrive->mWheelsSimData, vehNoDrive->mWheelsDynData))
			{
				//Driving on dynamic so we need to keep moving.
				vehicleConcurrentUpdates.wakeup = true;
			}
			else
			{
				//No driving inputs and the actor is asleep.
				//Set internal dynamics to sleep.
				setInternalDynamicsToZero(vehNoDrive);
				if(vehConcurrentUpdates) vehConcurrentUpdates->staySleeping = true;
				return;
			}
		}
	}

	//In each block of 4 wheels record how many wheels are active.
	PxU32 numActiveWheelsPerBlock4[PX_MAX_NB_SUSPWHEELTIRE4]={0,0,0,0,0};
	numActiveWheelsPerBlock4[0]=PxMin(numActiveWheels,(PxU32)4);
	for(PxU32 i=1;i<numWheels4-1;i++)
	{
		numActiveWheelsPerBlock4[i]=4;
	}
	numActiveWheelsPerBlock4[numWheels4-1]=numActiveWheelsInLast4;
	PX_ASSERT(numActiveWheels == numActiveWheelsPerBlock4[0] + numActiveWheelsPerBlock4[1] + numActiveWheelsPerBlock4[2] + numActiveWheelsPerBlock4[3] + numActiveWheelsPerBlock4[4]); 

	//Organise the shader data in blocks of 4.
	PxVehicleTireForceCalculator4 tires4ForceCalculators[PX_MAX_NB_SUSPWHEELTIRE4];
	for(PxU32 i=0;i<numWheels4;i++)
	{
		tires4ForceCalculators[i].mShaderData[0]=vehNoDrive->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+0];
		tires4ForceCalculators[i].mShaderData[1]=vehNoDrive->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+1];
		tires4ForceCalculators[i].mShaderData[2]=vehNoDrive->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+2];
		tires4ForceCalculators[i].mShaderData[3]=vehNoDrive->mWheelsDynData.mTireForceCalculators->mShaderData[4*i+3];
		tires4ForceCalculators[i].mShader=vehNoDrive->mWheelsDynData.mTireForceCalculators->mShader;
	}

	//Mark the suspension/tire constraints as dirty to force them to be updated in the sdk.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		wheels4DynDatas[i].getVehicletConstraintShader().mConstraint->markDirty();
	}

	//Need to store report data to pose the wheels.
	PxWheelQueryResult wheelQueryResults[PX_MAX_NB_WHEELS];

	//Center of mass local pose.
	PxTransform carChassisCMLocalPose;
	//Compute the transform of the center of mass.
	PxTransform origCarChassisTransform;
	PxTransform carChassisTransform;
	//Inverse mass and inertia to apply the tire/suspension forces as impulses.
	PxF32 inverseChassisMass;
	PxVec3 inverseInertia;
	//Linear and angular velocity.
	PxVec3 carChassisLinVel;
	PxVec3 carChassisAngVel;
	{
		carChassisCMLocalPose = vehActor->getCMassLocalPose();
		origCarChassisTransform = vehActor->getGlobalPose().transform(carChassisCMLocalPose);
		carChassisTransform = origCarChassisTransform;
		const PxF32 chassisMass = vehActor->getMass();
		inverseChassisMass = 1.0f/chassisMass;
		inverseInertia = vehActor->getMassSpaceInvInertiaTensor();
		carChassisLinVel = vehActor->getLinearVelocity();
		carChassisAngVel = vehActor->getAngularVelocity();
	}

	PxF32 maxAccel=0;
	PxF32 maxBrake=0;
	for(PxU32 i=0;i<numActiveWheels;i++)
	{
		maxAccel = PxMax(PxAbs(vehNoDrive->mDriveTorques[i]), maxAccel);
		maxBrake = PxMax(PxAbs(vehNoDrive->mBrakeTorques[i]), maxBrake);
	}
	const bool isIntentionToAccelerate = (maxAccel>0.0f && 0.0f==maxBrake);

	//Store the susp line raycast data.
	for(PxU32 i=0;i<numWheels4;i++)
	{
		storeRaycasts(wheels4DynDatas[i], &wheelQueryResults[4*i]);
	}

	//Ready to do the update.
	PxVec3 carChassisLinVelOrig=carChassisLinVel;
	PxVec3 carChassisAngVelOrig=carChassisAngVel;
	const PxU32 numSubSteps=computeNumberOfSubsteps(vehNoDrive->mWheelsSimData,carChassisLinVel,carChassisTransform,gForward);
	const PxF32 timeFraction=1.0f/(1.0f*numSubSteps);
	const PxF32 subTimestep=timestep*timeFraction;
	const PxF32 recipSubTimeStep=1.0f/subTimestep;
	const PxF32 recipTimestep=1.0f/timestep;
	const PxF32 minLongSlipDenominator=vehNoDrive->mWheelsSimData.mMinLongSlipDenominator;
	ProcessSuspWheelTireConstData constData={timeFraction, subTimestep, recipSubTimeStep, gravity, gravityMagnitude, recipGravityMagnitude, false, minLongSlipDenominator, vehActor, &drivableSurfaceToTireFrictionPairs};

	for(PxU32 k=0;k<numSubSteps;k++)
	{
		//Set the force and torque for the current update to zero.
		PxVec3 chassisForce(0,0,0);
		PxVec3 chassisTorque(0,0,0);

		for(PxU32 i=0;i<numWheels4;i++)
		{
			//Get the raw input torques.
			const PxF32* PX_RESTRICT rawBrakeTorques=&vehNoDrive->mBrakeTorques[4*i];
			const PxF32* PX_RESTRICT rawSteerAngles=&vehNoDrive->mSteerAngles[4*i];
			const PxF32* PX_RESTRICT rawDriveTorques=&vehNoDrive->mDriveTorques[4*i];
	
			//Work out which wheels are enabled.
			bool activeWheelStates[4]={false,false,false,false};
			computeWheelActiveStates(4*i, vehNoDrive->mWheelsSimData.mActiveWheelsBitmapBuffer, activeWheelStates);

			const PxVehicleWheels4SimData& wheels4SimData=wheels4SimDatas[i];
			PxVehicleWheels4DynData& wheels4DynData=wheels4DynDatas[i];

			//Compute the brake torques.
			PxF32 brakeTorques[4]={0.0f,0.0f,0.0f,0.0f};
			bool isBrakeApplied[4]={false,false,false,false};
			computeNoDriveBrakeTorques
				(wheels4SimData.mWheels,wheels4DynData.mWheelSpeeds,rawBrakeTorques,
				 brakeTorques,isBrakeApplied);

			//Compute the per wheel accel pedal values.
			bool isAccelApplied[4]={false,false,false,false};
			if(isIntentionToAccelerate)
			{
				computeIsAccelApplied(rawDriveTorques, isAccelApplied);
			}

			//Compute jounces, slips, tire forces, suspension forces etc.
			ProcessSuspWheelTireInputData inputData=
			{
				isIntentionToAccelerate, isAccelApplied, isBrakeApplied, rawSteerAngles, activeWheelStates,
				carChassisTransform, carChassisLinVel, carChassisAngVel,
				&wheels4SimData, &wheels4DynData, &tires4ForceCalculators[i], &tireLoadFilterData, numActiveWheelsPerBlock4[i]
			};
			ProcessSuspWheelTireOutputData outputData;
			processSuspTireWheels(4*i, constData, inputData, outputData);
			updateLowSpeedTimers(outputData.newLowForwardSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowForwardSpeedTimers);
			updateLowSpeedTimers(outputData.newLowSideSpeedTimers, (PxF32*)inputData.vehWheels4DynData->mTireLowSideSpeedTimers);
			updateJounces(outputData.jounces, (PxF32*)inputData.vehWheels4DynData->mJounces);
			if((numSubSteps-1) == k)
			{
				updateCachedHitData(outputData.cachedHitCounts, outputData.cachedHitPlanes, outputData.cachedHitDistances, outputData.cachedFrictionMultipliers, &wheels4DynData);
			}
			chassisForce+=outputData.chassisForce;
			chassisTorque+=outputData.chassisTorque;
			if(0 == k)
			{
				wheels4DynDatas[i].mVehicleConstraints->mData=outputData.vehConstraintData;
			}
			storeSuspWheelTireResults(outputData, inputData.steerAngles, &wheelQueryResults[4*i], numActiveWheelsPerBlock4[i]);
			storeHitActorForces(outputData, &vehicleConcurrentUpdates.concurrentWheelUpdates[4*i], numActiveWheelsPerBlock4[i]);

			//Integrate wheel speeds.
			const PxF32 wheelDampingRates[4]=
			{
				wheels4SimData.getWheelData(0).mDampingRate,
				wheels4SimData.getWheelData(1).mDampingRate,
				wheels4SimData.getWheelData(2).mDampingRate,
				wheels4SimData.getWheelData(3).mDampingRate
			};
			integrateNoDriveWheelSpeeds(
				subTimestep,
				brakeTorques,isBrakeApplied,rawDriveTorques,outputData.tireTorques,wheelDampingRates,
				wheels4SimData,wheels4DynData);

			integrateNoDriveWheelRotationAngles(
				subTimestep,
				rawDriveTorques,
				outputData.jounces, outputData.forwardSpeeds, isBrakeApplied,
				wheels4SimData,
				wheels4DynData);
		}

		//Integrate the chassis velocity by applying the accumulated force and torque.
		integrateBody(inverseChassisMass,inverseInertia,chassisForce,chassisTorque,subTimestep,carChassisLinVel,carChassisAngVel,carChassisTransform);
	}

	//Set the new chassis linear/angular velocity.
	if(!gApplyForces)
	{
		vehicleConcurrentUpdates.linearMomentumChange = carChassisLinVel;
		vehicleConcurrentUpdates.angularMomentumChange = carChassisAngVel;
	}
	else
	{
		//integration steps are: 
		//v = v0 + a*dt	(1)
		//x = x0 + v*dt	(2)
		//Sub (2) into (1.
		//x = x0 + v0*dt + a*dt*dt;
		//Rearrange for a
		//a = (x -x0 - v0*dt)/(dt*dt) = [(x-x0)/dt - v0/dt]
		//Rearrange again with v = (x-x0)/dt
		//a = (v - v0)/dt
		vehicleConcurrentUpdates.linearMomentumChange = (carChassisLinVel-carChassisLinVelOrig)*recipTimestep;
		vehicleConcurrentUpdates.angularMomentumChange = (carChassisAngVel-carChassisAngVelOrig)*recipTimestep;
	}

	//Pose the wheels from jounces, rotations angles, and steer angles.
	PxTransform localPoses0[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
	computeWheelLocalPoses(wheels4SimDatas[0],wheels4DynDatas[0],&wheelQueryResults[4*0],numActiveWheelsPerBlock4[0],carChassisCMLocalPose,localPoses0);
	wheelQueryResults[4*0 + 0].localPose = localPoses0[0];
	wheelQueryResults[4*0 + 1].localPose = localPoses0[1];
	wheelQueryResults[4*0 + 2].localPose = localPoses0[2];
	wheelQueryResults[4*0 + 3].localPose = localPoses0[3];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 0].localPose = localPoses0[0];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 1].localPose = localPoses0[1];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 2].localPose = localPoses0[2];
	vehicleConcurrentUpdates.concurrentWheelUpdates[4*0 + 3].localPose = localPoses0[3];
	for(PxU32 i=1;i<numWheels4;i++)
	{
		PxTransform localPoses[4] = {PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity), PxTransform(PxIdentity)};
		computeWheelLocalPoses(wheels4SimDatas[i],wheels4DynDatas[i],&wheelQueryResults[4*i],numActiveWheelsPerBlock4[i],carChassisCMLocalPose,localPoses);
		wheelQueryResults[4*i + 0].localPose = localPoses[0];
		wheelQueryResults[4*i + 1].localPose = localPoses[1];
		wheelQueryResults[4*i + 2].localPose = localPoses[2];
		wheelQueryResults[4*i + 3].localPose = localPoses[3];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 0].localPose = localPoses[0];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 1].localPose = localPoses[1];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 2].localPose = localPoses[2];
		vehicleConcurrentUpdates.concurrentWheelUpdates[4*i + 3].localPose = localPoses[3];
	}

	if(vehWheelQueryResults && vehWheelQueryResults->wheelQueryResults)
	{
		PxMemCopy(vehWheelQueryResults->wheelQueryResults, wheelQueryResults, sizeof(PxWheelQueryResult)*numActiveWheels);
	}

	if(vehConcurrentUpdates)
	{
		//Copy across to input data structure so that writes can be applied later.
		PxMemCopy(vehConcurrentUpdates->concurrentWheelUpdates, vehicleConcurrentUpdates.concurrentWheelUpdates, sizeof(PxVehicleWheelConcurrentUpdateData)*numActiveWheels);
		vehConcurrentUpdates->linearMomentumChange = vehicleConcurrentUpdates.linearMomentumChange;
		vehConcurrentUpdates->angularMomentumChange = vehicleConcurrentUpdates.angularMomentumChange;
		vehConcurrentUpdates->staySleeping = vehicleConcurrentUpdates.staySleeping;
		vehConcurrentUpdates->wakeup = vehicleConcurrentUpdates.wakeup;
	}
	else
	{
		//Apply the writes immediately.
		PxVehicleWheels* vehWheels[1]={vehNoDrive};
		PxVehiclePostUpdates(&vehicleConcurrentUpdates, 1, vehWheels);
	}
}


void PxVehicleUpdate::shiftOrigin(const PxVec3& shift, const PxU32 numVehicles, PxVehicleWheels** vehicles)
{
	for(PxU32 i=0; i < numVehicles; i++)
	{
		//Get the current car.
		PxVehicleWheels& veh = *vehicles[i];
		PxVehicleWheels4DynData* PX_RESTRICT wheels4DynData=veh.mWheelsDynData.mWheels4DynData;
		const PxU32 numWheels4=veh.mWheelsSimData.mNbWheels4;

		//Blocks of 4 wheels.
		for(PxU32 j=0; j < numWheels4; j++)
		{
			bool activeWheelStates[4]={false,false,false,false};
			computeWheelActiveStates(4*j, veh.mWheelsSimData.mActiveWheelsBitmapBuffer, activeWheelStates);

			if (wheels4DynData[j].mSqResults)  // this is set when a query has been scheduled
			{
				PxVehicleWheels4DynData::SuspLineRaycast& raycast = 
					(PxVehicleWheels4DynData::SuspLineRaycast&)wheels4DynData[j].mRaycastsOrCachedHitResults;
				
				for(PxU32 k=0; k < 4; k++)
				{
					if (activeWheelStates[k])
					{
						raycast.mStarts[k] -= shift;

						if (wheels4DynData[j].mSqResults[k].hasBlock)
							const_cast<PxVec3&>(wheels4DynData[j].mSqResults[k].block.position) -= shift;
					}
				}
			}
		}
	}
}

}//namespace physx

#if PX_DEBUG_VEHICLE_ON

/////////////////////////////////////////////////////////////////////////////////
//Update a single vehicle of any type and record the associated telemetry data.
/////////////////////////////////////////////////////////////////////////////////

void PxVehicleUpdate::updateSingleVehicleAndStoreTelemetryData
(const PxF32 timestep, const PxVec3& gravity, const PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
 PxVehicleWheels* vehWheels, PxVehicleWheelQueryResult* vehWheelQueryResults, PxVehicleTelemetryData& telemetryData)
{
	START_TIMER(TIMER_ALL);

	PX_CHECK_MSG(gravity.magnitude()>0, "gravity vector must have non-zero length");
	PX_CHECK_MSG(timestep>0, "timestep must be greater than zero");
	PX_CHECK_AND_RETURN(gThresholdForwardSpeedForWheelAngleIntegration>0, "PxInitVehicleSDK needs to be called before ever calling PxVehicleUpdateSingleVehicleAndStoreTelemetryData");
	PX_CHECK_MSG(vehWheels->mWheelsSimData.getNbWheels()==telemetryData.getNbWheelGraphs(), "vehicle and telemetry data need to have the same number of wheels");
	PX_CHECK_AND_RETURN(NULL==vehWheelQueryResults || vehWheelQueryResults->nbWheelQueryResults >= vehWheels->mWheelsSimData.getNbWheels(),
		"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");

#ifdef PX_CHECKED
	for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbWheels4;i++)
	{
		PX_CHECK_MSG(vehWheels->mWheelsDynData.mWheels4DynData[i].mSqResults, "Need to call PxVehicle4WSuspensionRaycasts before trying to update");
	}
	for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
	{
		PX_CHECK_MSG(vehWheels->mWheelsDynData.mTireForceCalculators->mShaderData[i], "Need to set non-null tire force shader data ptr");
	}
	PX_CHECK_MSG(vehWheels->mWheelsDynData.mTireForceCalculators->mShader, "Need to set non-null tire force shader function");

	for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
	{
		PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(i) || -1==vehWheels->mWheelsSimData.getWheelShapeMapping(i), 
			"Disabled wheels must not be associated with a PxShape:  use setWheelShapeMapping to remove the association");
		PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(i) || 0==vehWheels->mWheelsDynData.getWheelRotationSpeed(i), 
			"Disabled wheels must have zero rotation speed:  use setWheelRotationSpeed to set the wheel to zero rotation speed");
	}
#endif

	PxF32 engineGraphData[PxVehicleDriveGraphChannel::eMAX_NB_DRIVE_CHANNELS];
	PxMemZero(&engineGraphData[0], PxVehicleDriveGraphChannel::eMAX_NB_DRIVE_CHANNELS*sizeof(PxF32));
	gCarEngineGraphData=engineGraphData;

	PxF32 wheelGraphData[PX_MAX_NB_WHEELS][PxVehicleWheelGraphChannel::eMAX_NB_WHEEL_CHANNELS];
	PxMemZero(&wheelGraphData[0][0], PX_MAX_NB_WHEELS*PxVehicleWheelGraphChannel::eMAX_NB_WHEEL_CHANNELS*sizeof(PxF32));
	for(PxU32 i=0;i<4*vehWheels->mWheelsSimData.mNbWheels4;i++)
	{
		gCarWheelGraphData[i]=wheelGraphData[i];
	}
	for(PxU32 i=4*vehWheels->mWheelsSimData.mNbWheels4; i<PX_MAX_NB_WHEELS;i++)
	{
		gCarWheelGraphData[i]=NULL;
	}

	PxVec3 suspForceAppPoints[PX_MAX_NB_WHEELS];
	PxMemZero(suspForceAppPoints, PX_MAX_NB_WHEELS*sizeof(PxVec3));
	gCarSuspForceAppPoints=suspForceAppPoints;

	PxVec3 tireForceAppPoints[PX_MAX_NB_WHEELS];
	PxMemZero(tireForceAppPoints, PX_MAX_NB_WHEELS*sizeof(PxVec3));
	gCarTireForceAppPoints=tireForceAppPoints;

	const PxF32 gravityMagnitude=gravity.magnitude();
	const PxF32 recipGravityMagnitude=1.0f/gravityMagnitude;

	switch(vehWheels->mType)
	{
	case PxVehicleTypes::eDRIVE4W:
		{
			PxVehicleDrive4W* vehDrive4W=(PxVehicleDrive4W*)vehWheels;

			PxVehicleUpdate::updateDrive4W(
				timestep,
				gravity, gravityMagnitude, recipGravityMagnitude,
				vehicleDrivableSurfaceToTireFrictionPairs,
				vehDrive4W, vehWheelQueryResults, NULL);
				
			for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
			{
				telemetryData.mWheelGraphs[i].updateTimeSlice(wheelGraphData[i]);
				telemetryData.mSuspforceAppPoints[i]=suspForceAppPoints[i];
				telemetryData.mTireforceAppPoints[i]=tireForceAppPoints[i];
			}
			telemetryData.mEngineGraph->updateTimeSlice(engineGraphData);
		}
		break;
	case PxVehicleTypes::eDRIVENW:
		{
			PxVehicleDriveNW* vehDriveNW=(PxVehicleDriveNW*)vehWheels;

			PxVehicleUpdate::updateDriveNW(
				timestep,
				gravity, gravityMagnitude, recipGravityMagnitude,
				vehicleDrivableSurfaceToTireFrictionPairs,
				vehDriveNW, vehWheelQueryResults, NULL);

			for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
			{
				telemetryData.mWheelGraphs[i].updateTimeSlice(wheelGraphData[i]);
				telemetryData.mSuspforceAppPoints[i]=suspForceAppPoints[i];
				telemetryData.mTireforceAppPoints[i]=tireForceAppPoints[i];
			}
			telemetryData.mEngineGraph->updateTimeSlice(engineGraphData);
		}
		break;

	case PxVehicleTypes::eDRIVETANK:
		{
			PxVehicleDriveTank* vehDriveTank=(PxVehicleDriveTank*)vehWheels;

			PxVehicleUpdate::updateTank(
				timestep,
				gravity,gravityMagnitude,recipGravityMagnitude,
				vehicleDrivableSurfaceToTireFrictionPairs,
				vehDriveTank, vehWheelQueryResults, NULL);
				
			for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
			{
				telemetryData.mWheelGraphs[i].updateTimeSlice(wheelGraphData[i]);
				telemetryData.mSuspforceAppPoints[i]=suspForceAppPoints[i];
				telemetryData.mTireforceAppPoints[i]=tireForceAppPoints[i];
			}
			telemetryData.mEngineGraph->updateTimeSlice(engineGraphData);
		}
		break;
	case PxVehicleTypes::eNODRIVE:
		{
			PxVehicleNoDrive* vehDriveNoDrive=(PxVehicleNoDrive*)vehWheels;

			PxVehicleUpdate::updateNoDrive(					
				timestep,
				gravity,gravityMagnitude,recipGravityMagnitude,
				vehicleDrivableSurfaceToTireFrictionPairs,
				vehDriveNoDrive, vehWheelQueryResults, NULL);

			for(PxU32 i=0;i<vehWheels->mWheelsSimData.mNbActiveWheels;i++)
			{
				telemetryData.mWheelGraphs[i].updateTimeSlice(wheelGraphData[i]);
				telemetryData.mSuspforceAppPoints[i]=suspForceAppPoints[i];
				telemetryData.mTireforceAppPoints[i]=tireForceAppPoints[i];
			}
		}
		break;

	default:
		PX_CHECK_MSG(false, "updateSingleVehicleAndStoreTelemetryData - unsupported vehicle type"); 
		break;
	}

	END_TIMER(TIMER_ALL);

#if PX_VEHICLE_PROFILE 

	gTimerCount++;
	if(10==gTimerCount)
	{

		/*
		printf("%f %f %f %f %f %f %f %f %f\n", 
					localTimers[TIMER_ADMIN]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_GRAPHS]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_COMPONENTS_UPDATE]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_WHEELS]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_INTERNAL_DYNAMICS_SOLVER]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_POSTUPDATE1]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_POSTUPDATE2]/(1.0f*localTimers[TIMER_ALL]),
					localTimers[TIMER_POSTUPDATE3]/(1.0f*localTimers[TIMER_ALL]),
					floatTimeIn10sOfNs);
		*/

		printf("%f %f %f %f %f %f \n", 
			getTimerFraction(TIMER_WHEELS),
			getTimerFraction(TIMER_INTERNAL_DYNAMICS_SOLVER),
			getTimerFraction(TIMER_POSTUPDATE2),
			getTimerFraction(TIMER_POSTUPDATE3),
			getTimerInMilliseconds(TIMER_ALL),
			getTimerInMilliseconds(TIMER_RAYCASTS));

		gTimerCount=0;
		for(PxU32 i=0;i<MAX_NB_TIMERS;i++)
		{
			gTimers[i]=0;
		}
	}

#endif
}

void physx::PxVehicleUpdateSingleVehicleAndStoreTelemetryData
(const PxReal timestep, const PxVec3& gravity, const physx::PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
 PxVehicleWheels* focusVehicle, PxVehicleWheelQueryResult* wheelQueryResults, PxVehicleTelemetryData& telemetryData)
{
	PxVehicleUpdate::updateSingleVehicleAndStoreTelemetryData
		(timestep, gravity, vehicleDrivableSurfaceToTireFrictionPairs, focusVehicle, wheelQueryResults, telemetryData);
}

#endif

////////////////////////////////////////////////////////////
//Update an array of vehicles of any type
////////////////////////////////////////////////////////////

void PxVehicleUpdate::update
(const PxF32 timestep, const PxVec3& gravity, const PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
 const PxU32 numVehicles, PxVehicleWheels** vehicles, PxVehicleWheelQueryResult* vehicleWheelQueryResults, PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates)
{
	PX_CHECK_AND_RETURN(gravity.magnitude()>0, "gravity vector must have non-zero length");
	PX_CHECK_AND_RETURN(timestep>0, "timestep must be greater than zero");
	PX_CHECK_AND_RETURN(gThresholdForwardSpeedForWheelAngleIntegration>0, "PxInitVehicleSDK needs to be called before ever calling PxVehicleUpdates");

#ifdef PX_CHECKED
	for(PxU32 i=0;i<numVehicles;i++)
	{
		const PxVehicleWheels* const vehWheels=vehicles[i];
		for(PxU32 j=0;j<vehWheels->mWheelsSimData.mNbWheels4;j++)
		{
			PX_CHECK_MSG(
				vehWheels->mWheelsDynData.mWheels4DynData[j].mSqResults || 
				vehWheels->mWheelsDynData.mWheels4DynData[0].mHasCachedRaycastHitPlane ||
				(vehWheels->mWheelsSimData.getIsWheelDisabled(4*j+0) &&  
				 vehWheels->mWheelsSimData.getIsWheelDisabled(4*j+1) &&  
				 vehWheels->mWheelsSimData.getIsWheelDisabled(4*j+2) &&  
				 vehWheels->mWheelsSimData.getIsWheelDisabled(4*j+3)),
				"Need to call PxVehicle4WSuspensionRaycasts at least once before trying to update");
		}
		for(PxU32 j=0;j<vehWheels->mWheelsSimData.mNbActiveWheels;j++)
		{
			PX_CHECK_MSG(vehWheels->mWheelsDynData.mTireForceCalculators->mShaderData[j], "Need to set non-null tire force shader data ptr");
		}
		PX_CHECK_MSG(vehWheels->mWheelsDynData.mTireForceCalculators->mShader, "Need to set non-null tire force shader function");

		PX_CHECK_AND_RETURN(NULL==vehicleWheelQueryResults || vehicleWheelQueryResults[i].nbWheelQueryResults >= vehicles[i]->mWheelsSimData.getNbWheels(),
			"nbWheelQueryResults must always be greater than or equal to number of wheels in corresponding vehicle");

		for(PxU32 j=0;j<vehWheels->mWheelsSimData.mNbActiveWheels;j++)
		{
			PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(j) || -1==vehWheels->mWheelsSimData.getWheelShapeMapping(j), 
				"Disabled wheels must not be associated with a PxShape:  use setWheelShapeMapping to remove the association");

			PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(j) || 0==vehWheels->mWheelsDynData.getWheelRotationSpeed(j), 
				"Disabled wheels must have zero rotation speed:  use setWheelRotationSpeed to set the wheel to zero rotation speed");
		}

		PX_CHECK_AND_RETURN(!vehicleConcurrentUpdates || (vehicleConcurrentUpdates[i].concurrentWheelUpdates && vehicleConcurrentUpdates[i].nbConcurrentWheelUpdates >= vehicles[i]->mWheelsSimData.getNbWheels()),
			"vehicleConcurrentUpdates is illegally configured with either null pointers or with insufficient memory for successful concurrent updates."); 
	}
#endif

#if PX_DEBUG_VEHICLE_ON
	gCarEngineGraphData=NULL;
	for(PxU32 j=0;j<PX_MAX_NB_WHEELS;j++)
	{
		gCarWheelGraphData[j]=NULL;
	}
	gCarSuspForceAppPoints=NULL;
	gCarTireForceAppPoints=NULL;
#endif

	const PxF32 gravityMagnitude=gravity.magnitude();
	const PxF32 recipGravityMagnitude=1.0f/gravityMagnitude;

	for(PxU32 i=0;i<numVehicles;i++)
	{
		PxVehicleWheels* vehWheels=vehicles[i];
		PxVehicleWheelQueryResult* vehWheelQueryResults = vehicleWheelQueryResults ? &vehicleWheelQueryResults[i] : NULL;
		PxVehicleConcurrentUpdateData* vehConcurrentUpdateData = vehicleConcurrentUpdates ? &vehicleConcurrentUpdates[i] : NULL;
		switch(vehWheels->mType)
		{
		case PxVehicleTypes::eDRIVE4W:
			{
				PxVehicleDrive4W* vehDrive4W=(PxVehicleDrive4W*)vehWheels;

				PxVehicleUpdate::updateDrive4W(					
					timestep,
					gravity,gravityMagnitude,recipGravityMagnitude,
					vehicleDrivableSurfaceToTireFrictionPairs,
					vehDrive4W, vehWheelQueryResults, vehConcurrentUpdateData);
				}
			break;

		case PxVehicleTypes::eDRIVENW:
			{
				PxVehicleDriveNW* vehDriveNW=(PxVehicleDriveNW*)vehWheels;

				PxVehicleUpdate::updateDriveNW(					
					timestep,
					gravity,gravityMagnitude,recipGravityMagnitude,
					vehicleDrivableSurfaceToTireFrictionPairs,
					vehDriveNW, vehWheelQueryResults, vehConcurrentUpdateData);
			}
			break;

		case PxVehicleTypes::eDRIVETANK:
			{
				PxVehicleDriveTank* vehDriveTank=(PxVehicleDriveTank*)vehWheels;

				PxVehicleUpdate::updateTank(
					timestep,
					gravity,gravityMagnitude,recipGravityMagnitude,
					vehicleDrivableSurfaceToTireFrictionPairs,
					vehDriveTank, vehWheelQueryResults, vehConcurrentUpdateData);
			}
			break;	

		case PxVehicleTypes::eNODRIVE:
			{
				PxVehicleNoDrive* vehDriveNoDrive=(PxVehicleNoDrive*)vehWheels;

				PxVehicleUpdate::updateNoDrive(					
					timestep,
					gravity,gravityMagnitude,recipGravityMagnitude,
					vehicleDrivableSurfaceToTireFrictionPairs,
					vehDriveNoDrive, vehWheelQueryResults, vehConcurrentUpdateData);
			}
			break;
			
		default:
			PX_CHECK_MSG(false, "update - unsupported vehicle type"); 
			break;
		}
	}
}


void PxVehicleUpdate::updatePost
(const PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates, const PxU32 numVehicles, PxVehicleWheels** vehicles)
{
	PX_CHECK_AND_RETURN(vehicleConcurrentUpdates, "vehicleConcurrentUpdates must be non-null.");

#ifdef PX_CHECKED
	for(PxU32 i=0;i<numVehicles;i++)
	{
		PxVehicleWheels* vehWheels=vehicles[i];
		for(PxU32 j=0;j<vehWheels->mWheelsSimData.mNbActiveWheels;j++)
		{
			PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(j) || -1==vehWheels->mWheelsSimData.getWheelShapeMapping(j), 
				"Disabled wheels must not be associated with a PxShape:  use setWheelShapeMapping to remove the association");

			PX_CHECK_AND_RETURN(!vehWheels->mWheelsSimData.getIsWheelDisabled(j) || 0==vehWheels->mWheelsDynData.getWheelRotationSpeed(j), 
				"Disabled wheels must have zero rotation speed:  use setWheelRotationSpeed to set the wheel to zero rotation speed");

			PX_CHECK_AND_RETURN(vehicleConcurrentUpdates[i].concurrentWheelUpdates && vehicleConcurrentUpdates[i].nbConcurrentWheelUpdates >= vehWheels->mWheelsSimData.getNbWheels(),
				"vehicleConcurrentUpdates is illegally configured with either null pointers or insufficient memory for successful concurrent vehicle updates.");
		}
	}
#endif

	for(PxU32 i=0;i<numVehicles;i++)
	{
		//Get the ith vehicle and its actor.
		PxVehicleWheels* vehWheels=vehicles[i];
		PxRigidDynamic* vehActor = vehWheels->getRigidDynamicActor();

		//Get the concurrent update data for the ith vehicle.
		//This contains the data that couldn't get updated concurrently and now must be 
		//set sequentially.
		const PxVehicleConcurrentUpdateData& vehicleConcurrentUpdate = vehicleConcurrentUpdates[i];

		//Test if the actor is to remain sleeping.
		//If the actor is to remain sleeping then do nothing.
		if(!vehicleConcurrentUpdate.staySleeping)
		{
			//Wake the vehicle's actor up as required.
			if(vehicleConcurrentUpdate.wakeup)
			{
				vehActor->wakeUp();
			}

			//Apply momentum changes to vehicle's actor
			if(!gApplyForces)
			{
				vehActor->setLinearVelocity(vehicleConcurrentUpdate.linearMomentumChange, false);
				vehActor->setAngularVelocity(vehicleConcurrentUpdate.angularMomentumChange, false);
			}
			else
			{
				vehActor->addForce(vehicleConcurrentUpdate.linearMomentumChange, PxForceMode::eACCELERATION, false);
				vehActor->addTorque(vehicleConcurrentUpdate.angularMomentumChange, PxForceMode::eACCELERATION, false);
			}

			//In each block of 4 wheels record how many wheels are active.
			const PxU32 numActiveWheels=vehWheels->mWheelsSimData.mNbActiveWheels;
			const PxU32 numWheels4 = vehWheels->mWheelsSimData.getNbWheels4();
			const PxU32 numActiveWheelsInLast4=4-(4*numWheels4 - numActiveWheels);
			PxU32 numActiveWheelsPerBlock4[PX_MAX_NB_SUSPWHEELTIRE4]={0,0,0,0,0};
			numActiveWheelsPerBlock4[0]=PxMin(numActiveWheels,(PxU32)4);
			for(PxU32 j=1;j<numWheels4-1;j++)
			{
				numActiveWheelsPerBlock4[j]=4;
			}
			numActiveWheelsPerBlock4[numWheels4-1]=numActiveWheelsInLast4;
			PX_ASSERT(numActiveWheels == numActiveWheelsPerBlock4[0] + numActiveWheelsPerBlock4[1] + numActiveWheelsPerBlock4[2] + numActiveWheelsPerBlock4[3] + numActiveWheelsPerBlock4[4]); 

			//Apply the local poses to the shapes of the vehicle's actor that represent wheels.
			for(PxU32 j=0;j<numWheels4;j++)
			{
				PxTransform localPoses[4]=
				{
					vehicleConcurrentUpdate.concurrentWheelUpdates[j*4 + 0].localPose,
					vehicleConcurrentUpdate.concurrentWheelUpdates[j*4 + 1].localPose,
					vehicleConcurrentUpdate.concurrentWheelUpdates[j*4 + 2].localPose,
					vehicleConcurrentUpdate.concurrentWheelUpdates[j*4 + 3].localPose
				};
				poseWheels(vehWheels->mWheelsSimData.mWheels4SimData[j],localPoses,numActiveWheelsPerBlock4[j],vehActor);
			}

			//Apply forces to dynamic actors hit by the wheels.
			for(PxU32 j=0;j<numActiveWheels;j++)
			{
				PxRigidDynamic* hitActor=vehicleConcurrentUpdate.concurrentWheelUpdates[j].hitActor;
				if(hitActor)
				{
					const PxVec3& hitForce=vehicleConcurrentUpdate.concurrentWheelUpdates[j].hitActorForce;
					const PxVec3& hitForcePosition=vehicleConcurrentUpdate.concurrentWheelUpdates[j].hitActorForcePosition;
					PxRigidBodyExt::addForceAtPos(*hitActor,hitForce,hitForcePosition);
				}
			}
		}
	}
}


void physx::PxVehicleUpdates
(const PxReal timestep, const PxVec3& gravity, const PxVehicleDrivableSurfaceToTireFrictionPairs& vehicleDrivableSurfaceToTireFrictionPairs, 
 const PxU32 numVehicles, PxVehicleWheels** vehicles, PxVehicleWheelQueryResult* vehicleWheelQueryResults, PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates)
{
	START_PROFILER(ePROFILE_UPDATES)
	PxVehicleUpdate::update(timestep, gravity, vehicleDrivableSurfaceToTireFrictionPairs, numVehicles, vehicles, vehicleWheelQueryResults, vehicleConcurrentUpdates);
	STOP_PROFILER(ePROFILE_UPDATES)
}

void physx::PxVehiclePostUpdates
(const PxVehicleConcurrentUpdateData* vehicleConcurrentUpdates, const PxU32 numVehicles, PxVehicleWheels** vehicles)
{
	START_PROFILER(ePROFILE_POSTUPDATES)
	PxVehicleUpdate::updatePost(vehicleConcurrentUpdates, numVehicles, vehicles);
	STOP_PROFILER(ePROFILE_POSTUPDATES)
}

void physx::PxVehicleShiftOrigin(const PxVec3& shift, const PxU32 numVehicles, PxVehicleWheels** vehicles)
{
	PxVehicleUpdate::shiftOrigin(shift, numVehicles, vehicles);
}

///////////////////////////////////////////////////////////////////////////////////
//The following functions issue  a single batch of suspension raycasts for an array of vehicles of any type.
//The buffer of sceneQueryResults is distributed among the vehicles in the array 
//for use in the next PxVehicleUpdates call.
///////////////////////////////////////////////////////////////////////////////////

void PxVehicleWheels4SuspensionRaycasts
(PxBatchQuery* batchQuery, 
 const PxVehicleWheels4SimData& wheels4SimData, PxVehicleWheels4DynData& wheels4DynData, 
 const PxQueryFilterData* carFilterData, const bool* activeWheelStates, const PxU32 numActiveWheels,
 PxRigidDynamic* vehActor)
{
	//Get the transform of the chassis.
	PxTransform carChassisTrnsfm=vehActor->getGlobalPose().transform(vehActor->getCMassLocalPose());

	//Add a raycast for each wheel.
	for(PxU32 j=0;j<numActiveWheels;j++)
	{
		const PxVehicleSuspensionData& susp=wheels4SimData.getSuspensionData(j);
		const PxVehicleWheelData& wheel=wheels4SimData.getWheelData(j);

		const PxVec3& bodySpaceSuspTravelDir=wheels4SimData.getSuspTravelDirection(j);
		PxVec3 bodySpaceWheelCentreOffset=wheels4SimData.getWheelCentreOffset(j);
		PxF32 maxDroop=susp.mMaxDroop;
		PxF32 maxBounce=susp.mMaxCompression;
		PxF32 radius=wheel.mRadius;
		PX_ASSERT(maxBounce>=0);
		PX_ASSERT(maxDroop>=0);

		if(!activeWheelStates[j])
		{
			//For disabled wheels just issue a raycast of almost zero length.
			//This should be very cheap and ought to hit nothing.
			bodySpaceWheelCentreOffset=PxVec3(0,0,0);
			maxDroop=1e-5f*gToleranceScaleLength;
			maxBounce=1e-5f*gToleranceScaleLength;
			radius=1e-5f*gToleranceScaleLength;
		}

		PxVec3 suspLineStart;
		PxVec3 suspLineDir;
		computeSuspensionRaycast(carChassisTrnsfm,bodySpaceWheelCentreOffset,bodySpaceSuspTravelDir,radius,maxBounce,suspLineStart,suspLineDir);

		//Total length from top of wheel at max compression to bottom of wheel at max droop.
		PxF32 suspLineLength=radius + maxBounce  + maxDroop + radius;
		//Add another radius on for good measure.
		suspLineLength+=radius;

		//Store the susp line ray for later use.
		PxVehicleWheels4DynData::SuspLineRaycast& raycast = 
			(PxVehicleWheels4DynData::SuspLineRaycast&)wheels4DynData.mRaycastsOrCachedHitResults;
		raycast.mStarts[j]=suspLineStart;
		raycast.mDirs[j]=suspLineDir;
		raycast.mLengths[j]=suspLineLength;

		//Add the raycast to the scene query.
		batchQuery->raycast(
			suspLineStart, suspLineDir, suspLineLength, 0,
			PxHitFlag::ePOSITION|PxHitFlag::eNORMAL|PxHitFlag::eDISTANCE|PxHitFlag::eUV, carFilterData[j]);
	}
}

void PxVehicleUpdate::suspensionRaycasts(PxBatchQuery* batchQuery, const PxU32 numVehicles, PxVehicleWheels** vehicles, const PxU32 numSceneQueryResults, PxRaycastQueryResult* sceneQueryResults, const bool* vehiclesToRaycast)
{
	START_TIMER(TIMER_RAYCASTS);

	//Reset all hit counts to zero.
	for(PxU32 i=0;i<numSceneQueryResults;i++)
	{
		sceneQueryResults[i].hasBlock=false;
	}

	PxRaycastQueryResult* sqres=sceneQueryResults;

	const PxQueryFlags flags = PxQueryFlag::eSTATIC|PxQueryFlag::eDYNAMIC|PxQueryFlag::ePREFILTER;
	PxQueryFilterData carFilterData[4];
	carFilterData[0].flags=flags;
	carFilterData[1].flags=flags;
	carFilterData[2].flags=flags;
	carFilterData[3].flags=flags;

	//Work out the rays for the suspension line raycasts and perform all the raycasts.
	for(PxU32 i=0;i<numVehicles;i++)
	{
		//Get the current car.
		PxVehicleWheels& veh=*vehicles[i];
		const PxVehicleWheels4SimData* PX_RESTRICT wheels4SimData=veh.mWheelsSimData.mWheels4SimData;
		PxVehicleWheels4DynData* PX_RESTRICT wheels4DynData=veh.mWheelsDynData.mWheels4DynData;
		const PxU32 numWheels4=((veh.mWheelsSimData.mNbActiveWheels & ~3) >> 2);
		const PxU32 numActiveWheels=veh.mWheelsSimData.mNbActiveWheels;
		const PxU32 numActiveWheelsInLast4=numActiveWheels-4*numWheels4;
		PxRigidDynamic* vehActor=veh.mActor;

		//Set the results pointer and start the raycasts.
		PX_ASSERT(numActiveWheelsInLast4<4);

		//Blocks of 4 wheels.
		for(PxU32 j=0;j<numWheels4;j++)
		{
			bool activeWheelStates[4]={false,false,false,false};
			computeWheelActiveStates(4*j, veh.mWheelsSimData.mActiveWheelsBitmapBuffer, activeWheelStates);

			wheels4DynData[j].mSqResults=NULL;

			if(NULL==vehiclesToRaycast || vehiclesToRaycast[i])
			{
				if((sceneQueryResults + numSceneQueryResults) >= (sqres+4))
				{
					carFilterData[0].data=wheels4SimData[j].getSceneQueryFilterData(0);
					carFilterData[1].data=wheels4SimData[j].getSceneQueryFilterData(1);
					carFilterData[2].data=wheels4SimData[j].getSceneQueryFilterData(2);
					carFilterData[3].data=wheels4SimData[j].getSceneQueryFilterData(3);
					wheels4DynData[j].mSqResults=sqres;
					PxVehicleWheels4SuspensionRaycasts(batchQuery,wheels4SimData[j],wheels4DynData[j],carFilterData,activeWheelStates,4,vehActor);
				}
				else
				{
					PX_CHECK_MSG(false, "PxVehicleUpdate::suspensionRaycasts - numSceneQueryResults not big enough to support one raycast hit report per wheel.  Increase size of sceneQueryResults");
				}
				sqres+=4;
			}
		}
		//Remainder that don't make up a block of 4.
		if(numActiveWheelsInLast4>0)
		{
			const PxU32 j=numWheels4;

			bool activeWheelStates[4]={false,false,false,false};
			computeWheelActiveStates(4*j, veh.mWheelsSimData.mActiveWheelsBitmapBuffer, activeWheelStates);

			wheels4DynData[j].mSqResults=NULL;
			
			if(NULL==vehiclesToRaycast || vehiclesToRaycast[i])
			{
				if((sceneQueryResults + numSceneQueryResults) >= (sqres+numActiveWheelsInLast4))
				{
					if(0<numActiveWheelsInLast4) carFilterData[0].data=wheels4SimData[j].getSceneQueryFilterData(0);
					if(1<numActiveWheelsInLast4) carFilterData[1].data=wheels4SimData[j].getSceneQueryFilterData(1);
					if(2<numActiveWheelsInLast4) carFilterData[2].data=wheels4SimData[j].getSceneQueryFilterData(2);
					wheels4DynData[j].mSqResults=sqres;
					PxVehicleWheels4SuspensionRaycasts(batchQuery,wheels4SimData[j],wheels4DynData[j],carFilterData,activeWheelStates,numActiveWheelsInLast4,vehActor);
				}
				else
				{
					PX_CHECK_MSG(false, "PxVehicleUpdate::suspensionRaycasts - numSceneQueryResults not bit enough to support one raycast hit report per wheel.  Increase size of sceneQueryResults");
				}
				sqres+=numActiveWheelsInLast4;
			}
		}
	}

	batchQuery->execute();

	END_TIMER(TIMER_RAYCASTS);
}

void physx::PxVehicleSuspensionRaycasts(PxBatchQuery* batchQuery, const PxU32 numVehicles, PxVehicleWheels** vehicles, const PxU32 numSceneQueryesults, PxRaycastQueryResult* sceneQueryResults, const bool* vehiclesToRaycast)
{
	START_PROFILER(ePROFILE_RAYCASTS)
	PxVehicleUpdate::suspensionRaycasts(batchQuery, numVehicles, vehicles, numSceneQueryesults, sceneQueryResults, vehiclesToRaycast);
	STOP_PROFILER(ePROFILE_RAYCASTS)
}

