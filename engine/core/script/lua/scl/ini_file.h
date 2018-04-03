#pragma once

#include "scl/type.h"
#include "scl/varray.h"
#include "scl/string.h"
#include "scl/file.h"

namespace scl {

////////////////////////////////////
//	ini文件读取
//	注意！内部使用了动态内存分配
////////////////////////////////////
class ini_file
{
public:
	ini_file();
	virtual ~ini_file();

	//wchar
	bool	open		(const char* const fileName, const char* mode);
	bool	open_buffer	(const char* const buffer, const int buffer_len);
	void	get_string	(const char* sectionName, const char* keyName, char* output, const int outputMaxSize);
	int		get_int		(const char* sectionName, const char* keyName, const int defaultvalue = 0);
	uint	get_uint	(const char* sectionName, const char* keyName, const uint defaultvalue = 0);
	float	get_float	(const char* sectionName, const char* keyName, const float defaultvalue = 0);
	int64	get_int64	(const char* sectionName, const char* keyName, const int64 defaultvalue = 0);
	uint64	get_uint64	(const char* sectionName, const char* keyName, const uint64 defaultvalue = 0);
	bool	is_open		() const { return m_string != NULL; }

private:
	class key_value;
	class section;

	void				_parse					(const char* buffer, const int bufferSize);
	const key_value*	_get_key_value_position	(const char* const sectionName, const char* const keyName);

public:
	static const char* const GLOBAL_SECTION_NAME;

private:
	static const int	MAX_SECTION_COUNT		= 256;
	static const int	GLOBAL_SECTION_INDEX	= 0;
	
	char*					m_string;
	scl::varray<section>	m_sections;
};

} //namespace scl




