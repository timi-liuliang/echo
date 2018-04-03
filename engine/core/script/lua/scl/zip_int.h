#pragma once

#include "scl/type.h"
#include "scl/assert.h"

#ifdef SCL_WIN 
#include <Intrin.h>
#endif


namespace scl { 

//	只有第1个字节有head，其他字节均为数据本身信息
//	有符号数用zigzag转为无符号数后编码
//	1byte	0xxx xxxx	
//	2bytes	10xx  xxxx | xxxx xxxx
//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
//	5bytes	1111  0000 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx

//zip_xxx函数的参数说明：
//	byte* p	是目标缓冲区，压缩后将写入这个缓冲区
//	uint i	是等待压缩的值
//返回值是压缩后写入到 byte* p中的字节数
inline int	zip_int				(byte* p, int	i	);
inline int	zip_uint			(byte* p, uint	i	);
inline int	zip_int16			(byte* p, int16	i	);
inline int	zip_uint16			(byte* p, int16	i	);
inline int	zip_int64			(byte* p, uint64 i	);
inline int	zip_uint64			(byte* p, uint64 i	);
inline int	zip_int_size		(int i				);
inline int	zip_uint_size		(uint i				);
inline int	zip_int16_size		(int i				);
inline int	zip_uint16_size		(uint16 i			);
inline int	zip_int64_size		(byte* p, uint64 i	);
inline int	zip_uint64_size		(byte* p, uint64 i	);


//unzip_xxx函数的参数说明：
//	byte* p	是读取的缓冲区，从这个缓冲区读取一段字节流，然后解压缩
//	uint& i	是解压缩结果存入的值
//返回值是本次解压缩过程从 byte* p中读取的字节数
inline int	unzip_int			(byte* p, int& i);
inline int	unzip_uint			(byte* p, uint& i);
inline int	unzip_int16			(byte* p, int& i);
inline int	unzip_uint16		(byte* p, uint& i);
inline int	unzip_int64			(byte* p, uint64& i);
inline int	unzip_uint64		(byte* p, uint64& i);
inline int	unzip_int_size		(byte* p);
inline int	unzip_uint_size		(byte* p);
inline int	unzip_int16_size	(byte* p);
inline int	unzip_uint16_size	(byte* p);
inline int	unzip_int64_size	(byte* p);
inline int	unzip_uint64_size	(byte* p);



////////////////////////////////////
// implemente
////////////////////////////////////
inline uint zigzag(int c) //move the sig from msb to lsb
{
	return (c << 1) ^ (c >> 31);	//return ((~c) << 1) | (uint(c) >> 31); 
}

inline uint zigzag16(int c) //move the sig from msb to lsb
{
	return (c << 1) ^ (c >> 15);	//return ((~c) << 1) | (uint(c) >> 15); 
}

inline uint64 zigzag64(int64 c) //move the sig from msb to lsb
{
	return (c << 1) ^ (c >> 63);	//return ((~c) << 1) | (uint(c) >> 63); 
}


inline int unzigzag(uint c)
{
	if (c & 1)
		return (c >> 1) ^ 0xFFFFFFFF;
	else
		return (c >> 1);
}

inline int unzigzag16(uint c)
{
	if (c & 1)
		return (c >> 1) ^ 0xFFFF;
	else
		return (c >> 1);
}

inline int64 unzigzag64(uint64 c)
{
	if (c & 1)
		return (c >> 1) ^ 0xFFFFFFFFFFFFFFFF;
	else
		return (c >> 1);
}

#ifdef SCL_LINUX 
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif

//16位字节序调换
inline uint16 byteorder16(uint16 s)
{;
#ifdef SCL_WIN 
	_asm 
	{
		mov		ax, s
		xchg	ah,al
	}
#endif

#ifdef SCL_LINUX
	__asm__ __volatile__ 
	(	
		"mov	%0, %%ax\n"
		"xchg	%%ah, %%al\n"
		: : "r"(s) : "%eax" 
	);
#endif

	//TODO apple and android asm
#if defined(SCL_APPLE) || defined(SCL_ANDROID)
	return (s << 8) | (s >> 8);
#endif
}

//32位字节序调换
inline uint byteorder32(uint s)
{
#ifdef SCL_WIN
	_asm 
	{
		mov		eax, s
		bswap	eax	
	}
#endif

#ifdef SCL_LINUX
	__asm__ __volatile__ 
	(	
		"mov	%0, %%eax\n"
		"bswap	%%eax\n"
		: : "r"(s) : "%eax" 
	);
#endif
	//TODO apple and android asm
#if defined(SCL_APPLE) || defined(SCL_ANDROID)
	return (s << 24) | ((s << 8) & 0x00FF0000) | ((s >> 8) & 0x0000ff00) | (s >> 24);
#endif
}

//64位字节序调换
inline uint64 byteorder64(uint64 x)
{
#ifdef SCL_LINUX64
	__asm__ __volatile__ 
	(	
		"movq	%0, %%rax\n"
		"bswapq	%%rax\n"
		: : "r"(x) : "%rax" 
	);
#endif

	//TODO apple and android asm
#if defined(SCL_LINUX32) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return	(((uint64)(x) << 56) |
		(((uint64)(x) << 40) & 0xff000000000000ULL) |
		(((uint64)(x) << 24) & 0xff0000000000ULL) |
		(((uint64)(x) << 8) & 0xff00000000ULL) |
		(((uint64)(x) >> 8) & 0xff000000ULL) |
		(((uint64)(x) >> 24) & 0xff0000ULL) |
		(((uint64)(x) >> 40) & 0xff00ULL) |
		((uint64)(x) >> 56));
#endif

#ifdef SCL_WIN
	return	_byteswap_uint64(x);
#endif
}

