/**********************************************************************
*<
FILE: ioapi.h

DESCRIPTION:

CREATED BY: Dan Silva

HISTORY:

*>	Copyright (c) 1994, All Rights Reserved.
**********************************************************************/

#pragma once

#include "maxtypes.h"
#include "tab.h"
#include "baseinterface.h"
#include "strbasic.h"
#include "assetmanagement\AssetType.h"
#include "assetmanagement\AssetId.h"
#include "strclass.h"


class ILoad;
class INode;
class ReferenceMaker;
class ClassDesc;

/*! \sa  Class ILoad, <a href="ms-its:3dsmaxsdk.chm::/refs_root.html">References</a>.\n\n
\par Description:
This is the callback object used by <b>ILoad::RegisterPostLoadCallback()</b>.
The <b>proc()</b> method will be called when everything has been loaded and all
the references are in place. It is assumed that if the callback needs to be
deleted, the <b>proc()</b> method will do it.  */
#pragma warning(push)
#pragma warning(disable:4100)
class PostLoadCallback : public InterfaceServer {
public:
	/*! \remarks This method will be called when loading is complete.
	\param iload This class provides methods to load data from disk. */
	virtual void proc(ILoad *iload)=0;
	// Lower priorities are run first-- allowable values {0..10}: 0 is reserved for ParamBlock2PLCB & ParamBlockPLCB 
	/*! \remarks	This method determines the order that the various registered callbacks
	execute. This method is overridden, for example, by the ParmBlock2
	PostLoadCallbacks to return 1 so it can execute before the others.
	\return  The allowable return values are <b>0</b> to <b>10</b>, with 5
	being the default. <b>0</b> is reserved for ParamBlock2PLCB and
	ParamBlockPLCB..
	\par Default Implementation:
	<b>{ return 5; }</b> 
	\note Custom Attributes are added to their owner objects in a Priority 6 PostLoadCall. Object Custom Attributes and 
	object MetaData should be expected only in PostLoadCalls with a Priority of 7 or lower. Custom Attributes added in 
	PostLoadCalls with a higher Priority may be replaced by the Priority 6 PostLoadCall.*/	
	virtual int Priority() { return 5; }  
	/*! \remarks	This is a general purpose function that allows the API to be extended in the
	future. The 3ds Max development team can assign new <b>cmd</b> numbers and
	continue to add functionality to this class without having to 'break' the
	API.\n\n
	This is reserved for future use.
	\param cmd The command to execute.
	\param arg1 Optional argument 1 (defined uniquely for each <b>cmd</b>).
	\param arg2 Optional argument 2.
	\param arg3 Optional argument 3.
	\return  An integer return value (defined uniquely for each <b>cmd</b>).
	\par Default Implementation:
	<b>{ return 0; }</b> */
	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }
};
#pragma warning(pop)
// CCJ - 9.15.00
// This interface is used to remap node handles when a scene is merged.
/*! \sa  Class InterfaceServer,  \ref Reference_Messages, Class ExclList\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class facilitates taking care of node handles when merging and xref'ing
scenes.\n\n
When nodes are merged, their handles will be reassigned so that their handles
will not conflict with existing nodes in the scene. After the merge process has
completed, all merged objects will receive a reference notification;
\ref REFMSG_NODE_HANDLE_CHANGED. The PartID (see \ref partids) will be a pointer to a merge
manager interface that you can use to see if a specific handle was converted
and convert between the old and the new handle. <b>IMergeManager* pMergeManager
= (IMergeManager*)partID;</b> If a plug-in uses node handles in a persistent
manner it has to intercept this reference message and convert the previous
handles to the newly generated handles.  */
class IMergeManager : public InterfaceServer {
public:
	/*! \remarks This method allows you to obtain a new handle.
	\par Parameters:
	<b>ULONG oldHandle</b>\n\n
	The old handle you wish to obtain a new one for.
	\return  The new handle. */
	virtual ULONG	GetNewHandle(ULONG oldHandle) = 0;
	/*! \remarks This method allows you to check if a handle already exists.
	\par Parameters:
	<b>ULONG handle</b>\n\n
	The handle you wish to check for.
	\return  TRUE if the handle exists; otherwise FALSE. */
	virtual bool	HandleExist(ULONG handle) = 0;
};

