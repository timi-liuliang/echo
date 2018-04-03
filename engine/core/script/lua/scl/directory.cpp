#include "scl/directory.h"

#include "scl/string.h"
#include "scl/file.h"
#include "scl/pwstring.h"
#include "scl/wstring.h"
#include "scl/encoding.h"

#include <sys/stat.h>

#ifdef SCL_WIN
#include <Windows.h>
#include <direct.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#endif

namespace scl {

void file_find_data::clear()
{
	attributes				= 0;
    creation_time.i64		= 0;
    last_access_time.i64	= 0;
    last_write_time.i64		= 0;
    size.i64				= 0;
    m_name.clear();
	m_extname.clear();
}



directory::directory() 
	: m_handle(NULL)
#ifdef SCL_WIN
	, m_readingFirst(false)
#endif
{

}

//NOT Tested尚未测试
bool directory::create(const char* const dirname)
{
	if (NULL != dirname && 0 == dirname[0])
		return false;
	if (exists(dirname))
		return false;

	stringPath fullSubDirName = name().c_str();
	fullSubDirName += dirname;
#ifdef SCL_WIN
	if (_mkdir(fullSubDirName.c_str()) == 0)
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	if (mkdir(fullSubDirName.c_str(), 0777) == 0) //给予全部权限
#endif
	{
		return true;
	}
	else
	{
		//if (EEXIST == errno) Directory was not created because dirname is the name of an existing file, directory, or device.
		//if (ENOENT == errno) Path was not found.
		return false;
	}
}

//NOT Tested尚未测试
bool directory::create_dir(const char* const dirname)
{
	if (NULL == dirname)
		return false;
	if (0 == dirname[0])
		return false;
	if (file::exists(dirname))
		return false;

#ifdef SCL_WIN
	wstringPath wdirname;
	wdirname.from_ansi(dirname);
	if (_wmkdir(wdirname.c_str()) == 0)
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	if (mkdir(dirname, 0777) == 0) //给予全部权限
#endif
	{
		return true;
	}
	else
	{
		//if (EEXIST == errno) Directory was not created because dirname is the name of an existing file, directory, or device.
		//if (ENOENT == errno) Path was not found.
		return false;
	}
}

//NOT Tested尚未测试
bool directory::exists(const char* const subDirName)
{
	stringPath fullSubDirName = name().c_str();
	fullSubDirName += subDirName;
	if (file::exists(fullSubDirName.c_str()))
	{
		//打开文件属性，看是否为directory
#ifdef SCL_WIN
		struct _stat filestat;
		_stat(fullSubDirName.c_str(), &filestat);
		if (filestat.st_mode & _S_IFDIR)
		{
			return true;
		}
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE)
		struct stat filestat;
		stat(fullSubDirName.c_str(), &filestat);
		if (S_ISDIR(filestat.st_mode))
		{
			return true;
		}
#endif
	}
	return false;
}





#ifdef SCL_WIN

////////////////////////////////////
//	tool function: _convertFileData
////////////////////////////////////
void _convertFileData(const WIN32_FIND_DATAW& from, file_find_data& to, const char* const dirname)
{
	to.clear();
	to.attributes				= from.dwFileAttributes;
    to.creation_time.s64.low	= from.ftCreationTime.dwLowDateTime;
	to.creation_time.s64.high	= from.ftCreationTime.dwHighDateTime;
    to.last_access_time.s64.low	= from.ftLastAccessTime.dwLowDateTime;
	to.last_access_time.s64.high= from.ftLastAccessTime.dwHighDateTime;
    to.last_write_time.s64.low	= from.ftLastWriteTime.dwLowDateTime;
	to.last_write_time.s64.high	= from.ftLastWriteTime.dwHighDateTime;
    to.size.s64.high			= from.nFileSizeHigh;
    to.size.s64.low				= from.nFileSizeLow;
	pwstring pwfname(const_cast<wchar*>(from.cFileName), MAX_PATH);
	pwfname.to_ansi(to.name().c_str(), to.name().capacity());
	extract_fileext_to(to.name(), to.extname());

	to.fullname() = dirname;
	to.fullname() += to.name();
}

