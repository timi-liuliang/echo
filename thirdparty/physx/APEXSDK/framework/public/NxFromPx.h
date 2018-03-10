/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_FROM_PX_H
#define NX_FROM_PX_H

/*!
\file
\brief Conversion utilities between PhysX 2.8 and 3.0 data types
*/

#include "NxApexDefs.h"

#if NX_SDK_VERSION_MAJOR == 2
#include "NxVec3.h"
#include "NxMat33.h"
#include "NxMat34.h"
#include "NxStream.h"
#include "NxBounds3.h"

#include "foundation/PxQuat.h"
#include "PxFileBuf.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include "geometry/PxConvexMesh.h"
#include "PxFileBuf.h"
#include "foundation/PxIO.h"
#endif

#include "foundation/PxVec3.h"
#include "foundation/PxBounds3.h"
#include "PxMat33Legacy.h"
#include "PxMat34Legacy.h"

namespace physx
{
namespace apex
{

PX_PUSH_PACK_DEFAULT

#if NX_SDK_VERSION_MAJOR == 2
//! \brief Convert vector
PX_INLINE void PxFromNxVec3(physx::PxVec3& out, const NxVec3& in)
{
	out = physx::PxVec3(in.x, in.y, in.z);
}
//! \brief Convert vector
PX_INLINE physx::PxVec3 PxFromNxVec3(const NxVec3& in)
{
	return physx::PxVec3(in.x, in.y, in.z);
}
//! \brief Convert vector
PX_INLINE void NxFromPxVec3(NxVec3& out, const physx::PxVec3& in)
{
	out.set(in.x, in.y, in.z);
}
//! \brief Convert vector
PX_INLINE NxVec3 NxFromPxVec3(const physx::PxVec3& in)
{
	return NxVec3(in.x, in.y, in.z);
}
//! \brief Convert quaternion
PX_INLINE void PxFromNxQuat(physx::PxQuat& out, const NxQuat& in)
{
	out = physx::PxQuat(in.x, in.y, in.z, in.w);
}
//! \brief Convert quaternion
PX_INLINE void NxFromPxQuat(NxQuat& out, const physx::PxQuat& in)
{
	out.setXYZW(in.x, in.y, in.z, in.w);
}
//! \brief Convert 3x3 matrix
PX_INLINE void PxFromNxMat33(physx::PxMat33Legacy& out, const NxMat33& in)
{
	physx::PxF32 buf[9];
	in.getRowMajor(buf);
	out.setRowMajor(buf);
}
//! \brief Convert 3x3 matrix
PX_INLINE void PxFromNxMat33(physx::PxMat33& out, const NxMat33& in)
{
	physx::PxF32 buf[9];
	in.getRowMajor(buf);
	out = physx::PxMat33(buf);
}
//! \brief Convert 3x3 matrix
PX_INLINE void NxFromPxMat33(NxMat33& out, const physx::PxMat33Legacy& in)
{
	physx::PxF32 buf[9];
	in.getRowMajor(buf);
	out.setRowMajor(buf);
}
//! \brief Convert 3x3 matrix
PX_INLINE void PxFromNxMat34(physx::PxMat34Legacy& out, const NxMat34& in)
{
	PxFromNxMat33(out.M, in.M);
	PxFromNxVec3(out.t, in.t);
}

//! \brief Convert 3x4 matrix
PX_INLINE void PxFromNxMat34(physx::PxMat44& out, const NxMat34& in)
{
	physx::PxMat34Legacy tmp;
	PxFromNxMat34(tmp, in);
	out = physx::PxMat44(tmp);
}
//! \brief Convert 3x4 matrix
PX_INLINE void NxFromPxMat34(NxMat34& out, const physx::PxMat34Legacy& in)
{
	NxFromPxMat33(out.M, in.M);
	NxFromPxVec3(out.t, in.t);
}
//! \brief Convert bounds
PX_INLINE void PxFromNxBounds3(physx::PxBounds3& out, const NxBounds3& in)
{
	PxFromNxVec3(out.minimum, in.min);
	PxFromNxVec3(out.maximum, in.max);
}
//! \brief Convert bounds
PX_INLINE void NxFromPxBounds3(NxBounds3& out, const physx::PxBounds3& in)
{
	NxFromPxVec3(out.min, in.minimum);
	NxFromPxVec3(out.max, in.maximum);
}
//! \brief Convert array to quaternion
PX_INLINE void ArrayFromPxQuat(physx::PxF32 arr[4], const physx::PxQuat& q)
{
	arr[0] = q.x;
	arr[1] = q.y;
	arr[2] = q.z;
	arr[3] = q.w;
}

//! \brief Convert vector (fast version)
PX_INLINE NxVec3& NxFromPxVec3Fast(physx::PxVec3& in)
{
	return reinterpret_cast<NxVec3&>(in);
}
//! \brief Convert vector (fast version)
PX_INLINE const NxVec3& NxFromPxVec3Fast(const physx::PxVec3& in)
{
	return reinterpret_cast<const NxVec3&>(in);
}
//! \brief Convert vector (fast version)
PX_INLINE physx::PxVec3& PxFromNxVec3Fast(NxVec3& in)
{
	return reinterpret_cast<physx::PxVec3&>(in);
}
//! \brief Convert vector (fast version)
PX_INLINE const physx::PxVec3& PxFromNxVec3Fast(const NxVec3& in)
{
	return reinterpret_cast<const physx::PxVec3&>(in);
}
//! \brief Convert quaternion (fast version)
PX_INLINE NxQuat& NxFromPxQuatFast(physx::PxQuat& in)
{
	return reinterpret_cast<NxQuat&>(in);
}
//! \brief Convert quaternion (fast version)
PX_INLINE const NxQuat& NxFromPxQuatFast(const physx::PxQuat& in)
{
	return reinterpret_cast<const NxQuat&>(in);
}
//! \brief Convert quaternion (fast version)
PX_INLINE physx::PxQuat& PxFromNxQuatFast(NxQuat& in)
{
	return reinterpret_cast<physx::PxQuat&>(in);
}
//! \brief Convert quaternion (fast version)
PX_INLINE const physx::PxQuat& PxFromNxQuatFast(const NxQuat& in)
{
	return reinterpret_cast<const physx::PxQuat&>(in);
}
//! \brief Convert 4x4 matrix
PX_INLINE void NxFromPxMat44(NxMat34& out, const physx::PxMat44& in)
{
	out.setColumnMajor44(in.front());
}

//! \brief Convert to Px vector
#define PXFROMNXVEC3(nxv) physx::PxVec3((nxv).x, (nxv).y, (nxv).z)
//! \brief Convert to Nx vector
#define NXFROMPXVEC3(pxv) NxVec3((pxv).x, (pxv).y, (pxv).z)

#endif // NX_SDK_VERSION_MAJOR == 2


/**
\brief A wrapper class that provides an NxStream API for a physx::general_PxIOStream2::PxFileBuf
*/
class NxFromPxStream : 
#if NX_SDK_VERSION_MAJOR == 2	
	public NxStream
#elif NX_SDK_VERSION_MAJOR == 3
	public physx::PxInputStream, public physx::PxOutputStream
#endif
{
private:
	physx::general_PxIOStream2::PxFileBuf& mPx;
	NxFromPxStream& operator=(const NxFromPxStream&)
	{
		return *this;
	}

public:
	//! \brief Constructor that takes a PxFileBuf
	NxFromPxStream(physx::general_PxIOStream2::PxFileBuf& px) : mPx(px) {}

#if NX_SDK_VERSION_MAJOR == 2
	//! \brief Read a byte from the stream
	physx::PxU8		readByte()								const
	{
		return mPx.readByte();
	}

	//! \brief Read a two byte word from the stream
	physx::PxU16	readWord()								const
	{
		return mPx.readWord();
	}

	//! \brief Read a four byte word from the stream
	physx::PxU32	readDword()								const
	{
		return mPx.readDword();
	}

	//! \brief Read a float from the stream
	physx::PxF32	readFloat()								const
	{
		return mPx.readFloat();
	}

	//! \brief Read a double from the stream
	physx::PxF64	readDouble()							const
	{
		return mPx.readDouble();
	}

	//! \brief Read a chunk of data from the stream
	void			readBuffer(void* buffer, physx::PxU32 size)	const
	{
		mPx.read(buffer, size);
	}

	//! \brief Write a byte to the stream
	NxStream&		storeByte(physx::PxU8 b)
	{
		mPx.storeByte(b);
		return *this;
	}

	//! \brief Write a two byte word to the stream
	NxStream&		storeWord(physx::PxU16 w)
	{
		mPx.storeWord(w);
		return *this;
	}

	//! \brief Write a four byte word to the stream
	NxStream&		storeDword(physx::PxU32 d)
	{
		mPx.storeDword(d);
		return *this;
	}

	//! \brief Write a float to the stream
	NxStream&		storeFloat(physx::PxF32 f)
	{
		mPx.storeFloat(f);
		return *this;
	}

	//! \brief Write a double to the stream
	NxStream&		storeDouble(physx::PxF64 f)
	{
		mPx.storeDouble(f);
		return *this;
	}

	//! \brief Write a chunk of data to the stream
	NxStream&		storeBuffer(const void* buffer, physx::PxU32 size)
	{
		mPx.write(buffer, size);
		return *this;
	}

#endif

	//! \brief Read data from the stream
	physx::PxU32	read(void* dest, PxU32 count)
	{
		return mPx.read(dest, count);
	}

	//! \brief Read data to the stream
	physx::PxU32 write(const void* src, PxU32 count)
	{
		return mPx.write(src, count);
	}
};


//! \brief Replacement for NxBounds3::transform()
PX_INLINE void PxBounds3boundsOfOBB(physx::PxBounds3& b, const physx::PxMat33Legacy& orientation, const physx::PxVec3& translation, const physx::PxVec3& halfDims)
{
	physx::PxF32 dimx = halfDims[0];
	physx::PxF32 dimy = halfDims[1];
	physx::PxF32 dimz = halfDims[2];

	physx::PxF32 x = physx::PxAbs(orientation(0, 0) * dimx) + physx::PxAbs(orientation(0, 1) * dimy) + physx::PxAbs(orientation(0, 2) * dimz);
	physx::PxF32 y = physx::PxAbs(orientation(1, 0) * dimx) + physx::PxAbs(orientation(1, 1) * dimy) + physx::PxAbs(orientation(1, 2) * dimz);
	physx::PxF32 z = physx::PxAbs(orientation(2, 0) * dimx) + physx::PxAbs(orientation(2, 1) * dimy) + physx::PxAbs(orientation(2, 2) * dimz);

	physx::PxVec3 minimum(-x + translation[0], -y + translation[1], -z + translation[2]);
	physx::PxVec3 maximum(x + translation[0], y + translation[1], z + translation[2]);
	b = physx::PxBounds3(minimum, maximum);
}

//! \brief Replacement for NxBounds3::transform()
PX_INLINE void PxBounds3Transform(physx::PxBounds3& b, const physx::PxMat33Legacy& orientation, const physx::PxVec3& translation)
{
	// convert to center and extents form
	physx::PxVec3 center, extents;
	center = b.getCenter();
	extents = b.getExtents();

	center = orientation * center + translation;
	PxBounds3boundsOfOBB(b, orientation, center, extents);
}

//! \brief Replacement for NxBounds3::transform()
PX_INLINE void PxBounds3boundsOfOBB(physx::PxBounds3& b, const physx::PxMat44& tm, const physx::PxVec3& center, const physx::PxVec3& halfDims)
{
	physx::PxF32 dimx = halfDims[0];
	physx::PxF32 dimy = halfDims[1];
	physx::PxF32 dimz = halfDims[2];

	physx::PxF32 x = physx::PxAbs(tm(0, 0) * dimx) + physx::PxAbs(tm(0, 1) * dimy) + physx::PxAbs(tm(0, 2) * dimz);
	physx::PxF32 y = physx::PxAbs(tm(1, 0) * dimx) + physx::PxAbs(tm(1, 1) * dimy) + physx::PxAbs(tm(1, 2) * dimz);
	physx::PxF32 z = physx::PxAbs(tm(2, 0) * dimx) + physx::PxAbs(tm(2, 1) * dimy) + physx::PxAbs(tm(2, 2) * dimz);

	physx::PxVec3 minimum(-x + center[0], -y + center[1], -z + center[2]);
	physx::PxVec3 maximum(x + center[0], y + center[1], z + center[2]);
	b = physx::PxBounds3(minimum, maximum);
}

//! \brief Replacement for NxBounds3::transform()
PX_INLINE void PxBounds3Transform(physx::PxBounds3& b, const physx::PxMat44& tm)
{
	// convert to center and extents form
	physx::PxVec3 center, extents;
	center = b.getCenter();
	extents = b.getExtents();

	center = tm.transform(center);
	PxBounds3boundsOfOBB(b, tm, center, extents);
}

//! \brief Replacement for NxVec3::equals
PX_INLINE bool PxVec3equals(const physx::PxVec3& a, const physx::PxVec3& v, physx::PxF32 epsilon)
{
	return
	    physx::PxEquals(a.x, v.x, epsilon) &&
	    physx::PxEquals(a.y, v.y, epsilon) &&
	    physx::PxEquals(a.z, v.z, epsilon);
}

/** \brief Get vector from array of floats */
PX_INLINE void PxVec3FromArray(physx::PxVec3& out, const physx::PxF32 arr[3])
{
	out = physx::PxVec3(arr[0], arr[1], arr[2]);
}

/** \brief Get vector from array of floats */
PX_INLINE void PxVec3FromArray(physx::PxVec3& out, const physx::PxF64 arr[3])
{
	out = physx::PxVec3((physx::PxF32)arr[0], (physx::PxF32)arr[1], (physx::PxF32)arr[2]);
}

/** \brief Get quat from array of floats */
PX_INLINE void PxQuatFromArray(physx::PxQuat& out, const physx::PxF32 arr[4])
{
	out.x = arr[0];
	out.y = arr[1];
	out.z = arr[2];
	out.w = arr[3];
}

PX_POP_PACK

}
} // end namespace physx::apex

#endif // NX_FROM_PX_H
