#pragma once

#include "engine/core/base/TypeDef.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class FrameState
	{
	public:
		FrameState()
			: m_uploadedTextureSizeInBytes(0)
			, m_uploadedGeometricSize(0)
			, m_lockTimes(0)
            , m_triangleNum(0)
		{}

		~FrameState() {}

		inline void reset()
		{
			m_triangleNum = 0;
			m_lockTimes = 0;
			m_maxFrameTimePerSecond = 0;
		}

		inline void incrUploadTextureSizeInBytes(ui32 _sizeInBytes)
		{
			m_uploadedTextureSizeInBytes += _sizeInBytes;
		}

		inline void decrUploadTextureSizeInBytes(ui32 _sizeInBytes)
		{
			m_uploadedTextureSizeInBytes -= _sizeInBytes;
		}

		inline void incrUploadGeometricSize(ui32 _sizeInBytes)
		{
			m_uploadedGeometricSize += _sizeInBytes;
		}

		inline void decrUploadGeometricSize(ui32 _sizeInBytes)
		{
			m_uploadedGeometricSize -= _sizeInBytes;
		}

		inline void incrLockTimes(ui32 _time)
		{
			m_lockTimes += _time;
		}

		void incrTriangleNum(ui32 _triangles)
		{
			m_triangleNum += _triangles;
		}

		inline ui32 getTextureSizeInBytes() const
		{
			return m_uploadedTextureSizeInBytes;
		}

		inline ui32 getVertexSize() const
		{
			return m_uploadedGeometricSize;
		}

		inline ui32 getTriangleNum() const
		{
			return m_triangleNum;
		}
	protected:
		ui32 m_uploadedTextureSizeInBytes;
		ui32 m_uploadedGeometricSize;
		ui32 m_lockTimes;
		ui32 m_maxFrameTimePerSecond;
		ui32 m_minTimeFrame;
		ui32 m_maxTimeFrame;
		ui32 m_triangleNum;
		String m_fpsMsg;
		String m_maxFrameTimeMsg;
		String m_lockTimesMsg;
		String m_drawCallTimesMsg;
		String m_triangleNumMsg;
		String m_uploadedTextureSizeInBytesMsg;
		String m_uploadedGeometricSizeMsg;
		String m_streamThreadCount;
		String m_mathOpCount;
	};
}
