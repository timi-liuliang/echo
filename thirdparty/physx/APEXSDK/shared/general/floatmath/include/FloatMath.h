/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef FLOAT_MATH_LIB_H

#define FLOAT_MATH_LIB_H

#include "PsShare.h"
#include "PsUserAllocated.h"

// a set of routines that let you do common 3d math
// operations without any vector, matrix, or quaternion
// classes or templates.
//
// a vector (or point) is a 'PxF32 *' to 3 floating point numbers.
// a matrix is a 'PxF32 *' to an array of 16 floating point numbers representing a 4x4 transformation matrix compatible with D3D or OGL
// a quaternion is a 'PxF32 *' to 4 floats representing a quaternion x,y,z,w
//
//


#include <float.h>
#include "PsShare.h"

namespace physx
{
	namespace general_floatmath2
	{
		using namespace shdfnd;

enum FM_ClipState
{
  FMCS_XMIN       = (1<<0),
  FMCS_XMAX       = (1<<1),
  FMCS_YMIN       = (1<<2),
  FMCS_YMAX       = (1<<3),
  FMCS_ZMIN       = (1<<4),
  FMCS_ZMAX       = (1<<5),
};

enum FM_Axis
{
  FM_XAXIS   = (1<<0),
  FM_YAXIS   = (1<<1),
  FM_ZAXIS   = (1<<2)
};

enum LineSegmentType
{
  LS_START,
  LS_MIDDLE,
  LS_END
};


const PxF32 FM_PI = 3.1415926535897932384626433832795028841971693993751f;
const PxF32 FM_DEG_TO_RAD = ((2.0f * FM_PI) / 360.0f);
const PxF32 FM_RAD_TO_DEG = (360.0f / (2.0f * FM_PI));

//***************** Float versions
//***
//*** vectors are assumed to be 3 floats or 3 doubles representing X, Y, Z
//*** quaternions are assumed to be 4 floats or 4 doubles representing X,Y,Z,W
//*** matrices are assumed to be 16 floats or 16 doubles representing a standard D3D or OpenGL style 4x4 matrix
//*** bounding volumes are expressed as two sets of 3 floats/PxF64 representing bmin(x,y,z) and bmax(x,y,z)
//*** Plane equations are assumed to be 4 floats or 4 doubles representing Ax,By,Cz,D

FM_Axis fm_getDominantAxis(const PxF32 normal[3]);
FM_Axis fm_getDominantAxis(const PxF64 normal[3]);

void fm_decomposeTransform(const PxF32 local_transform[16],PxF32 trans[3],PxF32 rot[4],PxF32 scale[3]);
void fm_decomposeTransform(const PxF64 local_transform[16],PxF64 trans[3],PxF64 rot[4],PxF64 scale[3]);

void  fm_multiplyTransform(const PxF32 *pA,const PxF32 *pB,PxF32 *pM);
void  fm_multiplyTransform(const PxF64 *pA,const PxF64 *pB,PxF64 *pM);

void  fm_inverseTransform(const PxF32 matrix[16],PxF32 inverse_matrix[16]);
void  fm_inverseTransform(const PxF64 matrix[16],PxF64 inverse_matrix[16]);

void  fm_identity(PxF32 matrix[16]); // set 4x4 matrix to identity.
void  fm_identity(PxF64 matrix[16]); // set 4x4 matrix to identity.

void  fm_inverseRT(const PxF32 matrix[16], const PxF32 pos[3], PxF32 t[3]); // inverse rotate translate the point.
void  fm_inverseRT(const PxF64 matrix[16],const PxF64 pos[3],PxF64 t[3]); // inverse rotate translate the point.

void  fm_transform(const PxF32 matrix[16], const PxF32 pos[3], PxF32 t[3]); // rotate and translate this point.
void  fm_transform(const PxF64 matrix[16],const PxF64 pos[3],PxF64 t[3]); // rotate and translate this point.

PxF32  fm_getDeterminant(const PxF32 matrix[16]);
PxF64 fm_getDeterminant(const PxF64 matrix[16]);

void fm_getSubMatrix(PxI32 ki,PxI32 kj,PxF32 pDst[16],const PxF32 matrix[16]);
void fm_getSubMatrix(PxI32 ki,PxI32 kj,PxF64 pDst[16],const PxF32 matrix[16]);

void  fm_rotate(const PxF32 matrix[16],const PxF32 pos[3],PxF32 t[3]); // only rotate the point by a 4x4 matrix, don't translate.
void  fm_rotate(const PxF64 matri[16],const PxF64 pos[3],PxF64 t[3]); // only rotate the point by a 4x4 matrix, don't translate.

void  fm_eulerToMatrix(PxF32 ax,PxF32 ay,PxF32 az,PxF32 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
void  fm_eulerToMatrix(PxF64 ax,PxF64 ay,PxF64 az,PxF64 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)

void  fm_getAABB(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 bmin[3],PxF32 bmax[3]);
void  fm_getAABB(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 bmin[3],PxF64 bmax[3]);

void  fm_getAABBCenter(const PxF32 bmin[3],const PxF32 bmax[3],PxF32 center[3]);
void  fm_getAABBCenter(const PxF64 bmin[3],const PxF64 bmax[3],PxF64 center[3]);

void fm_transformAABB(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 matrix[16],PxF32 tbmin[3],PxF32 tbmax[3]);
void fm_transformAABB(const PxF64 bmin[3],const PxF64 bmax[3],const PxF64 matrix[16],PxF64 tbmin[3],PxF64 tbmax[3]);

void  fm_eulerToQuat(PxF32 x,PxF32 y,PxF32 z,PxF32 quat[4]); // convert euler angles to quaternion.
void  fm_eulerToQuat(PxF64 x,PxF64 y,PxF64 z,PxF64 quat[4]); // convert euler angles to quaternion.

void  fm_quatToEuler(const PxF32 quat[4],PxF32 &ax,PxF32 &ay,PxF32 &az);
void  fm_quatToEuler(const PxF64 quat[4],PxF64 &ax,PxF64 &ay,PxF64 &az);

void  fm_eulerToQuat(const PxF32 euler[3],PxF32 quat[4]); // convert euler angles to quaternion. Angles must be radians not degrees!
void  fm_eulerToQuat(const PxF64 euler[3],PxF64 quat[4]); // convert euler angles to quaternion.

void  fm_scale(PxF32 x,PxF32 y,PxF32 z,PxF32 matrix[16]); // apply scale to the matrix.
void  fm_scale(PxF64 x,PxF64 y,PxF64 z,PxF64 matrix[16]); // apply scale to the matrix.

void  fm_eulerToQuatDX(PxF32 x,PxF32 y,PxF32 z,PxF32 quat[4]); // convert euler angles to quaternion using the fucked up DirectX method
void  fm_eulerToQuatDX(PxF64 x,PxF64 y,PxF64 z,PxF64 quat[4]); // convert euler angles to quaternion using the fucked up DirectX method

void  fm_eulerToMatrixDX(PxF32 x,PxF32 y,PxF32 z,PxF32 matrix[16]); // convert euler angles to quaternion using the fucked up DirectX method.
void  fm_eulerToMatrixDX(PxF64 x,PxF64 y,PxF64 z,PxF64 matrix[16]); // convert euler angles to quaternion using the fucked up DirectX method.

void  fm_quatToMatrix(const PxF32 quat[4],PxF32 matrix[16]); // convert quaterinion rotation to matrix, translation set to zero.
void  fm_quatToMatrix(const PxF64 quat[4],PxF64 matrix[16]); // convert quaterinion rotation to matrix, translation set to zero.

void  fm_quatRotate(const PxF32 quat[4],const PxF32 v[3],PxF32 r[3]); // rotate a vector directly by a quaternion.
void  fm_quatRotate(const PxF64 quat[4],const PxF64 v[3],PxF64 r[3]); // rotate a vector directly by a quaternion.

void  fm_getTranslation(const PxF32 matrix[16],PxF32 t[3]);
void  fm_getTranslation(const PxF64 matrix[16],PxF64 t[3]);

void  fm_setTranslation(const PxF32 *translation,PxF32 matrix[16]);
void  fm_setTranslation(const PxF64 *translation,PxF64 matrix[16]);

void  fm_multiplyQuat(const PxF32 *qa,const PxF32 *qb,PxF32 *quat);
void  fm_multiplyQuat(const PxF64 *qa,const PxF64 *qb,PxF64 *quat);

void  fm_matrixToQuat(const PxF32 matrix[16],PxF32 quat[4]); // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w
void  fm_matrixToQuat(const PxF64 matrix[16],PxF64 quat[4]); // convert the 3x3 portion of a 4x4 matrix into a quaterion as x,y,z,w

PxF32 fm_sphereVolume(PxF32 radius); // return's the volume of a sphere of this radius (4/3 PI * R cubed )
PxF64 fm_sphereVolume(PxF64 radius); // return's the volume of a sphere of this radius (4/3 PI * R cubed )

PxF32 fm_cylinderVolume(PxF32 radius,PxF32 h);
PxF64 fm_cylinderVolume(PxF64 radius,PxF64 h);

PxF32 fm_capsuleVolume(PxF32 radius,PxF32 h);
PxF64 fm_capsuleVolume(PxF64 radius,PxF64 h);

PxF32 fm_distance(const PxF32 p1[3],const PxF32 p2[3]);
PxF64 fm_distance(const PxF64 p1[3],const PxF64 p2[3]);

PxF32 fm_distanceSquared(const PxF32 p1[3],const PxF32 p2[3]);
PxF64 fm_distanceSquared(const PxF64 p1[3],const PxF64 p2[3]);

PxF32 fm_distanceSquaredXZ(const PxF32 p1[3],const PxF32 p2[3]);
PxF64 fm_distanceSquaredXZ(const PxF64 p1[3],const PxF64 p2[3]);

PxF32 fm_computePlane(const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3],PxF32 *n); // return D
PxF64 fm_computePlane(const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3],PxF64 *n); // return D

PxF32 fm_distToPlane(const PxF32 plane[4],const PxF32 pos[3]); // computes the distance of this point from the plane.
PxF64 fm_distToPlane(const PxF64 plane[4],const PxF64 pos[3]); // computes the distance of this point from the plane.

PxF32 fm_dot(const PxF32 p1[3],const PxF32 p2[3]);
PxF64 fm_dot(const PxF64 p1[3],const PxF64 p2[3]);

void  fm_cross(PxF32 cross[3],const PxF32 a[3],const PxF32 b[3]);
void  fm_cross(PxF64 cross[3],const PxF64 a[3],const PxF64 b[3]);

void  fm_computeNormalVector(PxF32 n[3],const PxF32 p1[3],const PxF32 p2[3]); // as P2-P1 normalized.
void  fm_computeNormalVector(PxF64 n[3],const PxF64 p1[3],const PxF64 p2[3]); // as P2-P1 normalized.

bool  fm_computeWindingOrder(const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3]); // returns true if the triangle is clockwise.
bool  fm_computeWindingOrder(const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3]); // returns true if the triangle is clockwise.

