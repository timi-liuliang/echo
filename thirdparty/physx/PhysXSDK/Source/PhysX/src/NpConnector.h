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


#ifndef PX_PHYSICS_NP_CONNECTOR
#define PX_PHYSICS_NP_CONNECTOR

#include "CmPhysXCommon.h"
#include "PsInlineArray.h"
#include "PxSerialFramework.h"
#include "CmUtils.h"

namespace physx
{

struct NpConnectorType
{
	enum Enum
	{
		eConstraint,
		eAggregate,
		eObserver,
		eInvalid
	};
};


class NpConnector
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
	NpConnector() : mType(NpConnectorType::eInvalid), mObject(NULL) {}
	NpConnector(NpConnectorType::Enum type, PxBase* object) : mType(Ps::to8(type)), mObject(object) {}
// PX_SERIALIZATION
	NpConnector(const NpConnector& c)
	{
		//special copy constructor that initializes padding bytes for meta data verification (PX_CHECKED only)		
		Cm::markSerializedMem(this, sizeof(NpConnector));
		mType = c.mType;
		mObject = c.mObject;
	}

	static	void	getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION

	PxU8				mType;			// Revisit whether the type is really necessary or whether the serializable type is enough.
										// Since joints might gonna inherit from observers to register for constraint release events, the type
										// is necessary because a joint has its own serializable type and could not be detected as observer anymore.
	PxU8				mPadding[3];	// PT: padding from prev byte
	PxBase*		mObject;		// So far the serialization framework only supports ptr resolve for PxBase objects.
										// However, so far the observers all are PxBase, hence this choice of type.
};


class NpConnectorIterator
{
public:
	PX_FORCE_INLINE NpConnectorIterator(NpConnector* c, PxU32 size, NpConnectorType::Enum type) : mConnectors(c), mSize(size), mIndex(0), mType(type) {}

	PX_FORCE_INLINE PxBase* getNext()
	{
		PxBase* s = NULL;
		while(mIndex < mSize)
		{
			NpConnector& c = mConnectors[mIndex];
			mIndex++;
			if (c.mType == mType)
				return c.mObject;
		}

		return s;
	}

private:
	NpConnector* mConnectors;
	PxU32 mSize;
	PxU32 mIndex;
	NpConnectorType::Enum mType;
};


class NpConnectorArray: public Ps::InlineArray<NpConnector, 4> 
{
//= ATTENTION! =====================================================================================
// Changing the data layout of this class breaks the binary serialization format.  See comments for 
// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
// accordingly.
//==================================================================================================
public:
// PX_SERIALIZATION
	NpConnectorArray(const PxEMPTY&) : Ps::InlineArray<NpConnector, 4> (PxEmpty) {}
	static	void	getBinaryMetaData(PxOutputStream& stream);
//~PX_SERIALIZATION
	NpConnectorArray() : Ps::InlineArray<NpConnector, 4>(PX_DEBUG_EXP("connectorArray")) 
	{
		//special default constructor that initializes padding bytes for meta data verification (PX_CHECKED only)
		Cm::markSerializedMem(this->mData, 4*sizeof(NpConnector));
	}
};

}

#endif
