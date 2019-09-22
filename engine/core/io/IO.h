#pragma once

#include <functional>
#include "engine/core/base/object.h"
#include "engine/core/thread/Threading.h"
#include "DataStream.h"
#include "MemoryReader.h"
#include "archive/Archive.h"
#include "archive/FileSystem.h"

namespace Echo
{
	class IO : public Object
	{
        ECHO_SINGLETON_CLASS(IO, Object);

	public:
		virtual ~IO();

		// get instance
		static IO* instance();

		// set res|user path
		void setResPath(const String& resPath);
		void setUserPath(const String& userPath);

		// open
		DataStream* open(const String& resourceName, ui32 accessMode = DataStream::READ);

		// is resource exist
		bool isExist(const String& filename);

		// convert between fullpath|respath
		String getFullPath(const String& filename);
		bool covertFullPathToResPath(const String& fullPath, String& resPath);

    public:
        // load|save string from|to file
        String loadFileToString(const String& filename);
        bool saveStringToFile(const String& filename, const String& content);

	private:
		IO();

	protected:
        EE_AUTO_MUTEX
		FileSystem*					m_resFileSystem;					// ("Res://")
		FileSystem*					m_userFileSystem;					// ("User://")
	};
}
