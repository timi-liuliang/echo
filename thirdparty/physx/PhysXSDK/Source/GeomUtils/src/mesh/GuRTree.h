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

#ifndef GU_RTREE_H
#define GU_RTREE_H

#include "PxSimpleTypes.h"
#include "PxVec4.h"
#include "PxBounds3.h"
#include "PsUserAllocated.h" // for PxSerializationContext
#include "PxSerialFramework.h"

#define RTREE_PAGE_SIZE 4 // changing this number will affect the mesh format
PX_COMPILE_TIME_ASSERT(RTREE_PAGE_SIZE == 4 || RTREE_PAGE_SIZE == 8); // using the low 5 bits for storage of index(childPtr) for dynamic rtree

#define RTREE_MINIMUM_BOUNDS_EPSILON	1e-4F

namespace physx
{

class PxInputStream;
class PxOutputStream;

using namespace physx::shdfnd;

namespace Gu {
	
	class Box;
	struct RTreePage;

	typedef PxF32 RTreeValue;

	/////////////////////////////////////////////////////////////////////////
	// quantized untransposed RTree node - used for offline build and dynamic insertion
	struct RTreeNodeQ
	{
		RTreeValue minx, miny, minz, maxx, maxy, maxz;
		PxU32 ptr; // lowest bit is leaf flag

		PX_FORCE_INLINE void setLeaf(bool set) { if (set) ptr |= 1; else ptr &= ~1; }
		PX_FORCE_INLINE PxU32 isLeaf() const { return ptr & 1; }
		PX_FORCE_INLINE void setEmpty();
		PX_FORCE_INLINE void grow(const RTreePage& page, int nodeIndex);
		PX_FORCE_INLINE void grow(const RTreeNodeQ& node);
	};

	/////////////////////////////////////////////////////////////////////////
	// RTreePage data structure, holds RTreePage::SIZE transposed nodes

	// RTreePage data structure, holds 8 transposed nodes
	PX_ALIGN_PREFIX(16)
	struct RTreePage
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		enum { SIZE = RTREE_PAGE_SIZE };
		static const RTreeValue MN, MX;
		RTreeValue minx[SIZE]; // [min=MX, max=MN] is used as a sentinel range for empty bounds
		RTreeValue miny[SIZE];
		RTreeValue minz[SIZE];
		RTreeValue maxx[SIZE];
		RTreeValue maxy[SIZE];
		RTreeValue maxz[SIZE];
		PxU32 ptrs[SIZE]; // for static rtree this is an offset relative to the first page divided by 16, for dynamics it's an absolute pointer divided by 16

		PX_FORCE_INLINE PxU32	nodeCount() const; // returns the number of occupied nodes in this page
		PX_FORCE_INLINE void	setEmpty(PxU32 startIndex = 0);
		PX_FORCE_INLINE void	copyNode(PxU32 targetIndex, const RTreePage& sourcePage, PxU32 sourceIndex);
		PX_FORCE_INLINE void	setNode(PxU32 targetIndex, const RTreeNodeQ& node);
		PX_FORCE_INLINE void	clearNode(PxU32 nodeIndex);
		PX_FORCE_INLINE void	getNode(PxU32 nodeIndex, RTreeNodeQ& result) const;
		PX_FORCE_INLINE void	computeBounds(RTreeNodeQ& bounds);
		PX_FORCE_INLINE void	adjustChildBounds(PxU32 index, const RTreeNodeQ& adjustedChildBounds);
		PX_FORCE_INLINE void	growChildBounds(PxU32 index, const RTreeNodeQ& adjustedChildBounds);
		PX_FORCE_INLINE PxU32	getNodeHandle(PxU32 index) const;
		PX_FORCE_INLINE PxU32	isLeaf(PxU32 index) const { return ptrs[index] & 1; }
	} PX_ALIGN_SUFFIX(16);

	/////////////////////////////////////////////////////////////////////////
	// RTree root data structure
#ifdef PX_VC
#pragma warning(push)
#pragma warning(disable: 4324)	// Padding was added at the end of a structure because of a __declspec(align) value.
#pragma warning(disable: 4251 ) // class needs to have dll-interface to be used by clients of class
#endif

	PX_ALIGN_PREFIX(16)
	struct PX_PHYSX_COMMON_API RTree
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================
		// PX_SERIALIZATION
		RTree(const PxEMPTY&);
		void	exportExtraData(PxSerializationContext&);
		void	importExtraData(PxDeserializationContext& context);
		static	void	getBinaryMetaData(PxOutputStream& stream);
		//~PX_SERIALIZATION

