/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "PsShare.h"
#include "NxApexRenderDebug.h"
#include "ApexRenderable.h"

#pragma warning(disable:4996)
#pragma warning(disable:4100)
#pragma warning(disable:4189)

#if NX_SDK_VERSION_MAJOR == 2
#include "NxDebugRenderable.h"
#include "NxFromPx.h"
#elif NX_SDK_VERSION_MAJOR == 3
#include <PxRenderBuffer.h>
#endif // NX_SDK_VERSION_MAJOR

#include "ApexRenderDebug.h"
#include "NxUserRenderer.h"
#include "ApexSDK.h"
#include "PxProcessRenderDebug.h"
#include "PsUserAllocated.h"
#include "foundation/PxIntrinsics.h"
#include "PsString.h"
#include "InternalRenderDebug.h"

namespace physx
{
namespace apex
{

#if defined(WITHOUT_DEBUG_VISUALIZE)

NiApexRenderDebug* createNiApexRenderDebug(ApexSDK* /*a*/, physx::PxProcessRenderDebug* /*debugChannel*/)
{
	return NULL;
}
void releaseNiApexRenderDebug(NiApexRenderDebug* /*n*/) 
{
}

#else

typedef physx::Array< NxApexRenderContext >      RenderContextVector;
typedef physx::Array< NxUserRenderResource*>    RenderResourceVector;

#if NX_SDK_VERSION_MAJOR == 2
PX_INLINE void getDebugLine(NxDebugLine& l, const physx::RenderDebugVertex* vtx)
{
	l.p0.set(vtx[0].mPos);
	l.p1.set(vtx[1].mPos);

	// try averaging the color
	const PxU32 color1 = (vtx[0].mColor >> 1) & 0x7f7f7f7f; // divide by 2
	const PxU32 color2 = (vtx[1].mColor >> 1) & 0x7f7f7f7f; // divide by 2

	l.color = color1 + color2;
}

PX_INLINE void getDebugTriangle(NxDebugTriangle& t, const physx::RenderDebugSolidVertex* vtx)
{
	t.p0.set(vtx[0].mPos);
	t.p1.set(vtx[1].mPos);
	t.p2.set(vtx[2].mPos);

	// drop the normals...

	// this is a bit slowish, oh well, it will finally die with the death of PhysX 2.x
	PxU32 totalColor = 0;
	for (PxU32 i = 0; i < 32; i += 8)
	{
		PxU32 color = 0;
		for (PxU32 j = 0; j < 3; j++)
		{
			color += (vtx[j].mColor >> i) & 0xff;
		}
		color /= 3;
		totalColor |= color << i;
	}

	t.color = totalColor;
}
#endif


class ApexRenderDebug : public NiApexRenderDebug, public ApexRWLockable, public physx::UserAllocated, public physx::RenderDebugInterface
{
public:
	APEX_RW_LOCKABLE_BOILERPLATE

	ApexRenderDebug(ApexSDK* sdk, PxProcessRenderDebug* debugChannel)
	{
		mApexSDK = sdk;
		mProcessRenderDebug = debugChannel;
		mRenderDebug = physx::createInternalRenderDebug(debugChannel);
		mWireFrameMaterial = INVALID_RESOURCE_ID;
		mSolidShadedMaterial = INVALID_RESOURCE_ID;
		mLastRenderSolidCount = 0;
		mLastRenderLineCount = 0;
		mRenderSolidCount = 0;
		mRenderLineCount = 0;
		mUseDebugRenderable = false;
	}

	virtual ~ApexRenderDebug(void)
	{
		if (mRenderDebug)
		{
			mRenderDebug->releaseRenderDebug();
		}
		// APEX specific stuff
		{
			RenderResourceVector::Iterator i;
			for (i = mRenderLineResources.begin(); i != mRenderLineResources.end(); ++i)
			{
				NxUserRenderResource* resource = (*i);
				PX_ASSERT(resource);
				NxUserRenderVertexBuffer* vbuffer = resource->getVertexBuffer(0);
				PX_ASSERT(vbuffer);
				mApexSDK->getUserRenderResourceManager()->releaseResource(*resource);
				mApexSDK->getUserRenderResourceManager()->releaseVertexBuffer(*vbuffer);
			}
		}
		{
			RenderResourceVector::Iterator i;
			for (i = mRenderSolidResources.begin(); i != mRenderSolidResources.end(); ++i)
			{
				NxUserRenderResource* resource = (*i);
				PX_ASSERT(resource);
				NxUserRenderVertexBuffer* vbuffer = resource->getVertexBuffer(0);
				PX_ASSERT(vbuffer);
				mApexSDK->getUserRenderResourceManager()->releaseResource(*resource);
				mApexSDK->getUserRenderResourceManager()->releaseVertexBuffer(*vbuffer);
			}
		}
		mApexSDK->getInternalResourceProvider()->releaseResource(mWireFrameMaterial);
		mApexSDK->getInternalResourceProvider()->releaseResource(mSolidShadedMaterial);
		if (mProcessRenderDebug)
		{
			mProcessRenderDebug->release();
		}
	};

