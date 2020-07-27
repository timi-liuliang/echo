#pragma once

#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class PathUtil
	{
	public:
		static const unsigned int	MAX_PATHNAME;		//!< 1024
		static const unsigned int	MAX_DIRNAME;		//!< 256
		static const unsigned int	MAX_FILENAME;		//!< 256
		static const unsigned int	MAX_EXTNAME;		//!< 256
		static const char			DOT;				//!< '.'
		static const char			SEPERATOR;			//!< '/'
		static const char			SLASH;				//!< '/'
		static const char			BACKSLASH;			//!< '\\'
		static const String			INVALID_PATH;		//!< _T("")

	public:
		PathUtil();
		~PathUtil();

		static void FormatPath(String& path, bool bLower = false);
		static void FormatPathAbsolut(String& path, bool bLower = false);
		static bool	IsFileExist(const String& file);
		static bool	IsDirExist(const String& dir);
		static bool IsHttpFile(const String& file);
		static bool IsLanFile(const String& file);
		static String GetHttpSafeFilePath(const String& httpfile);
		static String GetHttpFileInPath(const String& httpfile);
		static bool	IsEndWithSlash(const String& path);
		static bool	IsEndWithBackSlash(const String& path);
		static bool	IsEndWithSeperator(const String& path);
		static bool	IsRelativePath(const String& path);
		static bool IsAbsolutePath(const String& path);
		static bool IsDir(const String& path);
		static bool IsFile(const String& path);
		static bool IsPureFilename(const String& path);
		static bool IsDriveOrRoot(const String& path);
		static bool IsFileType(const String& file, const String& extWithDot);
		static bool IsSameFile(const String& file1, const String& file2);
		static bool CanWriteToFile(const String& file, bool bCreateNoExist);
		static String GetCurrentDir();
		static String GetFileDirPath(const String& filename, bool withLastSeperator=true);
		static String GetPureFilename(const String& filename, bool bNeedExt = true);
		static String GetPureDirname(const String& dirname);
		static String GetFileExt(const String& filename, bool bNeedDot = false);
		static String GetRenameExtFile(const String& filename, const String& extWithDot);
		static String GetRelativePath(const String& path, const String& rootPath);
		static String GetParentPath(const String& path);
		static String GetLastDirName(const String& path);
		static String GetLastPathName(const String& path);
		static String GetDrive(const String& path);
		static String GetDriveOrRoot(const String& path);
		static i64 GetFileSize(const String& file);
		static bool CreateDir(const String& dir);
		static bool EnsureDir(const String& dir);
		static bool RenameFile(const String& src, const String& dest);
		static bool RenameDir(const String& src, const String& dest);
		static void EnumFilesInDir(StringArray& ret, const String& rootPath, bool bIncDir = false, bool bIncSubDirs = false, bool bAbsPath = false);
		static void EnumSubDirs(StringArray& ret, const String& rootPath, bool bIncSubFolders = false);
		static bool CopyFilePath(const String& src, const String& dest, bool bOverWrite = true);
		static bool CopyDir(const String& srcDir, const String& destDir, bool bOverWrite = true, bool bIgnoreDotFolder=true);
		static bool DelFilesInDir(const String& folder);
		static bool DelPath(const String& path);
		static bool WriteData(const String& path, const void* data, int len, const char* mode="wb");
	};
}
