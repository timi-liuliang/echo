#include "MemoryReader.h"
#include "IO.h"

namespace Echo
{
    MemoryReader::MemoryReader(const String& file)
    {
        DataStream* stream = IO::instance()->open(file);
        if (stream)
        {
            m_size = (ui32)stream->size();
            m_data = (char*)EchoMalloc(m_size + 1);
            stream->read(m_data, m_size);
            m_data[m_size] = '\0';

            EchoSafeDelete(stream, DataStream);
        }
        else
        {
            m_size = 0;
            m_data = nullptr;
        }
    }

    MemoryReader::MemoryReader(const char* data, ui32 size)
    {
        if (size > 0)
        {
            m_size = size;
            m_data = (char*)EchoMalloc(m_size + 1);
            std::memcpy(m_data, data, m_size);
            m_data[m_size] = '\0';
        }
        else
        {
            m_size = 0;
            m_data = nullptr;
        }
    }

    MemoryReader::~MemoryReader()
    {
        EchoSafeFree(m_data);
        m_size = 0;
    }

    MemoryReaderAlign::MemoryReaderAlign(const String& file, int align)
    {
        DataStream* stream = IO::instance()->open(file);
        if (stream)
        {
            m_size = (ui32)stream->size();
            m_data = (char*)EchoMalloc(m_size + align + 1);
            m_dataAlign = (char*)((size_t(m_data) + align) & ~(align - 1));
            stream->read(m_dataAlign, m_size);
            m_dataAlign[m_size] = '\0';

            EchoSafeDelete(stream, DataStream);
        }
        else
        {
            m_size = 0;
            m_data = nullptr;
            m_dataAlign = nullptr;
        }
    }

    MemoryReaderAlign::~MemoryReaderAlign()
    {
        EchoSafeFree(m_data);
        m_dataAlign = nullptr;
        m_size = 0;
    }
}