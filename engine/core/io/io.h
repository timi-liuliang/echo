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

		// Res path
		void setResPath(const String& resPath);
		const String& getResPath();

		// Engine res path
		void setEngineResPath(const String& enginePath);
		const String& getEngineResPath();

		// Module res path
		void addModuleResPath(const String& moduleName, const String& moduleResPath);

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
		FileSystem					m_engineFileSystem;				// ("Engine://")
		vector<FileSystem>::type	m_moduleFileSystems;			// ("Module://")
		FileSystem					m_resFileSystem;				// ("Res://")
        vector<FilePackage*>::type  m_resFilePackages;
		FileSystem					m_userFileSystem;				// ("User://")
		FileSystem					m_externalFileSystem;
	};
}
