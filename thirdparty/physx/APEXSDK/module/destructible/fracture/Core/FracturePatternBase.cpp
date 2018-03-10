/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "RTdef.h"
#if RT_COMPILE
#include "PxMath.h"
#include "PxMat44.h"
#include "PsMathUtils.h"

#include "FracturePatternBase.h"
#include "ConvexBase.h"
#include "CompoundBase.h"
#include "MeshClipperBase.h"
#include "IslandDetectorBase.h"
#include "SimSceneBase.h"

namespace physx
{
namespace fracture
{
namespace base
{

#define HIDE_INVISIBLE_FACES 1

// --------------------------------------------------------------------------------------------
FracturePattern::FracturePattern(SimScene* scene):
	mScene(scene)
{
}

// --------------------------------------------------------------------------------------------
void FracturePattern::clear()
{
	mGeom.clear();
	mFirstConvexOfCell.clear();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::finalize()
{
	mFirstPiece.resize(mGeom.convexes.size(), -1);
	mSplitPiece.resize(mGeom.convexes.size(), false);
	mPieces.clear();

	// compute volumes of convexes
	mConvexVolumes.resize(mGeom.convexes.size(), 0.0f);
	Convex* c = mScene->createConvex();
	for (physx::PxU32 i = 0; i < mGeom.convexes.size(); i++) {
		c->createFromGeometry(mGeom, (physx::PxI32)i);
		mConvexVolumes[i] = c->getVolume();
	}
	mGeom.derivePlanes();
	PX_DELETE(c);
}

// --------------------------------------------------------------------------------------------
void FracturePattern::create3dVoronoi(const PxVec3 &dims, int numCells, float biasExp, float maxDist)
{
	clear();
	shdfnd::Array<PxVec3> points((physx::PxU32)numCells);
	float r = PxPow(dims.magnitude(), 1.0f/biasExp);
	float max2 = maxDist * maxDist;

	mBounds.minimum = -dims;
	mBounds.maximum = dims;

	for (int i = 0; i < numCells; i++) {
		PxVec3 &p = points[(physx::PxU32)i];
		do {
			p = PxVec3(shdfnd::rand(-r,r), shdfnd::rand(-r,r), shdfnd::rand(-r,r));
			float len = p.normalize();
			p *= PxPow(len, biasExp);
		} while (!mBounds.contains(p) || (maxDist < PX_MAX_F32 && p.magnitudeSquared() > max2));
	}
	// avoid numerical problems with flat quads!
	float eps = 0.001f * mBounds.getDimensions().magnitude();
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));

	Delaunay3d *d = mScene->getDelaunay3d();
	d->tetrahedralize(&points[0], (physx::PxI32)points.size(), sizeof(PxVec3), true);
	d->computeVoronoiMesh();
	mGeom = d->getGeometry();

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::createLocal3dVoronoi(const PxVec3 &dims, int numCells, float radius)
{
	clear();
	shdfnd::Array<PxVec3> points((physx::PxU32)numCells);
	float r = radius;
	float r2 = r*r;

	mBounds.minimum = -dims;
	mBounds.maximum = dims;

	for (int i = 0; i < numCells; i++) {
		PxVec3 &p = points[(physx::PxU32)i];
		do {
			p = PxVec3(shdfnd::rand(-r,r), shdfnd::rand(-r,r), shdfnd::rand(-r,r));		
		} while (!mBounds.contains(p) || p.magnitudeSquared() > r2);
	}
	// avoid numerical problems with flat quads!
	float eps = 0.001f * mBounds.getDimensions().magnitude();
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps), -dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps), -dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3( dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));
	points.pushBack(PxVec3(-dims.x + shdfnd::rand(-eps, eps),  dims.y + shdfnd::rand(-eps, eps),  dims.z + shdfnd::rand(-eps, eps)));