		RTree(); // offline static rtree constructor used with cooking

		~RTree() { release(); }

		void release();
		bool save(PxOutputStream& stream) const; // always saves as big endian
		bool load(PxInputStream& stream, PxU32 meshVersion); // converts to proper endian at load time

		////////////////////////////////////////////////////////////////////////////
		// QUERIES
		struct Callback
		{
			// result buffer should have room for at least RTreePage::SIZE items
			// should return true to continue traversal. If false is returned, traversal is aborted
			virtual bool processResults(PxU32 count, PxU32* buf) = 0;
			virtual void profile() {}
            virtual ~Callback() {}
		};

		struct CallbackRaycast
		{
			// result buffer should have room for at least RTreePage::SIZE items
			// should return true to continue traversal. If false is returned, traversal is aborted
			// newMaxT serves as both input and output, as input it's the maxT so far
			// set it to a new value (which should be smaller) and it will become the new far clip t
			virtual bool processResults(PxU32 count, PxU32* buf, PxF32& newMaxT) = 0;
            virtual ~CallbackRaycast() {}
		};

		// callback will be issued as soon as the buffer overflows maxResultsPerBlock-RTreePage:SIZE entries
		// use maxResults = RTreePage:SIZE and return false from callback for "first hit" early out
		void		traverseAABB(
						const PxVec3& boxMin, const PxVec3& boxMax,
						const PxU32 maxResultsPerBlock, PxU32* resultsBlockBuf, Callback* processResultsBlockCallback) const;
		void		traverseOBB(
						const Gu::Box& obb,
						const PxU32 maxResultsPerBlock, PxU32* resultsBlockBuf, Callback* processResultsBlockCallback) const;
		template <int inflate>
		//PX_PHYSX_COMMON_API
		void		traverseRay(
						const PxVec3& rayOrigin, const PxVec3& rayDir, // dir doesn't have to be normalized and is B-A for raySegment
						const PxU32 maxResults, PxU32* resultsPtr,
						Gu::RTree::CallbackRaycast* callback,
						const PxVec3* inflateAABBs, // inflate tree's AABBs by this amount. This function turns into AABB sweep.
						PxF32 maxT = PX_MAX_REAL // maximum ray t parameter, p(t)=origin+t*dir; use 1.0f for ray segment
						) const;

		////////////////////////////////////////////////////////////////////////////
		// DEBUG HELPER FUNCTIONS
		void		validate(); // verify that all children are indeed included in parent bounds
		void		openTextDump();
		void		closeTextDump();
		void		textDump(const char* prefix);
		void		maxscriptExport();
		PxU32		computeBottomLevelCount(PxU32 storedToMemMultiplier) const;

		////////////////////////////////////////////////////////////////////////////
		// DATA
		// remember to update save() and load() when adding or removing data
		PxVec4			mBoundsMin, mBoundsMax, mInvDiagonal, mDiagonalScaler;
		PxU32			mPageSize;
		PxU32			mNumRootPages;
		PxU32			mNumLevels;
		PxU32			mTotalNodes;
		PxU32			mTotalPages;
		PxU32			mFlags; enum { USER_ALLOCATED = 0x1, IS_DYNAMIC = 0x2 };
		PxU32			mUnused;
		RTreePage*		mPages;

		static PxU32	mVersion;

	protected:
		typedef PxU32 NodeHandle;
		void		validateRecursive(PxU32 level, RTreeNodeQ parentBounds, RTreePage* page);

		// has to be aligned to page size
		PX_FORCE_INLINE
		RTreePage*	getPageFromNodeHandle(NodeHandle nh) const { return pagePtrFrom32Bits(nh&~(sizeof(RTreePage)-1)); }

		PX_FORCE_INLINE
		PxU32		getNodeIdxFromNodeHandle(NodeHandle nh) const { return nh & (RTreePage::SIZE-1); }

		static PX_FORCE_INLINE
		PxU32		pagePtrTo32Bits(const RTreePage* page);

		static PX_FORCE_INLINE
		RTreePage*	pagePtrFrom32Bits(PxU32 page);
		PX_FORCE_INLINE

		RTreePage*	get64BitBasePage() const;
		#ifdef PX_P64
		static RTreePage* sFirstPoolPage;
		#endif // PX_P64

