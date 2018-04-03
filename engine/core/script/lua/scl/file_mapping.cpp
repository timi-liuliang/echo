#include "scl/file_mapping.h"

#include "scl/type.h"
#include "scl/assert.h"

#ifdef SCL_WIN
#include <Windows.h>
#endif

#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <memory.h>
#endif

namespace scl {


file_mapping::file_mapping() :
	m_buffer		(NULL),
	m_auto_close	(true),
	m_size			(0),
	m_seek			(0),
#ifdef SCL_WIN
	m_file_handle	(NULL),
	m_mapping_handle(NULL)
#elif defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	m_file			(-1)
#endif
{

}

file_mapping::~file_mapping()
{
	if (m_auto_close)
		close();
}

bool file_mapping::open(const char* const filename)
{
	assert(!is_open());	

#ifdef SCL_WIN
	wchar_t wfilename[MAX_PATH] = { 0 };
	const int cr = MultiByteToWideChar(CP_UTF8, 0, filename, strlen(filename), wfilename, MAX_PATH - 1);
	if (cr == 0)
		return false;
	HANDLE file = CreateFileW(wfilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		int r = GetLastError();
		assertf(false, "open file failed. file = [%s]", filename);
		return false;
	}

	m_file_handle = file;

	int sz = _get_file_size();
	if (sz <= 0)
		return false;

	HANDLE mh = CreateFileMapping(file, NULL, PAGE_READONLY, 0, 0, NULL);
	if (mh == NULL)
	{
		int r = GetLastError();
		assert(false);
		return false;
	}
	m_mapping_handle = mh;
#endif
	
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	int fd = ::open(filename, O_RDONLY);
	if (fd == -1)
	{
		perror("");
		return false;
	}
	m_file = fd;
#endif

	m_size = _get_file_size();
	return true;
}

void* file_mapping::map()
{
#ifdef SCL_WIN
	void* addr = MapViewOfFile(m_mapping_handle, FILE_MAP_READ, 0, 0, 0);
	m_buffer = addr;
	return addr;
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	int _size = size();
	if (_size <= 0)
		return NULL;
	void* addr = mmap(NULL, _size, PROT_READ, MAP_PRIVATE, m_file, 0);
	if (addr == MAP_FAILED)
	{
		perror("");
		return NULL;
	}
	m_buffer = addr;
	return addr;
#endif
}

void file_mapping::close()
{
#ifdef SCL_WIN
	if (NULL != m_buffer)
	{
		int r = UnmapViewOfFile(m_buffer);
		assert(r != 0);
		m_buffer = NULL;
	}
	if (NULL != m_mapping_handle)
	{
		CloseHandle(m_mapping_handle);
		m_mapping_handle = NULL;
	}
	if (NULL != m_file_handle)
	{
		CloseHandle(m_file_handle);
		m_file_handle = NULL;
	}
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	if (NULL != m_buffer)
	{
		munmap(m_buffer, size());
		m_buffer = NULL;
	}
	if (m_file >= 0)
	{
		::close(m_file);
		m_file = -1;
	}
#endif
}

bool file_mapping::is_open() const
{
#ifdef SCL_WIN
	return m_file_handle != NULL;
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) || defined(SCL_HTML5)
	return m_file != -1;
#endif
}

int file_mapping::_get_file_size() const
{
#ifdef SCL_WIN
	return GetFileSize(m_file_handle, NULL);
#endif
#if defined(SCL_LINUX) || defined(SCL_APPLE) || defined(SCL_ANDROID) ||defined(SCL_HTML5)
	int fd = m_file;
	struct stat sb;
	if (fstat(fd, &sb) == -1)
	{
		perror("");
		return 0;
	}
	return static_cast<int>(sb.st_size);
#endif
}

int file_mapping::read(void* data, const int _len) const
{
	int len = (_len + m_seek > m_size) ? (m_size - m_seek) : _len;
	memcpy(data, static_cast<byte*>(m_buffer) + m_seek, len);
	return len;
}

void file_mapping::seek(const SEEK start, const int offset)
{
	int pos = 0;
	if (start == CUR)
		pos = m_seek;
	if (start == END)
		pos = m_size - 1;
	m_seek = pos + offset;
}


} //namespace scl