PxF32  fm_normalize(PxF32 n[3]); // normalize this vector and return the distance
PxF64  fm_normalize(PxF64 n[3]); // normalize this vector and return the distance

PxF32  fm_normalizeQuat(PxF32 n[4]); // normalize this quat
PxF64  fm_normalizeQuat(PxF64 n[4]); // normalize this quat

void  fm_matrixMultiply(const PxF32 A[16],const PxF32 B[16],PxF32 dest[16]);
void  fm_matrixMultiply(const PxF64 A[16],const PxF64 B[16],PxF64 dest[16]);

void  fm_composeTransform(const PxF32 position[3],const PxF32 quat[4],const PxF32 scale[3],PxF32 matrix[16]);
void  fm_composeTransform(const PxF64 position[3],const PxF64 quat[4],const PxF64 scale[3],PxF64 matrix[16]);

PxF32 fm_computeArea(const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3]);
PxF64 fm_computeArea(const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3]);

void  fm_lerp(const PxF32 p1[3],const PxF32 p2[3],PxF32 dest[3],PxF32 lerpValue);
void  fm_lerp(const PxF64 p1[3],const PxF64 p2[3],PxF64 dest[3],PxF64 lerpValue);

bool  fm_insideTriangleXZ(const PxF32 test[3],const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3]);
bool  fm_insideTriangleXZ(const PxF64 test[3],const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3]);

