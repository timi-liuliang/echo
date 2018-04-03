#pragma once

#include "scl/type.h"
#include "scl/assert.h"
#include "scl/math.h"
#include "scl/stringdef.h"

#include <memory.h>

//TODO:
//	from_string
//	to_string
//	溢出处理
//	正负数处理

namespace scl {

//enum BYTEORDER
//{
//	BYTEORDER_INVALID		= -1,
//	BYTEORDER_BIG_ENDIAN	= 0,
//	BYTEORDER_LITTLE_ENDIAN	= 1,
//};
//
//inline BYTEORDER current_byteorder()
//{
//	static BYTEORDER bo = BYTEORDER_INVALID;
//	if (bo == BYTEORDER_INVALID)
//	{
//		int i = 1;
//		bo = static_cast<BYTEORDER>(*((char*)&i));
//	}
//	return bo;
//}

template <int MAX_BYTE_COUNT>
class big_int
{
public:
	big_int()									{ clear(); }
	template<typename T>		big_int(T i)	{ from(i); }

	void		from			(const int			i);
	void		from			(const uint			i);
	void		from			(const int64		i);
	void		from			(const uint64		i);
	void		from			(const char* const	s);
	
	void		add				(const big_int& bi);
	void		dec				(const big_int& bi);
	void		mul				(const big_int& bi);
	void		div				(const big_int& down);				 // up ÷ div = up.div(down);
	void		div				(const big_int& down, big_int& mod); // up ÷ div = up.div(down);

	//正负号相关
	void		not				()	{ m_sign = -m_sign;		}		//取反
	void		absolute		()	{ m_sign = 1;			}		//绝对值
	char		sign			()	const { assert(m_sign >= -1 && m_sign <= 1); return m_sign;		}		//正负号	,1表示正数，-1表示负数

	void		clear			();
	int			to_int			();
	uint		to_uint			();
	int64		to_int64		();
	uint64		to_uint64		();
	void		to_string		(char* buffer, const int buffer_size);
	int			compare			(const big_int& bi);
	bool		is_zero			() const;
	bool		is_positive		() const;
	bool		is_negative		() const;

	static		big_int			zero;

private:
	//下面4个函数，仅对值作运算，不考虑正负号
	void		_add_value		(const big_int& bi);
	void		_dec_value		(const big_int& bi);
	void		_mul_value		(const big_int& bi);
	void		_div_value		(const big_int& down, big_int& result_mod); // up ÷ div = up.div(down);
	int			_compare_value	(const big_int& bi) const;

