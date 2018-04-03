////////////////////////////////////////////////////////////////////////////////
//	string (a wrap of char[N])
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/stringdef.h"
#include "scl/pstring.h"

#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

namespace scl {

template<int MAX_COUNT>
class string
{
public:
	string()						{ _init(); }
	string(const char* s)			{ _init(); copy(s); }

	void 	copy		(const char* const s);
	void 	copy		(const char* const s, const int count);
	void	memcpy		(const void* p, const int length);
	void    substract	(const int n);
	void 	append		(const char* s);
	void 	append		(const char* s, const int count);
	void 	append		(const char	c);
	int 	compare		(const char* s, bool ignoreCase = false) const	{ return compare(s, max_size(), ignoreCase); }
	int		compare		(const char* const source, const int length, bool ignoreCase = false) const;
	//int 	compare		(const string& source, bool ignoreCase = false) const { return compare(source.c_str(), ignoreCase);  }
	void	erase		(const int startIndex = 0, const int length = -1);
	void	clear		()	{ ::memset(m_string, 0, MAX_COUNT); }
	int		format			(const char* const format, ...);	//返回新写入的字符数，不包含'\0'。当越界时，windows下返回-1，linux下返回“假如没有越界，应当写入的总字符数”
	int		format_append	(const char* const format, ...);
	int		format_arg		(const char* const format, va_list arg);
	int		format_arg_append(const char* const format, va_list arg);
	int		find			(const char c, const int startIndex = 0)			const	{ return find_first_of(c, startIndex); }
	int		find			(const char* const s, const int startIndex = 0)	const	{ return find_first_of(s, startIndex); }
	int		find_first_of	(const char c, const int startIndex = 0)			const;	//find_first_of 返回找到的字符串的起始index，没找到返回-1
	int		find_first_of	(const char* const s, const int startIndex = 0)	const;
	int		find_last_of	(const char c)			const;
	int		find_last_of	(const char* const s)	const;
	bool	contains		(const char c)			const;
	bool	contains		(const char* s)			const;
	//char*	find_line_end	(const char* rightMost) const;
	void	substr			(const int startIndex, const int length, char* output, const int outputMaxCount) const;
	bool	start_with		(const char* const s, bool ignoreCase = false) const;
	bool	end_with		(const char* const s, bool ignoreCase = false) const;

	bool	replace			(const char* const oldString, const char* const newString);
	int		replace_all		(const char* const oldString, const char* const newString);	//返回发生替换的次数
	void	insert			(const int positionIndex, const char* const insertString);
	void	insert			(const int positionIndex, const char c);

	void	toupper			();
	void	tolower			(); 		
	void	toupper			(const int index);
	void	tolower			(const int index); 		
	int		trim			();
	int		trim_left		();
	int		trim_right		();

	void 	from_int	(const int		value);
	void 	from_uint	(const uint		value);
	void 	from_double	(const double	value);
	void 	from_int64	(const int64	value);
	void 	from_uint64	(const uint64	value);
	int		to_int		() const;
	uint	to_uint		() const;
	double	to_double	() const;
	float	to_float	() const;
	int64	to_int64	() const;
	uint64	to_uint64	() const;
	uint	to_hex		() const;
	bool	to_bool		() const;
	
	//other property
	char*		c_str		()			{ return m_string; }
	const char*	c_str		()	const	{ return m_string; }
	int			length		()	const	{ return static_cast<int>(::strnlen(m_string, static_cast<size_t>(max_size()))); }
	bool		empty		()	const	{ return m_string[0] == 0; } // 返回字符串是否为空
	int			max_size	()	const	{ return MAX_COUNT;	}
	int			capacity	()	const	{ return max_size() - 1; }	//max length
	int			max_sizeof	()	const	{ return max_size() * sizeof(char); }
	int			sizeof_char	()	const	{ return sizeof(char); }
	scl::pstring		pstring	()			{ return scl::pstring(c_str(), max_size()); }
	const scl::pstring	pstring	()	const	{ return scl::pstring(const_cast<char*>(c_str()), max_size()); }