bool  fm_insideAABB(const PxF32 pos[3],const PxF32 bmin[3],const PxF32 bmax[3]);
bool  fm_insideAABB(const PxF64 pos[3],const PxF64 bmin[3],const PxF64 bmax[3]);

bool  fm_insideAABB(const PxF32 obmin[3],const PxF32 obmax[3],const PxF32 tbmin[3],const PxF32 tbmax[3]); // test if bounding box tbmin/tmbax is fully inside obmin/obmax
bool  fm_insideAABB(const PxF64 obmin[3],const PxF64 obmax[3],const PxF64 tbmin[3],const PxF64 tbmax[3]); // test if bounding box tbmin/tmbax is fully inside obmin/obmax

PxU32 fm_clipTestPoint(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 pos[3]);
PxU32 fm_clipTestPoint(const PxF64 bmin[3],const PxF64 bmax[3],const PxF64 pos[3]);

PxU32 fm_clipTestPointXZ(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 pos[3]); // only tests X and Z, not Y
PxU32 fm_clipTestPointXZ(const PxF64 bmin[3],const PxF64 bmax[3],const PxF64 pos[3]); // only tests X and Z, not Y


PxU32 fm_clipTestAABB(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3],PxU32 &andCode);
PxU32 fm_clipTestAABB(const PxF64 bmin[3],const PxF64 bmax[3],const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3],PxU32 &andCode);