//Return value for GetClassDescID if no class was found
#define NO_CLASSDESC_ID_RETURNED 0xffff


/*! \sa
<a href="ms-its:3dsmaxsdk.chm::/loading_and_saving.html">Loading and Saving</a>,
<a href="ms-its:3dsmaxsdk.chm::/ui_character_strings.html">Character Strings</a>, 
Class ILoad, Class ClassDesc, Class ClassDirectory, Class Interface.\n\n
\par Description:
This class provides methods to save data to disk.\n\n
Note: It is not valid, to write two <b>CStr</b>s in the same chunk of a 3ds Max
file, since <b>ILoad::ReadCStringChunk()</b> sets the size for the string to
the <b>ChunkSize</b>. However it is possible to write other data, such as two
ints, into the same chunk.\n\n
Note about member alignment: Please make sure that when you save data from your
plugin you save individual data members using a chunk ID instead of saving the
image of a class. Saving (and loading) a class image puts you at risk of
running into member alignment problems and as such could potentially corrupt
saved files. File IO would be put further at risk when you keep Intel's IA-64
architecture in mind which depends on member alignment. What you should not do
is outlined in the following example when loading a class image;
<b>iload-\>Read(\&myclass, sizeof(MyClass), \&ab);</b>\n\n
Once you change the class in such a way that it affects the data size you run
the risk of having to support different versions, file IO incompatibility, and
member alignment issues.\n\n
The following global function is not part of this class but is available for
use: */
#pragma warning(push)
#pragma warning(disable:4100)
class ISave : public InterfaceServer {
public:
	virtual ~ISave(){};

	// Returns the index of the referenced object in the Scene stream.
	/*! \remarks This method is not normally used because the reference
	hierarchy is saved automatically. In certain cases however this method
	is quite useful. This method is used in saving a pointer to some object
	(or a table of pointers). This is a pointer to one of the objects that
	the scene saves with the reference hierarchy, but it is not a pointer
	that itself is a reference.
	\param ptarg The pointer to save.
	\return  The id that may be saved to disk. */
	virtual int GetRefID(void *ptarg)=0;

	// Begin a chunk.
	/*! \remarks This method is used to begin a chunk. The ID passed need
	only be unique within the plug-ins data itself.
	\param id The id for the chunk. */
	virtual void BeginChunk(USHORT id)=0;

	// End a chunk, and back-patch the length.
	/*! \remarks This method is used to end a chunk, and back-patch the
	length. */
	virtual void EndChunk()=0;

	/*! \remarks This method is used internally for checking balanced
	BeginChunk/EndChunk. */
	virtual	int CurChunkDepth()=0;  // for checking balanced BeginChunk/EndChunk

	// write a block of bytes to the output stream.
	/*! \remarks This method writes a block of bytes to the output stream.
	\param buf The buffer to write.
	\param nbytes The number of bytes to write.
	\param nwrit The number of bytes actually written.
	\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. Note that the
	plug-in should not put up a message box if a write error occurs. It
	should simply return the error status. This prevents a overabundance of
	messages from appearing. */
	virtual IOResult Write(const void  *buf, ULONG nbytes, ULONG *nwrit)=0;

	// Write character strings
	/*! \remarks This method is used to write wide character strings.
	\param str The string to write.
	\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. */
	virtual IOResult WriteWString(const char *str)=0;
	/*! \remarks This method is used to write wide character strings.
	\param str The string to write.
	\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. */
	virtual IOResult WriteWString(const mwchar_t *str)=0;
	/*! \remarks This method is used to write single byte character
	strings.
	\param str The string to write.
	\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. */
	virtual IOResult WriteCString(const char *str)=0;
	/*! \remarks This method is used to write single byte character strings.
	\param str The string to write.
	\return  <b>IO_OK</b> - The write was acceptable - no errors.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. */
	virtual IOResult WriteCString(const mwchar_t *str)=0;

