//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.  
//
//

#pragma once

#include "GeomExport.h"
#include <WTypes.h>
#include "point2.h"
#include "point3.h"

// forward declarations
class Box3;
class Matrix3;

/** 
 * Calculates the barycentric coordinates of the point p in 3-dimensional space 
 * according to a reference triangle indicated by its three vertices p0, p1 and p2.
 * \param p0,p1,p2 The three vertices of the reference triangle in 3-dimensional space. 
 * \param p The point for which the barycentric coordinates should be calculated.
 * \return Barycentric coordinates of point p.
 * \see BaryCoords(Point2, Point2, Point2, Point2)
 */
GEOMEXPORT Point3 BaryCoords(Point3 p0, Point3 p1, Point3 p2, Point3 p);

/** 
 * Calculates the barycentric coordinates of the point p in 2-dimensional space 
 * according to a reference triangle indicated by its three vertices p0, p1 and p2.
 * \param p0,p1,p2 The three vertices of the reference triangle in 2-dimensional space. 
 * \param p The point for which the barycentric coordinates should be calculated.
 * \return Barycentric coordinates of point p.
 * \see BaryCoords(Point3, Point3, Point3, Point3)
 */
GEOMEXPORT Point3 BaryCoords(Point2 p0, Point2 p1, Point2 p2, Point2 p);

/**
 * Checks if the given ray intersects a three-dimensional box.
 * \param ray The ray
 * \param b The box  
 * \return \c true if the ray intersects the box, \c false otherwise.
 */
GEOMEXPORT BOOL RayHitsBox(Ray &ray, Box3& b);

/**
 * Calculates the distance of a given point from a line in two dimensions.
 * \param p0,p1 Pointers to the two 2-dimensional points with which the line is defined.
 * \param q Pointer to the 2-dimensional point whose distance from the line should be calculated.
 * \return The distance of the point from the line.
 */ 
GEOMEXPORT float  DistPtToLine(Point2 *p0, Point2 *p1, Point2 *q );

/**
 * Calculates the distance of a given point from a line in three dimensions.
 * \param p0,p1 Pointers to the two 3-dimensional points with which the line is defined.
 * \param q Pointer to the 3-dimensional point whose distance from the line should be calculated.
 * \return The distance of the point from the line.
 */ 
GEOMEXPORT float  Dist3DPtToLine(Point3* p0, Point3* p1, Point3*  q );


/**
 * \deprecated Do not use this function. Use ComputeBumpVec2D() instead.
 * This is here for compatibility only.
 *
 * Computes the 3 bump basis vectors from the UVW coordinates of the triangle.
 *
 * \param tv Texture coordinates at 3 triangle vertices
 * \param v Coordinates of triangle vertices (usually in camera space).
 * \param[out] bvec The 3 bump basis vectors (normalized) corresponding to the U,V,and W axes.
 */
GEOMEXPORT void ComputeBumpVectors(const Point3 tv[3], const Point3 v[3], Point3 bvec[3]);


/**
 * Computes the 2 Bump basis vectors from the UV, VW, or WU  at a triangle.
 *
 * \param axis Either AXIS_UV, AXIS_VW, or AXIS_WU .
 * \param tv Texture coordinates at 3 triangle vertices.
 * \param v Coordinates of triangle vertices (usually in camera space)
 * \param[out] bvec The 2 normalized bump basis vectors corresponding to the specified axes.
 *
 * \note This is the recommended way to compute bump vectors instead of ComputeBumpVectors() which 
 *			can give erroneous results.
 */
GEOMEXPORT void ComputeBumpVec2D(int axis, const Point3 tv[3], const Point3 v[3], Point3 bvec[2]);

/**
 * Computes the bump basis vector for the U texture channel (called the tangent),
 * and the cross product of the surface normal with the tangent (called the binormal).
 * These along with the surface normal are used as the basis vectors for normal mapping. 
 * \param tv Texture coordinates at 3 triangle vertices.
 * \param v Coordinates of triangle vertices (usually in camera space)
 * \param[out] bvec The 2 normalized bump basis vectors corresponding to the specified axes.
 */
GEOMEXPORT void ComputeTangentAndBinormal(const Point3 tv[3], const Point3 v[3], Point3 bvec[2]);

/**
 * Low precision compression of a vector from 12 bytes to 4 bytes. 
 * Only accurate to 1 part in 512. This is commonly used to compress normals. 
 * The vector has to be <= 1.0 in length. 
 * \param p The decompressed vector.
 * \return The 4-byte long compressed value of p
 * \see DeCompressNormal(), BMM_CHAN_NORMAL
 */
GEOMEXPORT ULONG CompressNormal(Point3 p);

/**
 * Decompresses a vector that was compressed using CompressNormal(). 
 * This function may also be used to decompress a surface normal from the G-Buffer 
 * using the BMM_CHAN_NORMAL channel. The decompressed vector has absolute error
 * \<.001 in each component.
 * \param n A vector compressed using CompressNormal()
 * \return A normalized decompressed normal vector
 * \see CompressNormal()
 */
GEOMEXPORT Point3 DeCompressNormal(ULONG n);

/**
 * Creates an arbitrary axis system given an up vector that conforms to the AutoCAD algorithm.
 * \param zAxis The up vector.
 * \param[out] matrix The new axis system. 
 */
GEOMEXPORT void ArbAxis(const Point3& zAxis, Matrix3& matrix);



