#include "live2d_cubism_motion_res.h"
#include "engine/core/io/IO.h"

namespace Echo
{
	Live2dCubismMotionRes::Live2dCubismMotionRes(const ResourcePath& path)
		: Res(path)
		, m_animationSize(0)
		, m_animationMemory(nullptr)
		, m_animation(nullptr)
	{
		build();
	}

	Live2dCubismMotionRes::~Live2dCubismMotionRes()
	{
		EchoSafeFree(m_animationMemory);
	}

	Live2dCubismMotionRes* Live2dCubismMotionRes::create(const ResourcePath& path)
	{
		Res* res = Res::get(path);
		if (res)
			return dynamic_cast<Live2dCubismMotionRes*>(res);
		else
			return EchoNew(Live2dCubismMotionRes(path));
	}

	// set res
	void Live2dCubismMotionRes::build()
	{
		if (!m_path.getPath().empty())
		{
			MemoryReader memReader(m_path.getPath());
			if (memReader.getSize())
			{
				m_animationSize = csmGetDeserializedSizeofAnimation(memReader.getData<char*>());
				m_animationMemory = EchoMalloc(m_animationSize);
				m_animation = csmDeserializeAnimationInPlace(memReader.getData<char*>(), m_animationMemory, m_animationSize);
			}
		}
	}
}