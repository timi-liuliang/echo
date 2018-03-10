 /**********************************************************************
 
	FILE: IDataChannel.h

	DESCRIPTION:  Intelligent Data Channel API

	CREATED BY: Attila Szabo, Discreet

	HISTORY: [attilas|19.6.2000]


 *>	Copyright (c) 1998-2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "maxtypes.h"
#include "baseinterface.h"

// forward declarations
class FPInterface;
class BitArray;

// Macros to be used if any of the method below are implemented in a dll
// The dll project that implements the methods needs to define DATACHANNEL_IMP
#ifdef DATACHANNEL_IMP
#define DataChanExport __declspec(dllexport)
#else
#define DataChanExport __declspec(dllimport)
#endif



// A data channel is a homogeneous collection of objects of a user defined 
// type (data objects). Data channels are uniquely identified by a Class_ID.
//
// Data channels can be associated with any element type of a Max object:
// faces or vertexes of Meshes, etc.
/*! \sa  Class InterfaceServer, Class Class_ID, Class IFaceDataChannel, Class IFaceDataChannelsEnumCallBack\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
A data channel is a homogeneous collection of objects of a user defined type
(data objects). Data channels are uniquely identified by a Class_ID. Data
channels can be associated with any element type of a 3ds Max object: faces or
vertexes of Meshes, etc. You can use the macro
<b>GetDataChannelInterface(obj)</b> to obtain a pointer to this interface. <br>
*/
class IDataChannel : public InterfaceServer
{
	public:
		// Returns the unique id of the channel
		/*! \remarks This method returns the unique class ID of the channel.
		*/
		virtual Class_ID DataChannelID() const =0;

		// Returns the number of data objects in this channel
		/*! \remarks This method returns the number of data objects in this
		channel.
		\par Default Implementation:
		<b>{ return 0; }</b> */
		virtual ULONG Count() const { return 0; }
		
		// Self destruction
		/*! \remarks Destructor. Deletes self. */
		virtual void DeleteThis() = 0;
};

// interface ID
#define DATACHANNEL_INTERFACE Interface_ID(0x38a718a8, 0x14685b4b)
#define GetDataChannelInterface(obj) ((IDataChannel*)obj->GetInterface(DATACHANNEL_INTERFACE)) 

// Face-data channel interface
// 
// This is an abstraction of a collection of data objects that is 
// associated with faces of Max objects
// Max objects that have face-data channels call the methods of this interface
// when those faces change in some way. The data channels can then react to
// the changes to the faces.
//
// Currently only Meshes support face-data channels.
/*! \sa  Class IDataChannel, Class IFaceDataChannelsEnumCallBack ,  Class IFaceDataMgr\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the face-data channel interface and as such is an
abstraction of a collection of data objects that is associated with faces of
3ds Max objects. 3ds Max objects that have face-data channels call the methods
of this interface when those faces change in some way. The data channels can
then react to the changes to the faces. You can use the macro
<b>GetFaceDataChannelInterface(obj)</b> to obtain a pointer to this
interface.\n\n
Currently in version 4.0 only Meshes support face-data channels.  */
class IFaceDataChannel : public IDataChannel
{
	public:

		//
		// --- face specific operations\events ---
		//
		// These methods are called by the owner of face-data channels
		// when its faces change in some way. It's up to the face-data channel 
		// to do wathever it wants to do on these notification methods.

		// Called when num new faces were created at index at in the
		// object's list of faces. Returns TRUE on success
		// ULONG at - index in the object's array of faces where the new faces 
		// were inserted
		// ULONG num - the number of new faces created
		/*! \remarks This method is called when <b>num</b> new faces are
		created at the index <b>at</b> in the object's list of faces.
		\par Parameters:
		<b>ULONG at</b>\n\n
		The index in the object's array of faces where the new faces are
		inserted.\n\n
		<b>ULONG num</b>\n\n
		The number of new faces which are created.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FacesCreated( ULONG at, ULONG num ) = 0;
		
		// Called when the owner object has cloned some of its faces and appended
		// then to its list of faces.
		// BitArray& set - bitarray with as many bits as many faces the owner 
		// object has. Bits set to 1 correspond to cloned faces
		/*! \remarks This method is called when the owner object has cloned
		some of its faces and appended these to its list of faces. The bits in
		the set array correspond to the cloned faces.
		\par Parameters:
		<b>BitArray\& set</b>\n\n
		The array of bits. Note that this array has as many bits as there are
		faces in the owner object.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FacesClonedAndAppended( BitArray& set ) = 0;

		// Called when faces were deleted in the owner object. Returns TRUE on success
		// BitArray& set - bitarray with as many bits as many faces the owner 
		// object has. Bits set to 1 correspond to deleted faces
		/*! \remarks This method is called when faces were deleted in the
		owner object. The bits in the set array correspond to the deleted
		faces.
		\par Parameters:
		<b>BitArray\& set</b>\n\n
		The array of bits. Note that this array has as many bits as there are
		faces in the owner object.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FacesDeleted( BitArray& set ) = 0;

		// Called when faces were deleted in the owner object. Returns TRUE on success
		// Allwos for a more efficient deletion of a range of data objects
		// than using a BitArray
		// ULONG from - index in the object's array of faces. Faces starting 
		// from this index were deleted
		// ULONG num - number of faces that were deleted
		/*! \remarks This method is called when faces were deleted in the
		owner object. This method allows for a more efficient deletion of a
		range of data objects than the previous BitArray based one.
		\par Parameters:
		<b>ULONG from</b>\n\n
		The index in the object's array of faces. Faces starting at this index
		were deleted.\n\n
		<b>ULONG num</b>\n\n
		The number of faces that were deleted.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FacesDeleted( ULONG from, ULONG num ) = 0;
		
		// Called when all faces in the owner object are deleted
		/*! \remarks This method is called when all faces in the owner object
		are deleted */
		virtual void AllFacesDeleted() = 0;

