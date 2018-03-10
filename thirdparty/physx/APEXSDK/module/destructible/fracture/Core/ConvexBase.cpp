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
#include "PxPhysics.h"
#include "PxCooking.h"
#include "PxDefaultStreams.h"
#include "PxShape.h"
#include "PxMath.h"
#include "PxRigidDynamic.h"
#include "PxConvexMesh.h"
#include "PxMat44.h"
#include "PsMathUtils.h"
#include "PxVec2.h"

#include "CompoundGeometryBase.h"
#include "SimSceneBase.h"
#include "PolygonTriangulatorBase.h"
#include "MeshClipperBase.h"

#include "ConvexBase.h"

#define COOK_TRIANGLES 0

#include <algorithm>

namespace physx
{

class PxPhysics;
class PxCooking;
class PxActor;
class PxScene;
class PxConvexMesh;

namespace fracture
{
namespace base
{

// --------------------------------------------------------------------------------------------
Convex::Convex(SimScene* scene)
{
	mScene = scene;
	mPxConvexMesh = NULL;
	mPxActor = NULL;
	mLocalPose = PxTransform::createIdentity();
	mParent = NULL;
	mNewConvex = NULL;
	mRefCounter = 0;
	mIsFarConvex = false;
	clear();
}

// --------------------------------------------------------------------------------------------
Convex::~Convex()
{
	clear();
	if (mNewConvex != NULL)
		PX_DELETE(mNewConvex);
}

// --------------------------------------------------------------------------------------------
void Convex::clear()
{
	mFaces.clear();
	mIndices.clear();
	mVertices.clear();
	mNormals.clear();
	mPlanes.clear();

	mVisVertices.clear();
	mVisNormals.clear();
	mVisTangents.clear();
	mVisTexCoords.clear();
	mVisTriIndices.clear();

	mHasExplicitVisMesh = false;
	mIsGhostConvex = false;
	mVisPolyStarts.clear();
	mVisPolyIndices.clear();
	mVisPolyNeighbors.clear();

	if (mPxConvexMesh != NULL)
		mPxConvexMesh->release();

	mPxConvexMesh = NULL;
	mPxActor = NULL;
	mLocalPose = PxTransform::createIdentity();

	mBounds.setEmpty();
	mMaterialOffset = PxVec3(0.0f, 0.0f, 0.0f);
	mTexScale = 1.0f;

	mUse2dTexture = false;
	mIndestructible = false;
	mMaterialId = 0;
	mSurfaceMaterialId = 0;
	mModelIslandNr = 0;

	mVolume = 0.0f;
	mVolumeDirty = true;
}

// --------------------------------------------------------------------------------------------
void Convex::createFromConvex(const Convex *convex, const PxTransform *trans)
{
	clear();
	mVertices = convex->mVertices;
	mFaces = convex->mFaces;
	clearFraceFlags(CompoundGeometry::FF_NEW);

	mIndices = convex->mIndices;
	mNormals = convex->mNormals;
	mMaterialOffset = convex->mMaterialOffset;
	mTexScale = convex->mTexScale;
	mIsGhostConvex = convex->mIsGhostConvex;
	mLocalPose = convex->mLocalPose;

	if (trans != NULL) {
		for (physx::PxU32 i = 0; i < mVertices.size(); i++)
			mVertices[i] = trans->transform(mVertices[i]);
		for (physx::PxU32 i = 0; i < mNormals.size(); i++)
			mNormals[i] = trans->rotate(mNormals[i]);
		mMaterialOffset -= trans->p;
	}
	finalize();
}

// --------------------------------------------------------------------------------------------
void Convex::transform(const PxMat44 &trans)
{
	for (physx::PxU32 i = 0; i < mVertices.size(); i++)
		mVertices[i] = trans.transform(mVertices[i]);
	for (physx::PxU32 i = 0; i < mNormals.size(); i++)
		mNormals[i] = trans.rotate(mNormals[i]);
	mMaterialOffset -= trans.getPosition();

	if (mHasExplicitVisMesh) {
		for (physx::PxU32 i = 0; i < mVisVertices.size(); i++)
			mVisVertices[i] = trans.transform(mVisVertices[i]);
		for (physx::PxU32 i = 0; i < mVisNormals.size(); i++)
			mVisNormals[i] = trans.rotate(mVisNormals[i]);
	}
	finalize();
}

// --------------------------------------------------------------------------------------------
void Convex::createFromGeometry(const CompoundGeometry &geom, int convexNr, const PxMat44 *trans)
{
	clear();
	const CompoundGeometry::Convex &c = geom.convexes[(physx::PxU32)convexNr];

	PxMat44 t = PxMat44::createIdentity();

	if (trans) 
		t = *trans;

	mVertices.resize((physx::PxU32)c.numVerts);
	for (int i = 0; i < c.numVerts; i++) 
		mVertices[(physx::PxU32)i] = t.transform(geom.vertices[physx::PxU32(c.firstVert + i)]);

	mFaces.resize((physx::PxU32)c.numFaces);
	int num = 0;
	const int *id = &geom.indices[(physx::PxU32)c.firstIndex];
	const PxVec3 *normals = (geom.normals.size() > 0) ? &geom.normals[(physx::PxU32)c.firstNormal] : NULL;

	for (physx::PxU32 i = 0; i < (physx::PxU32)c.numFaces; i++) {
		Face &f = mFaces[i];
		f.init();
		f.firstIndex = num;
		f.numIndices = *id++;
		f.flags = *id++;
		for (int j = 0; j < f.numIndices; j++) 
			mIndices.pushBack(*id++);
		if (f.flags & CompoundGeometry::FF_HAS_NORMALS) {
			f.firstNormal = (physx::PxI32)mNormals.size();
			for (int j = 0; j < f.numIndices; j++) {
				mNormals.pushBack(t.rotate(*normals));
				normals++;
			}
		}
		num += f.numIndices;
	}
	finalize();
}

// --------------------------------------------------------------------------------------------
void Convex::setPxActor(PxRigidActor *actor) 
{ 
	mPxActor = actor;
}

// --------------------------------------------------------------------------------------------
void Convex::setLocalPose(const PxTransform &pose)
{
	mLocalPose = pose;
}

// --------------------------------------------------------------------------------------------
PxVec3 Convex::getCenter() const
{
	PxVec3 center(0.0f, 0.0f, 0.0f);

	if (mVertices.empty())
		return center;

	physx::PxU32 numVerts = mVertices.size();
	for (physx::PxU32 i = 0; i < numVerts; i++)
		center += mVertices[i];
	center /= (float)numVerts;

	return center;
}

// --------------------------------------------------------------------------------------------
PxVec3 Convex::centerAtZero()
{
	PxVec3 center = getCenter();
	for (physx::PxU32 i = 0; i < mVertices.size(); i++)
		mVertices[i] -= center;
	finalize();
	return center;
}

// --------------------------------------------------------------------------------------------
PxTransform Convex::getGlobalPose() const
{
	if (mPxActor == NULL) 
		return mLocalPose;
	else 
		return mPxActor->getGlobalPose() * mLocalPose;
}

// --------------------------------------------------------------------------------------------
PxTransform Convex::getLocalPose() const
{
	return mLocalPose;
}

// --------------------------------------------------------------------------------------------
bool Convex::rayCast(const PxVec3 &orig, const PxVec3 &dir, float &dist, PxVec3 &normal) const
{
	PxVec3 o,d;
	PxTransform pose = getGlobalPose();
	d = pose.rotateInv(dir);
	o = pose.transformInv(orig);

	if (mHasExplicitVisMesh)
		return rayCastVisMesh(o,d, dist, normal);
	else
		return rayCastConvex(o,d, dist, normal);
}

// --------------------------------------------------------------------------------------------
bool Convex::rayCastConvex(const PxVec3 &orig, const PxVec3 &dir, float &dist, PxVec3 &normal) const
{
	float maxEntry = -PX_MAX_F32;
	float minExit = PX_MAX_F32;
	normal = PxVec3(0.0f, 1.0f, 0.0f);

	for (physx::PxU32 i = 0; i < mPlanes.size(); i++) {
		const PxPlane &plane = mPlanes[i];
		float dot = plane.n.dot(dir);
		if (dot == 0.0f)
			continue;

		float t = (plane.d - orig.dot(plane.n)) / dot;
		if (dot < 0.0f) {
			if (t > maxEntry) {
				maxEntry = t;
				normal = plane.n;
			}
		}
		else {
			minExit = PxMin(minExit, t);
		}
	}

	if (maxEntry > minExit) {
		dist = PX_MAX_F32;
		return false;
	}

	dist = maxEntry;
	normal.normalize();
	return dist >= 0.0f;
}

// --------------------------------------------------------------------------------------------
bool Convex::rayCastVisMesh(const PxVec3 &orig, const PxVec3 &dir, float &dist, PxVec3 &normal) const
{
	normal = PxVec3(0.0f, 1.0f, 0.0f);
	dist = PX_MAX_F32;
	int minTri = -1;

	for (physx::PxU32 i = 0; i < mVisTriIndices.size(); i += 3) {
		const PxVec3 &p0 = mVisVertices[(physx::PxU32)mVisTriIndices[i]];
		const PxVec3 &p1 = mVisVertices[(physx::PxU32)mVisTriIndices[i+1]];
		const PxVec3 &p2 = mVisVertices[(physx::PxU32)mVisTriIndices[i+2]];
		PxVec3 n = (p1-p0).cross(p2-p0);
		float t = dir.dot(n);
		if (t >= 0.0f)
			continue;
		t = n.dot(p0 - orig) / t;
		if (t < 0.0f)
			continue;

		// hit inside triangle?
		PxVec3 hit = orig + t * dir;
		float b2 = (p1-p0).cross(hit-p0).dot(n);
		float b0 = (p2-p1).cross(hit-p1).dot(n);
		float b1 = (p0-p2).cross(hit-p2).dot(n);
		if (b0 < 0.0f || b1 < 0.0f || b2 < 0.0f)
			continue;

		float d = (orig - hit).magnitude();

		if (d < dist) {
			dist = d;
			minTri = (physx::PxI32)i / 3;
			normal = n;
		}
	}
	normal.normalize();
	return minTri >= 0;
}

// --------------------------------------------------------------------------------------------
bool Convex::collide(const PxVec3 &pos, float r, float &penetration, PxVec3 &surfaceNormal, PxVec3 &surfaceVel) const
{
	PxVec3 p = getGlobalPose().transformInv(pos);
	
	float minDist = PX_MAX_F32;
	int minPlane = -1;

	for (physx::PxU32 i = 0; i < mPlanes.size(); i++) {
		const PxPlane &plane = mPlanes[i];
		float dist = plane.d - p.dot(plane.n) + r;
		if (dist < 0.0f)
			return false;
		if (dist < minDist) {
			minDist = dist;
			minPlane = (physx::PxI32)i;
		}
	}

	if (minPlane < 0)
		return false;
	else {
		PxPlane plane = mPlanes[(physx::PxU32)minPlane];
		surfaceNormal = plane.n;
		penetration = minDist;
		surfaceVel = PxVec3(0.0f, 0.0f, 0.0f);

		if (mPxActor != NULL) {
			PxRigidDynamic *actor = mPxActor->isRigidDynamic();
			if (actor != NULL) {
				surfaceNormal = getGlobalPose().rotate(surfaceNormal);
				surfaceVel = actor->getLinearVelocity() + actor->getAngularVelocity().cross(surfaceNormal * plane.d);
			}
		}
		return true;
	}
}

// --------------------------------------------------------------------------------------------
void Convex::finalize()
{
	updateBounds();
	updatePlanes();
}

// --------------------------------------------------------------------------------------------
void Convex::updateBounds()
{
	mBounds.setEmpty();
	for (physx::PxU32 i = 0; i < mVertices.size(); i++)
		mBounds.include(mVertices[i]);
}

// --------------------------------------------------------------------------------------------
void Convex::updatePlanes()
{
	physx::PxU32 numFaces = mFaces.size();
	mPlanes.resize(numFaces);

	for (physx::PxU32 i = 0; i < numFaces; i++) {
		Face &f = mFaces[i];
		if (f.numIndices < 3) { // should not happen
			mPlanes[i].n = PxVec3(0.0f, 1.0f, 0.0f);	
			mPlanes[i].d = 0.0f;
			continue;
		}
		PxVec3 n(0.0f, 0.0f, 0.0f);
		PxVec3 &p0 = mVertices[(physx::PxU32)mIndices[(physx::PxU32)f.firstIndex]];
		for (int j = 1; j < f.numIndices-1; j++) {
			PxVec3 &p1 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j)]];
			PxVec3 &p2 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j+1)]];
			n += (p1-p0).cross(p2-p0);
		}
		n.normalize();
		mPlanes[i].n = n;
		mPlanes[i].d = mPlanes[i].n.dot(p0);
	}
}

