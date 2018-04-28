#include "ResourcePath.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	ResourcePath::ResourcePath(const String& path, const char* exts)
	{
		m_supportExts = exts ? exts : PathUtil::GetFileExt(path, true);
		if(!path.empty())
			setPath(path);
	}

	bool ResourcePath::setPath(const String& path)
	{
		String ext = PathUtil::GetFileExt(path, true);
		if (path.empty() || isSupportExt(ext))
		{
			m_path = path;
			return true;
		}

		EchoLogError("set path [%s] failed, not support this ext [%s].",  path.c_str(), ext.c_str());

		return false;
	}

	bool ResourcePath::isSupportExt(const String& ext)
	{
		if (m_supportExts.empty())
			return true;

		StringArray exts = StringUtil::Split(m_supportExts, "|");
		for (const String& supportExt : exts)
		{
			if (StringUtil::Equal(supportExt, ext, false))
				return true;
		}

		return false;
	}
}