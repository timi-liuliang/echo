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
	}

	IO::~IO()
	{
	}

	IO* IO::instance()
	{
		static IO* inst = EchoNew(IO);
		return inst;
	}

    void IO::bindMethods()
    {
        CLASS_BIND_METHOD(IO, isExist);
        CLASS_BIND_METHOD(IO, loadFileToString);
        CLASS_BIND_METHOD(IO, saveStringToFile);
    }

    void IO::setEngineResPath(const String& enginePath)
    {
        String formatPath = enginePath;
        PathUtil::FormatPathAbsolut(formatPath);

        m_engineFileSystem.setPath(formatPath, "Engine://");
    }

    const String& IO::getEngineResPath()
    {
        return m_engineFileSystem.getPath();
    }

    void IO::addModuleResPath(const String& moduleName, const String& moduleResPath)
    {
        if (PathUtil::IsDirExist(moduleResPath))
        {
            FileSystem moduleFileSystem;
            moduleFileSystem.setPath(moduleResPath, "Module://" + moduleName + "/");

            m_moduleFileSystems.emplace_back(moduleFileSystem);
        }
    }

	void IO::setResPath(const String& resPath)
	{
        String finalPath = resPath;
        PathUtil::FormatPath(finalPath);
        PathUtil::FormatPathAbsolut(finalPath);

		m_resFileSystem.setPath(finalPath, "Res://");
        
        loadPackages();
	}

    const String& IO::getResPath()
    {
        return m_resFileSystem.getPath();
    }

    void IO::loadPackages()
    {
        StringArray packages;
        PathUtil::EnumFilesInDir(packages, m_resFileSystem.getPath(), false, false, true);
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
		m_userFileSystem.setPath(userPath, "User://");
	}

    const String& IO::getUserPath()
    {
        return m_userFileSystem.getPath();
    }

	DataStream* IO::open(const String& resourceName, ui32 accessMode)
	{
		if (StringUtil::StartWith(resourceName, "Res://"))
        {
            DataStream* stream = m_resFileSystem.open(resourceName, accessMode);
            if(stream)  return stream;
            
            for(FilePackage* package : m_resFilePackages)
            {
                stream = package->open(resourceName.c_str());
                if(stream)  return stream;
            }
        }
		else if (StringUtil::StartWith(resourceName, "User://"))
        {
            return m_userFileSystem.open(resourceName, accessMode);
        }
        else if (StringUtil::StartWith(resourceName, "Engine://"))
        {
            return m_engineFileSystem.open(resourceName, accessMode);
        }
        else if (StringUtil::StartWith(resourceName, "Module://"))
        {
            for (FileSystem& moduleFileSystem : m_moduleFileSystems)
            {
                if(StringUtil::StartWith(resourceName, moduleFileSystem.getPrefix()))
                    return moduleFileSystem.open(resourceName);
            }
        }
		else
        {
            return m_externalFileSystem.open(resourceName, accessMode);
        }

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());
		return  nullptr;
	}

	bool IO::isExist(const String& resourceName)
	{
        EE_LOCK_MUTEX(m_mutex)

		if (StringUtil::StartWith(resourceName, "Res://"))
		{
            if(m_resFileSystem.isExist(resourceName))
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
            return m_userFileSystem.isExist(resourceName);
		}
        else if (StringUtil::StartWith(resourceName, "Engine://"))
        {
            return m_engineFileSystem.isExist(resourceName);
        }
        else if (StringUtil::StartWith(resourceName, "Module://"))
        {
            for (FileSystem& moduleFileSystem : m_moduleFileSystems)
            {
                if (moduleFileSystem.isExist(resourceName))
                    return true;
            }
        }

		return PathUtil::IsFileExist(resourceName);
	}

	String IO::convertResPathToFullPath(const String& filename)
	{
		EE_LOCK_MUTEX(m_mutex)

		if (StringUtil::StartWith(filename, "Res://"))
		{
			return m_resFileSystem.getFullPath(filename);
		}
		else if (StringUtil::StartWith(filename, "User://"))
		{
            return m_userFileSystem.getFullPath(filename);
		}
        else if (StringUtil::StartWith(filename, "Engine://"))
        {
            return m_engineFileSystem.getFullPath(filename);
        }
        else if (StringUtil::StartWith(filename, "Module://"))
        {
            for (FileSystem& moduleFileSystem : m_moduleFileSystems)
            {
                if (StringUtil::StartWith(filename, moduleFileSystem.getPrefix()))
                    return moduleFileSystem.getFullPath(filename);
            }
        }

		EchoLogError("getFileLocation [%s] failed", filename.c_str());

		return ""; 
	}

	bool IO::convertFullPathToResPath(const String& fullPath, String& resPath)
	{
		String result = StringUtil::Replace(fullPath, m_resFileSystem.getPath(), m_resFileSystem.getPrefix());
		if (StringUtil::StartWith(result, "Res://"))
		{
			resPath = result;
			return true;
		}

        result = StringUtil::Replace(fullPath, m_userFileSystem.getPath(), m_userFileSystem.getPrefix());
        if (StringUtil::StartWith(result, "User://"))
        {
            resPath = result;
            return true;
        }

        result = StringUtil::Replace(fullPath, m_engineFileSystem.getPath(), m_engineFileSystem.getPrefix());
        if (StringUtil::StartWith(result, "Engine://"))
        {
            resPath = result;
            return true;
        }

        for (FileSystem& moduleFileSystem : m_moduleFileSystems)
        {
            result = StringUtil::Replace(fullPath, moduleFileSystem.getPath(), moduleFileSystem.getPrefix());
            if (StringUtil::StartWith(result, "Module://"))
            {
                resPath = result;
                return true;
            }
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

