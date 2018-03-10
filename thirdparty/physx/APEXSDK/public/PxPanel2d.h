/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PX_PANEL2D_H
#define PX_PANEL2D_H

#include "PxRenderDebug.h"

// This is a helper class to do 2d debug rendering in screenspace.
namespace physx
{
	namespace general_renderdebug4
	{


class PxPanel2D
{
public:
	PxPanel2D(RenderDebug *debugRender,PxU32 pwid,PxU32 phit)
	{
		mRenderDebug = debugRender;
		mWidth		= pwid;
		mHeight		= phit;
		mRecipX		= 1.0f / (pwid/2);
		mRecipY		= 1.0f / (phit/2);
		mRenderDebug->pushRenderState();
		mRenderDebug->addToCurrentState(DebugRenderState::ScreenSpace);
	}

	~PxPanel2D(void)
	{
		mRenderDebug->popRenderState();
	}


	void setFontSize(PxU32 fontSize)
	{
		const PxF32 fontPixel = 0.009f;
		mFontSize = (PxF32)(fontSize*fontPixel);
		mFontOffset = mFontSize*0.3f;
		mRenderDebug->setCurrentTextScale(mFontSize);
	}

	void printText(PxU32 x,PxU32 y,const char *text)
	{
		PxVec3 p;
		getPos(x,y,p);
		p.y-=mFontOffset;
		mRenderDebug->debugText(p,"%s",text);
	}

	void getPos(PxU32 x,PxU32 y,PxVec3 &p)
	{
		p.z = 0;
		p.x = (PxF32)x*mRecipX-1;
		p.y = 1-(PxF32)y*mRecipY;
	}

	void drawLine(PxU32 x1,PxU32 y1,PxU32 x2,PxU32 y2)
	{
		PxVec3 p1,p2;
		getPos(x1,y1,p1);
		getPos(x2,y2,p2);
		mRenderDebug->debugLine(p1,p2);
	}
private:
	RenderDebug	*mRenderDebug;

	PxF32		mFontSize;
	PxF32		mFontOffset;
	PxF32		mRecipX;
	PxF32		mRecipY;
	PxU32		mWidth;
	PxU32		mHeight;
};

	}; // end namespace general_renderdebug4
}; // end namespace physx

#endif
