#include "Engine/Core.h"
#include "Engine/core/Resource/FileSystem.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "Engine/core/main/Root.h"
#include "engine/core/Util/PathUtil.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifndef  ECHO_PLATFORM_WINDOWS
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <fnmatch.h>

/* Our simplified data entry structure */
struct _finddata_t
{
	char *name;
	int attrib;
	unsigned long size;
};

#define _A_NORMAL 0x00  /* Normalfile-Noread/writerestrictions */
#define _A_RDONLY 0x01  /* Read only file */
#define _A_HIDDEN 0x02  /* Hidden file */
#define _A_SYSTEM 0x04  /* System file */
#define _A_ARCH   0x20  /* Archive file */
#define _A_SUBDIR 0x10  /* Subdirectory */

intptr_t _findfirst(const char *pattern, struct _finddata_t *data);
int _findnext(intptr_t id, struct _finddata_t *data);
int _findclose(intptr_t id);
char* myStrdup(const char* s )
{
    char* t = NULL;
    if(s && (t = ECHO_ALLOC_T(char, strlen(s)+1)))
        strcpy(t,s);
    return t;
}

struct _find_search_t
{
	char *pattern;
	char *curfn;
	char *directory;
	int dirlen;
	DIR *dirfd;
};

intptr_t _findfirst(const char *pattern, struct _finddata_t *data)
{
	_find_search_t *fs = ECHO_NEW_T( _find_search_t );
	fs->curfn = NULL;
	fs->pattern = NULL;

	// Separate the mask from directory name
	const char *mask = strrchr (pattern, '/');
	if (mask)
	{
		fs->dirlen = mask - pattern;
		mask++;
		fs->directory = ECHO_ALLOC_T(char, fs->dirlen + 1);
		memcpy (fs->directory, pattern, fs->dirlen);
		fs->directory [fs->dirlen] = 0;
	}
	else
	{
		mask = pattern;
		fs->directory = myStrdup (".");
		fs->dirlen = 1;
	}

	fs->dirfd = opendir (fs->directory);
	if (!fs->dirfd)
	{
		_findclose ((intptr_t)fs);
		return -1;
	}

	/* Hack for "*.*" -> "*' from DOS/Windows */
	if (strcmp (mask, "*.*") == 0)
		mask += 2;
	fs->pattern = myStrdup (mask);

	/* Get the first entry */
	if (_findnext ((intptr_t)fs, data) < 0)
	{
		_findclose ((intptr_t)fs);
		return -1;
	}

	return (intptr_t)fs;
}

int _findnext(intptr_t id, struct _finddata_t *data)
{
	_find_search_t *fs = (_find_search_t *)id;

	/* Loop until we run out of entries or find the next one */
	dirent *entry;
	for (;;)
	{
		if (!(entry = readdir (fs->dirfd)))
			return -1;

		/* See if the filename matches our pattern */
		if (fnmatch (fs->pattern, entry->d_name, 0) == 0)
			break;
	}

	if (fs->curfn)
		ECHO_FREE (fs->curfn);
	data->name = fs->curfn = myStrdup (entry->d_name);

	size_t namelen = strlen (entry->d_name);
	char *xfn = ECHO_ALLOC_T(char, fs->dirlen + 1 + namelen + 1);
	sprintf (xfn, "%s/%s", fs->directory, entry->d_name);

	/* stat the file to get if it's a subdir and to find its length */
	struct stat stat_buf;
	if (stat (xfn, &stat_buf))
	{
		// Hmm strange, imitate a zero-length file then
		data->attrib = _A_NORMAL;
		data->size = 0;
	}
	else
	{
		if (S_ISDIR(stat_buf.st_mode))
			data->attrib = _A_SUBDIR;
		else
			/* Default type to a normal file */
			data->attrib = _A_NORMAL;

		data->size = (unsigned long)stat_buf.st_size;
	}

    ECHO_FREE(xfn);

	/* Files starting with a dot are hidden files in Unix */
	if (data->name [0] == '.')
		data->attrib |= _A_HIDDEN;

	return 0;
}

int _findclose(intptr_t id)
{
	int ret;
	_find_search_t *fs = (_find_search_t *)id;

	ret = fs->dirfd ? closedir (fs->dirfd) : 0;
	ECHO_FREE (fs->pattern);
	ECHO_FREE (fs->directory);
	if (fs->curfn)
		ECHO_FREE (fs->curfn);

    ECHO_DELETE_T(fs, _find_search_t);

	return ret;
}

#endif

#ifdef ECHO_PLATFORM_WINDOWS
#  define WIN32_LEAN_AND_MEAN
#  if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#  include <Windows.h>
#  include <direct.h>
#  include <io.h>
#endif


namespace Echo
{

