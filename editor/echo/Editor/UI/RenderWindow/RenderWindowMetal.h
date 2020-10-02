#pragma once

#include "RenderWindow.h"

#ifdef ECHO_PLATFORM_MAC

namespace Studio
{
    class RenderWindowMetal : public RenderWindow
    {
        Q_OBJECT
        
    public:
        RenderWindowMetal(QWidget* parent = NULL);
        virtual ~RenderWindowMetal();
        
        // begin render
        virtual void BeginRender() override;
        
    public slots:
        // render
        void RenderlMetal();
        
    private:
        // new metal window
        QWindow* newMetalWindow();
        
    private:
        void*   m_mtkView = nullptr;
    };
}

#endif