bool     fm_lineTestAABBXZ(const PxF32 p1[3],const PxF32 p2[3],const PxF32 bmin[3],const PxF32 bmax[3],PxF32 &time);
bool     fm_lineTestAABBXZ(const PxF64 p1[3],const PxF64 p2[3],const PxF64 bmin[3],const PxF64 bmax[3],PxF64 &time);

bool     fm_lineTestAABB(const PxF32 p1[3],const PxF32 p2[3],const PxF32 bmin[3],const PxF32 bmax[3],PxF32 &time);
bool     fm_lineTestAABB(const PxF64 p1[3],const PxF64 p2[3],const PxF64 bmin[3],const PxF64 bmax[3],PxF64 &time);


void  fm_initMinMax(const PxF32 p[3],PxF32 bmin[3],PxF32 bmax[3]);
void  fm_initMinMax(const PxF64 p[3],PxF64 bmin[3],PxF64 bmax[3]);

void  fm_initMinMax(PxF32 bmin[3],PxF32 bmax[3]);
void  fm_initMinMax(PxF64 bmin[3],PxF64 bmax[3]);

void  fm_minmax(const PxF32 p[3],PxF32 bmin[3],PxF32 bmax[3]); // accmulate to a min-max value
void  fm_minmax(const PxF64 p[3],PxF64 bmin[3],PxF64 bmax[3]); // accmulate to a min-max value


PxF32 fm_solveX(const PxF32 plane[4],PxF32 y,PxF32 z); // solve for X given this plane equation and the other two components.
PxF64 fm_solveX(const PxF64 plane[4],PxF64 y,PxF64 z); // solve for X given this plane equation and the other two components.

PxF32 fm_solveY(const PxF32 plane[4],PxF32 x,PxF32 z); // solve for Y given this plane equation and the other two components.
PxF64 fm_solveY(const PxF64 plane[4],PxF64 x,PxF64 z); // solve for Y given this plane equation and the other two components.

PxF32 fm_solveZ(const PxF32 plane[4],PxF32 x,PxF32 y); // solve for Z given this plane equation and the other two components.
PxF64 fm_solveZ(const PxF64 plane[4],PxF64 x,PxF64 y); // solve for Z given this plane equation and the other two components.

bool  fm_computeBestFitPlane(PxU32 vcount,     // number of input data points
                     const PxF32 *points,     // starting address of points array.
                     PxU32 vstride,    // stride between input points.
                     const PxF32 *weights,    // *optional point weighting values.
                     PxU32 wstride,    // weight stride for each vertex.
                     PxF32 plane[4]);

bool  fm_computeBestFitPlane(PxU32 vcount,     // number of input data points
                     const PxF64 *points,     // starting address of points array.
                     PxU32 vstride,    // stride between input points.
                     const PxF64 *weights,    // *optional point weighting values.
                     PxU32 wstride,    // weight stride for each vertex.
                     PxF64 plane[4]);

bool  fm_computeCentroid(PxU32 vcount,     // number of input data points
						 const PxF32 *points,     // starting address of points array.
						 PxU32 vstride,    // stride between input points.
						 PxF32 *center);

bool  fm_computeCentroid(PxU32 vcount,     // number of input data points
						 const PxF64 *points,     // starting address of points array.
						 PxU32 vstride,    // stride between input points.
						 PxF64 *center);


PxF32  fm_computeBestFitAABB(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 bmin[3],PxF32 bmax[3]); // returns the diagonal distance
PxF64 fm_computeBestFitAABB(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 bmin[3],PxF64 bmax[3]); // returns the diagonal distance

PxF32  fm_computeBestFitSphere(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 center[3]);
PxF64  fm_computeBestFitSphere(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 center[3]);

bool fm_lineSphereIntersect(const PxF32 center[3],PxF32 radius,const PxF32 p1[3],const PxF32 p2[3],PxF32 intersect[3]);
bool fm_lineSphereIntersect(const PxF64 center[3],PxF64 radius,const PxF64 p1[3],const PxF64 p2[3],PxF64 intersect[3]);

bool fm_intersectRayAABB(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 pos[3],const PxF32 dir[3],PxF32 intersect[3]);
bool fm_intersectLineSegmentAABB(const PxF32 bmin[3],const PxF32 bmax[3],const PxF32 p1[3],const PxF32 p2[3],PxF32 intersect[3]);

