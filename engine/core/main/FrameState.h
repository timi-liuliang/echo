#pragma once

#include "engine/core/base/type_def.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class FrameState
	{
	public:
		FrameState()
			: m_uploadedTextureSizeInBytes(0)
			, m_uploadedGeometricSize(0)
			, m_rendertargetSize(0)
			, m_drawCallTimes(0)
			, m_lockTimes(0)
			, m_fps(0)
			, m_triangleNum(0)
		{}

		~FrameState()
		{}

		// 重置
		void reset()
		{
			m_triangleNum = 0;
			m_drawCallTimes = 0;
			m_lockTimes = 0;
			m_fps = 0;
			m_maxFrameTimePerSecond = 0;
		}

		// 增加纹理占用大小(单位：字节)
		void incrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes += _sizeInBytes; }

		// 减少纹理占用大小(单位：字节)
		void decrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes -= _sizeInBytes; }

		// 增加几何体三角形大小
		void incrUploadGeometricSize(ui32 _sizeInBytes) { m_uploadedGeometricSize += _sizeInBytes; }

		// 减少几何体内记录大小
		void decrUploadGeometricSize(ui32 _sizeInBytes) { m_uploadedGeometricSize -= _sizeInBytes; }

		// 增加渲染批次
		void incrDrawCallTimes(ui32 _times) { m_drawCallTimes += _times; }

		// 增加锁次数
		void incrLockTimes(ui32 _time) { m_lockTimes += _time; }

		// 增加三角形数量
		void incrTriangleNum(ui32 _triangles) { m_triangleNum += _triangles; }

		// 获取渲染批次
		ui32 getDrawCalls() const { return m_drawCallTimes; }

		// 获取纹理尺寸
		ui32 getTextureSizeInBytes() const { return m_uploadedTextureSizeInBytes; }

		// 获取顶点大小
		ui32 getVertexSize() const { return m_uploadedGeometricSize; }

		// 获取三角形数量
		ui32 getTriangleNum() const { return m_triangleNum; }

		// 设置rendertarget内存大小
		void incrRendertargetSize(const ui32 size) { m_rendertargetSize += size; }
		void decrRendertargetSize(const ui32 size) { m_rendertargetSize -= size; }

		// 获取rendertarget内存大小
		ui32 getRendertargetSize() const { return m_rendertargetSize; }

	protected:
		ui32	m_uploadedTextureSizeInBytes;
		ui32	m_uploadedGeometricSize;
		ui32	m_rendertargetSize;
		ui32	m_drawCallTimes;
		ui32	m_lockTimes;
		ui32	m_fps;
		ui32	m_maxFrameTimePerSecond;
		ui32	m_minTimeFrame;
		ui32	m_maxTimeFrame;
		ui32	m_triangleNum;
		String	m_fpsMsg;
		String	m_maxFrameTimeMsg;
		String	m_lockTimesMsg;
		String	m_drawCallTimesMsg;
		String	m_triangleNumMsg;
		String	m_uploadedTextureSizeInBytesMsg;
		String	m_uploadedGeometricSizeMsg;
		String	m_streamThreadCount;
		String	m_mathOpCount;
	};
}