	/*
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	This method is used to enable or disable the use of a legacy NxDebugRenderable.  When enabled,
	use the getDebugRenderable() method to get a legacy NxDebugRenerable object that will contain
	all the debug output.
	*/
	virtual void	setUseDebugRenderable(bool state)
	{
		mUseDebugRenderable = state;
		if (state == false)
		{
#if NX_SDK_VERSION_MAJOR == 2
			mNxDebugTriangles.clear();
			mNxDebugLines.clear();
			mNxDebugTrianglesScreenSpace.clear();
			mNxDebugLinesScreenSpace.clear();
#endif
#if NX_SDK_VERSION_MAJOR == 3
			mPxDebugTriangles.clear();
			mPxDebugLines.clear();
			mPxDebugTrianglesScreenSpace.clear();
			mPxDebugLinesScreenSpace.clear();
#endif
		}
	}


#if NX_SDK_VERSION_MAJOR == 2
	/*
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getDebugRenderable(NxDebugRenderable& renderable)
	{
		NxDebugRenderable* r = &renderable;
		new(r) NxDebugRenderable(0, 0, mNxDebugLines.size(), mNxDebugLines.size() ? &mNxDebugLines[0] : NULL, mNxDebugTriangles.size(), mNxDebugTriangles.size() ? &mNxDebugTriangles[0] : NULL);
	}

	/*
	\brief Method to support rendering to a legacy PhysX SDK NxDebugRenderable object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).  These are the data items which should be rendered in screenspace

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getDebugRenderableScreenSpace(NxDebugRenderable& renderable)
	{
		NxDebugRenderable* r = &renderable;
		new(r) NxDebugRenderable(0, 0, mNxDebugLinesScreenSpace.size(), mNxDebugLinesScreenSpace.size() ? &mNxDebugLinesScreenSpace[0] : NULL, mNxDebugTrianglesScreenSpace.size(), mNxDebugTrianglesScreenSpace.size() ? &mNxDebugTrianglesScreenSpace[0] : NULL);
	}


	/*
	\brief Method to support rendering from an existing legacy PhysX SDK NxDebugRenderable object.

	The contents of the legacy NxDebugRenderable is added to the current contents of the
	NxApexRenderDebug object, and is output through the APEX Render Resources API.
	*/
	virtual void	addDebugRenderable(const NxDebugRenderable& debugRenderable)
	{
		// Points
		pushRenderState();

		const physx::PxU32 color      = mRenderDebug->getCurrentColor();;
		const physx::PxU32 arrowColor = mRenderDebug->getCurrentArrowColor();

		const physx::PxU32         numPoints = debugRenderable.getNbPoints();
		const NxDebugPoint* points    = debugRenderable.getPoints();
		for (physx::PxU32 i = 0; i < numPoints; i++)
		{
			const NxDebugPoint& point = points[i];
			setCurrentColor(point.color, arrowColor);
			debugPoint(PXFROMNXVEC3(point.p), 0.01f);
		}

		// Lines
		const physx::PxU32 numLines = debugRenderable.getNbLines();
		const NxDebugLine* lines    = debugRenderable.getLines();
		for (physx::PxU32 i = 0; i < numLines; i++)
		{
			const NxDebugLine& line = lines[i];
			setCurrentColor(line.color, arrowColor);
			debugLine(PXFROMNXVEC3(line.p0), PXFROMNXVEC3(line.p1));
		}

		// Triangles
		const physx::PxU32            numTriangles = debugRenderable.getNbTriangles();
		const NxDebugTriangle* triangles    = debugRenderable.getTriangles();
		for (physx::PxU32 i = 0; i < numTriangles; i++)
		{
			const NxDebugTriangle& triangle = triangles[i];
			setCurrentColor(triangle.color, arrowColor);
			debugTri(PXFROMNXVEC3(triangle.p0), PXFROMNXVEC3(triangle.p1), PXFROMNXVEC3(triangle.p2));
		}

		setCurrentColor(color, arrowColor);

		popRenderState();
	}
#elif NX_SDK_VERSION_MAJOR == 3

	/*
	\brief Method to support rendering to a legacy PhysX SDK PxRenderBuffer object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getRenderBuffer(PhysXRenderBuffer& renderable)
	{
		renderable.mNbPoints = 0;
		renderable.mPoints = NULL;
		renderable.mNbLines = mPxDebugLines.size();
		renderable.mLines = renderable.mNbLines ? &mPxDebugLines[0] : NULL;
		renderable.mNbTriangles = mPxDebugTriangles.size();
		renderable.mTriangles = renderable.mNbTriangles ? &mPxDebugTriangles[0] : NULL;
		renderable.mNbTexts = 0;
		renderable.mTexts = NULL;
	}

	/*
	\brief Method to support rendering to a legacy PhysX SDK PxRenderBuffer object instead
	of to the APEX Render Resources API (i.e.: NxApexRenderable).

	When enabled with a call to setUseDebugRenderable(true), this method will return a legacy
	NxDebugRenderable object that contains all of the output of the NxApexRenderDebug class.
	*/
	virtual void	getRenderBufferScreenSpace(PhysXRenderBuffer& renderable)
	{
		renderable.mNbPoints = 0;
		renderable.mPoints = NULL;
		renderable.mNbLines = mPxDebugLinesScreenSpace.size();
		renderable.mLines = renderable.mNbLines ? &mPxDebugLinesScreenSpace[0] : NULL;
		renderable.mNbTriangles = mPxDebugTrianglesScreenSpace.size();
		renderable.mTriangles = renderable.mNbTriangles ? &mPxDebugTrianglesScreenSpace[0] : NULL;
		renderable.mNbTexts = 0;
		renderable.mTexts = NULL;
	}


	virtual void	addDebugRenderable(const physx::PxRenderBuffer& renderBuffer)
	{
		// Points
		pushRenderState();

		const physx::PxU32 color          = mRenderDebug->getCurrentColor();;
		const physx::PxU32 arrowColor     = mRenderDebug->getCurrentArrowColor();

		const physx::PxU32 numPoints      = renderBuffer.getNbPoints();
		const physx::PxDebugPoint* points = renderBuffer.getPoints();
		for (physx::PxU32 i = 0; i < numPoints; i++)
		{
			const physx::PxDebugPoint& point = points[i];
			setCurrentColor(point.color, arrowColor);
			debugPoint(point.pos, 0.01f);
		}

		// Lines
		const physx::PxU32 numLines     = renderBuffer.getNbLines();
		const physx::PxDebugLine* lines = renderBuffer.getLines();
		for (physx::PxU32 i = 0; i < numLines; i++)
		{
			const physx::PxDebugLine& line = lines[i];
			debugGradientLine(line.pos0, line.pos1, line.color0, line.color1);
		}

		// Triangles
		const physx::PxU32 numTriangles         = renderBuffer.getNbTriangles();
		const physx::PxDebugTriangle* triangles = renderBuffer.getTriangles();
		for (physx::PxU32 i = 0; i < numTriangles; i++)
		{
			const physx::PxDebugTriangle& triangle = triangles[i];
			debugGradientTri(triangle.pos0, triangle.pos1, triangle.pos2, triangle.color0, triangle.color1, triangle.color2);
		}

		// Texts
		const physx::PxU32 numTexts				= renderBuffer.getNbTexts();
		const physx::PxDebugText* texts			= renderBuffer.getTexts();
		for (physx::PxU32 i = 0; i < numTexts; i++)
		{
			const physx::PxDebugText& text = texts[i];
			debugText(text.position, text.string);
		}

		setCurrentColor(color, arrowColor);

		popRenderState();
	}
#endif // NX_SDK_VERSION_MAJOR == 2

