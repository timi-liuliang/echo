////////////////////////////////////////////////////////////////////////////////
//	string define
//	2010.11.17 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/stringdef.h"

#ifdef SCL_WIN
#include <windows.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <wctype.h>
#include <ctype.h>
#include <iconv.h>
#include <string.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <stdarg.h>
#include <stdio.h>

namespace scl {

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
static union { char c[2]; unsigned short a; } endian_test = { { 0, 1 } };
#define WCHAR_TYPE (((char)endian_test.a) ? "UTF-32BE" : "UTF-32LE")
#endif

#define SCL_IS_FLOAT_CHAR(c) ((c >= '0' && c <= '9') || c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')
#define STR_ISSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
#define IS_INT_NUM(c) ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == '-' || c == '+')
#define WCS_ISSPACE(c) ((c) == L' ' || (c) == L'\t' || (c) == L'\r' || (c) == L'\n')

static const signed char c2i_table[] = { 
//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,	//0	 ~ 15
//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,	//16 ~ 31
//-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,	//32 ~ 47
 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,		//48 ~ 63
-1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,		//64 ~ 79
25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,		//80 ~ 95
-1,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,		//96 ~ 111
25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1		//112 ~ 127							
};

int wchar_to_ansi(
	char*			ansi, 
	const int		ansi_byte_size, 
	const wchar*	wide_string, 
	const int		wide_char_count,
	Encoding		ansi_encoding)
{
#ifdef SCL_WIN
	int codePage = 0;
	switch (ansi_encoding)
	{
	case Encoding_GBK:
		codePage = 936;
		break;
	case Encoding_UTF8:
		codePage = 65001;
		break;
	default:
		assertf(false, "wchar_to_ansi : invalid ansi encoding [%d]", ansi_encoding);
		break;
	};

	return ::WideCharToMultiByte(
		codePage,					//CodePage, 				
		0,							//Flags, 
		wide_string,				//WideCharStr,
		wide_char_count,			//WideCharCount,  
		ansi,						//MultiByteStr, 
		ansi_byte_size - 1,			//MultiByteCount,
		NULL,						//DefaultChar,    
		NULL);						//UsedDefaultChar
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	iconv_t cd = reinterpret_cast<iconv_t>(-1);
	switch (ansi_encoding)
	{
	case Encoding_GBK:
		cd = iconv_open("GBK", WCHAR_TYPE);
		break;
	case Encoding_UTF8:
		cd = iconv_open("UTF-8", WCHAR_TYPE);
		break;
	default:
		assertf(false, "wchar_to_ansi : invalid ansi encoding [%d]", ansi_encoding);
		break;
	};
	
	if (cd == reinterpret_cast<iconv_t>(-1))
		return -1;

	size_t	sourceLength	= (-1 == wide_char_count) ? (wcslen(wide_string) * sizeof(wchar)) : (wide_char_count * sizeof(wchar));
	size_t	destLength		= ansi_byte_size - 1;
	char*	pSource 		= const_cast<char*>(reinterpret_cast<const char*>(wide_string));
	char*	pDest			= ansi;

	size_t convertResult = iconv(cd, &pSource, &sourceLength, &pDest, &destLength);

	iconv_close(cd);
	return static_cast<int>(convertResult);
#endif
}

int ansi_to_wchar(
	wchar*			wide_string, 
	const int		wide_char_count,
	const char*		ansi, 
	const int		ansi_byte_size, 
	Encoding		ansi_encoding)
{
#ifdef SCL_WIN

	int codePage = 0;
	switch (ansi_encoding)
	{
	case Encoding_GBK:
		codePage = 936;
		break;
	case Encoding_UTF8:
		codePage = 65001;
		break;
	default:
		assertf(false, "wchar_to_ansi : invalid ansi encoding [%d]", ansi_encoding);
		break;
	};

	return ::MultiByteToWideChar(
		codePage, 
		0, 
		ansi, 
		ansi_byte_size, 
		wide_string, 
		wide_char_count - 1);

#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	iconv_t cd = reinterpret_cast<iconv_t>(-1);
	switch (ansi_encoding)
	{
	case Encoding_GBK:
		cd = iconv_open(WCHAR_TYPE, "GBK");
		break;
	case Encoding_UTF8:
		cd = iconv_open(WCHAR_TYPE, "UTF-8");
		break;
	default:
		assertf(false, "wchar_to_ansi : invalid ansi encoding [%d]", ansi_encoding);
		break;
	};

	if (cd == reinterpret_cast<iconv_t>(-1))
		return -1;

	size_t	sourceLength	= (-1 == ansi_byte_size) ? strlen(ansi) : ansi_byte_size;
	size_t	destLength		= (wide_char_count - 1) * sizeof(wchar);
	char*	pSource 		= const_cast<char*>(ansi);
	char*	pDest			= reinterpret_cast<char*>(wide_string);
	
	size_t convertResult = iconv(cd, &pSource, &sourceLength, &pDest, &destLength);

	iconv_close(cd);
	return static_cast<int>(convertResult);
#endif
}

#ifdef SCL_WIN
int scl_snprintf(
	char*		buffer,
	int			count,
	const char*	format,
	...)
{
	va_list arg;
	va_start(arg, format);

#pragma warning (disable: 4996)
	int r = vsnprintf(buffer, count, format, arg);
#pragma warning (default: 4996)

	va_end(arg);

	return r;
}

#endif


bool isalpha(char c)
{
	if ((c >= 'A' && c <= 'Z') 
		|| (c >= 'a' && c <= 'z'))
	{
		return true;	
	}

	return false;
}

bool iswalpha(wchar c)
{
	if ((c >= 'A' && c <= 'Z') 
		|| (c >= 'a' && c <= 'z'))
	{
		return true;	
	}

	return false;
}

bool isdigit(char c)
{
	if (c >= '0' && c <= '9')
	{
		return true;
	}
	return false;
}

bool iswdigit(wchar c)
{
	if (c >= '0' && c <= '9')
	{
		return true;
	}
	return false;
}

int trim_left(char* s, const int len)		
{ 
	int l = 0;
	if (len > 0) l = len;
	else l = static_cast<int>(strlen(s));

	//清除左侧的空字符
	int del = 0;
	for (int i = 0; i < l; ++i)
	{
		if (!scl::isspace(s[i]))
			break;
		del++;
	}
	//删除所有空字符
	for (int i = 0; i < l - del; ++i)
		s[i] = s[i + del];
	s[l - del] = 0;
	return del;
}

int trim_right(char* s, const int len)
{ 
	int l = 0;
	if (len > 0) l = len;
	else l = static_cast<int>(strlen(s));

	int del = 0;
	for (int i = l - 1; i > 0; --i)
	{
		if (!scl::isspace(s[i]))
			break;

		s[i] = 0;
		del++;
	}
	return del;
}


int trim(char* s)			
{ 
	int l		= static_cast<int>(strlen(s));
	int r_del	= trim_right(s, l);
	int l_del	= trim_left	(s, l);
	return r_del + l_del;
}

bool load_string_to_matrix(char* str, const int str_len, const char* seperator, uint16* output_array, const int output_capacity, uint16* output_row_count, uint16* output_column_count)
{
	int row = 0;
	int column_count = 0;

	//拆分成行
	char* lineContext = NULL;
	char* line = scl_strtok(str, "\n", &lineContext);
	while (NULL != line)
	{
		//解析行
		scl::trim(line);
		if (line[0] == 0)
		{
			line = scl_strtok(NULL, "\n", &lineContext);
			continue;
		}
		int column = 0;

		//拆分成字段
		char* numContext = NULL;
		char* num = scl_strtok(line, seperator, &numContext);
		while (NULL != num)
		{
			//解析字段
			if (column + row * column_count >= output_capacity)
				return false;
			output_array[column + row * column_count] = static_cast<uint16>(strtol(num, NULL, 10));
			num = scl_strtok(NULL, seperator, &numContext);
			++column;
		}
		if (column_count == 0)		//用第一行的列数作为解析的列数 
			column_count = column;
		if (column != column_count) //某些行的列数不一致，停止解析
			return false;
		line = scl_strtok(NULL, "\n", &lineContext);
		++row;
	}
	if (NULL != output_row_count)
		*output_row_count = row;
	if (NULL != output_column_count)
		*output_column_count = column_count;
	return true;
}


bool load_string_to_array(
	char*	str, 
	const int	str_len, 
	const char*	seperator,
	int*		output_array, 
	const int	output_capacity,
	uint16*		output_length)
{
	//解析行
	char* line = str;
	scl::trim(line);
	int column = 0;
	//拆分成字段
	char* numContext = NULL;
	char* num = scl_strtok(line, seperator, &numContext);
	while (NULL != num)
	{
		//解析字段
		if (column  >= output_capacity)
			return false;
		output_array[column] = static_cast<int>(strtol(num, NULL, 10));
		num = scl_strtok(NULL, seperator, &numContext);
		++column;
	}

	if (NULL != output_length)
		*output_length = column;
	return true;
}

void string_camel_to_all_upper(const char* const camel, char* const allupper, const int allupper_capacity, bool ignoreNumber, bool disableAdjacentUnderscore)
{
	const int len = static_cast<const int>(strlen(camel));
	int j = 0;
	for (int i = 0; i < len; ++i)
	{
		if (j >= allupper_capacity - 1)
			break;
		char c = camel[i];

		if (i > 0)
		{
			bool number = !ignoreNumber && isdigit(c);
			bool prev_is_underscore = camel[i - 1] == '_' && disableAdjacentUnderscore;
			if ((isupper(c) || number) && !prev_is_underscore)
				allupper[j++] = '_';
		}
		allupper[j++] = toupper(c);
	}
	allupper[j++] = 0;
}



#ifdef SCL_ANDROID
int wcsncasecmp(const wchar_t *s1, const wchar_t *s2, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        wchar_t wc1 = towlower(s1[i]);
        wchar_t wc2 = towlower(s2[i]);
        if (wc1 != wc2)
            return (wc1 > wc2) ? +1 : -1;
    }
    return 0;
}




