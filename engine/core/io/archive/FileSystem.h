#pragma once

#include "../DataStream.h"

namespace Echo
{
	class FileSystem 
	{
	public:
		FileSystem();
		~FileSystem();

		// prefix+path
		void setPath(const String& path, const String& prefix);

		// open
		DataStream* open(const String& filename, ui32 accessMode = DataStream::READ);

		// is case sensitive
		bool isCaseSensitive() const;

		// is exist
		bool isExist(const String& filename);

		// get prefix
		const String& getPrefix() const { return m_prefix; }

		// get path
		const String& getPath() const { return m_path; }

		// get full path
		String getFullPath(const String& resPath);

	private:
		String			m_path;
		String			m_prefix;
	};
}
