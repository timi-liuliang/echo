#include "Engine/Core.h"
#include "Engine/core/Resource/ResourcePack.h"
#include "engine/core/Util/PathUtil.h"
#include "zlib/zlib.h"
#include "engine/core/Util/AssertX.h"

namespace Echo
{
	static void* Zlib_User_Alloc(void* opaque,unsigned int items,unsigned int size)
	{
		return EchoMalloc(size*items);
	}

	static void Zlib_User_Free(void* opaque,void* ptr)
	{
		EchoSafeFree(ptr);
	}

	int ResourcePack::Zlib_UnCompress(unsigned char* dest,unsigned int* destLen,const unsigned char* source,unsigned int sourceLen)
	{
		z_stream stream;
		int err;

		stream.next_in = (Bytef*)source;
		stream.avail_in = (unsigned int)sourceLen;
		/* Check for source > 64K on 16-bit machine: */
		if (stream.avail_in != sourceLen) return Z_BUF_ERROR;

		stream.next_out = dest;
		stream.avail_out = *destLen;
		if (stream.avail_out != *destLen) return Z_BUF_ERROR;

		stream.zalloc = &Zlib_User_Alloc;  //0;
		stream.zfree  = &Zlib_User_Free;   //0;

		err = inflateInit(&stream);
		if (err != Z_OK) return err;

		err = inflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END) {
			inflateEnd(&stream);
			return err == Z_OK ? Z_BUF_ERROR : err;
		}
		*destLen = stream.total_out;