#ifdef SCL_LINUX 
#pragma GCC diagnostic warning "-Wreturn-type"
#endif


inline int zip_uint_size(uint i)
{
	if (i < 0x80)				//	1byte	0xxx xxxx	
		return 1;
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
		return 2;
	else if (i < 0x200000)		//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
		return 3;
	else if (i < 0x10000000)
		return 4;
	else						//	5bytes	1111  0000 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 5;
}


inline int zip_uint16_size(uint16 i)
{
	if (i < 0x80)				//	1byte	0xxx xxxx	
		return 1;
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
		return 2;
	else						//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
		return 3;
}

inline int zip_uint(byte* p, uint i)
{
	if (!p) return 0;
	if (i < 0x80)				//	1byte	0xxx xxxx	
	{
		*p = (byte)i;
		return 1;
	}
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
	{
		*(uint16*)p = (byteorder16((uint16)i) | 0x80);	//byteorder(0x8000) = 0x0080
		return 2;
	}
	else if (i < 0x200000)		//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
	{
		uint i32 = (byteorder32(i) | 0xC000); //byteorder(0x00C00000) = 0xC000
		*p					= byte(i32 >> 8);
		*(uint16*)(p + 1)	= uint16(i32 >> 16);
		//*(p + 1)	= byte(i32 >> 16);
		//*(p + 2)	= byte(i32 >> 24);
		return 3;
	}
	else if (i < 0x10000000)	//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		*(uint*)p = (byteorder32(i) | 0xE0); //byteorder(0xE0000000) = 0x000000E0
		return 4;
	}
	else						//	5bytes	1111  0000 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		*p = 0xF0;
		*(uint*)(p + 1) = byteorder32(i);
		return 5;
	}
}


inline int zip_uint16(byte* p, uint16 i)
{
	if (!p) return 0;
	if (i < 0x80)				//	1byte	0xxx xxxx	
	{
		*p = (byte)i;
		return 1;
	}
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
	{
		*(uint16*)p = (byteorder16((uint16)i) | 0x80);	//byteorder(0x8000) = 0x0080
		return 2;
	}
	else
	{
		*p = 0xC0;
		*(uint16*)(p + 1)	= byteorder16(i);
		return 3;
	}
}