	virtual physx::PxU32 getUpdateCount(void) const
	{
		return mRenderDebug->getUpdateCount();
	}

	virtual bool render(physx::PxF32 dtime, physx::RenderDebugInterface* iface)
	{
		return mRenderDebug->render(dtime, iface);
	}

	virtual void debugGraph(physx::PxU32 numPoints, physx::PxF32* points, physx::PxF32 graphMax, physx::PxF32 graphXPos, physx::PxF32 graphYPos, physx::PxF32 graphWidth, physx::PxF32 graphHeight, physx::PxU32 colorSwitchIndex)
	{
		mRenderDebug->debugGraph(numPoints, points, graphMax, graphXPos, graphYPos, graphWidth, graphHeight, colorSwitchIndex);
	}

	virtual void  reset(physx::PxI32 blockIndex) // -1 reset *everything*, 0 = reset everything except stuff inside blocks, > 0 reset a specific block of data.
	{
		mRenderDebug->reset(blockIndex);
	}

	virtual void  drawGrid(bool zup, physx::PxU32 gridSize) // draw a grid.
	{
		mRenderDebug->drawGrid(zup, gridSize);
	}

	virtual void  pushRenderState(void)
	{
		mRenderDebug->pushRenderState();
	}

	virtual void  popRenderState(void)
	{
		mRenderDebug->popRenderState();
	}


	virtual void  setCurrentColor(physx::PxU32 color, physx::PxU32 arrowColor)
	{
		mRenderDebug->setCurrentColor(color, arrowColor);
	}

	virtual void  setCurrentUserPointer(void* userPtr)
	{
		mRenderDebug->setCurrentUserPointer(userPtr);
	}


	virtual void  setCurrentUserId(physx::PxI32 userId)
	{
		mRenderDebug->setCurrentUserId(userId);
	}


	virtual void* getCurrentUserPointer(void) const
	{
		return mRenderDebug->getCurrentUserPointer();
	}

	virtual physx::PxI32 getCurrentUserId(void)
	{
		return mRenderDebug->getCurrentUserId();
	}


	virtual void  setCurrentDisplayTime(physx::PxF32 displayTime)
	{
		mRenderDebug->setCurrentDisplayTime(displayTime);
	}


	virtual physx::PxF32 getRenderScale(void)
	{
		return mRenderDebug->getRenderScale();
	}


	virtual void  setRenderScale(physx::PxF32 scale)
	{
		mRenderDebug->setRenderScale(scale);
	}


	virtual void  setCurrentState(physx::PxU32 state)
	{
		mRenderDebug->setCurrentState(state);
	}


	virtual void  addToCurrentState(physx::DebugRenderState::Enum state) // OR this state flag into the current state.
	{
		mRenderDebug->addToCurrentState(state);
	}


	virtual void  removeFromCurrentState(physx::DebugRenderState::Enum state)  // Remove this bit flat from the current state
	{
		mRenderDebug->removeFromCurrentState(state);
	}


	virtual void  setCurrentTextScale(physx::PxF32 textScale)
	{
		mRenderDebug->setCurrentTextScale(textScale);
	}


	virtual void  setCurrentArrowSize(physx::PxF32 arrowSize)
	{
		mRenderDebug->setCurrentArrowSize(arrowSize);
	}


	virtual physx::PxU32 getCurrentState(void) const
	{
		return mRenderDebug->getCurrentState();
	}

	virtual void  setRenderState(physx::PxU32 states,  // combination of render state flags
	                             physx::PxU32 color, // base color
	                             physx::PxF32 displayTime, // duration of display items.
	                             physx::PxU32 arrowColor, // secondary color, usually used for arrow head
	                             physx::PxF32 arrowSize,
	                             physx::PxF32 renderScale,
	                             physx::PxF32 textScale)      // seconary size, usually used for arrow head size.
	{
		mRenderDebug->setRenderState(states, color, displayTime, arrowColor, arrowSize, renderScale, textScale);
	}


	virtual physx::PxU32 getRenderState(physx::PxU32& color, physx::PxF32& displayTime, physx::PxU32& arrowColor, physx::PxF32& arrowSize, physx::PxF32& renderScale, physx::PxF32& textScale) const
	{
		return mRenderDebug->getRenderState(color, displayTime, arrowColor, arrowSize, renderScale, textScale);
	}


	virtual void  endDrawGroup(void)
	{
		mRenderDebug->endDrawGroup();
	}


	virtual void  setDrawGroupVisible(physx::PxI32 groupId, bool state)
	{
		mRenderDebug->setDrawGroupVisible(groupId, state);
	}


	virtual void debugRect2d(physx::PxF32 x1, physx::PxF32 y1, physx::PxF32 x2, physx::PxF32 y2)
	{
		mRenderDebug->debugRect2d(x1, y1, x2, y2);
	}


	virtual void  debugPolygon(physx::PxU32 pcount, const physx::PxVec3* points)
	{
		mRenderDebug->debugPolygon(pcount, points);
	}

	virtual void  debugLine(const physx::PxVec3& p1, const physx::PxVec3& p2)
	{
		mRenderDebug->debugLine(p1, p2);
	}


	virtual void  debugGradientLine(const physx::PxVec3& p1, const physx::PxVec3& p2, const physx::PxU32& c1, const physx::PxU32& c2)
	{
		mRenderDebug->debugGradientLine(p1, p2, c1, c2);
	}

	virtual void  debugOrientedLine(const physx::PxVec3& p1, const physx::PxVec3& p2, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedLine(p1, p2, transform);
	}

