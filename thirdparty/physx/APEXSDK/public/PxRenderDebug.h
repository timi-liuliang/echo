/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_RENDER_DEBUG_H
#define PX_RENDER_DEBUG_H

/*!
\file
\brief debug rendering classes and structures
*/

#include "foundation/PxVec3.h"
#include "foundation/PxQuat.h"
#include "foundation/PxMat33.h"
#include "foundation/PxMat44.h"
#include "foundation/PxBounds3.h"
#include "PxRenderDebugData.h"

namespace physx
{
	namespace general_renderdebug4
	{

/**
\brief Enums for debug colors
 */
struct DebugColors
{
	enum Enum
	{
		Default = 0,
		PoseArrows,
		MeshStatic,
		MeshDynamic,
		Shape,
		Text0,
		Text1,
		ForceArrowsLow,
		ForceArrowsNorm,
		ForceArrowsHigh,
		Color0,
		Color1,
		Color2,
		Color3,
		Color4,
		Color5,
		Red,
		Green,
		Blue,
		DarkRed,
		DarkGreen,
		DarkBlue,
		LightRed,
		LightGreen,
		LightBlue,
		Purple,
		DarkPurple,
		Yellow,
		Orange,
		Gold,
		Emerald,
		White,
		Black,
		Gray,
		LightGray,
		DarkGray,
		NUM_COLORS
	};
};


PX_PUSH_PACK_DEFAULT


/**
\brief Simple vertex with normal and color
 */
class RenderDebugSolidVertex
{
public:
	PxF32 mPos[3];
	PxF32 mNormal[3];
	PxU32 mColor;
};

/**
\brief simple vertex with color
 */
class RenderDebugVertex
{
public:

	PxF32 mPos[3];
	PxU32 mColor;
};



/**
\brief user provided rendering output methods
 */
class RenderDebugInterface
{
public:
  virtual void debugRenderLines(PxU32 lcount,const RenderDebugVertex *vertices,bool useZ,bool isScreenSpace) = 0;
  virtual void debugRenderTriangles(PxU32 tcount,const RenderDebugSolidVertex *vertices,bool useZ,bool isScreenSpace) = 0;

  virtual void debugRenderIndexedTriangleMesh(PxU32 triangleMeshId,const char *material,
	  PxU32 transformCount,
	  const PxF32 *localToWorld44,
	  const PxF32 *positions,
	  const PxF32 *normals,
	  const PxU32 *colors,
	  const PxF32 *tangents,
	  const PxF32 *binormals,
	  const PxF32 *texcoords0,
	  const PxF32 *texcoords1,
	  const PxF32 *texcoords2,
	  const PxF32 *texcoords3,
	  const PxU32  *indices,
	  PxU32 triCount,
	  PxU32 vertCount) = 0;
protected:
	virtual ~RenderDebugInterface(void)
	{

	}
};

/**
\brief class that draws debug rendering primitives
 */
class RenderDebug
{
public:

	virtual PxU32 getUpdateCount(void) const = 0;

    virtual bool render(PxF32 dtime,RenderDebugInterface *iface) = 0;

	virtual void debugGraph(PxU32 numPoints, PxF32 * points, PxF32 graphMax, PxF32 graphXPos, PxF32 graphYPos, PxF32 graphWidth, PxF32 graphHeight, PxU32 colorSwitchIndex = 0xFFFFFFFF) = 0;

	virtual void  reset(PxI32 blockIndex=-1) = 0; // -1 reset *everything*, 0 = reset everything except stuff inside blocks, > 0 reset a specific block of data.

	virtual void  drawGrid(bool zup=false,PxU32 gridSize=40) = 0; // draw a grid.

	virtual void  pushRenderState(void) = 0;

    virtual void  popRenderState(void) = 0;

	virtual void  setCurrentColor(PxU32 color=0xFFFFFF,PxU32 arrowColor=0xFF0000) = 0;
	virtual PxU32 getCurrentColor(void) const = 0;
	virtual PxU32 getCurrentArrowColor(void) const = 0;

	virtual void  setCurrentUserPointer(void *userPtr) = 0;

	virtual void  setCurrentUserId(PxI32 userId) = 0;

	virtual void * getCurrentUserPointer(void) const = 0;
	virtual PxI32 getCurrentUserId(void) = 0;

	virtual void  setCurrentDisplayTime(PxF32 displayTime=0.0001f) = 0;

	virtual PxF32 getRenderScale(void) = 0;

	virtual void  setRenderScale(PxF32 scale) = 0;

	virtual void  setCurrentState(PxU32 states=0) = 0;

