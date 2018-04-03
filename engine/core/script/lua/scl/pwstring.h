////////////////////////////////////////////////////////////////////////////////
//	pwstring (a wrap of wchar*)
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/stringdef.h"

#include <wchar.h>
#include <stdarg.h>

namespace scl {

class pwstring
{
public:
	pwstring();

	//maxCount传入0表示忽略长度，即对PString的调用不涉及最大长度，maxCount传入-1表示自动计算长度，+1后作为maxCount
	pwstring	(wchar* s, const int maxCount = -1, const wchar* initString = NULL); 
	void init	(wchar* s, const int maxCount = -1, const wchar* initString = NULL);
	void alloc	(const int maxCount, const wchar* initString = NULL) { init(new wchar[maxCount], maxCount, initString); }
	void free	() { if (NULL != m_string) { delete[] m_string; m_string = NULL; } }

	void 	copy		(const wchar* const s);
	void 	copy		(const wchar* const s, const int count);
	void 	append		(const wchar* s);
	void 	append		(const wchar* s, const int count);
	void 	append		(const wchar	c);
	int 	compare		(const wchar* s, bool ignoreCase = false) const	{ return compare(s, max_size(), ignoreCase); }
	int		compare		(const wchar* const source, const int length, bool ignoreCase = false) const;
	//int 	compare		(const pwstring& source, bool ignoreCase = false) const { return compare(source.c_str(), ignoreCase);  }
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
	int			length		()	const	{ return static_cast<int>(wcsnlen(m_string, max_size())); }
	bool		empty		()	const	{ return m_string[0] == 0; } // 返回字符串是否为空
	int			max_size	()	const	{ return MAX_COUNT;	}
	int			capacity	()	const	{ return max_size() - 1; }	//max length
	int			max_sizeof	()	const	{ return max_size() * sizeof(wchar); }
	int			sizeof_char	()	const	{ return sizeof(wchar); }

	//operators
	pwstring& 		operator=	(const wchar* s			)	{ copy(s); return *this; }
	pwstring& 		operator=	(const int value		)	{ from_int(value); return *this; }
	pwstring& 		operator=	(const uint value		)	{ from_uint(value); return *this; }
	pwstring& 		operator=	(const double value		)	{ from_double(value); return *this; }
	pwstring& 		operator=	(const int64 value		)	{ from_int64(value); return *this; }
	pwstring& 		operator=	(const uint64 value		)	{ from_uint64(value); return *this; }
	pwstring& 		operator+=	(const wchar* s			)	{ append(s); return *this; }
	pwstring& 		operator+=	(const pwstring& s		)	{ append(s.c_str()); return *this; }
	pwstring& 		operator+=	(const wchar  c			)	{ append(c); return *this; }
	bool 			operator==	(const wchar* s			) const { return compare(s) == 0; }
	bool 			operator==	(const pwstring& s		) const { return compare(s.c_str()) == 0; }
	bool 			operator>	(const pwstring& s		) const { return compare(s.c_str()) > 0; }
	bool 			operator<	(const pwstring& s		) const { return compare(s.c_str()) < 0; }
	bool 			operator!=	(const wchar* s			) const { return compare(s) != 0; }
	bool 			operator!=	(const pwstring& s		) const { return compare(s.c_str()) != 0; }
	wchar&			operator[]	(const int index)		{ if (NULL == m_string || index > capacity()) { assert(0);throw 1; } return m_string[index]; }
	const wchar&	operator[]	(const int index) const { if (NULL == m_string || index > capacity()) { assert(0);throw 1; } return m_string[index]; }
	void			safe_terminate()					{ m_string[max_size() - 1] = 0; }
	
private:
	wchar*	m_string;
	int		MAX_COUNT;
};


} //namespace scl

using scl::pwstring;

