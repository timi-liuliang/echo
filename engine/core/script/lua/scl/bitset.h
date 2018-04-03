#pragma once

#include "scl/assert.h"
#include "scl/bitdef.h"

#ifdef SCL_ANDROID
#include <string.h>
#endif

#include <memory.h>

namespace scl {

////////////////////////////////////
//	bitset
//	所有position(pos 参数)都是指从m_flag最左侧记为零开始计算的bit位置索引（类似普通C数组）
////////////////////////////////////
template <int MAX_SIZE>
class bitset
{
public:
	static const int FLAG_SIZE = div8(MAX_SIZE) + mod8_flag(MAX_SIZE);

	bitset();

	void					set				(const int pos)		{ _set_bit	(_get_flag(pos), mod8(pos));		}
	void					set				()					{ ::memset(m_flag, 0xFF, sizeof(m_flag));		}
	void					reset			(const int pos)		{ _reset_bit(_get_flag(pos), mod8(pos));		}
	void					reset			()					{ ::memset(m_flag, 0, sizeof(m_flag));			}
	void					flip			(const int pos)		{ _flip_bit	(_get_flag(pos), mod8(pos));		}
	void					clear			()					{ ::memset(m_flag, 0, sizeof(m_flag)); }
	bool					dirty			() const				{ for (int i = 0; i < FLAG_SIZE; ++i) { if (m_flag[i] != 0) return true; } return false; }
	void					assign			(const unsigned char* flags, const int sizeofFlag);
	const unsigned char*	flags			() const { return m_flag; }
	unsigned char*			flags			() { return m_flag; }

	bool operator[](const int pos) const	{ return _get_bit(_get_flag_const(pos), mod8(pos));	}

	template <typename T> void map(T& s) { for (int i = 0; i < FLAG_SIZE; ++i) s << m_flag[i]; }

private:
	unsigned char&			_get_flag(const int pos);
	const unsigned char&	_get_flag_const(const int pos) const;

private:
	unsigned char	m_flag[FLAG_SIZE];
};

template <int MAX_SIZE>
bitset<MAX_SIZE>::bitset()
{
	::memset(m_flag, 0, sizeof(m_flag));
}

template <int MAX_SIZE>
unsigned char& bitset<MAX_SIZE>::_get_flag( const int pos )
{
	if (pos < 0 || pos >= MAX_SIZE)
	{
		assert(0);
		return m_flag[0];
	}
	int flagIndex	= div8(pos);
	return m_flag[flagIndex];
}

template <int MAX_SIZE>
const unsigned char& bitset<MAX_SIZE>::_get_flag_const( const int pos ) const
{
	if (pos < 0 || pos >= MAX_SIZE)
	{
		assert(0);
		return m_flag[0];
	}
	int flagIndex	= div8(pos);
	return m_flag[flagIndex];
}

template <int MAX_SIZE>
void bitset<MAX_SIZE>::assign(const unsigned char* flags, const int sizeofData) 
{ 
	if (sizeofData <= static_cast<int>(sizeof(m_flag))) 
		::memcpy(m_flag, flags, sizeofData); 
	else 
		assert(false); 
}

} //namespace scl



