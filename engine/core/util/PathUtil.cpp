#include "PathUtil.h"
#include "StringUtil.h"
#include <fstream>

#ifdef ECHO_PLATFORM_WINDOWS
#	include "engine/core/util/DirentWin32.h"
#	include <direct.h>
#	if (ECHO_COMPILER == ECHO_COMPILER_MSVC)
#		pragma warning(disable: 4996)
#	endif
#else
#	include <unistd.h>
#	include <sys/stat.h>
#	include <dirent.h>
#endif

namespace Echo
{
	const unsigned int PathUtil::MAX_PATHNAME = 1024;
	const unsigned int PathUtil::MAX_DIRNAME  = 256;
	const unsigned int PathUtil::MAX_FILENAME = 256;
	const unsigned int PathUtil::MAX_EXTNAME  = 256;

	const char PathUtil::DOT = '.';
	const char PathUtil::SEPERATOR = '/';
	const char PathUtil::SLASH = '/';
	const char PathUtil::BACKSLASH = '\\';

	const String PathUtil::INVALID_PATH = "";

	//////////////////////////////////////////////////////////////////////////

	PathUtil::PathUtil()
	{
	}

	PathUtil::~PathUtil()
	{
	}

	void PathUtil::FormatPath(String& file, bool bLower)
	{
		if(PathUtil::IsHttpFile(file))
		{
			file = StringUtil::Replace(file, BACKSLASH, SEPERATOR);
			//while(StringUtil::ReplaceRet(file, "//", "/"))
			//	continue;

			file = StringUtil::Replace(file, "http:/", "http://");
		}
		else 
		{
			bool bLan = false;
			bLan = IsLanFile(file);
			file = StringUtil::Replace(file, BACKSLASH, SEPERATOR);
			//while(StringUtil::ReplaceRet(file, "//", "/"))
			//	continue;

			if(bLan)
				file = SLASH + file;

			if( !IsFile( file) && !IsEndWithSeperator( file))
				file = file + SLASH;
		}

		if(bLower)
			StringUtil::LowerCase(file);
	}

	void PathUtil::FormatPathAbsolut(String& path, bool bLower /* = false */)
	{
		if (path.empty())
		{
			return;
		}

		path = StringUtil::Replace(path, BACKSLASH, SEPERATOR);
		StringArray pathArray = StringUtil::Split(path, "/");
		size_t size = pathArray.size();
		if (size == 0)
			return;
		int backTraceNum = 0;
		for (int i = size - 1; i >= 0; --i)
		{
			if (pathArray[i] == "..")
			{
				++backTraceNum;
				pathArray[i] = "";
			}
			else if (pathArray[i] == ".")
			{
				pathArray[i] = "";
			}
			else
			{
				if (backTraceNum > 0)
				{
					--backTraceNum;
					pathArray[i] = "";
				}
			}
		}
#ifdef ECHO_PLATFORM_WINDOWS
		String realPath = "";
#else
        String realPath = path[0]== '/' ? "/" : "";
#endif
		for (size_t i = 0; i < size-1; ++i)
		{
			if (!pathArray[i].empty())
			{
				realPath += pathArray[i];
				realPath += "/";
			}
		}

		if (!pathArray[size-1].empty())
		{
			realPath += pathArray[size - 1];
			if (!IsFile(pathArray[size - 1]))
			{
				realPath += "/";
			}
		}

		if (bLower)
			StringUtil::LowerCase(realPath);

		path = realPath;
	}

	bool PathUtil::IsFileExist(const String& file)
	{
#ifdef ECHO_PLATFORM_HTML5
		struct ::stat path_stat;
		if (::lstat(file.c_str(), &path_stat) != 0) 
		{
//			const int error = errno;
//			if (error == ENOENT) 
//			{
				// Only bother logging if something other than the path not existing
				// went wrong.
// 				std::cout
// 					<< "Failed to lstat path: " << path << "; errno=" << error << "; "
// 					<< std::strerror(error) << endl;
//			}
			return false;
		}

// 		std::cout
// 			<< std::oct << "Mode for path=" << path << ": " << path_stat.st_mode
// 			<< std::dec << endl;

		return true;
#else
		struct stat st;

		if(IsEndWithSeperator(file))
			return false;

		const char* filename = file.c_str();

		/* ignore hidden files */
		if(filename[0] == '.')
		{
			bool bRet = false;

			if(file.length() > 2)
			{
				if(filename[1] == '/' || filename[1] == '\\')
					bRet = true;
			}

			if(!bRet && file.length() > 3)
			{
				if(filename[1] == '.' && (filename[2] == '/' || filename[2] == '\\'))
					bRet = true;
			}

			if(!bRet)
				return false;
		}

		/* get dirent status */
		if(stat(filename, &st) == -1)
			return false;

		if(S_ISDIR(st.st_mode))
			return false;

		return true;
#endif
	}

