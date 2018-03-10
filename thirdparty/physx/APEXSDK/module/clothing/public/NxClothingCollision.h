/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef NX_CLOTHING_COLLISION_H
#define NX_CLOTHING_COLLISION_H

/*!
\file
\brief classes NxClothingCollision, NxClothingPlane, NxClothingConvex, NxClothingSphere, NxClothingCapsule, NxClothingTriangleMesh
*/

#include "NxApexInterface.h"

namespace physx
{
	class PxPlane;

namespace apex
{
PX_PUSH_PACK_DEFAULT


/**
\brief Type of the NxClothingCollision
*/
struct NxClothingCollisionType
{
	/**
	\brief Enum.
	*/
	enum Enum
	{
		Plane,
		Convex,
		Sphere,
		Capsule,
		TriangleMesh
	};
};



class NxClothingPlane;
class NxClothingConvex;
class NxClothingSphere;
class NxClothingCapsule;
class NxClothingTriangleMesh;

/**
\brief Base class of all clothing collision types.
*/
class NxClothingCollision : public NxApexInterface
{
public:
	/**
	\brief Returns the Type of this collision object.
	*/
	virtual NxClothingCollisionType::Enum getType() const = 0;

	/**
	\brief Returns the pointer to this object if it is a plane, NULL otherwise.
	*/
	virtual NxClothingPlane* isPlane() = 0;

	/**
	\brief Returns the pointer to this object if it is a convex, NULL otherwise.
	*/
	virtual NxClothingConvex* isConvex() = 0;

	/**
	\brief Returns the pointer to this object if it is a sphere, NULL otherwise.
	*/
	virtual NxClothingSphere* isSphere() = 0;

	/**
	\brief Returns the pointer to this object if it is a capsule, NULL otherwise.
	*/
	virtual NxClothingCapsule* isCapsule() = 0;

	/**
	\brief Returns the pointer to this object if it is a triangle mesh, NULL otherwise.
	*/
	virtual NxClothingTriangleMesh* isTriangleMesh() = 0;
};



/**
\brief Plane collision of a clothing actor.
*/
class NxClothingPlane : public NxClothingCollision
{
public:
	/**
	\brief Sets the plane equation of the plane.
	*/
	virtual void setPlane(const PxPlane& plane) = 0;

	/**
	\brief Returns the plane equation of the plane.
	*/
	virtual const PxPlane& getPlane() = 0;

	/**
	\brief Returns the number of convexes that reference this plane.
	*/
	virtual PxU32 getRefCount() const = 0;
};



/**
\brief Convex collision of a clothing actor.

A convex is represented by a list of NxClothingPlanes.
*/
class NxClothingConvex : public NxClothingCollision
{
public:
	/**
	\brief Returns the number of planes that define this convex.
	*/
	virtual PxU32 getNumPlanes() = 0;

	/**
	\brief Returns pointer to the planes that define this convex.
	*/
	virtual NxClothingPlane** getPlanes() = 0;

	/**
	\brief Releases this convex and all its planes.

	\note Only use this if the planes are used exclusively by this convex. Planes referenced by other convexes will not be released.
	*/
	virtual void releaseWithPlanes() = 0;
};



/**
\brief Sphere collision of a clothing actor.
*/
class NxClothingSphere : public NxClothingCollision
{
public:
	/**
	\brief Sets the position of this sphere.
	*/
	virtual void setPosition(const PxVec3& position) = 0;

	/**
	\brief Returns the position of this sphere.
	*/
	virtual const PxVec3& getPosition() const = 0;

	/**
	\brief Sets the radius of this sphere.
	*/

	/**
	\brief Sets the Radius of this sphere.
	*/
	virtual void setRadius(PxF32 radius) = 0;

	/**
	\brief Returns the Radius of this sphere.
	*/
	virtual PxF32 getRadius() const = 0;

	/**
	\brief Returns the number of capsules that reference this plane.
	*/
	virtual PxU32 getRefCount() const = 0;
};



/**
\brief Capsule collision of a clothing actor.

A capsule is represented by two NxClothingSpheres.
*/
class NxClothingCapsule : public NxClothingCollision
{
public:
	/**
	\brief Returns the pointers to the two spheres that represent this capsule.
	*/
	virtual NxClothingSphere** getSpheres() = 0;