bool fm_lineIntersectsTriangle(const PxF32 rayStart[3],const PxF32 rayEnd[3],const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3],PxF32 sect[3]);
bool fm_lineIntersectsTriangle(const PxF64 rayStart[3],const PxF64 rayEnd[3],const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3],PxF64 sect[3]);

bool fm_rayIntersectsTriangle(const PxF32 origin[3],const PxF32 dir[3],const PxF32 v0[3],const PxF32 v1[3],const PxF32 v2[3],PxF32 &t);
bool fm_rayIntersectsTriangle(const PxF64 origin[3],const PxF64 dir[3],const PxF64 v0[3],const PxF64 v1[3],const PxF64 v2[3],PxF64 &t);

bool fm_raySphereIntersect(const PxF32 center[3],PxF32 radius,const PxF32 pos[3],const PxF32 dir[3],PxF32 distance,PxF32 intersect[3]);
bool fm_raySphereIntersect(const PxF64 center[3],PxF64 radius,const PxF64 pos[3],const PxF64 dir[3],PxF64 distance,PxF64 intersect[3]);

void fm_catmullRom(PxF32 out_vector[3],const PxF32 p1[3],const PxF32 p2[3],const PxF32 p3[3],const PxF32 *p4, const PxF32 s);
void fm_catmullRom(PxF64 out_vector[3],const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3],const PxF64 *p4, const PxF64 s);

bool fm_intersectAABB(const PxF32 bmin1[3],const PxF32 bmax1[3],const PxF32 bmin2[3],const PxF32 bmax2[3]);
bool fm_intersectAABB(const PxF64 bmin1[3],const PxF64 bmax1[3],const PxF64 bmin2[3],const PxF64 bmax2[3]);


// computes the rotation quaternion to go from unit-vector v0 to unit-vector v1
void fm_rotationArc(const PxF32 v0[3],const PxF32 v1[3],PxF32 quat[4]);
void fm_rotationArc(const PxF64 v0[3],const PxF64 v1[3],PxF64 quat[4]);

PxF32  fm_distancePointLineSegment(const PxF32 Point[3],const PxF32 LineStart[3],const PxF32 LineEnd[3],PxF32 intersection[3],LineSegmentType &type,PxF32 epsilon);
PxF64 fm_distancePointLineSegment(const PxF64 Point[3],const PxF64 LineStart[3],const PxF64 LineEnd[3],PxF64 intersection[3],LineSegmentType &type,PxF64 epsilon);


bool fm_colinear(const PxF64 p1[3],const PxF64 p2[3],const PxF64 p3[3],PxF64 epsilon=0.999);               // true if these three points in a row are co-linear
bool fm_colinear(const PxF32  p1[3],const PxF32  p2[3],const PxF32 p3[3],PxF32 epsilon=0.999f);

bool fm_colinear(const PxF32 a1[3],const PxF32 a2[3],const PxF32 b1[3],const PxF32 b2[3],PxF32 epsilon=0.999f);  // true if these two line segments are co-linear.
bool fm_colinear(const PxF64 a1[3],const PxF64 a2[3],const PxF64 b1[3],const PxF64 b2[3],PxF64 epsilon=0.999);  // true if these two line segments are co-linear.

enum IntersectResult
{
  IR_DONT_INTERSECT,
  IR_DO_INTERSECT,
  IR_COINCIDENT,
  IR_PARALLEL,
};

IntersectResult fm_intersectLineSegments2d(const PxF32 a1[3], const PxF32 a2[3], const PxF32 b1[3], const PxF32 b2[3], PxF32 intersectionPoint[3]);
IntersectResult fm_intersectLineSegments2d(const PxF64 a1[3],const PxF64 a2[3],const PxF64 b1[3],const PxF64 b2[3],PxF64 intersectionPoint[3]);

IntersectResult fm_intersectLineSegments2dTime(const PxF32 a1[3], const PxF32 a2[3], const PxF32 b1[3], const PxF32 b2[3],PxF32 &t1,PxF32 &t2);
IntersectResult fm_intersectLineSegments2dTime(const PxF64 a1[3],const PxF64 a2[3],const PxF64 b1[3],const PxF64 b2[3],PxF64 &t1,PxF64 &t2);

// Plane-Triangle splitting

enum PlaneTriResult
{
  PTR_ON_PLANE,
  PTR_FRONT,
  PTR_BACK,
  PTR_SPLIT,
};

