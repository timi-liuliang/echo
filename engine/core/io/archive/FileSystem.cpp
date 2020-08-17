#include "FileSystem.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/Exception.h"
#include "engine/core/main/Engine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/stream/FileHandleDataStream.h"
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ECHO_PLATFORM_WINDOWS
#  include <Windows.h>
#  include <direct.h>
#  include <io.h>
#endif

namespace Echo
{
	FileSystem::FileSystem()
	{
	}

	FileSystem::~FileSystem()
	{
	}

	bool FileSystem::isCaseSensitive(void) const
	{
		return true;
	}

	void FileSystem::setPath(const String& path, const String& prefix)
	{
		m_path = path;
		m_prefix = prefix;
	}

	DataStream* FileSystem::open(const String& filename, ui32 accessMode)
	{
		String fullPath = getFullPath( filename);

        // create dir
        if (accessMode != DataStream::READ)
        {
            String dirPath = PathUtil::GetFileDirPath(fullPath);
            if (!PathUtil::IsDirExist(dirPath))
                PathUtil::CreateDir(dirPath);
        }

        // open file
		FileHandleDataStream* stream = EchoNew(FileHandleDataStream(fullPath, accessMode));
        if (stream->fail())
        {
            EchoSafeDelete(stream, FileHandleDataStream);

            EchoLogError("Error: Cannot open file: %s in FileSystem::open[%s]", filename.c_str(), strerror(errno));
            return nullptr;
        }

		return stream;
	}

	bool FileSystem::isExist(const String& filename)
	{
		String fullPath = getFullPath( filename);

		struct stat tagStat;
		bool ret = (stat(fullPath.c_str(), &tagStat) == 0);

		return ret;
	}

	String FileSystem::getFullPath(const String& resPath)
	{
		String fullPath = StringUtil::Replace(resPath, m_prefix, m_path);
		return fullPath;
	}
}
