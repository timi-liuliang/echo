#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
    class DecoderWav
    {
    public:
        struct Header
        {
            char            m_riff[4];
            unsigned long   m_riffSize;
            char            m_szWave[4];
        };
        
    public:
        DecoderWav(const char* path);
        ~DecoderWav();
        
    private:
        String              m_path;
        class MemoryReader* m_memoryReader = nullptr;
        Header*             m_header = nullptr;
    };
}
