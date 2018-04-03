////////////////////////////////////////////////////////////////////////////////
//	pwstring (a wrap of wchar*)
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////

#include "scl/pwstring.h"

#include <stdarg.h>
#include <stdio.h>

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <wctype.h>
#endif


namespace scl {



pwstring::pwstring() : 
m_string(NULL),
MAX_COUNT(0)
{ 

}

pwstring::pwstring(wchar* s, const int maxCount, const wchar* initString) // maxCount = -1, initString = NULL
{ 
	init(s, maxCount, initString);
}

void pwstring::init(wchar* s, const int maxCount, const wchar* initString) //maxCount = -1, initString = NULL
{ 
	m_string = s; 
	MAX_COUNT = maxCount; 
	if (maxCount == -1)
	{ 
		MAX_COUNT = static_cast<int>(::wcslen(m_string) + 1);
	}
	if (NULL != initString)
	{
		clear();
		copy(initString);
	}
}

void pwstring::copy(const wchar* const source)
{ 
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif
	::wcsncpy(m_string, source, MAX_COUNT - 1);
#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif

	safe_terminate();
}


void pwstring::copy(const wchar* const source, const int copyCount)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int limitCopyCount = copyCount > capacity() ? capacity() : copyCount;

#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif
	::wcsncpy(m_string, source, limitCopyCount);

#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif
	safe_terminate();
}


void pwstring::append(const wchar* source)							
{ 
	if (NULL == m_string)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif
	::wcsncat(m_string, source, freeLength);

#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif
	safe_terminate();
}


void pwstring::append(const wchar* source, const int count)
{
	if (NULL == m_string)
		return;
	if (NULL == source)
		return;

	int freeLength = MAX_COUNT - length() - 1;
	if (freeLength <= 0)
		return;
#ifdef SCL_WIN
#pragma warning (disable: 4996)
#endif
	if (freeLength < count )
		::wcsncat(m_string, source, freeLength);
	else
		::wcsncat(m_string, source, count);

#ifdef SCL_WIN
#pragma warning (default: 4996)
#endif
	safe_terminate();
}


void pwstring::append(const wchar	c)							
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


int	pwstring::compare(const wchar* const source, const int length, bool ignoreCase) const	
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


void	pwstring::erase(const int startIndex, const int removeLength) 
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



int	pwstring::format_arg(const wchar* const format, va_list argumentpointer)	
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



int	pwstring::format_arg_append(const wchar* const format, va_list arg)	
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


int pwstring::format(const wchar* const format, ...) 
{ 
	va_list SFA_arg;
	va_start(SFA_arg, format);
	int writeCount = format_arg(format, SFA_arg);
	va_end(SFA_arg);
	return writeCount;
}


int pwstring::format_append(const wchar* const format, ...) 
{ 
	va_list arg;
	va_start(arg, format);
	int writeCount = format_arg_append(format, arg);
	va_end(arg);
	return writeCount;
}


int pwstring::find_first_of(const wchar c) const
{
	const wchar* pFind = ::wcschr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pwstring::find_first_of(const wchar* s) const
{
	const wchar* pFind = ::wcsstr(m_string, s);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pwstring::find_last_of(const wchar c) const
{
	const wchar* pFind = ::wcsrchr(m_string, c);
	if (NULL == pFind)
	{
		return -1;
	}
	return static_cast<int>(pFind - m_string);
}


int pwstring::find_last_of(const wchar* s) const
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
	return static_cast<int>(pFind - m_string);
}


bool pwstring::contains(const wchar c) const 
{ 
	return find_first_of(c) != -1; 
}


bool pwstring::contains(const wchar* s) const 
{ 
	return find_first_of(s) != -1; 
}


void pwstring::substr(const int startIndex, const int subLength, wchar* output, const int outputMaxCount) const
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

bool pwstring::start_with(const wchar* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<int>(::wcslen(s));
	const int thisLength	= length();
	if (compareLength > thisLength)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	return 0 == compare(s, compareLength, ignoreCase);
}

bool pwstring::end_with(const wchar* const s, bool ignoreCase) const
{
	const int compareLength = static_cast<int>(::wcslen(s));
	const int thisLength = length();
	const int startIndex = thisLength - compareLength;
	if (startIndex < 0)
		return false;
	if (compareLength == 0 && thisLength != 0)
		return false;
	const pwstring sub(m_string + startIndex, compareLength + 1);
	return 0 == sub.compare(s, ignoreCase);
}



bool pwstring::replace(const wchar* const oldString, const wchar* const newString) 
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
	const int oldLength = static_cast<int>(::wcsnlen(oldString, stringMaxLength));
	int newLength = static_cast<int>(::wcsnlen(newString, stringMaxLength));
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



int	pwstring::replace_all(const wchar* const oldString, const wchar* const newString)//返回是发生替换的次数
{ 
	int replacedTimes = 0;  
	while (replace(oldString, newString))
	{
		replacedTimes++;
	}
	return replacedTimes;
} 


void pwstring::insert(const int positionIndex, const wchar* const insertString) 
{ 
	const int stringLength = length();
	const int stringMaxLength = MAX_COUNT - 1;

	if (NULL == insertString)
		return;

	int insertLength = static_cast<int>(::wcsnlen(insertString, stringMaxLength));
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



void pwstring::insert(const int positionIndex, const wchar insertChar) 
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



void pwstring::toupper() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::towupper(m_string[i]);
	}	
}