inline int zip_uint64(byte* p, uint64 i)
{
	if (!p) return 0;
	if (i < 0x80)				//	1byte	0xxx xxxx	
	{
		*p = (byte)i;
		return 1;
	}
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
	{
		*(uint16*)p			= (byteorder16((uint16)i) | 0x80);	//byteorder(0x8000) = 0x0080
		return 2;
	}
	else if (i < 0x200000)		//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
	{
		uint i32			= (byteorder32((uint32)i) | 0xC000); //byteorder(0x00C00000) = 0xC000
		*p					= byte(i32 >> 8);
		*(uint16*)(p + 1)	= uint16(i32 >> 16);
		return 3;
	}
	else if (i < 0x10000000)	//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		*(uint*)p			= (byteorder32((uint32)i) | 0xE0); //byteorder(0xE0000000) = 0x000000E0
		return 4;
	}
	else if (i < 0x0800000000ULL) // 5bytes 1111  0xxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		uint64 i64			=	(byteorder64(i) | 0xF0000000ULL); //byteorder64(0xF000000000) = 0xF0000000
		*p					=	byte(i64 >> 24);
		*(uint32*)(p + 1)	=	uint32(i64 >> 32);
		return 5;
	}
	else if (i < 0x040000000000ULL)	// 6bytes 1111  10xx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		uint64 i64			= (byteorder64(i) | 0xF80000ULL); //byteorder64(0xF80000000000) = 0xF80000
		*(uint16*)p			= uint16(i64 >> 16);
		*(uint32*)(p + 2)	= uint32(i64 >> 32);
		return 6;
	}
	else if (i < 0x02000000000000ULL) // 7bytes 1111  110x | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		uint64 i64			= (byteorder64(i) |  0xFC00ULL); //byteorder64(0xFC000000000000) = 0xFC00
		*p					= byte(i64 >> 8);
		*(uint16*)(p + 1)	= uint16(i64 >> 16);
		*(uint32*)(p + 3)	= uint32(i64 >> 32);
		return 7;
	}
	else if (i < 0x0100000000000000ULL) //8bytes 1111  1110 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{
		*(uint64*)p			= (byteorder64(i) | 0xFEULL);
		return 8;
	}
	else if (i < 0x8000000000000000ULL) //9bytes 1111  1111 | 0xxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{	
		*p					= 0xFF;
		*(uint64*)(p + 1)	= byteorder64(i);
		return 9;
	}
	else	//10bytes 1111  1111 | 10xx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
	{	
		*(uint16*)p			= 0x80FF;
		*(uint64*)(p + 2)	= byteorder64(i);
		return 10;
	}
}


inline int zip_uint64_size(uint64 i)
{
	if (i < 0x80)				//	1byte	0xxx xxxx	
		return 1;
	else if (i < 0x4000)		//	2bytes	10xx  xxxx | xxxx xxxx
		return 2;
	else if (i < 0x200000)		//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
		return 3;
	else if (i < 0x10000000)	//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 4;
	else if (i < 0x0800000000ULL) // 5bytes 1111  0xxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 5;
	else if (i < 0x040000000000ULL)	// 6bytes 1111  10xx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 6;
	else if (i < 0x02000000000000ULL) // 7bytes 1111  110x | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 7;
	else if (i < 0x0100000000000000ULL) //8bytes 1111  1110 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 8;
	else if (i < 0x8000000000000000ULL) //9bytes 1111  1111 | 0xxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 9;
	else	//10bytes 1111  1111 | 10xx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		return 10;
}

inline int zip_int(byte* p, int i)
{
	uint z = zigzag(i);
	return zip_uint(p, z);
}

inline int zip_int16(byte* p, int16 i)
{
	uint16 z = zigzag16(i);
	return zip_uint16(p, z);
}


inline int zip_int64(byte* p, int64 i)
{
	uint64 z = zigzag64(i);
	return zip_uint64(p, z);
}


inline int zip_int_size(int i)
{
	uint z = zigzag(i);
	return zip_uint_size(z);
}

inline int zip_int16_size(int16 i)
{
	uint16 z = zigzag16(i);
	return zip_uint16_size(z);
}


inline int zip_int64_size(int64 i)
{
	uint64 z = zigzag64(i);
	return zip_uint64_size(z);
}

inline int unzip_uint(byte* p, uint& i)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
		{
			i = *p;
			return 1;
		}
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
		{
			uint16 c16 = *(uint16*)p;
			i = byteorder16(c16) & 0x3FFF;
			return 2;
		}
	case 0xC0: case 0xD0:	//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
		{
			uint16 high	= (*p & 0x1F);
			uint16 low	=  byteorder16(*(uint16*)(p + 1));
			i = (high << 16) + low;
			return 3;
		}
	case 0xE0:				//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		{
			i = byteorder32((*(uint*)p)) & 0x0FFFFFFF;
			return 4;
		}
	case 0xF0:				//	5bytes	1111  0000 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		{
			i = byteorder32(*((uint*)(p + 1)));
			return 5;
		}
	default: 
		{
			assert(false);
		}
		return 0;
	}
}