// --------------------------------------------------------------------------------------------
void Convex::getWorldBounds(PxBounds3 &bounds) const
{
	bounds = PxBounds3::transformSafe(getGlobalPose(), mBounds);
}

// --------------------------------------------------------------------------------------------
void Convex::getLocalBounds(PxBounds3 &bounds) const
{
	bounds = mBounds;
}

// --------------------------------------------------------------------------------------------
void Convex::intersectWithConvex(const PxPlane *planes, int numPlanes, const PxMat44 &trans, bool &empty)
{
	empty = false;

	PxMat33 M(trans.getBasis(0), trans.getBasis(1), trans.getBasis(2));
	PxMat33 M1 = M.getInverse();
	PxMat33 M1T = M1.getTranspose();

	for (int i = 0; i < numPlanes; i++) {
		PxPlane p;
		p.n = M1T.transform(planes[i].n);
		p.d = planes[i].d + trans.getPosition().dot(p.n);
		float len = p.n.normalize();
		p.d /= len;
		cut(p.n, p.d, empty);
		if (empty)
			return;
	}
} 

// --------------------------------------------------------------------------------------------
bool Convex::cut(const PxVec3 &localPlaneN, float localPlaneD, bool &cutEmpty, bool setNewFaceFlag)
{
	float eps = 1e-4f;

	//bool pos = false;
	//bool neg = false;

	PxVec3 pn = localPlaneN;
	float  pd = localPlaneD;

	// does the plane pass through the convex?
	float minD = PX_MAX_F32;
	float maxD = -PX_MAX_F32;

	for (physx::PxU32 i = 0; i < mVertices.size(); i++) {
		float dot = mVertices[i].dot(pn);
		if (dot < minD) minD = dot;
		if (dot > maxD) maxD = dot;
	}

	cutEmpty = minD > pd - eps;
	if (cutEmpty || maxD < pd + eps)
		return false;

	// new: avoid singular cases
	for (physx::PxU32 i = 0; i < mVertices.size(); i++) {
		float dot = mVertices[i].dot(pn);
		if (fabs(dot - pd) < 1e-5f)
			pd += 1e-5f;				
	}

	// member so the memory does not have to be re-allocated for every cut
	if (mNewConvex == NULL)
		mNewConvex = mScene->createConvex();
	mNewConvex->clear();

	// create vertices
	physx::PxU32 numFaces = mFaces.size();
	shdfnd::Array<int> newNr(mVertices.size(), -1);

	struct Cut {
		void set(int i0, int i1, int newId) {
			this->i0 = i0; this->i1 = i1; this->newId = newId; next = -1;
		}
		bool is(int i0, int i1) { 
			return (this->i0 == i0 && this->i1 == i1) || (this->i0 == i1 && this->i1 == i0);
		}
		int i0, i1;
		int newId;
		int next;
	};
	shdfnd::Array<Cut> cuts;

	for (physx::PxU32 i = 0; i < numFaces; i++) {
		Face &f = mFaces[i];
		int *ids = &mIndices[(physx::PxU32)f.firstIndex];
		mNewConvex->mFaces.resize(mNewConvex->mFaces.size() + 1);
		Face &newFace = mNewConvex->mFaces[mNewConvex->mFaces.size()-1];
		newFace.init();
		newFace.flags = f.flags;
		newFace.firstIndex = (physx::PxI32)mNewConvex->mIndices.size();
		bool hasNormals = (f.flags & CompoundGeometry::FF_HAS_NORMALS) != 0;
		if (hasNormals)
			newFace.firstNormal = (physx::PxI32)mNewConvex->mNormals.size();

		int cutNrs[2];
		physx::PxU32 numCuts = 0;
		bool winding = false;

		for (int j = 0; j < f.numIndices; j++) {
			int j1 = (j+1)%f.numIndices;
			int i0 = ids[j];
			int i1 = ids[j1];
			PxVec3 &p0 = mVertices[(physx::PxU32)i0];
			PxVec3 &p1 = mVertices[(physx::PxU32)i1];
			PxVec3 n0,n1;
			if (hasNormals) {
				n0 = mNormals[physx::PxU32(f.firstNormal+j)];
				n1 = mNormals[physx::PxU32(f.firstNormal+j1)];
			}
			bool sign0 = p0.dot(pn) >= pd;
			bool sign1 = p1.dot(pn) >= pd;
			if (!sign0) {
				if (newNr[(physx::PxU32)i0] < 0) {
					newNr[(physx::PxU32)i0] = (physx::PxI32)mNewConvex->mVertices.size();
					mNewConvex->mVertices.pushBack(p0);
				}
				mNewConvex->mIndices.pushBack(newNr[(physx::PxU32)i0]);
				if (hasNormals)
					mNewConvex->mNormals.pushBack(n0);
				if (numCuts == 1)
					winding = true;
			}

			// cut?
			if (sign0 != sign1) {
				// do we have the cut vertex already?
				physx::PxU32 totalCuts = cuts.size();
				physx::PxU32 k = 0;
				while (k < totalCuts && !cuts[k].is(i0,i1))
					k++;

				float t = (pd - p0.dot(pn)) / (p1 - p0).dot(pn);
				// create new cut vertex
				if (k == totalCuts) {
					cuts.resize(totalCuts+1);
					cuts[totalCuts].set(i0, i1, (physx::PxI32)mNewConvex->mVertices.size());
					PxVec3 p = p0 + (p1-p0) * t;
					mNewConvex->mVertices.pushBack(p);
				}
				mNewConvex->mIndices.pushBack(cuts[k].newId);
				if (hasNormals) {
					PxVec3 n = n0 + (n1-n0) * t;
					n.normalize();
					mNewConvex->mNormals.pushBack(n);
				}
				if (numCuts >= 2) {	// numerical problems!
					cutEmpty = true;
					return false;
				}
				cutNrs[numCuts] = (physx::PxI32)k;
				numCuts++;
			}
		}

		if (numCuts == 1) {	// numerical problems!
			cutEmpty = true;
			return false;
		}

		if (numCuts == 2) {
			Cut &cut0 = cuts[(physx::PxU32)cutNrs[0]];
			Cut &cut1 = cuts[(physx::PxU32)cutNrs[1]];
			if (winding) {
				cut0.next = cutNrs[1];
			}
			else {
				cut1.next = cutNrs[0];
			}
		}

		// check whether the new convex got a new face

		newFace.numIndices = (physx::PxI32)mNewConvex->mIndices.size() - newFace.firstIndex;
		if (newFace.numIndices == 0)
			mNewConvex->mFaces.popBack();
		else if (newFace.numIndices < 3) {
			cutEmpty = true;
			return false;
		}
	}

	// create closing face
	Face closingFace;
	closingFace.init();
	closingFace.firstIndex = (physx::PxI32)mNewConvex->mIndices.size();

	int nr = 0;
	for (physx::PxU32 i = 0; i < cuts.size(); i++) {
		mNewConvex->mIndices.pushBack(cuts[(physx::PxU32)nr].newId);
		closingFace.numIndices++;
		nr = cuts[(physx::PxU32)nr].next;

		if (nr < 0) {		// numerical problems
			cutEmpty = true;
			return false;
		}
	}
	if (closingFace.numIndices < 3) {
		cutEmpty = true;
		return false;
	}
	if (setNewFaceFlag)
		closingFace.flags |= CompoundGeometry::FF_NEW;

	mNewConvex->mFaces.pushBack(closingFace);

	mVertices = mNewConvex->mVertices;
	mIndices = mNewConvex->mIndices;
	mFaces = mNewConvex->mFaces;
	mNormals = mNewConvex->mNormals;
	mVolumeDirty = true;
	finalize();

	return true;
}

