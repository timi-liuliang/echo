/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */

#ifndef __APEX_STREAM_H__
#define __APEX_STREAM_H__

#include "NxApexDefs.h"
#include "foundation/PxPlane.h"


namespace physx
{
namespace apex
{

// Public, useful operators for serializing nonversioned data follow.
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, bool& b)
{
	b = (0 != stream.readByte());
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxI8& b)
{
	b = (physx::PxI8)stream.readByte();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxI16& w)
{
	w = (physx::PxI16)stream.readWord();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxI32& d)
{
	d = (physx::PxI32)stream.readDword();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxU8& b)
{
	b = stream.readByte();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxU16& w)
{
	w = stream.readWord();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxU32& d)
{
	d = stream.readDword();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxF32& f)
{
	f = stream.readFloat();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxF64& f)
{
	f = stream.readDouble();
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec2& v)
{
	stream >> v.x >> v.y;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec3& v)
{
	stream >> v.x >> v.y >> v.z;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxVec4& v)
{
	stream >> v.x >> v.y >> v.z >> v.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxBounds3& b)
{
	stream >> b.minimum >> b.maximum;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxQuat& q)
{
	stream >> q.x >> q.y >> q.z >> q.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxPlane& p)
{
	stream >> p.n.x >> p.n.y >> p.n.z >> p.d;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator >> (physx::PxFileBuf& stream, physx::PxMat44& m)
{
	stream >> m.column0 >> m.column1 >> m.column2 >> m.column3;
	return stream;
}

// The opposite of the above operators--takes data and writes it to a stream.
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const bool b)
{
	stream.storeByte(b ? (physx::PxU8)1 : (physx::PxU8)0);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxI8 b)
{
	stream.storeByte((physx::PxU8)b);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxI16 w)
{
	stream.storeWord((physx::PxU16)w);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxI32 d)
{
	stream.storeDword((physx::PxU32)d);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxU8 b)
{
	stream.storeByte(b);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxU16 w)
{
	stream.storeWord(w);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxU32 d)
{
	stream.storeDword(d);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxF32 f)
{
	stream.storeFloat(f);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxF64 f)
{
	stream.storeDouble(f);
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec2& v)
{
	stream << v.x << v.y;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec3& v)
{
	stream << v.x << v.y << v.z;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxVec4& v)
{
	stream << v.x << v.y << v.z << v.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxBounds3& b)
{
	stream << b.minimum << b.maximum;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxQuat& q)
{
	stream << q.x << q.y << q.z << q.w;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxPlane& p)
{
	stream << p.n.x << p.n.y << p.n.z << p.d;
	return stream;
}
PX_INLINE physx::PxFileBuf& operator << (physx::PxFileBuf& stream, const physx::PxMat44& m)
{
	stream << m.column0 << m.column1 << m.column2 << m.column3;
	return stream;
}


}
} // end namespace apex

#endif // __APEX_STREAM_H__