	// are we saveing a MAX file or a MAT lib
	/*! \remarks Determines if we are saving a standard 3ds Max file
	(<b>.MAX</b>) or a material library (<b>.MAT</b>).
	\return  One of the following values:\n\n
	<b>IOTYPE_MAX</b>\n\n
	<b>IOTYPE_MATLIB</b> */
	virtual FileIOType DoingWhat()=0;

	// Test this for saving old version.  
	// Returns 0 for the current version, 2000 for version 2.0.
	/*! \remarks This method returns a value to indicate the current version of the
	file being saved. It returns 0 for the current version, MAX_RELEASE_R12 for	3ds Max 2010, etc. 
	Call this method to determine the format in which to save the data.
	- Adding a new chunk (unknown to the previous version) should be fine. 
	Plug-ins should be written to skip over unkown chunks. 
	- Removing a chunk is not fine. Never delete a chunk id. 
	Plug-ins need to be prepared to save out the data corresponding to the old 
	chunk id when this method returns the version of a previous 3ds Max release.
	- Changing the contents of an existent chunk is not fine, since it can break file compatibility.
	Add a new chunk to store the modified data structure, and keep the code that loads the old chunk. 
	Be prepared to convert the old data to the new format if that makes sense.
	\see GetSavingVersion(), \ref 3dsMaxRelease
	*/
	virtual DWORD SavingVersion() { return  0; }

	// <JBW> get a load ref id for the given RefMaker's ClassDesc in the ClassDirectory stream
	/*! \remarks This method is available in release 3.0 and later
	only.\n\n
	This method returns a load reference ID for the given Reference Maker's
	ClassDesc object in the ClassDirectory stream.
	\param rm Points to the reference maker.
	\par Default Implementation:
	<b>{ return 0xffff; }</b> */
	virtual USHORT GetClassDescID(ReferenceMaker* rm) { return NO_CLASSDESC_ID_RETURNED; }

	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }

	// The name of the scene file being saved
	/*! \remarks Returns the name of the scene file being saved. */
	virtual const MCHAR* FileName() { return NULL; }

	// Save Asset Metadata.
	/*! \remarks This method is used by Asset Manager to save to the scene file the asset meta data.
	\param assetId The id for the asset. 
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred, otherwise <b>IO_OK</b>. */
	virtual IOResult SaveAssetMetaData(const MaxSDK::AssetManagement::AssetId& assetId)  { return IO_OK; }

	// Save Asset Metadata.
	/*! \remarks This method is used to tell when to store an asset as a guid rather than as the asset filename.
	\return  A return value of true indicates to store eassets as guids, otherwise as the asset filename. */
	virtual bool SaveAssetAsAssetIDGuid() { return false; }

};


#pragma warning(pop)
/*! \sa
<a href="ms-its:3dsmaxsdk.chm::/loading_and_saving.html">Loading and
Saving</a>,
<a href="ms-its:3dsmaxsdk.chm::/ui_character_strings.html">Character Strings</a>, 
Class ISave, Class PostLoadCallback, Class INode, Class ClassDesc, Class ClassDirectory, Class Interface.\n\n
\par Description:
This class provides methods to load data from disk and to register post load
callbacks.\n\n
Note: It is not valid, to write two <b>CStr</b>s in the same chunk of a 3ds Max
file, since <b>ILoad::ReadCStringChunk()</b> sets the size for the string to
the <b>ChunkSize</b>. However it is possible to write other data, such as two
ints, into the same chunk.\n\n
All methods of this class are implemented by the system.  */
#pragma warning(push)
#pragma warning(disable:4100)
class ILoad : public InterfaceServer {
public:
	enum BPFlag {
		FORCEMERGE = (1<<0)
	};

