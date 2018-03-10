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
         
#include "PxcContactTraceLineCallback.h"
#include "PxcContactMethodImpl.h"
#include "PxcNpCache.h"
#include "GuGeometryUnion.h"
#include "GuHeightFieldUtil.h"
#include "CmRenderBuffer.h"
#include "GuContactBuffer.h"
#include "GuContactMethodImpl.h"

using namespace physx;

#define DISTANCE_BASED_TEST

#ifdef __SPU__
extern unsigned char HeightFieldBuffer[sizeof(Gu::HeightField)+16];
#include "CmMemFetch.h"
#endif


#ifndef __SPU__
/////////
#if 0
	#include "CmRenderOutput.h"
	#include "PxsContext.h"
	static void gVisualizeBox(const Gu::Box& box, PxcNpThreadContext& context, PxU32 color=0xffffff)
	{
		PxMat33 rot(box.rot.column0, box.rot.column1, box.rot.column2);
		PxMat44 m(rot, box.center);

		Cm::DebugBox db(box.extents);

		Cm::RenderOutput& out = context.mRenderOutput;
		out << color << m;
		out << db;
	}
	static void gVisualizeLine(const PxVec3& a, const PxVec3& b, PxcNpThreadContext& context, PxU32 color=0xffffff)
	{
		PxMat44 m = PxMat44(PxIdentity);

		Cm::RenderOutput& out = context.mRenderOutput;
		out << color << m << Cm::RenderOutput::LINES << a << b;
	}
#endif
/////////
#endif

// ptchernev TODO: make sure these are ok before shipping
static const bool gCompileBoxVertex         = true;
static const bool gCompileEdgeEdge          = true;
static const bool gCompileHeightFieldVertex = true;

static const PxReal signs[24] = 
{
	-1,-1,-1,
	-1,-1, 1,
	-1, 1,-1,
	-1, 1, 1,
	1,-1,-1,
	1,-1, 1,
	1, 1,-1,
	1, 1, 1,
};

static const PxU8 edges[24] = 
{
	0,1,
	1,3,
	3,2,
	2,0,
	4,5,
	5,7,
	7,6,
	6,4,
	0,4,
	1,5,
	2,6,
	3,7,
};

static bool PxcDepenetrateBox(	const PxVec3& point, 
								const PxVec3& safeNormal, 
								const PxVec3& dimensions, 
								float /*contactDistance*/,
								PxVec3& normal, 
								PxReal& distance)
{
	PxVec3 faceNormal(PxReal(0));
	PxReal distance1 = -PX_MAX_REAL; // cant be more
	PxReal distance2 = -PX_MAX_REAL; // cant be more
	PxI32 poly1 = -1;
	PxI32 poly2 = -2;

	for (PxU32 poly = 0; poly < 6; poly++)
	{
		PxU32 dim = poly % 3;

		PxReal sign = (poly > 2) ? -PxReal(1) : PxReal(1);

		PxVec3 n(PxVec3(0));

		n[dim] = sign;
		PxReal proj = n[dim] * safeNormal[dim];
		PxReal d    = n[dim] * (point[dim] - sign * dimensions[dim]);

#ifdef DISTANCE_BASED_TEST
		// PT: I'm not really sure about contactDistance here
		// AP: enabling this causes jitter in DE2740
		//d -= contactDistance;
#endif

		if (d >= 0)
			return false;

		if (proj > 0)
		{
			if (d > distance1) // less penetration
			{
				distance1 = d;
				faceNormal = n;
				poly1 = (PxI32)poly;
			}

			// distance2 / d = 1 / proj 
			PxReal tmp = d / proj;
			if (tmp > distance2)
			{
				distance2 = tmp;
				poly2 = (PxI32)poly;
			}
		}
	}

	if (poly1 == poly2)
	{
		PX_ASSERT(faceNormal.magnitudeSquared() != 0.0f);
		normal = faceNormal;
		distance = -distance1;
	}
	else
	{
		normal = safeNormal;
		distance = -distance2;
	}

	return true;
}

