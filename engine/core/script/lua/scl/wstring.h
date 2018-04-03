////////////////////////////////////////////////////////////////////////////////
//	wstring (a wrap of wchar[N])
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/stringdef.h"
#include "scl/pwstring.h"
#include "scl/string.h"

#include <stdarg.h>

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <wctype.h>
#endif

namespace scl {

template<int MAX_COUNT>
class wstring
{
public:
	wstring()						{ _init(); }
	wstring(const wchar* s)			{ _init(); copy(s); }

	void 	copy		(const wchar* const s);
	void 	copy		(const wchar* const s, const int count);
	void 	append		(const wchar* s);
	void 	append		(const wchar* s, const int count);
	void 	append		(const wchar	c);
	int 	compare		(const wchar* s, bool ignoreCase = false) const	{ return compare(s, max_size(), ignoreCase); }
	int		compare		(const wchar* const source, const int length, bool ignoreCase = false) const;
	int 	compare		(const wstring& s, bool ignoreCase = false) const { return compare(s.c_str(), ignoreCase); }
	void	erase		(const int startIndex = 0, const int length = -1);
	void	clear		()	{ ::wmemset(m_string, 0, MAX_COUNT); }
	int		format			(const wchar* const format, ...);	//返回新写入的字符数，不包含'\0'。当越界时，windows下返回-1，linux下返回“假如没有越界，应当写入的总字符数”
	int		format_append	(const wchar* const format, ...);
	int		format_arg		(const wchar* const format, va_list arg);
	int		format_arg_append(const wchar* const format, va_list arg);
	int		find			(const wchar c)			const	{ return find_first_of(c); }
	int		find			(const wchar* const s)	const	{ return find_first_of(s); }
	int		find_first_of	(const wchar c)			const;	//find_first_of 返回找到的字符串的起始index，没找到返回-1
	int		find_first_of	(const wchar* const s)	const;
	int		find_last_of	(const wchar c)			const;
	int		find_last_of	(const wchar* const s)	const;
	bool	contains		(const wchar c)			const;
	bool	contains		(const wchar* s)		const;
	//wchar*	find_line_end	(const wchar* rightMost) const;
	void	substr			(const int startIndex, const int length, wchar* output, const int outputMaxCount) const;
	bool	start_with		(const wchar* const s, bool ignoreCase = false) const;
	bool	end_with		(const wchar* const s, bool ignoreCase = false) const;

	bool	replace			(const wchar* const oldString, const wchar* const newString);
	int		replace_all		(const wchar* const oldString, const wchar* const newString);	//返回发生替换的次数
	void	insert			(const int positionIndex, const wchar* const insertString);
	void	insert			(const int positionIndex, const wchar c);

	void	toupper			();
	void	tolower			(); 		
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

	//from/to multi byte
	void	from_utf8	(const char* utf8	);
	void	from_gbk	(const char* gbk	);
	void	from_ansi	(const char* ansi	);
	void	to_utf8		(char* out, const int maxCount) const;
	void	to_gbk		(char* out, const int maxCount) const;
	void	to_ansi		(char* out, const int maxCount) const;

	//other property
	wchar*		c_str		()			{ return m_string; }
	const wchar*c_str		()	const	{ return m_string; }
	int			length		()	const	{ return static_cast<int>(::wcsnlen(m_string, max_size())); }
	bool		empty		()	const	{ return m_string[0] == 0; } // 返回字符串是否为空
	int			max_size	()	const	{ return MAX_COUNT;	}
	int			capacity	()	const	{ return max_size() - 1; }	//max length
	int			max_sizeof	()	const	{ return max_size() * sizeof(wchar); }
	int			sizeof_char	()	const	{ return sizeof(wchar); }
	scl::pwstring		pstring	()			{ return scl::pwstring(c_str(), max_size()); }
	const scl::pwstring	pstring	()	const	{ return scl::pwstring(const_cast<wchar*>(c_str()), max_size()); }