	virtual ~ILoad(){};

	// Returns the memory address of the ith object Scene stream.
	/*! \remarks This method may be used to load a pointer from disk. It
	returns the memory address of the specified object Scene stream. This
	may be NULL if the address is not available. See
	<b>RecordBackpatch()</b> above for a work around.
	\param imaker This is the index returned from <b>ISave::GetRefID()</b>. that was used
	to save the pointer. */
	virtual void* GetAddr(int imaker)=0;

	// If GetAddr() returns NULL, then call this to get the address
	// backpatched later, when it is known.  patchThis must point at
	// a valid pointer location. RecordBackpatch will patch the
	// address immediately if it is available.
	/*! \remarks This method may be used to load a pointer from disk. This
	is a pointer that was saved using <b>ISave::GetRefID()</b>. You pass
	the index returned from <b>GetRefID()</b> and a pointer to a pointer
	that will get set. This method will patch the address immediately if it
	is available, otherwise it will happen later when it is known. During
	the load process if you need to work with this information you'll have
	to use a post load callback since all the addresses are not updated
	immediately. See <b>RegisterPostLoadCallback()</b> above.
	\param imaker This is the index returned from <b>ISave::GetRefID()</b>.
	\param patchThis This is a pointer to the pointer you want patched.
	\param flags  This flag indicates that backpatches (and their subsequent references)
	should be merged as well. */
	virtual void RecordBackpatch(int imaker, void** patchThis, DWORD flags = 0)=0;

	// When the root of a reference hierarchy is loaded, its
	// Load() can call this to store away a pointer to itself
	// for later retrieval.
	/*! \remarks This method is used internally. */
	virtual void SetRootAddr(void *addr)=0;
	/*! \remarks This method is used internally. */
	virtual void* GetRootAddr()=0;

	// if OpenChunk returns IO_OK, use following 3 function to get the 
	// info about the chunk. IO_END indicates no more chunks at this level
	/*! \remarks This method is used to open a chunk. If
	<b>OpenChunk()</b> returns <b>IO_OK</b>, use the following 3 functions
	to get the info about the chunk. If it returns <b>IO_END</b> this
	indicates there are no more chunks at this level.
	\return  <b>IO_OK</b> - The result was acceptable - no errors.\n\n
	<b>IO_END</b> - This is returned from <b>ILoad::OpenChunk()</b> when
	the end of the chunks at a certain level have been reached. It is used
	as a signal to terminates the processing of chunks at that level.\n\n
	<b>IO_ERROR</b> - This is returned if an error occurred. Note that the
	plug-in should not put up a message box if a read error occurred. It
	should simply return the error status. This prevents a overabundance of
	messages from appearing. */
	virtual IOResult OpenChunk()=0;

	// These give info about the most recently opened chunk
	/*! \remarks This method returns the ID of the most recently opened
	chunk. */
	virtual USHORT CurChunkID()=0;
	/*! \remarks This method returns the type of the most recently opened
	chunk. This may be one of the following values:\n\n
	<b>NEW_CHUNK</b>\n\n
	<b>CONTAINER_CHUNK</b>\n\n
	<b>DATA_CHUNK</b> */
	virtual ChunkType CurChunkType()=0;
	/*! \remarks This method returns the chunk length NOT including the header. */
	virtual	ULONGLONG CurChunkLength()=0;  // chunk length NOT including header
	/*! \remarks This method is used internally for checking for balanced
	OpenChunk/CloseChunk pairs. */
	virtual	int CurChunkDepth()=0;  // for checking balanced OpenChunk/CloseChunk

