#pragma once

#include <functional>
#include "engine/core/base/object.h"
#include "engine/core/thread/Threading.h"
#include "stream/DataStream.h"
#include "memory_reader.h"
#include "archive/FileSystem.h"
#include "archive/FilePackage.h"

namespace Echo
{
	class IO : public Object
	{
        ECHO_SINGLETON_CLASS(IO, Object);

	public:
		virtual ~IO();

		// get instance
		static IO* instance();

		// Engine path
		void addEnginePath(const String& prefix, const String& enginePath);
		const vector<FileSystem*>::type& getEnginePathes();

		// Res path
		void setResPath(const String& resPath);
		const String& getResPath();

		// User path
		void setUserPath(const String& userPath);
		const String& getUserPath();

		// open
		DataStream* open(const String& resourceName, ui32 accessMode = DataStream::READ);

		// is resource exist
		bool isExist(const String& filename);

		// convert between fullpath|respath
		String convertResPathToFullPath(const String& filename);
		bool convertFullPathToResPath(const String& fullPath, String& resPath);

    public:
        // load|save string from|to file
        String loadFileToString(const String& filename);
        bool saveStringToFile(const String& filename, const String& content);

	private:
		IO();
        
        // init packages
        void loadPackages();

	protected:
		EE_MUTEX					(m_mutex);
		vector<FileSystem*>::type	m_engineFileSystems;						// ("Engine://")
		FileSystem*					m_resFileSystem = nullptr;					// ("Res://")
        vector<FilePackage*>::type  m_resFilePackages;
		FileSystem*					m_userFileSystem = nullptr;					// ("User://")
		FileSystem*					m_externalFileSystem = nullptr;
	};
}