PlaneTriResult fm_planeTriIntersection(const PxF32 plane[4],    // the plane equation in Ax+By+Cz+D format
                                    const PxF32 *triangle, // the source triangle.
                                    PxU32 tstride,  // stride in bytes of the input and output *vertices*
                                    PxF32        epsilon,  // the co-planer epsilon value.
                                    PxF32       *front,    // the triangle in front of the
                                    PxU32 &fcount,  // number of vertices in the 'front' triangle
                                    PxF32       *back,     // the triangle in back of the plane
                                    PxU32 &bcount); // the number of vertices in the 'back' triangle.


PlaneTriResult fm_planeTriIntersection(const PxF64 plane[4],    // the plane equation in Ax+By+Cz+D format
                                    const PxF64 *triangle, // the source triangle.
                                    PxU32 tstride,  // stride in bytes of the input and output *vertices*
                                    PxF64        epsilon,  // the co-planer epsilon value.
                                    PxF64       *front,    // the triangle in front of the
                                    PxU32 &fcount,  // number of vertices in the 'front' triangle
                                    PxF64       *back,     // the triangle in back of the plane
                                    PxU32 &bcount); // the number of vertices in the 'back' triangle.


void fm_intersectPointPlane(const PxF32 p1[3],const PxF32 p2[3],PxF32 *split,const PxF32 plane[4]);
void fm_intersectPointPlane(const PxF64 p1[3],const PxF64 p2[3],PxF64 *split,const PxF64 plane[4]);

PlaneTriResult fm_getSidePlane(const PxF32 p[3],const PxF32 plane[4],PxF32 epsilon);
PlaneTriResult fm_getSidePlane(const PxF64 p[3],const PxF64 plane[4],PxF64 epsilon);


void fm_computeBestFitOBB(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 *sides,PxF32 matrix[16],bool bruteForce=true);
void fm_computeBestFitOBB(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 *sides,PxF64 matrix[16],bool bruteForce=true);

void fm_computeBestFitOBB(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 *sides,PxF32 pos[3],PxF32 quat[4],bool bruteForce=true);
void fm_computeBestFitOBB(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 *sides,PxF64 pos[3],PxF64 quat[4],bool bruteForce=true);

void fm_computeBestFitABB(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 *sides,PxF32 pos[3]);
void fm_computeBestFitABB(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF64 *sides,PxF64 pos[3]);


//** Note, if the returned capsule height is less than zero, then you must represent it is a sphere of size radius.
void fm_computeBestFitCapsule(PxU32 vcount,const PxF32 *points,PxU32 pstride,PxF32 &radius,PxF32 &height,PxF32 matrix[16],bool bruteForce=true);
void fm_computeBestFitCapsule(PxU32 vcount,const PxF64 *points,PxU32 pstride,PxF32 &radius,PxF32 &height,PxF64 matrix[16],bool bruteForce=true);


void fm_planeToMatrix(const PxF32 plane[4],PxF32 matrix[16]); // convert a plane equation to a 4x4 rotation matrix.  Reference vector is 0,1,0
void fm_planeToQuat(const PxF32 plane[4],PxF32 quat[4],PxF32 pos[3]); // convert a plane equation to a quaternion and translation

void fm_planeToMatrix(const PxF64 plane[4],PxF64 matrix[16]); // convert a plane equation to a 4x4 rotation matrix
void fm_planeToQuat(const PxF64 plane[4],PxF64 quat[4],PxF64 pos[3]); // convert a plane equation to a quaternion and translation

inline void fm_doubleToFloat3(const PxF64 p[3],PxF32 t[3]) { t[0] = (PxF32) p[0]; t[1] = (PxF32)p[1]; t[2] = (PxF32)p[2]; };
inline void fm_floatToDouble3(const PxF32 p[3],PxF64 t[3]) { t[0] = (PxF64)p[0]; t[1] = (PxF64)p[1]; t[2] = (PxF64)p[2]; };


void  fm_eulerMatrix(PxF32 ax,PxF32 ay,PxF32 az,PxF32 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)
void  fm_eulerMatrix(PxF64 ax,PxF64 ay,PxF64 az,PxF64 matrix[16]); // convert euler (in radians) to a dest 4x4 matrix (translation set to zero)


PxF32  fm_computeMeshVolume(const PxF32 *vertices,PxU32 tcount,const PxU32 *indices);
PxF64 fm_computeMeshVolume(const PxF64 *vertices,PxU32 tcount,const PxU32 *indices);


#define FM_DEFAULT_GRANULARITY 0.001f  // 1 millimeter is the default granularity

