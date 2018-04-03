#pragma once

#include "scl/type.h"
#include "scl/string.h"

namespace scl {

struct struct64_t
{
	uint32 low;
	uint32 high;
};
	
union file_time
{
	struct64_t	s64;
	uint64		i64;
};
union file_size
{
	struct64_t	s64;
	uint64		i64;
};

class file_find_data
{
public:
	file_find_data			() { clear(); }
	void clear				();
	bool is_directory		() const;	//当前文件是否为目录，不包括“.”和“..”
	bool is_any_directory	() const;	//当前文件是否为目录，包括“.”和“..”
	bool is_file			() const { return !is_any_directory(); }
	bool is_hide			() const;
	pstring name			() const { return m_name.pstring(); }
	pstring fullname		() const { return m_fullname.pstring(); }
	pstring extname			() const { return m_extname.pstring(); }

	uint32		attributes;
    file_time	creation_time;
    file_time	last_access_time;
    file_time	last_write_time;
	file_size	size;

    //string<14>  alternateName;

private:
	stringPath  m_name;
	string32	m_extname;
	stringPath	m_fullname;
};

////////////////////////////////////
// directory
////////////////////////////////////
class directory
{
public:
	directory();
	~directory();

	bool					open	(const char* const directoryName);
	bool					next	();
	const	file_find_data& current	() const { return m_data; }
	pstring					name	() { return m_name.pstring(); }
	bool					create	(const char* const dirname);
	bool					exists	(const char* const subDirName);
	//static bool exists();

	static bool create_dir(const char* const dirname);

private:
	void*			m_handle;
	file_find_data	m_data;
#ifdef SCL_WIN
	bool			m_readingFirst;
#endif
	stringPath		m_name;
};

} //namespace scl


