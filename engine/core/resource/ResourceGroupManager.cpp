#include "engine/core/Util/AssertX.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/TimeProfiler.h"
#include "ArchiveManager.h"
#include "ResourceGroupManager.h"
#include "engine/core/Util/LogManager.h"

namespace Echo
{
	__ImplementSingleton(ResourceGroupManager);

	// 构造函数
	ResourceGroupManager::ResourceGroupManager()
	{
		__ConstructSingleton;
	}

	// 析构函数
	ResourceGroupManager::~ResourceGroupManager()
	{
		reset();

		__DestructSingleton;
	}

	// 重置
	void ResourceGroupManager::reset()
	{
		for (list<Archive*>::type::iterator it = m_archiveList.begin(); it != m_archiveList.end(); it++)
		{
			ArchiveManager::Instance()->unload((*it)->getName());
		}
		m_archiveList.clear();
		m_resourceIndexCaseSensitive.clear();
	}

	// 添加资源存档
	void ResourceGroupManager::addArchive(const String& name, const String& locType, bool isOverWrite)
	{
		TIME_PROFILE
		(
			// Get archive
			Archive* pArch = ArchiveManager::Instance()->load(name, locType);
			{
				EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

				// Add to location list
				m_archiveList.push_back(pArch);

				// Index resources
				StringArray vec = pArch->list();
				for (StringArray::iterator it = vec.begin(); it != vec.end(); ++it)
					addToIndex(*it, pArch, isOverWrite);

				EchoLogInfo("Added resource [%s] location of type %s", name.c_str(), locType.c_str());
			}
		)
	}

	// 移除资源存档
	void ResourceGroupManager::removeArchive(const String& name)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		// Remove from location list
		for (ArchiveList::iterator li = m_archiveList.begin(); li != m_archiveList.end(); ++li)
		{
			Archive* pArch = *li;
			if (pArch->getName() == name)
			{
				// 移除引用
				for (FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.begin(); rit != m_resourceIndexCaseSensitive.end();)
				{
					if (rit->second == pArch)
					{
						m_resourceIndexCaseSensitive.erase(rit++);
					}
					else
					{
						++rit;
					}
				}
				ArchiveManager::Instance()->unload(name);
				m_archiveList.erase(li);

				break;
			}
		}