	//-----------------------------------------------------------------------
	FileSystemArchive::FileSystemArchive(const String& name, const String& archType )
		: Archive(name, archType)
	{
	}
	//-----------------------------------------------------------------------
	bool FileSystemArchive::isCaseSensitive(void) const
	{
		return true;
	}
	//-----------------------------------------------------------------------
	static bool is_absolute_path(const char* path)
	{
#ifdef ECHO_PLATFORM_WINDOWS
		if (isalpha(ui8(path[0])) && path[1] == ':')
			return true;
#endif
		return path[0] == '/' || path[0] == '\\';
	}
	
	//-----------------------------------------------------------------------
	FileSystemArchive::~FileSystemArchive()
	{
		unload();
	}

	// 加载
	void FileSystemArchive::load()
	{
		StringArray strs;
		mName = PathUtil::IsRelativePath( mName) ? Root::instance()->getRootPath() + mName : mName;
		if (!Echo::PathUtil::IsDirExist(mName.c_str()))
		{
			EchoLogError("Archive(FileSystem) : [%s] isn't exist!", mName.c_str());
			return;
		}

		Echo::PathUtil::FormatPath(mName);
		PathUtil::EnumFilesInDir(strs, mName, false, true, true);

		// 记录名称与路径映射
		for( size_t i=0; i<strs.size(); i++)
		{
			String fileName = PathUtil::GetPureFilename( strs[i]);
			StringUtil::LowerCase( fileName);
			if( m_files.find(fileName)==m_files.end())
				m_files[fileName] = strs[i];
			else
				EchoLogError( "file [%s] is not unique", fileName.c_str());
		}
	}

	// 卸载
	void FileSystemArchive::unload()
	{
	}
	//-----------------------------------------------------------------------
	String FileSystemArchive::location(const String& filename)
	{
		return m_files[filename];
	}

	// 打开资源
	DataStream* FileSystemArchive::open(const String& filename)
	{
		String fullPath = location( filename);

		// Use filesystem to determine size 
		// (quicker than streaming to the end and back)
		struct stat tagStat;
		int ret = stat(fullPath.c_str(), &tagStat);
		assert(ret == 0);
		(void)ret;  // Silence warning

		std::ifstream* origStream = EchoNew(std::ifstream);
		origStream->open(fullPath.c_str(), std::ios::in | std::ios::binary);

		if(origStream->fail())
		{
			EchoSafeDelete(origStream, basic_ifstream);
			EchoLogError("Error: Cannot open file: %s in FileSystemArchive::open[%s]", filename.c_str(), strerror(errno));

			return NULL;
		}
		FileStreamDataStream* stream = EchoNew(FileStreamDataStream(filename, origStream, tagStat.st_size, true));

		return stream;
	}


	//-----------------------------------------------------------------------
	StringArray FileSystemArchive::list(bool recursive, bool dirs)
	{
		StringArray strs; strs.reserve( m_files.size());
		for( map<String, String>::type::iterator it=m_files.begin(); it!=m_files.end(); it++)
		{
			strs.push_back( it->first);
		}

		return strs;
	}
	//-----------------------------------------------------------------------
	FileInfoList* FileSystemArchive::listFileInfo(bool dirs)
	{
		FileInfoList* ret = EchoNew(FileInfoList);

		//findFiles("*", dirs, 0, ret);

		return ret;
	}
	//-----------------------------------------------------------------------
	bool FileSystemArchive::exists(const String& filename)
	{
		String full_path = location( filename);

		struct stat tagStat;
		bool ret = (stat(full_path.c_str(), &tagStat) == 0);

		// stat will return true if the filename is absolute, but we need to check
		// the file is actually in this archive
		if (ret && is_absolute_path(filename.c_str()))
		{
			// only valid if full path starts with our base
#ifdef ECHO_PLATFORM_WINDOWS
			// case insensitive on windows
			String lowerCaseName = mName;
			StringUtil::LowerCase(lowerCaseName);
			ret = StringUtil::StartWith(full_path, lowerCaseName, true);
#else
			// case sensitive
			ret = StringUtil::StartWith(full_path, mName);
#endif
		}

		return ret;
	}

	// 添加文件
	bool FileSystemArchive::addFile(const String& fullPath)
	{
		String fileName = PathUtil::GetPureFilename(fullPath);
		StringUtil::LowerCase(fileName);
		if (m_files.find(fileName) == m_files.end())
		{
			m_files[fileName] = fullPath;
			return true;
		}
		else
		{
			EchoLogError("FileSystemArchive::addFile [%s] failed", fileName.c_str());
			return false;
		}
	}

	// 移除文件
	bool FileSystemArchive::removeFile(const char* _fileName)
	{
		String fileName = _fileName;
		StringUtil::LowerCase(fileName);

		map<String, String>::type::iterator it = m_files.find( fileName);
		if ( it != m_files.end())
		{
			m_files.erase(it);
			return true;
		}
		else
		{
			EchoLogError("FileSystemArchive::removeFile [%s] failed", fileName.c_str());
			return false;
		}
	}

	// 获取类型
	const String& FileSystemArchiveFactory::getType(void) const
	{
		static String name = "filesystem";
		return name;
	}
}
