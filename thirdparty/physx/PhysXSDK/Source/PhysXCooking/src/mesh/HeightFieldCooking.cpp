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

#include "GuHeightField.h"
#include "PxIO.h"
#include "GuSerialize.h"

using namespace physx;
using namespace Gu;

namespace physx
{

bool saveHeightField(const HeightField& hf, PxOutputStream& stream, bool endian)
{
	// write header
	if(!writeHeader('H', 'F', 'H', 'F', PX_HEIGHTFIELD_VERSION, endian, stream))
		return false;

	const Gu::HeightFieldData& hfData = hf.getData();

	// write mData members
	writeDword(hfData.rows, endian, stream);
	writeDword(hfData.columns, endian, stream);
	writeFloat(hfData.rowLimit, endian, stream);
	writeFloat(hfData.colLimit, endian, stream);
	writeFloat(hfData.nbColumns, endian, stream);
	writeFloat(hfData.thickness, endian, stream);
	writeFloat(hfData.convexEdgeThreshold, endian, stream);
	writeWord(hfData.flags, endian, stream);
	writeDword(hfData.format, endian, stream);

	writeFloat(hfData.mAABB.minimum.x, endian, stream);
	writeFloat(hfData.mAABB.minimum.y, endian, stream);
	writeFloat(hfData.mAABB.minimum.z, endian, stream);
	writeFloat(hfData.mAABB.maximum.x, endian, stream);
	writeFloat(hfData.mAABB.maximum.y, endian, stream);
	writeFloat(hfData.mAABB.maximum.z, endian, stream);
	writeDword(hfData.rowsPadded, endian, stream);
	writeDword(hfData.columnsPadded, endian, stream);
	writeDword(hfData.tilesU, endian, stream);
	writeDword(hfData.tilesV, endian, stream);

	// write this-> members
	writeDword(hf.mSampleStride, endian, stream);
	writeDword(hf.mNbSamples, endian, stream);
	writeFloat(hf.mMinHeight, endian, stream);
	writeFloat(hf.mMaxHeight, endian, stream);

	// write samples
	for(PxU32 i = 0; i < hf.mNbSamples; i++)
	{
		const PxHeightFieldSample& s = hfData.samples[i];
		writeWord((PxU16)s.height, endian, stream);
		stream.write(&s.materialIndex0, sizeof(s.materialIndex0));
		stream.write(&s.materialIndex1, sizeof(s.materialIndex1));
	}

	return true;
}

}