unsigned long long wcstoull(const wchar_t* s, wchar_t** end, int base)
{
	//TODO check overflow
	// parse string to long long
	// string format : [whitespaces][{+|-}][0[{x|X}]][digits]
	const wchar_t* p = s;
	int flag = 1;
	unsigned long long n = 0;

	//[whitespaces]
	while (*p && WCS_ISSPACE(*p))
		++p;

	//[{+|-}]
	if (*p == L'+' || *p == L'-')
	{
		flag = (*p == L'-') ? -1 : 1;
		++p;
	}

	//[0[{x|X}]]
	if (base <= 1)
	{
		base = 10;
		if (*p == L'0')
		{
			base = 8;
			const wchar_t nc = *(p + 1);
			if (nc == L'x' || nc == L'X')
			{
				base = 16;
				++p;
			}
			++p;
		}
	}

	//[digits]
	if (!(base & base - 1))  //base is power of 2
	{
		int bs = "\0\1\2\4\7\3\6\5"[(0x17*base)>>5&7]; //bs = log2(base)
		while (*p && !WCS_ISSPACE(*p))
		{
			int i = (*p & 0x7F) - L'0';
			if (i < 0) i = 0;
			int digit = c2i_table[i];
			if (digit >= 0 && digit < base) // TODO check overflow : n <= (LLONG_MAX - digit) / base
				n = (n << bs) + digit;
			++p;
		}
	}
	else
	{
		while (*p && !WCS_ISSPACE(*p))
		{
			int i = (*p & 0x7F) - L'0';
			if (i < 0) i = 0;
			int digit = c2i_table[i];
			if (digit >= 0 && digit < base) // TODO check overflow : n <= (LLONG_MAX - digit) / base
				n = n * base + digit;
			++p;
		}
	}
	n = n * flag;
	if (NULL != end)
		*end = (wchar_t*)p;
	return n;
}

