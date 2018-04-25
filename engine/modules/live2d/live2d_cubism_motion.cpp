#include "live2d_cubism_motion.h"
#include "CubismNativeComponents-3.0/Live2DCubismFramework.h"

namespace Echo
{
	Live2dCubismMotion::Live2dCubismMotion(const ResourcePath& path)
		: m_resPath("", ".json")
		, m_jsonMemory(nullptr)
		, m_animationSize(0)
		, m_animationMemory(nullptr)
	{
		setRes(path);
	}

	Live2dCubismMotion::~Live2dCubismMotion()
	{
		EchoSafeDelete(m_jsonMemory, MemoryReader);
	}

	// set res
	void Live2dCubismMotion::setRes(const ResourcePath& path)
	{
		if (m_resPath.setPath(path.getPath()))
		{
			m_jsonMemory = EchoNew(MemoryReader(m_resPath.getPath()));
			if (m_jsonMemory->getSize())
			{
				m_animationSize = csmGetDeserializedSizeofAnimation( m_jsonMemory->getData<char*>());
				m_animationMemory = EchoMalloc(m_animationSize);
				m_animation = csmDeserializeAnimationInPlace( m_jsonMemory->getData<char*>(), m_animationMemory, m_animationSize);
			}
		}
	}
}