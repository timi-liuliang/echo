#ifndef __FileSystem_H__
#define __FileSystem_H__

#include "engine/core/resource/Archive.h"
#include "engine/core/resource/ArchiveFactory.h"

namespace Echo
{
	/**
	 * 文件系统存档
	 */
	class FileSystemArchive : public Archive 
	{
	public:
		FileSystemArchive(const String& name, const String& archType );
		~FileSystemArchive();

		bool isCaseSensitive(void) const;

		void load();

		void unload();

		DataStream* open(const String& filename);

		String location(const String& filename);

		StringArray list(bool recursive = true, bool dirs = false);

		FileInfoList* listFileInfo(bool dirs = false);

		bool exists(const String& filename);

		// 是否可以添加文件(新文件加入到任意一个FileSystemArchive中都是合理的,所以直接返回true)
		virtual bool isCanAddFile(const String& fullPath) { return true; }

		// 添加文件
		virtual bool addFile(const String& fullPath);

		// 移除文件
		virtual bool removeFile( const char* fileName);

		// 是否异步
		virtual bool isAsync(const char* fileName) { return false; }

		// 设置异步加载回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback) { return false; }

	private:
		map<String, String>::type		m_files;		// 所有文件(文件名称，文件路径)
	};

	class FileSystemArchiveFactory : public ArchiveFactory
	{
	public:
		virtual ~FileSystemArchiveFactory() {}

		const String& getType(void) const;

		Archive* createInstance(const String& name) 
		{
			return EchoNew(FileSystemArchive(name, "filesystem"));
		}

		void destroyInstance(Archive* arch) { EchoSafeDelete(arch, Archive); }
	};
}

#endif
