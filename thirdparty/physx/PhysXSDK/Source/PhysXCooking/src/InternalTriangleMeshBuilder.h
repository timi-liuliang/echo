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


#ifndef PX_COLLISION_INTERNALTRIANGLEMESHBUILDER
#define PX_COLLISION_INTERNALTRIANGLEMESHBUILDER

#include "GuInternalTriangleMesh.h"
#include "cooking/PxCooking.h"

namespace physx
{

	namespace Gu
	{
		class EdgeListBuilder;
	}

	// PT: we can't inherit from InternalTriangleMesh for this builder since it's aggregated in other classes! And of course I
	// don't want to change that just because of cooking. 
	class InternalTriangleMeshBuilder
	{
		public:
											InternalTriangleMeshBuilder(InternalTriangleMesh* mesh, const PxCookingParams& params);
											~InternalTriangleMeshBuilder();

				bool						cleanMesh(bool validate);
				void						fillRemapTable();	
				void						remapTopology(const PxU32* order);
				bool						createRTree();
		
				void						createSharedEdgeData(bool buildAdjacencies, bool buildActiveEdges);
				InternalTriangleMesh*		mesh;

	private:
				InternalTriangleMeshBuilder& operator=(const InternalTriangleMeshBuilder&);
				Gu::EdgeListBuilder*		edgeList;
				const PxCookingParams&		mParams;

				void						releaseEdgeList();
				void						createEdgeList();
	};

}

#endif
