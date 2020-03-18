#include "ResourcePack.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/Exception.h"
#include "ZipArchive.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	ZipArchive::ZipArchive(const String& strName, const String& strArchType)
		:	Archive(strName, strArchType)
		,	mLoaded(false)
	{
	}

	ZipArchive::~ZipArchive()
	{
		unload();
	}

	bool ZipArchive::isCaseSensitive(void) const 
	{ 
		return true; 
	}

	void ZipArchive::load()
	{
		EE_LOCK_MUTEX(m_mutex)
		String fullName = Engine::instance()->getResPath() + mName;
		m_resourcePack.openFile(fullName.c_str());
		mLoaded = true;
	}

	void ZipArchive::unload()
	{
		EE_LOCK_MUTEX(m_mutex)
		if (mLoaded)
		{
			m_resourcePack.closeFile();
			mLoaded = false;
		}
	}

	DataStream* ZipArchive::open(const String& strFilename)
	{
		EE_LOCK_MUTEX(m_mutex)
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

	StringArray ZipArchive::list(bool bRecursive /*= true*/, bool bDirs /*= false*/)
	{
		EE_LOCK_MUTEX(m_mutex)
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

	FileInfoList* ZipArchive::listFileInfo(bool bDirs /*= false*/)
	{
		EE_LOCK_MUTEX(m_mutex)
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

	bool ZipArchive::exists(const String& strFilename)
	{
		EE_LOCK_MUTEX(m_mutex)
		return m_resourcePack.isFileExist(strFilename.c_str());
	}
}
