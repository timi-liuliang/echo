/**********************************************************************
*<
FILE: HWVertex.h

DESCRIPTION: Hardware Vertex Interface Implementation

CREATED BY: Peter Watje

HISTORY: Created June 13, 2008

Copyright 2008 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   

**********************************************************************/


#pragma once
#include "export.h"
#include "maxheap.h" 
#include "tab.h" 

class Point3;

namespace GFX_MESH
{
	/*! \brief This is the maximum number of supported vertex color channels */
	static const unsigned int kMAX_COLORS = 2;
	/*! \brief This is the maximum number of supported uvw mapping channels */
	static const unsigned int kMAX_TEXTURES = 8;

/*! \brief This enums list all our possible vertex components for our vertex vbuffer*/
enum VertexType { 
	kPos = 1,												//!< Position of the vertex
	kNormal = 2 ,											//!< Normal of the vertex
	kVC0  = 4 ,kVC1 = 8 ,									//!< Vertex Color 0-1
	kUVW0 = 16 ,kUVW1 = 32 ,kUVW2 = 64, kUVW3 = 128,		//!< UVW channels 0-7
	kUVW4 = 256, kUVW5 = 512, kUVW6 = 1024, kUVW7 = 2048,
	kBinormal = 4096, kTangent = 8192						//!< Binormal and tangent channels To Be Implemented
};

//! \brief This class is used to help with managing hardware(hw) vertex buffer.
/*! It is not designed to be used as representation of an entire vertex buffer but
used to map into an existing buffer.  */
class HWVertex : public  MaxHeapOperators
{
public:

	//! \brief Constructor
	/*! 
	\param[in]  vType the description of the HW vertex which can be any combination of VertexType
	*/
	DllExport HWVertex(DWORD vType);
	DllExport HWVertex();
	DllExport ~HWVertex();


	//! \brief Lets you set the vertex type ie whether it is just a pos, pos+normal, pos+normal+UVW0 etc.
	/*! \param[in]  vType the description of the HW vertex which can be any combination of VertexType	*/
	DllExport void SetVertexType(DWORD vType);
	//! \brief Lets you get the vertex type
	DllExport DWORD GetVertexType();


	//! \brief This sets the size of each UVW channel.
	/*! By default a UVW channel is 2 floats just UV.
	You can change the size to just a U, UV, or UVW with this.  This must be set before
	accessing any of the vertex data or calling SetBuffer
	\param[in] 	id which channel
	\param[in]  size the size of the channel 1 to 3	*/
	DllExport void	SetUVWSize(unsigned int id, unsigned int size);

	//! \brief This gets the size of a specific UVW channel.
	/*! By default a UVW channel is 2 floats just UV.
	You can change the size to just a U, UV, or UVW with this.  
	\param[in] 	id which channel	*/
	DllExport unsigned int	GetUVWSize(unsigned int id);

	//! \brief This lets you map the hw vertex to a buffer.
	/*! Note you need to call SetUVWSize first
	before calling this if you want to change the UVW channel sizes
	\param[in] 	vertexBuffer the buffer that you want to map onto	*/
	DllExport void	SetBuffer(BYTE *vertexBuffer);
	
	/*! \brief Returns the stride of the hw vertex in bytes	*/
	DllExport unsigned int GetStride();

	/*! \brief Gets the Position component if there is one 	*/
	DllExport  Point3 GetPos();
	/*! \brief Sets the Position component if there is one 
	\param[in] 	p the new position value	*/
	DllExport  void SetPos(Point3 p);
	/*! \brief Returns the pointer to position portion of the vertex	*/
	DllExport  Point3* GetPosPtr();

	/*! \brief Gets the Normal component if there is one */
	DllExport  Point3 GetNormal();
	/*! \brief Sets the Normal component if there is one
	\param[in] 	p the new normal value	*/
	DllExport  void SetNormal(Point3 p);
	/*! \brief Returns the pointer to normal portion of the vertex	*/
	DllExport  Point3* GetNormalPtr();

	/*! \brief Returns the maximum number of supported color channel */
	DllExport unsigned int GetMaxVC();

	/*! \brief  gets the color component if there is one (RBGA byte format)
	\param[in]   id is which channel to get set */
	DllExport DWORD GetVC(unsigned int id);
	/*! \brief  sets the color component if there is one (RBGA byte format)
	\param[in]   id is which channel to get set 
	\param[in]   c is the color in RBGA byte format */
	DllExport void SetVC(unsigned int id, DWORD c);
	/*! \brief Returns the pointer to vertex color portion of the vertex	*/
	DllExport DWORD* GetVCPtr(unsigned int id);

