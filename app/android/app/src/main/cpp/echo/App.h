#pragma once

#include <engine/core/util/StringUtil.h>

namespace Echo
{
    class App
    {
    public:
        App() {}
        ~App() {}

        // instance
        static App* instance();

        // init
        void initRes(const String& resDir, const String& userDir);
        void initEngine(i32 width, i32 height);

        // tick
        void tick();

    private:
        String      m_resDir;
        String      m_userDir;
    };
}