class fm_VertexIndex
{
public:
  virtual PxU32          getIndex(const PxF32 pos[3],bool &newPos) = 0;  // get welded index for this PxF32 vector[3]
  virtual PxU32          getIndex(const PxF64 pos[3],bool &newPos) = 0;  // get welded index for this PxF64 vector[3]
  virtual const PxF32 *   getVerticesFloat(void) const = 0;
  virtual const PxF64 *  getVerticesDouble(void) const = 0;
  virtual const PxF32 *   getVertexFloat(PxU32 index) const = 0;
  virtual const PxF64 *  getVertexDouble(PxU32 index) const = 0;
  virtual PxU32          getVcount(void) const = 0;
  virtual bool            isDouble(void) const = 0;
  virtual bool            saveAsObj(const char *fname,PxU32 tcount,PxU32 *indices) = 0;
};

fm_VertexIndex * fm_createVertexIndex(PxF64 granularity,bool snapToGrid); // create an indexed vertex system for doubles
fm_VertexIndex * fm_createVertexIndex(PxF32 granularity,bool snapToGrid);  // create an indexed vertext system for floats
void             fm_releaseVertexIndex(fm_VertexIndex *vindex);



#if 0 // currently disabled

class fm_LineSegment
{
public:
  fm_LineSegment(void)
  {
    mE1 = mE2 = 0;
  }

  fm_LineSegment(PxU32 e1,PxU32 e2)
  {
    mE1 = e1;
    mE2 = e2;
  }

  PxU32 mE1;
  PxU32 mE2;
};


// LineSweep *only* supports doublees.  As a geometric operation it needs as much precision as possible.
class fm_LineSweep
{
public:

 virtual fm_LineSegment * performLineSweep(const fm_LineSegment *segments,
                                   PxU32 icount,
                                   const PxF64 *planeEquation,
                                   fm_VertexIndex *pool,
                                   PxU32 &scount) = 0;


};

fm_LineSweep * fm_createLineSweep(void);
void           fm_releaseLineSweep(fm_LineSweep *sweep);

#endif

class fm_Triangulate
{
public:
  virtual const PxF64 *       triangulate3d(PxU32 pcount,
                                             const PxF64 *points,
                                             PxU32 vstride,
                                             PxU32 &tcount,
                                             bool consolidate,
                                             PxF64 epsilon) = 0;

  virtual const PxF32  *       triangulate3d(PxU32 pcount,
                                             const PxF32  *points,
                                             PxU32 vstride,
                                             PxU32 &tcount,
                                             bool consolidate,
                                             PxF32 epsilon) = 0;
};

fm_Triangulate * fm_createTriangulate(void);
void             fm_releaseTriangulate(fm_Triangulate *t);


const PxF32 * fm_getPoint(const PxF32 *points,PxU32 pstride,PxU32 index);
const PxF64 * fm_getPoint(const PxF64 *points,PxU32 pstride,PxU32 index);

bool   fm_insideTriangle(PxF32 Ax, PxF32 Ay,PxF32 Bx, PxF32 By,PxF32 Cx, PxF32 Cy,PxF32 Px, PxF32 Py);
bool   fm_insideTriangle(PxF64 Ax, PxF64 Ay,PxF64 Bx, PxF64 By,PxF64 Cx, PxF64 Cy,PxF64 Px, PxF64 Py);
PxF32  fm_areaPolygon2d(PxU32 pcount,const PxF32 *points,PxU32 pstride);
PxF64 fm_areaPolygon2d(PxU32 pcount,const PxF64 *points,PxU32 pstride);

bool  fm_pointInsidePolygon2d(PxU32 pcount,const PxF32 *points,PxU32 pstride,const PxF32 *point,PxU32 xindex=0,PxU32 yindex=1);
bool  fm_pointInsidePolygon2d(PxU32 pcount,const PxF64 *points,PxU32 pstride,const PxF64 *point,PxU32 xindex=0,PxU32 yindex=1);

PxU32 fm_consolidatePolygon(PxU32 pcount,const PxF32 *points,PxU32 pstride,PxF32 *dest,PxF32 epsilon=0.999999f); // collapses co-linear edges.
PxU32 fm_consolidatePolygon(PxU32 pcount,const PxF64 *points,PxU32 pstride,PxF64 *dest,PxF64 epsilon=0.999999); // collapses co-linear edges.


bool fm_computeSplitPlane(PxU32 vcount,const PxF64 *vertices,PxU32 tcount,const PxU32 *indices,PxF64 *plane);
bool fm_computeSplitPlane(PxU32 vcount,const PxF32 *vertices,PxU32 tcount,const PxU32 *indices,PxF32 *plane);

