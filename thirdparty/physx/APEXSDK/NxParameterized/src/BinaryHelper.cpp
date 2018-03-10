/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


// WARNING: before doing any changes to this file
// check comments at the head of BinSerializer.cpp

#include <ctype.h>
#include <stdio.h>

#include "BinaryHelper.h"

namespace NxParameterized
{

void dumpBytes(const char *data, physx::PxU32 nbytes)
{
	printf("Total = %d bytes\n", nbytes);

	for(physx::PxU32 i = 0; i < nbytes; i += 16)
	{
		printf("%08x: ", i);

		//Print bytes
		for(physx::PxU32 j = i; j < i + 16; ++j)
		{
			if( nbytes < j )
			{
				//Pad with whites
				for(; j < i + 16; ++j)
					printf("   ");

				break;
			}

			char c = data[j];
			printf("%02x ", c);
		}

		//Print chars
		for(physx::PxU32 j = i; j < i + 16; ++j)
		{
			if( nbytes < j )
				break;

			char c = data[j];
			printf("%c", isprint(c) ? c : '.');
		}

		printf("\n");
	}
}

void Dictionary::setOffset(const char *s, physx::PxU32 off)
{
	for(physx::PxU32 i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
		{
			entries[i].offset = off;
			return;
		}

	PX_ASSERT(0 && "String not found");
}

physx::PxU32 Dictionary::getOffset(const char *s) const
{
	for(physx::PxU32 i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
			return entries[i].offset;

	PX_ASSERT(0 && "String not found");
	return (physx::PxU32)-1;
}

void Dictionary::serialize(StringBuf &res) const
{
	res.append(Canonize(entries.size()));

	for(physx::PxU32 i = 0; i < entries.size(); ++i)
	{
		const char *s = entries[i].s;
		res.appendBytes(s, 1 + (physx::PxU32)strlen(s));
	}
}

physx::PxU32 Dictionary::put(const char *s)
{
	PX_ASSERT(s && "NULL in dictionary");

	for(physx::PxU32 i = 0; i < entries.size(); ++i)
		if( 0 == strcmp(s, entries[i].s) )
			return i;

	Entry e = {s, 0};
	entries.pushBack(e);
	return entries.size() - 1;
}

}