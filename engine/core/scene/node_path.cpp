#include "node_path.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	NodePath::NodePath()
	{

	}

	NodePath::NodePath(const String& path, const char* exts)
	{
		m_supportTypes = exts ? exts : PathUtil::GetFileExt(path, true);
		if (!path.empty())
			setPath(path);
	}

	bool NodePath::setPath(const String& path)
	{
		String ext = PathUtil::GetFileExt(path, true);
		if (path.empty() || isSupportType(ext))
		{
			m_path = path;
			return true;
		}

		EchoLogError("set path [%s] failed, not support this ext [%s].", path.c_str(), ext.c_str());

		return false;
	}

	bool NodePath::isSupportType(const String& ext)
	{
		if (m_supportTypes.empty())
			return true;

		StringArray exts = StringUtil::Split(m_supportTypes, "|");
		for (const String& supportExt : exts)
		{
			if (StringUtil::Equal(supportExt, ext, false))
				return true;
		}

		return false;
	}
}