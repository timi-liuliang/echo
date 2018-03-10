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

        
#ifndef PXS_CONTACTMANAGER_H
#define PXS_CONTACTMANAGER_H

#include "PxvConfig.h"
#include "PxvManager.h"
#include "PxcSolverConstraintDesc.h"
#include "PxcFrictionPatch.h"
#include "PxcNpWorkUnit.h"
#include "PxcNpContactPrep.h"
#include "PxcSolverContactPF.h"
#include "PxcSolverContact.h"
#include "PxcSolverConstraintTypes.h"

namespace physx
{

class PxsThreadContext;
class PxsContext;
class PxsRigidBody;
struct PxsCCDBody;
class PxsMaterialManager;
struct PxsCCDShape;

enum PxsPairVisColor 
{

	eVIS_COLOR_SWEPTINTEGRATE_OFF = 0x000000,
	eVIS_COLOR_SWEPTINTEGRATE_SLOW = 0x404040,
	eVIS_COLOR_SWEPTINTEGRATE_CLEAR = 0x007f00,
	eVIS_COLOR_SWEPTINTEGRATE_IMPACT = 0x1680ff,
	eVIS_COLOR_SWEPTINTEGRATE_FAIL = 0x0000ff

};


/**
\brief Additional header structure for CCD contact data stream.
*/
struct PxsCCDContactHeader
{
	/**
	\brief Stream for next collision. The same pair can collide multiple times during multiple CCD passes.
	*/
	PxsCCDContactHeader* nextStream;	//4    //8
	/**
	\brief Size (in bytes) of the CCD contact stream (not including force buffer)
	*/
	PxU16 contactStreamSize;			//6    //10
	/**
	\brief Defines whether the stream is from a previous pass.
	
	It could happen that the stream can not get allocated because we run out of memory. In that case the current event should not use the stream
	from an event of the previous pass.
	*/
	PxU16 isFromPreviousPass;			//8    //12
};


class PxsContactManager
{
public:
											PxsContactManager(PxsContext* context, PxU32 index);
											~PxsContactManager();

						void				init(const PxvManagerDescRigidRigid& desc, const PxsMaterialManager* materialManager);

	//PX_FORCE_INLINE		void				setDynamicFriction(PxReal v)								{ mNpUnit.dynamicFriction = v;				}
	//PX_FORCE_INLINE		PxReal				getDynamicFriction()								const	{ return mNpUnit.dynamicFriction;			}

	//PX_FORCE_INLINE		void				setStaticFriction(PxReal v)									{ mNpUnit.staticFriction = v;				}
	//PX_FORCE_INLINE		PxReal				getStaticFriction()									const	{ return mNpUnit.staticFriction;			}

	//PX_FORCE_INLINE		void				setRestitution(PxReal v)									{ mNpUnit.restitution = v;					}
	//PX_FORCE_INLINE		PxReal				getRestitution()									const	{ return mNpUnit.restitution;				}

	PX_FORCE_INLINE		void				setDisableStrongFriction(PxU32 d)							{ (!d)	? mNpUnit.flags &= ~PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION 
																												: mNpUnit.flags |= PxcNpWorkUnitFlag::eDISABLE_STRONG_FRICTION; }

	PX_FORCE_INLINE		PxReal				getRestDistance()									const	{ return mNpUnit.restDistance;				}
	PX_FORCE_INLINE		void				setRestDistance(PxReal v)									{ mNpUnit.restDistance = v;					}

	//PX_FORCE_INLINE		PxReal				getCorrelationDistance()							const	{ return mNpUnit.correlationDistance;		}
	//PX_FORCE_INLINE		void				setCorrelationDistance(PxReal v)							{ mNpUnit.correlationDistance = v;			}

						void				destroy();

	PX_FORCE_INLINE		PxReal				getDominance0()										const	{ return mNpUnit.dominance0;				}
	PX_FORCE_INLINE		void				setDominance0(PxReal v)										{ mNpUnit.dominance0 = v;					}

	PX_FORCE_INLINE		PxReal				getDominance1()										const	{ return mNpUnit.dominance1;				}
	PX_FORCE_INLINE		void				setDominance1(PxReal v)										{ mNpUnit.dominance1 = v;					}

	PX_FORCE_INLINE		PxU16				getTouchStatus()									const	{ return (PxU16)(mNpUnit.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_TOUCH); }
	PX_FORCE_INLINE		PxU16				touchStatusKnown()									const	{ return (PxU16)(mNpUnit.statusFlags & PxcNpWorkUnitStatusFlag::eTOUCH_KNOWN); }
	PX_FORCE_INLINE		PxU32				getIndex()											const	{ return mNpUnit.index;						}
	
	PX_FORCE_INLINE		PxU16				getHasSolverConstraint()							const	{ return (PxU16)((mNpUnit.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_SOLVER_CONSTRAINTS) && (mNpUnit.contactCount != 0));	}

	PX_FORCE_INLINE		PxU16				getHasCCDRetouch()									const	{ return (PxU16)(mNpUnit.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH); }
	PX_FORCE_INLINE		void				clearCCDRetouch()											{ mNpUnit.statusFlags &= ~PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH; }
	PX_FORCE_INLINE		void				raiseCCDRetouch()											{ mNpUnit.statusFlags |= PxcNpWorkUnitStatusFlag::eHAS_CCD_RETOUCH; }

	PX_FORCE_INLINE		PxU16				getHasContactForces()								const	{ return (PxU16)(mNpUnit.statusFlags & PxcNpWorkUnitStatusFlag::eHAS_CONTACT_FORCES); }

	// flags stuff - needs to be refactored