	virtual void  addToCurrentState(DebugRenderState::Enum state) = 0; // OR this state flag into the current state.

	virtual void  removeFromCurrentState(DebugRenderState::Enum state) = 0; // Remove this bit flat from the current state

	virtual void  setCurrentTextScale(PxF32 textScale) = 0;

	virtual void  setCurrentArrowSize(PxF32 arrowSize) = 0;

	virtual PxU32 getCurrentState(void) const = 0;

	virtual void  setRenderState(PxU32 states=0,  // combination of render state flags
	                             PxU32 color=0xFFFFFF, // base color
                                 PxF32 displayTime=0.0001f, // duration of display items.
	                             PxU32 arrowColor=0xFF0000, // secondary color, usually used for arrow head
                                 PxF32 arrowSize=0.1f,
								 PxF32 renderScale=1.0f,
								 PxF32 textScale=1.0f) = 0;      // seconary size, usually used for arrow head size.


	virtual PxU32 getRenderState(PxU32 &color,PxF32 &displayTime,PxU32 &arrowColor,PxF32 &arrowSize,PxF32 &renderScale,PxF32 &textScale) const = 0;


	virtual void  endDrawGroup(void) = 0;

	virtual void  setDrawGroupVisible(PxI32 groupId,bool state) = 0;

	virtual void debugRect2d(PxF32 x1,PxF32 y1,PxF32 x2,PxF32 y2) = 0;

	virtual void  debugPolygon(PxU32 pcount,const PxVec3 *points) = 0;

	virtual void  debugLine(const PxVec3 &p1,const PxVec3 &p2) = 0;

	virtual void  debugGradientLine(const PxVec3 &p1,const PxVec3 &p2,const PxU32 &c1,const PxU32 &c2) = 0;

	virtual void  debugOrientedLine(const PxVec3 &p1,const PxVec3 &p2,const PxMat44 &transform) = 0;

	virtual void  debugRay(const PxVec3 &p1,const PxVec3 &p2) = 0;

	virtual void  debugCylinder(const PxVec3 &p1,const PxVec3 &p2,PxF32 radius) = 0;

	virtual void  debugThickRay(const PxVec3 &p1,const PxVec3 &p2,PxF32 raySize=0.02f) = 0;

	virtual void  debugPlane(const PxVec3 &normal,PxF32 dCoff,PxF32 radius1,PxF32 radius2) = 0;

	virtual void  debugTri(const PxVec3 &p1,const PxVec3 &p2,const PxVec3 &p3) = 0;

	virtual void  debugTriNormals(const PxVec3 &p1,const PxVec3 &p2,const PxVec3 &p3,const PxVec3 &n1,const PxVec3 &n2,const PxVec3 &n3) = 0;

	virtual void  debugGradientTri(const PxVec3 &p1,const PxVec3 &p2,const PxVec3 &p3,const PxU32 &c1,const PxU32 &c2,const PxU32 &c3) = 0;

	virtual void  debugGradientTriNormals(const PxVec3 &p1,const PxVec3 &p2,const PxVec3 &p3,const PxVec3 &n1,const PxVec3 &n2,const PxVec3 &n3,const PxU32 &c1,const PxU32 &c2,const PxU32 &c3) = 0;

	virtual void  debugBound(const PxVec3 &bmin,const PxVec3 &bmax) = 0;

	virtual void  debugBound(const PxBounds3 &b) = 0;

	virtual void  debugOrientedBound(const PxVec3 &sides,const PxMat44 &transform) = 0;

	virtual void  debugOrientedBound(const PxVec3 &bmin,const PxVec3 &bmax,const PxVec3 &pos,const PxQuat &quat) = 0; // the rotation as a quaternion

	virtual void  debugOrientedBound(const PxVec3 &bmin,const PxVec3 &bmax,const PxMat44 &xform) = 0; // the rotation as a quaternion

	virtual void  debugSphere(const PxVec3 &pos,PxF32 radius) = 0;

	virtual void  debugOrientedSphere(PxF32 radius, PxU32 subdivision, const PxMat44 &transform) = 0;

	virtual void  debugOrientedSphere(const PxVec3 &radius, PxU32 subdivision, const PxMat44 &transform) = 0;

	virtual void  debugOrientedCapsule(PxF32 radius,PxF32 height,PxU32 subdivision,const PxMat44 &transform) = 0;

	virtual void  debugOrientedCapsuleTapered(PxF32 radius1, PxF32 radius2, PxF32 height, PxU32 subdivision, const PxMat44& transform) = 0;

	virtual void  debugOrientedCylinder(PxF32 radius,PxF32 height,PxU32 subdivision,bool closeSides,const PxMat44 &transform) = 0;

