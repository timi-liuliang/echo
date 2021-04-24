#pragma once

#include "RenderWindow.h"
#include "engine/core/base/echo_def.h"

namespace Studio
{
    class RenderWindowMetal : public RenderWindow
    {
        Q_OBJECT
        
    public:
        RenderWindowMetal(QWidget* parent = NULL);
        virtual ~RenderWindowMetal();
        
        // begin render
        virtual void beginRender() override;
        
    public slots:
#ifdef ECHO_PLATFORM_MAC
        // render
        void RenderMetal();
#else
        void RenderMetal() {}
#endif
        
    private:
        // new metal window
        QWindow* newMetalWindow();
        
    private:
        void*   m_mtkView = nullptr;
    };
}

