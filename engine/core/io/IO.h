#pragma once

#include <functional>
#include "engine/core/thread/Threading.h"
#include "DataStream.h"
#include "archive/Archive.h"
#include "FileSystem.h"

namespace Echo
{
	/**
	 * 资源管理器 
	 */
	class IO
	{
	public:
		EE_AUTO_MUTEX

		typedef list<Archive*>::type				 ArchiveList;
		typedef map<String, Archive*>::type			 FileArchiveMapping;
	public:
		virtual ~IO();

		// get instance
		static IO* instance();

		// 设置引擎资源路径
		void setResPath(const String& resPath);

		// 设置用户资源路径
		void setUserPath(const String& userPath);

		// 打开资源
		DataStream* open(const String& resourceName);

		// 重置
		void reset();

		// 添加资源存档
		void addArchive(const String& name, const String& locType, bool isOverWrite = false);

		// 移除资源存档
		void removeArchive(const String& name);

		// 异步打开资源
		void openResourceAsync(const String& resourceName, EchoOpenResourceCb callback);

		// 是否为异步资源
		bool isAsync( const char* resouceName);

		// 判断资源是否存在
		bool isResourceExists(const String& filename);

		// 获取文件全路径(相应于存档类型)
		String getFullPath(const String& filename);

		// 通过全路径获取资源路径
		bool covertFullPathToResPath(const String& fullPath, String& resPath);

		// 根据组获取所有资源存档
		const ArchiveList& getArchiveList() { return m_archives; }

		// 获取指定后缀名的所有文件(.bmp|.tga)
		void listFilesWithExt( StringArray& oFiles, const char* extWithDot);
		int getArchiveCount() const { return (int)m_resourceIndexCaseSensitive.size(); }

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
		IO();

		// 添加到索引
		void addToIndex(const String& filename, Archive* arch, bool isOverWrite=false);

		// 移除到索引
		void removeFromIndex( const String& filename);

		// 获取文件所在的存档
		Archive* FindFileArchive(const String& filename);

	protected:
		FileSystem*					m_resFileSystem;					// 引擎资源管理("Res://")
		FileSystem*					m_userFileSystem;					// 用户资源管理("User://")
		ArchiveList					m_archives;							// 存档列表
		FileArchiveMapping			m_resourceIndexCaseSensitive;		// 文件名与存档映射关系
	};
}