//Box-Heightfield and Convex-Heightfield do not support positive values for contactDistance,
//and if in this case we would emit contacts normally, we'd cause things to jitter.
//as a workaround we add contactDistance to the distance values that we emit in contacts.
//this has the effect that the biasing will work exactly as if we had specified a legacy skinWidth of (contactDistance - restDistance)

namespace physx
{
bool PxcContactBoxHeightFieldUnified(CONTACT_METHOD_ARGS)
{

	return Gu::contactBoxHeightfield(shape0, shape1, transform0, transform1, contactDistance, (Gu::Cache&)npCache, contactBuffer);
}
bool PxcContactBoxHeightField(CONTACT_METHOD_ARGS)
{
	PX_UNUSED(npCache);
	//PXC_WARN_ONCE(contactDistance > 0.0f, "PxcContactBoxHeightField: Box-Heightfield does not support distance based contact generation! Ignoring contactOffset > 0!");

	// Get actual shape data
	const PxBoxGeometry& shapeBox = shape0.get<const PxBoxGeometry>();
	const PxHeightFieldGeometryLL& hfGeom = shape1.get<const PxHeightFieldGeometryLL>();

#ifdef __SPU__
	const Gu::HeightField& hf = *Cm::memFetchAsync<const Gu::HeightField>(HeightFieldBuffer, Cm::MemFetchPtr(static_cast<Gu::HeightField*>(hfGeom.heightField)), sizeof(Gu::HeightField), 1);
	Cm::memFetchWait(1);
#if HF_TILED_MEMORY_LAYOUT
	g_sampleCache.init((uintptr_t)(hf.getData().samples), hf.getData().tilesU);
#endif
#else
	const Gu::HeightField& hf = *static_cast<Gu::HeightField*>(hfGeom.heightField);
#endif
	const Gu::HeightFieldUtil hfUtil(hfGeom, hf);

	PX_ASSERT(contactBuffer.count==0);

	Cm::Matrix34 boxShapeAbsPose(transform0);
	Cm::Matrix34 hfShapeAbsPose(transform1);

	PxMat33 left( hfShapeAbsPose.base0, hfShapeAbsPose.base1, hfShapeAbsPose.base2);
	PxMat33 right( boxShapeAbsPose.base0, boxShapeAbsPose.base1, boxShapeAbsPose.base2);

	Cm::Matrix34 boxShape2HfShape(left.getInverse()* right, left.getInverse()*(boxShapeAbsPose.base3 - hfShapeAbsPose.base3));

	// Get box vertices.
	PxVec3 boxVertices[8];
	PxVec3 boxVertexNormals[8];
	for(PxU32 i=0; i<8; i++) 
	{
		boxVertices[i] = PxVec3(shapeBox.halfExtents.x*signs[3*i], shapeBox.halfExtents.y*signs[3*i+1], shapeBox.halfExtents.z*signs[3*i+2]);

		boxVertexNormals[i] = PxVec3(signs[3*i], signs[3*i+1], signs[3*i+2]);
		boxVertexNormals[i].normalize();
	}

	// Transform box vertices to HeightFieldShape space.
	PxVec3 boxVerticesInHfShape[8];
	PxVec3 boxVertexNormalsInHfShape[8];
	for(PxU32 i=0; i<8; i++) 
	{
		boxVerticesInHfShape[i] = boxShape2HfShape.transform(boxVertices[i]);
		boxVertexNormalsInHfShape[i] = boxShape2HfShape.rotate(boxVertexNormals[i]);
	}

	// bounds of box on HeightField.
	PxVec3 aabbMin(boxVerticesInHfShape[0]);
	PxVec3 aabbMax(boxVerticesInHfShape[0]);
	for(PxU32 i=1; i<8; i++) 
	{
		for(PxU32 dim = 0; dim < 3; dim++)
		{
			aabbMin[dim] = PxMin(aabbMin[dim], boxVerticesInHfShape[i][dim]);
			aabbMax[dim] = PxMax(aabbMax[dim], boxVerticesInHfShape[i][dim]);
		}
	}

	const bool thicknessNegOrNull = (hf.getThicknessFast() <= 0.0f);	// PT: don't do this each time! FCMPs are slow.

	// Compute the height field extreme over the bounds area.
//	PxReal hfExtreme = thicknessNegOrNull ? -PX_MAX_REAL : PX_MAX_REAL;
	// PT: we already computed those!
//	const PxReal oneOverRowScale = 1.0f / shapeHeightField.rowScale;
//	const PxReal oneOverColumnScale = 1.0f / shapeHeightField.columnScale;
	const PxReal oneOverRowScale = hfUtil.getOneOverRowScale();
	const PxReal oneOverColumnScale = hfUtil.getOneOverColumnScale();

	PxU32 minRow;
	PxU32 maxRow;
	PxU32 minColumn;
	PxU32 maxColumn;

	if (hfGeom.rowScale < 0)
	{
		minRow = hf.getMinRow(aabbMax.x * oneOverRowScale);
		maxRow = hf.getMaxRow(aabbMin.x * oneOverRowScale);
	}
	else
	{
		minRow = hf.getMinRow(aabbMin.x * oneOverRowScale);
		maxRow = hf.getMaxRow(aabbMax.x * oneOverRowScale);
	}

	if (hfGeom.columnScale < 0)
	{
		minColumn = hf.getMinColumn(aabbMax.z * oneOverColumnScale);
		maxColumn = hf.getMaxColumn(aabbMin.z * oneOverColumnScale);
	}
	else
	{
		minColumn = hf.getMinColumn(aabbMin.z * oneOverColumnScale);
		maxColumn = hf.getMaxColumn(aabbMax.z * oneOverColumnScale);
	}

	PxReal hfExtreme = hf.computeExtreme(minRow, maxRow, minColumn, maxColumn);

	//hfExtreme *= hfShape.getHeightScale();
	hfExtreme *= hfGeom.heightScale;

	// Return if convex is on the wrong side of the extreme.
	if (thicknessNegOrNull)
	{
		if (aabbMin.y > hfExtreme) return false;
	}
	else
	{
		if (aabbMax.y < hfExtreme) return false;
	}

	// Test box vertices.
	if (gCompileBoxVertex)
	{
		for(PxU32 i=0; i<8; i++)
		{
// PT: orientation culling
if(PX_SIR(boxVertexNormalsInHfShape[i].y)>0)
	continue;


			const PxVec3& boxVertexInHfShape = boxVerticesInHfShape[i];

#if 0
	PxVec3 pt = boxShapeAbsPose.transform(boxVertices[i]);
	PxVec3 worldNormal = boxShapeAbsPose.rotate(boxVertexNormals[i]);
	//gVisualizeLine(pt, pt+PxVec3(1.0f,0.0f,0.0f), context, PxDebugColor::eARGB_RED);
	//gVisualizeLine(pt, pt+PxVec3(0.0f,1.0f,0.0f), context, PxDebugColor::eARGB_GREEN);
	//gVisualizeLine(pt, pt+PxVec3(0.0f,0.0f,1.0f), context, PxDebugColor::eARGB_BLUE);
	gVisualizeLine(pt, pt+worldNormal, context, PxDebugColor::eARGB_MAGENTA);
#endif

//////// SAME CODE AS IN CONVEX-HF
			const bool insideExtreme =
				thicknessNegOrNull ? (boxVertexInHfShape.y < hfExtreme + contactDistance) : (boxVertexInHfShape.y > hfExtreme-contactDistance);

			//if (insideExtreme && hfShape.isShapePointOnHeightField(boxVertexInHfShape.x, boxVertexInHfShape.z))
			if (insideExtreme && hfUtil.isShapePointOnHeightField(boxVertexInHfShape.x, boxVertexInHfShape.z))
			{
				//PxReal y = hfShape.getHeightAtShapePoint(boxVertexInHfShape.x, boxVertexInHfShape.z);
//				const PxReal y = hfUtil.getHeightAtShapePoint(boxVertexInHfShape.x, boxVertexInHfShape.z);

				// PT: compute this once, reuse results (3 times!)
				// PT: TODO: also reuse this in EE tests
				PxReal fracX, fracZ;
				const PxU32 vertexIndex = hfUtil.getHeightField().computeCellCoordinates(
					boxVertexInHfShape.x * oneOverRowScale, boxVertexInHfShape.z * oneOverColumnScale, fracX, fracZ);

				const PxReal y = hfUtil.getHeightAtShapePoint2(vertexIndex, fracX, fracZ);

				const PxReal dy = boxVertexInHfShape.y - y;
#ifdef DISTANCE_BASED_TEST
				if (hf.isDeltaHeightInsideExtent(dy, contactDistance))
#else
				if (hf.isDeltaHeightInsideExtent(dy))
#endif
				{
					const PxU32 faceIndex = hfUtil.getFaceIndexAtShapePointNoTest2(vertexIndex, fracX, fracZ);
					if (faceIndex != 0xffffffff)
					{
//						PxcMaterialIndex material = hfShape.getTriangleMaterial(feature);
						PxVec3 n;
						//n = hfShape.getNormalAtShapePoint(boxVertexInHfShape.x, boxVertexInHfShape.z);
//						n = hfUtil.getNormalAtShapePoint(boxVertexInHfShape.x, boxVertexInHfShape.z);
						n = hfUtil.getNormalAtShapePoint2(vertexIndex, fracX, fracZ);
						n = n.getNormalized();

						contactBuffer
#ifdef DISTANCE_BASED_TEST
							.contact(boxShapeAbsPose.transform(boxVertices[i]), hfShapeAbsPose.rotate(n), n.y*dy/* + contactDistance*/, PXC_CONTACT_NO_FACE_INDEX, faceIndex);
#else
							.contact(boxShapeAbsPose.transform(boxVertices[i]), hfShapeAbsPose.rotate(n), n.y*dy + contactDistance, PXC_CONTACT_NO_FACE_INDEX, feature);//add contactDistance to compensate for fact that we don't support dist based contacts! See comment at start of funct.
#endif
					}
				}
			}
////////~SAME CODE AS IN CONVEX-HF
		}
	}

	// Test box edges.
	if (gCompileEdgeEdge)
	{
		for(PxU32 i=0; i<12; i++) 
		{
// PT: orientation culling
//float worldNormalY = (boxVertexNormalsInHfShape[edges[2*i]].y + boxVertexNormalsInHfShape[edges[2*i+1]].y)*0.5f;
float worldNormalY = boxVertexNormalsInHfShape[edges[2*i]].y + boxVertexNormalsInHfShape[edges[2*i+1]].y;
if(worldNormalY>0.0f)
	continue;


			const PxVec3& v0 = boxVerticesInHfShape[edges[2*i]];
			const PxVec3& v1 = boxVerticesInHfShape[edges[2*i+1]];

#if 0
	PxVec3 pt0 = boxShapeAbsPose.transform(boxVertices[edges[2*i]]);
	PxVec3 pt1 = boxShapeAbsPose.transform(boxVertices[edges[2*i+1]]);

	PxVec3 worldNormal0 = boxShapeAbsPose.rotate(boxVertexNormals[edges[2*i]]);
	PxVec3 worldNormal1 = boxShapeAbsPose.rotate(boxVertexNormals[edges[2*i+1]]);

	PxVec3 pt = (pt0 + pt1)*0.5f;
	PxVec3 worldNormal = (worldNormal0 + worldNormal1)*0.5f;

	gVisualizeLine(pt, pt+worldNormal, context, PxDebugColor::eARGB_CYAN);
#endif



			if (hf.getThicknessFast())	// PT: errr...? not the same test as in the convex code?
			{
				if ((v0.y > hfExtreme) && (v1.y > hfExtreme)) continue;
			}
			else
			{
				if ((v0.y < hfExtreme) && (v1.y < hfExtreme)) continue;
			}
			PxcContactTraceSegmentCallback cb(v1 - v0,
				contactBuffer,
				hfShapeAbsPose, contactDistance/*, context.mRenderOutput*/);

			//context.mRenderOutput << PxVec3(1,0,0) << Gu::Debug::convertToPxMat44(transform1)
			//	<< Cm::RenderOutput::LINES << v0+PxVec3(0.01f) << v1+PxVec3(0.01f);
			//hfShape.traceSegment<PxcContactTraceSegmentCallback>(v0, v1, &cb);
			MPT_SET_CONTEXT("cobh", transform1, PxMeshScale());
			hfUtil.traceSegment<PxcContactTraceSegmentCallback, true, true, false>(v0, v1, &cb);
		}
	}

	// Test HeightField vertices.
	if (gCompileHeightFieldVertex)
	{
		// Iterate over all HeightField vertices inside the bounds.
		for(PxU32 row = minRow; row <= maxRow; row++)
		{
			for(PxU32 column = minColumn; column <= maxColumn; column++)
			{
				const PxU32 vertexIndex = row * hf.getNbColumnsFast() + column;

				//if (!hfShape.isCollisionVertex(vertexIndex)) continue;
				if (!hfUtil.isCollisionVertex(vertexIndex, row, column)) continue;

				// Check if hf vertex is inside the box.
				//PxVec3 hfVertex;
				//hfVertex.set(hfShape.getRowScale() * row, hfShape.getHeightScale() * hfShape.getHeight(vertexIndex), hfShape.getColumnScale() * column);
//				const PxVec3 hfVertex(shapeHeightField.rowScale * row, shapeHeightField.columnScale * hfShape.getHeight(vertexIndex), shapeHeightField.columnScale * column);
				const PxVec3 hfVertex(hfGeom.rowScale * row, hfGeom.heightScale * hf.getHeight(vertexIndex), hfGeom.columnScale * column);

				const PxVec3 hfVertexInBoxShape = boxShape2HfShape.transformTranspose(hfVertex);
				if ((PxAbs(hfVertexInBoxShape.x) - shapeBox.halfExtents.x - contactDistance < 0)
					&& (PxAbs(hfVertexInBoxShape.y) - shapeBox.halfExtents.y - contactDistance < 0)
					&& (PxAbs(hfVertexInBoxShape.z) - shapeBox.halfExtents.z - contactDistance < 0))
				{

					// ptchernev: should have done this in HeightFieldShape
					// check if this really is a collision vertex
					//PxVec3 hfVertexNormal = thicknessNegOrNull ? hfShape.getVertexNormal(vertexIndex) : -hfShape.getVertexNormal(vertexIndex);
//					PxVec3 hfVertexNormal = thicknessNegOrNull ? hfUtil.getVertexNormal(vertexIndex) : -hfUtil.getVertexNormal(vertexIndex);
					const PxVec3 nrm = hfUtil.getVertexNormal(vertexIndex, row, column);
					PxVec3 hfVertexNormal = thicknessNegOrNull ? nrm : -nrm;
					hfVertexNormal = hfVertexNormal.getNormalized();
					const PxVec3 hfVertexNormalInBoxShape = boxShape2HfShape.rotateTranspose(hfVertexNormal);
					PxVec3 normal;
					PxReal depth;
					if (!PxcDepenetrateBox(hfVertexInBoxShape, -hfVertexNormalInBoxShape, shapeBox.halfExtents, contactDistance, normal, depth)) 
					{
						continue;
					}
					
//					PxMat33 rot(boxShape2HfShape[0],boxShape2HfShape[1],boxShape2HfShape[2]);
//					PxVec3 normalInHfShape = rot * (-normal);
					PxVec3 normalInHfShape = boxShape2HfShape.rotate(-normal);
					//hfShape.clipShapeNormalToVertexVoronoi(normalInHfShape, vertexIndex);
					hfUtil.clipShapeNormalToVertexVoronoi(normalInHfShape, vertexIndex, row, column);
					if (normalInHfShape.dot(hfVertexNormal) < PX_EPS_REAL) 
					{
						// hmm, I dont think this can happen
						continue;
					}
					normalInHfShape = normalInHfShape.getNormalized();
					const PxU32 faceIndex = hfUtil.getVertexFaceIndex(vertexIndex, row, column);
					contactBuffer
						.contact(hfShapeAbsPose.transform(hfVertex), hfShapeAbsPose.rotate(normalInHfShape),
#ifdef DISTANCE_BASED_TEST
						-depth,
#else
						-depth + contactDistance,	//add contactDistance to compensate for fact that we don't support dist based contacts! See comment at start of funct.
#endif
						PXC_CONTACT_NO_FACE_INDEX, faceIndex);
				}
			}
		}
	}

	return contactBuffer.count > 0;
}
}
