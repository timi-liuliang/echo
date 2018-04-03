////////////////////////////////////////////////////////////////////////////////
//	file.cpp
//	File类实现
//	2010.09.03 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/file.h"
#include "scl/assert.h"
#include "scl/string.h"
#include "scl/wstring.h"
#include "scl/path.h"
#include <stdio.h>

#include <sys/types.h> 
#include <sys/stat.h>
#include <memory.h>

#ifdef SCL_WIN
#include <io.h>
#include <errno.h>
#include <Windows.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
#include <stdlib.h>
#include <wchar.h>
#include <unistd.h>
#endif

#if defined(SCL_ANDROID)
#include "scl/android.h"
#include <unistd.h>
#endif

namespace scl {

file::file()
{
	m_file			= NULL;
	m_size			= 0;
	m_line_count	= 0;
	m_linefeed_type = LINEFEED_TYPE_INVALID;
}

file::~file()
{
	close();
}

////////////////////////////////////////////////////////////////////////////////
//	Open函数(Ansi)
////////////////////////////////////////////////////////////////////////////////
bool file::open(const char* const name, const char* const mode)
{
	if (m_file)
	{
		//重复打开文件
		assert(0);
		return false;
	}

	//处理不同平台下的编码问题，因为::fopen函数总是假设传入的char* filename是操作系统使用的编码格式
#ifdef SCL_WIN
	//static const int MAX_PATH = 260;
	//static const int MAX_MODE = 128;
	wstringPath wname;
	wname.from_ansi(name);
	wstring128 wmode;
	wmode.from_ansi(mode);
	_wfopen_s(reinterpret_cast<FILE**>(&m_file), wname.c_str(), wmode.c_str());
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined (SCL_ANDROID) || defined(SCL_HTML5)

#ifdef _SCL_ENCODING_GBK_
	wstringPath wfilename;
	wfilename.from_gbk(name);
	stringPath utf8filename;
	wfilename.to_utf8(utf8filename.c_str(), utf8filename.capacity());
	m_file = fopen(utf8filename.c_str(), mode);
#else // using utf8
	m_file = fopen(name, mode);
#endif

#endif

	if (NULL == m_file)
	{
		return false;
	}

	return true;
}


bool file::close()
{
	if (!m_file)
		return false;
	fclose(static_cast<FILE*>(m_file));
	m_file = NULL;
	m_size = 0;
	m_line_count = 0;
	return true;
}

bool file::eof()
{
	return 0 != feof(static_cast<FILE*>(m_file));
}

int file::read(void* data, const int elementCount, const int elementSize) const
{
	if (!m_file)
	{
		return false;
	}
	int readElementCount = static_cast<int>(::fread(data, elementSize, elementCount, static_cast<FILE*>(m_file)));
	return readElementCount * elementSize;
}

//int	file::ReadStringAll(void* buffer, const int bufferSize)
//{
//	//跳过UTF16的"FF FE"文件头
//	const int UTF16HeaderSize = 2;
//	seek(UTF16HeaderSize, file::SEEK_POSITION_BEGIN);
//	int fileSize = GetSize();
//	if (fileSize > bufferSize)
//	{
//		assert(false);
//		return 0;
//	}
//	return read(buffer, sizeof(byte), fileSize - UTF16HeaderSize);
//}

int file::write(const void* data, const int elementCount, const int elementSize)
{
	if (!m_file)
	{
		return false;
	}
	int writeCount = 0;
	writeCount += _inner_write(data, elementSize, elementCount);
	return writeCount;
}

//int file::writeString(const wchar* message, const int stringLength)
//{
//	//写入宽字符串之前检查文件是否完全为空，
//	//如果为空就写入utf16文件头
//	int writeCount = 0;
//	if (size() == 0)
//	{
// 		byte utfHead[2] = { 0 };
// 		utfHead[0] = 0xFF;
// 		utfHead[1] = 0xFE;
//		writeCount += _innerWrite(utfHead, 1, 2);
//	}
//	for (int i = 0; i < stringLength; ++i)
//	{
//		int16 utf16Char = static_cast<int16>(message[i]);
//		writeCount += _innerWrite(&utf16Char, sizeof(int16), 1);
//	}
//	return writeCount;
//}

int file::write_string(const char* message, const int stringLength)
{
	int writeCount = _inner_write(message, sizeof(char), stringLength);
	return writeCount;
}

int file::read_string(wchar* message, const int maxCount)
{
	if (!m_file)
	{
		assert(false);
		return false;
	}
	skip_bom();

#ifdef SCL_WIN
	int readCount = read(message, maxCount, sizeof(wchar));
	message[readCount / sizeof(wchar)] = 0;
	return readCount / sizeof(wchar);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE)
	uint16* p16 = reinterpret_cast<uint16*>(message);
	uint16* p16ReadStart = p16 + maxCount;
	int readCount = read(p16ReadStart, maxCount, sizeof(uint16));
	for (int i = 0; i < readCount; ++i)
	{
		message[i] = p16ReadStart[i];
	}
	message[readCount / sizeof(uint16)] = 0;
	return readCount / sizeof(uint16);
#endif
}