	virtual void  debugOrientedCircle(PxF32 radius,PxU32 subdivision,const PxMat44 &transform) = 0;

	virtual void  debugPoint(const PxVec3 &pos,PxF32 radius) = 0;
	virtual void  debugPoint(const PxVec3 &pos,const PxVec3 &scale) = 0;
	virtual void  debugQuad(const PxVec3 &pos,const PxVec2 &scale,PxF32 orientation) = 0;

	virtual void  debugAxes(const PxMat44 &transform,PxF32 distance=0.1f,PxF32 brightness=1.0f) = 0;

    virtual void debugArc(const PxVec3 &center,const PxVec3 &p1,const PxVec3 &p2,PxF32 arrowSize=0.1f,bool showRoot=false) = 0;

    virtual void debugThickArc(const PxVec3 &center,const PxVec3 &p1,const PxVec3 &p2,PxF32 thickness=0.02f,bool showRoot=false) = 0;

    virtual void debugText(const PxVec3 &pos,const char *fmt,...) = 0;

    virtual void debugOrientedText(const PxVec3 &pos,const PxQuat &rot,const char *fmt,...) = 0;

    virtual void debugOrientedText(const PxMat44 &xform,const char *fmt,...) = 0;

	virtual void setViewMatrix(const PxF32 *view) = 0;

	virtual void setProjectionMatrix(const PxF32 *projection) = 0;

	virtual const PxF32* getViewProjectionMatrix(void) const = 0;
	virtual const PxF32 *getViewMatrix(void) const = 0;
	virtual const PxF32 *getProjectionMatrix(void) const = 0;

	virtual void  eulerToQuat(const PxVec3 &angles, PxQuat &q) = 0; // angles are in degrees.

	virtual PxI32 beginDrawGroup(const PxMat44 &pose) = 0;

	virtual void  setDrawGroupPose(PxI32 blockId,const PxMat44 &pose) = 0;

	virtual void  debugDetailedSphere(const PxVec3 &pos,PxF32 radius,PxU32 stepCount) = 0;

	virtual void  setPose(const PxMat44 &pose) = 0;

	virtual const PxMat44 & getPose(void) const = 0;

	/* \brief Create an createDebugGraphDesc.  This is the manual way of setting up a graph.  Every parameter can
	and must be customized when using this constructor.
	*/
	virtual DebugGraphDesc* createDebugGraphDesc(void) = 0;

	virtual void releaseDebugGraphDesc(DebugGraphDesc *desc) = 0;

	/**
	\brief Create an createDebugGraphDesc using the minimal amount of work.  This constructor provides for six custom
	graphs to be simultaneously drawn on the display at one time numbered 0 to 5.  The position, color, and size
	of the graphs are automatically set based on the graphNum argument.
	*/
	virtual DebugGraphDesc* createDebugGraphDesc(PxU32 graphNum,PxU32 dataCount,const PxF32 *dataArray, PxF32 maxY, char* xLabel, char* yLabel) = 0;

	virtual void debugGraph(DebugGraphDesc& graphDesc) = 0;

	/**
	\brief Set a debug color value by name.
	*/
	virtual void setDebugColor(DebugColors::Enum colorEnum, PxU32 value) = 0;

	/**
	\brief Return a debug color value by name.
	*/
	virtual PxU32 getDebugColor(DebugColors::Enum colorEnum) const = 0;

	/**
	\brief Return a debug color value by RGB inputs
	*/
	virtual PxU32 getDebugColor(float red, float green, float blue) const = 0;

	virtual void getDebugBounds(PxBounds3 &b) const = 0;

	virtual void renderIndexedTriangleMesh(PxU32 triangleMeshId,const char *material,
		PxU32 transformCount,
		const PxF32 *localToWorld44,
		const PxF32 *positions,
		const PxF32 *normals,
		const PxU32 *colors,
		const PxF32 *tangents,
		const PxF32 *binormals,
		const PxF32 *texcoords0,
		const PxF32 *texcoords1,
		const PxF32 *texcoords2,
		const PxF32 *texcoords3,
		const PxU32  *indices,
		PxU32 triCount,
		PxU32 vertCount) = 0;

	virtual	PxU32 getTriangleMeshId(void) = 0;

	virtual void setFrameTime(PxF32 ftime) = 0;
	virtual PxF32 getFrameTime(void) const = 0;

	virtual void releaseRenderDebug(void) = 0;

protected:

	virtual ~RenderDebug(void) { };
};


PX_POP_PACK

}; // end of namespace 
using namespace general_renderdebug4;
}; // end of namespace

#endif // PX_RENDER_DEBUG_H
