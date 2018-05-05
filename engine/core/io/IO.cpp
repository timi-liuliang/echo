#include "engine/core/Util/AssertX.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/PathUtil.h"
#include "engine/core/Util/TimeProfiler.h"
#include "IO.h"
#include "engine/core/Util/LogManager.h"

namespace Echo
{
	// 构造函数
	IO::IO()
	{
		m_resFileSystem = EchoNew(FileSystem);
		m_userFileSystem = EchoNew(FileSystem);
	}

	// 析构函数
	IO::~IO()
	{
		reset();
	}

	// get instance
	IO* IO::instance()
	{
		IO* inst = EchoNew(IO);
		return inst;
	}

	// 设置引擎资源路径
	void IO::setResPath(const String& resPath)
	{
		m_resFileSystem->setPath(resPath, "Res://");
	}

	// 设置用户资源路径
	void IO::setUserPath(const String& userPath)
	{
		m_userFileSystem->setPath(userPath, "User://");
	}

	// 打开资源
	DataStream* IO::open(const String& resourceName)
	{
		if (StringUtil::StartWith(resourceName, "Res://"))
		{
			DataStream* stream = m_resFileSystem->open(resourceName);
			if (!stream)
			{

			}

			return stream;
		}
		else if (StringUtil::StartWith(resourceName, "User://"))
		{

		}

		//// 打开资源
		//String  lcResourceName = resourceName;
		//if (!PathUtil::IsAbsolutePath(resourceName))
		//{
		//	StringUtil::LowerCase(lcResourceName);
		//}

		//// 查找所有存档
		//if ( !pArch)
		//	pArch = getArchiveByFileName(lcResourceName.c_str());

		//// 打开资源
		//if (pArch)
		//{
		//	DataStream* stream = pArch->open(lcResourceName);
		//	if (stream == nullptr)
		//	{
		//		EchoLogError("[DataStream* ResourceGroupManager::openResource:%d]::Cannot open a resource [%s] in [%s] ", __LINE__, resourceName.c_str(), archiveName); 
		//		return nullptr; 
		//	}

		//	return stream;
		//}

		EchoLogError("Cannot locate a resource [%s] ResourceGroupManager::openResource", resourceName.c_str());

		return  nullptr;
	}

	// 重置
	void IO::reset()
	{
		EchoSafeDeleteContainer(m_archives, Archive);
		m_resourceIndexCaseSensitive.clear();
	}

	// 添加资源存档
	void IO::addArchive(const String& name, const String& locType, bool isOverWrite)
	{
		TIME_PROFILE
		(
			// Get archive
			Archive* pArch = Archive::load(name, locType);
			{
				EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

				// Add to location list
				m_archives.push_back(pArch);

				// Index resources
				StringArray vec = pArch->list();
				for (StringArray::iterator it = vec.begin(); it != vec.end(); ++it)
					addToIndex(*it, pArch, isOverWrite);

				EchoLogInfo("Added resource [%s] location of type %s", name.c_str(), locType.c_str());
			}
		)
	}

	// 移除资源存档
	void IO::removeArchive(const String& name)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		// Remove from location list
		for (ArchiveList::iterator li = m_archives.begin(); li != m_archives.end(); ++li)
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
				m_archives.erase(li);

				break;
			}
		}

		EchoLogInfo("Removed resource [%s] location", name.c_str());
	}

	// 获取文件所在的存档
	Archive* IO::getArchiveByFileName(const char* fileName)
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
	Archive* IO::getArchiveByName(const char* archiveName)
	{
		for (Archive* archive : m_archives)
		{
			if (archive->getName() == archiveName)
				return archive;
		}

		return NULL;
	}

	void IO::openResourceAsync(const String& resourceName, EchoOpenResourceCb callback)
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
	bool IO::isAsync(const char* fileName)
	{
		Archive* archive = getArchiveByFileName(fileName);

		return archive ? archive->isAsync(fileName) : false;
	}

	// 判断资源是否存在
	bool IO::isResourceExists(const String& resourceName)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)

		if (StringUtil::StartWith(resourceName, "Res://"))
		{
			return m_resFileSystem->isExist(resourceName);
		}
		else if (StringUtil::StartWith(resourceName, "User://"))
		{

		}

		return false;
	}

	// 获取文件位置
	String IO::getFullPath(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX

		String ret;

		if (StringUtil::StartWith(filename, "Res://"))
		{
			return m_resFileSystem->getFullPath(filename);
		}
		else if (StringUtil::StartWith(filename, "User://"))
		{

		}

		EchoLogError("getFileLocation [%s] failed", filename.c_str());

		return ret; 
	}

	// 通过全路径获取资源路径
	bool IO::covertFullPathToResPath(const String& fullPath, String& resPath)
	{
		String result = StringUtil::Replace(fullPath, m_resFileSystem->getPath(), m_resFileSystem->getPrefix());
		if (StringUtil::StartWith(result, "Res://"))
		{
			resPath = result;
			return true;
		}
		
		return false;
	}

	// 获取指定后缀名的所有文件
	void IO::listFilesWithExt( StringArray& oFiles, const char* extWithDot)
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
	void IO::foreach_archive_unsafe(std::function<bool (const Echo::String&, const int index)> callback)
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

	void IO::listFileWithPathAndExt(StringArray& oFiles, const char* path, const char* extWithDot)
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
			String filePath = getFullPath(itf->first);
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
	void IO::addToIndex(const String& filename, Archive* arch, bool isOverWrite)
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
	Archive* IO::FindFileArchive(const String& filename)
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
	void IO::removeFromIndex(const String& filename)
	{
		EE_LOCK_AUTO_MUTEX

		FileArchiveMapping::iterator it = m_resourceIndexCaseSensitive.find(filename);
		if (it != this->m_resourceIndexCaseSensitive.end())
			m_resourceIndexCaseSensitive.erase(it);
	}

	// 添加文件(运行期添加文件)
	void IO::addFile(const String& archiveType, const String& fullPath)
	{
#ifdef ECHO_EDITOR_MODE
		Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath);
		StringUtil::LowerCase(fileName);
		if ( !isResourceExists(fileName) && !fileName.empty())
		{
			for (ArchiveList::iterator it = m_archives.begin(); it != m_archives.end(); it++)
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
	void IO::removeFile(const String& _fileName)
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

