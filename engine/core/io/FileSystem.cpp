#include "FileSystem.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "Engine/core/main/Root.h"
#include "engine/core/Util/PathUtil.h"
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

	// 设置路径
	void FileSystem::setPath(const String& path, const String& prefix)
	{
		m_path = path;
		m_prefix = prefix;
	}

	// 打开资源
	DataStream* FileSystem::open(const String& filename)
	{
		String fullPath = getFullPath( filename);

		// Use filesystem to determine size 
		// (quicker than streaming to the end and back)
		struct stat tagStat;
		int ret = stat(fullPath.c_str(), &tagStat);
		if (ret == 0)
		{
			std::ifstream* origStream = EchoNew(std::ifstream);
			origStream->open(fullPath.c_str(), std::ios::in | std::ios::binary);
			if (!origStream->fail())
			{
				FileStreamDataStream* stream = EchoNew(FileStreamDataStream(filename, origStream, tagStat.st_size, true));
				return stream;
			}
			else
			{
				EchoSafeDelete(origStream, basic_ifstream);
			}
		}

		EchoLogError("Error: Cannot open file: %s in FileSystem::open[%s]", filename.c_str(), strerror(errno));

		return nullptr;
	}

	bool FileSystem::isExist(const String& filename)
	{
		String fullPath = getFullPath( filename);

		struct stat tagStat;
		bool ret = (stat(fullPath.c_str(), &tagStat) == 0);

		return ret;
	}

	// 全路径
	String FileSystem::getFullPath(const String& resPath)
	{
		String fullPath = StringUtil::Replace(resPath, m_prefix, m_path);
		return fullPath;
	}
}