		// Called when a face has been copied from index from in the owner object's
		// array of faces to the face at index to.
		// ULONG from - index of source face
		// ULONG to - index of dest face
		/*! \remarks This method is called when a face has been copied from
		index from in the owner object's array of faces to the face at index
		<b>to</b>.
		\par Parameters:
		<b>ULONG from</b>\n\n
		The index of the source face.\n\n
		<b>ULONG to</b>\n\n
		The index of the destination face.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FaceCopied( ULONG from, ULONG to ) = 0;

		// Called when a new face has been created in the owner object based on
		// data interpolated from other faces
		// ULONG numSrc - the number of faces used in the interpolation
		// ULONG* srcFaces - array of numSrc face indeces in the owner object's 
		// face array. These faces were used when creating the new face
		// float* coeff - array of numSrc coefficients used in the interpolation
		// ULONG targetFace - the index in the owner object's array of faces of the 
		// newly created face
		/*! \remarks This method is called when a new face has been created in
		the owner object based on data interpolated from other faces.
		\par Parameters:
		<b>ULONG numSrc</b>\n\n
		The number of faces used in the interpolation.\n\n
		<b>ULONG* srcFaces</b>\n\n
		The array of <b>numSrc</b> face indices into the owner object's face
		array. These faces were used when creating the new face.\n\n
		<b>float* coeff</b>\n\n
		The array of <b>numSrc</b> coefficients used in the interpolation.\n\n
		<b>ULONG targetFac</b>\n\n
		The index in the owner object's array of faces of the newly created
		face.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL FaceInterpolated( ULONG numSrc, 
															ULONG* srcFaces, 
															float* coeff, 
															ULONG targetFace ) = 0;

		//
		// --- geometry pipeline (stack) specific methods ---
		//
		// These methods are called when the owner object is flowing up the 
		// pipeline (stack). They must be implemented to ensure that the 
		// face-data channel flows up the pipeline correctly.
		// The owner object expects the face-data to do exactly what the 
		// names of these methods imply. These can be seen as commands that are
		// given by the owner object to the face-data channel

		// Allocates an empty data-channel 
		/*! \remarks This method will allocate an empty data-channel. */
		virtual IFaceDataChannel* CreateChannel( ) = 0;

		// The data-channel needs to allocate a new instance of itself and fill it 
		// with copies of all data items it stores.
		// This method exist to make it more efficient to clone the whole data-channel
		/*! \remarks The data-channel needs to allocate a new instance of
		itself and fill it with copies of all data items it stores. Note: This
		method makes it more efficient to clone the whole data-channel. */
		virtual IFaceDataChannel* CloneChannel( ) = 0;

		// The data-channel needs to append the data objects in the fromChan
		// to itself. 
		/*! \remarks The data-channel needs to append the data objects in the
		<b>fromChan</b> to itself.
		\par Parameters:
		<b>const IFaceDataChannel* fromChan</b>\n\n
		The channel containing the data objects to append.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL AppendChannel( const IFaceDataChannel* fromChan ) = 0;
};

// interface ID
#define FACEDATACHANNEL_INTERFACE Interface_ID(0x181358d5, 0x3cab1bc9)
#define GetFaceDataChannelInterface(obj) ((IFaceDataChannel*)obj->GetInterface(FACEDATACHANNEL_INTERFACE)) 

// Interface class that allows to execute a callback method (Proc) for all 
// face-data channels of an object.
//
// Developers should derive their own classes from this interface and 
// overwrite the Proc method to call the desired IFaceDataChannel method
// It is up to derived class to interpret the context parameter passed to
// Proc.
//
// --- Usage ---
// Classes that hold face-data channels, can implement a method called
// EnumFaceDataChannels( IFaceDataEnumCallBack& cb, void* pContext)
// This method would be called with a reference to an instance of a class
// derived from IFaceDataEnumCallBack in which Proc was overwritten. The 
// implementation of EnumFaceDataChannels would call cb.Proc for each of 
// the face-data channels of the object
//
// Warning:
// Deleting data channels from within Proc can lead to unexpected behaviour
/*! \sa  Class IDataChannel, Class IFaceDataChannel , Class IFaceDataMgr\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This is an interface class that will allow a callback procedure to execute for
all face data channels of an object. You should derive your own classes from
this interface and overwrite the <b>Proc()</b> method to call the desired
<b>IFaceDataChannel</b> method. It is up to the derived class to interpret the
context parameter passed to <b>Proc()</b>.\n\n
Classes that hold face data channels can implement the method:\n\n
<b>EnumFaceDataChannels(IFaceDataEnumCallBack\& cb, void* pContext)</b>\n\n
This method would be called with a reference to an instance of a class derived
from <b>IFaceDataEnumCallBack</b> in which <b>Proc()</b> is overwritten. The
implementation of <b>EnumFaceDataChannels</b> would call <b>cb.Proc</b> for
each of the face-data channels of the object\n\n
Note: Do not delete data channels from within the Proc(). This could lead to
unexpected behaviour.  */
class IFaceDataChannelsEnumCallBack: public MaxHeapOperators
{
	public:
		/*! \remarks Destructor */
		virtual ~IFaceDataChannelsEnumCallBack() {;}
		/*! \remarks The callback method that should be overridden.
		\par Parameters:
		<b>IFaceDataChannel* pChan</b>\n\n
		A pointer to the face-data channel interface.\n\n
		<b>void* pContext</b>\n\n
		A pointer to the context data.
		\return  TRUE if successful, otherwise FALSE. */
		virtual BOOL Proc( IFaceDataChannel* pChan, void* pContext ) = 0;
};




