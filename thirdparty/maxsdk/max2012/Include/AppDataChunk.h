//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

#include "CoreExport.h"
#include "maxheap.h"
#include "maxtypes.h"
// forward declarations
class ILoad;
class ISave;

// An individual app data chunk
/*! \brief An application/plugin specific custom data that can be attached to animatables.
Instances of this class store custom data defined by plugins and are attached to
animatables when plugins call Animatable::AddAppDataChunk. Plugins don't need to
instanciate directly objects of type AppDataChunk, but can get to the instances
stored on animatables by calling Animatable::GetAppDataChunk.
*/
class AppDataChunk: public MaxHeapOperators {
	public:
		// Note that data pointer should be allocated with standard malloc
		// since it will be freed in the destructor.
		//! \brief  Constructor
		/*! The data members are initialized to the values passed. 
		\param cid - The Class_ID of the owner of the custom data.
		\param sid - The SuperClassID of the owner of the custom data.
		\param sbid - An extra ID that lets the owner identify its sub-custom-data (chunks).
		\param len - The length of the data in bytes.
		\param data - Pointer to the actual data. The data should be allocated 
		on the heap by client code using MAX_malloc(). This will allow the system to 
		free it safely (using MAX_free()). MAX_malloc() and MAX_free() are memory 
		management routines implemented by the system. 
		\note Important: Not allocating on the heap the data passed to this method may 
		lead to unexpected behaviour of the application. 
		\note Important: Client code does not need to	free the data that has been passed 
		to this method. The system will free it when the Animatable is deleted or when
		client code explicitely removes the custom data chunk from the Animatable by
		calling Animatable::RemoveAppDataChunk.		*/
		CoreExport AppDataChunk(Class_ID cid, SClass_ID sid, DWORD sbid, DWORD len, void *data);
		/*! Constructor
		The length of the data buffer is set to 0 and the data pointer set to NULL.
		*/
		CoreExport AppDataChunk();

		//! \brief  Destructor
		/*! The data buffer is freed using MAX_free(). This implies that plug-ins must 
		use MAX_malloc() to allocate it.
		*/
		CoreExport ~AppDataChunk();

		//! The Class_ID of the owner of this chunk
		Class_ID  classID;
		//! The SuperClassID of the owner of this chunk.
		SClass_ID superClassID;
		//! An extra ID that lets the owner identify its sub chunks.
		DWORD subID;
		//! The length of the data in bytes.
		DWORD length;
		//! The chunk data itself
		void *data;

		//! \brief  Loads this AppDataChunk
		/*! The system implements this method to load the AppDataChunk from disk.
		*/
		CoreExport IOResult Load(ILoad *iload);
		//! \brief  Saves this AppDataChunk
		/*! The system implements this method to save the AppDataChunk to disk.
		*/
		CoreExport IOResult Save(ISave *isave);

	private: 
		// Disable accidental copies/assignments -- declared, not defined.
		AppDataChunk(const AppDataChunk&);
		AppDataChunk& operator=(const AppDataChunk&);
};
