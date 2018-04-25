#include "live2d_cubism_motion.h"
#include "CubismNativeComponents-3.0/Live2DCubismFramework.h"
#include "engine/core/main/Root.h"

namespace Echo
{
	Live2dCubismMotion::Live2dCubismMotion(const ResourcePath& path)
		: m_resPath("", ".json")
		, m_animationSize(0)
		, m_animationMemory(nullptr)
		, m_isPlaying(false)
	{
		setRes(path);
	}

	Live2dCubismMotion::~Live2dCubismMotion()
	{
		EchoSafeFree(m_animationMemory);
	}

	// set res
	void Live2dCubismMotion::setRes(const ResourcePath& path)
	{
		if (m_resPath.setPath(path.getPath()))
		{
			MemoryReader memReader(m_resPath.getPath());
			if (memReader.getSize())
			{
				m_animationSize = csmGetDeserializedSizeofAnimation(memReader.getData<char*>());
				m_animationMemory = EchoMalloc(m_animationSize);
				m_animation = csmDeserializeAnimationInPlace(memReader.getData<char*>(), m_animationMemory, m_animationSize);
			}
		}
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
		if (m_isPlaying)
		{
			float animationBlendWeight = 1.f;

			csmUpdateAnimationState(&m_animationState, delta);
			csmEvaluateAnimationFAST(
				m_animation,
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