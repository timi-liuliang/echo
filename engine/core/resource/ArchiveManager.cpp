#include "ArchiveManager.h"
#include "ArchiveFactory.h"
#include "Archive.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/LogManager.h"

namespace Echo
{
	typedef void (*createFunc)( Archive**, const String& );

	//---------------------------------------------------------------------

	ArchiveManager* ArchiveManager::Instance()
	{
		EchoAssert(ms_pSingleton);
		return ms_pSingleton;
	}

	//-----------------------------------------------------------------------
	Archive* ArchiveManager::load( const String& filename, const String& archiveType)
	{
		ArchiveMap::iterator i = mArchives.find(filename);
		Archive* pArch = 0;

		if (i == mArchives.end())
		{
			// Search factories
			ArchiveFactoryMap::iterator it = mArchFactories.find(archiveType);
			if (it == mArchFactories.end())
			{
				EchoException("Error: Cannot find an archive factory  to deal with archive of type %s in ArchiveManager::load", archiveType.c_str());
			}
			pArch = it->second->createInstance(filename);
			pArch->load();
			mArchives[filename] = pArch;
		}
		else
		{
			EchoLogError( "Archive [%s] is not unique or you should only load it once!", filename.c_str());

			pArch = i->second;
		}
		return pArch;
	}
	//-----------------------------------------------------------------------
	void ArchiveManager::unload(Archive* arch)
	{
		unload(arch->getName());
	}

	// ж�ش浵
	void ArchiveManager::unload(const String& filename)
	{
		ArchiveMap::iterator i = mArchives.find(filename);
		if (i != mArchives.end())
		{
			i->second->unload();
			// Find factory to destroy
			ArchiveFactoryMap::iterator fit = mArchFactories.find(i->second->getType());
			if (fit != mArchFactories.end())
			{
				fit->second->destroyInstance(i->second);
				mArchives.erase(i);
			}
			else
			{
				// Factory not found
				EchoLogError("Error: Cannot find an archive factory to deal with  archive of type %s in ArchiveManager::~ArchiveManager", i->second->getType().c_str());
			}
		}
	}
	//-----------------------------------------------------------------------
	ArchiveManager::ArchiveMap& ArchiveManager::getArchiveIterator(void)
	{
		return mArchives;
	}
	//-----------------------------------------------------------------------
	ArchiveManager::~ArchiveManager()
	{
		// Unload & delete resources in turn
		for( ArchiveMap::iterator it = mArchives.begin(); it != mArchives.end(); ++it )
		{
			Archive* arch = it->second;
			// Unload
			arch->unload();
			// Find factory to destroy
			ArchiveFactoryMap::iterator fit = mArchFactories.find(arch->getType());
			if (fit == mArchFactories.end())
			{
				// Factory not found
				EchoException("Error: Cannot find an archive factory to deal with archive of type %s in ArchiveManager::~ArchiveManager", arch->getType().c_str());
			}
			fit->second->destroyInstance(arch);

		}
		// Empty the list
		mArchives.clear();
	}
	//-----------------------------------------------------------------------
	void ArchiveManager::addArchiveFactory(ArchiveFactory* factory)
	{        
		mArchFactories.insert( ArchiveFactoryMap::value_type( factory->getType(), factory ) );
		EchoLogInfo("ArchiveFactory for archive type %s registered.", factory->getType().c_str());
	}
    //-----------------------------------------------------------------------
    DataStream* ArchiveManager::exists(const String& filename)
    {
        Archive* archive = NULL;
        for( ArchiveMap::iterator it = mArchives.begin(); it != mArchives.end(); ++it )
		{
			archive = it->second;
            if(archive->exists(filename))
            {
                return archive->open(filename);
            }
        }
        return NULL;
    }
}