int file::seek(int shift, SEEK_POSITION startPosition)
{
	if (!m_file)
	{
		assert(0);
		return 0;
	}
	int result = fseek(static_cast<FILE*>(m_file), shift, startPosition);
	assert(result == 0);
	return result;
}

//bool file::getLine(wchar* data, const int maxCount)
//{
//	return NULL != fgetws(data, maxCount, static_cast<FILE*>(m_file));	
//}

bool file::get_line(char* data, const int maxCount)
{
	return NULL != fgets(data, maxCount, static_cast<FILE*>(m_file));	
}

//使用全局对象，避免占用局部栈空间
const int MAX_LINE_COUNT_TEMP_STRING_LENGTH = 16 * 1024;
wchar lineCountTempString[MAX_LINE_COUNT_TEMP_STRING_LENGTH];

int file::line_count()
{
	if (!m_file)
		return 0;
	if (m_line_count > 0)
		return m_line_count;

	const int MAX_TEMP_BUFFER_SIZE = 1024 * 32;
	m_line_count = 1;
	char temp[MAX_TEMP_BUFFER_SIZE] = { 0 };
	bool fileIsEmpty = true;
	int oldPosition = static_cast<int>(::ftell(static_cast<FILE*>(m_file)));
	seek(0, SEEK_POSITION_BEGIN);
	int readCount = static_cast<int>(::fread(temp, sizeof(temp[0]), MAX_TEMP_BUFFER_SIZE, static_cast<FILE*>(m_file)));
	while (readCount > 0)
	{
		fileIsEmpty = false;
		for (int i = 0; i < readCount; ++i)
		{
			char c = temp[i];
			if (c != '\n' && c != '\r')
				continue;
			char next = 0;
			if (i >= readCount - 1) //buffer中的最后一个字符是\r或者\n,此时需要从文件中再读一个字符
			{
				int nread = static_cast<int>(::fread(&next, sizeof(next), 1, static_cast<FILE*>(m_file)));
				if (nread > 0)
					fseek(static_cast<FILE*>(m_file), -nread, SEEK_CUR);
				else if(feof(static_cast<FILE*>(m_file)))
					continue; //文件已经没有内容了，最后一个换行符不计算在内
			}
			else
				next = temp[i + 1];

			char prev  = 0;
			if (i > 0) prev = temp[i - 1];

			if (c == '\n')
			{
				++m_line_count; //windows/linux换行符
			}
			else if (c == '\r')
			{
				if (prev != '\n' && next != '\n') //macOS换行符
					++m_line_count;
			}
		}
		memset(temp, 0, sizeof(temp));
		readCount = static_cast<int>(::fread(temp, sizeof(temp[0]), MAX_TEMP_BUFFER_SIZE, static_cast<FILE*>(m_file)));
	}
	seek(oldPosition, SEEK_POSITION_BEGIN);
	return fileIsEmpty ? 0 : m_line_count;
}

void file::flush()
{
	::fflush(static_cast<FILE*>(m_file));
}

int file::tell()
{
	return static_cast<int>(::ftell(static_cast<FILE*>(m_file)));
}

uint64 file::size()
{
	if (!m_file)
		return 0;

	if (m_size > 0)
		return m_size;

	m_size = 0;

	int oldPosition = tell();
	
	//将缓冲区内容写入文件，以便能够获取fileStat
	fflush(static_cast<FILE*>(m_file));
	struct stat fileStat;

#ifdef SCL_WIN
	int result = ::fstat(_fileno(static_cast<FILE*>(m_file)), &fileStat);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	int result = ::fstat(fileno(static_cast<FILE*>(m_file)), &fileStat);
#endif

	if( result != 0 )
	{
		assert(false);
		return 0;
		//if (errno == EBADF)
		//	printf( "Bad file descriptor.\n" );
		//else if (errno == EINVAL)
		//	printf( "Invalid argument to _fstat.\n" );
	}
	m_size = fileStat.st_size;

	seek(oldPosition, SEEK_POSITION_BEGIN);

	assert(m_size >= 0);
	return m_size;
}

