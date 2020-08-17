#pragma once

#include <engine/core/io/stream/MemoryDataStream.h>
#include "Archive.h"
#include "ResourcePack.h"

namespace Echo
{
	class ZipArchive : public Archive 
	{
	public:
		ZipArchive(const String& strName, const String& strArchType);
		virtual ~ZipArchive();

		// 是否大小写敏感
		bool isCaseSensitive(void) const;

		// 加载
		void load();

		// 卸载
		void unload();

		// 打开文件
		DataStream* open(const String& strFilename);

		// 罗列所有文件
		StringArray list(bool bRecursive = true, bool bDirs = false);

		// 列出文件信息
		FileInfoList* listFileInfo(bool bDirs = false);

		// 是否已存在
		bool exists(const String& strFilename);

		// 是否异步
		virtual bool isAsync(const char* fileName) { return false; }

		// 设置异步加载回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback) { return false; }

	protected:
		ResourcePack		m_resourcePack;		// 资源包
		bool				mLoaded;			// 加载状态，是否已加载成功
		EE_MUTEX			(m_mutex)
	};

	/**
	 * 资源包数据流
	 */
	class ResourcePackDataStream : public MemoryDataStream
	{
	public:
		ResourcePackDataStream(const String& strName, void* pvMem, size_t uSize, bool bFreeOnClose = false, bool bReadOnly = false)
			:	MemoryDataStream(strName, pvMem, uSize, bFreeOnClose, bReadOnly) {}
	};
}