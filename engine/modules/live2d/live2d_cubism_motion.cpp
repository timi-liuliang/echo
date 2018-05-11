#include "live2d_cubism_motion.h"
#include "CubismNativeComponents-3.0/Live2DCubismFramework.h"
#include "engine/core/resource/Res.h"
#include "engine/core/main/Root.h"

namespace Echo
{
	Live2dCubismMotion::Live2dCubismMotion(const ResourcePath& path)
		: m_isPlaying(false)
		, m_motionRes(nullptr)
	{
		m_motionRes = Live2dCubismMotionRes::create( path);
	}

	Live2dCubismMotion::~Live2dCubismMotion()
	{
		m_motionRes->subRefCount();
	}

	// play
	void Live2dCubismMotion::play()
	{
		csmResetAnimationState(&m_animationState);
		m_isPlaying = Root::instance()->getConfig().m_isGame;
	}

	// tick
	void Live2dCubismMotion::tick(float delta, csmModel* model, csmModelHashTable* table)
	{
		if (m_isPlaying && m_motionRes)
		{
			float animationBlendWeight = 1.f;

			csmUpdateAnimationState(&m_animationState, delta);
			csmEvaluateAnimationFAST(
				m_motionRes->getAnim(),
				&m_animationState, 
				csmOverrideFloatBlendFunction, 
				animationBlendWeight,
				model,
				table,
				0,
				0);
		}
	}
}