	virtual void  debugRay(const physx::PxVec3& p1, const physx::PxVec3& p2)
	{
		mRenderDebug->debugRay(p1, p2);
	}


	virtual void  debugCylinder(const physx::PxVec3& p1, const physx::PxVec3& p2, physx::PxF32 radius)
	{
		mRenderDebug->debugCylinder(p1, p2, radius);
	}

	virtual void  debugThickRay(const physx::PxVec3& p1, const physx::PxVec3& p2, physx::PxF32 raySize)
	{
		mRenderDebug->debugThickRay(p1, p2, raySize);
	}

	virtual void  debugPlane(const physx::PxVec3& normal, physx::PxF32 dCoff, physx::PxF32 radius1, physx::PxF32 radius2)
	{
		mRenderDebug->debugPlane(normal, dCoff, radius1, radius2);
	}


	virtual void  debugTri(const physx::PxVec3& p1, const physx::PxVec3& p2, const physx::PxVec3& p3)
	{
		mRenderDebug->debugTri(p1, p2, p3);
	}

	virtual void  debugTriNormals(const physx::PxVec3& p1, const physx::PxVec3& p2, const physx::PxVec3& p3, const physx::PxVec3& n1, const physx::PxVec3& n2, const physx::PxVec3& n3)
	{
		mRenderDebug->debugTriNormals(p1, p2, p3, n1, n2, n3);
	}

	virtual void  debugGradientTri(const physx::PxVec3& p1, const physx::PxVec3& p2, const physx::PxVec3& p3, const physx::PxU32& c1, const physx::PxU32& c2, const physx::PxU32& c3)
	{
		mRenderDebug->debugGradientTri(p1, p2, p3, c1, c2, c3);
	}


	virtual void  debugGradientTriNormals(const physx::PxVec3& p1, const physx::PxVec3& p2, const physx::PxVec3& p3, const physx::PxVec3& n1, const physx::PxVec3& n2, const physx::PxVec3& n3, const physx::PxU32& c1, const physx::PxU32& c2, const physx::PxU32& c3)
	{
		mRenderDebug->debugGradientTriNormals(p1, p2, p3, n1, n2, n3, c1, c2, c3);
	}

	virtual void  debugBound(const physx::PxVec3& bmin, const physx::PxVec3& bmax)
	{
		mRenderDebug->debugBound(bmin, bmax);
	}

	virtual void  debugBound(const physx::PxBounds3& b)
	{
		mRenderDebug->debugBound(b);
	}

	virtual void  debugOrientedBound(const physx::PxVec3& sides, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedBound(sides, transform);
	}

	virtual void  debugOrientedBound(const physx::PxVec3& bmin, const physx::PxVec3& bmax, const physx::PxVec3& pos, const physx::PxQuat& quat) // the rotation as a quaternion
	{
		mRenderDebug->debugOrientedBound(bmin, bmax, pos, quat);
	}

	virtual void  debugOrientedBound(const physx::PxVec3& bmin, const physx::PxVec3& bmax, const PxMat44& xform) // the rotation as a quaternion
	{
		mRenderDebug->debugOrientedBound(bmin, bmax, xform);
	}

	virtual void  debugSphere(const physx::PxVec3& pos, physx::PxF32 radius)
	{
		mRenderDebug->debugSphere(pos, radius);
	}

	virtual void  debugOrientedSphere(physx::PxF32 radius, PxU32 subdivision, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedSphere(radius, subdivision, transform);
	}

	virtual void  debugOrientedSphere(const physx::PxVec3 &radius, PxU32 subdivision, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedSphere(radius, subdivision, transform);
	}


	virtual void  debugOrientedCapsule(physx::PxF32 radius, physx::PxF32 height, physx::PxU32 subdivision, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedCapsule(radius, height, subdivision, transform);
	}

	virtual void debugOrientedCapsuleTapered(PxF32 radius1, PxF32 radius2, PxF32 height, PxU32 subdivision, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedCapsuleTapered(radius1, radius2, height, subdivision, transform);
	}

	virtual void  debugOrientedCylinder(physx::PxF32 radius, physx::PxF32 height, physx::PxU32 subdivision, bool closeSides, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedCylinder(radius, height, subdivision, closeSides, transform);
	}

	virtual void  debugOrientedCircle(physx::PxF32 radius, physx::PxU32 subdivision, const PxMat44& transform)
	{
		mRenderDebug->debugOrientedCircle(radius, subdivision, transform);
	}

	virtual void  debugQuad(const physx::PxVec3& pos,const physx::PxVec2 &scale,PxF32 rotation)
	{
		mRenderDebug->debugQuad(pos,scale,rotation);
	}


	virtual void  debugPoint(const physx::PxVec3& pos, physx::PxF32 radius)
	{
		mRenderDebug->debugPoint(pos, radius);
	}

	virtual void  debugPoint(const physx::PxVec3& pos,const physx::PxVec3 &radius)
	{
		mRenderDebug->debugPoint(pos, radius);
	}


	virtual void  debugAxes(const PxMat44& transform, physx::PxF32 distance, physx::PxF32 brightness)
	{
		mRenderDebug->debugAxes(transform, distance, brightness);
	}

	virtual void debugArc(const physx::PxVec3& center, const physx::PxVec3& p1, const physx::PxVec3& p2, physx::PxF32 arrowSize, bool showRoot)
	{
		mRenderDebug->debugArc(center, p1, p2, arrowSize, showRoot);
	}

	virtual void debugThickArc(const physx::PxVec3& center, const physx::PxVec3& p1, const physx::PxVec3& p2, physx::PxF32 thickness, bool showRoot)
	{
		mRenderDebug->debugThickArc(center, p1, p2, thickness, showRoot);
	}

	virtual void debugText(const physx::PxVec3& pos, const char* fmt, ...)
	{
		char wbuff[512];
		wbuff[511] = 0;
		va_list arg;
		va_start(arg, fmt);
		physx::string::_vsnprintf(wbuff, sizeof(wbuff) - 1, fmt, arg);
		va_end(arg);
		mRenderDebug->debugText(pos, "%s", wbuff);
	}