	//operators
	wstring& 		operator=	(const wchar* s			)	{ copy(s); return *this; }
	wstring& 		operator=	(const int value		)	{ from_int(value); return *this; }
	wstring& 		operator=	(const uint value		)	{ from_uint(value); return *this; }
	wstring& 		operator=	(const double value		)	{ from_double(value); return *this; }
	wstring& 		operator=	(const int64 value		)	{ from_int64(value); return *this; }
	wstring& 		operator=	(const uint64 value		)	{ from_uint64(value); return *this; }
	wstring& 		operator+=	(const wchar* s			)	{ append(s); return *this; }
	wstring& 		operator+=	(const wstring& s		)	{ append(s.c_str()); return *this; }
	wstring& 		operator+=	(const wchar  c			)	{ append(c); return *this; }
	bool 			operator==	(const wchar* s			) const { return compare(s) == 0; }
	bool 			operator==	(const wstring& s		) const { return compare(s.c_str()) == 0; }
	bool 			operator>	(const wstring& s		) const { return compare(s.c_str()) > 0; }
	bool 			operator<	(const wstring& s		) const { return compare(s.c_str()) < 0; }
	bool 			operator!=	(const wchar* s			) const { return compare(s) != 0; }
	bool 			operator!=	(const wstring& s		) const { return compare(s.c_str()) != 0; }
	wchar&			operator[]	(const int index)		{ if (NULL == m_string || index > capacity()) { assert(0); throw 1; } return m_string[index]; }
	const wchar&	operator[]	(const int index) const { if (NULL == m_string || index > capacity()) { assert(0); throw 1; } return m_string[index]; }
	void			safe_terminate()					{ m_string[max_size() - 1] = 0; }