inline int unzip_uint16(byte* p, uint16& i)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
		{
			i = *p;
			return 1;
		}
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
		{
			uint16 c16 = *(uint16*)p;
			i = byteorder16(c16) & 0x3FFF;
			return 2;
		}
	case 0xC0:	//	3bytes	1100 0000 | xxxx xxxx | xxxx xxxx
		{
			i = byteorder16(*(uint16*)(p + 1));
			return 3;
		}
	default: 
		{
			assert(false);
		}
		return 0;
	}
}

inline int unzip_uint_size(byte* p)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
			return 1;
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
			return 2;
	case 0xC0: case 0xD0:	//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
			return 3;
	case 0xE0:				//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
			return 4;
	case 0xF0:				//	5bytes	1111  0000 | xxxx xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
			return 5;
	default: return 0;
	}
}


inline int unzip_uint16_size(byte* p)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
			return 1;
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
			return 2;
	case 0xC0:				//	3bytes	1100 0000 | xxxx xxxx | xxxx xxxx
			return 3;
	default: 
		{
			assert(false);
		}
		return 0;
	}
}

inline int unzip_int_size(byte* p)
{
	return unzip_uint_size(p);
}

inline int unzip_int16_size(byte* p)
{
	return unzip_uint16_size(p);
}

inline int unzip_int64_size(byte* p)
{
	return unzip_uint64_size(p);
}


inline int unzip_uint64(byte* p, uint64& i)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
		{
			i = *p;
			return 1;
		}
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
		{
			uint16 c16 = *(uint16*)p;
			i = byteorder16(c16) & 0x3FFF;
			return 2;
		}
	case 0xC0: case 0xD0:	//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
		{
			uint16 high	= (*p & 0x1F);
			uint16 low	=  byteorder16(*(uint16*)(p + 1));
			i = (high << 16) + low;
			return 3;
		}
	case 0xE0:				//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
		{
			i = byteorder32((*(uint*)p)) & 0x0FFFFFFF;
			return 4;
		}
	case 0xF0:			
		{
			switch (*p & 0x0F)
			{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: //	5byte	0xxx xxxx	
				{
					uint64 high = (*p & 0x07);
					uint64 low	= byteorder32(*(uint32*)(p + 1));
					i			= (high << 32) + low;
					return 5;
				}
			case 0x08: case 0x09: case 0x0A: case 0x0B:	//	6bytes	10xx  xxxx | xxxx xxxx
				{
					uint64 high	= byteorder16(*(uint16*)p) & 0x03FF;
					uint64 low	= byteorder32(*(uint32*)(p + 2));
					i			= (high << 32) + low;
					return 6;
				}
			case 0x0C: case 0x0D:	//	7bytes
				{
					uint64 high1 = *p & 0x01;
					uint64 high2 = byteorder16(*(uint16*)(p + 1));
					uint64 low	 = byteorder32(*(uint32*)(p + 3));
					i			 = (high1 << 48) + (high2 << 32) + low;
					return 7;
				}
			case 0x0E:		//	8bytes
				{
					i = byteorder64(*(uint64*)p) & 0x00FFFFFFFFFFFFFFULL;
					return 8;
				}
			case 0x0F:				
				{
					byte lead2 = *(p + 1);
					if (lead2 & 0x80)		//10 bytes
					{
						i = byteorder64(*(uint64*)(p + 2));
						return 10;
					}
					else					//9 bytes
					{
						i = byteorder64(*(uint64*)(p + 1));
						return 9;
					}
				}
			default : 
				{
					//assert(false);
					return 0;
				}
			}
		}
	default: 
		{
			//assert(false);
			return 0;
		}
	}
}