// --------------------------------------------------------------------------------------------
PxConvexMesh* Convex::createPxConvexMesh(Compound *parent, PxPhysics *pxPhysics, PxCooking *pxCooking)
{
	mParent = parent;

	if (mVertices.empty())
		return NULL;

	if (mPxConvexMesh != NULL) 
		return mPxConvexMesh;

	mPxConvexMesh = NULL;
	mPxActor = NULL;

#if COOK_TRIANGLES
	// create tri mesh
	// in this tri mesh vertices are shared in contrast to the tri mesh for rendering
	shdfnd::Array<PxU32> indices;
	for (int i = 0; i < (int)mFaces.size(); i++) {
		Face &f = mFaces[i];
		if (f.numIndices < 3)
			continue;
		int *ids = &mIndices[f.firstIndex];
		for (int j = 1; j < f.numIndices-1; j++) {
			indices.pushBack(ids[0]);
			indices.pushBack(ids[j]);
			indices.pushBack(ids[j+1]);
		}
	}

	PxConvexMeshDesc meshDesc;
	meshDesc.setToDefault();
	meshDesc.points.count	  = mVertices.size();
	meshDesc.points.stride    = sizeof(PxVec3);
	meshDesc.points.data	  = &mVertices[0];
	meshDesc.triangles.count  = indices.size() / 3;
	meshDesc.triangles.stride = 3*sizeof(PxU32);
	meshDesc.triangles.data   = &indices[0];
#else
	shdfnd::Array<PxHullPolygon> polygons;
	polygons.reserve(mFaces.size());
	if (mPlanes.size() != mFaces.size())
		updatePlanes();

	for (physx::PxU32 i = 0; i < mFaces.size(); i++) {
		Face &f = mFaces[i];
		if (f.numIndices < 3)
			continue;
		PxHullPolygon p;
		p.mIndexBase = (physx::PxU16)f.firstIndex;
		p.mNbVerts = (physx::PxU16)f.numIndices;
		p.mPlane[0] = mPlanes[i].n.x;
		p.mPlane[1] = mPlanes[i].n.y;
		p.mPlane[2] = mPlanes[i].n.z;
		p.mPlane[3] = -mPlanes[i].d;
		polygons.pushBack(p);
	}

	PxConvexMeshDesc meshDesc;
	meshDesc.setToDefault();
	meshDesc.points.count	  = mVertices.size();
	meshDesc.points.stride    = sizeof(PxVec3);
	meshDesc.points.data	  = &mVertices[0];
	meshDesc.indices.count    = mIndices.size();
	meshDesc.indices.data     = &mIndices[0];
	meshDesc.indices.stride   = sizeof(PxU32);
	meshDesc.polygons.count   = mFaces.size();
	meshDesc.polygons.data    = &polygons[0];
	meshDesc.polygons.stride  = sizeof(PxHullPolygon);

#endif

	// Cooking from memory
	PxDefaultMemoryOutputStream outBuffer;
	if (!pxCooking->cookConvexMesh(meshDesc, outBuffer))
		return NULL;

	PxDefaultMemoryInputData inBuffer(outBuffer.getData(), outBuffer.getSize());
	mPxConvexMesh = pxPhysics->createConvexMesh(inBuffer);

	return mPxConvexMesh;
}

