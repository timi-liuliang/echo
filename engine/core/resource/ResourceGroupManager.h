#pragma once

#include <functional>
#include "engine/core/Thread/Threading.h"
#include "engine/core/resource/DataStream.h"
#include "engine/core/resource/Archive.h"
#include "engine/core/util/Singleton.h"

namespace Echo
{
	/**
	 * 资源管理器 
	 */
	class ResourceGroupManager
	{
		__DeclareSingleton(ResourceGroupManager);

	public:
		EE_AUTO_MUTEX

		typedef list<Archive*>::type				 ArchiveList;
		typedef map<String, Archive*>::type			 FileArchiveMapping;
	public:
		ResourceGroupManager();
		virtual ~ResourceGroupManager();

		// 重置
		void reset();

		// 添加资源存档
		void addArchive(const String& name, const String& locType, bool isOverWrite = false);

		// 移除资源存档
		void removeArchive(const String& name);

		// 打开资源
		DataStream* openResource(const String& resourceName, const char* archiveName=NULL);

		// 异步打开资源
		void openResourceAsync(const String& resourceName, EchoOpenResourceCb callback);

		// 是否为异步资源
		bool isAsync( const char* resouceName);

		// 判断资源是否存在
		bool isResourceExists(const String& filename);

		// 获取文件全路径(相应于存档类型)
		String getFileLocation(const String& filename);

		// 根据组获取所有资源存档
		const ArchiveList& getArchiveList() { return m_archiveList; }

		// 获取指定后缀名的所有文件(.bmp|.tga)
		void listFilesWithExt( StringArray& oFiles, const char* extWithDot);
		void foreach_archive_unsafe(std::function<bool (const Echo::String&, const int)> callback);
		int getArchiveCount() const { return m_resourceIndexCaseSensitive.size(); }

		// 获取指定目录和指定后缀的资源存档
		void listFileWithPathAndExt(StringArray& oFiles, const char* path, const char* extWithDot);

		// 添加文件
		void addFile(const String& archiveType, const String& fullPath);

		// 移除文件
		void removeFile( const String& fileName);

		// 获取文件所在的存档
		Archive* getArchiveByFileName( const char* fileName);

		// 根据存档名称获取存档
		Archive* getArchiveByName( const char* archiveName);

	private:
		// 添加到索引
		void addToIndex(const String& filename, Archive* arch, bool isOverWrite=false);

		// 移除到索引
		void removeFromIndex( const String& filename);

		// 获取文件所在的存档
		Archive* FindFileArchive(const String& filename);

	protected:
		ArchiveList					m_archiveList;							// 存档列表
		FileArchiveMapping			m_resourceIndexCaseSensitive;			// 文件名与存档映射关系
	};
}
