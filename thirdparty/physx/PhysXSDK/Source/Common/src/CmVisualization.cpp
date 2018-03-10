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

#include "CmPhysXCommon.h"
#include "PxTransform.h"
#include "CmRenderOutput.h"
#include "CmVisualization.h"

using namespace physx;
using namespace Cm;

void Cm::visualizeJointFrames(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& parent,
							  const PxTransform& child)
{
	out << parent << Cm::DebugBasis(PxVec3(scale, scale, scale) * 1.5f,
		PxU32(PxDebugColor::eARGB_DARKRED), PxU32(PxDebugColor::eARGB_DARKGREEN), PxU32(PxDebugColor::eARGB_DARKBLUE));
	out << child << Cm::DebugBasis(PxVec3(scale, scale, scale));	
}

void Cm::visualizeLinearLimit(RenderOutput& out,
							  PxReal scale,
							  const PxTransform& t0,
							  const PxTransform& /*t1*/,
							  PxReal value,
							  bool active)
{
	// debug circle is around z-axis, and we want it around x-axis
	PxTransform r(t0.p+value*t0.q.getBasisVector0(), t0.q*PxQuat(PxPi/2,PxVec3(0,1.f,0)));
	out << (active ? PxDebugColor::eARGB_RED : PxDebugColor::eARGB_GREY);
	out << PxTransform(PxIdentity);
	out << Cm::DebugArrow(t0.p,r.p);

	out << r << Cm::DebugCircle(20, scale*0.3f);
}

void Cm::visualizeAngularLimit(RenderOutput& out,
							   PxReal scale,
							   const PxTransform& t,
							   PxReal lower,
							   PxReal upper,
							   bool active)
{
	out << t << (active ? PxDebugColor::eARGB_RED : PxDebugColor::eARGB_GREY);
	
	out << Cm::RenderOutput::LINES 
		<< PxVec3(0) << PxVec3(0, PxCos(lower), PxSin(lower)) * scale
		<< PxVec3(0) << PxVec3(0, PxCos(upper), PxSin(upper)) * scale;

	out << Cm::RenderOutput::LINESTRIP;
	PxReal angle = lower, step = (upper-lower)/20;

	for(PxU32 i=0; i<=20; i++, angle += step)
		out << PxVec3(0, PxCos(angle), PxSin(angle)) * scale;
}

void Cm::visualizeLimitCone(RenderOutput& out,
							PxReal scale,
							const PxTransform& t,
							PxReal tanQSwingY,
							PxReal tanQSwingZ,
							bool active)
{
	out << t << (active ? PxDebugColor::eARGB_RED : PxDebugColor::eARGB_GREY);	
	out << Cm::RenderOutput::LINES;

	PxVec3 prev(0,0,0);
	
	const int LINES = 32;

	for(PxU32 i=0;i<=32;i++)
	{
		PxReal angle = 2*PxPi/LINES*i;
		PxReal c = PxCos(angle), s = PxSin(angle);
		PxVec3 rv(0,-tanQSwingZ*s, tanQSwingY*c);
		PxReal rv2 = rv.magnitudeSquared();
		PxQuat q = PxQuat(0,2*rv.y,2*rv.z,1-rv2) * (1/(1+rv2));
		PxVec3 a = q.rotate(PxVec3(1.0f,0,0)) * scale;

		out << prev << a << PxVec3(0) << a;
		prev = a;
	}
}


void Cm::visualizeDoubleCone(Cm::RenderOutput& out,
							 PxReal scale,
							 const PxTransform& t,
							 PxReal angle,
							 bool active)
{
	out << t << (active ? PxDebugColor::eARGB_RED : PxDebugColor::eARGB_GREY);	

	const PxReal height = PxSin(angle);//, radius = cos(angle);

	const PxU32 LINES = 32;

	out << Cm::RenderOutput::LINESTRIP;

	const PxReal step = PxPi*2/LINES;

	for(PxU32 i=0; i<=LINES; i++)
		out << PxVec3(height, PxCos(step * i), PxSin(step * i)) * scale;

	angle = 0;
	out << Cm::RenderOutput::LINESTRIP;
	for(PxU32 i=0; i<=LINES; i++, angle += PxPi*2/LINES)
		out << PxVec3(-height, PxCos(step * i), PxSin(step * i)) * scale;

	angle = 0;
	out << Cm::RenderOutput::LINES;
	for(PxU32 i=0;i<LINES;i++, angle += PxPi*2/LINES)
	{
		out << PxVec3(0) << PxVec3(-height, PxCos(step * i), PxSin(step * i)) * scale;
		out << PxVec3(0) << PxVec3(height, PxCos(step * i), PxSin(step * i)) * scale;
	}
}

