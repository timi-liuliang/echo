#ifndef __APKFileSystemArchive_H__
#define __APKFileSystemArchive_H__

#ifdef ECHO_PLATFORM_ANDROID

#include "FileIO/Archive.h"
#include "FileIO/ArchiveManager.h"
#include "FileIO/ArchiveFactory.h"
#include <android/asset_manager.h>

namespace Echo
{
	class APKFileSystemArchive : public Archive
	{
	private:
		AAssetManager* mAssetMgr;
		String mPathPreFix;

	public:
		APKFileSystemArchive(const String& name, const String& archType, AAssetManager* assetMgr);
		~APKFileSystemArchive();

		/// @copydoc Archive::isCaseSensitive
		bool isCaseSensitive(void) const;

		/// @copydoc Archive::load
		void load();
		/// @copydoc Archive::unload
		void unload();

		/// @copydoc Archive::open
		DataStream* open(const String& filename);

		/// @copydoc Archive::list
		StringArray list(bool recursive = true, bool dirs = false);

		/// @copydoc Archive::listFileInfo
		FileInfoList* listFileInfo(bool dirs = false);

		/// @copydoc Archive::exists
		bool exists(const String& filename);

		// 判断文件是否异步加载文件
		virtual bool isAsync( const char* fileName) { return false; }

		// 设置异步加载回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback) { return false; }
	};

	class APKFileSystemArchiveFactory : public ArchiveFactory
	{
	public:
		APKFileSystemArchiveFactory(AAssetManager* assetMgr) 
			: mAssetMgr(assetMgr) 
		{}

		virtual ~APKFileSystemArchiveFactory()
		{}

		/// @copydoc FactoryObj::getType
		const String& getType(void) const;
		/// @copydoc FactoryObj::createInstance
		Archive* createInstance(const String& name)
		{
			return EchoNew(APKFileSystemArchive(name, getType(), mAssetMgr));
		}
		/// @copydoc FactoryObj::destroyInstance
		void destroyInstance(Archive* arch)
		{
			EchoSafeDelete(arch, Archive); 
		}

	private:
		AAssetManager* mAssetMgr;
	};
}

#endif
#endif