// --------------------------------------------------------------------------------------------
void Convex::setMaterialOffset(const PxVec3 &offset)
{
	if (mHasExplicitVisMesh) {
		for (physx::PxU32 i = 0; i < mVisVertices.size(); i++) 
			mVisVertices[i] += mMaterialOffset - offset;
	}
	mMaterialOffset = offset;
}

// --------------------------------------------------------------------------------------------
void Convex::createVisMeshFromConvex()
{
	// vertices are duplicated for each face to get sharp edges with multiple normals
	mHasExplicitVisMesh = false;
	mVisVertices.clear();
	mVisNormals.clear();
	mVisTangents.clear();
	mVisTriIndices.clear();
	mVisTexCoords.clear();
	PxVec3 n;
	PxVec3 t0, t1;

	for (physx::PxU32 i = 0; i < mFaces.size(); i++) {
		Face &f = mFaces[i];
		if (f.flags & CompoundGeometry::FF_INVISIBLE)
			continue;
		if (f.numIndices < 3)
			continue;
		// normal
		int *ids = &mIndices[(physx::PxU32)f.firstIndex];
		if (!(f.flags & CompoundGeometry::FF_HAS_NORMALS)) {
			n = (mVertices[(physx::PxU32)ids[1]] - mVertices[(physx::PxU32)ids[0]]).cross(mVertices[(physx::PxU32)ids[2]] - mVertices[(physx::PxU32)ids[0]]);
			n.normalize();
		}
		// tangents
		if (fabs(n.x) < fabs(n.y) && fabs(n.x) < fabs(n.z))
			t0 = PxVec3(1.0f, 0.0f, 0.0f);
		else if (fabs(n.y) < fabs(n.z))
			t0 = PxVec3(0.0f, 1.0f, 0.0);
		else
			t0 = PxVec3(0.0f, 0.0f, 1.0f);
		t1 = n.cross(t0);
		t1.normalize();
		t0 = t1.cross(n);

		int firstVertNr = (physx::PxI32)mVisVertices.size();
		for (int j = 0; j < f.numIndices; j++) {
			PxVec3 p = mVertices[(physx::PxU32)ids[(physx::PxU32)j]];
			mVisVertices.pushBack(p);
			if (f.flags & CompoundGeometry::FF_HAS_NORMALS) {
				mVisNormals.pushBack(mNormals[physx::PxU32(f.firstNormal + j)]);
				mVisTangents.pushBack(PxVec3(0.0f, 0.0f, 0.0f));	// on surface, not bump map
			}
			else {
				mVisNormals.pushBack(n);
				mVisTangents.pushBack(t0);	// internal face, bump map
			}
			mVisTexCoords.pushBack(p.dot(t0) * mTexScale);
			mVisTexCoords.pushBack(p.dot(t1) * mTexScale);
		}
		for (int j = 1; j < f.numIndices-1; j++) {
			mVisTriIndices.pushBack(firstVertNr);
			mVisTriIndices.pushBack(firstVertNr+j);
			mVisTriIndices.pushBack(firstVertNr+j+1);
		}
	}
}

