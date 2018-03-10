#pragma once

#include <engine/core/resource/ArchiveFactory.h>
#include "engine/modules/net/NetConnectionManager.h"

#ifdef ECHO_USE_LIBEVENT
namespace Echo
{
	/**
	* 网络文件系统存档
	*/
	class HttpFileSystemArchive : public Archive
	{
	public:
		HttpFileSystemArchive(const String& name, const String& archType);
		~HttpFileSystemArchive();

		// 是否大小写敏感
		bool isCaseSensitive(void) const;

		// 加载
		void load();

		// 卸载
		void unload();

		// 打开文件
		DataStream* open(const String& filename);

		// 获取全路径
		String location(const String& filename);

		// 列出所有文件
		StringArray list(bool recursive = true, bool dirs = false);

		// 获取文件信息列表
		FileInfoList* listFileInfo(bool dirs = false);

		// 判断文件是否存在
		bool exists(const String& filename);

		// 是否可以添加文件(新文件加入到任意一个FileSystemArchive中都是合理的,所以直接返回true)
		virtual bool isCanAddFile(const String& fullPath) { return true; }

		// 添加文件
		virtual bool addFile(const String& fullPath);

		// 移除文件
		virtual bool removeFile(const char* fileName);

		// 是否异步
		virtual bool isAsync(const char* fileName) { return true; }

		// 设置异步回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback); 

	public: 
		// 资源加载成功回调
		void onOpenFileComplete(const Echo::String& name, void* data, size_t size, NetErrCode::ErrCode err); 

	private:
		typedef map<String, vector<EchoOpenResourceCb>::type >::type ResourceCallbackMapping; 

		String						m_url;			// 服务器地址
		set<String>::type			m_files;		// 所有文件(文件名称，文件路径)
		ResourceCallbackMapping		m_callbacks;	// 
	};

	/**
	 * 存档工厂类
	 */
	class HttpFileSystemArchiveFactory : public ArchiveFactory
	{
	public:
		virtual ~HttpFileSystemArchiveFactory() {}

		// 获取类型
		const String& getType(void) const;

		// 创建实例
		Archive* createInstance(const String& name) { return EchoNew(HttpFileSystemArchive(name, "http")); }

		// 销毁实例
		void destroyInstance(Archive* arch) { EchoDelete(arch); }
	};
}
#else
namespace Echo
{
	class HttpFileSystemArchiveFactory : public ArchiveFactory
	{
	public:
		virtual ~HttpFileSystemArchiveFactory() {}

		// 获取类型
		const String& getType(void) const
		{
			static String name = "http";
			return name;
		}

		virtual Archive* createInstance(const String& name) { return nullptr; }
		virtual void destroyInstance(Archive*) {}
	};
}
#endif
