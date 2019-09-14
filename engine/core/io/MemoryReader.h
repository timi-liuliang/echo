#pragma once

#include "DataStream.h"

namespace Echo
{
    class MemoryReader
    {
    public:
        MemoryReader(const String& file);
        MemoryReader(const char* data, ui32 size);
        ~MemoryReader();

        // get size
        template <typename T> T getData() { return reinterpret_cast<T>(m_data); }

        // get data
        ui32 getSize() { return m_size; }

    private:
        char*   m_data;
        ui32	m_size;
    };

    class MemoryReaderAlign
    {
    public:
        MemoryReaderAlign(const String& file, int align = 0);
        ~MemoryReaderAlign();

        // get data
        template <typename T> T getData() { return static_cast<T>(m_dataAlign); }

        // get size
        ui32 getSize() { return m_size; }

    private:
        char*   m_dataAlign;
        char*   m_data;
        ui32    m_size;
    };
}