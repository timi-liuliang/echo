#include "decoder_wav.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
    DecoderWav::DecoderWav(const char* path)
    {
        m_memoryReader = EchoNew(MemoryReader(path));
        if(m_memoryReader->getSize())
        {
            char* dataPtr = m_memoryReader->getData<char*>();
            m_header = (Header*)dataPtr; dataPtr+= sizeof(Header);
            
        }
    }
    
    DecoderWav::~DecoderWav()
    {
        
    }
}