	/**
	\brief Releases this sphere and its two spheres.

	\note Only use releaseWithSpheres if the 2 spheres are used exclusively by this capsule. Spheres referenced by other convexes will not be released.
	*/
	virtual void releaseWithSpheres() = 0;
};



/**
\brief Triangle mesh collision of a clothing actor.
*/
class NxClothingTriangleMesh : public NxClothingCollision
{
public:
	/**
	\brief Read access to the current triangles of the mesh.
	unlockTriangles needs to be called when done reading.
	\param[out]	ids			Pointer to the triangle ids. Not written if NULL is provided.
	\param[out]	triangles	Pointer to the triangles. Contains (3*number of triangles) vertex positions. Not written if NULL is provided.
	\return					Current number of triangles
	*/
	virtual PxU32 lockTriangles(const PxU32** ids, const PxVec3** triangles) = 0;

	/**
	\brief Write access to the current triangles of the mesh. Changing Ids is not allowed.
	unlockTriangles needs to be called when done editing.
	\param[out]	ids			pointer to the triangle ids. not written if NULL is provided.
	\param[out]	triangles	pointer to the triangles. Contains (3*number of triangles) vertex positions. Not written if NULL is provided.
	\return					Current number of triangles
	*/
	virtual PxU32 lockTrianglesWrite(const PxU32** ids, PxVec3** triangles) = 0;

	/**
	\brief Unlock the mesh data after editing or reading.
	*/
	virtual void unlockTriangles() = 0;

	/**
	\brief Adds a triangle to the mesh. Clockwise winding.
	If a triangle with a given id already exists, it will update that triangle with the new values.
	Ids for triangle removal and adding are accumulated and processed during simulate. That means a triangle
	will not be added if the same id is removed in the same frame, independent of the order of the remove and add calls.
	\param[in]	id					User provided triangle id that allows identifing and removing the triangle.
	\param[in]	v0					First vertex of triangle
	\param[in]	v1					Second vertex of triangle
	\param[in]	v2					Third vertex of triangle
	*/
	virtual void addTriangle(PxU32 id, const PxVec3& v0, const PxVec3& v1, const PxVec3& v2) = 0;

	/**
	\brief Adds a list of triangles to the mesh. Clockwise winding.
	If a triangle with a given id already exists, it will update that triangle with the new values.
	Ids for triangle removal and adding are accumulated and processed during simulate. That means a triangle
	will not be added if the same id is removed in the same frame, independent of the order of the remove and add calls.
	\param[in]	ids					User provided triangle indices that allows identifing and removing the triangle.
									Needs to contain numTriangles indices.
	\param[in]	triangleVertices	Triangle vertices to add. Needs to contain 3*numTriangles vertex positions
	\param[in]	numTriangles		Number of triangles to add
	*/
	virtual void addTriangles(const PxU32* ids, const PxVec3* triangleVertices, PxU32 numTriangles) = 0;

	/**
	\brief Removes triangle with given id.
	Ids for triangle removal and adding are accumulated and processed during simulate. That means a triangle
	will not be added if the same id is removed in the same frame, independent of the order of the remove and add calls.
	\param[in]	id		id of the triangle to be removed
	*/
	virtual void removeTriangle(PxU32 id) = 0;

	/**
	\brief Removes triangles.
	Ids for triangle removal and adding are accumulated and processed during simulate. That means a triangle
	will not be added if the same id is removed in the same frame, independent of the order of the remove and add calls.
	\param[in]	ids					ids of the triangles to be removed. Needs to contain numTriangle indices.
	\param[in]	numTriangles		Number of triangles to remove
	*/
	virtual void removeTriangles(const PxU32* ids, PxU32 numTriangles) = 0;

	/**
	\brief Clear all triangles to start with an empty mesh.
	*/
	virtual void clearTriangles() = 0;

	/**
	\brief Sets the global pose of the mesh.
	*/
	virtual void setPose(PxMat44 pose) = 0;

	/**
	\brief Returns the global pose of the mesh.
	*/
	virtual const PxMat44& getPose() const = 0;
};



PX_POP_PACK
} // namespace apex
} // namespace physx


#endif // NX_CLOTHING_COLLISION_H