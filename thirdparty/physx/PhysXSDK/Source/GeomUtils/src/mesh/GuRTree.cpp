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

#define RTREE_TEXT_DUMP_ENABLE		0
#if defined(PX_P64)
#define RTREE_PAGES_PER_POOL_SLAB	16384 // preallocate all pages in first batch to make sure we stay within 32 bits for relative pointers.. this is 2 megs
#else
#define RTREE_PAGES_PER_POOL_SLAB	128
#endif

#define INSERT_SCAN_LOOKAHEAD		1 // enable one level lookahead scan for determining which child page is best to insert a node into

#include "GuRTree.h"
#include "PsSort.h"
#include "GuSerialize.h"
#include "CmMemFetch.h"
#include "CmUtils.h"

using namespace physx;
using Ps::Array;
using Ps::sort;
using namespace Gu;

namespace physx
{
namespace Gu {

/////////////////////////////////////////////////////////////////////////
#ifdef PX_X360
#define CONVERT_PTR_TO_INT PxU32
#else
#define CONVERT_PTR_TO_INT PxU64
#endif

#ifdef PX_P64
RTreePage* RTree::sFirstPoolPage = NULL; // used for relative addressing on 64-bit platforms
#endif

/////////////////////////////////////////////////////////////////////////
RTree::RTree()
{
	PX_ASSERT((Cm::MemFetchPtr(this) & 15) == 0);
	mFlags = 0;
	mPages = NULL;
	mTotalNodes = 0;
	mNumLevels = 0;
	mPageSize = RTreePage::SIZE;
}

/////////////////////////////////////////////////////////////////////////
PxU32 RTree::mVersion = 1;

bool RTree::save(PxOutputStream& stream) const
{
	// save the RTree root structure followed immediately by RTreePage pages to an output stream
	bool mismatch = (littleEndian() == 1);
	writeChunk('R', 'T', 'R', 'E', stream);
	writeDword(mVersion, mismatch, stream);
	writeFloatBuffer(&mBoundsMin.x, 4, mismatch, stream);
	writeFloatBuffer(&mBoundsMax.x, 4, mismatch, stream);
	writeFloatBuffer(&mInvDiagonal.x, 4, mismatch, stream);
	writeFloatBuffer(&mDiagonalScaler.x, 4, mismatch, stream);
	writeDword(mPageSize, mismatch, stream);
	writeDword(mNumRootPages, mismatch, stream);
	writeDword(mNumLevels, mismatch, stream);
	writeDword(mTotalNodes, mismatch, stream);
	writeDword(mTotalPages, mismatch, stream);
	writeDword(mUnused, mismatch, stream);
	for (PxU32 j = 0; j < mTotalPages; j++)
	{
		writeFloatBuffer(mPages[j].minx, RTreePage::SIZE, mismatch, stream);
		writeFloatBuffer(mPages[j].miny, RTreePage::SIZE, mismatch, stream);
		writeFloatBuffer(mPages[j].minz, RTreePage::SIZE, mismatch, stream);
		writeFloatBuffer(mPages[j].maxx, RTreePage::SIZE, mismatch, stream);
		writeFloatBuffer(mPages[j].maxy, RTreePage::SIZE, mismatch, stream);
		writeFloatBuffer(mPages[j].maxz, RTreePage::SIZE, mismatch, stream);
		WriteDwordBuffer(mPages[j].ptrs, RTreePage::SIZE, mismatch, stream);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
bool RTree::load(PxInputStream& stream, PxU32 meshVersion)
{
	PX_ASSERT((mFlags & IS_DYNAMIC) == 0);
	PX_UNUSED(meshVersion);

	release();

	PxI8 a, b, c, d;
	readChunk(a, b, c, d, stream);
	if(a!='R' || b!='T' || c!='R' || d!='E')
		return false;

	bool mismatch = (littleEndian() == 1);
	if(readDword(mismatch, stream) != mVersion)
		return false;

	readFloatBuffer(&mBoundsMin.x, 4, mismatch, stream);
	readFloatBuffer(&mBoundsMax.x, 4, mismatch, stream);
	readFloatBuffer(&mInvDiagonal.x, 4, mismatch, stream);
	readFloatBuffer(&mDiagonalScaler.x, 4, mismatch, stream);
	mPageSize = readDword(mismatch, stream);
	mNumRootPages = readDword(mismatch, stream);
	mNumLevels = readDword(mismatch, stream);
	mTotalNodes = readDword(mismatch, stream);
	mTotalPages = readDword(mismatch, stream);
	mUnused = readDword(mismatch, stream);

	mPages = static_cast<RTreePage*>(
		Ps::AlignedAllocator<128>().allocate(sizeof(RTreePage)*mTotalPages, __FILE__, __LINE__));
	Cm::markSerializedMem(mPages, sizeof(RTreePage)*mTotalPages);
	for (PxU32 j = 0; j < mTotalPages; j++)
	{
		readFloatBuffer(mPages[j].minx, RTreePage::SIZE, mismatch, stream);
		readFloatBuffer(mPages[j].miny, RTreePage::SIZE, mismatch, stream);
		readFloatBuffer(mPages[j].minz, RTreePage::SIZE, mismatch, stream);
		readFloatBuffer(mPages[j].maxx, RTreePage::SIZE, mismatch, stream);
		readFloatBuffer(mPages[j].maxy, RTreePage::SIZE, mismatch, stream);
		readFloatBuffer(mPages[j].maxz, RTreePage::SIZE, mismatch, stream);
		ReadDwordBuffer(mPages[j].ptrs, RTreePage::SIZE, mismatch, stream);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////
PxU32 RTree::computeBottomLevelCount(PxU32 multiplier) const
{
	PX_ASSERT((mFlags & IS_DYNAMIC) == 0);
	PxU32 topCount = 0, curCount = mNumRootPages;
	const RTreePage* rightMostPage = &mPages[mNumRootPages-1];
	PX_ASSERT(rightMostPage);
	for (PxU32 level = 0; level < mNumLevels-1; level++)
	{
		topCount += curCount;
		PxU32 nc = rightMostPage->nodeCount();
		PX_ASSERT(nc > 0 && nc <= RTreePage::SIZE);
		// old version pointer, up to PX_MESH_VERSION 8
		PxU32 ptr = (rightMostPage->ptrs[nc-1]) * multiplier;
		PX_ASSERT(ptr % sizeof(RTreePage) == 0);
		const RTreePage* rightMostPageNext = mPages + (ptr / sizeof(RTreePage));
		curCount = PxU32(rightMostPageNext - rightMostPage);
		rightMostPage = rightMostPageNext;
	}

	return mTotalPages - topCount;
}

/////////////////////////////////////////////////////////////////////////
RTree::RTree(const PxEMPTY&)
{
	mFlags |= USER_ALLOCATED;
}

/////////////////////////////////////////////////////////////////////////
void RTree::release()
{
	if ((mFlags & USER_ALLOCATED) == 0 && mPages)
	{
		Ps::AlignedAllocator<128>().deallocate(mPages);
		mPages = NULL;
	}
}

// PX_SERIALIZATION
/////////////////////////////////////////////////////////////////////////
void RTree::exportExtraData(PxSerializationContext& stream)
{
	stream.alignData(128);
	stream.writeData(mPages, mTotalPages*sizeof(RTreePage));
}

/////////////////////////////////////////////////////////////////////////
void RTree::importExtraData(PxDeserializationContext& context)
{
	context.alignExtraData(128);
	mPages = context.readExtraData<RTreePage>(mTotalPages);
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE PxU32 RTreePage::nodeCount() const
{
	for (int j = 0; j < RTreePage::SIZE; j ++)
		if (minx[j] == MX)
			return (PxU32)j;

	return RTreePage::SIZE;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::clearNode(PxU32 nodeIndex)
{
	PX_ASSERT(nodeIndex < RTreePage::SIZE);
	minx[nodeIndex] = miny[nodeIndex] = minz[nodeIndex] = MX; // initialize empty node with sentinels
	maxx[nodeIndex] = maxy[nodeIndex] = maxz[nodeIndex] = MN;
	ptrs[nodeIndex] = 0;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::getNode(const PxU32 nodeIndex, RTreeNodeQ& r) const
{
	PX_ASSERT(nodeIndex < RTreePage::SIZE);
	r.minx = minx[nodeIndex];
	r.miny = miny[nodeIndex];
	r.minz = minz[nodeIndex];
	r.maxx = maxx[nodeIndex];
	r.maxy = maxy[nodeIndex];
	r.maxz = maxz[nodeIndex];
	r.ptr  = ptrs[nodeIndex];
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE PxU32 RTreePage::getNodeHandle(PxU32 index) const
{
	PX_ASSERT(index < RTreePage::SIZE);
	PX_ASSERT(CONVERT_PTR_TO_INT(reinterpret_cast<size_t>(this)) % sizeof(RTreePage) == 0);
	PxU32 result = (RTree::pagePtrTo32Bits(this) | index);
	return result;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::setEmpty(PxU32 startIndex)
{
	PX_ASSERT(startIndex < RTreePage::SIZE);
	for (PxU32 j = startIndex; j < RTreePage::SIZE; j ++)
		clearNode(j);
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::computeBounds(RTreeNodeQ& newBounds)
{
	RTreeValue _minx = MX, _miny = MX, _minz = MX, _maxx = MN, _maxy = MN, _maxz = MN;
	for (int j = 0; j < RTreePage::SIZE; j++)
	{
		if (minx[j] > maxx[j])
			continue;
		_minx = PxMin(_minx, minx[j]);
		_miny = PxMin(_miny, miny[j]);
		_minz = PxMin(_minz, minz[j]);
		_maxx = PxMax(_maxx, maxx[j]);
		_maxy = PxMax(_maxy, maxy[j]);
		_maxz = PxMax(_maxz, maxz[j]);
	}
	newBounds.minx = _minx;
	newBounds.miny = _miny;
	newBounds.minz = _minz;
	newBounds.maxx = _maxx;
	newBounds.maxy = _maxy;
	newBounds.maxz = _maxz;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::adjustChildBounds(PxU32 index, const RTreeNodeQ& adjChild)
{
	PX_ASSERT(index < RTreePage::SIZE);
	minx[index] = adjChild.minx;
	miny[index] = adjChild.miny;
	minz[index] = adjChild.minz;
	maxx[index] = adjChild.maxx;
	maxy[index] = adjChild.maxy;
	maxz[index] = adjChild.maxz;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::growChildBounds(PxU32 index, const RTreeNodeQ& child)
{
	PX_ASSERT(index < RTreePage::SIZE);
	minx[index] = PxMin(minx[index], child.minx);
	miny[index] = PxMin(miny[index], child.miny);
	minz[index] = PxMin(minz[index], child.minz);
	maxx[index] = PxMax(maxx[index], child.maxx);
	maxy[index] = PxMax(maxy[index], child.maxy);
	maxz[index] = PxMax(maxz[index], child.maxz);
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::copyNode(PxU32 targetIndex, const RTreePage& sourcePage, PxU32 sourceIndex)
{
	PX_ASSERT(targetIndex < RTreePage::SIZE);
	PX_ASSERT(sourceIndex < RTreePage::SIZE);
	minx[targetIndex] = sourcePage.minx[sourceIndex];
	miny[targetIndex] = sourcePage.miny[sourceIndex];
	minz[targetIndex] = sourcePage.minz[sourceIndex];
	maxx[targetIndex] = sourcePage.maxx[sourceIndex];
	maxy[targetIndex] = sourcePage.maxy[sourceIndex];
	maxz[targetIndex] = sourcePage.maxz[sourceIndex];
	ptrs[targetIndex] = sourcePage.ptrs[sourceIndex];
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreePage::setNode(PxU32 targetIndex, const RTreeNodeQ& sourceNode)
{
	PX_ASSERT(targetIndex < RTreePage::SIZE);
	minx[targetIndex] = sourceNode.minx;
	miny[targetIndex] = sourceNode.miny;
	minz[targetIndex] = sourceNode.minz;
	maxx[targetIndex] = sourceNode.maxx;
	maxy[targetIndex] = sourceNode.maxy;
	maxz[targetIndex] = sourceNode.maxz;
	ptrs[targetIndex] = sourceNode.ptr;
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreeNodeQ::grow(const RTreePage& page, int nodeIndex)
{
	PX_ASSERT(nodeIndex < RTreePage::SIZE);
	minx = PxMin(minx, page.minx[nodeIndex]);
	miny = PxMin(miny, page.miny[nodeIndex]);
	minz = PxMin(minz, page.minz[nodeIndex]);
	maxx = PxMax(maxx, page.maxx[nodeIndex]);
	maxy = PxMax(maxy, page.maxy[nodeIndex]);
	maxz = PxMax(maxz, page.maxz[nodeIndex]);
}

/////////////////////////////////////////////////////////////////////////
PX_FORCE_INLINE void RTreeNodeQ::grow(const RTreeNodeQ& node)
{
	minx = PxMin(minx, node.minx); miny = PxMin(miny, node.miny); minz = PxMin(minz, node.minz);
	maxx = PxMax(maxx, node.maxx); maxy = PxMax(maxy, node.maxy); maxz = PxMax(maxz, node.maxz);
}

/////////////////////////////////////////////////////////////////////////
void RTree::validateRecursive(PxU32 level, RTreeNodeQ parentBounds, RTreePage* page)
{
	PX_UNUSED(parentBounds);

	static PxU32 validateCounter = 0; // this is to suppress a warning that recursive call has no side effects
	validateCounter++;

	RTreeNodeQ n;
	PxU32 pageNodeCount = page->nodeCount();
	for (PxU32 j = 0; j < pageNodeCount; j++)
	{
		page->getNode(j, n);
		if (n.minx > n.maxx)
			continue;
		PX_ASSERT(n.minx >= parentBounds.minx); PX_ASSERT(n.miny >= parentBounds.miny); PX_ASSERT(n.minz >= parentBounds.minz);
		PX_ASSERT(n.maxx <= parentBounds.maxx); PX_ASSERT(n.maxy <= parentBounds.maxy); PX_ASSERT(n.maxz <= parentBounds.maxz);
		if (!n.isLeaf())
		{
			PX_ASSERT((n.ptr&1) == 0);
            RTreePage* childPage = (RTreePage*)(size_t(CONVERT_PTR_TO_INT(reinterpret_cast<size_t>(get64BitBasePage()))) + n.ptr);
			validateRecursive(level+1, n, childPage);
		}
	}
	RTreeNodeQ recomputedBounds;
	page->computeBounds(recomputedBounds);
	PX_ASSERT(recomputedBounds.minx == parentBounds.minx);
	PX_ASSERT(recomputedBounds.miny == parentBounds.miny);
	PX_ASSERT(recomputedBounds.minz == parentBounds.minz);
	PX_ASSERT(recomputedBounds.maxx == parentBounds.maxx);
	PX_ASSERT(recomputedBounds.maxy == parentBounds.maxy);
	PX_ASSERT(recomputedBounds.maxz == parentBounds.maxz);
}

/////////////////////////////////////////////////////////////////////////
void RTree::validate()
{
	for (PxU32 j = 0; j < mNumRootPages; j++)
	{
		RTreeNodeQ rootBounds;
		mPages[j].computeBounds(rootBounds);
		validateRecursive(0, rootBounds, mPages+j);
	}
}

//~PX_SERIALIZATION
const RTreeValue RTreePage::MN = -PX_MAX_REAL;
const RTreeValue RTreePage::MX = PX_MAX_REAL;

} // namespace Gu

}