////////////////////////////////////
//	file_find_data member functions
////////////////////////////////////
bool file_find_data::is_directory() const
{
	return is_any_directory() && m_name != "." && m_name != "..";
}

bool file_find_data::is_any_directory() const
{
	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

////////////////////////////////////
//	directory member functions
////////////////////////////////////
bool directory::open(const char* const directoryName)
{
	//置为已经读取了第一个数据
	m_readingFirst = true;
	
	//保存目录名
	m_name = directoryName;
	normalize_path(m_name.pstring());
	return true;
}

bool directory::next()
{
	WIN32_FIND_DATAW win32FindData;
	bool result = false;
	if (m_readingFirst)	//这里是为了特殊处理FindFirstFileW返回的第一个文件的信息
	{
		//转换为wchar
		wstringPath wcsname;
		wcsname.from_ansi(m_name.c_str());

		//find目录下第一个文件
		wcsname += L"\\*";
		
		m_handle = FindFirstFileW(wcsname.c_str(), &win32FindData);
		result = (m_handle != INVALID_HANDLE_VALUE);
		m_readingFirst = false;
	}
	else
	{
		result = FindNextFileW(m_handle, &win32FindData) != 0;
	}
	if (result)
	{
		_convertFileData(win32FindData, m_data, m_name.c_str());
	}

	return result;
}

bool file_find_data::is_hide() const
{
	return (attributes & FILE_ATTRIBUTE_HIDDEN) != 0;
}

directory::~directory()
{
	FindClose(m_handle);
}

#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)

////////////////////////////////////
//	tool function: _convertFileData
////////////////////////////////////
void _convertFileData(const struct stat& from, class file_find_data& to, const char* const dirname)
{
	to.clear();
	to.attributes			= from.st_mode;
	to.creation_time.i64	= from.st_ctime;
    to.last_access_time.i64	= from.st_atime;
    to.last_write_time.i64	= from.st_mtime;
    to.size.i64				= from.st_size;
}

////////////////////////////////////
//	file_find_data member functions
////////////////////////////////////
bool file_find_data::is_directory() const
{
	return is_any_directory() && m_name != "." && m_name != "..";
}


bool file_find_data::is_any_directory() const
{
	return S_ISDIR(attributes);
}

bool file_find_data::is_hide() const
{
	return m_name[0] == '.';
}

////////////////////////////////////
//	directory member functions
////////////////////////////////////
bool directory::open(const char* const directoryName)
{
	m_handle = opendir(directoryName);
	if (NULL == m_handle)
	{
		perror("directory::open");
	}
	//保存目录名
	m_name = directoryName;
	normalize_path(m_name.pstring());

	return m_handle != NULL;
}

bool directory::next()
{
	//读取下个目录
	dirent* pdata = readdir(static_cast<DIR*>(m_handle));
	if (NULL == pdata)
	{
		//if (EBADF == errno)
		//{
		//	perror("readdir() failed");
		//}
		return false;
	}

	//组成完整的路径文件名
	stringPath childName = m_name;
	childName += pdata->d_name;

	//获取file state
	struct stat buf;
	if (0 != stat(childName.c_str(), &buf))
	{
		perror("stat() failed");
		return false;
	}

	//将数据转为跨操作系统的通用的file_find_data
	_convertFileData(buf, m_data, m_name.c_str());
	m_data.name() = pdata->d_name; 
	extract_fileext_to(m_data.name(), m_data.extname());
	m_data.fullname() = m_name.c_str();
	m_data.fullname() += m_data.name().c_str();

	return true;
}

directory::~directory()
{
	closedir(static_cast<DIR*>(m_handle));
}




#endif


} //namespace scl