	virtual void debugOrientedText(const physx::PxVec3& pos, const physx::PxQuat& rot, const char* fmt, ...)
	{
		char wbuff[512];
		wbuff[511] = 0;
		va_list arg;
		va_start(arg, fmt);
		physx::string::_vsnprintf(wbuff, sizeof(wbuff) - 1, fmt, arg);
		va_end(arg);
		mRenderDebug->debugOrientedText(pos, rot, "%s", wbuff);
	}

	virtual void debugOrientedText(const PxMat44& xform, const char* fmt, ...)
	{
		char wbuff[512];
		wbuff[511] = 0;
		va_list arg;
		va_start(arg, fmt);
		physx::string::_vsnprintf(wbuff, sizeof(wbuff) - 1, fmt, arg);
		va_end(arg);
		mRenderDebug->debugOrientedText(xform, "%s", wbuff);
	}

	virtual void setViewMatrix(const physx::PxF32* view)
	{
		mRenderDebug->setViewMatrix(view);
	}

	virtual void setProjectionMatrix(const physx::PxF32* projection)
	{
		mRenderDebug->setProjectionMatrix(projection);
	}

	virtual const physx::PxF32* getViewProjectionMatrix(void) const
	{
		return mRenderDebug->getViewProjectionMatrix();
	}

	virtual const physx::PxF32* getViewMatrix(void) const
	{
		return mRenderDebug->getViewMatrix();
	}

	virtual const physx::PxF32* getProjectionMatrix(void) const
	{
		return mRenderDebug->getProjectionMatrix();
	}

	virtual void  eulerToQuat(const physx::PxVec3& angles, physx::PxQuat& q)  // angles are in degrees.
	{
		mRenderDebug->eulerToQuat(angles, q);
	}


	virtual physx::PxI32 beginDrawGroup(const PxMat44& pose)
	{
		return mRenderDebug->beginDrawGroup(pose);
	}


	virtual void  setDrawGroupPose(physx::PxI32 blockId, const PxMat44& pose)
	{
		mRenderDebug->setDrawGroupPose(blockId, pose);
	}

	virtual void  debugDetailedSphere(const physx::PxVec3& pos, physx::PxF32 radius, physx::PxU32 stepCount)
	{
		mRenderDebug->debugDetailedSphere(pos, radius, stepCount);
	}

	virtual void  setPose(const PxMat44& pose)
	{
		mRenderDebug->setPose(pose);
	}

	const PxMat44& getPose(void) const
	{
		return mRenderDebug->getPose();
	}

	/* \brief Create an createDebugGraphDesc.  This is the manual way of setting up a graph.  Every parameter can
	and must be customized when using this constructor.
	*/
	virtual physx::DebugGraphDesc* createDebugGraphDesc(void)
	{
		return mRenderDebug->createDebugGraphDesc();
	}

	/**
	\brief Create an createDebugGraphDesc using the minimal amount of work.  This constructor provides for six custom
	graphs to be simultaneously drawn on the display at one time numbered 0 to 5.  The position, color, and size
	of the graphs are automatically set based on the graphNum argument.
	*/
	virtual physx::DebugGraphDesc* createDebugGraphDesc(physx::PxU32 graphNum, physx::PxU32 dataCount, const physx::PxF32* dataArray, physx::PxF32 maxY, char* xLabel, char* yLabel)
	{
		return mRenderDebug->createDebugGraphDesc(graphNum, dataCount, dataArray, maxY, xLabel, yLabel);
	}

	virtual void releaseDebugGraphDesc(physx::DebugGraphDesc* desc)
	{
		mRenderDebug->releaseDebugGraphDesc(desc);
	}

	virtual void debugGraph(physx::DebugGraphDesc& graphDesc)
	{
		mRenderDebug->debugGraph(graphDesc);
	}

	/**
	\brief Set a debug color value by name.
	*/
	virtual void setDebugColor(physx::DebugColors::Enum colorEnum, physx::PxU32 value)
	{
		mRenderDebug->setDebugColor(colorEnum, value);
	}

	/**
	\brief Return a debug color value by name.
	*/
	virtual physx::PxU32 getDebugColor(physx::DebugColors::Enum colorEnum) const
	{
		return mRenderDebug->getDebugColor(colorEnum);
	}

	/**
	\brief Return a debug color value by RGB inputs
	*/
	virtual physx::PxU32 getDebugColor(float red, float green, float blue) const
	{
		return mRenderDebug->getDebugColor(red, green, blue);
	}


	/**
	\brief Release an object instance.

	Calling this will unhook the class and delete it from memory.
	You should not keep any reference to this class instance after calling release
	*/
	virtual void release()
	{
		delete this;
	}

	virtual void lockRenderResources()
	{

	}

	/**
	\brief Unlocks the renderable data of this NxApexRenderable actor.

	See locking semantics for NxApexRenderDataProvider::lockRenderResources().
	*/
	virtual void unlockRenderResources()
	{

	}

