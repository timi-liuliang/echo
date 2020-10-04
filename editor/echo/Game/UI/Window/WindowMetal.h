#pragma once

#include "Window.h"
#include "engine/core/base/echo_def.h"

namespace Game
{
    class WindowMetal : public Window
    {
        Q_OBJECT
        
    public:
        WindowMetal(QWidget* parent = NULL);
        virtual ~WindowMetal();
        
        // begin render
        virtual void start(const Echo::String& echoProject) override;
        
    public slots:
#ifdef ECHO_PLATFORM_MAC
        // render
        void renderMetal();
#else
        void renderMetal() {}
#endif
        
    private:
        // new metal window
        QWindow* newMetalWindow();
        
    private:
        void*   m_mtkView = nullptr;
    };
}

