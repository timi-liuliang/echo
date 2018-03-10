#ifndef __ECHO_ARCHIVEFACTORY_H__
#define __ECHO_ARCHIVEFACTORY_H__

#include "engine/core/resource/Archive.h"

namespace Echo
{

	class ECHO_EXPORT_FOUNDATION ArchiveFactory
	{
	public:
		/** Returns the factory type.
		@return
		The factory type.
		*/
		virtual const String& getType() const = 0;

		/** Creates a new object.
		@param name Name of the object to create
		@return
		An object created by the factory. The type of the object depends on
		the factory.
		*/
		virtual Archive* createInstance( const String& name ) = 0;    
		/** Destroys an object which was created by this factory.
		@param ptr Pointer to the object to destroy
		*/
		virtual void destroyInstance( Archive* ) = 0;    

		virtual ~ArchiveFactory() {}
		// No methods, must just override all methods inherited from ArchiveFactory
	};
}

#endif