		friend struct RTreePage;
	} PX_ALIGN_SUFFIX(16);
#ifdef PX_VC
#pragma warning(pop)
#endif

	/////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE PxU32 RTree::pagePtrTo32Bits(const RTreePage* page)
	{
		#ifdef PX_P64
			PX_ASSERT(PxU64(page) >= PxU64(sFirstPoolPage));
			PxU64 delta = PxU64(page)-PxU64(sFirstPoolPage);
			PX_ASSERT(delta <= 0xFFFFffff);
			return PxU32(delta);
		#else
			return PxU32(page);
		#endif //PX_P64
	}

	/////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE RTreePage* RTree::pagePtrFrom32Bits(PxU32 page)
	{
		#ifdef PX_P64
			return reinterpret_cast<RTreePage*>(PxU64(sFirstPoolPage)+page);
		#else
			return reinterpret_cast<RTreePage*>(page);
		#endif //PX_P64
	}

	/////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE RTreePage* RTree::get64BitBasePage() const
	{
		#ifdef PX_P64
			if (mFlags & IS_DYNAMIC)
				return sFirstPoolPage;
			else
				return mPages;
		#else
			return mPages;
		#endif
	}

	// explicit instantiations for traverseRay
	// XXX: dima: g++ 4.4 won't compile this => skipping by PX_LINUX
#if (defined(PX_X86) || defined(PX_X360)) && !(defined(PX_LINUX) || defined(PX_APPLE) || defined(PX_ANDROID))
	template
	//PX_PHYSX_COMMON_API
	void RTree::traverseRay<0>(
		const PxVec3&, const PxVec3&, const PxU32, PxU32*, Gu::RTree::CallbackRaycast*, const PxVec3*, PxF32 maxT) const;
	template
	//PX_PHYSX_COMMON_API
	void RTree::traverseRay<1>(
		const PxVec3&, const PxVec3&, const PxU32, PxU32*, Gu::RTree::CallbackRaycast*, const PxVec3*, PxF32 maxT) const;
#endif

	/////////////////////////////////////////////////////////////////////////
	PX_FORCE_INLINE void RTreeNodeQ::setEmpty()
	{
		minx = miny = minz = RTreePage::MX;
		maxx = maxy = maxz = RTreePage::MN;
	}


	// AP: former contents of OPC_ModelData.h
	class MeshInterface;

	// bit 1 is always expected to be set to differentiate between leaf and non-leaf node
	PX_FORCE_INLINE PxU32 LeafGetNbTriangles(PxU32 Data) { return ((Data>>1) & 15)+1; }
	PX_FORCE_INLINE PxU32 LeafGetTriangleIndex(PxU32 Data) { return Data>>5; }
	PX_FORCE_INLINE PxU32 LeafSetData(PxU32 nb, PxU32 index)
	{
		PX_ASSERT(nb>0 && nb<=16); PX_ASSERT(index < (1<<27));
		return (index<<5)|(((nb-1)&15)<<1) | 1;
	}

	struct LeafTriangles
	{
	//= ATTENTION! =====================================================================================
	// Changing the data layout of this class breaks the binary serialization format.  See comments for 
	// PX_BINARY_SERIAL_VERSION.  If a modification is required, please adjust the getBinaryMetaData 
	// function.  If the modification is made on a custom branch, please change PX_BINARY_SERIAL_VERSION
	// accordingly.
	//==================================================================================================

		PxU32			Data;

		// Gets number of triangles in the leaf, returns the number of triangles N, with 0 < N <= 16
		PX_FORCE_INLINE	PxU32	GetNbTriangles()				const	{ return LeafGetNbTriangles(Data); }

		// Gets triangle index for this leaf. Indexed model's array of indices retrieved with RTreeMidphase::GetIndices()
		PX_FORCE_INLINE	PxU32	GetTriangleIndex()				const	{ return LeafGetTriangleIndex(Data); }
		PX_FORCE_INLINE	void	SetData(PxU32 nb, PxU32 index)			{ Data = LeafSetData(nb, index); }
	};

	struct RTreeMidphaseData 
	{
		const MeshInterface*	mIMesh;
		const Gu::RTree*		mRTree;
		PxReal					mGeomEpsilon;
	};

	PX_COMPILE_TIME_ASSERT(sizeof(LeafTriangles)==4); // RTree has space for 4 bytes

} // namespace Gu

}

#endif // #ifdef PX_COLLISION_RTREE
