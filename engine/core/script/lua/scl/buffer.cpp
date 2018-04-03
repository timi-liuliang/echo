////////////////////////////////////////////////////////////
//	buffer.h
//  caolei 2012.05.27
////////////////////////////////////////////////////////////
#include "scl/buffer.h"
#include "scl/assert.h"

#ifdef SCL_ANDROID
#include <stdlib.h>
#else
#include <memory.h>
#endif


namespace scl {

buffer::buffer() :
	m_buffer	(NULL),
	m_start		(0),
	m_end		(0),
	m_maxLength	(0),
	m_autoDelete(true)
{
}

buffer::buffer(void* p, const int max_length, const int start, const int end) :
	m_buffer	(static_cast<uint8*>(p)),
	m_start		(start),
	m_end		(end),
	m_maxLength	(max_length),
	m_autoDelete(false)
{
		
}


buffer::~buffer()
{
	if (m_autoDelete)
	{
		delete[] m_buffer;
		m_buffer = NULL;
	}
}


void buffer::alloc(const int maxLength)
{
	assert(NULL == m_buffer);
	m_buffer = new uint8[maxLength];
	::memset(m_buffer, 0, maxLength);
	m_maxLength = maxLength;
	m_start 	= 0;
	m_end		= 0;
	m_autoDelete= true;
}

void buffer::init(void* pData, const int maxLength, const int start, const int end)
{
	assert(NULL == m_buffer);
	m_buffer	= static_cast<uint8*>(pData);
	m_maxLength = maxLength;
	m_start 	= start;
	m_end		= end;
	m_autoDelete= false;
}

int buffer::read(void* p, const int readLength)
{
	const int copyLength = peek(p, readLength);
	m_start += copyLength;
	return copyLength;
}


int buffer::peek(void* p, const int readLength)
{
	int copyLength			= readLength;
	const int bufferLength	= length();
	if (copyLength > bufferLength)
	{
		copyLength = bufferLength;
	}
	::memcpy(p, m_buffer + m_start, copyLength);
	return copyLength;
}

void buffer::write( const void* const p, const int length )
{
	const int freeLength = m_maxLength - m_end;
	if (freeLength < length)
	{
		assertf(0, "free length = %d, need length = %d", freeLength, length);
		return;
	}
	::memcpy(m_buffer + m_end, p, length);
	m_end += length;
}

} //namespace scl 

