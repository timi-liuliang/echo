#ifndef __ECHO_ZIP_ARCHIVE_H__
#define __ECHO_ZIP_ARCHIVE_H__

#include <engine/core/resource/DataStream.h>
#include "engine/core/resource/Archive.h"
#include "engine/core/resource/ArchiveFactory.h"
#include "ResourcePack.h"

namespace Echo
{
	/**
	 * 资源包存档
	 */
	class ZipArchive : public Archive 
	{
		EE_AUTO_MUTEX
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
	};

	/**
	 * ZipArchive工厂
	 */
	class ZipArchiveFactory : public ArchiveFactory
	{
	public:
		virtual ~ZipArchiveFactory() {}

		// 获取当前类型
		const String& getType(void) const{ static String strName = "pkg"; return strName;}

		// 创建实例
		Archive *createInstance(const String& strName) { return EchoNew(ZipArchive(strName, "pkg"));}

		// 销毁实例
		void destroyInstance(Archive* arch) { EchoSafeDelete(arch, Archive); }
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

#endif