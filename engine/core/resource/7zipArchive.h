#pragma once

#include <engine/core/Thread/Threading.h>
#include "ArchiveFactory.h"

namespace Echo
{
	/**
	* 7zipArchive(Sevenzip Sz)
	*/
	class SzArchive : public Archive
	{
	public:
		SzArchive(const String& name, const String& archType);
		~SzArchive();

		// 是否大小写敏感
		bool isCaseSensitive(void) const;

		// 加载
		void load();

		// 卸载
		void unload();

		// 打开文件
		DataStream* open(const String& filename);

		// 列出所有文件
		StringArray list(bool recursive = true, bool dirs = false);

		// 获取文件信息列表
		FileInfoList* listFileInfo(bool dirs = false);

		// 判断文件是否存在
		bool exists(const String& filename);

		// 是否异步
		virtual bool isAsync(const char* fileName) { return false; }

		// 设置异步回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback) { return false; }

	public:
		// 静态方法，解压资源包
		static void extractTo(const String& szip, const String& outPath);

		// 静态方法，压缩资源包
		static void updateArchive(const String& szip, const String& inPath);
	private:
		String						m_name;			// 名称
		map<String, int>::type		m_files;		// 文件名与索引映射
		void*						m_7zipHandle;	// 7zip操作
		Echo::Mutex					m_7zipMutexLock;// 线程锁
	};

	/**
	* 存档工厂类
	*/
	class SzArchiveFactory : public ArchiveFactory
	{
	public:
		virtual ~SzArchiveFactory() {}

		// 获取类型
		const String& getType(void) const;

		// 创建实例
		Archive* createInstance(const String& name) { return EchoNew(SzArchive(name, "7zip")); }

		// 销毁实例
		void destroyInstance(Archive* arch) { EchoSafeDelete(arch, Archive); }
	};
}
