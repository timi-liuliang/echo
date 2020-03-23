#include "engine/core/util/AssertX.h"
#include "engine/core/util/Exception.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/TimeProfiler.h"
#include "IO.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	IO::IO()
	{
		m_resFileSystem = EchoNew(FileSystem);
		m_userFileSystem = EchoNew(FileSystem);
		m_externalFileSystem = EchoNew(FileSystem);
	}

	IO::~IO()
	{
		EchoSafeDelete(m_resFileSystem, FileSystem);
		EchoSafeDelete(m_userFileSystem, FileSystem);
		EchoSafeDelete(m_externalFileSystem, FileSystem);
	}

	IO* IO::instance()
	{
		static IO* inst = EchoNew(IO);
		return inst;
	}

    void IO::bindMethods()
    {
        CLASS_BIND_METHOD(IO, isExist,          "isExist");
        CLASS_BIND_METHOD(IO, loadFileToString, "loadFileToString");
        CLASS_BIND_METHOD(IO, saveStringToFile, "saveStringToFile");
    }

	void IO::setResPath(const String& resPath)
	{
		m_resFileSystem->setPath(resPath, "Res://");
	}

	void IO::setUserPath(const String& userPath)
	{
		m_userFileSystem->setPath(userPath, "User://");
	}

	DataStream* IO::open(const String& resourceName, ui32 accessMode)
	{
		if (StringUtil::StartWith(resourceName, "Res://"))          return m_resFileSystem->open(resourceName, accessMode);
		else if (StringUtil::StartWith(resourceName, "User://"))    return m_userFileSystem->open(resourceName, accessMode);
		else														return m_externalFileSystem->open(resourceName, accessMode);

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());
		return  nullptr;
	}

	bool IO::isExist(const String& resourceName)
	{
		EE_LOCK_MUTEX(m_mutex)

		if (StringUtil::StartWith(resourceName, "Res://"))
		{
			return m_resFileSystem->isExist(resourceName);
		}
		else if (StringUtil::StartWith(resourceName, "User://"))
		{
            return m_userFileSystem->isExist(resourceName);
		}

		return false;
	}

	String IO::convertResPathToFullPath(const String& filename)
	{
		EE_LOCK_MUTEX(m_mutex)

		if (StringUtil::StartWith(filename, "Res://"))
		{
			return m_resFileSystem->getFullPath(filename);
		}
		else if (StringUtil::StartWith(filename, "User://"))
		{
            return m_userFileSystem->getFullPath(filename);
		}

		EchoLogError("getFileLocation [%s] failed", filename.c_str());

		return ""; 
	}

	bool IO::convertFullPathToResPath(const String& fullPath, String& resPath)
	{
		String result = StringUtil::Replace(fullPath, m_resFileSystem->getPath(), m_resFileSystem->getPrefix());
		if (StringUtil::StartWith(result, "Res://"))
		{
			resPath = result;
			return true;
		}

        result = StringUtil::Replace(fullPath, m_userFileSystem->getPath(), m_userFileSystem->getPrefix());
        if (StringUtil::StartWith(result, "User://"))
        {
            resPath = result;
            return true;
        }
		
		return false;
	}

    String IO::loadFileToString(const String& filename)
    {
        MemoryReader reader(filename);
        return reader.getSize() ? reader.getData<const char*>() : StringUtil::BLANK;
    }

    bool IO::saveStringToFile(const String& filename, const String& content)
    {
        DataStream* stream = open(filename, DataStream::WRITE);
        if (stream && stream->isWriteable())
        {
            stream->write(content.data(), content.size());
            stream->close();
            EchoSafeDelete(stream, DataStream);

            return true;
        }

        return false;
    }
}