	//operators
	string& 		operator=	(const char* s			)	{ copy(s); return *this; }
	string& 		operator=	(const int value		)	{ from_int(value); return *this; }
	string& 		operator=	(const uint value		)	{ from_uint(value); return *this; }
	string& 		operator=	(const double value		)	{ from_double(value); return *this; }
	string& 		operator=	(const int64 value		)	{ from_int64(value); return *this; }
	string& 		operator=	(const uint64 value		)	{ from_uint64(value); return *this; }
	string& 		operator+=	(const char* s			)	{ append(s); return *this; }
	string& 		operator+=	(const string& s		)	{ append(s.c_str()); return *this; }
	string& 		operator+=	(const char  c			)	{ append(c); return *this; }
	bool 			operator==	(const char* s			) const { return compare(s) == 0; }
	bool 			operator==	(const string& s		) const { return compare(s.c_str()) == 0; }
	bool 			operator>	(const string& s		) const { return compare(s.c_str()) > 0; }
	bool 			operator<	(const string& s		) const { return compare(s.c_str()) < 0; }
	bool 			operator!=	(const char* s			) const { return compare(s) != 0; }
	bool 			operator!=	(const string& s		) const { return compare(s.c_str()) != 0; }
	char&			operator[]	(const int index)		{ if (index > capacity()) {assert(0); throw 1; } return m_string[index]; }
	const char&		operator[]	(const int index) const { if (index > capacity()) { assert(0); throw 1; } return m_string[index]; }
	void			safe_terminate()					{ m_string[max_size() - 1] = 0; }

