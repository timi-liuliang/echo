#include "Buffer.h"
#include "AssertX.h"

namespace Echo
{
	// ¹¹Ôìº¯Êý
	Buffer::Buffer()
		: m_pData(NULL)
		, m_size(0)
		, m_bAutoFree(false)
	{
	}

	Buffer::Buffer(ui32 size, void *pData, bool bAutoFree)
		: m_bAutoFree(bAutoFree)
		, m_pData(NULL)
		, m_size(0)
	{
		set(size, pData, bAutoFree);
	}

	Buffer::~Buffer()
	{
		clear();
	}

	void Buffer::allocate(ui32 size)
	{
		EchoAssertX(size > 0, "Buffer allocated 0 size.");

		clear();

		m_pData = (Byte*)EchoMalloc(sizeof(Byte)*size);
		m_size = size;
		m_bAutoFree = true;
	}

	void Buffer::clear()
	{
		if (m_bAutoFree && m_pData)
		{
			EchoSafeFree(m_pData);
		}

		m_pData = NULL;
		m_size = 0;
		m_bAutoFree = false;
	}

	void Buffer::set(ui32 size, void* pData, bool bAutoFree)
	{
		clear();

		m_pData = (Byte*)pData;
		m_size = size;
		m_bAutoFree = bAutoFree;
	}

	void Buffer::copyBuffer(const Buffer& buff)
	{
		allocate(buff.getSize());
		memcpy(m_pData, buff.getData(), buff.getSize());
	}

	Byte* Buffer::getData() const
	{
		return m_pData;
	}

	ui32 Buffer::getSize() const
	{ 
		return m_size; 
	}

	ui32 Buffer::takeData(Byte*& pData)
	{
		pData = m_pData;
		m_pData = NULL;
		ui32 size = m_size;
		m_size = 0;
		m_bAutoFree = false;
		return size;
	}
}