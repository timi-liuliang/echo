#pragma once

#include "engine/core/util/Singleton.h"
#include "engine/core/memory/MemManager.h"
#include "engine/core/resource/ArchiveFactory.h"

namespace Echo
{
	/**
	 * 存档管理器
	 */
	class ECHO_EXPORT_FOUNDATION ArchiveManager : public Singleton<ArchiveManager>
	{
		typedef map<String, ArchiveFactory*>::type ArchiveFactoryMap;
		typedef map<String, Archive*>::type ArchiveMap;
	public:
		static ArchiveManager* Instance();

		/** Default constructor - should never get called by a client app.
		*/
		ArchiveManager() {}
		/** Default destructor.
		*/
		virtual ~ArchiveManager();

		/** Opens an archive for file reading.
		@remarks
		The archives are created using class factories within
		extension libraries.
		@param filename
		The filename that will be opened
		@param refLibrary
		The library that contains the data-handling code
		@returns
		If the function succeeds, a valid pointer to an Archive
		object is returned.
		@par
		If the function fails, an exception is thrown.
		*/
		Archive* load( const String& filename, const String& archiveType);

		/** Unloads an archive.
		@remarks
		You must ensure that this archive is not being used before removing it.
		*/
		void unload(Archive* arch);
		/** Unloads an archive by name.
		@remarks
		You must ensure that this archive is not being used before removing it.
		*/
		void unload(const String& filename);

		/** Get an iterator over the Archives in this Manager. */
		ArchiveMap& getArchiveIterator(void);

		void addArchiveFactory(ArchiveFactory* factory);

        // 检索文件相关的归档接口是否存在
        DataStream* exists(const String& filename);

	protected:
		/// Factories available to create archives, indexed by archive type (String identifier e.g. 'Zip')
		ArchiveFactoryMap mArchFactories;
		/// Currently loaded archives
		ArchiveMap mArchives;
	};
}