		EchoLogInfo("Removed resource [%s] location", name.c_str());
	}

	// 获取文件所在的存档
	Archive* ResourceGroupManager::getArchiveByFileName(const char* fileName)
	{
		String  lcResourceName = fileName;
		StringUtil::LowerCase(lcResourceName);
		Archive*pArch = NULL;	
		{
			EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
			FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(lcResourceName);
			if (rit != m_resourceIndexCaseSensitive.end())
			{
				pArch = rit->second;
			}
		}

		return pArch;
	}

	// 根据存档名称获取存档
	Archive* ResourceGroupManager::getArchiveByName(const char* archiveName)
	{
		for (Archive* archive : m_archiveList)
		{
			if (archive->getName() == archiveName)
				return archive;
		}

		return NULL;
	}
	
	// 打开资源
	DataStream* ResourceGroupManager::openResource( const String& resourceName, const char* archiveName)
	{
		// 打开资源
		String  lcResourceName = resourceName;
		if (!PathUtil::IsAbsolutePath(resourceName))
		{
			StringUtil::LowerCase(lcResourceName);
		}

		// 获取对应的存档文件
		Archive* pArch = NULL;
		if (archiveName && strlen(archiveName))
		{
			EchoAssert(!StringUtil::IsHaveUpper(archiveName));
			pArch = getArchiveByName(archiveName);

		}

		// 查找所有存档
		if ( !pArch)
			pArch = getArchiveByFileName(lcResourceName.c_str());

		// 打开资源
		if (pArch)
		{
			DataStream* stream = pArch->open(lcResourceName);
			if (stream == nullptr)
			{
				EchoLogError("[DataStream* ResourceGroupManager::openResource:%d]::Cannot open a resource [%s] in [%s] ", __LINE__, resourceName.c_str(), archiveName); 
				return nullptr; 
			}

			return stream;
		}

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());
		return NULL;
	}

	void ResourceGroupManager::openResourceAsync(const String& resourceName, EchoOpenResourceCb callback)
	{
		String  lcResourceName = resourceName;
		StringUtil::LowerCase(lcResourceName);
		Archive* pArch = getArchiveByFileName(lcResourceName.c_str());
		if (pArch)
		{
			pArch->addAsyncCompleteCallback(resourceName, callback); 
			pArch->open(lcResourceName);
		}
	}

	// 是否为异步资源
	bool ResourceGroupManager::isAsync(const char* fileName)
	{
		Archive* archive = getArchiveByFileName(fileName);

		return archive ? archive->isAsync(fileName) : false;
	}

	// 判断资源是否存在
	bool ResourceGroupManager::isResourceExists(const String& resourceName)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		// Try indexes first
		String lcResourceName = resourceName;
		StringUtil::LowerCase(lcResourceName);
		FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(lcResourceName);
		if (rit != m_resourceIndexCaseSensitive.end())
		{
			// Found in the index
			return true;
		}

		return false;
	}

	// 获取文件位置
	String ResourceGroupManager::getFileLocation(const String& filename)
	{
		String ret;
        {
			String lowerCaseName = filename;
			StringUtil::LowerCase(lowerCaseName);

			EE_LOCK_AUTO_MUTEX

			Archive *pArch = NULL;
            FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find( lowerCaseName);
			if (rit != m_resourceIndexCaseSensitive.end())
			{
				// Found in the index
				pArch = rit->second;
                ret = pArch->location( lowerCaseName);
				return ret;
			}
        }

		EchoLogError("getFileLocation [%s] failed", filename.c_str());

		return ret; 
	}

	// 获取指定后缀名的所有文件
	void ResourceGroupManager::listFilesWithExt( StringArray& oFiles, const char* extWithDot)
	{
		oFiles.clear();

		// 分解后缀
		StringArray exts = StringUtil::Split( extWithDot, "|");

		EE_LOCK_AUTO_MUTEX
		for( FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf!=m_resourceIndexCaseSensitive.end(); itf++)
		{
			String fileExt = PathUtil::GetFileExt(itf->first, true);
			for (size_t i = 0; i < exts.size(); i++)
			{
				if (exts[i] == fileExt)
					oFiles.push_back(itf->first);
			}
		}
	}

		// 获取指定后缀名的所有文件
	void ResourceGroupManager::foreach_archive_unsafe(std::function<bool (const Echo::String&, const int index)> callback)
	{
//		EE_LOCK_AUTO_MUTEX
		int i = 0;
		for( FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf!=m_resourceIndexCaseSensitive.end(); itf++)
		{
			bool r = callback(itf->first, i++);
			if (!r)
				break;
		}
	}

	void ResourceGroupManager::listFileWithPathAndExt(StringArray& oFiles, const char* path, const char* extWithDot)
	{
		oFiles.clear();

		// 分解后缀
		StringArray exts = StringUtil::Split(extWithDot, "|");
		
		String realPath = path;
		PathUtil::FormatPath(realPath, true);
		EE_LOCK_AUTO_MUTEX
		for (FileArchiveMapping::iterator itf = m_resourceIndexCaseSensitive.begin(); itf != m_resourceIndexCaseSensitive.end(); itf++)
		{
			String fileExt = PathUtil::GetFileExt(itf->first, true);
			String filePath = getFileLocation(itf->first);
			filePath = PathUtil::GetFileDirPath(filePath);
			PathUtil::FormatPathAbsolut(filePath, true);
			PathUtil::FormatPathAbsolut(realPath, true);
			for (size_t i = 0; i < exts.size(); i++)
			{
				if (exts[i] == fileExt && StringUtil::StartWith(filePath, realPath))
					oFiles.push_back(itf->first);
			}
		}
	}


	// 添加索引
	void ResourceGroupManager::addToIndex(const String& filename, Archive* arch, bool isOverWrite)
	{
		EE_LOCK_AUTO_MUTEX
		Echo::String fileName = filename;
		StringUtil::LowerCase(fileName);
		if (m_resourceIndexCaseSensitive.find(fileName) == m_resourceIndexCaseSensitive.end() || isOverWrite)
		{
			// internal, assumes mutex lock has already been obtained
			m_resourceIndexCaseSensitive[fileName] = arch;
		}
		else
		{
			EchoLogError("file [%s] is not unique!", fileName.c_str());
		}
	}

	// 获取存档
	Archive* ResourceGroupManager::FindFileArchive(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX
		FileArchiveMapping::iterator rit = m_resourceIndexCaseSensitive.find(filename);
		if(rit != m_resourceIndexCaseSensitive.end())
		{
			return rit->second;
		}

		return NULL;
	}

	// 移除到索引
	void ResourceGroupManager::removeFromIndex(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX

		FileArchiveMapping::iterator it = m_resourceIndexCaseSensitive.find(filename);
		if (it != this->m_resourceIndexCaseSensitive.end())
			m_resourceIndexCaseSensitive.erase(it);
	}

	// 添加文件(运行期添加文件)
	void ResourceGroupManager::addFile(const String& archiveType, const String& fullPath)
	{
#ifdef ECHO_EDITOR_MODE
		Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
		StringUtil::LowerCase(fileName);
		if ( !isResourceExists(fileName) && !fileName.empty())
		{
			for (ArchiveList::iterator it = m_archiveList.begin(); it != m_archiveList.end(); it++)
			{
				Archive* archive = *it;
				if (archive->getType() == archiveType && archive->isCanAddFile(fullPath))
				{
					archive->addFile(fullPath);
					addToIndex(fileName, archive);
					break;
				}
			}
		}
		else if (fileName.empty())
		{
			EchoLogError("add file [%s] failed...", fileName.c_str());
		}
#endif
	}

	// 移除文件
	void ResourceGroupManager::removeFile(const String& _fileName)
	{
		String fileName = _fileName;
		StringUtil::LowerCase(fileName);

		Archive* archive = FindFileArchive(fileName);
		if (archive)
		{
			archive->removeFile(fileName.c_str());

			// 移除引用
			removeFromIndex(fileName);
		}		
	}
}