inline int unzip_uint64_size(byte* p)
{
	if (!p) return 0;
	switch (*p & 0xF0)
	{
	case 0x00: case 0x10: case 0x20: case 0x30: case 0x40: case 0x50: case 0x60: case 0x70: //	1byte	0xxx xxxx	
			return 1;
	case 0x80: case 0x90: case 0xA0: case 0xB0:	//	2bytes	10xx  xxxx | xxxx xxxx
			return 2;
	case 0xC0: case 0xD0:	//	3bytes	110x  xxxx | xxxx xxxx | xxxx xxxx
			return 3;
	case 0xE0:				//	4bytes	1110  xxxx | xxxx xxxx | xxxx xxxx | xxxx xxxx
			return 4;
	case 0xF0:			
		{
			switch (*p & 0x0F)
			{
			case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: //	5byte	0xxx xxxx	
					return 5;
			case 0x08: case 0x09: case 0x0A: case 0x0B:	//	6bytes	10xx  xxxx | xxxx xxxx
					return 6;
			case 0x0C: case 0x0D:	//	7bytes
					return 7;
			case 0x0E:		//	8bytes
					return 8;
			case 0x0F:				
				{
					byte lead2 = *(p + 1);
					if (lead2 & 0x80)		//10 bytes
						return 10;
					else					//9 bytes
						return 9;
				}
			default : 
				{
					//assert(false);
					return 0;
				}
			}
		}
	default: 
		{
			//assert(false);
			return 0;
		}
	}
}

inline int unzip_int(byte* p, int& i)
{
	uint z = 0;
	int count = unzip_uint(p, z);
	i = unzigzag(z);
	return count;
}

inline int unzip_int16(byte* p, int16& i)
{
	uint16 z = 0;
	int count = unzip_uint16(p, z);
	i = unzigzag16(z);
	return count;
}


inline int unzip_int64(byte* p, int64& i)
{
	uint64 z = 0;
	int count = unzip_uint64(p, z);
	i = unzigzag64(z);
	return count;
}


template <typename T>
class TypeParser
{
public:
	static int zip			(byte* p, T i	) { *reinterpret_cast<T*>(p) = i; return sizeof(T); }
	static int zip_size		(T i			) { return sizeof(T); }
	static int unzip		(byte* p, T& i	) { i = *reinterpret_cast<T*>(p); return sizeof(T); }
	static int unzip_size	(byte* p		) { return sizeof(T); }
};

template <>
class TypeParser<int>
{
public:
	static int zip			(byte* p, int i	) { return scl::zip_int(p, i);		}
	static int zip_size		(int i			) { return scl::zip_int_size(i);	}
	static int unzip		(byte* p, int& i) { return scl::unzip_int(p, i);	}
	static int unzip_size	(byte* p		) { return scl::unzip_int_size(p);	}
};

template <>
class TypeParser<uint>
{
public:
	static int zip			(byte* p, uint i	) { return scl::zip_uint(p, i);		}
	static int zip_size		(uint i				) { return scl::zip_uint_size(i);	}
	static int unzip		(byte* p, uint& i	) { return scl::unzip_uint(p, i);	}
	static int unzip_size	(byte* p			) { return scl::unzip_uint_size(p);	}
};

template <>
class TypeParser<int16>
{
public:
	static int zip			(byte* p, int16 i	) { return scl::zip_int16(p, i);		}
	static int zip_size		(int16 i			) { return scl::zip_int16_size(i);		}
	static int unzip		(byte* p, int16& i	) { return scl::unzip_int16(p, i);		}
	static int unzip_size	(byte* p			) { return scl::unzip_int16_size(p);	}
};

template <>
class TypeParser<uint16>
{
public:
	static int zip			(byte* p, uint16 i	) { return scl::zip_uint16(p, i);		}
	static int zip_size		(uint16 i			) { return scl::zip_uint16_size(i);		}
	static int unzip		(byte* p, uint16& i	) { return scl::unzip_uint16(p, i);		}
	static int unzip_size	(byte* p			) { return scl::unzip_uint16_size(p);	}
};

template <>
class TypeParser<int64>
{
public:
	static int zip			(byte* p, int64 i	) { return scl::zip_int64(p, i);		}
	static int zip_size		(int64 i			) { return scl::zip_int64_size(i);		}
	static int unzip		(byte* p, int64& i	) { return scl::unzip_int64(p, i);		}
	static int unzip_size	(byte* p			) { return scl::unzip_int64_size(p);	}
};

template <>
class TypeParser<uint64>
{
public:
	static int zip			(byte* p, uint64 i	) { return scl::zip_uint64(p, i);		}
	static int zip_size		(uint64 i			) { return scl::zip_uint64_size(i);		}
	static int unzip		(byte* p, uint64& i	) { return scl::unzip_uint64(p, i);		}
	static int unzip_size	(byte* p			) { return scl::unzip_uint64_size(p);	}
};

}	//namespace scl