// --------------------------------------------------------------------------------------------
struct Edge {
	// not using indices for edge match but positions
	// so duplicated vertices are handled as one vertex
	bool smaller(const PxVec3 &v0, const PxVec3 &v1) const {
		if (v0.x < v1.x) return true;
		if (v0.x > v1.x) return false;
		if (v0.y < v1.y) return true;
		if (v0.y > v1.y) return false;
		return (v0.z < v1.z);
	}
	void set(const PxVec3 &v0, const PxVec3 &v1, int faceNr, int edgeNr) {
		if (smaller(v0,v1)) { this->v0 = v0; this->v1 = v1; } 
		else { this->v0 = v1; this->v1 = v0; }
		this->faceNr = faceNr; this->edgeNr = edgeNr;
	}
	bool operator < (const Edge &e) const {
		if (smaller(v0, e.v0)) return true;
		if (v0 == e.v0 && smaller(v1, e.v1)) return true;
		return false;
	}
	bool operator == (const Edge &e) const { return v0 == e.v0 && v1 == e.v1; }
	PxVec3 v0,v1;
	int faceNr, edgeNr;
};

// --------------------------------------------------------------------------------------------
bool Convex::computeVisMeshNeighbors()
{
	physx::PxU32 numIndices = mVisPolyIndices.size();
	physx::PxU32 numPolygons = mVisPolyStarts.size()-1;

	shdfnd::Array<Edge> edges(numIndices);

	for (physx::PxU32 i = 0; i < numPolygons; i++) {
		int first = mVisPolyStarts[i];
		int num = mVisPolyStarts[i+1] - first;
		for (int j = 0; j < num; j++) {
			edges[physx::PxU32(first + j)].set(
				mVisVertices[(physx::PxU32)mVisPolyIndices[physx::PxU32(first + j)]], 
				mVisVertices[(physx::PxU32)mVisPolyIndices[physx::PxU32(first + (j+1)%num)]], (physx::PxI32)i, first + j);
		}
	}
	std::sort(edges.begin(), edges.end());

	mVisPolyNeighbors.resize(numIndices);
	bool manifold = true;
	physx::PxU32 i = 0;
	while (i < edges.size()) {
		Edge &e0 = edges[i];
		i++;
		if (i < edges.size() && edges[i] == e0) {
			Edge &e1 = edges[i];
			mVisPolyNeighbors[(physx::PxU32)e0.edgeNr] = e1.faceNr;
			mVisPolyNeighbors[(physx::PxU32)e1.edgeNr] = e0.faceNr;
			i++;
		}
		while (i < edges.size() && edges[i] == e0) {
			manifold = false;
			i++;
		}
	}
	return manifold;
}

// --------------------------------------------------------------------------------------------
bool Convex::setExplicitVisMeshFromTriangles(int numVertices, const PxVec3 *vertices, const PxVec3 *normals, const PxVec2 *texcoords,
								int numIndices, const PxU32 *indices, PxTransform *trans, const PxVec3* scale)
{
	mHasExplicitVisMesh = true;

	// reduce to vertices referenced by indices
	// needed if submesh is provided
	shdfnd::Array<int> oldToNew((physx::PxU32)numVertices, -1);
	mVisVertices.clear();
	mVisNormals.clear();
	mVisTangents.clear();
	mVisTexCoords.clear();

	const float scaleMagn = scale ? scale->magnitude() : 1.f;

	for (int i = 0; i < numIndices; i++) {
		physx::PxU32 id = (physx::PxU32)indices[(physx::PxU32)i];
		if (oldToNew[id] < 0) {
			oldToNew[id] = (physx::PxI32)mVisVertices.size();

			PxVec3 p = vertices[id];
			PxVec3 n = normals[id];
			if (scale != NULL)
			{
				p.x = scale->x*p.x;
				p.y = scale->y*p.y;
				p.z = scale->z*p.z;
				n.x = scale->x*n.x/scaleMagn;
				n.y = scale->y*n.y/scaleMagn;
				n.z = scale->z*n.z/scaleMagn;
			}
			if (trans != NULL) {
				p = trans->transform(p);
				n = trans->rotate(n);
			}
			mVisVertices.pushBack(p);
			mVisNormals.pushBack(n);
			mVisTangents.pushBack(PxVec3(0.0f, 0.0f, 0.0f));	// on surface, no bump map
			mVisTexCoords.pushBack(texcoords[id].x);		// to do, get as input
			mVisTexCoords.pushBack(texcoords[id].y);
		}
	}

	physx::PxU32 numTris = (physx::PxU32)numIndices / 3;
	mVisPolyStarts.resize(numTris+1);
	for (physx::PxU32 i = 0; i < numTris+1; i++)
		mVisPolyStarts[i] = 3*(physx::PxI32)i;

	mVisPolyIndices.resize((physx::PxU32)numIndices);
	for (physx::PxU32 i = 0; i < (physx::PxU32)numIndices; i++) 
		mVisPolyIndices[i] = oldToNew[(physx::PxU32)indices[i]];

	mVisTriIndices.clear();

	bool manifold = computeVisMeshNeighbors();
	createVisTrisFromPolys();
	computeVisTangentsFromPoly();

	return manifold;
}

