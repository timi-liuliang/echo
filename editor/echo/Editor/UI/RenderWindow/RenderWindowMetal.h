#pragma once

#include "RenderWindow.h"

namespace Studio
{
    class RenderWindowMetal : public RenderWindow
    {
        Q_OBJECT
        
    public:
        RenderWindowMetal(QWidget* parent = NULL);
        virtual ~RenderWindowMetal();
        
    private:
        // new metal window
        QWindow* newMetalWindow();
    };
}
