////////////////////////////////////////////////////////////////////////////////
//	string define
//	2010.11.17 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/assert.h"

////////////////////////////////////////////////////////////////////////
//	_SCL_ENCODING_GBK_ 说明：
//		_SCL_ENCODING_GBK_这个宏决定了string和pstring的from_ansi和to_ansi函数的行为
//
//		如果定义了_SCL_ENCODING_GBK_
//		那么from_ansi和to_ansi将会把wchar*和gbk编码的char*相互转换
//
//		如果没有定义_SCL_ENCODING_GBK_，
//		那么from_ansi和to_ansi将会把wchar*和utf8编码的char*相互转换
////////////////////////////////////////////////////////////////////////

//	#define _SCL_ENCODING_GBK_


namespace scl {

#ifdef SCL_WIN

#define scl_strncasecmp		_strnicmp
#define scl_strcasecmp		_stricmp
#define scl_strtoi64		_strtoi64
#define scl_strtoui64		_strtoui64
#define scl_strtok			strtok_s
#define	scl_wcsncasecmp		_wcsnicmp
#define	scl_wcstoi64		_wcstoi64
#define scl_wcstoui64		_wcstoui64
#define scl_wcstok			wcstok_s
#define scl_snprintf		scl::_snprintf

//win32下，直接使用_snprintf会导致warning，所以这里间接调用一下
int _snprintf(
	char*		buffer,
	int			count,
	const char*	format,
	...);

#define SCL_STR_FORMAT_I64	"%I64d"
#define SCL_STR_FORMAT_UI64	"%I64u"
#define SCL_STR_NEW_LINE	"\r\n"
#define SCL_WCS_FORMAT_I64	L"%I64d"
#define SCL_WCS_FORMAT_UI64	L"%I64u"
#define SCL_WCS_NEW_LINE	L"\r\n"

#endif

#ifdef SCL_ANDROID
int					wcsncasecmp	(const wchar_t *, const wchar_t *, int);
unsigned long long	wcstoull	(const wchar_t* s, wchar_t** end, int base);
long long			wcstoll		(const wchar_t* s, wchar_t** end, int base);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)

#define scl_strncasecmp		strncasecmp
#define scl_strcasecmp		strcasecmp
#define scl_strtoi64		strtoll
#define scl_strtoui64		strtoull
#define scl_strtok			strtok_r
#define scl_wcstok			wcstok
#define scl_snprintf		snprintf

#define SCL_STR_FORMAT_I64	"%lld"
#define SCL_STR_FORMAT_UI64 "%llu"
#define SCL_STR_NEW_LINE	"\n"
#define SCL_WCS_FORMAT_I64	L"%lld"
#define SCL_WCS_FORMAT_UI64	L"%llu"
#define SCL_WCS_NEW_LINE	L"\n"

#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_HTML5)
#define scl_wcsncasecmp		wcsncasecmp
#define scl_wcstoi64		wcstoll
#define scl_wcstoui64		wcstoull
#endif

#if defined(SCL_ANDROID)
#define scl_wcsncasecmp		scl::wcsncasecmp
#define scl_wcstoi64		scl::wcstoll
#define scl_wcstoui64		scl::wcstoull
#endif

#define scl_strtohex(s) ::strtoul(s, NULL, 16)

void			strcpy			(char* dest, const int max_size, const char* const src);
void			strncpy			(char* dest, const int max_size, const char* const src, const int copy_count);
int				sprintf			(char* buff, const int bufflen, const char* const format, ...);
double			strtod			(const char* const s);
float			strtof			(const char* const s, float _default = 0);
unsigned long	strtoul			(const char* s, char** end, int base);
long			strtol			(const char* s, char** end, int base);

////////////////////////////////////
//	Encoding function
//	implemented in pwstring.cpp
////////////////////////////////////
enum Encoding
{
	Encoding_GBK,
	Encoding_UTF8,
	Encoding_UTF16,
};