	PX_FORCE_INLINE		Ps::IntBool			isChangeable()										const	{ return Ps::IntBool(mFlags & PXS_CM_CHANGEABLE);		}
	PX_FORCE_INLINE		Ps::IntBool			getCCD()											const	{ return Ps::IntBool(mFlags & PXS_CM_CCD_LINEAR) && !mNpUnit.disableCCDContact; }
	PX_FORCE_INLINE		Ps::IntBool			getHadCCDContact()									const	{ return Ps::IntBool(mFlags & PXS_CM_CCD_CONTACT); }
	PX_FORCE_INLINE		void				setHadCCDContact()											{ mFlags |= PXS_CM_CCD_CONTACT; }
						void				setCCD(bool enable);
	PX_FORCE_INLINE		void				clearCCDContactInfo()										{ mFlags &= ~PXS_CM_CCD_CONTACT; mNpUnit.ccdContacts = NULL; }

	PX_FORCE_INLINE		PxcNpWorkUnit&		getWorkUnit()												{ return mNpUnit; }
	PX_FORCE_INLINE		const PxcNpWorkUnit&		getWorkUnit()	const								{ return mNpUnit; }

						PxU32				getConstraintDesc(PxcSolverConstraintDesc& desc)	const;
						PxU32				getFrictionConstraintDesc(PxcSolverConstraintDesc& desc)	const;

	PX_FORCE_INLINE		void*				getUserData()										const	{ return mUserData;			}
	PX_FORCE_INLINE		void				setUserData(void* data)										{ mUserData = data;			}

	// Setup solver-constraints
						void				resetCachedState();
	PX_FORCE_INLINE		PxU32				getContactCount()	{ return mNpUnit.contactCount; }

	PX_FORCE_INLINE		PxModifiableContact*		
											getContactsForModification() 
	{
		return (PxModifiableContact*)(mNpUnit.compressedContacts + sizeof(PxModifyContactHeader) + sizeof(PxContactPatchBase)); 
	}
						PxU32				getContactPointData(const void*& compressedContactData, PxU32& compressedContactSize, const PxReal*& forces,
																const PxsCCDContactHeader*& ccdContactData)
											{
												//points = mNpUnit.contactPoints;

												PX_ASSERT((mNpUnit.compressedContacts != NULL && mNpUnit.compressedContactSize > 0) || 
													(mNpUnit.compressedContacts == NULL && mNpUnit.compressedContactSize == 0));

												ccdContactData = reinterpret_cast<const PxsCCDContactHeader*>(mNpUnit.ccdContacts);
												compressedContactData = mNpUnit.compressedContacts;
												compressedContactSize = mNpUnit.compressedContactSize;
												forces = PxcNpWorkUnitGetContactForcesDC_Safe(mNpUnit);
												return mNpUnit.contactCount;
											}

						PxsRigidBody*							getRigidBody0() const { return mRigidBody0;}
						PxsRigidBody*							getRigidBody1() const { return mRigidBody1;}

private:
						//KS - moving this up - we want to get at flags
						PxsRigidBody*		mRigidBody0;					//100		//144
						PxsRigidBody*		mRigidBody1;					//104		//152	

						PxU32				mFlags;							//108		//156
						void*				mUserData;						//112		//164

						friend class PxsContext;
	// everything required for narrow phase to run
	PX_ALIGN_PREFIX(16)	PxcNpWorkUnit		mNpUnit PX_ALIGN_SUFFIX(16);	


						

	enum
	{
		PXS_CM_CHANGEABLE				= (1<<0),
		PXS_CM_CCD_LINEAR = (1<<1),
		PXS_CM_CCD_CONTACT = (1 << 2)
	};

	friend class PxsDynamicsContext;
	friend struct PxsCCDPair;
	friend class PxsIslandManager;
	friend class PxsCCDContext;
};

PX_INLINE PxU32 PxsContactManager::getConstraintDesc(PxcSolverConstraintDesc& desc)	const
{
	setConstraintLength(desc, mNpUnit.solverConstraintSize);			
	desc.constraint	= mNpUnit.solverConstraintPointer;
	PxU32 writeBackLength;
	void* writeBack = PxcNpWorkUnitGetContactForcesDC_Safe(mNpUnit, writeBackLength);
	desc.writeBack = writeBack;
	setWritebackLength(desc, writeBackLength);

	return mNpUnit.axisConstraintCount;
}


PX_INLINE PxU32 PxsContactManager::getFrictionConstraintDesc(PxcSolverConstraintDesc& desc)	const
{
	PxcSolverContactCoulombHeader* header = (PxcSolverContactCoulombHeader*)mNpUnit.solverConstraintPointer;
	if(header)
	{
		PxU32 frictionOffset = header->frictionOffset;
		PxU8* PX_RESTRICT constraint =  (PxU8*)header + frictionOffset;
		const PxU32 length = (mNpUnit.solverConstraintSize - frictionOffset);
		setConstraintLength(desc, length);
		desc.constraint	= constraint;

		//desc.writeBack	= mNpUnit.contactForces;
		setWritebackLength(desc, getHasContactForces() ? mNpUnit.contactCount * sizeof(PxReal) : 0);

		PX_ASSERT(*constraint < PXS_SC_CONSTRAINT_TYPE_COUNT);
		return PxU32(header->type != PXS_SC_TYPE_NOFRICTION_RB_CONTACT ? mNpUnit.axisConstraintCount : 0);
	}
	else
	{
		desc.constraint = NULL;
		setConstraintLength(desc, 0);
		setWritebackLength(desc, 0);
		return 0;
	}
}

}

#endif
