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
        EchoSafeDeleteContainer(m_engineFileSystems, FileSystem);
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

    void IO::addEnginePath(const String& prefix, const String& enginePath)
    {
        if (StringUtil::StartWith(prefix, "Engine://"))
        {
            for (FileSystem* fileSystem : m_engineFileSystems)
            {
                if (StringUtil::Equal(prefix, fileSystem->getPrefix()))
                    return;
            }
        }

        FileSystem* newFileSystem = EchoNew(FileSystem);
        newFileSystem->setPath(enginePath, prefix);

        m_engineFileSystems.emplace_back(newFileSystem);
    }

    const vector<FileSystem*>::type& IO::getEnginePathes()
    {
        return m_engineFileSystems;
    }

	void IO::setResPath(const String& resPath)
	{
		m_resFileSystem->setPath(resPath, "Res://");
        
        loadPackages();
	}

    const String& IO::getResPath()
    {
        return m_resFileSystem->getPath();
    }

    void IO::loadPackages()
    {
        StringArray packages;
        PathUtil::EnumFilesInDir(packages, m_resFileSystem->getPath(), false, false, true);
        for(const String& package : packages)
        {
            if(PathUtil::GetFileExt(package, true)==".pkg")
            {
                FilePackage* filePackage = EchoNew(FilePackage(package.c_str()));
                m_resFilePackages.push_back(filePackage);
            }
        }
    }

	void IO::setUserPath(const String& userPath)
	{
		m_userFileSystem->setPath(userPath, "User://");
	}

    const String& IO::getUserPath()
    {
        return m_userFileSystem->getPath();
    }

	DataStream* IO::open(const String& resourceName, ui32 accessMode)
	{
        if (StringUtil::StartWith(resourceName, "Engine://"))
        {
            for (FileSystem* fileSystem : m_engineFileSystems)
            {
                if (StringUtil::StartWith(resourceName, fileSystem->getPrefix()))
                {
                    return fileSystem->open(resourceName, accessMode);
                }
            }
        }
		else if (StringUtil::StartWith(resourceName, "Res://"))
        {
            DataStream* stream = m_resFileSystem->open(resourceName, accessMode);
            if(stream)  return stream;
            
            for(FilePackage* package : m_resFilePackages)
            {
                stream = package->open(resourceName.c_str());
                if(stream)  return stream;
            }
        }
		else if (StringUtil::StartWith(resourceName, "User://"))
        {
            return m_userFileSystem->open(resourceName, accessMode);
        }
		else
        {
            return m_externalFileSystem->open(resourceName, accessMode);
        }

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());
		return  nullptr;
	}

	bool IO::isExist(const String& resourceName)
	{
        EE_LOCK_MUTEX(m_mutex)

		if (StringUtil::StartWith(resourceName, "Res://"))
		{
            if(m_resFileSystem->isExist(resourceName))
                return true;
                
			for(FilePackage* package : m_resFilePackages)
            {
                if(package->isExist(resourceName))
                    return true;
            }
            
            return false;
		}
		else if (StringUtil::StartWith(resourceName, "User://"))
		{
            return m_userFileSystem->isExist(resourceName);
		}

		return PathUtil::IsFileExist(resourceName);
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

