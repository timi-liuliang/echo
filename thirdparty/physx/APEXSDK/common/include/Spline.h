/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef SPLINE_H

#define SPLINE_H


/** @file spline.h
 *  @brief A utility class to manage 3d spline curves.
 *
 *  This is used heavily by the terrain terraforming tools for roads, lakes, and flatten operations.
 *
 *  @author John W. Ratcliff
*/

/** @file spline.cpp
 *  @brief A utility class to manage 3d spline curves.
 *
 *  This is used heavily by the terrain terraforming tools for roads, lakes, and flatten operations.
 *
 *  @author John W. Ratcliff
*/


#include "PsArray.h"
#include "PsUserAllocated.h"
#include "foundation/PxVec3.h"

class SplineNode
{
public:
	float GetY(void) const
	{ 
		return y; 
	};
	float x;              // time/distance x-axis component.
	float y;              // y component.
	float u;
	float p;
};

typedef physx::shdfnd::Array< SplineNode > SplineNodeVector;
typedef physx::shdfnd::Array< physx::PxVec3 > PxVec3Vector;
typedef physx::shdfnd::Array< physx::PxU32 > PxU32Vector;

class Spline : public physx::shdfnd::UserAllocated
{
public:
	void Reserve(physx::PxI32 size)
	{
		mNodes.reserve((physx::PxU32)size);
	};
	void AddNode(float x,float y);
	void ComputeSpline(void);
	float Evaluate(float x,physx::PxU32 &index,physx::PxF32 &fraction) const; // evaluate Y component based on X
	physx::PxI32 GetSize(void)  const
	{ 
		return (physx::PxI32)mNodes.size(); 
	}
	float GetEntry(physx::PxI32 i) const 
	{ 
		return mNodes[(physx::PxU32)i].GetY(); 
	};
	void Clear(void)
	{
		mNodes.clear();
	};
private:
	SplineNodeVector mNodes; // nodes.
};

class SplineCurve : public physx::shdfnd::UserAllocated
{
public:
	void Reserve(physx::PxI32 size)
	{
		mXaxis.Reserve(size);
		mYaxis.Reserve(size);
		mZaxis.Reserve(size);
	};

	void setControlPoints(const PxVec3Vector &points)
	{
		Clear();
		Reserve( (physx::PxI32)points.size() );
		for (physx::PxU32 i=0; i<points.size(); i++)
		{
			AddControlPoint(points[i]);
		}
		ComputeSpline();
	}

	float AddControlPoint(const physx::PxVec3& p); // add control point, time is computed based on distance along the curve.
	void AddControlPoint(const physx::PxVec3& p,float t); // add control point.

	void GetPointOnSpline(float t,physx::PxVec3 &pos)
	{
		float d = t*mTime;
		physx::PxU32 index;
		physx::PxF32 fraction;
		pos = Evaluate(d,index,fraction);
	}

	physx::PxVec3 Evaluate(float dist,physx::PxU32 &index,physx::PxF32 &fraction);

	float GetLength(void) { return mTime; }; //total length of spline

	physx::PxI32 GetSize(void) { return mXaxis.GetSize(); };

	physx::PxVec3 GetEntry(physx::PxI32 i);

	void ComputeSpline(void); // compute spline.

	void Clear(void)
	{
		mXaxis.Clear();
		mYaxis.Clear();
		mZaxis.Clear();
		mTime = 0;
	};

	float Set(const PxVec3Vector &vlist)
	{
		Clear();
		physx::PxI32 count = (physx::PxI32)vlist.size();
		Reserve(count);
		for (physx::PxU32 i=0; i<vlist.size(); i++)
		{
			AddControlPoint( vlist[i] );
		}
		ComputeSpline();
		return mTime;
	};

	void ResampleControlPoints(const PxVec3Vector &inputpoints,
									  PxVec3Vector &outputpoints,
									  PxU32Vector &outputIndex,
 									float dtime)
	{
		float length = Set(inputpoints);
		for (float l=0; l<=length; l+=dtime)
		{
			physx::PxU32 index;
			physx::PxF32 fraction;
			physx::PxVec3 pos = Evaluate(l,index,fraction);
			outputpoints.pushBack(pos);
			outputIndex.pushBack(index);
		}
	};

private:
	float  mTime; // time/distance traveled.
	Spline mXaxis;
	Spline mYaxis;
	Spline mZaxis;
};

#endif
