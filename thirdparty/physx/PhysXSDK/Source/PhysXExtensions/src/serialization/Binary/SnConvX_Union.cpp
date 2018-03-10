/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#include "SnConvX.h"

using namespace physx;

void Sn::ConvX::resetUnions()
{
	mUnions.clear();
}

bool Sn::ConvX::registerUnion(const char* name)
{
	displayMessage(PxErrorCode::eDEBUG_INFO, "Registering union: %s\n", name);

	Sn::Union u;
	u.mName	= name;

	mUnions.pushBack(u);
	return true;
}

bool Sn::ConvX::registerUnionType(const char* unionName, const char* typeName, int typeValue)
{
	const PxU32 nb = mUnions.size();
	for(PxU32 i=0;i<nb;i++)
	{
		if(strcmp(mUnions[i].mName, unionName)==0)
		{
			UnionType t;
			t.mTypeName		= typeName;
			t.mTypeValue	= typeValue;
			mUnions[i].mTypes.pushBack(t);			
			displayMessage(PxErrorCode::eDEBUG_INFO, "Registering union type: %s | %s | %d\n", unionName, typeName, typeValue);
			return true;
		}
	}

	displayMessage(PxErrorCode::eINTERNAL_ERROR, "PxBinaryConverter: union not found: %s, please check the source metadata.\n", unionName);
	return false;
}

const char* Sn::ConvX::getTypeName(const char* unionName, int typeValue)
{
	const PxU32 nb = mUnions.size();
	for(PxU32 i=0;i<nb;i++)
	{
		if(strcmp(mUnions[i].mName, unionName)==0)
		{
			const PxU32 nbTypes = mUnions[i].mTypes.size();
			for(PxU32 j=0;j<nbTypes;j++)
			{
				const UnionType& t = mUnions[i].mTypes[j];
				if(t.mTypeValue==typeValue)
					return t.mTypeName;
			}
			break;
		}
	}
	displayMessage(PxErrorCode::eINTERNAL_ERROR,
		"PxBinaryConverter: union type not found: %s, type %d, please check the source metadata.\n", unionName, typeValue);		
	assert(0);
	return NULL;
}