void pwstring::tolower() 		
{ 
	int len = length();
	for (int i = 0; i < len; ++i)
	{
		m_string[i] = ::towlower(m_string[i]);
	}	
}


int pwstring::trim()			
{ 
	int rightCount = trim_right();
	int leftCount = trim_left();
	return rightCount + leftCount;
}


int pwstring::trim_left()		
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


int pwstring::trim_right()		
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



void pwstring::from_int(const int value) 
{ 
	clear();
	format(L"%d", value);
}


void pwstring::from_uint(const uint value) 
{ 	
	clear();
	format(L"%u", value);
}


void pwstring::from_double(const double value)
{
	clear();
	format(L"%f", value);
}


void pwstring::from_int64(const int64 value)
{
	clear();
	format(SCL_WCS_FORMAT_I64, value);
}


void pwstring::from_uint64(const uint64 value)
{
	clear();
	format(SCL_WCS_FORMAT_UI64, value);
}


int pwstring::to_int() const
{
	return static_cast<int>(::wcstol(m_string, NULL, 10));
}


uint pwstring::to_uint() const
{
	return static_cast<int>(::wcstoul(m_string, NULL, 10));
}


double pwstring::to_double() const
{
	return ::wcstod(m_string, NULL);
}


float pwstring::to_float() const
{
	return static_cast<float>(::wcstod(m_string, NULL));
}


int64 pwstring::to_int64() const
{
	return scl_wcstoi64(m_string, NULL, 10);
}


uint64 pwstring::to_uint64() const
{
	return scl_wcstoui64(m_string, NULL, 10);
}


uint pwstring::to_hex() const
{
	return static_cast<uint>(::wcstoul(m_string, NULL, 16));
}

void pwstring::from_ansi(const char* ansi)
{
    clear();
#if defined(_SCL_ENCODING_GBK_)
	from_gbk(ansi);
#else
	from_utf8(ansi);
#endif
}


void pwstring::from_utf8(const char* utf8)
{
    clear();
	ansi_to_wchar(m_string, MAX_COUNT, utf8, -1, Encoding_UTF8);
	safe_terminate();
}


void pwstring::from_gbk(const char* gbk)
{
    clear();
	ansi_to_wchar(m_string, MAX_COUNT, gbk, -1, Encoding_GBK);
	safe_terminate();
}


void pwstring::to_ansi(char* ansi, const int maxCount) const
{
#if defined(_SCL_ENCODING_GBK_)
	to_gbk(ansi, maxCount);
#else
	to_utf8(ansi, maxCount);
#endif
}


void pwstring::to_utf8(char* utf8, const int maxCount) const
{
	wchar_to_ansi(utf8, maxCount, m_string, -1, Encoding_UTF8);
}


void pwstring::to_gbk(char* gbk, const int maxCount) const
{
	wchar_to_ansi(gbk, maxCount - 1, m_string, -1, Encoding_GBK);
}





} //namespace scl