	bool PathUtil::IsDirExist(const String& dir)
	{
		if(IsDriveOrRoot(dir))
			return true;

		String tempPath = dir;
		if(IsEndWithSeperator(tempPath))
		{
			tempPath = tempPath.substr(0, tempPath.length() - 1);
		}

		/* get dirent status */
        struct stat st;
		if(stat(tempPath.c_str(), &st) == -1)
			return false;

		if(S_ISDIR(st.st_mode))
			return true;

		return false;
	}

	bool PathUtil::IsHttpFile(const String& file)
	{
		String tempFile = file;
		StringUtil::LowerCase(tempFile);

		String key = "http:";
		size_t keyLen = key.length();

		if(tempFile.length() < keyLen)
			return false;

		if(tempFile.substr(0, keyLen) == key)
			return true;
		else
			return false;
	}

	bool PathUtil::IsLanFile(const String& file)
	{
		String tempFile = StringUtil::Replace(file, BACKSLASH, SEPERATOR);

		String key = "//";
		size_t keyLen = key.length();

		if(tempFile.length() < keyLen)
			return false;

		if(tempFile.substr(0, keyLen) == key)
			return true;
		else
			return false;
	}

	String PathUtil::GetHttpSafeFilePath(const String& httpfile)
	{
		String ret = httpfile;
		FormatPath(ret);
		StringUtil::Trim(ret);
		ret = StringUtil::Replace(ret, " ", "%20");
		return ret;
	}

	String PathUtil::GetHttpFileInPath(const String& httpfile)
	{
		String ret = httpfile;
		FormatPath(ret);

		size_t pos = ret.find_last_of(SEPERATOR);
		if(pos == String::npos)
			return INVALID_PATH;

		return ret.substr(0, pos + 1);
	}

	bool PathUtil::IsEndWithSlash(const String& path)
	{
		if(!path.length())
			return false;

		return path[path.length() - 1] == SLASH;
	}

	bool PathUtil::IsEndWithBackSlash(const String& path)
	{
		if(!path.length())
			return false;

		return path[path.length() - 1] == BACKSLASH;
	}

	bool PathUtil::IsEndWithSeperator(const String& path)
	{
		if(!path.length())
			return false;

		size_t pos = path.length() - 1;
		return path[pos] == SLASH || path[pos] == BACKSLASH;
	}

	// 是否是相对路径
	bool PathUtil::IsRelativePath(const String& path)
	{
		return !IsAbsolutePath( path);
	}

	// 是否为绝对路径
	bool PathUtil::IsAbsolutePath(const String& path)
	{
		// 若为空则必为相对路径
		if( path.empty())
			return false;

// 以'/'开头
#ifndef ECHO_PLATFORM_WINDOWS
		if( path[0]==SLASH)
			return true;
#endif
		if( path.find(':') != String::npos)
			return true;

		return false;
	}

	bool PathUtil::IsDir(const String& path)
	{
		if(path.length() == 0)
			return false;

		return IsEndWithSeperator(path);
	}

	bool PathUtil::IsFile(const String& path)
	{
		String pureFileName = GetPureFilename( path);
		size_t pos = pureFileName.find_last_of(DOT);
		if(pos == String::npos)
		{
			return false;
		}

		return true;
	}

	bool PathUtil::IsPureFilename(const String& path)
	{
		if(!path.length())
			return false;

		if(path[0] == DOT)
			return false;

		if(path.find(SLASH) != String::npos || path.find(BACKSLASH) != String::npos)
			return false;

		return true;
	}

	bool PathUtil::IsDriveOrRoot(const String& path)
	{
        // unix
        if(path.size()==1 && path[0] == SEPERATOR)
            return true;
        
        // windows drive, like c:/
		if((path.length() == 2 || path.length() ==3) && path[1] == ':')
			return true;

		return false;
	}

