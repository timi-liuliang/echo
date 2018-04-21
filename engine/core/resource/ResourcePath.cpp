#include "ResourcePath.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
	ResourcePath::ResourcePath(const String& path, const char* exts)
	{
		m_supportExts = exts ? exts : PathUtil::GetFileExt(path, true);
		setPath(path);
	}

	bool ResourcePath::setPath(const String& path)
	{
		String ext = PathUtil::GetFileExt(path, true);
		if (isSupportExt(ext))
		{
			m_path = path;
			return true;
		}

		return false;
	}

	bool ResourcePath::isSupportExt(const String& ext)
	{
		if (m_supportExts.empty())
			return true;

		StringArray exts = StringUtil::Split(m_supportExts, "|");
		for (const String& supportExt : exts)
		{
			if (supportExt == ext)
				return true;
		}

		return false;
	}
}