	/**
	\brief Update the renderable data of this NxApexRenderable actor.

	When called, this method will use the NxUserRenderResourceManager interface to inform the user
	about its render resource needs.  It will also call the writeBuffer() methods of various graphics
	buffers.  It must be called by the user each frame before any calls to dispatchRenderResources().
	If the actor is not being rendered, this function may also be skipped.
	*/
	virtual void updateRenderResources(bool /*rewriteBuffers*/ = false, void* /*userRenderData*/ = 0)
	{
		URR_SCOPE;

		mRenderSolidContexts.clear();
		mRenderLineContexts.clear();

		// Free up the line draw vertex buffer resources if the debug renderer is now using a lot less memory than the last frame.
		if (mRenderLineCount < (mLastRenderLineCount / 2))
		{
			RenderResourceVector::Iterator i;
			for (i = mRenderLineResources.begin(); i != mRenderLineResources.end(); ++i)
			{
				NxUserRenderResource* resource = (*i);
				PX_ASSERT(resource);
				NxUserRenderVertexBuffer* vbuffer = resource->getVertexBuffer(0);
				PX_ASSERT(vbuffer);
				mApexSDK->getUserRenderResourceManager()->releaseResource(*resource);
				mApexSDK->getUserRenderResourceManager()->releaseVertexBuffer(*vbuffer);
			}
			mRenderLineResources.clear();
		}
		// free up the solid shaded triangle vertex buffers if the debug renderer is now using a lot less memory than the last frame.
		if (mRenderSolidCount < mLastRenderSolidCount / 2) // if we have less than 1/2 the number of solid shaded triangles we did last frame, free up the resources.
		{
			RenderResourceVector::Iterator i;
			for (i = mRenderSolidResources.begin(); i != mRenderSolidResources.end(); ++i)
			{
				NxUserRenderResource* resource = (*i);
				PX_ASSERT(resource);
				NxUserRenderVertexBuffer* vbuffer = resource->getVertexBuffer(0);
				PX_ASSERT(vbuffer);
				mApexSDK->getUserRenderResourceManager()->releaseResource(*resource);
				mApexSDK->getUserRenderResourceManager()->releaseVertexBuffer(*vbuffer);
			}
			mRenderSolidResources.clear();
		}

		mLastRenderSolidCount = mRenderSolidCount;
		mLastRenderLineCount  = mRenderLineCount;
		mRenderSolidCount = 0;
		mRenderLineCount = 0;

#if NX_SDK_VERSION_MAJOR == 2
		mNxDebugLines.clear();
		mNxDebugTriangles.clear();
		mNxDebugLinesScreenSpace.clear();
		mNxDebugTrianglesScreenSpace.clear();
#endif
#if NX_SDK_VERSION_MAJOR == 3
		mPxDebugLines.clear();
		mPxDebugTriangles.clear();
		mPxDebugLinesScreenSpace.clear();
		mPxDebugTrianglesScreenSpace.clear();
#endif
		render(mRenderDebug->getFrameTime(), this);
	}

	virtual void dispatchRenderResources(NxUserRenderer& renderer)
	{
		{
			physx::PxU32 index = 0;
			RenderContextVector::Iterator i;
			for (i = mRenderLineContexts.begin(); i != mRenderLineContexts.end(); ++i)
			{
				NxApexRenderContext& c = (*i);
				const PxMat44& pose = mRenderDebug->getPose();
				c.local2world = pose;
				c.world2local = pose.inverseRT();
				c.renderResource = mRenderLineResources[index];
				renderer.renderResource(c);
				index++;
			}
		}
		{
			physx::PxU32 index = 0;
			RenderContextVector::Iterator i;
			for (i = mRenderSolidContexts.begin(); i != mRenderSolidContexts.end(); ++i)
			{
				NxApexRenderContext& c = (*i);
				const PxMat44& pose = mRenderDebug->getPose();
				c.local2world = pose;
				c.world2local = pose.inverseRT();
				c.renderResource = mRenderSolidResources[index];
				renderer.renderResource(c);
				index++;
			}
		}
	}

	/**
	Returns AABB covering rendered data.  The actor's world bounds is updated each frame
	during NxApexScene::fetchResults().  This function does not require the NxApexRenderable actor to be locked.
	*/
	virtual physx::PxBounds3 getBounds() const
	{
		physx::PxBounds3 b;
		mRenderDebug->getDebugBounds(b);
		return b;
	}

	virtual void getDebugBounds(physx::PxBounds3& b) const
	{
		return mRenderDebug->getDebugBounds(b);
	}

	virtual physx::PxF32 getFrameTime(void) const
	{
		return mRenderDebug->getFrameTime();
	}

	virtual void setFrameTime(physx::PxF32 ftime)
	{
		mRenderDebug->setFrameTime(ftime);
	}

	virtual void debugRenderLines(physx::PxU32 lcount, const physx::RenderDebugVertex* vertices, bool /*useZ*/, bool isScreenSpace)
	{
#if NX_SDK_VERSION_MAJOR == 2
		if (mUseDebugRenderable)
		{
			for (physx::PxU32 i = 0; i < lcount; i++)
			{
				NxDebugLine l;
				getDebugLine(l, vertices);
				if ( isScreenSpace )
				{
					mNxDebugLinesScreenSpace.pushBack(l);
				}
				else
				{
					mNxDebugLines.pushBack(l);
				}
				vertices += 2;
			}
		}
		else
#endif
#if NX_SDK_VERSION_MAJOR == 3
			if (mUseDebugRenderable)
			{
				for (physx::PxU32 i = 0; i < lcount; i++)
				{
					PxVec3 v1( vertices[0].mPos[0], vertices[0].mPos[1], vertices[0].mPos[2] );
					PxVec3 v2( vertices[1].mPos[0], vertices[1].mPos[1], vertices[1].mPos[2] );
					PxDebugLine l(v1,v2,vertices->mColor);
					l.color1 = vertices[1].mColor;
					if ( isScreenSpace )
					{
						mPxDebugLinesScreenSpace.pushBack(l);
					}
					else
					{
						mPxDebugLines.pushBack(l);
					}
					vertices += 2;
				}
			}
			else
#endif
		{
			mRenderLineCount += (lcount * 2);
			if (mWireFrameMaterial == INVALID_RESOURCE_ID)
			{
				const char* mname = mApexSDK->getWireframeMaterial();
				NxResID name_space = mApexSDK->getInternalResourceProvider()->createNameSpace(APEX_MATERIALS_NAME_SPACE);
				mWireFrameMaterial = mApexSDK->getInternalResourceProvider()->createResource(name_space, mname, true);
			}

			PX_ASSERT((lcount * 2) <= MAX_LINE_VERTEX);

			physx::PxU32 rcount = (physx::PxU32)mRenderLineContexts.size();
			NxApexRenderContext context;

			NxUserRenderResource* resource;

			if (rcount < mRenderLineResources.size())
			{
				resource = mRenderLineResources[rcount];
			}
			else
			{
				NxUserRenderResourceDesc resourceDesc;
				NxUserRenderVertexBufferDesc vbdesc;
				vbdesc.hint = NxRenderBufferHint::DYNAMIC;
				vbdesc.buffersRequest[NxRenderVertexSemantic::POSITION] = NxRenderDataFormat::FLOAT3;
				vbdesc.buffersRequest[NxRenderVertexSemantic::COLOR]    = NxRenderDataFormat::B8G8R8A8;
				vbdesc.maxVerts = MAX_LINE_VERTEX;
				resourceDesc.cullMode = NxRenderCullMode::NONE;

				for (PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
				{
					PX_ASSERT(vbdesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, vbdesc.buffersRequest[i]));
				}

				NxUserRenderVertexBuffer* vb = mApexSDK->getUserRenderResourceManager()->createVertexBuffer(vbdesc);
				NxUserRenderVertexBuffer* vertexBuffers[1] = { vb };
				resourceDesc.vertexBuffers = vertexBuffers;
				resourceDesc.numVertexBuffers = 1;

				resourceDesc.primitives = NxRenderPrimitiveType::LINES;

				resource = mApexSDK->getUserRenderResourceManager()->createResource(resourceDesc);
				resource->setMaterial(mApexSDK->getInternalResourceProvider()->getResource(mWireFrameMaterial));
				mRenderLineResources.pushBack(resource);
			}

			NxUserRenderVertexBuffer* vb = resource->getVertexBuffer(0);

			resource->setVertexBufferRange(0, lcount * 2);

			NxApexRenderVertexBufferData writeData;
			writeData.setSemanticData(NxRenderVertexSemantic::POSITION, vertices[0].mPos,   sizeof(physx::RenderDebugVertex), NxRenderDataFormat::FLOAT3);
			writeData.setSemanticData(NxRenderVertexSemantic::COLOR,   &vertices[0].mColor, sizeof(physx::RenderDebugVertex), NxRenderDataFormat::B8G8R8A8);
			vb->writeBuffer(writeData, 0, lcount * 2);

			context.isScreenSpace = isScreenSpace;
			context.local2world = physx::PxMat44::createIdentity();
			context.renderResource = 0;
			context.world2local = physx::PxMat44::createIdentity();

			mRenderLineContexts.pushBack(context);
		}

	}