long long wcstoll(const wchar_t *s, wchar_t** end, int base)
{
	return wcstoull(s, end, base);
}


#endif //SCL_ANDROID

void strcpy(char* dest, const int max_size, const char* const src)
{
#ifdef SCL_WIN 
	strcpy_s(dest, max_size, src);
#else
    ::strncpy(dest, src, max_size);
#endif
	dest[max_size - 1] = 0;
}

void strncpy(char* dest, const int max_size, const char* const src, const int copy_count)
{
#ifdef SCL_WIN 
	strncpy_s(dest, max_size, src, copy_count);
#else
	int len = copy_count > max_size - 1 ? max_size - 1 : copy_count;
    ::strncpy(dest, src, len);
#endif
	dest[max_size - 1] = 0;
}

int sprintf(char* buff, const int bufflen, const char* const format, ...)
{
	va_list arg;
	va_start(arg, format);
	int r = 0;
#ifdef SCL_WIN 
	r = vsprintf_s(buff, bufflen, format, arg);
#else
	r = vsnprintf(buff, bufflen, format, arg);
#endif
	va_end(arg);

	buff[bufflen - 1] = 0;
	return r;
}



double strtod(const char* const s)
{
	//parse string "[-]ddd.ddd[e+-ddd]" to double

	const char*	p		= s;
	double		r		= 0;
	int			flag	= 0;	// 0 = parsing interger, 1 = parsing decimal, 2 = parsing exponent
	double		dec		= 10;	// decimal count
	int			exp		= 0;	// exponent
	int			exp_sign= 1;	// exponent sign

	while (const char c = *p++)
	{
		if (c >= '0' && c <= '9')		
		{
			if (flag == 0)
				r = r * 10 + (c - '0');
			else if (flag == 1)
			{
				r = r + (c - '0') / dec;
				dec *= 10;
			}
			else if (flag == 2)
			{
				exp = exp * 10 + (c - '0');
			}
		}
		else if (c == '.')
			flag = 1;
		else if (c == 'e' || c == 'E')
		{
			if (*p++ == '-')
				exp_sign = -1;
			flag = 2;
		}
	}

	if (s[0] == '-')
		r = -r;

	while (exp--)
	{
		if (exp_sign == 1)
			r = r * 10;
		else
			r = r / 10;
	}

	return r;
}


