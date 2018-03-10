/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef APEX_INTERFACE_H
#define APEX_INTERFACE_H

#include "PsShare.h"
#include "PsUserAllocated.h"

namespace physx
{
	namespace debugger
	{
		namespace comm
		{
			class PvdDataStream;
		}
	}

namespace apex
{

/**
 *  Class defines semi-public interface to ApexResource objects
 *	Resource - gets added to a list, will be deleted when the list is deleted
 */
class NxApexResource
{
public:
	virtual void    release() = 0;
	virtual void	setListIndex(class NxResourceList& list, physx::PxU32 index) = 0;
	virtual physx::PxU32	getListIndex() const = 0;
	virtual void	initPvdInstances(physx::debugger::comm::PvdDataStream& /*pvdStream*/) {};
};

/**
Class that implements resource ID and bank
*/
class ApexResource : public physx::UserAllocated
{
public:
	ApexResource() : m_listIndex(0xFFFFFFFF), m_list(NULL) {}
	void removeSelf();
	virtual ~ApexResource();

	physx::PxU32			m_listIndex;
	class NxResourceList*	m_list;
};


/**
Initialized Template class.
*/
template <class DescType>class InitTemplate
{
	//gotta make a derived class cause of protected ctor
public:
	InitTemplate() : isSet(false) {}

	bool isSet;
	DescType data;


	void set(const DescType* desc)
	{
		if (desc)
		{
			isSet = true;
			//memcpy(this,desc, sizeof(DescType));
			data = *desc;
		}
		else
		{
			isSet = false;
		}
	}


	bool get(DescType& dest) const
	{
		if (isSet)
		{
			//memcpy(&dest,this, sizeof(DescType));
			dest = data;
			return true;
		}
		else
		{
			return false;
		}

	}
};

} // namespace apex
} // namespace physx

#endif // APEX_INTERFACE_H