	bool PathUtil::IsFileType(const String& filename, const String& extWithDot)
	{
		size_t fileLen = filename.length();
		size_t extLen = extWithDot.length();

		if(fileLen < extLen)
			return false;

		String fileExt = filename.substr(fileLen - extLen);

		if(fileExt == extWithDot)
			return true;

		return false;
	}

	bool PathUtil::IsSameFile(const String& file1, const String& file2)
	{
		if(!IsFileExist(file1) || !IsFileExist(file2))
			return false;

		const char* szFile1 = file1.c_str();
		const char* szFile2 = file2.c_str();

		struct stat st1;
		struct stat st2;

		/* get dirent status */
		if(stat(szFile1, &st1) == -1 || stat(szFile2, &st2) == -1)
			return false;

		/* if dirent is a directory */
		if(S_ISDIR(st1.st_mode) || S_ISDIR(st2.st_mode))
			return false;

		if(	st1.st_mtime == st2.st_mtime &&			
			st1.st_size == st2.st_size)
			return true;
		else
			return false;
	}

	bool PathUtil::CanWriteToFile(const String& file, bool bCreateNoExist)
	{
		if(!file.length())
			return false;

		if(bCreateNoExist)
		{
			String dirPath = GetFileDirPath(file);

			if(!EnsureDir(dirPath))
				return false;
		}

		FILE* pFile = fopen(file.c_str(), "wbS");
		if(!pFile)
			return false;

		fclose(pFile);
		return true;
	}

#if defined(ECHO_PLATFORM_WINDOWS) || defined(ECHO_PLATFORM_MAC) || defined(ECHO_PLATFORM_HTML5)
	String PathUtil::GetCurrentDir()
	{
		char buff[MAX_PATHNAME];
		return getcwd(buff, MAX_PATHNAME);
	}
#endif

	String PathUtil::GetFileDirPath(const String& filename)
	{
		String path = filename;
		size_t pos = path.find_last_of(SEPERATOR);
		if(pos == String::npos)
		{
			return INVALID_PATH;
		}
		path = path.substr(0, pos + 1);

		return path;
	}

	String PathUtil::GetPureFilename(const String& filename, bool bNeedExt)
	{
		String pureFilename = filename;
		pureFilename = StringUtil::Replace(pureFilename, BACKSLASH, SEPERATOR);

		size_t pos = pureFilename.find_last_of(SEPERATOR);
		if(pos != String::npos)
		{
			pureFilename = pureFilename.substr(pos + 1);
		}

		if(!bNeedExt)
		{
			size_t dotPos = pureFilename.find_last_of(DOT);
			pureFilename = pureFilename.substr(0, dotPos);
		}

		return pureFilename;
	}

	String PathUtil::GetPureDirname(const String& dirname)
	{
		if(!IsDir(dirname))
			return INVALID_PATH;

		String pureDirname = dirname;
		pureDirname = pureDirname.substr(0, pureDirname.length() - 1);
		pureDirname = StringUtil::Replace(pureDirname, BACKSLASH, SLASH);

		size_t pos = pureDirname.find_last_of(SEPERATOR);
		if(pos != String::npos)
		{
			pureDirname = pureDirname.substr(pos + 1);
		}

		pureDirname += SEPERATOR;

		return pureDirname;
	}

	String PathUtil::GetFileExt(const String& filename, bool bNeedDot)
	{
		size_t pos = filename.find_last_of(DOT);
		if(pos == String::npos)
		{
			return INVALID_PATH;
		}

		if(!bNeedDot)
			++pos;

		return filename.substr(pos);
	}
	 
	String PathUtil::GetRenameExtFile(const String& filename, const String& extWithDot)
	{
		String newFilename = filename;
		newFilename = StringUtil::Replace(newFilename, BACKSLASH, SLASH);

		size_t dotPos = newFilename.find_last_of(DOT);
		newFilename = newFilename.substr(0, dotPos) + extWithDot;

		FormatPath(newFilename);

		return newFilename;
	}

