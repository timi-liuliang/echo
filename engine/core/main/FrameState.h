#pragma once

#include "engine/core/base/type_def.h"
#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/base/object.h"

namespace Echo
{
	class FrameState : public Object
	{
        ECHO_SINGLETON_CLASS(FrameState, Object);
        
	public:
        FrameState();
        ~FrameState();
        
        // instance
        static FrameState* instance();

        // reset
        void reset();
        
        // tick
        void tick(float elapsedTime);
        
        // get fps
        ui32 getFps() const { return m_fps; }

		// texture size
        ui32 getTextureSizeInBytes() const { return m_uploadedTextureSizeInBytes; }
		void incrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes += _sizeInBytes; }
		void decrUploadTextureSizeInBytes(ui32 _sizeInBytes) { m_uploadedTextureSizeInBytes -= _sizeInBytes; }

		// vertex count
        ui32 getVertexSize() const { return m_uploadedVertexSize; }
		void incrUploadVertexSize(ui32 _sizeInBytes) { m_uploadedVertexSize += _sizeInBytes; }
		void decrUploadVertexSize(ui32 _sizeInBytes) { m_uploadedVertexSize -= _sizeInBytes; }

		// triangle number
        ui32 getTriangleNum() const { return m_triangleNum; }
		void incrTriangleNum(ui32 _triangles) { m_triangleNum += _triangles; }

		// render target
        ui32 getRendertargetSize() const { return m_rendertargetSize; }
		void incrRendertargetSize(const ui32 size) { m_rendertargetSize += size; }
		void decrRendertargetSize(const ui32 size) { m_rendertargetSize -= size; }
        
        // draw calls
        void incrDrawCallTimes(ui32 _times) { m_drawCallTimes += _times; }
        ui32 getDrawCalls() const { return m_drawCallTimes; }
        
        // get current time
        const ui32& getCurrentTime() const;
        
    private:
        // calculate fps
        void calcuateFps(float elapsedTime);

	protected:
        ui32    m_currentTime = 0.0;
        ui32    m_fps = 0;
		ui32	m_uploadedTextureSizeInBytes = 0;
		ui32	m_uploadedVertexSize = 0;
        ui32    m_triangleNum = 0;
		ui32	m_rendertargetSize = 0;
		ui32	m_drawCallTimes = 0;
	};
}