// --------------------------------------------------------------------------------------------
bool Convex::setExplicitVisMeshFromPolygons(int numVertices, const PxVec3 *vertices, const PxVec3 *normals, const PxVec3 *tangents, const float *texCoords, 
								int numPolygons, const int *polyStarts, 
								int numIndices, const int *indices, PxTransform *trans, const PxVec3* scale)
{
	mHasExplicitVisMesh = true;

	mVisVertices.resize((physx::PxU32)numVertices);
	mVisNormals.resize((physx::PxU32)numVertices);
	mVisTangents.resize((physx::PxU32)numVertices);
	mVisTexCoords.resize(2*(physx::PxU32)numVertices);

	const float scaleMagn = scale ? scale->magnitude() : 1.f;

	for (physx::PxU32 i = 0; i < (physx::PxU32)numVertices; i++) {
		if (trans != NULL) {
			mVisVertices[i] = trans->transform(vertices[i]);
			mVisNormals[i] = trans->rotate(normals[i]);
			if (tangents != NULL)
				mVisTangents[i] = trans->rotate(tangents[i]);
			else
				mVisTangents[i] = PxVec3(0.0f, 0.0f, 0.0f);
		}
		else {
			mVisVertices[i] = vertices[i];
			mVisNormals[i] = normals[i];
			if (tangents != NULL)
				mVisTangents[i] = tangents[i];
			else
				mVisTangents[i] = PxVec3(0.0f, 0.0f, 0.0f);
		}
		if (texCoords != NULL) {
			mVisTexCoords[2*i] = texCoords[2*i];
			mVisTexCoords[2*i+1] = texCoords[2*i+1];
		}
		if (scale != NULL)
		{
			mVisVertices[i].x *= scale->x;
			mVisVertices[i].y *= scale->y;
			mVisVertices[i].z *= scale->z;
			mVisNormals[i].x *= scale->x/scaleMagn;
			mVisNormals[i].y *= scale->y/scaleMagn;
			mVisNormals[i].z *= scale->z/scaleMagn;
		}
	}

	mVisPolyStarts.resize((physx::PxU32)numPolygons+1);
	for (physx::PxU32 i = 0; i < (physx::PxU32)numPolygons+1; i++)
		mVisPolyStarts[i] = polyStarts[i];

	mVisPolyIndices.resize((physx::PxU32)numIndices);
	for (physx::PxU32 i = 0; i < (physx::PxU32)numIndices; i++)
		mVisPolyIndices[i] = indices[i];

	bool manifold = computeVisMeshNeighbors();
	createVisTrisFromPolys();
	computeVisTangentsFromPoly();

	return manifold;
}

// --------------------------------------------------------------------------------------------
void Convex::computeVisTangentsFromPoly() {
	// Must be called after createVisTrisFromPolys
	//Adapt from Lengyel, Eric. "Computing Tangent Space Basis Vectors for an Arbitrary Mesh". Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html

	physx::PxU32 numTris = mVisTriIndices.size() / 3;
	int* tris = &mVisTriIndices[0];
	physx::PxU32 numV = mVisVertices.size();

	for (physx::PxU32 i = 0; i < numTris; i++)
    {
        physx::PxU32 i0 = (physx::PxU32)tris[0];
        physx::PxU32 i1 = (physx::PxU32)tris[1];
        physx::PxU32 i2 = (physx::PxU32)tris[2];
        	
        PxVec3& v0 = mVisVertices[i0];
        PxVec3& v1 = mVisVertices[i1];
        PxVec3& v2 = mVisVertices[i2];

		
        float* tex0 = &mVisTexCoords[2*i0];
        float* tex1 = &mVisTexCoords[2*i1];
        float* tex2 = &mVisTexCoords[2*i2];
        
        float x0 = v1.x - v0.x;
        float x1 = v2.x - v0.x;
        float y0 = v1.y - v0.y;
        float y1 = v2.y - v0.y;
        float z1 = v1.z - v0.z;
        float z2 = v2.z - v0.z;
        
        float s0 = tex1[0] - tex0[0];
        float s1 = tex2[0] - tex0[0];
        float t0 = tex1[1] - tex0[1];
        float t1 = tex2[1] - tex0[1];
        
        float idet = 1.0f / (s0*t1 - s1*t0);
        PxVec3 t = idet*PxVec3(t1*x0 - t0*x1,t1*y0 - t0*y1,t1*z1 - t0*z2);
		//NxVec3 b = idet*NxVec3(s0*x1 - s1*x0,s0*y1 - s1*y0,s0*z2 - s1*z1);
        
/*		if ( (i0 == 0) || (i1 == 0) || (i2 == 0) ) {
			cout<<t[0]<<" "<<t[1]<<" "<<t[2]<<endl;
			ok= true;
		}*/
        mVisTangents[i0] += t;
        mVisTangents[i1] += t;
        mVisTangents[i2] += t;      
        
        tris+=3;
    }
    //cout<<"OK = "<<ok<<endl;

	for (physx::PxU32 i = 0; i < numV; i++) 
    {
        PxVec3& n = mVisNormals[i];
        PxVec3& t = mVisTangents[i];
	
        
        // Gram-Schmidt orthogonalize
        t = (t - n * n.dot(t));
		t.normalize();	

		if (!t.isFinite()) t = PxVec3(0.0f, 0.0f, 0.0f);

		mVisTexCoords[2*i] += 5.0f;
	}	
}

