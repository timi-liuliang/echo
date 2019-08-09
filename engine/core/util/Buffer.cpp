#include "Buffer.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	Buffer::Buffer()
	{
	}

	Buffer::Buffer(ui32 size, void *pData, bool bAutoFree)
		: m_isAutoFree(bAutoFree)
	{
		set(size, pData, bAutoFree);
	}

	Buffer::~Buffer()
	{
		clear();
	}

	void Buffer::allocate(ui32 size)
	{
        if (size > 0)
        {
            clear();

            m_data = (Byte*)EchoMalloc(sizeof(Byte)*size);
            m_size = size;
            m_isAutoFree = true;
        }
        else
        {
            EchoLogError("Buffer allocated 0 size.");
        }
	}

	void Buffer::clear()
	{
		if (m_isAutoFree && m_data)
		{
			EchoSafeFree(m_data);
		}

		m_data = NULL;
		m_size = 0;
		m_isAutoFree = false;
	}

	void Buffer::set(ui32 size, void* pData, bool bAutoFree)
	{
		clear();

		m_data = (Byte*)pData;
		m_size = size;
		m_isAutoFree = bAutoFree;
	}

	void Buffer::set(const Buffer& buff)
	{
		allocate(buff.getSize());
		memcpy(m_data, buff.getData(), buff.getSize());
	}

	Byte* Buffer::getData() const
	{
		return m_data;
	}

	ui32 Buffer::getSize() const
	{ 
		return m_size; 
	}

	ui32 Buffer::takeData(Byte*& pData)
	{
		pData = m_data;
		m_data = NULL;
		ui32 size = m_size;
		m_size = 0;
		m_isAutoFree = false;
		return size;
	}
}
