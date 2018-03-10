/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_TEST_BASE_H
#define NX_TEST_BASE_H

#include "NxApexUsingNamespace.h"

/*!
\file
\brief An NxTestBase is an interface for managing unit tests
*/

namespace physx
{

namespace apex
{

/**
	Defines the NxUnitTestsActor API which is instantiated from an NxUnitTestsAsset
*/
class NxTestBase
{
public:

	/**
	\brief Returns the name of the test at this index.

	\param [in] unitTestsIndex : The test number to refer to; must be less than the result of getUnitTestCount
	*/
	virtual const char* getUnitTestsName(PxI32 unitTestsIndex) const = 0;

	/**
	\brief Returns the index of the test for this name.

	\param [in] unitTestsName : The test name to refer to; if there is no such name method returns -1
	*/
	virtual PxI32 getUnitTestsIndex(const char* unitTestsName) const = 0;

	/**
	\brief run unit test

	\param [in] unitTestsID : The unit test number to refer to; must be less than the result of getUnitTestsCount
	\param [in] dataPtr : The pointer to data which is needed for unit test.
	*/
	virtual bool runUnitTests(PxI32 unitTestsID, void* dataPtr) const = 0;

	/**
	\brief check unit test
	*/
	virtual bool checkUnitTests(PxI32 unitTestsID, void* dataPtr) const = 0;

	/**
	\brief returns the number of unit tests
	*/
	virtual PxU32 getUnitTestsCount() const = 0;

	/**
	\brief set level of runtime check
	\param [in] level - level of runtime check

	 level = 0 - no runtime checks
	 level = 1 - easy-weight checks. Perfomance should not drop more than 10-20%
	 level = 2 - medium-weight checks. Perfomance drop less than 300-500%
	 level = 3 - hard-weight checks. Perfomance drop isn't limited
	*/
	virtual void setRuntimeCheckLevel(PxU32 level) = 0;
};


}; // end of apex namespace
}; // end of physx namespace

#endif