	String PathUtil::GetRelativePath(const String& path, const String& rootPath)
	{
		String lowerFileOrPath = path;
		String lowerRootPath = rootPath;

		FormatPath(lowerFileOrPath, true);
		FormatPath(lowerRootPath, true);

		if(!IsEndWithSeperator(lowerRootPath))
		{
			lowerRootPath += SEPERATOR;
		}

		size_t ret = lowerFileOrPath.find(lowerRootPath);
		if(ret == 0)
		{
			String result = path.substr(lowerRootPath.length());
			return result;
		}
		else
		{
			return INVALID_PATH;
		}

		return INVALID_PATH;
	}

	String PathUtil::GetParentPath(const String& fileOrPath)
	{
		String tempFile = fileOrPath;
		FormatPath(tempFile);
		if(IsDir(tempFile))
		{
			tempFile = tempFile.substr(0, tempFile.length() - 1);
		}
		tempFile = GetFileDirPath(tempFile);

		return tempFile;
	}

	String PathUtil::GetLastDirName(const String& path)
	{
		String tempPath = path;
		if(!IsDir(tempPath))
			return INVALID_PATH;

		tempPath = tempPath.substr(0, tempPath.length() - 1);

		return GetPureFilename(tempPath, true);
	}

	String PathUtil::GetLastPathName(const String& filename)
	{
		String tempFile = filename;
		bool bIsFolder = IsDir(tempFile);
		if(bIsFolder)
		{
			tempFile = tempFile.substr(0, tempFile.length() - 1);
		}

		String ret = GetPureFilename(tempFile, true);

		if(bIsFolder)
			return ret + SEPERATOR;
		else
			return ret;
	}

	String PathUtil::GetDrive(const String& path)
	{
		String tempPath = path;
		FormatPath(tempPath);
		size_t pos = tempPath.find(":/");
		if(!pos)
			return INVALID_PATH;

		return tempPath.substr(0, pos + 2);
	}

	String PathUtil::GetDriveOrRoot(const String& path)
	{
		String tempPath = path;
		FormatPath(tempPath);
		size_t pos = tempPath.find(":/");
		if(pos!=String::npos)
		{
			tempPath = tempPath.substr(0, pos + 2);
		}
		else
		{
			if(tempPath[0] == SEPERATOR)
				tempPath = SEPERATOR;
			else
				tempPath = INVALID_PATH;
		}

		return tempPath;
	}

	i64 PathUtil::GetFileSize(const String& file)
	{
		struct stat st;

		/* get dirent status */
		if(stat(file.c_str(), &st) == -1)
			return false;

		/* if dirent is a directory */
		if(S_ISDIR(st.st_mode))
			return false;

		return st.st_size;
	}

	bool PathUtil::CreateDir(const String& dir)
	{
		vector<String>::type paths;
		char seps[] = "/\\";
		char pathBuf[MAX_PATHNAME];
		char* token = NULL;

		memset(pathBuf, 0, MAX_PATHNAME);
		memcpy(pathBuf, dir.c_str(), dir.length());

		token = strtok(pathBuf, seps);
		String tempDir;
        while(token != NULL)
		{
			tempDir = token;
            
#ifdef ECHO_PLATFORM_WINDOWS
            if(!IsDriveOrRoot(tempDir))
			{
				paths.push_back(tempDir);
			}
#else
            paths.push_back(tempDir);
#endif
            
			token = strtok(NULL, seps);
		}
        
        tempDir = GetDriveOrRoot(dir);
		for(size_t i=0; i < paths.size(); ++i)
		{
			tempDir += paths[i];
			tempDir += SEPERATOR;

			if(!IsDirExist(tempDir))
			{
#ifdef ECHO_PLATFORM_WINDOWS
				if(mkdir(tempDir.c_str()) != 0)
#else
				if(mkdir(tempDir.c_str(), S_IRWXU|S_IRWXG|S_IRWXO) != 0)
#endif
					return false;
			}
		}

		return true;
	}

	bool PathUtil::EnsureDir(const String& dir)
	{
		if(!IsDir(dir))
			return false;

		if(!IsDirExist(dir))
		{
			if(!CreateDir(dir))
				return false;
		}

		return true;
	}

	bool PathUtil::RenameFile(const String& src, const String& dest)
	{
		if(!rename(src.c_str(), dest.c_str()))
			return true;
		else
			return false;
	}

