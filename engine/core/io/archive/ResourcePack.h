#pragma once

#include <engine/core/io/DataStream.h>
#include "engine/core/thread/Threading.h"

#define MAX_FILE_NODE_NUM 20000
#define MIN_FILE_NODE_NUM 50			

#define MAX_MEMORY_CACHE 1024*1024

namespace Echo
{
	/**
	 * 资源包
	 */
	class ResourcePack
	{
	public:
		// 文件使用状态
		enum FILE_USING_TYPE
		{
			FILE_NOT_USING = 0,
			FILE_IN_USING,
			FILE_DELETE
		};

		// 压缩类型
		enum FILE_COMPRESS_TYPE
		{
			CT_NONE = 0,		// 无压缩
			CT_ZLIB				// ZLIB压缩
		};

		// 文件节点
		struct FileNode
		{
			char			m_sFileName[64];
			ui32			m_nOffset;
			ui32			m_nFileRealSize;
			ui32			m_nFileCompressSize;
			ui8				m_nCompressType;
			ui8				m_nUseFlag;

			FileNode()
			{
				memset(m_sFileName, 0, sizeof(m_sFileName));
				m_nUseFlag = FILE_NOT_USING;
				m_nOffset = 0;
				m_nFileRealSize = 0;
				m_nFileCompressSize = 0;
				m_nCompressType = 0;
			}
		};
		typedef map<String, int>::type FileNodeMap;

	public:
		ResourcePack();
		~ResourcePack();

		// 打开文件
		bool openFile(const char* lpszArchiveFile, ui16 access = DataStream::READ);

		// 关闭文件，覆写头
		bool closeFile();
		
		// 获取版本号
		const char* getVersion();

		// 读取文件
		const unsigned char* readFile(const char* lpszFileName, unsigned int& nOutLength) ;

		void ReleaseExtraMemory();

		bool CopyFileList(const FileNode* pFileNode, int nFileNodeCount);

		bool CopyFileCompressData(const char* lpszArchiveFile, unsigned long nOffset);

		const FileNode* GetFileList(int& nFileListCount);

		const char* getError();

		// 文件是否存在
		bool isFileExist(const char* lpszFileName);

		// 创建文件
		bool createFile(const char *lpszArchiveFile, int nFileNodeNum = 512);
		bool addFile(const char *lpszFilePath, const char *lpszFileName, int nCompressType);
		bool addFile(const char *lpszFileName, const FileNode* pFileNode, const char *pData, int nSize);
		bool DelFile(const char *lpszFileName);
		bool SetVersion(const char* szVersion);
		bool SetArchivePath( const char* apszArchivePath );
		bool GetFileCompressData( const char* aszFileName, char* aszOutPut );
	
	private:
		int GetEmptyFileNode();
		bool RebuildPacket();
	
	public:
		bool CompressData(FileNode& fileNode, const char* filePath, unsigned int& nCompressSize);
		int	Zlib_Compress(unsigned char* dest,unsigned int* destLen,const unsigned char* source,unsigned int sourceLen);

	private:
		int GetFileNodeByName(const char* lpszFileName);

		int GetFileNodeByName_fast(const char* lpszFileName);

		void ResetMember();

		void FreeMemory();

		bool UnCompressData(const FileNode& fileNode, unsigned int& nRealSize);

		int Zlib_UnCompress(unsigned char* dest,unsigned int* destLen,const unsigned char* source,unsigned int sourceLen);
	
	private:
		String				m_error;						// 错误信息
		unsigned char*		m_RealFileBuffer;				// 文件缓存(解压)
		ui32				m_RealFileBufferSize;			// 文件缓存大小(解压)
		unsigned char*		m_CompressBuffer;				// 文件缓存(压缩)
		ui32				m_CompressBufferSize;			// 文件缓存大小

		unsigned char*		m_UnCompressBuffer;				
		ui32				m_UnCompressBufferSize;		
		unsigned char*		m_OutBuffer;					
		ui32				m_OutBufferSize;

	private:
		FileHandleDataStream*	m_fileHandle;					// 文件句柄
		ui32					m_nFileListOffset;				// m_pFileNode数据偏移量	
		char					m_sFileVersion[64];				// 打包文件版本号
		FileNode*				m_pFileNode;					// 文件结点数据
		i32						m_nFilesListNum;				// 文件数量
		FileNodeMap				m_FileNodeMap;					// 文件目录(快速文件结点查询)
		String					m_sArchiveFileName;				// 文件存储路径
		bool					m_bArchiveFileChange;
		String					m_sArchiveFilePath;				// 当前存档文件路径

		EE_MUTEX(m_UnpressLock);
	};
}
