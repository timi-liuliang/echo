#pragma once

#include "engine/core/resource/Res.h"

extern "C"
{
#include "CubismNativeComponents-3.0/Live2DCubismFramework.h"
}

namespace Echo
{
	class Live2dCubismMotionRes : public Res
	{
	public:
		Live2dCubismMotionRes(const ResourcePath& path);
		virtual ~Live2dCubismMotionRes();

		// create
		static Live2dCubismMotionRes* create(const ResourcePath& path);

		// set res
		void build();

		// get animation
		csmAnimation* getAnim() { return m_animation; }

	protected:
		ui32				m_animationSize;
		void*				m_animationMemory;
		csmAnimation*		m_animation;
	};
	typedef ResRef<Live2dCubismMotionRes> Live2dCubismMotionResPtr;
}