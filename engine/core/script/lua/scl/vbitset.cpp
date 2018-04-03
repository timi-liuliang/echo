#include "scl/vbitset.h"
	
#include "scl/assert.h"

#ifdef SCL_ANDROID
#include <string.h>
#include <stdlib.h>
#endif

#ifdef SCL_APPLE
#include <stdlib.h>
#endif

#ifdef SCL_WIN
#include <malloc.h>
#endif

#include <memory.h>

namespace scl {

vbitset::vbitset() : m_flag(NULL), m_bit_count(0) 
{
}


vbitset::~vbitset()
{
	if (NULL != m_flag) 
	{
		::free(m_flag);
		m_flag = NULL;
	}
	m_bit_count = 0;
}


unsigned char& vbitset::_get_flag(const int pos)
{
	if (NULL == m_flag || bit_count() <= 0)
	{
		assert(0);
		return m_flag[0];
	}
	if (pos < 0 || pos >= bit_count())
	{
		assert(0);
		return m_flag[0];
	}
	int flagIndex	= div8(pos);
	return m_flag[flagIndex];
}

const unsigned char& vbitset::_get_flag_const( const int pos ) const
{
	if (NULL == m_flag || bit_count() <= 0)
	{
		assert(0);
		return m_flag[0];
	}
	if (pos < 0 || pos >= bit_count())
	{
		assert(0);
		return m_flag[0];
	}
	int flagIndex	= div8(pos);
	return m_flag[flagIndex];
}

void vbitset::assign(const unsigned char* flags, const int sizeofData) 
{ 
	if (NULL == m_flag || bit_count() <= 0)
	{
		assert(0);
		return;
	}

	if (sizeofData <= byte_count()) 
		::memcpy(m_flag, flags, sizeofData); 
	else 
		assert(false); 
}

void vbitset::init(const int bit_count)
{
	m_bit_count = bit_count; 
	if (NULL != m_flag)
	{
		::free(m_flag);
	}
	m_flag = (unsigned char*)malloc(byte_count());
	reset();
}

void vbitset::set()
{
	::memset(m_flag, 0xFF, byte_count());
}

void vbitset::reset()
{
	::memset(m_flag, 0, byte_count());
}

void vbitset::clear()
{
	::memset(m_flag, 0, byte_count());
}

bool vbitset::dirty() const
{
	for (int i = 0; i < byte_count(); ++i) 
	{ 
		if (m_flag[i] != 0) 
			return true; 
	}
	return false;
}

} //namespace scl

