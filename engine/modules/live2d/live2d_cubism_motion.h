#pragma once

#include "engine/core/io/DataStream.h"
#include "engine/core/resource/ResourcePath.h"

extern "C"
{
#include "CubismNativeComponents-3.0/Live2DCubismFramework.h"
}

namespace Echo
{
	class Live2dCubismMotion
	{
	public:
		Live2dCubismMotion(const ResourcePath& path);
		~Live2dCubismMotion();

		// set res
		void setRes(const ResourcePath& path);

	private:
		ResourcePath		m_resPath;
		MemoryReader*		m_jsonMemory;
		ui32				m_animationSize;
		void*				m_animationMemory;
		csmAnimation*		m_animation;
	};
}