		err = inflateEnd(&stream);
		return err;
	}

	int	ResourcePack::Zlib_Compress(unsigned char* dest,unsigned int* destLen,const unsigned char* source,unsigned int sourceLen)
	{
		z_stream stream;
		int err;

		stream.next_in = (Bytef*)source;
		stream.avail_in = (unsigned int)sourceLen;
		stream.next_out = dest;
		stream.avail_out = (unsigned int)*destLen;
		if ((uLong)stream.avail_out != *destLen) return Z_BUF_ERROR;

		stream.zalloc = &Zlib_User_Alloc;	//0;
		stream.zfree  = &Zlib_User_Free;		//0;
		stream.opaque = (void*)0;

		err = deflateInit(&stream, Z_BEST_SPEED);
		if (err != Z_OK) return err;

		err = deflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END) {
			deflateEnd(&stream);
			return err == Z_OK ? Z_BUF_ERROR : err;
		}
		*destLen = stream.total_out;

		err = deflateEnd(&stream);
		return err;
	}

	ResourcePack::ResourcePack()
	{
		m_nFileListOffset		= 0;
		m_nFilesListNum			= MIN_FILE_NODE_NUM;
		m_pFileNode				= NULL;
		m_fileHandle			= NULL;
		m_bArchiveFileChange	= false;
		m_error			= "";
		memset(m_sFileVersion, 0, sizeof(m_sFileVersion));

		m_RealFileBuffer = NULL;
		m_RealFileBufferSize = 0;
		m_CompressBuffer = NULL;
		m_CompressBufferSize = 0;
		m_UnCompressBuffer = NULL;
		m_CompressBufferSize = 0;
		m_OutBuffer = NULL;
		m_OutBufferSize = 0;
	}

	void ResourcePack::FreeMemory()
	{
		if(m_RealFileBuffer)
		{
			EchoSafeFree(m_RealFileBuffer);
			m_RealFileBuffer = NULL;
			m_RealFileBufferSize = 0;
		}

		if(m_CompressBuffer)
		{
			EchoSafeFree(m_CompressBuffer);
			m_CompressBuffer = NULL;
			m_CompressBufferSize = 0;
		}

		if(m_UnCompressBuffer)
		{
			EchoSafeFree(m_UnCompressBuffer);
			m_UnCompressBuffer = NULL;
			m_UnCompressBufferSize = 0;
		}

		if(m_OutBuffer)
		{
			EchoSafeFree(m_OutBuffer);
			m_OutBuffer = NULL;
			m_OutBufferSize = 0;
		}
	}

	// 析构函数
	ResourcePack::~ResourcePack()
	{
		if(m_fileHandle)
		{
			EchoSafeDelete(m_fileHandle, FileHandleDataStream);
		}
       
        ECHO_DELETE_ARRAY_T(m_pFileNode, FileNode, static_cast<size_t>(m_nFilesListNum));
		FreeMemory();
	}

	void ResourcePack::ResetMember()
	{
		m_nFileListOffset		= 0;
		m_nFilesListNum			= MIN_FILE_NODE_NUM;
		
        ECHO_DELETE_ARRAY_T(m_pFileNode, FileNode, static_cast<size_t>(m_nFilesListNum));

		m_pFileNode				= NULL;
		m_fileHandle			= NULL;
		m_bArchiveFileChange	= false;
		m_error			= "";
		memset(m_sFileVersion, 0, sizeof(m_sFileVersion));
		FreeMemory();
		m_FileNodeMap.clear();
	}
	
	// 创建文件
	bool ResourcePack::createFile(const char *lpszArchiveFile, int nFileNodeNum)
	{
		if(!lpszArchiveFile)
		{
			m_error = "Error in function CLizArchive::CreateFile, lpszArchiveFile is NULL!\n";
			return false;
		}
		if(m_fileHandle)
		{
			m_error = "Error in function CLizArchive::CreateFile, m_pArchiveFile is not NULL!\n";
			EchoSafeDelete(m_fileHandle, FileHandleDataStream);
			m_fileHandle = NULL;
			return false;
		}

		// 判断文件是否存在
		if( PathUtil::IsFileExist( lpszArchiveFile))
		{
			m_error = "Error in function CLizArchive::CreateFile, archive exist!\n";
			EchoSafeDelete(m_fileHandle, FileHandleDataStream);
			m_fileHandle = NULL;
			return false;
		}
		m_sArchiveFileName = lpszArchiveFile;

		FileHandleDataStream archiveStream(lpszArchiveFile, DataStream::WRITE);
		if (archiveStream.fail())
		{
			m_error = "Error in function CLizArchive::CreateFile, open archive file error!\n";
			return false;
		}

		if(nFileNodeNum>=MIN_FILE_NODE_NUM && nFileNodeNum<=MAX_FILE_NODE_NUM)
			m_nFilesListNum = nFileNodeNum;
		if(m_nFilesListNum<MIN_FILE_NODE_NUM)
			m_nFilesListNum = MIN_FILE_NODE_NUM;
		else if(m_nFilesListNum>MAX_FILE_NODE_NUM)
			m_nFilesListNum = MAX_FILE_NODE_NUM;

		m_nFileListOffset = sizeof(m_nFileListOffset) + sizeof(m_nFilesListNum) + sizeof(m_sFileVersion);

		archiveStream.write(&m_nFileListOffset, sizeof(m_nFileListOffset));
		archiveStream.write(&m_nFilesListNum, sizeof(m_nFilesListNum));
		archiveStream.write(m_sFileVersion, sizeof(m_sFileVersion));

		m_pFileNode = ECHO_NEW_ARRAY_T(FileNode, m_nFilesListNum);

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::CreateFile, create archive file list error!\n";
			return false;
		}

		for(int i=0; i<m_nFilesListNum; i++)
		{
			archiveStream.write(&m_pFileNode[i], sizeof(FileNode));
		}
	
        ECHO_DELETE_ARRAY_T(m_pFileNode, FileNode, static_cast<size_t>(m_nFilesListNum));

		return true;
	}

	// 打开文件
	bool ResourcePack::openFile(const char* lpszArchiveFile, ui16 access)
	{
		EchoAssert(lpszArchiveFile);
		EchoAssert(!m_fileHandle);

		String tmpArchiveFile;
		m_FileNodeMap.clear();

		EE_LOCK_MUTEX(m_UnpressLock);

		tmpArchiveFile = lpszArchiveFile;

		ResetMember();
		m_fileHandle = EchoNew(FileHandleDataStream(tmpArchiveFile, access));
		EchoAssert(m_fileHandle);

		m_sArchiveFileName = tmpArchiveFile;

		m_fileHandle->read(&m_nFileListOffset, sizeof(m_nFileListOffset));
		m_fileHandle->read(&m_nFilesListNum, sizeof(m_nFilesListNum));
		m_fileHandle->read(m_sFileVersion, sizeof(m_sFileVersion));

		EchoAssert(m_nFileListOffset!=0);
		EchoAssert(m_nFilesListNum!=0);
		EchoAssert(m_nFilesListNum>=MIN_FILE_NODE_NUM);
		EchoAssert(m_nFilesListNum<=MAX_FILE_NODE_NUM);
	
		ECHO_DELETE_ARRAY_T(m_pFileNode, FileNode, static_cast<size_t>(m_nFilesListNum));

        m_pFileNode = ECHO_NEW_ARRAY_T(FileNode, m_nFilesListNum);

		m_fileHandle->seek(m_nFileListOffset, SEEK_SET);
		for(int i=0; i<m_nFilesListNum; i++)
		{
			m_fileHandle->read(&m_pFileNode[i], sizeof(FileNode));
			if(m_pFileNode[i].m_sFileName[0] != 0 && m_pFileNode[i].m_nUseFlag == FILE_IN_USING)
			{
				m_FileNodeMap.insert(std::pair<String, int>(String(m_pFileNode[i].m_sFileName), i));
			}
		}

		return true;
	}

	// 添加文件
	bool ResourcePack::addFile(const char *lpszFilePath, const char *lpszFileName, int nCompressType)
	{
		if(!lpszFilePath || !lpszFileName)
		{
			m_error = "Error in function CLizArchive::AddFile, lpszFilePath or lpszFileName is NULL!\n";
			return false;
		}

		if (strlen(lpszFileName) >= 64)
		{
			m_error = Echo::StringUtil::Format( "[%s] is too long", lpszFileName);
			return false;
		}

		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::AddFile, archive file is closed!\n";
			return false;
		}

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::AddFile, file list error!\n";
			return false;
		}

		if(isFileExist(lpszFileName))
		{
			m_error = "Error in function CLizArchive::AddFile, the file added is exist!\n";
			return false;
		}

		int nEmptyFilePos = GetEmptyFileNode();
		if(nEmptyFilePos==-1)
		{
			m_error = "Error in function CLizArchive::AddFile, empty file's position is error!\n";
			return false;
		}
		else if(nEmptyFilePos==-2)
		{
			if(!RebuildPacket())
			{
				m_error = "Error in function CLizArchive::AddFile, rebuild packet error!\n";
				return false;
			}
			return addFile(lpszFilePath, lpszFileName, nCompressType);
		}

		if(nCompressType == -1)
		{
			m_pFileNode[nEmptyFilePos].m_nCompressType = (rand()%2);
		}
		else
		{
			m_pFileNode[nEmptyFilePos].m_nCompressType = nCompressType;
		}
		strncpy(m_pFileNode[nEmptyFilePos].m_sFileName, lpszFileName, sizeof(m_pFileNode[nEmptyFilePos].m_sFileName));

		unsigned int RealSize = 0;
		if(!CompressData (m_pFileNode[nEmptyFilePos], lpszFilePath, RealSize))
			return false;

		m_bArchiveFileChange = true;
		return true;
	}

	bool ResourcePack::addFile(const char *lpszFileName, const FileNode* pFileNode, const char *pData, int nSize)
	{
		int					nEmptyFilePos	= -1;
		int					nFileRealSize	= 0;
		//unsigned long		nCompressSizeHash	= 0;

		if(!lpszFileName)
		{
			m_error = "Error in function CLizArchive::AddFile, lpszFileName is NULL!\n";
			return false;
		}

		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::AddFile, archive file is closed!\n";
			return false;
		}

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::AddFile, file list error!\n";
			return true;
		}

		if(isFileExist(lpszFileName))
		{
			m_error = "Error in function CLizArchive::AddFile, the file added is exist!\n";
			printf("Press any key to continue");
			getc(stdin);
			return true;
		}

		nEmptyFilePos = GetEmptyFileNode();
		if(nEmptyFilePos==-1)
		{
			m_error = "Error in function CLizArchive::AddFile, empty file's position is error!\n";
			return false;
		}
		else if(nEmptyFilePos==-2)
		{
			if(!RebuildPacket())
			{
				m_error = "Error in function CLizArchive::AddFile, rebuild packet error!\n";
				return false;
			}
			return addFile(lpszFileName, pFileNode, pData, nSize);
		}

		nFileRealSize = pFileNode->m_nFileRealSize;

		if(nSize<=0)
		{
			m_error = "Error in function CLizArchive::AddFile, compress file error!\n";
			getc(stdin);
			return true;
		}

		m_fileHandle->seek( 0L, SEEK_END);
		ui32 currFileOffset = m_fileHandle->tell();
		m_fileHandle->write(pData, nSize);


		strncpy(m_pFileNode[nEmptyFilePos].m_sFileName, lpszFileName, sizeof(m_pFileNode[nEmptyFilePos].m_sFileName));
		m_pFileNode[nEmptyFilePos].m_nUseFlag = FILE_IN_USING;
		m_pFileNode[nEmptyFilePos].m_nOffset = currFileOffset;
		m_pFileNode[nEmptyFilePos].m_nFileRealSize = nFileRealSize;
		m_pFileNode[nEmptyFilePos].m_nFileCompressSize = nSize;
		m_pFileNode[nEmptyFilePos].m_nCompressType = pFileNode->m_nCompressType;

		m_bArchiveFileChange = true;

		return true;
	}

	// 删除文件
	bool ResourcePack::DelFile(const char *lpszFileName)
	{
		int	nFileNodePos	= 0;
		if(!lpszFileName)
		{
			m_error = "Error in function CLizArchive::DelFile, lpszFileName is NULL!\n";
			return false;
		}

		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::DelFile, archive file is closed!\n";
			return false;
		}

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::DelFile, file list error!\n";
			return true;
		}

		nFileNodePos = GetFileNodeByName(lpszFileName);
		if(nFileNodePos<0)
		{
			m_error = "Error in function CLizArchive::DelFile, the file deleted is not existed!\n";
			return false;
		}

		m_pFileNode[nFileNodePos].m_nUseFlag = FILE_DELETE;

		m_bArchiveFileChange = true;
		return true;
	}

	// 设置资源包版本号
	bool ResourcePack::SetVersion(const char* szVersion)
	{
		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::SetVersion, archive file is closed!\n";
			return false;
		}

		memset(m_sFileVersion, 0, sizeof(m_sFileVersion));
		strcpy(m_sFileVersion, szVersion);
		m_fileHandle->seek(sizeof(m_nFileListOffset)+sizeof(m_nFilesListNum), SEEK_SET);
		m_fileHandle->write(m_sFileVersion, sizeof(m_sFileVersion));

		return true;
	}

	// 关闭文件，覆写头
	bool ResourcePack::closeFile()
	{
		EchoAssert(m_fileHandle);
		
		if(m_bArchiveFileChange)
		{
			m_fileHandle->seek( m_nFileListOffset, SEEK_SET);
			for(int i=0; i<m_nFilesListNum; i++)
			{
				m_fileHandle->write(&m_pFileNode[i], sizeof(FileNode));
			}
		}
		
		EchoSafeDelete(m_fileHandle, FileHandleDataStream);
		return true;
	}

	// 获取版本号
	const char* ResourcePack::getVersion()
	{
		EchoAssert(m_fileHandle);

		return m_sFileVersion;
	}

	// 读文件
	const unsigned char* ResourcePack::readFile(const char* lpszFileName, unsigned int& nOutLength)
	{
		int	nFileNodePos	= 0;

		EchoAssert(lpszFileName);
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);

		nFileNodePos = GetFileNodeByName_fast(lpszFileName);
		if(nFileNodePos<0)
		{
			m_error = "Error in function CLizArchive::ReadFile, the file deleted is not existed!\n";
			return NULL;
		}

		m_UnpressLock.Lock();
		if(UnCompressData (m_pFileNode[nFileNodePos], nOutLength))
		{
			m_UnpressLock.Unlock();
			return m_OutBuffer;
		}
		m_UnpressLock.Unlock();
		nOutLength = 0;
		return NULL;
	}

	bool ResourcePack::RebuildPacket()
	{
		ResourcePack rebuildPacket;
		rebuildPacket.ResetMember();

		String szRebuildPacketName;
		int nFileListNum = 0;

		szRebuildPacketName = StringUtil::Format("%s.new", m_sArchiveFileName.c_str());
		if(m_nFilesListNum*2 > 10000)
			nFileListNum = m_nFilesListNum + 2000;
		else
			nFileListNum = m_nFilesListNum * 2;


        if(PathUtil::IsFileExist(szRebuildPacketName))
        {
            // 如果存在就删除
            PathUtil::DelPath(szRebuildPacketName);
        }
        
		if(!rebuildPacket.createFile(szRebuildPacketName.c_str(), nFileListNum))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, rebuildPacket create file error!\n";
			return false;
		}

		if(!rebuildPacket.openFile(szRebuildPacketName.c_str()))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, rebuildPacket open file error!\n";
			return false;
		}

		rebuildPacket.SetVersion(getVersion());

		if(!rebuildPacket.CopyFileList(m_pFileNode, m_nFilesListNum))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, rebuildPacket copy file list error!\n";
			return false;
		}

		closeFile();

		if(!rebuildPacket.CopyFileCompressData(m_sArchiveFileName.c_str(), m_nFileListOffset+m_nFilesListNum*sizeof(FileNode)))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, rebuildPacket copy file compress data error!\n";
			return false;
		}

		rebuildPacket.closeFile();

		String strFilename = m_sArchiveFileName;
		if(!PathUtil::DelPath(strFilename))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, delete old packet file error!\n";
			return false;
		}

		String strPacketName = szRebuildPacketName;
		String strArchiveName = m_sArchiveFileName;
		if(!PathUtil::RenameFile(strPacketName, strArchiveName))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, rename rebuild packet file error!\n";
			return false;
		}

		if(!openFile(m_sArchiveFileName.c_str()))
		{
			m_error = "Error in function CLizArchive::RebuildPacket, open rebuild packet file error!\n";
			return false;
		}

		return true;
	}

	int ResourcePack::GetEmptyFileNode()
	{
		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::GetEmptyFileNode, archive file is closed!\n";
			return -1;
		}

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::GetEmptyFileNode, file list error!\n";
			return -1;
		}

		for(int i=0; i<m_nFilesListNum; i++)
		{
			if(m_pFileNode[i].m_nUseFlag == FILE_NOT_USING)
				return i;
		}

		return -2;
	}

	bool ResourcePack::SetArchivePath( const char* apszArchivePath )
	{
		if ( NULL == apszArchivePath )
		{
			return false;
		}

		m_sArchiveFilePath = StringUtil::Format("%s/", apszArchivePath );

		return true;
	}

	// 获取文件压缩数据
	bool ResourcePack::GetFileCompressData( const char* aszFileName, char* aszOutPut )
	{
		if ( NULL == aszFileName )
		{
			m_error = "Error in function CLizArchive::GetFileCompressData, aszFileName is NULL!\n";
			return false;
		}

		if ( NULL == aszOutPut )
		{
			m_error = "Error in function CLizArchive::GetFileCompressData, aszOutPut is NULL!\n";
			return false;
		}

		if(!m_fileHandle)
		{
			m_error = "Error in function CLizArchive::GetFileCompressData, archive file is closed!\n";
			return false;
		}

		if(!m_pFileNode)
		{
			m_error = "Error in function CLizArchive::GetFileCompressData, file list error!\n";
			return true;
		}

		if(!isFileExist(aszFileName))
		{
			m_error = "Error in function CLizArchive::GetFileCompressData, the file added is not exist!\n";
			printf("Press any key to Continue");
			getc(stdin);
			return true;
		}

		int liFilePos = GetFileNodeByName( aszFileName );
		if ( liFilePos < 0 )
		{
			return false;
		}
		m_fileHandle->seek( m_pFileNode[liFilePos].m_nOffset, SEEK_SET);
		m_fileHandle->read(aszOutPut, sizeof(char) * m_pFileNode[liFilePos].m_nFileCompressSize);

		return true;
	}

	// 压缩数据
	bool ResourcePack::CompressData(FileNode &fileNode, const char* filePath, unsigned int &nCompressSize)
	{
		const unsigned char	*lpszOut		= NULL;
		unsigned int		nFileRealSize	= 0;
		unsigned int        compressSize	= 0;

		String str = String(filePath) + fileNode.m_sFileName;
		FileHandleDataStream fp(str.c_str(), DataStream::READ);
		if(fp.fail())
		{
			m_error = "Error in function CLizArchive::CompressData, the file added is error!\n";
			return false;
		}

		nFileRealSize = fp.size();
		if(nFileRealSize > m_RealFileBufferSize)
		{
			m_RealFileBufferSize = nFileRealSize;
			if(m_RealFileBuffer)
			{
				EchoSafeFree(m_RealFileBuffer);
			}
			m_RealFileBuffer = (unsigned char*)EchoMalloc(m_RealFileBufferSize);
		}

		fp.seek(0L);
		fp.read(m_RealFileBuffer, nFileRealSize);

		switch(fileNode.m_nCompressType)
		{
		case CT_NONE:
			lpszOut = m_RealFileBuffer;
			nCompressSize = nFileRealSize;
			break;
		case CT_ZLIB:
			if(nFileRealSize*2 > m_CompressBufferSize)
			{
				if(m_CompressBuffer)
				{
					EchoSafeFree(m_CompressBuffer);
				}
				m_CompressBufferSize = (nFileRealSize*2)>(MAX_MEMORY_CACHE)?(nFileRealSize*2):(MAX_MEMORY_CACHE);
				m_CompressBuffer = (unsigned char*)EchoMalloc(m_CompressBufferSize);
			}
			if(nFileRealSize >= 8)
			{
				compressSize = m_CompressBufferSize;
				Zlib_Compress(m_CompressBuffer, &compressSize, m_RealFileBuffer, nFileRealSize);
				nCompressSize = compressSize;
			}
			else
			{
				memcpy(m_CompressBuffer, m_RealFileBuffer, nFileRealSize);
				nCompressSize = nFileRealSize;
			}
			lpszOut = m_CompressBuffer;
			break;
		default:
			lpszOut = NULL;	

			return false;
		}
		if(nCompressSize<=0)
		{
			m_error = "Error in function CLizArchive::CompressData, compress file error!\n";
			getc(stdin);
			lpszOut = NULL;	
			return true;
		}

		// 文件指针移到尾部
		m_fileHandle->seek( 0, SEEK_END);
		ui32 currFileOffset = m_fileHandle->tell();
		m_fileHandle->write(lpszOut, nCompressSize);

		fileNode.m_nUseFlag = FILE_IN_USING;
		fileNode.m_nOffset = currFileOffset;
		fileNode.m_nFileRealSize = nFileRealSize;
		fileNode.m_nFileCompressSize = nCompressSize;

		lpszOut = NULL;

		return true;
	}

	// 获取文件列表
	const ResourcePack::FileNode* ResourcePack::GetFileList(int& nFileListCount)
	{
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);

		nFileListCount = m_nFilesListNum;
		return m_pFileNode;
	}

	bool ResourcePack::CopyFileList(const FileNode* pFileNode, int nFileNodeCount)
	{
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);

		if(m_nFilesListNum < nFileNodeCount)
		{
			m_error = "Error in function CLizArchive::CopyFileList, file list number is not enough!\n";
			return false;
		}

		unsigned long nFileOffset = 0;
		nFileOffset = sizeof(FileNode)*(m_nFilesListNum - nFileNodeCount);

		for(int i=0; i<nFileNodeCount; i++)
		{
			memcpy(&m_pFileNode[i], &pFileNode[i], sizeof(FileNode));
			m_pFileNode[i].m_nOffset += nFileOffset;
		}

		m_bArchiveFileChange = true;
		return true;
	}

	bool ResourcePack::CopyFileCompressData(const char* lpszArchiveFile, unsigned long nOffset)
	{
		char p = 0;
		FileHandleDataStream *pfArchiveFile = NULL;

		EchoAssert(lpszArchiveFile);
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);
		
		pfArchiveFile = EchoNew(FileHandleDataStream(lpszArchiveFile, DataStream::READ));
		if(!pfArchiveFile)
		{
			m_error = "Error in function CLizArchive::CopyFileCompressData, open old archive file error!\n";
			return false;
		}

		pfArchiveFile->seek( 0, SEEK_END);

		pfArchiveFile->seek(nOffset, SEEK_SET);
		m_fileHandle->seek( m_nFileListOffset + m_nFilesListNum*sizeof(FileNode), SEEK_SET);
        char* pScr = (char*)EchoMalloc(pfArchiveFile->size()-nOffset);
		pfArchiveFile->read(pScr, pfArchiveFile->size() - nOffset);
		m_fileHandle->write(pScr, pfArchiveFile->size() - nOffset);
        
        EchoSafeFree(pScr);
        
		EchoSafeDelete(pfArchiveFile, FileHandleDataStream);
		pfArchiveFile = NULL;
		m_bArchiveFileChange = true;
		return true;
	}

	const char* ResourcePack::getError()
	{
		return m_error.c_str();
	}

	// 判断文件是否存在
	bool ResourcePack::isFileExist(const char* lpszFileName)
	{
		EchoAssert(lpszFileName);
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);

		return m_FileNodeMap.find(lpszFileName) == m_FileNodeMap.end() ? false : true;
	}

	int ResourcePack::GetFileNodeByName_fast(const char* lpszFileName)
	{
		EchoAssert(lpszFileName);
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);
		
		FileNodeMap::iterator it = m_FileNodeMap.find(String(lpszFileName));
		if(it != m_FileNodeMap.end())
		{
			return it->second;
		}

		return -1;
	}

	int ResourcePack::GetFileNodeByName(const char* lpszFileName)
	{
		EchoAssert(lpszFileName);
		EchoAssert(m_fileHandle);
		EchoAssert(m_pFileNode);

		for(int i=0; i<m_nFilesListNum; i++)
		{
			if(m_pFileNode[i].m_nUseFlag == FILE_IN_USING && strcmp(m_pFileNode[i].m_sFileName, lpszFileName)==0)
				return i;
		}

		return -1;
	}

	void ResourcePack::ReleaseExtraMemory()
	{
		if(m_OutBufferSize > MAX_MEMORY_CACHE)
		{
			EchoSafeFree(m_OutBuffer);
			m_OutBuffer = NULL;
			m_OutBufferSize = 0;
		}
		if(m_UnCompressBufferSize > MAX_MEMORY_CACHE)
		{
			EchoSafeFree(m_UnCompressBuffer);
			m_UnCompressBuffer = NULL;
			m_UnCompressBufferSize = 0;
		}
	}

	bool ResourcePack::UnCompressData (const FileNode& fileNode, unsigned int& nRealSize)
	{
		if(m_OutBufferSize > MAX_MEMORY_CACHE)
		{
			EchoSafeFree(m_OutBuffer);
			m_OutBuffer = NULL;
			m_OutBufferSize = 0;
		}
		if(m_UnCompressBufferSize > MAX_MEMORY_CACHE)
		{
			EchoSafeFree(m_UnCompressBuffer);
			m_UnCompressBuffer = NULL;
			m_UnCompressBufferSize = 0;
		}

		nRealSize = fileNode.m_nFileRealSize;
		if(nRealSize > m_OutBufferSize)
		{
			if(m_OutBuffer)
			{
				EchoSafeFree(m_OutBuffer);
			}
			m_OutBufferSize = nRealSize;
			m_OutBuffer = (unsigned char*)EchoMalloc(nRealSize);
		}

		switch(fileNode.m_nCompressType)
		{
		case CT_NONE:
			{
				m_fileHandle->seek(fileNode.m_nOffset, SEEK_SET);
				m_fileHandle->read(m_OutBuffer, nRealSize);
			}
			break;
		case CT_ZLIB:
			{
				unsigned int compressSize = fileNode.m_nFileCompressSize;
				if(m_UnCompressBufferSize < compressSize)
				{
					if(m_UnCompressBuffer)
					{
						EchoSafeFree(m_UnCompressBuffer);
					}
					m_UnCompressBufferSize = compressSize;
					m_UnCompressBuffer = (unsigned char*)EchoMalloc(compressSize);
				}
				m_fileHandle->seek(fileNode.m_nOffset, SEEK_SET);
				m_fileHandle->read(m_UnCompressBuffer, compressSize);
				if(nRealSize >= 8)
				{
					unsigned int unzipSize = nRealSize;
					int nRet = Zlib_UnCompress(m_OutBuffer, &unzipSize, m_UnCompressBuffer, compressSize);
					if( Z_OK != nRet )
						return false;
				}
				else
				{
					memcpy(m_OutBuffer, m_UnCompressBuffer, nRealSize);
				}
				if(m_UnCompressBufferSize > MAX_MEMORY_CACHE)
				{
					EchoSafeFree(m_UnCompressBuffer);
					m_UnCompressBuffer = NULL;
					m_UnCompressBufferSize = 0;
				}
			}
			break;
		default:
			return false;
		}

		return true;
	}
}