	template <typename StreamerT> 
	inline void map(StreamerT& s) 
	{ 
		int l = length();

		//内容
		scl::buffer buf(m_string, capacity(), 0, l);
		s << buf; 
		l = buf.length();

		if (l < 0 || l >= MAX_COUNT)
		{
			assertf(0, "error buffer length %d", l);
			l = MAX_COUNT;
		}

		m_string[l] = 0;
	}

private:
	void _init() { clear(); }
	
private:
	char	m_string[MAX_COUNT];
};


#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif

template<int MAX_COUNT>
void string<MAX_COUNT>::copy(const char* const source)
{ 
	if (NULL == source)
		return;

	::strncpy(m_string, source, MAX_COUNT - 1);

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::copy(const char* const source, const int copyCount)
{

	if (NULL == source)
		return;

	int limitCopyCount = copyCount > capacity() ? capacity() : copyCount;
	::strncpy(m_string, source, limitCopyCount);

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::memcpy(const void* p, const int length)
{
	if (NULL == m_string)
		return;
	if (NULL == p)
		return;

	if (length > capacity())
	{
		assert(false);
		return;
	}
	::memcpy(m_string, p, length);

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::substract(const int n)
{
	if (NULL == m_string)
		return;
	
	int pos = length() - n;
	if (pos < 0)
		pos = 0;
	m_string[pos] = 0;

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::append(const char* source)							
{ 
	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	::strncat(m_string, source, freeLength);

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::append(const char* source, const int count)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	if (freeLength < count )
		::strncat(m_string, source, freeLength);
	else
		::strncat(m_string, source, count);

	safe_terminate();
}

template<int MAX_COUNT>
void string<MAX_COUNT>::append(const char	c)							
{ 
	if (NULL == m_string)
		return;
	if (c == 0)
		return;

	int len = length();
	if (len >= capacity())
		return;

	m_string[len]		= c;
	m_string[len + 1]	= 0;

	safe_terminate();
}

template<int MAX_COUNT>
int	string<MAX_COUNT>::compare(const char* const source, const int length, bool ignoreCase) const	
{ 
	if (m_string == source)
		return 0;
	if (NULL == source)
		return 1;

	int result = 0;
	if (ignoreCase)
		result = scl_strncasecmp(m_string, source, length);
	else
		result = ::strncmp(m_string, source, length);

	return result;	
}

template<int MAX_COUNT>
void	string<MAX_COUNT>::erase(const int startIndex, const int removeLength) 
{ 
	//确定需要删除的实际长度
	int len = length();
	int realRemoveLength = removeLength;
	if (realRemoveLength > len - startIndex || realRemoveLength == -1)
	{
		realRemoveLength = len - startIndex;
	}
	if (realRemoveLength <= 0)
	{
		return;
	}

	//执行删除
	int moveLength = len - startIndex - realRemoveLength;
	for (int i = startIndex; i < startIndex + moveLength; ++i)
	{
		m_string[i] = m_string[i + removeLength];
	}
	m_string[len - realRemoveLength] = 0;
}


template<int MAX_COUNT>
int	string<MAX_COUNT>::format_arg(const char* const format, va_list argumentpointer)	
{ 
	//这里不再使用vsprintf_s的版本，因为vsprintf_s版本在Debug下，超出缓冲区长度的时候会报错，并且无法屏蔽

	//这里第二个参数在windows和linux下含义不同
	//		在linux下，第二个参数包含'\0'
	//		在windows下，第二个参数不包含'\0'
	//所以，这里统一传MAX_COUNT，如果超长，在windows下会溢出，此时safe_terminate();会发挥作用
	//linux下safe_terminate()无论是否调用都没关系

	int writtenCount = ::vsnprintf(m_string, MAX_COUNT, format, argumentpointer);
	safe_terminate();
	return writtenCount;
}


template<int MAX_COUNT>
int	string<MAX_COUNT>::format_arg_append(const char* const format, va_list arg)	
{ 
	int freeLength = capacity() - length(); 
	if (freeLength > 0) 
	{ 
		//这里第二个参数在windows和linux下含义不同
		//		在linux下，第二个参数包含'\0'
		//		在windows下，第二个参数不包含'\0'
		//所以，这里统一传freeLength + 1，如果超长，在windows下会溢出，此时safe_terminate();会发挥作用
		//linux下safe_terminate()无论是否调用都没关系
		int writeCount = ::vsnprintf(m_string + length(), freeLength + 1, format, arg);
		safe_terminate();
		return writeCount;
	}
	return 0;
}

template<int MAX_COUNT>
int string<MAX_COUNT>::format(const char* const format, ...) 
{ 
	va_list SFA_arg;
	va_start(SFA_arg, format);
	int writeCount = format_arg(format, SFA_arg);
	va_end(SFA_arg);
	return writeCount;
}

template<int MAX_COUNT>
int string<MAX_COUNT>::format_append(const char* const format, ...) 
{ 
	va_list arg;
	va_start(arg, format);
	int writeCount = format_arg_append(format, arg);
	va_end(arg);
	return writeCount;
}


template<int MAX_COUNT>
int string<MAX_COUNT>::find_first_of(const char c, const int startIndex) const
{
	const char* pFind = ::strchr(m_string + startIndex, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}

template<int MAX_COUNT>
int string<MAX_COUNT>::find_first_of(const char* s, const int startIndex) const
{
	const char* pFind = ::strstr(m_string + startIndex, s);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}

template<int MAX_COUNT>
int string<MAX_COUNT>::find_last_of(const char c) const
{
	const char* pFind = ::strrchr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
int string<MAX_COUNT>::find_last_of(const char* s) const
{
	const char* pFind = NULL;
	const char* pSearch = ::strstr(m_string, s);
	if (NULL == pSearch)
	{
		return -1;
	}
	while (pSearch)
	{
		pFind = pSearch;
		pSearch++;
		pSearch = ::strstr(pSearch, s);
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
bool string<MAX_COUNT>::contains(const char c) const 
{ 
	return find_first_of(c) != -1; 
}

template<int MAX_COUNT>
bool string<MAX_COUNT>::contains(const char* s) const 
{ 
	return find_first_of(s) != -1; 
}

template<int MAX_COUNT>
void string<MAX_COUNT>::substr(const int startIndex, const int subLength, char* output, const int outputMaxCount) const
{
	int copyCount = 0;
	int sourceLength = length();
	for (int i = startIndex; i < sourceLength; ++i)
	{
		if (copyCount >= subLength)
			break;
		if (copyCount >= outputMaxCount - 1)
			break;

		output[copyCount] = m_string[i];
		copyCount++;
	}
	output[copyCount] = 0;
}

template<int MAX_COUNT>
bool string<MAX_COUNT>::start_with(const char* const s, bool ignoreCase) const
{
	const int compareLength = ::strlen(s);
	const int thisLength	= length();
	if (compareLength > thisLength)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	return 0 == compare(s, compareLength, ignoreCase);
}


template<int MAX_COUNT>
bool string<MAX_COUNT>::end_with(const char* const s, bool ignoreCase) const
{
	const int compareLength = ::strlen(s);
	const int thisLength = length();
	const int startIndex = thisLength - compareLength;
	if (startIndex < 0)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	scl::pstring sub(const_cast<char*>(m_string) + startIndex, compareLength + 1);
	return 0 == sub.compare(s, ignoreCase);
}


template<int MAX_COUNT>
bool	string<MAX_COUNT>::replace(const char* const oldString, const char* const newString) 
{ 
	const int stringLength		= length();
	const int stringMaxLength	= MAX_COUNT - 1;

	//int changedCount = 0;
	//找到需替换的字符串
	const int oldIndex = find_first_of(oldString);
	if (-1 == oldIndex)
	{
		return false;
	}

	//oldString和newString的长度都不应该超过当前string，超过的部分不处理
	const int oldLength = static_cast<int>(::strnlen(oldString, stringMaxLength));
	int newLength = static_cast<int>(::strnlen(newString, stringMaxLength));
	if (oldLength > newLength)
	{
		::memcpy(m_string + oldIndex, newString, newLength);
		//将多余的地方补齐
		const int moveCount = oldLength - newLength;
		erase(oldIndex + newLength, moveCount);
	}
	else if (oldLength < newLength)
	{
		//将oldIndex后面的字符串右移，空出足够的空间
		const int diff = newLength - oldLength;
		int moveCount = stringLength - (oldIndex + oldLength);
		int newEnd = (stringLength + diff) - 1;
		//如果newEnd越界，安全截断
		if (newEnd > capacity() - 1)
		{
			const int overflowed = newEnd - (capacity() - 1);
			moveCount -= overflowed;
			newEnd = capacity() - 1;
		}
		for (int i = newEnd; i > newEnd - moveCount; --i)
		{
			m_string[i] = m_string[i - diff];
		}
		//如果newLength太长，安全截断
		if (oldIndex + newLength >= MAX_COUNT)
		{
			newLength -= ((oldIndex + newLength - MAX_COUNT) + 1);
		}
		::memcpy(m_string + oldIndex, newString, newLength);
		m_string[newEnd + 1] = 0;
		//changedCount = moveCount + offset;
	}
	else if (oldLength == newLength)
	{
		::memcpy(m_string + oldIndex, newString, oldLength);
		//字符串长度没有发生变化
	}
	else
	{
		assert(0);
		return false;
	}
	return true;
}


template<int MAX_COUNT>
int	string<MAX_COUNT>::replace_all(const char* const oldString, const char* const newString)//返回是发生替换的次数
{ 
	int replacedTimes = 0;  
	while (replace(oldString, newString))
	{
		replacedTimes++;
	}
	return replacedTimes;
} 

template<int MAX_COUNT>
void string<MAX_COUNT>::insert(const int positionIndex, const char* const insertString) 
{ 
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	if (NULL == insertString)
		return;

	int insertLength = ::strnlen(insertString, stringMaxLength);
	//检查插入字符串长度
	if (insertLength <= 0)
		return;

	//检查插入位置
	if (positionIndex > stringLength || positionIndex < 0)
		return;

	int moveCount	= stringLength - positionIndex;
	int newEnd		= stringLength + insertLength - 1;
	//检查插入后总长度
	if (newEnd > stringMaxLength - 1)
	{
		const int overflowed = newEnd - (stringMaxLength - 1);
		moveCount -= overflowed;
		newEnd = stringMaxLength - 1;
	}
	
	for (int i = newEnd; i > newEnd - moveCount; --i)
	{
		m_string[i] = m_string[i - insertLength];
	}
	const int copyMaxIndex = positionIndex + insertLength;
	if (copyMaxIndex > MAX_COUNT - 1)
	{
		insertLength -= (copyMaxIndex - (MAX_COUNT - 1));
	}
	::memcpy(m_string + positionIndex, insertString, insertLength);
	m_string[newEnd + 1] = 0;
}


template<int MAX_COUNT>
void string<MAX_COUNT>::insert(const int positionIndex, const char insertChar) 
{
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	const int insertLength = 1;

	//检查插入位置
	if (positionIndex > stringLength || positionIndex < 0)
		return;
	if (stringLength + insertLength >= MAX_COUNT)
		return;

	int newEnd = stringLength + insertLength - 1;
	//检查插入后总长度
	if (newEnd > stringMaxLength)
	{
		return;
	}
	int moveCount	= stringLength - positionIndex;
	for (int i = newEnd; i > newEnd - moveCount; --i)
	{
		m_string[i] = m_string[i - insertLength];
	}
	m_string[positionIndex] = insertChar;
	m_string[stringLength + insertLength] = 0;
}


template<int MAX_COUNT>
void string<MAX_COUNT>::toupper() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::toupper(m_string[i]);
	}	
}

template<int MAX_COUNT>
void string<MAX_COUNT>::tolower() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::tolower(m_string[i]);
	}	
}

template<int MAX_COUNT>
void string<MAX_COUNT>::toupper(const int index) 		
{ 
	if (index < 0 || index >= MAX_COUNT)
	{
		assert(0);
		return;
	}
	m_string[index] = ::toupper(m_string[index]);
}

template<int MAX_COUNT>
void string<MAX_COUNT>::tolower(const int index) 		
{ 
	if (index < 0 || index >= MAX_COUNT)
	{
		assert(0);
		return;
	}
	m_string[index] = ::tolower(m_string[index]);
}

template<int MAX_COUNT>
int string<MAX_COUNT>::trim()			
{ 
	int rightCount = trim_right();
	int leftCount = trim_left();
	return rightCount + leftCount;
}

template<int MAX_COUNT>
int string<MAX_COUNT>::trim_left()		
{ 
	int stringLength = length();

	//清除左侧的空字符
	int emptyCount = 0;
	for (int i = 0; i < stringLength; ++i)
	{
		if (!scl::isspace(m_string[i]))
			break;

		emptyCount++;
	}
	//删除所有空字符
	erase(0, emptyCount);
	return emptyCount;
}

template<int MAX_COUNT>
int string<MAX_COUNT>::trim_right()		
{ 
	int stringLength = length();
	int deleteEndCount = 0;
	//先清除末尾的空字符
	for (int i = stringLength - 1; i > 0; --i)
	{
		if (!scl::isspace(m_string[i]))
			break;

		m_string[i] = 0;
		deleteEndCount++;
	}
	return deleteEndCount;
}


template<int MAX_COUNT>
void string<MAX_COUNT>::from_int(const int value) 
{ 
	clear();
	format("%d", value);
}

template<int MAX_COUNT>
void string<MAX_COUNT>::from_uint(const uint value) 
{ 	
	clear();
	format("%u", value);
}

template<int MAX_COUNT>
void string<MAX_COUNT>::from_double(const double value)
{
	clear();
	format("%f", value);
}

template<int MAX_COUNT>
void string<MAX_COUNT>::from_int64(const int64 value)
{
	clear();
	format(SCL_STR_FORMAT_I64, value);
}

template<int MAX_COUNT>
void string<MAX_COUNT>::from_uint64(const uint64 value)
{
	clear();
	format(SCL_STR_FORMAT_UI64, value);
}

template<int MAX_COUNT>
int string<MAX_COUNT>::to_int() const
{
	return static_cast<int>(::strtol(m_string, NULL, 0));
}

template<int MAX_COUNT>
uint string<MAX_COUNT>::to_uint() const
{
	return static_cast<uint>(::strtoul(m_string, NULL, 0));
}

template<int MAX_COUNT>
double string<MAX_COUNT>::to_double() const
{
	return ::strtod(m_string, NULL);
}

template<int MAX_COUNT>
float string<MAX_COUNT>::to_float() const
{
	return static_cast<float>(::strtod(m_string, NULL));
}

template<int MAX_COUNT>
int64 string<MAX_COUNT>::to_int64() const
{
	return scl_strtoi64(m_string, NULL, 0);
}

template<int MAX_COUNT>
uint64 string<MAX_COUNT>::to_uint64() const
{
	return scl_strtoui64(m_string, NULL, 0);
}

template<int MAX_COUNT>
uint string<MAX_COUNT>::to_hex() const
{
	return ::strtoul(m_string, NULL, 16);
}

template<int MAX_COUNT>
bool string<MAX_COUNT>::to_bool() const
{
	const char c = m_string[0];
	if (c == 'f' || c == 'F' || c == '0')
		return false;
	//if (c == 't' || c == 'T' || c == '1')
	//	return true;
	return true;
}


#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif

////////////////////////////////////
// StringN
////////////////////////////////////
typedef 	string<8> 	string8;
typedef 	string<16> 	string16;
typedef		string<32> 	string32;
typedef 	string<64> 	string64;
typedef 	string<128> string128;
typedef 	string<256> string256;
typedef 	string<260> string260;
#ifdef SCL_WIN
typedef 	string<260> stringPath;
#else
typedef 	string<512> stringPath;
#endif
typedef 	string<512> string512;
typedef 	string<1024> string1024;
typedef 	string<2048> string2048;
typedef 	string<4096> string4096;
typedef 	string<8192> string8192;
typedef 	string<16384> string16384;
typedef		string<65536> string65536;

typedef 	string<1024> string1k;
typedef 	string<2048> string2k;
typedef 	string<4096> string4k;
typedef 	string<8192> string8k;
typedef 	string<16384> string16k;
typedef		string<65536> string64k;

} //namespace scl

using scl::string8;
using scl::string16;
using scl::string32;
using scl::string64;
using scl::string128;
using scl::string256;
using scl::string260;
using scl::stringPath;
using scl::string512;
using scl::string1024;
using scl::string2048;
using scl::string4096;
using scl::string8192;
using scl::string16384;
using scl::string1k;
using scl::string2k;
using scl::string4k;
using scl::string8k;
using scl::string16k;
using scl::string64k;