	bool PathUtil::RenameDir(const String& src, const String& dest)
	{
		if(!rename(src.c_str(), dest.c_str()))
			return true;
		else
			return false;
	}

	void _EnumFilesInDir(StringArray& ret,
		const String& rootPath,
		bool bIncDir,
		bool bIncSubDirs,
		bool bAbsPath,
		String& curRelPath)
	{
		DIR* pDir;
		struct dirent* pDirInfo;
		struct stat st;

		String tempRootDir = rootPath;
		PathUtil::FormatPath(tempRootDir);
		if(PathUtil::IsEndWithSeperator(tempRootDir))
			tempRootDir = tempRootDir.substr(0, tempRootDir.length() -1);

		const char* szDir = tempRootDir.c_str();

		// open dirent directory
		if((pDir = opendir(szDir)) == NULL)
			return ;

		// read all files in this dir
		String subRelPath = curRelPath;
		String fileEntry;
		while((pDirInfo = readdir(pDir)) != NULL)
		{
			String fullname;

			/* ignore hidden files */
			if(pDirInfo->d_name[0] == '.')
				continue;

			fullname  = szDir;
			fullname += "/";
			fullname += pDirInfo->d_name;

			/* get dirent status */
			if(stat(fullname.c_str(), &st) == -1)
				continue ;

			/* if dirent is a directory, call itself */
			if(S_ISDIR(st.st_mode))
			{
				if(bIncDir)
				{
					String dirEntry = fullname;
					dirEntry += PathUtil::SEPERATOR;
					ret.push_back(dirEntry);
				}

				if(bIncSubDirs)
				{
					subRelPath = pDirInfo->d_name;
					subRelPath += PathUtil::SEPERATOR;

					_EnumFilesInDir(ret, fullname, bIncDir, bIncSubDirs, bAbsPath, curRelPath);
				}
			}
			else
			{
				if(bAbsPath)
					fileEntry = fullname;
				else
					fileEntry = curRelPath + pDirInfo->d_name;

				ret.push_back(fileEntry);
			}
		}
        
        closedir(pDir);
	}


	void PathUtil::EnumFilesInDir(StringArray& ret,
		const String& rootPath,
		bool bIncDir,
		bool bIncSubDirs,
		bool bAbsPath)
	{
		String curRelPath;
		_EnumFilesInDir(ret, rootPath, bIncDir, bIncSubDirs, bAbsPath, curRelPath);
	}

	void PathUtil::EnumSubDirs(StringArray& ret,
		const String& rootPath,
		bool bIncSubDirs)
	{
		DIR* pDir;
		struct dirent* pDirInfo;
		struct stat st;

		String tempRootDir = rootPath;
		FormatPath(tempRootDir);
		if(IsEndWithSeperator(tempRootDir))
			tempRootDir = tempRootDir.substr(0, tempRootDir.length() -1);

		const char* szDir = tempRootDir.c_str();

		// open dirent directory
		if((pDir = opendir(szDir)) == NULL)
			return ;

		// read all files in this dir
		while((pDirInfo = readdir(pDir)) != NULL)
		{
			char fullname[255];
			memset(fullname, 0, sizeof(fullname));

			/* ignore hidden files */
			if(pDirInfo->d_name[0] == '.')
				continue;

			strncpy(fullname, szDir, 255);
			strncat(fullname, "/", 255);
			strncat(fullname, pDirInfo->d_name, 255);

			/* get dirent status */
			if(stat(fullname, &st) == -1)
				return ;

			/* if dirent is a directory, call itself */
			if(S_ISDIR(st.st_mode))
			{
				if(bIncSubDirs)
				{
					String subFolder = fullname;
					ret.push_back(subFolder);

					EnumSubDirs(ret, subFolder, bIncSubDirs);
				}
			}
		} // while
	}

	bool PathUtil::CopyFilePath(const String& src, const String& dest, bool bOverWrite)
	{
		String destDir = GetFileDirPath(dest);
		if(!EnsureDir(destDir))
			return false;

		std::ifstream fin(src.c_str(), std::ios::in|std::ios::binary);
		std::ofstream fout(dest.c_str(), std::ios::out | std::ios::binary);

		char buf[4096];
		while( !fin.eof())
		{
			fin.read(buf, 4096);
			fout.write(buf, fin.gcount());
		}

		fin.close();
		fout.close();

		if(GetFileSize(src) == GetFileSize(dest))
			return true;
		else
			return false;
	}