	/*! \brief Returns the maximum number of supported UVW/texture channel	*/
	DllExport unsigned int GetMaxUVW();

	/*! \brief Gets sets the UVW component if there is one returning *float to the start of the uwv coord.
	\param[in]  id is which channel to get set	*/
	DllExport  float* GetUVW(unsigned int id);
	/*! \brief Sets sets the UVW component if there is one returning *float to the start of the uwv coord.
	\param[in]  id is which channel to get set	
	\param[in]  uv the uv data to set		*/
	DllExport  void   SetUVW(unsigned int id, float *uv);

	/*! \brief gets the Tangent component if there is one */
	DllExport  Point3 GetTangent();
	/*! \brief gets the pointer Tangent component if there is one */
	DllExport  Point3* GetTangentPtr();
	/*! \brief sets the Tangent component if there is one 
	\param[in]  p the tangent value	*/
	DllExport  void SetTangent(Point3 p);

	/*! \brief gets the BiNormal component if there is one */
	DllExport  Point3 GetBinormal();
	/*! \brief gets the pointer BiNormal component if there is one */
	DllExport  Point3* GetBinormalPtr();
	/*! \brief sets the BiNormal component if there is one 
	\param[in]  p the BiNormal value	*/
	DllExport  void SetBinormal(Point3 p);

	/*! \brief This lets you copy the vertex data from another vertex
	\param[in]  hwVertSource this is the vertex that you want to copy from	*/
	DllExport  void CopyDataFrom(HWVertex &hwVertSource);
	/*! \brief Returns a raw pointer the vertex buffer	*/
	DllExport BYTE* GetRawBufferPointer();

private:
	//This initializes all our data
	void Init();
	//this computes the stride of the vertex in bytes
	void	ComputeStride();
	//this computes our pointers offsets into vertex to allow quick look ups.
	//needs to be called after the stride has been computed
	void	ComputePointers();

	DWORD	mVertexType;	//Description of the HW Vertx
	BYTE*	mVertex;		//the pointer into the raw hw vertex buffer
	unsigned int mStride;	//the stride in byes of the hw vertex

	Point3		*mP;					//Pointer to our Position component
	Point3		*mN;					//Pointer to our Normal component
	DWORD		*mVC[kMAX_COLORS];	//Array of Pointers to our vertex color component

	unsigned int mUVWSizes[kMAX_TEXTURES];	//Array of sizes of each UVW channel
	float		*mUVW[kMAX_TEXTURES];		//Array of Pointers to our vertex UVW/texture component
	Point3		*mTangent;					//Pointer to our Tangent component
	Point3		*mBinormal;					//Pointer to our Binormal component

};

/*! \brief This is a class that lets you create a hw vertex buffer.  It allows you to define, create, edit and access the
buffer */
class HWVertexBuffer : public  MaxHeapOperators
{
public:
	/*! \brief Constructor
		\param[in]  vType the description of the HW vertex which can be any combination of VertexType	*/
	DllExport HWVertexBuffer(DWORD vType);
	/*! \brief  Destructor */
	DllExport ~HWVertexBuffer();

	/*! \brief  Frees the vertex buffer */
	DllExport void FreeBuffer();

	//! \brief This sets the size of each UVW channel.
	/*! By default a UVW channel is 2 point3 a UV.
	you can change the size to just a U, UV, or UVW with this.  This must be set before
	calling create buffer if you want to change the channel sizes
	\param[in]	id which channel
	\param[in]  size the size of the channel 1 to 3	  */
	DllExport void SetUVWSize(unsigned int id, unsigned int size);

	/*! \brief This lets you associate a HW texture channel with a max map channel 
	\param[in]	hwChannel  the hw channel
	\param[in]	maxMapChannel the Max channel	*/
	DllExport void SetMaxMapChannel(unsigned int hwChannel, int maxMapChannel);
	/*! \brief This lets get the map channel associated with a hw channel
	\param[in]	hwChannel  the hw channel	*/
	DllExport int GetMaxMapChannel(unsigned int hwChannel);

	//! \brief This allocates our buffer. Returns false if the buffer creation failed.
	/*! 
		\param[in]  numVertice is the number of hw vertices to create for this buffer	
		\return true is succeeded, false if failed typically an out of mememory error
	*/
	DllExport bool CreateBuffer(unsigned int numVertice);

