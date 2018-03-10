/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef PX_CONVX_OUTPUT_H
#define PX_CONVX_OUTPUT_H

namespace physx { namespace Sn {

	struct PxMetaDataEntry;
	class ConvX;
	
	typedef void	(Sn::ConvX::*ConvertCallback)	(const char* src, const PxMetaDataEntry& entry, const PxMetaDataEntry& dstEntry);

	inline_ void flip(short& v)
	{
		char* b = (char*)&v;
		char temp = b[0];
		b[0] = b[1];
		b[1] = temp;
	}

	inline_ void flip(unsigned short& v)
	{
		flip((short&)v);
	}

	inline_ void flip(int& v)
	{
		char* b = (char*)&v;

		char temp = b[0];
		b[0] = b[3];
		b[3] = temp;
		temp = b[1];
		b[1] = b[2];
		b[2] = temp;
	}

	inline_ void flip(unsigned int& v)
	{
		flip((int&)v);
	}

	inline_ void flip(long long& v)
	{
		char* b = (char*)&v;

		char temp = b[0];
		b[0] = b[7];
		b[7] = temp;
		temp = b[1];
		b[1] = b[6];
		b[6] = temp;
		temp = b[2];
		b[2] = b[5];
		b[5] = temp;
		temp = b[3];
		b[3] = b[4];
		b[4] = temp;
	}

	inline_ void flip(float& v)
	{
		flip((int&)v);
	}

	inline_ void flip(void*& v)
	{
		flip((int&)v);
	}

	inline_ void flip(const char*& v)
	{
		flip((int&)v);
	}
} }

#endif