	// close the currently opened chunk, and position at the next chunk
	//  return of IO_ERROR indicates there is no open chunk to close
	/*! \remarks This method is used to close the currently opened chunk,
	and position at the next chunk.
	\return  A return value of <b>IO_ERROR</b> indicates there is no open
	chunk to close; otherwise <b>IO_OK</b>. */
	virtual IOResult CloseChunk()=0;

	// Look at the next chunk ID without opening it.
	// returns 0 if no more chunks
	/*! \remarks This method returns the ID of the next chunk without
	opening it. It returns 0 if there are no more chunks. */
	virtual	USHORT PeekNextChunkID()=0;

	// Read a block of bytes from the output stream.
	/*! \remarks This method is used to read a block of bytes from the
	output stream.
	\param buf A pointer to the buffer to read.
	\param nbytes The number of bytes to read.
	\param nread The number of bytes that were read.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
	otherwise <b>IO_OK</b>. */
	virtual IOResult Read(void  *buf, ULONG nbytes, ULONG *nread )=0;

	// Read a string from a string chunk assumes chunk is already open, 
	// it will NOT close the chunk. Sets buf to point
	// to a char string.  Don't delete buf: ILoad will take care of it.

	//   Read a string that was stored as Wide chars. 
	/*! \remarks This method read a string that was stored as Wide
	characters. Note: This method reads a string from a string chunk. It is
	assumed the chunk is already open, it will NOT close the chunk.
	\param buf A pointer to an array of characters.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
	otherwise <b>IO_OK</b>. */
	virtual IOResult ReadWStringChunk(char** buf)=0;
	/*! \remarks This method read a string that was stored as Wide chars. Note:
	This method reads a string from a string chunk. It is assumed the chunk is
	already open, it will NOT close the chunk.
	\param buf A pointer to an array of wide characters.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
	otherwise <b>IO_OK</b>. */
	virtual IOResult ReadWStringChunk(mwchar_t** buf)=0;

	//   Read a string that was stored as single byte chars
	/*! \remarks This method reads a string that was stored as single byte
	characters.
	\param buf A pointer to an array of single byte characters. This method will
	allocate an internal buffer, stored in the ILoadImp class that is big
	enough to hold the string chunk read in. You must then copy or parse
	out the data and store it in your own area: you can't hang on to the
	string pointer it hands back because it will not be valid.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
	otherwise <b>IO_OK</b>. */
	virtual IOResult ReadCStringChunk(char** buf)=0;
	/*! \remarks This method may be used to read a string that was stored as
	single byte characters.
	\param buf A pointer to an array of wide characters. This method will allocate an internal
	buffer, stored in the ILoadImp class that is big enough to hold the string
	chunk read in. You must then copy or parse out the data and store it in your
	own area: you can't hang on to the string pointer it hands back because it will
	not be valid.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred,
	otherwise <b>IO_OK</b>. */
	virtual IOResult ReadCStringChunk(mwchar_t** buf)=0;

	// Call this if you encounter obsolete data to cause a
	//  message to be displayed after loading.
	/*! \remarks You may call this if you encounter obsolete data to cause
	a message to be displayed after loading. */
	virtual	void SetObsolete()=0;		

	// Register procedure to be called after loading. These will
	// be called in the order that they are registered.
	// It is assumed that if the callback needs to be deleted,
	// the proc will do it.
	/*! \remarks Registers a procedure to be called after loading. These
	will be called in the order that they are registered. It is assumed
	that if the callback needs to be deleted, the proc will do it.
	\param cb Points to the callback object. */
	virtual void RegisterPostLoadCallback(PostLoadCallback *cb)=0;

	// Gets the various directories. Constants are defined in
	// JAGAPI.H
	/*! \remarks Retrieves the specified standard 3ds Max directory name (fonts,
	scenes, images, ...).
	\param which Specifies the directory name to retrieve. See \ref MAXDirIDs. The
	constants are defined in <b>MAXAPI.H</b>
	\return  The name of the specified directory. */
	virtual const MCHAR *GetDir(int which)=0;

