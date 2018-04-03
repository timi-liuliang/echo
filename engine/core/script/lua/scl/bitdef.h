#pragma once

namespace scl {

#define div8(n) (((n) & 0xFFFFFFF8) >> 3)					//	div8(n) = n / 8;
#define mod8(n) ((n) & 0x00000007)						//	mode8(n) = n % 8
#define mod8_flag(n) (((n) & 0x00000007) == 0 ? 0 : 1)	//	mode_flag(n) = (n % 8 == 0 ? 0 : 1)

//bool _get_bit(const unsigned char& c, const int bit_index);
//void _set_bit(unsigned char& c, const int bit_index);
//void _reset_bit(unsigned char& c, const int bit_index);


//最左侧为1,其他位为0的char，用于移位
static const unsigned char ONE = 0x80;

//获得某个char中的pos位置上的bit置,返回true表示1，false表示2
//注意！pos是从char左侧开始计算的索引
inline bool _get_bit(const unsigned char& c, const int bit_index)
{
	unsigned char v = (c & (ONE >> bit_index));
	return (v != 0);
}

//设置某个char中的pos位置上的bit为1
//注意！pos是从char左侧开始计算的索引
inline void _set_bit(unsigned char& c, const int bit_index)
{
	c = (c  | (ONE >> bit_index));
}

//设置某个char中的pos位置上的bit为0
//注意！pos是从char左侧开始计算的索引
inline void _reset_bit(unsigned char& c, const int bit_index)
{
	c = (c  & (~(ONE >> bit_index)));
}

//反转某个char中的pos位置上的bit
//注意！pos是从char左侧开始计算的索引
inline void _flip_bit(unsigned char& c, const int bit_index)
{
	if (_get_bit(c, bit_index))
		_reset_bit(c, bit_index);
	else
		_set_bit(c, bit_index);
}

} //namespace scl