	template <typename T> void map(T& s) { s << this->pstring(); }

private:
	void _init() { clear(); }
	
private:
	wchar	m_string[MAX_COUNT];
};

#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif

template<int MAX_COUNT>
void wstring<MAX_COUNT>::copy(const wchar* const source)
{ 
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	::wcsncpy(m_string, source, MAX_COUNT - 1);

	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::copy(const wchar* const source, const int copyCount)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int limitCopyCount = copyCount > capacity() ? capacity() : copyCount;
	::wcsncpy(m_string, source, limitCopyCount);

	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::append(const wchar* source)							
{ 
	if (NULL == m_string)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	::wcsncat(m_string, source, freeLength);

	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::append(const wchar* source, const int count)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
	if (freeLength < count )
		::wcsncat(m_string, source, freeLength);
	else
		::wcsncat(m_string, source, count);

	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::append(const wchar	c)							
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
int	wstring<MAX_COUNT>::compare(const wchar* const source, const int length, bool ignoreCase) const	
{ 
	if (m_string == source)
		return 0;
	if (NULL == m_string)
		return -1;
	if (NULL == source)
		return 1;

	int result = 0;
	if (ignoreCase)
		result = scl_wcsncasecmp(m_string, source, length);
	else
		result = ::wcsncmp(m_string, source, length);

	return result;	
}

template<int MAX_COUNT>
void	wstring<MAX_COUNT>::erase(const int startIndex, const int removeLength) 
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
int	wstring<MAX_COUNT>::format_arg(const wchar* const format, va_list argumentpointer)	
{ 
	//这里不再使用vsprintf_s的版本，因为vsprintf_s版本在Debug下，超出缓冲区长度的时候会报错，并且无法屏蔽

	//这里第二个参数在windows和linux下含义不同
	//		在linux下，第二个参数包含'\0'
	//		在windows下，第二个参数不包含'\0'
	//所以，这里统一传MAX_COUNT，如果超长，在windows下会溢出，此时safe_terminate();会发挥作用
	//linux下safe_terminate()无论是否调用都没关系

	int writtenCount = ::vswprintf(m_string, MAX_COUNT, format, argumentpointer);
	safe_terminate();
	return writtenCount;
}


template<int MAX_COUNT>
int	wstring<MAX_COUNT>::format_arg_append(const wchar* const format, va_list arg)	
{ 
	int freeLength = capacity() - length();  
	if (freeLength > 0) 
	{ 
		//这里第二个参数在windows和linux下含义不同
		//		在linux下，第二个参数包含'\0'
		//		在windows下，第二个参数不包含'\0'
		//所以，这里统一传freeLength + 1，如果超长，在windows下会溢出，此时safe_terminate();会发挥作用
		//linux下safe_terminate()无论是否调用都没关系
		int writeCount = ::vswprintf(m_string + length(), freeLength + 1, format, arg);
		safe_terminate();
		return writeCount;
	}
	return 0;
}


template<int MAX_COUNT>
int wstring<MAX_COUNT>::format(const wchar* const format, ...) 
{ 
	va_list SFA_arg;
	va_start(SFA_arg, format);
	int writeCount = format_arg(format, SFA_arg);
	va_end(SFA_arg);
	return writeCount;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::format_append(const wchar* const format, ...) 
{ 
	va_list arg;
	va_start(arg, format);
	int writeCount = format_arg_append(format, arg);
	va_end(arg);
	return writeCount;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::find_first_of(const wchar c) const
{
	const wchar* pFind = ::wcschr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::find_first_of(const wchar* findString) const
{
	const wchar* pFind = ::wcsstr(m_string, findString);
	if (NULL == pFind)
	{
		return -1;
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::find_last_of(const wchar c) const
{
	const wchar* pFind = ::wcsrchr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::find_last_of(const wchar* s) const
{
	const wchar* pFind = NULL;
	const wchar* pSearch = ::wcsstr(m_string, s);
	if (NULL == pSearch)
	{
		return -1;
	}
	while (pSearch)
	{
		pFind = pSearch;
		pSearch++;
		pSearch = ::wcsstr(pSearch, s);
	}
	return pFind - m_string;
}

template<int MAX_COUNT>
bool wstring<MAX_COUNT>::contains(const wchar c) const 
{ 
	return find_first_of(c) != -1; 
}

template<int MAX_COUNT>
bool wstring<MAX_COUNT>::contains(const wchar* s) const 
{ 
	return find_first_of(s) != -1; 
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::substr(const int startIndex, const int subLength, wchar* output, const int outputMaxCount) const
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
bool wstring<MAX_COUNT>::start_with(const wchar* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<const int>(::wcslen(s));
	const int thisLength	= length();
	if (compareLength > thisLength)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	return 0 == compare(s, compareLength, ignoreCase);
}

template<int MAX_COUNT>
bool wstring<MAX_COUNT>::end_with(const wchar* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<const int>(::wcslen(s));
	const int thisLength = length();
	const int startIndex = thisLength - compareLength;
	if (startIndex < 0)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	const scl::pwstring sub(const_cast<wchar*>(m_string) + startIndex, compareLength + 1);
	return 0 == sub.compare(s, ignoreCase);
}


template<int MAX_COUNT>
bool	wstring<MAX_COUNT>::replace(const wchar* const oldString, const wchar* const newString) 
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
	const int oldLength = ::wcsnlen(oldString, stringMaxLength);
	int newLength = ::wcsnlen(newString, stringMaxLength);
	if (oldLength > newLength)
	{
		::wmemcpy(m_string + oldIndex, newString, newLength);
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
		::wmemcpy(m_string + oldIndex, newString, newLength);
		m_string[newEnd + 1] = 0;
		//changedCount = moveCount + offset;
	}
	else if (oldLength == newLength)
	{
		::wmemcpy(m_string + oldIndex, newString, oldLength);
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
int	wstring<MAX_COUNT>::replace_all(const wchar* const oldString, const wchar* const newString)//返回是发生替换的次数
{ 
	int replacedTimes = 0;  
	while (replace(oldString, newString))
	{
		replacedTimes++;
	}
	return replacedTimes;
} 

template<int MAX_COUNT>
void wstring<MAX_COUNT>::insert(const int positionIndex, const wchar* const insertString) 
{ 
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	if (NULL == insertString)
		return;

	int insertLength = ::wcsnlen(insertString, stringMaxLength);
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
	::wmemcpy(m_string + positionIndex, insertString, insertLength);
	m_string[newEnd + 1] = 0;
}


template<int MAX_COUNT>
void wstring<MAX_COUNT>::insert(const int positionIndex, const wchar insertChar) 
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
void wstring<MAX_COUNT>::toupper() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::towupper(m_string[i]);
	}	
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::tolower() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::towlower(m_string[i]);
	}	
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::trim()			
{ 
	int rightCount = trim_right();
	int leftCount = trim_left();
	return rightCount + leftCount;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::trim_left()		
{ 
	int stringLength = length();

	//清除左侧的空字符
	int emptyCount = 0;
	for (int i = 0; i < stringLength; ++i)
	{
		if (!scl::iswspace(m_string[i]))
			break;

		emptyCount++;
	}
	//删除所有空字符
	erase(0, emptyCount);
	return emptyCount;
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::trim_right()		
{ 
	int stringLength = length();
	int deleteEndCount = 0;
	//先清除末尾的空字符
	for (int i = stringLength - 1; i > 0; --i)
	{
		if (!scl::iswspace(m_string[i]))
			break;

		m_string[i] = 0;
		deleteEndCount++;
	}
	return deleteEndCount;
}


template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_int(const int value) 
{ 
	clear();
	format(L"%d", value);
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_uint(const uint value) 
{ 	
	clear();
	format(L"%u", value);
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_double(const double value)
{
	clear();
	format(L"%f", value);
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_int64(const int64 value)
{
	clear();
	format(SCL_WCS_FORMAT_I64, value);
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_uint64(const uint64 value)
{
	clear();
	format(SCL_WCS_FORMAT_UI64, value);
}

template<int MAX_COUNT>
int wstring<MAX_COUNT>::to_int() const
{
#ifdef SCL_ANDROID
	string<MAX_COUNT> _s;
	to_utf8(_s.c_str(), _s.capacity());
	return _s.to_uint();
#else
	return ::wcstol(m_string, NULL, 10);
#endif
}

template<int MAX_COUNT>
uint wstring<MAX_COUNT>::to_uint() const
{
#ifdef SCL_ANDROID
	string<MAX_COUNT> _s;
	to_utf8(_s.c_str(), _s.capacity());
	return _s.to_uint();
#else
	return ::wcstoul(m_string, NULL, 10);
#endif
}

template<int MAX_COUNT>
double wstring<MAX_COUNT>::to_double() const
{
#ifdef SCL_ANDROID
	string<MAX_COUNT> _s;
	to_utf8(_s.c_str(), _s.capacity());
	return _s.to_double();
#else
	return ::wcstod(m_string, NULL);
#endif
}

template<int MAX_COUNT>
float wstring<MAX_COUNT>::to_float() const
{
#ifdef SCL_ANDROID
	string<MAX_COUNT> _s;
	to_utf8(_s.c_str(), _s.capacity());
	return _s.to_float();
#else
	return static_cast<float>(::wcstod(m_string, NULL));
#endif
}

template<int MAX_COUNT>
int64 wstring<MAX_COUNT>::to_int64() const
{
	return scl_wcstoi64(m_string, NULL, 10);
}

template<int MAX_COUNT>
uint64 wstring<MAX_COUNT>::to_uint64() const
{
	return scl_wcstoui64(m_string, NULL, 10);
}

template<int MAX_COUNT>
uint wstring<MAX_COUNT>::to_hex() const
{
#ifdef SCL_ANDROID
	string<MAX_COUNT> _s;
	to_utf8(_s.c_str(), _s.capacity());
	return _s.to_hex();
#else
	return ::wcstoul(m_string, NULL, 16);
#endif
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_ansi(const char* ansi)
{
    clear();
#if defined(_SCL_ENCODING_GBK_)
	from_gbk(ansi);
#else
	from_utf8(ansi);
#endif
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_utf8(const char* utf8)
{
    clear();
	ansi_to_wchar(m_string, MAX_COUNT, utf8, -1, Encoding_UTF8);
	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::from_gbk(const char* gbk)
{
    clear();
	ansi_to_wchar(m_string, MAX_COUNT, gbk, -1, Encoding_GBK);
	safe_terminate();
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::to_ansi(char* ansi, const int maxCount) const
{
#if defined(_SCL_ENCODING_GBK_)
	to_gbk(ansi, maxCount);
#else
	to_utf8(ansi, maxCount);
#endif
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::to_utf8(char* utf8, const int maxCount) const
{
	wchar_to_ansi(utf8, maxCount, m_string, -1, Encoding_UTF8);
}

template<int MAX_COUNT>
void wstring<MAX_COUNT>::to_gbk(char* gbk, const int maxCount) const
{
	wchar_to_ansi(gbk, maxCount - 1, m_string, -1, Encoding_GBK);
}

#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif

////////////////////////////////////
// StringN
////////////////////////////////////
typedef 	wstring<8> 		wstring8;
typedef 	wstring<16> 	wstring16;
typedef		wstring<32> 	wstring32;
typedef 	wstring<64> 	wstring64;
typedef 	wstring<128> 	wstring128;
typedef 	wstring<256> 	wstring256;
typedef 	wstring<260> 	wstring260;
typedef 	wstring<260> 	wstringPath;
typedef 	wstring<512> 	wstring512;
typedef 	wstring<1024>	wstring1024;

} //namespace scl


using scl::wstring8;
using scl::wstring16;
using scl::wstring32;
using scl::wstring64;
using scl::wstring128;
using scl::wstring256;
using scl::wstring260;
using scl::wstringPath;
using scl::wstring512;
using scl::wstring1024;

