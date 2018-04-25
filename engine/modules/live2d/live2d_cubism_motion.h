#pragma once

#include "engine/core/io/DataStream.h"
#include "engine/core/resource/ResourcePath.h"
#include "live2d_cubism_motion_res.h"

namespace Echo
{
	class Live2dCubismMotion
	{
	public:
		Live2dCubismMotion(const ResourcePath& path);
		~Live2dCubismMotion();

		// play
		void play();

		// tick
		void tick(float delta, csmModel* model, csmModelHashTable* table);

	private:
		csmAnimationState		m_animationState;
		bool					m_isPlaying;
		Live2dCubismMotionRes*	m_motionRes;
	};
}