	// are we Loading a MAX file or a MAT lib
	/*! \remarks Determines if we are loading a standard 3ds Max file
	(<b>.MAX</b>) or a material library (<b>.MAT</b>).
	\return  One of the following values:\n\n
	<b>IOTYPE_MAX</b>\n\n
	<b>IOTYPE_MATLIB</b> */
	virtual FileIOType DoingWhat()=0;

	// Root node to attach to when loading node with no parent
	/*! \remarks Returns the root node to attach to when loading a node
	with no parent. */
	virtual INode *RootNode()=0;

	// <JBW> get the ClassDesc corresponding to the given refID in the ClassDirectory stream
	// companion to ISave::GetClassDescID() for saving & loading ClassDesc references
	/*! \remarks This method is available in release 3.0 and later
	only.\n\n
	Returns a pointer to the ClassDesc corresponding to the specified
	reference ID in the ClassDirectory stream.
	\param refID The reference ID in the ClassDirectory stream.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
	virtual ClassDesc* GetClassDesc(USHORT refID) { return NULL; }

	virtual INT_PTR Execute(int cmd, ULONG_PTR arg1=0, ULONG_PTR arg2=0, ULONG_PTR arg3=0) { return 0; }

	//! \brief Retrieves the version and build number of 3ds Max used to save the file being loaded.
	/*! Plugins can call this method when loading a max file is being loaded. 
	\return A value describing the version of 3ds Max used to save the file. 
	The value is composed of a <b>LOWORD(value)</b>= Build number, and a 
	<b>HIWORD(value)</b> = <b>MAX_RELEASE</b>. 
	For example, 41 for build #41, and 4000 for 3ds Max version 4.0 
	\note Plugins should not rely on the value returned by this function for 
	versioning their own data saved to the max file. Plugins should rather use either:
	- Parameter block versioning for versioning data stored in parameter blocks. 
	See IParamBlock2::GetVersion() and the P_VERSION flag for versioning instances 
	of class ParamBlockDesc2.
	- Their own versioning scheme for data that is not stored in a parameter block.
	Loading and saving a max file in the absence of the plugin will maintain its data 
	intact, but the max version and build numbers will be updated, so next time the 
	file is loaded in the presence of the plugin, the max version number could be 
	different than when the plugin data was saved.
	*/
	virtual DWORD GetFileSaveVersion() { return 0; };

	/*! Records information needed to remap a reference to a node.
	References to nodes held by object-xrefed reference makers need to be 
	remapped when updating xref records. In most cases, Max will automatically 
	record data required to remap node references when xrefing objects. 
	Plug-ins that save node pointers into the max file need to be modified to
	indirectly reference the node, in order for that plug-in to be xref-able. 
	For the legacy version of the plugin to be xref-able, the plugin must 
	record in a PostLoadCallback information needed for remapping the 
	indirect reference it makes to nodes. This can be done by calling this method.
	\param aRefMaker - A scene entity that makes a reference to a node
	\param aRefIdx - The index of the reference aRefMaker makes to a node 
	\param aTargetNode - The node being referenced
	\param aIsIndirectRef - If true, the reference to the node is indirect, otherwise
	it's a regular reference
	\return - true if a node-ref-remap entry was recorded successfully, false otherwise
	*/
	virtual bool RecordNodeRefRemap(ReferenceMaker& aRefMaker, int aRefIdx, INode* aTargetNode, bool aIsIndirectRef) {return false;}

	// The name of the scene file being loaded
	/*! \remarks Returns the name of the scene file being loaded. */
	virtual const MCHAR* FileName() { return NULL; }

	// Load Asset Metadata.
	/*! \remarks This method is used by Asset Manager to load from the scene file all asset meta data.
	\return  A return value of <b>IO_ERROR</b> indicates an error occurred, otherwise <b>IO_OK</b>. */
	virtual IOResult LoadAssetMetaData()  { return IO_OK; }
};
#pragma warning(pop)


