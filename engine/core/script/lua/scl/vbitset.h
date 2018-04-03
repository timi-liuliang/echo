#pragma once

#include "scl/bitdef.h"

namespace scl {

////////////////////////////////////
//	bitset
//	所有position(pos 参数)都是指从m_flag最左侧记为零开始计算的bit位置索引（类似普通C数组）
////////////////////////////////////
class vbitset
{
public:
	vbitset();
	~vbitset();

	void					init			(const int bit_count);
	void					recapacity		(const int bit_count) { init(bit_count); }

	void					set				(const int pos)		{ _set_bit	(_get_flag(pos), mod8(pos));		}
	void					set				();
	void					reset			(const int pos)		{ _reset_bit(_get_flag(pos), mod8(pos));		}
	void					reset			();
	void					flip			(const int pos)		{ _flip_bit	(_get_flag(pos), mod8(pos));		}
	void					clear			();
	bool					dirty			() const;
	void					assign			(const unsigned char* flags, const int sizeofFlag);
	const unsigned char*	flags			() const { return m_flag; }
	unsigned char*			flags			() { return m_flag; }
	int						byte_count		() const { return div8(bit_count()) + mod8_flag(bit_count()); }
	int						bit_count		() const { return m_bit_count; }

	bool operator[](const int pos) const	{ return _get_bit(_get_flag_const(pos), mod8(pos));	}

	template <typename T> void map(T& s) { for (int i = 0; i < byte_count(); ++i) s << m_flag[i]; }

private:
	unsigned char&			_get_flag(const int pos);
	const unsigned char&	_get_flag_const(const int pos) const;

private:
	unsigned char*	m_flag;
	int				m_bit_count; //bit count of m_flag
};


} //namespace scl



