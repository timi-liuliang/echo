#pragma once

#include <engine/core/io/stream/FileHandleDataStream.h>
#include "engine/core/util/XmlBinary.h"
#include "engine/core/thread/Threading.h"

namespace Echo
{
	class FilePackage
	{
	public:
		FilePackage(const char* packageFile);
		~FilePackage();

		// open
		DataStream* open(const char* fileName);
        
        // is exist
        bool isExist(const String& filename);
        
		// add data
		static void compressFolder(const char* folderPath);

	private:
		// compress|uncompress
		int uncompress(unsigned char* dest, unsigned int* destLen, const unsigned char* source, unsigned int sourceLen);
		int	compress(unsigned char* dest, unsigned int* destLen, const unsigned char* source, unsigned int sourceLen);

	private:
        String                      m_packageFile;
        map<String,String>::type    m_files;
		XmlBinaryReader             m_reader;
	};
}
