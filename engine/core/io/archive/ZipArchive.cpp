#include "ResourcePack.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "ZipArchive.h"
#include "Engine/core/main/Root.h"

namespace Echo
{
	// 构造函数
	ZipArchive::ZipArchive(const String& strName, const String& strArchType)
		:	Archive(strName, strArchType)
		,	mLoaded(false)
	{
	}

	// 析构函数
	ZipArchive::~ZipArchive()
	{
		unload();
	}

	// 是否大小写敏感
	bool ZipArchive::isCaseSensitive(void) const 
	{ 
		return true; 
	}

	// 加载 
	void ZipArchive::load()
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		String fullName = Root::instance()->getResPath() + mName;
		m_resourcePack.openFile(fullName.c_str());
		mLoaded = true;
	}

	// 卸载
	void ZipArchive::unload()
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		if (mLoaded)
		{
			m_resourcePack.closeFile();
			mLoaded = false;
		}
	}

	// 打开文件
	DataStream* ZipArchive::open(const String& strFilename)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		DataStream* pResult = NULL;
		const unsigned char* cpbFileData	= NULL;
		unsigned char* pData		= NULL;
		unsigned int nOutLength		= 0;
		String strFile = strFilename;
 		cpbFileData = m_resourcePack.readFile(strFile.c_str(), nOutLength);
		if (cpbFileData)
		{
			pData = (unsigned char*)ECHO_MALLOC(nOutLength);

			memcpy(pData, cpbFileData, nOutLength);

			m_resourcePack.ReleaseExtraMemory();

			pResult = EchoNew(ResourcePackDataStream(strFile, pData, nOutLength, true));
			return pResult;
		}
			
		EchoLogError("Error: Error in function ZipArchive::open, read file %s fail!", strFile.c_str());
		return NULL;
	}

	// 列出所有文件
	StringArray ZipArchive::list(bool bRecursive /*= true*/, bool bDirs /*= false*/)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		const ResourcePack::FileNode* cpFileNodes	= NULL;
		int	nOutFileListCount			= 0;
		StringArray ret;
		FileInfo fileInfo; 


		cpFileNodes = m_resourcePack.GetFileList(nOutFileListCount);
		if(!cpFileNodes)
			EchoLogError("Error: Error in function ZipArchive::list, get file list fail!");

		for(int i = 0; i < nOutFileListCount; i++)
		{
			if(cpFileNodes[i].m_nUseFlag == ResourcePack::FILE_NOT_USING)
			{
				break;
			}
			else if (cpFileNodes[i].m_nUseFlag == ResourcePack::FILE_DELETE)
			{
				continue;
			}

			ret.push_back(cpFileNodes[i].m_sFileName);
		}
		return ret;
	}

	// 列出文件信息
	FileInfoList* ZipArchive::listFileInfo(bool bDirs /*= false*/)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		const ResourcePack::FileNode* cpFileNodes = NULL;
		int	nOutFileListCount			= 0;
		FileInfoList* pResult			= NULL;
		FileInfo fileInfo;

		cpFileNodes = m_resourcePack.GetFileList(nOutFileListCount);
		if(!cpFileNodes)
			EchoLogError("Error: Error in function ZipArchive::list, get file list fail!");

		pResult = EchoNew(FileInfoList);

		for(int i = 0; i < nOutFileListCount; i++)
		{
			if (cpFileNodes[i].m_nUseFlag == ResourcePack::FILE_NOT_USING)
			{
				break;
			}
			else if (cpFileNodes[i].m_nUseFlag == ResourcePack::FILE_DELETE)
			{
				continue;
			}
			fileInfo.archive  = this;
			fileInfo.filename = cpFileNodes[i].m_sFileName;	
			fileInfo.basename = cpFileNodes[i].m_sFileName;	
			fileInfo.path	  = "";
			fileInfo.compressedSize = cpFileNodes[i].m_nFileCompressSize;
			fileInfo.uncompressedSize = cpFileNodes[i].m_nFileRealSize;

			pResult->push_back(fileInfo);
		}
		return pResult;
	}

	// 判断文件是否存在
	bool ZipArchive::exists(const String& strFilename)
	{
		EE_LOCK_MUTEX(AUTO_MUTEX_NAME)
		return m_resourcePack.isFileExist(strFilename.c_str());
	}
}