int file::_inner_write(const void* data, const int elemSize, const int elemCount)
{
	int writeCount = static_cast<int>(::fwrite(data, elemSize, elemCount, static_cast<FILE*>(m_file)));
	_add_size(writeCount * elemSize);
	return writeCount;
}

bool file::skip_bom()
{
	//跳过utf16和utf8的BOM
	seek(0, SEEK_POSITION_BEGIN);
	byte bom[3] = { 0 };
	read(bom, 2);
	if ((0xFF == bom[0] && 0xFE == bom[1])		//utf16 LE
		|| (0xFE == bom[0] && 0xFF == bom[1]))	//utf16 BE
	{
		return true;
	}
	else if (bom[0] == 0xEF && bom[1] == 0xBB) //maybe utf8 or maybe gbk
	{
		read(bom + 2, 1);
		if (bom[2] == 0xBF)
		{
			return true;
		}
	}
	seek(0, SEEK_POSITION_BEGIN);
	return false;
}

bool file::exists(const wchar* const fileName)
{

#ifdef SCL_WIN
	return (_waccess(fileName, 0) == 0);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	const int MAX_PATH = 512;
	char ansiFileName[MAX_PATH] = { 0 };
	wcstombs(ansiFileName, fileName, MAX_PATH);
	return (access(ansiFileName, 0) == 0);
#endif

}

bool file::exists(const char* const fileName)
{

#ifdef SCL_WIN
	wstringPath wfilename;
	wfilename.from_ansi(fileName);
	//return (_access(fileName, 0) == 0);
	return (_waccess(wfilename.c_str(), 0) == 0);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
	return (access(fileName, 0) == 0);
#endif

}

int file::get_linefeed_size(LINEFEED_TYPE type)
{
	switch (type)
	{
	case LINEFEED_TYPE_INVALID:
		return 0;
	case LINEFEED_TYPE_LINUX:
	case LINEFEED_TYPE_MACOS:
		return 1;
	case LINEFEED_TYPE_WINDOWS:
		return 2;
	default:
		assert(false);
		break;
	};
	return 0;
}

file::LINEFEED_TYPE file::get_linefeed_type()
{
	if (!m_file)
		return LINEFEED_TYPE_INVALID;

	static const int	MAX_TEMP_BUFFER_SIZE = 1024;
	static char			temp[MAX_TEMP_BUFFER_SIZE];

	int oldPosition = tell();
	seek(0, SEEK_POSITION_BEGIN);

	int readCount = 0;
	while (memset(temp, 0, sizeof(temp))
		   && (readCount = static_cast<int>(::fread(temp, sizeof(temp[0]), MAX_TEMP_BUFFER_SIZE, static_cast<FILE*>(m_file)))))
	{
		for (int i = 0; i < readCount; ++i)
		{
			char c = temp[i];
			if (c != '\n' && c != '\r')
				continue;

			char next = 0;
			if (i + 1 < readCount) 
				next = temp[i + 1];

			if (c == '\n')
			{
				if (next == '\r')
					m_linefeed_type = LINEFEED_TYPE_WINDOWS;	//windows/linux换行符
				else
					m_linefeed_type = LINEFEED_TYPE_LINUX;		//windows/linux换行符

				seek(oldPosition, SEEK_POSITION_BEGIN);
				return m_linefeed_type;
			}
			else if (c == '\r')
			{
				if (next == '\n')
					m_linefeed_type = LINEFEED_TYPE_WINDOWS;	//windows/linux换行符
				else
					m_linefeed_type = LINEFEED_TYPE_MACOS;		//windows/linux换行符

				seek(oldPosition, SEEK_POSITION_BEGIN);
				return m_linefeed_type;
			}
		}
	}
	
	return LINEFEED_TYPE_INVALID;
}

bool file::remove(const char* const filename)
{
#ifdef SCL_WIN
	wstringPath wfilename;
	wfilename.from_ansi(filename);
	return (DeleteFileW(wfilename.c_str()) != 0);
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
    return (::remove(filename) == 0);
#endif
}

bool file::rename(const char* const oldname, const char* const newname)
{
#ifdef SCL_WIN
	wstringPath w_old;
	wstringPath w_new;
	w_old.from_ansi(oldname);
	w_new.from_ansi(newname);

	BOOL r = MoveFileW(w_old.c_str(), w_new.c_str());
	return r != 0;
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID)
    return (::rename(oldname, newname) == 0);
#endif
}