int wchar_to_ansi(
	char*			ansi, 
	const int		ansi_byte_size, 
	const wchar*	wide_string, 
	const int		wide_char_count,
	Encoding		ansi_encoding);

int ansi_to_wchar(
	wchar*			wide_string, 
	const int		wide_char_count,
	const char*		ansi, 
	const int		ansi_byte_size, 
	Encoding		ansi_encoding);

bool isalpha(char c);
bool iswalpha(wchar c);

bool isdigit(char c);
bool iswdigit(wchar c);

inline bool isspace(char c)
{
	//space char = 0x09 – 0x0D or 0x20
	if (c >= 0x09 && c <= 0x0D) return true;
	if (c == 0x20) return true;
	return false;
}


inline bool iswspace(wchar c)
{
	//space char = 0x09 – 0x0D or 0x20
	if (c >= 0x09 && c <= 0x0D) return true;
	if (c == 0x20) return true;
	return false;
}

int trim		(char* s);
int trim_left	(char* s, const int l = -1);
int trim_right	(char* s, const int l = -1);

////////////////////////////////////////////////////////////////////////
// load_string_to_matrix
//
//		功能是将一个形如"1,2,3,4 \n 5,6,7,8 "的字符串转换为一个二维数组
//		二维数组的第一行包含4个整数 1 2 3 4
//		二维数组的第二行包含4个整数 5 6 7 6
//
// 参数：
//		str					待转换的字符串
//		str_len				待转换的字符串长度
//		seperator			分隔符，该参数直接传递给strtok函数
//		output_array		输出数组的指针
//		output_capacity		输出数组的最大长度
//		output_row_count	返回值，返回输出一共有多少行，即二维数组的高度
//		output_column_count	返回值，返回输出一共有多少列，即二维数组的宽度
//
////////////////////////////////////////////////////////////////////////
bool load_string_to_matrix(
	char*		str, 
	const int	str_len, 
	const char*	seperator,
	uint16*		output_array, 
	const int	output_capacity,
	uint16*		output_height,
	uint16*		output_width);

////////////////////////////////////////////////////////////////////////
// load_string_to_array
//
//		功能是将一个形如"1,2,3,4"的字符串转换为一个int数组
//
// 参数：
//		str					待转换的字符串
//		str_len				待转换的字符串长度
//		seperator			分隔符，该参数直接传递给strtok函数
//		output_array		输出数组的指针
//		output_capacity		输出数组的最大长度
//		output_length		返回值，返回输出一共有多少个int
//
////////////////////////////////////////////////////////////////////////
bool load_string_to_array(
	char*		str, 
	const int	str_len, 
	const char*	seperator,
	int*		output_array, 
	const int	output_capacity,
	uint16*		output_length = NULL);

////////////////////////////////////////////////////////////////////////
// string_to_float_array
//
//		功能是将一个形如"1.2 , 0.33 , 3.14f , 4"的字符串转换为一个float数组
//		分隔符为任意"不属于构成float的字符"的字符, 例如" 1.001, { 2.33 } [ 3.14f ]" 也可以
//
// 参数：
//		s					待转换的字符串
//		out					输出的double数组
//		out_max				输出数组的最大长度
////////////////////////////////////////////////////////////////////////
void string_to_float_array(char* s, double* out, const int out_max);
void string_to_uint_array(char* s, int base, unsigned int* out, const int out_max);

//将形如TypeName的字符串转换为TYPE_NAME的形式
void string_camel_to_all_upper(const char* const camel, char* const allupper, const int allupper_capacity, bool ignoreNumber = true, bool disableAdjacentUnderscore = true);

} //namespace scl


#ifdef SCL_ANDROID
inline int wcsnlen(const wchar_t *wcs, int maxsize)
{
    int n;
    for (n = 0; n < maxsize && *wcs; n++, wcs++);
    return n;
}
#endif


