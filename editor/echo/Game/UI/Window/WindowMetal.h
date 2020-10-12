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
        
    private:
        // new metal window
        QWidget* newMetalWindow();
        
    private:
        void*   m_mtkView = nullptr;
    };
}

