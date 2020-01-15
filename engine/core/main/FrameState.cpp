#include "FrameState.h"
#include "engine/core/util/Timer.h"

namespace Echo
{
    FrameState::FrameState()
    {}

    FrameState::~FrameState()
    {}

    FrameState* FrameState::instance()
    {
        static FrameState* inst = EchoNew(FrameState);
        return inst;
    }

    void FrameState::bindMethods()
    {
        CLASS_BIND_METHOD(FrameState, getFps, "getFps");
    }

    void FrameState::reset()
    {
        m_triangleNum = 0;
        m_drawCallTimes = 0;
    }

    void FrameState::tick(float elapsedTime)
    {
        calcuateFps(elapsedTime);
        
        m_currentTime = static_cast<ui32>(Time::instance()->getMilliseconds());
    }

    void FrameState::calcuateFps(float elapsedTime)
    {
        static float totalElapsedTime = 0.f;
        static float totalElapsedFrame = 0.f;
        
        totalElapsedTime += elapsedTime;
        totalElapsedFrame += 1.f;
        
        if( totalElapsedTime >= 0.5f)
        {
            m_fps = totalElapsedFrame / totalElapsedTime;
            
            totalElapsedTime = 0.f;
            totalElapsedFrame = 0.f;
        }
    }
}