	int			_small_div		(const big_int& bi);	//调用这个函数，要求被除数和除数的位数差不会超过1
	void		_mul_byte		(byte b);
	void		_shift_left		(const int shift = 1);	//例如:0x00AB	_shift_left(1) = 0x0AB0	_shift_left(2) = 0xAB00
	void		_shift_right	(const int shift = 1);	//例如:0xAB00 	_shift_right(1) = 0x0AB0 	_shift_right(2) = 0x00AB
	int			_length			() const;
	void		_copy			(big_int& dest, const int src_start, const int src_length = -1);	//src_start		从低位开始的索引 src_length	复制长度，如果为-1，那么长度为 _length() - src_start

private:
	char	m_sign;								//正负号	,1表示正数，-1表示负数
	byte	m_buffer	[MAX_BYTE_COUNT];		// m_buffer使用little-endian。 从左（低地址）向右（高地址），为从低位向高位。 m_buffer[0]是最低位（lsb），m_buffer[MAX - 1]是最高位（msb）
};

template <int MAX_BYTE_COUNT> big_int<MAX_BYTE_COUNT> big_int<MAX_BYTE_COUNT>::zero;

//int32 int64 is basic type
typedef big_int<16>		int128;
typedef big_int<32>		int256;
typedef big_int<64>		int512;
typedef big_int<128>	int1024;
typedef big_int<256>	int2048;
typedef big_int<512>	int4096;
typedef big_int<1024>	int8192;

typedef big_int<128>	int1k;
typedef big_int<256>	int2k;
typedef big_int<512>	int4k;
typedef big_int<1024>	int8k;


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::clear()
{
	m_sign = 0;
	memset(m_buffer, 0, sizeof(m_buffer));
}

template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::from(const int i)
{
	if (MAX_BYTE_COUNT < sizeof(i))
	{
		assert(0);
		return;
	}
	clear();
	if (i < 0)
	{
		m_sign	= -1;
		int abs = scl::absolute(i);
		memcpy(m_buffer, &abs, sizeof(i));
	}
	else if (i > 0)
	{
		m_sign = 1;
		memcpy(m_buffer, &i, sizeof(i));
	}
}

template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::from(const uint i)
{
	if (MAX_BYTE_COUNT < sizeof(i))
	{
		assert(0);
		return;
	}
	clear();
	m_sign = i > 0;
	memcpy(m_buffer, &i, sizeof(i));
}

template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::from(const int64 i)
{
	if (MAX_BYTE_COUNT < sizeof(i))
	{
		assert(0);
		return;
	}
	clear();
	if (i < 0)
	{
		m_sign	= -1;
		int64 abs = scl::absolute(i);
		memcpy(m_buffer, &abs, sizeof(i));
	}
	else if (i > 0)
	{
		m_sign = 1;
		memcpy(m_buffer, &i, sizeof(i));
	}
}

template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::from(const uint64 i)
{
	if (MAX_BYTE_COUNT < sizeof(i))
	{
		assert(0);
		return;
	}
	clear();
	m_sign = i > 0;
	memcpy(m_buffer, &i, sizeof(i));
}

template <int MAX_BYTE_COUNT>
int64 big_int<MAX_BYTE_COUNT>::to_int64()
{
	assert(_length() <= 8);
	int64 r = 0;
	memcpy(&r, m_buffer, sizeof(r));
	return r * sign();
}


template <int MAX_BYTE_COUNT>
uint64 big_int<MAX_BYTE_COUNT>::to_uint64()
{
	assert(_length() <= 8);
	uint64 r = 0;
	memcpy(&r, m_buffer, sizeof(r));
	return r;
}


template <int MAX_BYTE_COUNT>
int big_int<MAX_BYTE_COUNT>::to_int()
{
	assert(_length() <= 4);
	int r = 0;
	memcpy(&r, m_buffer, sizeof(r));
	return r * sign();
}


template <int MAX_BYTE_COUNT>
uint big_int<MAX_BYTE_COUNT>::to_uint()
{
	assert(_length() <= 4);
	uint r = 0;
	memcpy(&r, m_buffer, sizeof(r));
	return r;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::to_string(char* buffer, const int buffer_size)
{
	if (is_zero())
	{
		buffer[0] = '0';
		buffer[1] = 0;
		return;
	}
	big_int t(*this);
	int n = buffer_size;
	while (!t.is_zero() && n-- >= 0)
	{
		big_int mod;
		t.div(10, mod);
		int imod = scl::absolute(mod.to_int());
		assert(imod >= 0 && imod <= 9);
		buffer[n] = imod + '0';
	}
	if (is_negative() && --n >= 0)
		buffer[n] = '-';
	for (int k = 0; k < buffer_size - n; ++k) 
		buffer[k] = buffer[n + k];
	
	buffer[buffer_size - n] = 0;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::add(const big_int& bi)
{
	if (this->sign() * bi.sign() >= 0)			//同号,直接相加
	{
		_add_value(bi);
	}
	else										//异号
	{
		if (this->_compare_value(bi) > 0)		//本身绝对值大
		{
			_dec_value(bi);
		}
		else									//本身绝对值小
		{
			big_int t;
			t = *this;
			*this = bi;
			_dec_value(t);
		}
	}
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::dec(const big_int& bi)
{
	if (this->sign() * bi.sign() <= 0)	//异号相减，直接值相加
	{
		_add_value(bi);
	}
	else								//同号相减，做减法
	{
		const int compare = this->_compare_value(bi);
		if (compare == 0)
		{
			clear();
		}
		else if (compare < 0)
		{
			big_int t(*this);
			*this = bi;
			this->_dec_value(t);
			this->not();
		}
		else //if compare > 0
		{
			this->_dec_value(bi);
		}
	}
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::mul(const big_int& bi)
{
	_mul_value(bi);
	m_sign = m_sign * bi.m_sign;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::div(const big_int& down) //up.div(down)
{
	big_int mod;
	div(down, mod);
}



template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::div(const big_int& down, big_int& mod) //up.div(down)
{
	_div_value(down, mod);
	m_sign = m_sign * down.m_sign;
	if (_compare_value(zero) == 0)
		m_sign = 0;
}

template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_add_value(const big_int& bi)
{
	assert(m_buffer != bi.m_buffer);
	if (bi._compare_value(zero) == 0)
		return;

	if (this->_compare_value(zero) == 0)
	{
		*this = bi;
		return;
	}
	int carry = 0;
	for (int i = 0; i < MAX_BYTE_COUNT; ++i)
	{
		int n = m_buffer[i];
		const int sum = n + bi.m_buffer[i] + carry;
		if (sum > 0xFF)
		{
			carry = sum / 0x0100;
			m_buffer[i] = sum - (carry * 0x0100);
		}
		else
		{
			carry = 0;
			m_buffer[i] = sum;
		}
	}
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_dec_value(const big_int& bi)
{
	assert(m_buffer != bi.m_buffer);
	if (bi._compare_value(zero) == 0)
		return;

	if (this->_compare_value(zero) == 0)
	{
		*this = bi;
		m_sign = -m_sign;
		return;
	}
	int borrow = 0;
	for (int i = 0; i < MAX_BYTE_COUNT; ++i)
	{
		int n = m_buffer[i];
		const int dec = n - bi.m_buffer[i] - borrow;
		if (dec < 0)
		{
			borrow = -dec / 0x0100 + 1;
			m_buffer[i] = dec + (borrow * 0x0100);
		}
		else
		{
			borrow = 0;
			m_buffer[i] = dec;
		}
	}
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_mul_value(const big_int& bi)
{
	assert(m_buffer != bi.m_buffer);
	if (this->_length() == 0 || bi._length() == 0)
	{
		memset(m_buffer, 0, sizeof(m_buffer));
		return;
	}
	big_int result;
	for (int i = 0; i < MAX_BYTE_COUNT; ++i)
	{
		big_int product(*this);
		product._mul_byte(bi.m_buffer[i]);
		product._shift_left(i);
		result._add_value(product);
	}
	result.m_sign = this->m_sign;
	*this = result;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_div_value(const big_int& down, big_int& result_mod) //up.div(down)
{
	assert(m_buffer != down.m_buffer);
	assert(down._length() > 0);

	if (_compare_value(down) < 0)		//被除数比除数小，返回0
	{
		result_mod = *this;
		memset(m_buffer, 0, sizeof(m_buffer));
		return;
	}
	else if (_compare_value(down) == 0)//被除数与除数相等，返回1
	{
		result_mod.clear();
		memset(m_buffer, 0, sizeof(m_buffer));
		m_buffer[0] = 1;
		return;
	}

	int up_len		= this->_length();
	int down_len	= down._length();

	big_int result;
	result.m_sign = this->m_sign;

	//如果除数为n位，那么被除数的高n-1位的商都是0，所以这里直接拷贝被除数的高n-1位到mod中，增加效率
	big_int mod;
	_copy(mod, up_len - down_len + 1, down_len - 1);
	mod.m_sign = this->m_sign;

	//从最高位右侧的n-1位开始除
	for (int i = (up_len - 1) - (down_len - 1); i >= 0; --i)
	{
		mod._shift_left();
		mod.m_buffer[0]		= this->m_buffer[i];
		int quotient		= mod._small_div(down);
		result.m_buffer[i]	= static_cast<byte>(quotient);

		//计算余数
		//计算商和除数的整数乘积
		big_int product(quotient);
		product._mul_value(down);

		//计算余数
		mod._dec_value(product);
	}

	result_mod = mod;
	*this = result;
}



template <int MAX_BYTE_COUNT>
int big_int<MAX_BYTE_COUNT>::_small_div(const big_int& bi)
{
	assert(m_buffer != bi.m_buffer);

	//取两个big int的长度
	int up_len		= this->_length();
	int down_len	= bi._length();

	assert(down_len > 0);

	if (up_len == down_len)
	{
		int up = m_buffer[up_len - 1];
		int down = bi.m_buffer[down_len - 1];
		return up / down;
	}
	else if (up_len > down_len)
	{
		int up = 0;
		memcpy(&up, &(m_buffer[up_len - 2]), 2);
		int down = bi.m_buffer[down_len - 1];
		return up / down;
	}
	else if (up_len < down_len)
	{
		return 0;//assert(false);
	}
	return 0;
}


template <int MAX_BYTE_COUNT>
int big_int<MAX_BYTE_COUNT>::compare(const big_int& bi)
{
	if (this->is_positive())
	{
		if (bi.is_positive())
			return _compare_value(bi);
		else if (bi.is_negative() || bi.is_zero())
			return 1;
		else
			assert(false);
	}
	else if (this->is_negative())
	{
		if (bi.is_positive() || bi.is_zero())
			return -1;
		else if (bi.is_negative())
			return -_compare_value(bi);
		else
			assert(false);
	}
	else if (this->is_zero())
	{
		if (bi.is_positive())
			return -1;
		else if (bi.is_negative())
			return 1;
		else if (bi.is_zero())
			return 0;
		else
			assert(false);
	}
	else
		assert(false);
}


template <int MAX_BYTE_COUNT>
int big_int<MAX_BYTE_COUNT>::_compare_value(const big_int& bi) const
{
	if (m_buffer == bi.m_buffer)
		return 0;

	for (int i = MAX_BYTE_COUNT - 1; i >= 0; --i)
	{
		int diff = m_buffer[i] - bi.m_buffer[i];
		if (diff)
			return diff / scl::absolute(diff);
	}
	return 0;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_mul_byte(byte b)
{
	if (b == 0)
		memset(m_buffer, 0, sizeof(m_buffer));

	int carry = 0;
	for (int i = 0; i < MAX_BYTE_COUNT; ++i)
	{
		int n = m_buffer[i];
		const int product = n * b + carry;
		if (product > 0xFF)
		{
			carry = product / 0x0100;
			m_buffer[i] = product - (carry * 0x0100);
		}
		else
		{
			m_buffer[i] = product;
			carry = 0;
		}
	}
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_shift_left(const int shift)
{
	if (shift == 0)
		return;

	for (int i = MAX_BYTE_COUNT - 1; i > shift - 1; --i)
		m_buffer[i] = m_buffer[i - shift];

	memset(m_buffer, 0, shift);
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_shift_right(const int shift)
{
	if (shift == 0)
		return;

	for (int i = 0; i < MAX_BYTE_COUNT - shift; ++i)
		m_buffer[i] = m_buffer[i + shift];

	memset(m_buffer + (MAX_BYTE_COUNT - 1 - shift), 0, shift);
}


template <int MAX_BYTE_COUNT>
int big_int<MAX_BYTE_COUNT>::_length() const
{
	for (int i = MAX_BYTE_COUNT - 1; i >= 0; --i)
		if (m_buffer[i] != 0)
			return i + 1;

	return 0;
}


template <int MAX_BYTE_COUNT>
void big_int<MAX_BYTE_COUNT>::_copy(big_int& dest, const int src_start, const int src_length)
{
	if (src_start < 0 || src_start >= MAX_BYTE_COUNT)
		return;

	int copy_length = src_length;
	if (src_length == -1)
		copy_length = _length() - src_start;

	if (src_start + copy_length >= MAX_BYTE_COUNT)
		copy_length = MAX_BYTE_COUNT - src_start;

	for (int i = src_start; i < src_start + copy_length; ++i)
		dest.m_buffer[i - src_start] = m_buffer[i];
}

template <int MAX_BYTE_COUNT>
bool scl::big_int<MAX_BYTE_COUNT>::is_zero() const 
{ 
	assert(m_sign >= -1 && m_sign <= 1);
	assert(_compare_value(zero) == scl::absolute(m_sign)); //校验value与value是否匹配
	return m_sign == 0;
}


template <int MAX_BYTE_COUNT>
bool scl::big_int<MAX_BYTE_COUNT>::is_positive() const 
{ 
	assert(m_sign >= -1 && m_sign <= 1);
	assert(_compare_value(zero) == scl::absolute(m_sign)); //校验value
	return m_sign == 1;
}


template <int MAX_BYTE_COUNT>
bool scl::big_int<MAX_BYTE_COUNT>::is_negative() const 
{ 
	assert(m_sign >= -1 && m_sign <= 1);
	assert(_compare_value(zero) == scl::absolute(m_sign)); //校验value
	return m_sign == -1;
}


template <int MAX_BYTE_COUNT>
void scl::big_int<MAX_BYTE_COUNT>::from(const char* const s)
{
	const char* p = s;
	clear();
	int sign = 1;
	while (char c = *p++)
	{
		if (!is_zero())
			_mul_value(10);

		if (scl::isdigit(c))
		{
			_add_value(c - '0');
			if (_compare_value(zero) != 0)
				m_sign = sign;
		}
		else	//非数字字符的处理
		{
			if (is_zero())
			{
				if (c == '-')
					sign = -sign;
				else
				{
					if (c != '+')
						assert(false);
				}
			}		
			else
				assert(false);
		}
	}
}


} //namespace scl