	/*! \brief returns the stride in bytes of a hw vertex */
	DllExport unsigned int GetStride();
	/*! \brief returns the size in bytes of the entire buffer */
	DllExport inline unsigned int Size();

	/*! \brief returns the number of hw vertices in the buffer */
	DllExport inline unsigned int NumberVertices();

	//! \brief  this lets you set the size of the buffer,
	/*! \brief  this lets you set the size of the buffer, it returns true if everything was allocated correctly  
	/param unsigned int ct the number of vertices to set the buffer count to 
	*/
	DllExport bool SetNumberVertices(unsigned int ct);


	/*! \brief This returns a hwvertex of a specific vertex
	\param[in]  id is which vertex you want to look at */
	DllExport inline HWVertex GetHWVert(int id);


	/*! \brief  returns a raw pointer to the vertex buffer */
	DllExport BYTE *GetRawBufferPointer();



	/*! \brief  This lets you set whether the vertex buffer will also maintain a connection list back to the original geometry*/
	/*! \param[in] hasConnectionData whether the vertex buffer will also build connection info from the max mesh to the HW mesh*/
	DllExport void SetHasConnectionData(bool hasConnectionData);
	/*! \brief  This returns whether the vertex buffer has connection data back to the original geometry*/
	DllExport bool GetHasConnectionData();
	//! \brief  This lets you set the connection data
	/*! This lets you set the connection data.  It only tracks position and normals.
	\param[in]  i this is the index in the hw vertex buffer
	\param[in]  oldPosID  this is the position index in max mesh vertex list
	\param[in]  oldNormalID	this is the position index in max mesh gfxnormal list, set this value if you want to use the gfxfacenormal list.  This is used when the face has no smoothing group	*/
	DllExport void SetConnectionData(unsigned int i,unsigned int oldPosID, int oldNormalID);

	/*! \brief  This returns position mapping list */
	DllExport unsigned int *GetPositionConnectionList();
	/*! \brief  This returns normal mapping list */
	DllExport int *GetNormalConnectionList();

private:

	bool				mHasConnectionData;		//whethe the connection data is allocated also
	Tab<unsigned int>	mOriginalVertID;		// this is an index list back into the original Max mesh so you 
	Tab<int>	mOriginalGFXNormalID; // this is an index list back into the original Max mesh so you 
										//this is special cased to handle vertices attached to a face with no smoothing group
										// in this case the value of the normal will be negative.  If that is the case you lookup the 
										// the normal in face normal list

	DWORD			mVertexType;		//Description of the HW Vertx
	HWVertex		mHWVertex;			//our HW vertex overlay so we can easily look up components in the hw vertex buffer

	unsigned int	mNumberVertices;	//number of vertices in the hw vertex buffer
	unsigned int	mStride;			//stride of each vertex in bytes
	Tab<BYTE>		mVertexBuffer;		//raw hw vertex data
	int				mMaxMapChannel[kMAX_TEXTURES];		//this maps the HW UVW channel to a Max UVW channel
};

/*! \brief this is a class used to track mesh vertices to hw vertices. Each MeshVertes represents 
a max mesh vertex and has a list of indices in the HWVertex buffer which reference that mesh vertex*/
class MeshVertex : public  MaxHeapOperators
{
public:
	/*! \brief Constructor */
	DllExport MeshVertex();


	/*! \brief  clears out the mHWVertex list but does not deallocate it */
	DllExport inline void InitHWVert();

	/*! \brief  sets the size of mHWVertex, this is useful to preallocate blank space to reduce allocates for performance reasons*/
	DllExport inline void SetSize(unsigned int s);

	//! \brief This is used to check to see if the mesh vertex already references a matching hw vertex.  It returns true if so and returns that index in id. 
	/*! 
		
		\param hwVert  - this is the raw pointer to the HWVertex that we want to see if is in our list
		\param vertexBuffer - this is the raw pointer of all our current HWVerticess
		\param stride - this is the stride of each vertex 
		\param id this is the index that was found
	*/
	DllExport inline bool IsInList(BYTE *hwVert, BYTE *vertexBuffer, unsigned int stride, DWORD &id);
	
	//! \brief  This adds a new hw vert index to our mesh vertex.
	/*!  If IsInList returns false we need to add this vertex 
	\param id is the vertex to add
	*/
	DllExport inline void  AddHWVert(DWORD id);


private:
	//this is a list of indices into the HW vertex buffer that are needed to represent this mesh vertex
	Tab<DWORD>	mHWVertex;

};
}