void fm_nearestPointInTriangle(const PxF32 *pos,const PxF32 *p1,const PxF32 *p2,const PxF32 *p3,PxF32 *nearest);
void fm_nearestPointInTriangle(const PxF64 *pos,const PxF64 *p1,const PxF64 *p2,const PxF64 *p3,PxF64 *nearest);

PxF32  fm_areaTriangle(const PxF32 *p1,const PxF32 *p2,const PxF32 *p3);
PxF64 fm_areaTriangle(const PxF64 *p1,const PxF64 *p2,const PxF64 *p3);

void fm_subtract(const PxF32 *A,const PxF32 *B,PxF32 *diff); // compute A-B and store the result in 'diff'
void fm_subtract(const PxF64 *A,const PxF64 *B,PxF64 *diff); // compute A-B and store the result in 'diff'

void fm_multiply(PxF32 *A,PxF32 scaler);
void fm_multiply(PxF64 *A,PxF64 scaler);

void fm_add(const PxF32 *A,const PxF32 *B,PxF32 *sum);
void fm_add(const PxF64 *A,const PxF64 *B,PxF64 *sum);

void fm_copy3(const PxF32 *source,PxF32 *dest);
void fm_copy3(const PxF64 *source,PxF64 *dest);

// re-indexes an indexed triangle mesh but drops unused vertices.  The output_indices can be the same pointer as the input indices.
// the output_vertices can point to the input vertices if you desire.  The output_vertices buffer should be at least the same size
// is the input buffer.  The routine returns the new vertex count after re-indexing.
PxU32  fm_copyUniqueVertices(PxU32 vcount,const PxF32 *input_vertices,PxF32 *output_vertices,PxU32 tcount,const PxU32 *input_indices,PxU32 *output_indices);
PxU32  fm_copyUniqueVertices(PxU32 vcount,const PxF64 *input_vertices,PxF64 *output_vertices,PxU32 tcount,const PxU32 *input_indices,PxU32 *output_indices);

bool    fm_isMeshCoplanar(PxU32 tcount,const PxU32 *indices,const PxF32 *vertices,bool doubleSided); // returns true if this collection of indexed triangles are co-planar!
bool    fm_isMeshCoplanar(PxU32 tcount,const PxU32 *indices,const PxF64 *vertices,bool doubleSided); // returns true if this collection of indexed triangles are co-planar!

bool    fm_samePlane(const PxF32 p1[4],const PxF32 p2[4],PxF32 normalEpsilon=0.01f,PxF32 dEpsilon=0.001f,bool doubleSided=false); // returns true if these two plane equations are identical within an epsilon
bool    fm_samePlane(const PxF64 p1[4],const PxF64 p2[4],PxF64 normalEpsilon=0.01,PxF64 dEpsilon=0.001,bool doubleSided=false);

void    fm_OBBtoAABB(const PxF32 obmin[3],const PxF32 obmax[3],const PxF32 matrix[16],PxF32 abmin[3],PxF32 abmax[3]);

// a utility class that will tesseleate a mesh.
class fm_Tesselate
{
public:
  virtual const PxU32 * tesselate(fm_VertexIndex *vindex,PxU32 tcount,const PxU32 *indices,PxF32 longEdge,PxU32 maxDepth,PxU32 &outcount) = 0;
};

fm_Tesselate * fm_createTesselate(void);
void           fm_releaseTesselate(fm_Tesselate *t);

void fm_computeMeanNormals(PxU32 vcount,       // the number of vertices
                           const PxF32 *vertices,     // the base address of the vertex position data.
                           PxU32 vstride,      // the stride between position data.
                           PxF32 *normals,            // the base address  of the destination for mean vector normals
                           PxU32 nstride,      // the stride between normals
                           PxU32 tcount,       // the number of triangles
                           const PxU32 *indices);     // the triangle indices

void fm_computeMeanNormals(PxU32 vcount,       // the number of vertices
                           const PxF64 *vertices,     // the base address of the vertex position data.
                           PxU32 vstride,      // the stride between position data.
                           PxF64 *normals,            // the base address  of the destination for mean vector normals
                           PxU32 nstride,      // the stride between normals
                           PxU32 tcount,       // the number of triangles
                           const PxU32 *indices);     // the triangle indices


bool fm_isValidTriangle(const PxF32 *p1,const PxF32 *p2,const PxF32 *p3,PxF32 epsilon=0.00001f);
bool fm_isValidTriangle(const PxF64 *p1,const PxF64 *p2,const PxF64 *p3,PxF64 epsilon=0.00001f);

}; // end of namespace
using namespace general_floatmath2;
};

#endif