	virtual void debugRenderTriangles(physx::PxU32 tcount, const physx::RenderDebugSolidVertex* vertices, bool /*useZ*/, bool isScreenSpace)
	{
#if NX_SDK_VERSION_MAJOR == 2
		if (mUseDebugRenderable)
		{
			for (physx::PxU32 i = 0; i < tcount; i++)
			{
				NxDebugTriangle t;
				getDebugTriangle(t, vertices);
				if ( isScreenSpace )
				{
					mNxDebugTrianglesScreenSpace.pushBack(t);
				}
				else
				{
					mNxDebugTriangles.pushBack(t);
				}
				vertices += 3;
			}
		}
		else
#endif
#if NX_SDK_VERSION_MAJOR == 3
			if (mUseDebugRenderable)
			{
				for (physx::PxU32 i = 0; i < tcount; i++)
				{
					PxVec3 v1( vertices[0].mPos[0], vertices[0].mPos[1], vertices[0].mPos[2] );
					PxVec3 v2( vertices[1].mPos[0], vertices[1].mPos[1], vertices[1].mPos[2] );
					PxVec3 v3( vertices[2].mPos[0], vertices[2].mPos[1], vertices[2].mPos[2] );
					PxDebugTriangle t( v1,v2, v3, vertices->mColor );
					t.color1 = vertices[1].mColor;
					t.color2 = vertices[2].mColor;
					if ( isScreenSpace )
					{
						mPxDebugTrianglesScreenSpace.pushBack(t);
					}
					else
					{
						mPxDebugTriangles.pushBack(t);
					}
					vertices += 3;
				}
			}
			else
#endif
		{

			mRenderSolidCount += (tcount * 3);

			if (mSolidShadedMaterial == INVALID_RESOURCE_ID)
			{
				const char* mname = mApexSDK->getSolidShadedMaterial();
				NxResID name_space = mApexSDK->getInternalResourceProvider()->createNameSpace(APEX_MATERIALS_NAME_SPACE);
				mSolidShadedMaterial = mApexSDK->getInternalResourceProvider()->createResource(name_space, mname, true);
			}

			PX_ASSERT((tcount * 3) <= MAX_SOLID_VERTEX);

			physx::PxU32 rcount = (physx::PxU32)mRenderSolidContexts.size();
			NxApexRenderContext context;

			NxUserRenderResource* resource;

			if (rcount < mRenderSolidResources.size())
			{
				resource = mRenderSolidResources[rcount];
			}
			else
			{
				NxUserRenderResourceDesc renderResourceDesc;
				NxUserRenderVertexBufferDesc vbdesc;
				vbdesc.hint = NxRenderBufferHint::DYNAMIC;
				vbdesc.buffersRequest[NxRenderVertexSemantic::POSITION] = NxRenderDataFormat::FLOAT3;
				vbdesc.buffersRequest[NxRenderVertexSemantic::NORMAL] = NxRenderDataFormat::FLOAT3;
				vbdesc.buffersRequest[NxRenderVertexSemantic::COLOR]    = NxRenderDataFormat::B8G8R8A8;
				vbdesc.maxVerts = MAX_SOLID_VERTEX;
				renderResourceDesc.cullMode = NxRenderCullMode::COUNTER_CLOCKWISE;

				for (PxU32 i = 0; i < NxRenderVertexSemantic::NUM_SEMANTICS; i++)
				{
					PX_ASSERT(vbdesc.buffersRequest[i] == NxRenderDataFormat::UNSPECIFIED || vertexSemanticFormatValid((NxRenderVertexSemantic::Enum)i, vbdesc.buffersRequest[i]));
				}

				NxUserRenderVertexBuffer* vb = mApexSDK->getUserRenderResourceManager()->createVertexBuffer(vbdesc);
				NxUserRenderVertexBuffer* vertexBuffers[1] = { vb };
				renderResourceDesc.vertexBuffers = vertexBuffers;
				renderResourceDesc.numVertexBuffers = 1;

				renderResourceDesc.primitives = NxRenderPrimitiveType::TRIANGLES;
				resource = mApexSDK->getUserRenderResourceManager()->createResource(renderResourceDesc);
				resource->setMaterial(mApexSDK->getInternalResourceProvider()->getResource(mSolidShadedMaterial));
				mRenderSolidResources.pushBack(resource);
			}

			NxUserRenderVertexBuffer* vb = resource->getVertexBuffer(0);

			resource->setVertexBufferRange(0, tcount * 3);

			NxApexRenderVertexBufferData writeData;
			writeData.setSemanticData(NxRenderVertexSemantic::POSITION, vertices[0].mPos,    sizeof(physx::RenderDebugSolidVertex), NxRenderDataFormat::FLOAT3);
			writeData.setSemanticData(NxRenderVertexSemantic::NORMAL,   vertices[0].mNormal, sizeof(physx::RenderDebugSolidVertex), NxRenderDataFormat::FLOAT3);
			writeData.setSemanticData(NxRenderVertexSemantic::COLOR,   &vertices[0].mColor,  sizeof(physx::RenderDebugSolidVertex), NxRenderDataFormat::B8G8R8A8);
			vb->writeBuffer(writeData, 0, tcount * 3);

			context.isScreenSpace = isScreenSpace;
			context.local2world = physx::PxMat44::createIdentity();
			context.renderResource = 0;
			context.world2local = physx::PxMat44::createIdentity();

			mRenderSolidContexts.pushBack(context);
		}
	}

