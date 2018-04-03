////////////////////////////////////////////////////////////////////////////////
//	pstring (a wrap of char*)
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/stringdef.h"
#include "scl/buffer.h"

#include <memory.h>
#include <stdarg.h>
#include <string.h>

namespace scl {

class pstring
{
public:
	pstring();

	//maxCount传入0表示忽略长度，即对PString的调用不涉及最大长度，maxCount传入-1表示自动计算长度，+1后作为maxCount
	pstring		(char* s, const int maxCount = -1, const char* initString = NULL); 
	void init	(char* s, const int maxCount = -1, const char* initString = NULL);
	void alloc	(const int maxCount, const char* initString = NULL) { init(new char[maxCount], maxCount, initString); }
	void free	() { if (NULL != m_string) { delete[] m_string; m_string = NULL; } }

	void 	copy		(const char* const s);
	void 	copy		(const char* const s, const int count);
	void	memcpy		(const void* p, const int length);
	void 	append		(const char* s);
	void 	append		(const char* s, const int count);
	void 	append		(const char	c);
	int 	compare		(const char* s, bool ignoreCase = false) const	{ return compare(s, max_size(), ignoreCase); }
	int		compare		(const char* const source, const int length, bool ignoreCase = false) const;
	//int 	compare		(const pstring& source, bool ignoreCase = false) const { return compare(source.c_str(), ignoreCase);  }
	void	erase		(const int startIndex = 0, const int length = -1);
	void	clear		()	{ ::memset(m_string, 0, MAX_COUNT); }
	int		format			(const char* const format, ...);	//返回新写入的字符数，不包含'\0'。当越界时，windows下返回-1，linux下返回“假如没有越界，应当写入的总字符数”
	int		format_append	(const char* const format, ...);
	int		format_arg		(const char* const format, va_list arg);
	int		format_arg_append(const char* const format, va_list arg);
	int		find			(const char c, const int startIndex = 0)			const	{ return find_first_of(c, startIndex); }
	int		find			(const char* const s, const int startIndex = 0)		const	{ return find_first_of(s, startIndex); }
	int		find_first_of	(const char c, const int startIndex = 0)			const;	//find_first_of 返回找到的字符串的起始index，没找到返回-1
	int		find_first_of	(const char* const s, const int startIndex = 0)		const;
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
	int			length		()	const;
	bool		empty		()	const	{ return m_string[0] == 0; } // 返回字符串是否为空
	int			max_size	()	const	{ return MAX_COUNT;	}
	int			capacity	()	const	{ return max_size() - 1; }	//max length
	int			max_sizeof	()	const	{ return max_size() * sizeof(char); }
	int			sizeof_char	()	const	{ return sizeof(char); }

	//operators
	pstring& 		operator=	(const char* s			)	{ copy(s); return *this; }
	pstring& 		operator=	(const int value		)	{ from_int(value); return *this; }
	pstring& 		operator=	(const uint value		)	{ from_uint(value); return *this; }
	pstring& 		operator=	(const double value		)	{ from_double(value); return *this; }
	pstring& 		operator=	(const int64 value		)	{ from_int64(value); return *this; }
	pstring& 		operator=	(const uint64 value		)	{ from_uint64(value); return *this; }
	pstring& 		operator+=	(const char* s			)	{ append(s); return *this; }
	pstring& 		operator+=	(const pstring& s		)	{ append(s.c_str()); return *this; }
	pstring& 		operator+=	(const char  c			)	{ append(c); return *this; }
	bool 			operator==	(const char* s			) const { return compare(s) == 0; }
	bool 			operator==	(const pstring& s		) const { return compare(s.c_str()) == 0; }
	bool 			operator>	(const pstring& s		) const { return compare(s.c_str()) > 0; }
	bool 			operator<	(const pstring& s		) const { return compare(s.c_str()) < 0; }
	bool 			operator!=	(const char* s			) const { return compare(s) != 0; }
	bool 			operator!=	(const pstring& s		) const { return compare(s.c_str()) != 0; }
	char&			operator[]	(const int index)		{ if (NULL == m_string || index > capacity()) { assert(0);throw 1; } return m_string[index]; }
	const char&		operator[]	(const int index) const { if (NULL == m_string || index > capacity()) { assert(0);throw 1; } return m_string[index]; }
	void			safe_terminate()					{ m_string[max_size() - 1] = 0; }

	template <typename StreamerT> 
	inline void map(StreamerT& s) 
	{ 
		int l = length();

		//内容
		scl::buffer buf(m_string, capacity(), 0, l);
		s << buf; 
		l = buf.length();

		m_string[l] = 0;
	}
	
private:
	char*	m_string;
	int		MAX_COUNT;
};


} //namespace scl

using scl::pstring;