void string_to_float_array(char* s, double* out, const int out_max)
{
	if (NULL == s || 0 == s[0])
		return;
	int			out_index	= 0;
	const int	l			= static_cast<int>(strlen(s));
	int			current		= 0;
	bool		parsing		= false;
	for (int i = 0; i < l; ++i)
	{
		char& c = s[i];
		if (!parsing)
		{
			if (SCL_IS_FLOAT_CHAR(c))
			{
				parsing = true;
				current = i;
			}
		}
		else
		{
			if (!SCL_IS_FLOAT_CHAR(c))
			{
				parsing = false;
				c = 0;
				if (out_index >= out_max)
				{
					assert(false);
					return;
				}
				out[out_index++] = scl::strtof(&s[current]);
			}
		}
	}
}


unsigned long strtoul(const char* s, char** end, int base)
{
	return static_cast<unsigned long>(strtol(s, end, base));
}

long strtol(const char* s, char** end, int base)
{
	//TODO check overflow
	// parse string to long long
	// string format : [whitespaces][{+|-}][0[{x|X}]][digits]
	const char*		p		= s;
	int				flag	= 1;
	long			n		= 0;

	//[whitespaces]
	while (*p && STR_ISSPACE(*p))
		++p;

	//[{+|-}]
	if (*p == '+' || *p == '-')
	{
		flag = (*p == '-') ? -1 : 1;
		++p;
	}

	//[0[{x|X}]]
	if (base <= 1)
	{
		base = 10;
		if (*p == '0')
		{
			base = 8;
			const char nc = *(p + 1);
			if (nc == 'x' || nc == 'X')
			{
				base = 16;
				++p;
			}
			++p;
		}
	}

	//[digits]
	if (!(base & base - 1))  //base is power of 2
	{
		int bs = "\0\1\2\4\7\3\6\5"[(0x17*base)>>5&7]; //bs = log2(base)
		while (*p && !STR_ISSPACE(*p))
		{
			int i = (*p & 0x7F) - '0';
			if (i < 0) i = 0;
			int digit = c2i_table[i];
			if (digit >= 0 && digit < base) // TODO check overflow : n <= (LLONG_MAX - digit) / base
				n = (n << bs) + digit;
			++p;
		}
	}
	else
	{
		while (*p && !STR_ISSPACE(*p))
		{
			int i = (*p & 0x7F) - '0';
			if (i < 0) i = 0;
			int digit = c2i_table[i];
			if (digit >= 0 && digit < base) // TODO check overflow : n <= (LLONG_MAX - digit) / base
				n = n * base + digit;
			++p;
		}
	}
	n = n * flag;
	if (NULL != end)
		*end = (char*)p;
	return n;
}


void string_to_uint_array(char* s, int base, unsigned int* out, const int out_max)
{
	int			out_index	= 0;
	const int	l			= static_cast<int>(strlen(s));
	int			current		= 0;
	bool		parsing		= false;
	for (int i = 0; i < l; ++i)
	{
		char& c = s[i];
		if (!parsing)
		{
			if (IS_INT_NUM(c))
			{
				parsing = true;
				current = i;
			}
		}
		else
		{
			if (!IS_INT_NUM(c) || i == l - 1)
			{
				parsing = false;
				if (!IS_INT_NUM(c)) 
					c = 0;
				if (out_index >= out_max)
				{
					assert(false);
					return;
				}
				out[out_index++] = static_cast<unsigned int>(scl::strtoul(&s[current], NULL, base));
			}
		}
	}
}

float strtof(const char* const s, float _default)
{
	if (NULL == s || 0 == s[0])
		return _default;
	else
		return static_cast<float>(strtod(s));
}


} //namespace scl


