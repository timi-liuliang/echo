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

#ifndef PX_SIMSTATEDATA
#define PX_SIMSTATEDATA

#include "ScBodyCore.h"

namespace physx
{
namespace Sc
{
	struct Kinematic : public KinematicTransform
	{
		// The following members buffer the original body data to restore them when switching back to dynamic body
		// (for kinematics the corresponding LowLevel properties are set to predefined values)
		PxVec3			backupInverseInertia;			// The inverse of the body space inertia tensor
		PxReal			backupInvMass;					// The inverse of the body mass
		PxReal			backupLinearDamping;			// The velocity is scaled by (1.0f - this * dt) inside integrateVelocity() every substep.
		PxReal			backupAngularDamping;
		PxReal			backupMaxAngVelSq;				// The angular velocity's magnitude is clamped to this maximum value.
		PxReal			backupMaxLinVelSq;				// The angular velocity's magnitude is clamped to this maximum value	
	};
	PX_COMPILE_TIME_ASSERT(0 == (sizeof(Kinematic) & 0x0f));

	// Important: Struct is reset in setForcesToDefaults.

	enum VelocityModFlags
	{
		VMF_GRAVITY_DIRTY	= (1 << 0),
		VMF_ACC_DIRTY		= (1 << 1),
		VMF_VEL_DIRTY		= (1 << 2)
	};

	struct VelocityMod
	{
		PxVec3	linearPerSec;		// A request to change the linear velocity by this much each second. The velocity is changed by this * dt inside integrateVelocity().
		PxU8	flags;
		PxU8	pad0[3];
		PxVec3	angularPerSec;
		PxU8	pad1[3];
		PxU8	type;
		PxVec3	linearPerStep;		// A request to change the linear velocity by this much the next step. The velocity is changed inside updateForces().
		PxU32	pad2;
		PxVec3	angularPerStep;
		PxU32	pad3;

		PX_FORCE_INLINE	void					clear()													{ linearPerSec = angularPerSec = linearPerStep = angularPerStep = PxVec3(0.0f); }

		PX_FORCE_INLINE const PxVec3&			getLinearVelModPerSec()							const	{ return linearPerSec;			}
		PX_FORCE_INLINE void					accumulateLinearVelModPerSec(const PxVec3& v)			{ linearPerSec += v;			}
		PX_FORCE_INLINE void					clearLinearVelModPerSec()								{ linearPerSec = PxVec3(0.0f);	}

		PX_FORCE_INLINE const PxVec3&			getLinearVelModPerStep()						const	{ return linearPerStep;			}
		PX_FORCE_INLINE void					accumulateLinearVelModPerStep(const PxVec3& v)			{ linearPerStep += v;			}
		PX_FORCE_INLINE void					clearLinearVelModPerStep()								{ linearPerStep = PxVec3(0.0f);	}

		PX_FORCE_INLINE const PxVec3&			getAngularVelModPerSec()						const	{ return angularPerSec;			}
		PX_FORCE_INLINE void					accumulateAngularVelModPerSec(const PxVec3& v)			{ angularPerSec += v;			}
		PX_FORCE_INLINE void					clearAngularVelModPerSec()								{ angularPerSec = PxVec3(0.0f);	}

		PX_FORCE_INLINE const PxVec3&			getAngularVelModPerStep()						const	{ return angularPerStep;		}
		PX_FORCE_INLINE void					accumulateAngularVelModPerStep(const PxVec3& v)			{ angularPerStep += v;			}
		PX_FORCE_INLINE void					clearAngularVelModPerStep()								{ angularPerStep = PxVec3(0.0f);}

		PX_FORCE_INLINE void					notifyAddAcceleration()									{ flags |= VMF_ACC_DIRTY;		}
		PX_FORCE_INLINE void					notifyClearAcceleration()								{ flags |= VMF_ACC_DIRTY;		}
		PX_FORCE_INLINE void					notifyAddVelocity()										{ flags |= VMF_VEL_DIRTY;		}
		PX_FORCE_INLINE void					notifyClearVelocity()									{ flags |= VMF_VEL_DIRTY;		}
	};
	PX_COMPILE_TIME_ASSERT(sizeof(VelocityMod) == sizeof(Kinematic));


	// Structure to store data for kinematics (target pose etc.)
	// note: we do not delete this object for kinematics even if no target is set.
	struct SimStateData : public Ps::UserAllocated	// TODO: may want to optimize the allocation of this further.
	{
		PxU8 data[sizeof(Kinematic)];

		enum Enum
		{
			eVelMod=0,
			eKine
		};

		SimStateData(){}
		SimStateData(const PxU8 type)
		{
			PxMemZero(data, sizeof(Kinematic));
			Kinematic* kine = (Kinematic*)data;
			kine->type = type;
		}

		PX_FORCE_INLINE PxU32 getType() const { Kinematic* kine = (Kinematic*)data; return kine->type;}
		PX_FORCE_INLINE bool isKine() const {return eKine == getType();}
		PX_FORCE_INLINE bool isVelMod() const {return eVelMod == getType();}

		Kinematic* getKinematicData() { Kinematic* kine = (Kinematic*)data; PX_ASSERT(eKine == kine->type);  return kine;}
		VelocityMod* getVelocityModData() { VelocityMod* velmod = (VelocityMod*)data; PX_ASSERT(eVelMod == velmod->type); return velmod;}
		const Kinematic* getKinematicData() const { Kinematic* kine = (Kinematic*)data; PX_ASSERT(eKine == kine->type);  return kine;}
		const VelocityMod* getVelocityModData() const { VelocityMod* velmod = (VelocityMod*)data; PX_ASSERT(eVelMod == velmod->type); return velmod;}
	};

} // namespace Sc

}  // namespace physx

#endif //PX_SIMSTATEDATA