	physx::PxU32 getCurrentColor(void) const
	{
		return mRenderDebug->getCurrentColor();
	}

	physx::PxU32 getCurrentArrowColor(void) const
	{
		return mRenderDebug->getCurrentArrowColor();
	}

	virtual physx::PxProcessRenderDebug* getProcessRenderDebug(void)
	{
		return mProcessRenderDebug;
	}

	virtual void debugRenderIndexedTriangleMesh(
			physx::PxU32 /*triangleMeshId*/, const char* /*material*/,
	        physx::PxU32 /*transformCount*/,
	        const physx::PxF32* /*localToWorld44*/,
	        const physx::PxF32* /*positions*/,
	        const physx::PxF32* /*normals*/,
	        const physx::PxU32* /*colors*/,
	        const physx::PxF32* /*tangents*/,
	        const physx::PxF32* /*binormals*/,
	        const physx::PxF32* /*texcoords0*/,
	        const physx::PxF32* /*texcoords1*/,
	        const physx::PxF32* /*texcoords2*/,
	        const physx::PxF32* /*texcoords3*/,
	        const physx::PxU32*  /*indices*/,
	        physx::PxU32 /*triCount*/,
	        physx::PxU32 /*vertCount*/)
	{
	}

	virtual void renderIndexedTriangleMesh(physx::PxU32 triangleMeshId, const char* material,
	                                       physx::PxU32 transformCount,
	                                       const physx::PxF32* localToWorld44,
	                                       const physx::PxF32* positions,
	                                       const physx::PxF32* normals,
	                                       const physx::PxU32* colors,
	                                       const physx::PxF32* tangents,
	                                       const physx::PxF32* binormals,
	                                       const physx::PxF32* texcoords0,
	                                       const physx::PxF32* texcoords1,
	                                       const physx::PxF32* texcoords2,
	                                       const physx::PxF32* texcoords3,
	                                       const physx::PxU32*  indices,
	                                       physx::PxU32 triCount,
	                                       physx::PxU32 vertCount)
	{
		mRenderDebug->renderIndexedTriangleMesh(triangleMeshId, material, transformCount, localToWorld44, positions, normals, colors, tangents, binormals, texcoords0, texcoords1, texcoords2, texcoords3, indices, triCount, vertCount);
	}

	virtual	physx::PxU32 getTriangleMeshId(void)
	{
		return mRenderDebug->getTriangleMeshId();
	}

	virtual void releaseRenderDebug(void)
	{
		release();
	}

private:
	physx::PxProcessRenderDebug*	mProcessRenderDebug;
	physx::RenderDebug*	mRenderDebug;
	physx::PxU32                             mRenderSolidCount;
	physx::PxU32                             mRenderLineCount;

	physx::PxU32                             mLastRenderLineCount;
	physx::PxU32                             mLastRenderSolidCount;
	ApexSDK*	mApexSDK;
	NxResID                           mWireFrameMaterial;
	NxResID                           mSolidShadedMaterial;
	RenderResourceVector              mRenderLineResources;
	RenderContextVector               mRenderLineContexts;

	RenderResourceVector              mRenderSolidResources;
	RenderContextVector               mRenderSolidContexts;

	bool							  mUseDebugRenderable;
#if NX_SDK_VERSION_MAJOR == 2
	physx::Array<NxDebugLine>		  mNxDebugLines;
	physx::Array<NxDebugTriangle>	  mNxDebugTriangles;
	physx::Array<NxDebugLine>		  mNxDebugLinesScreenSpace;
	physx::Array<NxDebugTriangle>	  mNxDebugTrianglesScreenSpace;
#endif
#if NX_SDK_VERSION_MAJOR == 3
	physx::Array<PxDebugLine>		  mPxDebugLines;
	physx::Array<PxDebugTriangle>	  mPxDebugTriangles;
	physx::Array<PxDebugLine>		  mPxDebugLinesScreenSpace;
	physx::Array<PxDebugTriangle>	  mPxDebugTrianglesScreenSpace;
#endif

};


NiApexRenderDebug* createNiApexRenderDebug(ApexSDK* a, physx::PxProcessRenderDebug* debugChannel)
{
	ApexRenderDebug* m = PX_NEW(ApexRenderDebug)(a, debugChannel);
	return static_cast< NiApexRenderDebug*>(m);
}

void				releaseNiApexRenderDebug(NiApexRenderDebug* n)
{
	ApexRenderDebug* m = static_cast< ApexRenderDebug*>(n);
	delete m;
}

#endif // WITHOUT_DEBUG_VISUALIZE

}
} // end namespace physx::apex
