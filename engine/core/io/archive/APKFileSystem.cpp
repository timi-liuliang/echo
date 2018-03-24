#include "Engine/Core.h"

#ifdef ECHO_PLATFORM_ANDROID

#include "Engine/Resource/APKFileSystem.h"
#include "Util/LogManager.h"

namespace Echo
{
	APKFileSystemArchive::APKFileSystemArchive(const String& name, const String& archType, AAssetManager* assetMgr)
		: Archive(name, archType), mAssetMgr(assetMgr)
	{
		if (mName.size() > 0 && mName[0] == '/')
			mName.erase(mName.begin());

		mPathPreFix = mName;
		if (mPathPreFix.size() > 0)
			mPathPreFix += "/";
	}

	APKFileSystemArchive::~APKFileSystemArchive()
	{
		unload();
	}

	bool APKFileSystemArchive::isCaseSensitive() const
	{
		return true;
	}

	void APKFileSystemArchive::load()
	{

	}

	void APKFileSystemArchive::unload()
	{

	}

	DataStream* APKFileSystemArchive::open(const String& filename)
	{
		DataStream* stream = NULL;
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_BUFFER);
		if(asset)
		{
			off_t length = AAsset_getLength(asset);
			void* membuf = ECHO_MALLOC(length);
			memcpy(membuf, AAsset_getBuffer(asset), length);
			AAsset_close(asset);

			stream = EchoNew(MemoryDataStream(membuf, length, true, true));
		}
		return stream;
	}

	StringArray APKFileSystemArchive::list(bool recursive, bool dirs)
	{
		StringArray strs;

		AAssetDir* dir = AAssetManager_openDir(mAssetMgr, mName.c_str());
		const char* fileName = NULL;
		while((fileName = AAssetDir_getNextFileName(dir)) != NULL)
		{
			strs.push_back(fileName);
		}
		AAssetDir_close(dir);

		return strs;
	}

	//-----------------------------------------------------------------------
	FileInfoList* APKFileSystemArchive::listFileInfo(bool dirs)
	{
		// Note that we have to tell the SharedPtr to use ECHO_DELETE_T not ECHO_DELETE by passing category
		FileInfoList* ret = EchoNew(FileInfoList);

		//findFiles("*", dirs, 0, ret);

		return ret;
	}

	bool APKFileSystemArchive::exists(const String& filename)
	{
		AAsset* asset = AAssetManager_open(mAssetMgr, (mPathPreFix + filename).c_str(), AASSET_MODE_UNKNOWN);
		if(asset)
		{
			AAsset_close(asset);
			return true;
		}
		return false;
	}

	const String& APKFileSystemArchiveFactory::getType() const
	{
		static String type = "APKFileSystem";
		return type;
	}
}

#endif