////////////////////////////////////
//规范化一个路径名
//将所有的'\'替换为'/',同时,当autoAddSlash为true, 且必要的时候，会在末尾添加'/'
////////////////////////////////////
void normalize_path(pstring path, bool auto_add_slash)
{
	char* p = path.c_str();
	while (*p)
	{
		if (*p == '\\')
			*p = '/';
		++p;
	}
	const int len = static_cast<int>(p - path.c_str());
	if (auto_add_slash && *(p - 1) != '/' && len + 1 < path.capacity())
	{
		*p = '/';
		*(p + 1) = 0;
	}
}

////////////////////////////////////
//从路径中提取文件名（withExt参数表示是否保留扩展名）
////////////////////////////////////
void extract_filename(pstring fullname, bool with_ext)
{
	int name_start	= 0;		//不带路径的文件名起始位置
	int ext_dot		= - 1;		//扩展名分隔符位置
	char* pfullname = fullname.c_str();
	char* p = pfullname;
	while (*p)
	{
		if (*p == '.')	
			ext_dot = static_cast<int>(p - pfullname);
		if (*p == '\\' || *p == '/')
			name_start = static_cast<int>(p - pfullname + 1);
		++p;
	}

	const int len = static_cast<int>(p - pfullname);
	if (ext_dot == -1)
		ext_dot = len;
	int copy_len = len - name_start;
	if (!with_ext)
		copy_len -= len - ext_dot;
	char* cp = pfullname;
	for (int i = 0; i < copy_len; ++i)
		cp[i] = cp[i + name_start];		
	cp[copy_len] = 0;
}

////////////////////////////////////
//从路径中提取文件名，不保留扩展名
////////////////////////////////////
void extract_filename_no_ext(pstring fullname)
{
	extract_filename(fullname, false);
}

////////////////////////////////////
//从路径中提取扩展名，注意，结果中不包括“.”，例如“a.txt”，返回结果是“txt”而不是“.txt”
////////////////////////////////////
void extract_fileext(pstring fullname)
{
	extract_fileext_to(fullname, fullname);
}

////////////////////////////////////
//从路径中提取扩展名
////////////////////////////////////
void extract_fileext_to(pstring _fullname, pstring ext)
{
	int ext_dot = -1;		//扩展名分隔符位置
	char* fullname = _fullname.c_str();
	char* p = fullname;
	while (*p)
	{
		if (*p == '.')
			ext_dot = static_cast<int>(p - fullname);
		if (*p == '\\' || *p == '/') // 如果出现了路径分隔符，则之前找到的所有dot都不可能是扩展名的dot
			ext_dot = -1;
		++p;
	}

	const int len = static_cast<int>(p - fullname);
	if (ext_dot == -1)
		return;
	int copylen = len - ext_dot - 1;
	if (copylen > ext.capacity())
		copylen = ext.capacity();
	char* dst = ext.c_str();
	char* src = fullname;
	for (int i = 0; i < copylen; ++i)
		dst[i] = src[i + ext_dot + 1];
	dst[copylen] = 0;;
}

////////////////////////////////////
//从路径中提取路径名，不包含文件名
////////////////////////////////////
void extract_path(pstring filename, bool add_current_path)
{
	char* last_slash = NULL;
	char* p = filename.c_str();
	while (*p)
	{
		if (*p == '\\' || *p == '/')
			last_slash = p;
		++p;
	}
	if (NULL != last_slash && last_slash - p + 1 < filename.capacity())
		*(last_slash + 1) = 0;
	else
	{
		if (add_current_path)
			scl::strcpy(filename.c_str(), filename.capacity(), "./");	//没有找到分隔符，说明路径名只包含文件名，返回当前路径
		else
			filename.clear();
	}
}

void get_exe_name(pstring exename)
{
	//第一次打开文件，需要顺序查找所有的文件
	stringPath exe_full_name;

#ifdef SCL_WIN
	GetModuleFileNameA(NULL, exe_full_name.c_str(), exe_full_name.capacity()); 
#endif

#if defined(SCL_LINUX)
	readlink("/proc/self/exe", exe_full_name.c_str(), exe_full_name.capacity());
#endif

#if defined(SCL_APPLE)
    get_application_path(exe_full_name.c_str(), exe_full_name.capacity());
#endif

#if defined(SCL_ANDROID)
	get_android_package_name(exe_full_name.c_str(), exe_full_name.capacity());
#endif

#if !defined(SCL_ANDROID)
	extract_filename(exe_full_name.pstring(), false);
#endif

	if (exe_full_name.empty())
	{
		assert(false);
		return;
	}
	exename = exe_full_name.c_str();
}



} //namespace scl