	bool PathUtil::CopyDir(const String& srcDir, const String& destDir, bool bOverWrite)
	{
		DIR* pDir;
		struct dirent* pDirInfo;
		struct stat st;

		String tempSrcDir = srcDir;
		String tempDestDir = destDir;
		FormatPath(tempSrcDir);
		FormatPath(tempDestDir);

		const char* dirname = tempSrcDir.c_str();

		// open dirent directory
		if((pDir = opendir(dirname)) == NULL)
			return false;

		// read all files in this dir
		while((pDirInfo = readdir(pDir)) != NULL)
		{
			char fullname[MAX_PATHNAME];
			memset(fullname, 0, sizeof(fullname));

			/* ignore hidden files */
			if(pDirInfo->d_name[0] == '.')
				continue;

			strncpy(fullname, dirname, 255);
			strncat(fullname, pDirInfo->d_name, 255);

			/* get dirent status */
			if(stat(fullname, &st) == -1)
				return false;

			/* if dirent is a directory, call itself */
			if(S_ISDIR(st.st_mode))
			{
				String subDir = fullname;
				subDir += SEPERATOR;
				String subDestDir = tempDestDir + GetPureDirname(subDir);

				if(!IsEndWithSeperator(subDestDir))
					subDestDir += SEPERATOR;

				PathUtil::CopyDir(subDir, subDestDir, bOverWrite);
			}
			else
			{
				String destFile = tempDestDir + pDirInfo->d_name;
				PathUtil::CopyFilePath( fullname, destFile, bOverWrite);
			}
		} // while

		return true;
	}

	bool PathUtil::DelFilesInDir(const String& dir)
	{
		DIR* pDir;
		struct dirent* pDirInfo;
		struct stat st;

		String tempDir = dir;
		FormatPath(tempDir);
		if(IsEndWithSeperator(tempDir))
			tempDir = tempDir.substr(0, tempDir.length() -1);

		const char* dirname = tempDir.c_str();

		// open dirent directory
		if((pDir = opendir(dirname)) == NULL)
			return false;

		// read all files in this dir
		while((pDirInfo = readdir(pDir)) != NULL)
		{
			char fullname[255];
			memset(fullname, 0, sizeof(fullname));

			/* ignore hidden files */
			if(pDirInfo->d_name[0] == '.')
				continue;

			strncpy(fullname, dirname, 255);
			strncat(fullname, "/", 255);
			strncat(fullname, pDirInfo->d_name, 255);

			/* get dirent status */
			if(stat(fullname, &st) == -1)
            {
                closedir(pDir);
                return false;
            }

			/* if dirent is a directory, call itself */
			if(S_ISDIR(st.st_mode))
			{
				if(DelFilesInDir(fullname))
				{
					if(rmdir(fullname) != 0)
                    {
                        closedir(pDir);
                        return false;
                    }
				}
				else
                {
                    closedir(pDir);
                    return false;
                }
			}
			else
			{
				// delete file
				if(remove(fullname) != 0)
                {
                    closedir(pDir);
                    return false;
                }
			}
		}
        closedir(pDir);

		return true;
	}

	bool PathUtil::DelPath(const String& path)
	{
		struct stat st;

		String tempPath = path;
		FormatPath(tempPath);
		if(IsEndWithSeperator(tempPath))
			tempPath = tempPath.substr(0, tempPath.length() -1);

		const char* szPath = tempPath.c_str();

		/* get dirent status */
		if(stat(szPath, &st) == -1)
			return false;

		bool flag = false;

		/* if dirent is a directory, call itself */
		if(S_ISDIR(st.st_mode))
		{
			if(DelFilesInDir(tempPath))
			{
				if(rmdir(szPath) == 0)
					flag = true;
			}
		}
		else
		{
			// delete file
			if(remove(szPath) == 0)
				flag = true;
		}

		return flag;
	}

	bool PathUtil::WriteData(const String& path, const void* data, int len, const char* mode)
	{
		FILE* fileHandle = fopen(path.c_str(), mode);
		if (fileHandle)
		{
			fwrite(data, len, 1, fileHandle);

			fflush(fileHandle);
			fclose(fileHandle);

			return true;
		}

		return false;
	}
}