// --------------------------------------------------------------------------------------------
void Convex::createVisTrisFromPolys() 
{
	mVisTriIndices.clear();
	for (physx::PxU32 i = 0; i < mVisPolyStarts.size()-1; i++) {
		physx::PxU32 first = (physx::PxU32)mVisPolyStarts[i];
		physx::PxU32 num = (physx::PxU32)mVisPolyStarts[i+1] - first;
		if (num == 3) {
			mVisTriIndices.pushBack(mVisPolyIndices[first]);
			mVisTriIndices.pushBack(mVisPolyIndices[first+1]);
			mVisTriIndices.pushBack(mVisPolyIndices[first+2]);
			continue;
		}
		PolygonTriangulator *pt = mScene->getPolygonTriangulator(); //PolygonTriangulator::getInstance();
		pt->triangulate(&mVisVertices[0], (physx::PxI32)num, &mVisPolyIndices[first]);
		const shdfnd::Array<int> indices = pt->getIndices();

		physx::PxU32 numIds = mVisTriIndices.size();
		mVisTriIndices.resize(numIds + indices.size());
		for (physx::PxU32 j = 0; j < indices.size(); j++)
			mVisTriIndices[numIds+j] = indices[j];
	}
}

// --------------------------------------------------------------------------------------------
void Convex::transformVisualMesh(const PxTransform &trans)
{
	for (physx::PxU32 i = 0; i < mVisVertices.size(); i++) 
		mVisVertices[i] = trans.transform(mVisVertices[i]);
	for (physx::PxU32 i = 0; i < mVisNormals.size(); i++) 
		mVisNormals[i] = trans.rotate(mVisNormals[i]);
	for (physx::PxU32 i = 0; i < mVisTangents.size(); i++) 
		mVisTangents[i] = trans.rotate(mVisTangents[i]);
}

// --------------------------------------------------------------------------------------------
bool Convex::insideVisualMesh(const PxVec3 &pos) const
{
	int num[6] = {0,0,0,0,0,0};
	const physx::PxU32 numAxes = 2;		// max 3: the nigher the more robust but slower

	for (physx::PxU32 i = 0; i < mVisTriIndices.size(); i += 3) {
		const PxVec3 &p0 = mVisVertices[(physx::PxU32)mVisTriIndices[i]];
		const PxVec3 &p1 = mVisVertices[(physx::PxU32)mVisTriIndices[i+1]];
		const PxVec3 &p2 = mVisVertices[(physx::PxU32)mVisTriIndices[i+2]];
		PxVec3 n = (p1-p0).cross(p2-p0);
		float d = n.dot(p0);
		float ds = d - pos.dot(n);

		// for all axes, cound the hits of the ray starting from pos with the mesh
		for (physx::PxU32 j = 0; j < numAxes; j++) {
			if (n[j] == 0.0f) 
				continue;

			physx::PxU32 j0 = (j+1)%3;
			physx::PxU32 j1 = (j+2)%3;
			float x0 = p0[j0];
			float y0 = p0[j1];
			float x1 = p1[j0];
			float y1 = p1[j1];
			float x2 = p2[j0];
			float y2 = p2[j1];

			float px = pos[j0];
			float py = pos[j1];

			// inside triangle?
			float d0 = (x1-x0) * (py-y0) - (y1-y0) * (px-x0);
			float d1 = (x2-x1) * (py-y1) - (y2-y1) * (px-x1);
			float d2 = (x0-x2) * (py-y2) - (y0-y2) * (px-x2);
			bool inside = 
				(d0 <= 0.0f && d1 <= 0.0f && d2 <= 0.0f) ||
				(d0 >= 0.0f && d1 >= 0.0f && d2 >= 0.0f);
			if (!inside)
				continue;

			float s = ds / n[j];
			if (s > 0.0f) 
				num[2*j] += (n[j] > 0.0f) ? +1 : -1;
			else 
				num[2*j+1] += (n[j] < 0.0f) ? +1 : -1;
		}
	}
	physx::PxU32 numVotes = 0;
	for (physx::PxU32 i = 0; i < 6; i++) {
		if (num[i] > 0)
			numVotes++;
	}
	return numVotes > numAxes;
}

// --------------------------------------------------------------------------------------------
bool Convex::clipVisualMesh(MeshClipper *clipper, const PxTransform &trans, shdfnd::Array<Convex*> &newConvexes)
{
	bool empty, completelyInside;
	newConvexes.clear();

	clipper->clip(this, trans, mTexScale, true, empty, completelyInside);

	if (completelyInside) {
		mHasExplicitVisMesh = false;
		return true;
	}

	if (empty)
		return false;

	const MeshClipper::Mesh &mesh = clipper->getMesh(0);

	mVisVertices = mesh.vertices;
	mVisNormals = mesh.normals;
	mVisTangents = mesh.tangents;
	mVisTexCoords = mesh.texCoords;
	mVisPolyStarts = mesh.polyStarts;
	mVisPolyIndices = mesh.polyIndices;
	mVisPolyNeighbors = mesh.polyNeighbors;
	mHasExplicitVisMesh = true;

	createVisTrisFromPolys();

	finalize();

	int numMeshes = clipper->getNumMeshes();
	if (numMeshes > 1) {
		newConvexes.resize((physx::PxU32)numMeshes-1);
		for (int i = 1; i < numMeshes; i++) {
			const MeshClipper::Mesh &mesh = clipper->getMesh(i);
			Convex *c = mScene->createConvex();
			newConvexes[(physx::PxU32)i-1] = c;
			c->createFromConvex(this);
			c->mVisVertices = mesh.vertices;
			c->mVisNormals = mesh.normals;
			c->mVisTangents = mesh.tangents;
			c->mVisTexCoords = mesh.texCoords;
			c->mVisPolyStarts = mesh.polyStarts;
			c->mVisPolyIndices = mesh.polyIndices;
			c->mVisPolyNeighbors = mesh.polyNeighbors;
			c->mHasExplicitVisMesh = true;
			c->createVisTrisFromPolys();
			c->finalize();
		}
	}
	return true;
}