	Delaunay3d *d = mScene->getDelaunay3d();
	d->tetrahedralize(&points[0], (physx::PxI32)points.size(), sizeof(PxVec3), true);
	d->computeVoronoiMesh();
	mGeom = d->getGeometry();

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::createRegular3dVoronoi(const PxVec3 &dims, float pieceSize, float relRandOffset)
{
	clear();
	mBounds.minimum= -dims;
	mBounds.maximum= dims;
	shdfnd::Array<PxVec3> points;

	float rel = relRandOffset;
	if (rel < 1e-4f)
		rel = 1e-4f;		// avoid singular cases

	float off = rel * pieceSize;

	int numX = (int)(2.0f*dims.x / pieceSize) + 1;
	int numY = (int)(2.0f*dims.y / pieceSize) + 1;
	int numZ = (int)(2.0f*dims.z / pieceSize) + 1;

	for (int xi = 0; xi < numX; xi++) {
		for (int yi = 0; yi < numY; yi++) {
			for (int zi = 0; zi < numZ; zi++) {
				points.pushBack(PxVec3(
					mBounds.minimum.x + xi * pieceSize + shdfnd::rand(-off, off),
					mBounds.minimum.y + yi * pieceSize + shdfnd::rand(-off, off),
					mBounds.minimum.z + zi * pieceSize + shdfnd::rand(-off, off)));
			}
		}
	}

	Delaunay3d *d = mScene->getDelaunay3d();
	d->tetrahedralize(&points[0], (physx::PxI32)points.size(), sizeof(PxVec3), true);
	d->computeVoronoiMesh();
	mGeom = d->getGeometry();

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::addBox(const PxBounds3 &bounds)
{
	static int boxFaces[6][4] = {{0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}, {0,3,2,1}, {4,5,6,7}};
	CompoundGeometry::Convex c;
	mGeom.initConvex(c);

	c.numVerts = 8;
	mGeom.vertices.pushBack(PxVec3(bounds.minimum.x, bounds.minimum.y, bounds.minimum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.maximum.x, bounds.minimum.y, bounds.minimum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.maximum.x, bounds.maximum.y, bounds.minimum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.minimum.x, bounds.maximum.y, bounds.minimum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.minimum.x, bounds.minimum.y, bounds.maximum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.maximum.x, bounds.minimum.y, bounds.maximum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.maximum.x, bounds.maximum.y, bounds.maximum.z));
	mGeom.vertices.pushBack(PxVec3(bounds.minimum.x, bounds.maximum.y, bounds.maximum.z));

	c.numFaces = 6;
	for (int i = 0; i < 6; i++) {
		mGeom.indices.pushBack(4);		// size
		mGeom.indices.pushBack(0);		// flags
		for (int j = 0; j < 4; j++) 
			mGeom.indices.pushBack(boxFaces[i][j]);
	}
	mGeom.convexes.pushBack(c);
}

// --------------------------------------------------------------------------------------------
void FracturePattern::createCrack(const PxVec3 &dims, float spacing, float zRand)
{
	clear();

	float minZ = -1.2f * zRand;
	float maxZ =  1.2f * zRand;

	PxBounds3 bounds;
	bounds.minimum = PxVec3(-dims.x, -dims.y, -dims.z);
	bounds.maximum = PxVec3( dims.x,  dims.y, minZ);
	addBox(bounds);

	shdfnd::Array<PxVec3> points;

	mBounds.minimum = -dims;
	mBounds.maximum = dims;

	float area = 4.0f * dims.x * dims.y;
	int numCells = (int)(area / (spacing * spacing));

	for (int i = 0; i < numCells; i++) {
		PxVec3 p = PxVec3(shdfnd::rand(-dims.x, dims.x), shdfnd::rand(-dims.y,dims.y), 0.0f);
		points.pushBack(p);
	}

	Delaunay2d *d = mScene->getDelaunay2d();
	d->triangulate(&points[0], (physx::PxI32)points.size(), sizeof(PxVec3), true);
	d->computeVoronoiMesh();

	// convert to 3d
	const shdfnd::Array<Delaunay2d::Convex> &convexes = d->getConvexes();
	shdfnd::Array<PxVec3> verts = d->getConvexVerts();
	for (physx::PxU32 i = 0; i < verts.size(); i++) 
		verts[i].z += shdfnd::rand(-zRand, zRand);

	int numConvexes = (physx::PxI32)convexes.size();

	const shdfnd::Array<int> &neighbors = d->getConvexNeighbors();

	for (int side = 0; side < 2; side++) {
		for (physx::PxU32 i = 0; i < convexes.size(); i++) {
			const Delaunay2d::Convex &c2 = convexes[i];

			CompoundGeometry::Convex c3;
			mGeom.initConvex(c3);

			// neighbors
			c3.numNeighbors = c2.numNeighbors + 2;
			for (int j = 0; j < c2.numNeighbors; j++) {
				int n = neighbors[physx::PxU32(c2.firstNeighbor + j)] + 1;
				if (side == 0) 
					mGeom.neighbors.pushBack(n);
				else
					mGeom.neighbors.pushBack(numConvexes + n);
			}
			if (side == 0) {
				mGeom.neighbors.pushBack(1 + numConvexes + (physx::PxI32)i);
				mGeom.neighbors.pushBack(0);
			}
			else {
				mGeom.neighbors.pushBack(1 + (physx::PxI32)i);
				mGeom.neighbors.pushBack(1 + 2*numConvexes);
			}

			// vertices
			c3.numVerts = 2 * c2.numVerts;
			for (int j = 0; j < c2.numVerts; j++) {
				PxVec3 p = verts[physx::PxU32(c2.firstVert+j)];
				if (side == 0) {
					mGeom.vertices.pushBack(PxVec3(p.x, p.y, minZ));
					mGeom.vertices.pushBack(p);
				}
				else {
					mGeom.vertices.pushBack(p);
					mGeom.vertices.pushBack(PxVec3(p.x, p.y, maxZ));
				}
			}

			// faces
			c3.numFaces = c2.numVerts + 2;
			mGeom.indices.pushBack(c2.numVerts);
			mGeom.indices.pushBack(0);	// flags
			for (int j = 0; j < c2.numVerts; j++)
				mGeom.indices.pushBack(2*j+1);
			mGeom.indices.pushBack(c2.numVerts);
			mGeom.indices.pushBack(0);	// flags
			for (int j = c2.numVerts-1; j >= 0; j--)
				mGeom.indices.pushBack(2*j);
			for (int j = 0; j < c2.numVerts; j++) {
				mGeom.indices.pushBack(4);	// size
				mGeom.indices.pushBack(0);	// flags
				int k = (j+1)%c2.numVerts;
				mGeom.indices.pushBack(2*j);
				mGeom.indices.pushBack(2*k);
				mGeom.indices.pushBack(2*k+1);
				mGeom.indices.pushBack(2*j+1);
			}

			mGeom.convexes.pushBack(c3);
		}
	}

	bounds.minimum = PxVec3(-dims.x, -dims.y, maxZ);
	bounds.maximum = PxVec3( dims.x,  dims.y, dims.z);
	addBox(bounds);

	// two cells
	mFirstConvexOfCell.pushBack(0);
	mFirstConvexOfCell.pushBack(1 + numConvexes);
	mFirstConvexOfCell.pushBack(1 + 2*numConvexes + 1);

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::create2dVoronoi(const PxVec3 &dims, int numCells, float biasExp, int numRays)
{
	clear();
	shdfnd::Array<PxVec3> points;
	float r0 = dims.magnitude();
	float r = shdfnd::pow(r0, 1.0f/biasExp);

	mBounds.minimum= -dims;
	mBounds.maximum= dims;

	if (numRays == 0) {
		for (int i = 0; i < numCells; i++) {
			PxVec3 p;
			do {
				p = PxVec3(shdfnd::rand(-r,r), shdfnd::rand(-r,r), 0.0f);
				float len = p.normalize();
				p *= shdfnd::pow(len, biasExp);
			} while (!mBounds.contains(p));
			points.pushBack(p);
		}
	}
	else {
		float diag = mBounds.getDimensions().magnitude();
		float eps = 0.0025f * diag;

		int cellsPerRay = numCells / numRays + 1;
		float dr = 0.05f * diag / cellsPerRay;
		float dphi = PxTwoPi / numRays;

		for (int i = 0; i < numRays; i++) {
			float phi = i * dphi;
			PxVec3 n(PxCos(phi), PxSin(phi), 0.0f);

			for (int j = 0; j < cellsPerRay; j++) {
				float rj = dr * powf((float)(j+1), biasExp);
				PxVec3 p = n * rj;
				p.x += shdfnd::rand(-rj * eps, rj * eps);
				p.y += shdfnd::rand(-rj * eps, rj * eps);
				if (!mBounds.contains(p))
					break;
				points.pushBack(p);
			}
		}
	}

	float s = 1.2f;
	points.pushBack(PxVec3(-s*dims.x, -s*dims.y, 0.0f));
	points.pushBack(PxVec3( s*dims.x, -s*dims.y, 0.0f));
	points.pushBack(PxVec3( s*dims.x,  s*dims.y, 0.0f));
	points.pushBack(PxVec3(-s*dims.x,  s*dims.y, 0.0f));

	Delaunay2d *d = mScene->getDelaunay2d();
	d->triangulate(&points[0], (physx::PxI32)points.size(), sizeof(PxVec3), true);
	d->computeVoronoiMesh();

	// convert to 3d
	const shdfnd::Array<Delaunay2d::Convex> &convexes = d->getConvexes();
	const shdfnd::Array<PxVec3> &verts = d->getConvexVerts();
	mGeom.neighbors = d->getConvexNeighbors();

	mGeom.convexes.resize(convexes.size());
	for (physx::PxU32 i = 0; i < convexes.size(); i++) {
		CompoundGeometry::Convex &c3 = mGeom.convexes[i];
		const Delaunay2d::Convex &c2 = convexes[i];

		c3.firstNeighbor = c2.firstNeighbor;
		c3.numNeighbors = c2.numNeighbors;

		c3.firstVert = (physx::PxI32)mGeom.vertices.size();
		c3.numVerts = 2 * c2.numVerts;
		for (int j = 0; j < c2.numVerts; j++) {
			PxVec3 p = verts[physx::PxU32(c2.firstVert+j)];
			p.z = mBounds.minimum.z;
			mGeom.vertices.pushBack(p);
			p.z = mBounds.maximum.z;
			mGeom.vertices.pushBack(p);
		}

		c3.firstIndex = (physx::PxI32)mGeom.indices.size();
		c3.numFaces = c2.numVerts + 2;
		mGeom.indices.pushBack(c2.numVerts);
		mGeom.indices.pushBack(CompoundGeometry::FF_OBJECT_SURFACE);
		for (int j = 0; j < c2.numVerts; j++)
			mGeom.indices.pushBack(2*j+1);
		mGeom.indices.pushBack(c2.numVerts);
		mGeom.indices.pushBack(CompoundGeometry::FF_OBJECT_SURFACE);
		for (int j = c2.numVerts-1; j >= 0; j--)
			mGeom.indices.pushBack(2*j);
		for (int j = 0; j < c2.numVerts; j++) {
			mGeom.indices.pushBack(4);
			mGeom.indices.pushBack(0);
			int k = (j+1)%c2.numVerts;
			mGeom.indices.pushBack(2*j);
			mGeom.indices.pushBack(2*k);
			mGeom.indices.pushBack(2*k+1);
			mGeom.indices.pushBack(2*j+1);
		}
	}

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::createGlass(float radius, float thickness, int numSectors, float sectorRand, float firstSegmentSize, float segmentScale, float segmentRand)
{
	clear();
	CompoundGeometry::Convex c;
	static int faces[6][4] = {{0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}, {3,2,1,0}, {4,5,6,7}};

	shdfnd::Array<float> secAngles((physx::PxU32)numSectors);
	float dSec = PxTwoPi / (float)numSectors;
	for (physx::PxU32 i = 0; i < (physx::PxU32)numSectors; i++) 
		secAngles[i] = i * dSec + shdfnd::rand(-sectorRand, sectorRand) * dSec;

	// how many segments?
	float segSize = firstSegmentSize;
	float d = 0.0f;
	int numSegs = 0;
	while (d < radius) {
		numSegs++;
		d += segSize;
		segSize *= segmentScale;
	}

	for (physx::PxU32 i = 0; i < (physx::PxU32)numSectors; i++) {
		float segSize = firstSegmentSize;
		float d = 0.0f;
		float r0 = 0.0f;
		float r1 = 0.0f;
		float p0, p1;

		float a0 = secAngles[i];
		float a1 = secAngles[(i + 1) % numSectors];

		for (int j = 0; j < numSegs; j++) {
			d += segSize;
			p0 = r0;
			p1 = r1;
			r0 = d + shdfnd::rand(-segmentRand, segmentRand) * segSize;
			r1 = d + shdfnd::rand(-segmentRand, segmentRand) * segSize;
			segSize *= segmentScale;

			mGeom.initConvex(c);
			c.numFaces = 6;
			c.numNeighbors = 6;
			c.numVerts = 8;
			c.firstPlane = 0;
			mGeom.convexes.pushBack(c);

			float sin0 = PxSin(a0); 
			float sin1 = PxSin(a1); 
			float cos0 = PxCos(a0); 
			float cos1 = PxCos(a1); 

			mGeom.vertices.pushBack(PxVec3(p0 * cos0, p0 * sin0, -thickness));
			mGeom.vertices.pushBack(PxVec3(r0 * cos0, r0 * sin0, -thickness));
			mGeom.vertices.pushBack(PxVec3(r1 * cos1, r1 * sin1, -thickness));
			mGeom.vertices.pushBack(PxVec3(p1 * cos1, p1 * sin1, -thickness));
			mGeom.vertices.pushBack(PxVec3(p0 * cos0, p0 * sin0, thickness));
			mGeom.vertices.pushBack(PxVec3(r0 * cos0, r0 * sin0, thickness));
			mGeom.vertices.pushBack(PxVec3(r1 * cos1, r1 * sin1, thickness));
			mGeom.vertices.pushBack(PxVec3(p1 * cos1, p1 * sin1, thickness));

			for (physx::PxU32 k = 0; k < 6; k++) {
				mGeom.indices.pushBack(4);	// face size
				mGeom.indices.pushBack(CompoundGeometry::FF_OBJECT_SURFACE);	// flags
				mGeom.indices.pushBack(faces[k][0]);
				mGeom.indices.pushBack(faces[k][1]);
				mGeom.indices.pushBack(faces[k][2]);
				mGeom.indices.pushBack(faces[k][3]);
			}
			mGeom.neighbors.pushBack(i > 0 ? ((physx::PxI32)i-1) * numSegs + j : (numSectors-1) * numSegs + j);
			mGeom.neighbors.pushBack(j < numSegs-1 ? (physx::PxI32)i * numSegs + j+1 : -1);
			mGeom.neighbors.pushBack((physx::PxI32)i < numSectors-1 ? ((physx::PxI32)i+1) * numSegs + j : j);
			mGeom.neighbors.pushBack(j > 0 ? (physx::PxI32)i * numSegs + j-1 : -1);
			mGeom.neighbors.pushBack(-1);
			mGeom.neighbors.pushBack(-1);
		}
	}
	finalize();
}
// --------------------------------------------------------------------------------------------
void FracturePattern::createRegularCubeMesh(const PxVec3 &extents, float cubeSize)
{
	clear();
	int numX = (int)(2.0f*extents.x / cubeSize) + 1;
	int numY = (int)(2.0f*extents.y / cubeSize) + 1;
	int numZ = (int)(2.0f*extents.z / cubeSize) + 1;

	CompoundGeometry::Convex c;
	static int faces[6][4] = {{0,1,5,4}, {1,2,6,5}, {2,3,7,6}, {3,0,4,7}, {3,2,1,0}, {4,5,6,7}};

	for (int xi = 0; xi < numX; xi++) {
		for (int yi = 0; yi < numY; yi++) {
			for (int zi = 0; zi < numZ; zi++) {
				mGeom.initConvex(c);
				c.numFaces = 6;
				c.numNeighbors = 6;
				c.numVerts = 8;
				c.firstPlane = 0;
				mGeom.convexes.pushBack(c);

				PxVec3 p0(-extents.x + xi*cubeSize, -extents.y + yi*cubeSize, -extents.z + zi*cubeSize);
				PxVec3 p1 = p0 + PxVec3(cubeSize, cubeSize, cubeSize);
				mGeom.vertices.pushBack(PxVec3(p0.x, p0.y, p0.z)); 
				mGeom.vertices.pushBack(PxVec3(p1.x, p0.y, p0.z));
				mGeom.vertices.pushBack(PxVec3(p1.x, p1.y, p0.z));
				mGeom.vertices.pushBack(PxVec3(p0.x, p1.y, p0.z));
				mGeom.vertices.pushBack(PxVec3(p0.x, p0.y, p1.z));
				mGeom.vertices.pushBack(PxVec3(p1.x, p0.y, p1.z));
				mGeom.vertices.pushBack(PxVec3(p1.x, p1.y, p1.z));
				mGeom.vertices.pushBack(PxVec3(p0.x, p1.y, p1.z));
				for (int i = 0; i < 6; i++) {
					mGeom.indices.pushBack(4);		// face size
					mGeom.indices.pushBack(CompoundGeometry::FF_OBJECT_SURFACE);	// flags
					mGeom.indices.pushBack(faces[i][0]);
					mGeom.indices.pushBack(faces[i][1]);
					mGeom.indices.pushBack(faces[i][2]);
					mGeom.indices.pushBack(faces[i][3]);
				}
				mGeom.neighbors.pushBack(yi > 0 ? (xi * numY + yi-1) * numZ + zi : -1);
				mGeom.neighbors.pushBack(xi < numX-1 ? ((xi+1) * numY + yi) * numZ + zi : -1);
				mGeom.neighbors.pushBack(yi < numY-1 ? (xi * numY + yi+1) * numZ + zi : -1);
				mGeom.neighbors.pushBack(xi > 0 ? ((xi-1) * numY + yi) * numZ + zi : -1);
				mGeom.neighbors.pushBack(zi > 0 ? (xi * numY + yi) * numZ + zi-1 : -1);
				mGeom.neighbors.pushBack(zi < numZ-1 ? (xi * numY + yi) * numZ + zi+1 : -1);
			}
		}
	}

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::createCrosses(const PxVec3 &dims, float spacing)
{
	clear();

	int numX = (int)floorf(2.0f * dims.x / spacing) + 1;
	int numY = (int)floorf(2.0f * dims.y / spacing) + 1;
	int numZ = (int)floorf(2.0f * dims.z / spacing) + 1;

	static int offset[5] = {0,3,1,4,2};
	static int cross[5][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}, {0,0}};
	PxBounds3 bounds;

	for (int zi = 0; zi < numZ; zi++) {
		for (int yi = 0; yi < numY; yi++) {
			for (int xi = 0; xi < numX; xi += 5) {
				float z = -dims.z + (zi + xi % 2 + yi % 2) * spacing;
				float y = -dims.y + yi * spacing;
				float x = -dims.x + (xi + offset[yi % 5]) * spacing;

				mFirstConvexOfCell.pushBack((physx::PxI32)mGeom.convexes.size());

				for (int i = 0; i < 5; i++) {
					bounds.minimum.x = x + cross[i][0] * spacing;
					bounds.minimum.y = y + cross[i][1] * spacing;
					bounds.minimum.z = z;
					bounds.maximum = bounds.minimum + PxVec3(spacing, spacing, spacing);
					addBox(bounds);
				}
			}
		}
	}

	mFirstConvexOfCell.pushBack((physx::PxI32)mGeom.convexes.size());

	finalize();
}

// --------------------------------------------------------------------------------------------
void FracturePattern::getConvexIntersection(const Convex *convex, const PxMat44 &trans, float minConvexSize, int /*numFitDirections*/) const
{
	PxTransform localPose = convex->getLocalPose();
	const shdfnd::Array<PxPlane> convexPlanes = convex->getPlanes();

	mScene->profileBegin("find pattern candidates");

	// cull outside convexes
	shdfnd::Array<int> candidates(mGeom.convexes.size());
	for (physx::PxU32 i = 0; i < candidates.size(); i++)
		candidates[i] = (physx::PxI32)i;

	for (physx::PxU32 i = 0; i < convexPlanes.size(); i++) {
		PxPlane plane = convexPlanes[i];
		plane.n = localPose.rotate(plane.n);		// only valid for rigid transforms!
		plane.d = plane.d + localPose.p.dot(plane.n);

		physx::PxU32 num = 0;
		for (physx::PxU32 j = 0; j < candidates.size(); j++) {
			const CompoundGeometry::Convex &c = mGeom.convexes[(physx::PxU32)candidates[j]];
			bool inside = false;
			for (int k = 0; k < c.numVerts; k++) {
				PxVec3 p = trans.transform(mGeom.vertices[physx::PxU32(c.firstVert + k)]);
				if (p.dot(plane.n) < plane.d) {
					inside = true;
					break;
				}
			}
			if (inside) {
				candidates[num] = candidates[j];
				num++;
			}
		}
		candidates.resize(num);
	}

	mScene->profileEnd("find pattern candidates");

	// retreive convexes
	Piece piece;
	shdfnd::Array<Convex*> newConvexes;

	mScene->profileBegin("clip all");

	mScene->profileBegin("clipper init");
	MeshClipper *clipper = mScene->getMeshClipper();
	clipper->init(convex);
	mScene->profileEnd("clipper init");

	mScene->profileBegin("clipper clip");

	for (physx::PxU32 i = 0; i < candidates.size(); i++) {
		physx::PxU32 convexNr = (physx::PxU32)candidates[i];
		Convex *c = mScene->createConvex();
		const PxTransform transformTemp = convex->getLocalPose();
		c->createFromConvex(convex, &transformTemp);
		const CompoundGeometry::Convex &gc = mGeom.convexes[convexNr];

		bool empty = true;
		c->intersectWithConvex(&mGeom.planes[(physx::PxU32)gc.firstPlane], gc.numFaces, trans, empty);

		if (empty) {
			PX_DELETE(c);
			continue;
		}

		const PxBounds3 &bounds = c->getBounds();
		PxVec3 dim = bounds.getDimensions();
		if (!c->isGhostConvex() && (dim.x < minConvexSize || dim.y < minConvexSize || dim.z < minConvexSize)) {
			PX_DELETE(c);
			continue;
		}
	
		empty = false;
		newConvexes.clear();

		if (convex->hasExplicitVisMesh()) {
			if (!c->clipVisualMesh(clipper, convex->getLocalPose(), newConvexes)) {
				PX_DELETE(c);
				continue;
			}
			if (newConvexes.size() > 0) 
				mSplitPiece[convexNr] = true;
		}

		c->setModelIslandNr(convex->getModelIslandNr());
		c->setSurfaceMaterialId(convex->getSurfaceMaterialId());
		newConvexes.pushBack(c);

		for (physx::PxU32 i = 0; i < newConvexes.size(); i++) {
			Convex *c = newConvexes[i];
			const PxBounds3 &bounds = c->getBounds();
			PxVec3 dim = bounds.getDimensions();
			if (!c->isGhostConvex() && (dim.x < minConvexSize || dim.y < minConvexSize || dim.z < minConvexSize)) {
				// TODO: Spawn debris?
				PX_DELETE(c);
				continue;
			}
			piece.convex = newConvexes[i];
			piece.next = mFirstPiece[convexNr];
			mFirstPiece[convexNr] = (physx::PxI32)mPieces.size();
			mPieces.pushBack(piece);
		}
	}
	mScene->profileBegin("clipper clip");

	mScene->profileEnd("clip all");

}

// --------------------------------------------------------------------------------------------
void FracturePattern::getCompoundIntersection(const Compound *compound, const PxMat44 &trans, float radius, float minConvexSize,
		shdfnd::Array<int> &compoundSizes, shdfnd::Array<Convex*> &newConvexes) const
{
	// radius = 0.0f : complete fracture
	// radius > 0.0f : partial (local) fracture

	//{

	//	IslandDetector *id = IslandDetector::getInstance();
	//	id->detect(&convexes[0], convexes.size(), true);
	//}

	const shdfnd::Array<Convex*> &convexes = compound->getConvexes();
	//const shdfnd::Array<PxBounds3>& attachmentBounds = compound->getAttachmentBounds();

	PxMat33 m(trans.getBasis(0), trans.getBasis(1), trans.getBasis(2));
	float transScale = m.getDeterminant();

	newConvexes.clear();
	compoundSizes.clear();

	PxVec3 fractureCenter = trans.getPosition();

	for (physx::PxU32 i = 0; i < mGeom.convexes.size(); i++) {
		mFirstPiece[i] = -1;
		mSplitPiece[i] = false;
	}
	mPieces.clear();

	shdfnd::Array<Convex*> farConvexes;
	shdfnd::Array<Convex*> cellConvexes;

	// fracture convexes
	int numFitDirections = 7;

	// complete fracture
	if (radius == 0.0f) {
		for (physx::PxU32 i = 0; i < convexes.size(); i++) {
			Convex *c = convexes[i];
			//if (c->isGhostConvex())
			//	/*int foo = 0*/;
			getConvexIntersection(c, trans, minConvexSize, numFitDirections);
		}
	}
	else {	// partial fracture

		// fracture non-ghost convexes and determine furthest fractured vertex
		float maxGhostRadius2 = 0.0f;

		for (physx::PxU32 i = 0; i < convexes.size(); i++) {
			Convex *c = convexes[i];
			if (c->isGhostConvex())
				continue;

			PxTransform localPose = c->getLocalPose();
			PxVec3 localCenter = localPose.transformInv(fractureCenter);

			// is the convex outside of the fracture sphere?
			if (c->insideFattened(localCenter, radius)) {
				getConvexIntersection(c, trans, minConvexSize, numFitDirections);

				const shdfnd::Array<PxVec3> &verts = c->getVertices();
				for (physx::PxU32 j = 0; j < verts.size(); j++) {
					float r2 = (verts[j] - localCenter).magnitudeSquared();
					if (r2 > maxGhostRadius2)
						maxGhostRadius2 = r2;
				}
			}
			else {
				c->transform(localPose);
				c->clearFraceFlags(CompoundGeometry::FF_NEW);
				c->setIsFarConvex(true);
				farConvexes.pushBack(c);		// double use -> that is why we need reference counters for convexes!
			}
		}

		// added overlap test in island detector, no need for ghost anymore!

//		// fracture ghost convexes using the maxGhostRadius
//		float maxGhostRadius = PxSqrt(maxGhostRadius2);
//
//		for (int i = 0; i < (int)convexes.size(); i++) {
//			Convex *c = convexes[i];
//			if (!c->isGhostConvex())
//				continue;
//			PxTransform localPose = c->getLocalPose();
//			PxVec3 localCenter = localPose.transformInv(fractureCenter);
//
//			// is the convex outside of the fracture sphere?
//			if (c->insideFattened(localCenter, maxGhostRadius)) {
////			if (c->insideFattened(localCenter, radius)) {	// foo
//				getConvexIntersection(c, trans, minConvexSize, numFitDirections);
//			}
//			else {
//				c->transform(localPose);
//				farConvexes.pushBack(c);		// double use -> that is why we need reference counters for convexes!
//			}
//		}
	}

	// collect pieces
	bool complexCells = !mFirstConvexOfCell.empty();
	physx::PxU32  numCells = complexCells ? mFirstConvexOfCell.size()-1 : mGeom.convexes.size();

	for (physx::PxU32 i = 0; i < numCells; i++) {
		physx::PxU32 numConvexes = physx::PxU32(complexCells ? mFirstConvexOfCell[i+1] - mFirstConvexOfCell[i] : 1);

		cellConvexes.clear();
		PxVec3 matOff = PxVec3(0.0f, 0.0f, 0.0f);
		bool hasGhosts = false;
		bool splitCell = false;


		for (physx::PxU32 k = 0; k < numConvexes; k++) {
			physx::PxU32 convexNr = complexCells ? (physx::PxU32)mFirstConvexOfCell[i] + k : i;

			if (mSplitPiece[convexNr]) 
				splitCell = true;

			if (mFirstPiece[convexNr] < 0)
				continue;

			int nr = mFirstPiece[convexNr];
			while (nr >= 0) {
				Convex *c = mPieces[(physx::PxU32)nr].convex;
				if (c->isGhostConvex())
					hasGhosts = true;
				else
					matOff = c->getMaterialOffset();
				cellConvexes.pushBack(c);
				nr = mPieces[(physx::PxU32)nr].next;
			}
		}

		// fit to visual mesh, only if there are no ghost convexes, otherwise the faces do to match anymore
		if (!hasGhosts) {
			physx::PxU32 num = 0;
			for (physx::PxU32 j = 0; j < cellConvexes.size(); j++) {
				Convex *c = cellConvexes[j];
				if (c->hasExplicitVisMesh()) {
					bool empty = false;
					c->fitToVisualMesh(empty, numFitDirections);
					if (empty) {
						PX_DELETE(c);
						continue;
					}
				}
				cellConvexes[num] = c;
				num++;
			}
			cellConvexes.resize(num);
		}

		if (cellConvexes.size() == 1) {
			newConvexes.pushBack(cellConvexes[0]);
			compoundSizes.pushBack(1);
			continue;
		}

		// all volume covered with new pieces?
		bool hasExplicitMesh = false;

		float volumeFraction = 0.0f;
		for (physx::PxU32 j = 0; j < cellConvexes.size(); j++) {
			const Convex *c = cellConvexes[j];
			float vol = c->isGhostConvex() ? -c->getVolume() : c->getVolume();
			volumeFraction += vol;
			if (c->hasExplicitVisMesh())
				hasExplicitMesh = true;
		}

		float vol = mConvexVolumes[i] * transScale;
		float volDiff = fabsf(vol - volumeFraction) / vol;

		if (splitCell) {			// each convex piece within the pattern piece becomes a separate compound
			for (physx::PxU32 j = 0; j < cellConvexes.size(); j++) {
				newConvexes.pushBack(cellConvexes[j]);
				compoundSizes.pushBack(1);
			}
		}
		else if (!complexCells && !hasExplicitMesh && volDiff < 0.01f) {		// the entire pattern piece becomes a convex
			// delete parts
			for (physx::PxU32 j = 0; j < cellConvexes.size(); j++) 
				PX_DELETE(cellConvexes[j]);

			// create single piece
			Convex *c = mScene->createConvex();
			c->createFromGeometry(mGeom, (physx::PxI32)i, &trans);
			newConvexes.pushBack(c);
			c->setMaterialOffset(matOff);
			compoundSizes.pushBack(1);
		}
		else {							// all connected convex pieces within the pattern cell become one compound 
			IslandDetector *id = mScene->getIslandDetector();
			id->detect(cellConvexes, false);
			const shdfnd::Array<IslandDetector::Island> &islands = id->getIslands();
			const shdfnd::Array<int> &islandConvexes = id->getIslandConvexes();

			for (physx::PxU32 j = 0; j < islands.size(); j++) {
				const IslandDetector::Island &is = islands[j];
				for (int k = 0; k < is.size; k++) 
					newConvexes.pushBack(cellConvexes[(physx::PxU32)islandConvexes[physx::PxU32(is.firstNr+k)]]);
				compoundSizes.pushBack(is.size);
			}
		}
	}

	// merge far convexes for local fracture
	if (radius > 0.0f) {
		farConvexes.reserve(farConvexes.size() + newConvexes.size());

		physx::PxU32 oldConvexNr = 0;
		physx::PxU32 newConvexNr = 0;
		physx::PxU32 numNewCompounds = 0;

		for (physx::PxU32 i = 0; i < compoundSizes.size(); i++) {
			physx::PxU32 oldCompoundSize = (physx::PxU32)compoundSizes[i];
			physx::PxU32 newCompoundSize = 0;
			bool onlyGhosts = true;
			for (physx::PxU32 j = 0; j < oldCompoundSize; j++) {
				Convex *c = newConvexes[oldConvexNr + j]; 
				if (!c->insideFattened(fractureCenter, radius)) {
					c->clearFraceFlags(CompoundGeometry::FF_NEW);
					c->setIsFarConvex(true);
					farConvexes.pushBack(c);
				}
				else {
					if (!c->isGhostConvex())
						onlyGhosts = false;
					newConvexes[newConvexNr] = c;
					newConvexNr++;
					newCompoundSize++;
				}
			}
			if (newCompoundSize > 0) {
				if (onlyGhosts) {		// remove compounds that contain only ghost convexes
					for (physx::PxU32 j = 0; j < newCompoundSize; j++) {
						newConvexNr--;
						PX_DELETE(newConvexes[newConvexNr]);
					}
				}
				else {
					compoundSizes[numNewCompounds] = (physx::PxI32)newCompoundSize;
					numNewCompounds++;
				}
			}
			oldConvexNr += oldCompoundSize;
		}
		newConvexes.resize(newConvexNr);
		compoundSizes.resize(numNewCompounds);
	}

	if (farConvexes.size() > 0) {
		IslandDetector *id = mScene->getIslandDetector();

		mScene->profileBegin("island detection");

		id->detect(farConvexes, true); 

		mScene->profileEnd("island detection");

		const shdfnd::Array<IslandDetector::Island> &islands = id->getIslands();
		const shdfnd::Array<int> &islandConvexes = id->getIslandConvexes();

		for (physx::PxU32 i = 0; i < islands.size(); i++) {
			const IslandDetector::Island &island = islands[i];
			compoundSizes.pushBack(island.size);
			for (int j = 0; j < island.size; j++) {
				int nr = islandConvexes[physx::PxU32(island.firstNr + j)];
				newConvexes.pushBack(farConvexes[(physx::PxU32)nr]);
			}
		}

		//const shdfnd::Array<int> &pairs = id->getNeighborEdges();
		//FILE *f = fopen("c:\\test.txt", "a");
		//fprintf(f, "convexes %i, edges %i\n", farConvexes.size(), pairs.size()/2);
		//fclose(f);

		//StaticTester::getInstance()->setStretchThreshold(1.0f); // 1 newton / m^2
		//StaticTester::getInstance()->setCompressionThreshold(10.0f); // 10 newton / m^2
		//StaticTester::getInstance()->setTorqueThreshold(2.0f); // 2 newton / m
		//StaticTester::getInstance()->analyse(farConvexes, attachmentBounds, id->getNeighborEdges());

#if HIDE_INVISIBLE_FACES
		if (radius != 0.0f) {
		// invisible faces may open up with partial fracturing
			const shdfnd::Array<float> faceCoverage = mScene->getIslandDetector()->getFaceCoverage();
			physx::PxU32 globalFaceNr = 0;
			for (physx::PxU32 i = 0; i < farConvexes.size(); i++) {
				Convex *c = farConvexes[i];
				c->updateFaceVisibility(&faceCoverage[globalFaceNr]);
				globalFaceNr += c->getFaces().size();
			}
			// brute force
			//for (int i = 0; i < (int)newConvexes.size(); i++) {
			//	newConvexes[i]->updateFaceVisibility
			//	newConvexes[i]->removeInvisibleFacesFlags();
			//}
		}
#endif
	}

	//if (farConvexes.size() > 0) {
	//	compoundSizes.pushBack(farConvexes.size());
	//	for (int i = 0; i < (int)farConvexes.size(); i++)
	//		newConvexes.pushBack(farConvexes[i]);
	//}

//	printf("%i new convexes, %i new compounds\n", newConvexes.size(), compoundSizes.size());
}

}
}
}
#endif