// --------------------------------------------------------------------------------------------
void Convex::fitToVisualMesh(bool &cutEmpty, int numFitDirections)
{
	cutEmpty = false;
	if (!mHasExplicitVisMesh)
		return;

	static const int numNormals = 3 + 4;
	static const float normals[numNormals][3] = {
		{1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}
		// avoid singular cases
		// {1.0f, 1.1f, 1.2f}, {-1.3f, 1.2f, 1.0f}, {1.0f, -1.2f, 1.0f}, {1.2f, 1.3f, -1.0f}
	};

	cutEmpty = false;
	const float eps = 1e-3f;

	int num = numFitDirections;
	if (num > numNormals)
		num = numNormals;

	for (int i = 0; i < num; i++) {
		PxVec3 n(normals[i][0], normals[i][1], normals[i][2]);
		n.normalize();
		float minVD = PX_MAX_F32;
		float maxVD = -PX_MAX_F32;
		float minCD = PX_MAX_F32;
		float maxCD = -PX_MAX_F32;
		for (physx::PxU32 j = 0; j < mVisVertices.size(); j++) {
			float d = mVisVertices[j].dot(n);
			if (d < minVD) minVD = d;
			if (d > maxVD) maxVD = d;
		}
		for (physx::PxU32 j = 0; j < mVertices.size(); j++) {
			float d = mVertices[j].dot(n);
			if (d < minCD) minCD = d;
			if (d > maxCD) maxCD = d;
		}
		if (maxVD < maxCD - eps) {
			cut(n, maxVD, cutEmpty, false);
			if (cutEmpty)
				return;
		}
		if (minVD > minCD + eps) {
			cut(-n, -minVD - eps, cutEmpty, false);
			if (cutEmpty)
				return;
		}
	}
	finalize();
}

// --------------------------------------------------------------------------------------------
float Convex::getVolume() const
{
	if (!mVolumeDirty)
		return mVolume;

	mVolume = 0.0f;
	for (physx::PxU32 i = 0; i < mFaces.size(); i++) {
		const Face &f = mFaces[i];
		if (f.numIndices < 3)
			continue;
		const int *ids = &mIndices[(physx::PxU32)f.firstIndex];
		const PxVec3 &p0 = mVertices[(physx::PxU32)ids[0]];
		for (int j = 1; j < f.numIndices-1; j++) {
			const PxVec3 &p1 = mVertices[(physx::PxU32)ids[(physx::PxU32)j]];
			const PxVec3 &p2 = mVertices[(physx::PxU32)ids[(physx::PxU32)j+1]];
			mVolume += p0.cross(p1).dot(p2);
		}
	}
	mVolume *= (1.0f / 6.0f);
	mVolumeDirty = false;
	return mVolume;
}

// --------------------------------------------------------------------------------------------
void Convex::removeInvisibleFacesFlags()
{
	for (physx::PxU32 i = 0; i < mFaces.size(); i++) 
		mFaces[i].flags &= ~CompoundGeometry::FF_INVISIBLE;
}

// --------------------------------------------------------------------------------------------
void Convex::updateFaceVisibility(const float *faceCoverage)
{
	for (physx::PxU32 i = 0; i < mFaces.size(); i++) {
		if (faceCoverage[i] > 0.95f)
			mFaces[i].flags |= CompoundGeometry::FF_INVISIBLE;
		else
			mFaces[i].flags &= ~CompoundGeometry::FF_INVISIBLE;
	}
}

// --------------------------------------------------------------------------------------------
void Convex::clearFraceFlags(unsigned int flag)
{
	for (physx::PxU32 i = 0; i < mFaces.size(); i++) 
		mFaces[i].flags &= ~flag;
}

// --------------------------------------------------------------------------------------------
bool Convex::insideFattened(const PxVec3 &pos, float r) const
{
	// early out with bounding box
	if (pos.x < mBounds.minimum.x - r || pos.x > mBounds.maximum.x + r)
		return false;
	if (pos.y < mBounds.minimum.y - r || pos.y > mBounds.maximum.y + r)
		return false;
	if (pos.z < mBounds.minimum.z - r || pos.z > mBounds.maximum.z + r)
		return false;

	// planes
	for (physx::PxU32 i = 0; i < mPlanes.size(); i++) {
		if (mPlanes[i].n.dot(pos) > mPlanes[i].d + r)
			return false;
	}
	return true;
}

// --------------------------------------------------------------------------------------------
bool Convex::check() {
	PxVec3 center = getCenter();

	for (physx::PxU32 i = 0; i < mFaces.size(); i++) {
		Face &f = mFaces[i];
		if (f.numIndices < 3)
			return false;

		PxVec3 &p0 = mVertices[(physx::PxU32)mIndices[(physx::PxU32)f.firstIndex]];
		PxReal A = 0.0f;
		PxVec3 n(0.0, 0.0, 0.0);

		for (int j = 1; j < f.numIndices-1; j++) {
			PxVec3 &p1 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j)]];
			PxVec3 &p2 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j+1)]];
			PxVec3 nj = (p1-p0).cross(p2-p0);
			A += nj.magnitude();
			n += nj;
		}
		if (A < 1e-6)
			return false;

		PxReal d = n.dot(p0);
		if (n.dot(center) > d)
			return false;
	}
	return true;
}

bool Convex::isOnConvexSurface(const PxVec3 pts) const {
	const float EPSILON = 1e-6f;
	physx::PxU32 numF = mFaces.size();
	for (physx::PxU32 i = 0; i < numF; i++) {

		const Face& f = mFaces[i];
		int nv = f.numIndices;

		float areaFan = 0.0f;
		// Compute face area
		for (int j = 1; j < nv-1; j++) {
			const PxVec3& p0 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex)]];
			const PxVec3& p1 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j)]];
			const PxVec3& p2 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+(j+1))]];
			areaFan += 0.5f* (((p1-p0).cross(p2-p0))).magnitude();
		}
		float areaP = 0.0f;

		for (int j = 0; j < nv; j++) {

			const PxVec3& p1 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+j)]];
			const PxVec3& p2 = mVertices[(physx::PxU32)mIndices[physx::PxU32(f.firstIndex+(j+1) % nv)]];
			areaP += 0.5f*(((p1-pts).cross(p2-pts))).magnitude();
		}
		if ((areaP - areaFan)/areaFan < EPSILON) {
			return true;
		}
	}
	return false;

}

}
}
}
#endif
