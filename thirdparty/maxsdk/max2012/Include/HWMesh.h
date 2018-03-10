/**********************************************************************
*<
FILE: HWMesh.h

DESCRIPTION: Hardware Mesh Interface Implementation

CREATED BY: Peter Watje

HISTORY: Created June 13, 2008

Copyright 2008 Autodesk, Inc.  All rights reserved.

Use of this software is subject to the terms of the Autodesk license agreement 
provided at the time of installation or download, or which otherwise accompanies 
this software in either electronic or hard copy form.   


These classes are designed to allow the creation of a retained hardware mesh that the display system 
can quickly display.  With the old code you had an mesh/mnmesh which had a hardwaremesh interface.  
The mesh then made a copy of itself but a tuple version and that was stored in the hardwaremesh interface.
That interface was then passed the gfx layer which would then split the mesh based on size and material
id's into index and vertex buffers that the gfx layer could consume.

The new hwmesh pushes more of the work on the mesh end. This is to prevent the massive amount of
copying we had with the old scheme.  Instead of copy the entire mesh into the hardware mesh and then 
turning it into a tuple mesh.  The mesh passes a chunk to the hardware mesh.  This chunk is based on
MatID and vertex buffer count (typically less then a dword).  The hardware mesh then turns this chunk 
into a tuple mesh which is then passed to the gfx layer to have the hw vertex and index buffer created.
Once that is done the tuple mesh is deleted and the next chunk is passed in.

Mesh : HardwareMesh
			HWTupleMeshContainer - which is container that holds all our sub meshes.  Each entry is 1 matID and
								is typically has less than 65,500 vertex indices so we can use WORD indices to
								reduce memory foot print
				Tab<GFX_MESH::HWTupleMesh *> this is the list of sub meshes.  Each sub mesh contains
									1 vertex buffer 
									2 index buffer 1 for shaded faces and 1 for wireframe(lineLis)
									IHWDrawMesh which is a pointer back to the raw hardware buffers that the device created.

So to create a HWTupleMesh
	1.  The mesh first gets a list a faces that all have the same MatID and have less than the MaxVerts
	2.  That chunk is then converted into a tuple mesh and stored GFX_MESH::HWMesh
	3.  The GFX_MESH::HWMesh is then passed to the graphics window which tries a graphics window specific
		buffers.  If this is successfull the HWMesh tuple data is deleted.  If it fails the data is kept
		and when the data will be used to draw the mesh in immediate mode.  This can happen is we run out 
		of memory.
	4.  Repeat step one till you have no more faces to process

When the mesh is drawn it just calls draw on each GFX_MESH::HWMesh entry.  If the entry contains graphic
window specific buffers.  That is just immediately blasted out, if the hardware specific data is not there
the tuple data is still there and can be drawn in immediate mode.
					

**********************************************************************/
#pragma once

#include "maxheap.h" 
#include "tab.h" 
#include "HWVertex.h"
#include "HWIndex.h"
// forward declarations
class BitArray;
class GraphicsWindow;
class Point3; 

namespace GFX_MESH
{


	//! \brief This is a wrapper around our low level HW mesh.
	/*! This is a interface around our low level vertex buffer this will allow you to directly update the low level buffers 
	without recreating them, draw the buffers, and also give you access to release the buffers.
	This hides the actual vertex and index buffer away from the coder since it may be different
	depending on the device.  Each GFX_MESH::HWMesh has a pointer to one of these.
	It has a simple reference counter to know when to get deleted since these can be batched up to
	be drawn and could potentially get deleted before the actual draw.
	*/
	class IHWDrawMesh : public  MaxHeapOperators
	{
		
	public:
		//! \brief Constructor
		DllExport IHWDrawMesh();
		virtual ~IHWDrawMesh() { ; }
		//! \brief This draws the mesh to the display using the current render limits 
		/*!  
			\param gw the graphics window to draw the mesh to.
			\param attribute  if this mesh contains multiple matIDs this is which matID to draw. The old hwmesh stored multiple matIDs per mesh the new one only has one.
		*/
		virtual void Draw(GraphicsWindow *gw, DWORD attribute) = 0;

		//! \brief  Allows you direct access to the low level buffer to change values without recreating the mesh
		/*!  
		\param vertexList This is a pointer of all the vertices on the mesh.  The changed vertices should be in the list
		\param gfxNormalList This is a pointer of all the gfx normals on the mesh.  The changed vertices should be in the list
		\param faceNormalList This is a pointer of all the face normals on the mesh
		\param changedVerts This is a bitarray that tags which vertices are changed so we only update those vertices
		\param count This is number of vertices in vertex List
		\param vertMapList This is mapping that lets the HW vertex look up which vertex it belongs to
		\param normalsMapList This is mapping that lets the HW vertex look up which normal it belongs to.  If this 
								value is negative it is made positive and then face normallist is used to lookup the normal		
								This occurs when there is no smoothing group.
		*/
		virtual void UpdateVertexBuffer(Point3 *vertexList,Point3 *gfxNormalList, Point3 *faceNormalList, BitArray &changedVerts, int count,
										unsigned int *vertMapList,  int *normalsMapList) = 0;

		//! \brief This releases the buffers if the class is no longer referenced by anything.  You should not delete this class but call Release on it.
		virtual void Release() = 0;
		
		/*! \brief When ever you reference this pointer and want to hang onto it you need to call this
		*/
		DllExport void MakeRef();


	protected:		

		
		/*! \brief  You should not typically call this but should call release when you are done with the pointer\n\n
		*/
		DllExport void DecRefCount();

		
		/*!   \brief  returns whether the class can be deleted ie the ref count is zero
		*/
		DllExport bool CanDelete();

		int mRefCount;  //this is just a simple ref counter, when it is zero we can safely delete this class

	};

	//! \brief This is a wrapper class around our HW Mesh.  
	/*! Both our new and old HW mesh derive from this class and the pointer to this class is used to pass
		the HW mesh around.  The old code would just pass around a DWORD_PTR and then cast it to internal HW Mesh.	*/
	class IHWSubMesh : public  MaxHeapOperators
	{

	public:
		virtual ~IHWSubMesh() 	{ 	}
		
		//! \brief  Draws the mesh to the display using the current render limits 
		/*!
		\param	gw The diffuse color
		\param attribute If this mesh contains multiple matIDs this is which matID to draw. The old hwmesh stored multiple matIDs per mesh the new one only has one.
		*/
		virtual void Draw(GraphicsWindow *gw, DWORD attribute) = 0;

		//! \brief Allows you direct access to the low level buffer to change values without recreating the mesh
		/*!  		
		\param vertexList This is a pointer of all the vertices on the mesh
		\param gfxNormalList This is a pointer of all the gfx normals on the mesh
		\param faceNormalList This is a pointer of all the face normals on the mesh
		\param changedVerts This is a bitarray of changed verts
		\param count This is number of vertices in vertex List
		\param vertMapList This is mapping that lets the HW vertex look up which vertex it belongs to
		\param normalsMapList This is mapping that lets the HW vertex look up which normal it belongs to.  If this 
			value is negative it is made positive and then face normallist is used to lookup the normal		
			This occurs when there is no smoothing group.
		*/
		virtual void UpdateVertexBuffer(Point3 *vertexList,Point3 *gfxNormalList, Point3 *faceNormalList, BitArray &changedVerts, int count,
			unsigned int *vertMapList,  int *normalsMapList) = 0;

	

	};



	//! \brief This class is a container for vertex and index buffers.
	/*!  This class is a container for vertex and index buffers.  It represents mesh that has one material on it.
		It allows you to generate buffers and fill them out.  It has one vertex buffer and 2 index buffers, one for
		the shaded mesh and one for the wire frame.
	*/
	class HWTupleMesh : public  MaxHeapOperators
	{
	public:
		//! \brief Constructor
		/*! \param vertexType is the vertex type ie Pos|Normals|UVWs etc that this mesh will use.  See HWVertex.h, enum VertexType for
			the list of supported types
		*/
		DllExport HWTupleMesh(DWORD vertexType);

		//! \brief Destructor
		DllExport ~HWTupleMesh();

		//! \brief Returns the size of the buffers in bytes.		
		DllExport unsigned int Size();

		//! \brief This returns the hw vertex buffer for this hw mesh.
		/*!  Typical usage it to create the mesh. Get the hw vertex buffer, set it parameters, create it, then fill it out.
		*/
		DllExport HWVertexBuffer*	GetHWVertexBuffer();

		//! \brief This lets you add a index buffer to the mesh. 
		/*!  We support 2 index buffer per mesh so that different draw primtives can share the same vertex buffer.  The first is the shaded face buffer
		the second is the wire frame line list. It returns the id of this index buffer
		
		\param numberPrimitives is the number of primitives ie triangleList, lineList etc for this buffer
		\param primitive is the type of primitive it is ie trianglelist, linelist etc.
		\param use16Bit is whether the buffer buffer will be 16 bit or 32 bit indices		
		*/
		DllExport unsigned int AddHWIndexBuffer(unsigned int numberPrimitives, PrimitiveType primitive, bool use16Bit);

		//! \brief This returns a particular index buffer.		
		DllExport HWIndexBuffer*	GetHWIndexBuffer(unsigned int id);	

		//! \brief This lets you delete a particular index buffer.  It returns true if successful
		DllExport bool				ReleaseHWIndexBuffer(unsigned int id);	

		//! \brief This returns the number of index buffers
		DllExport unsigned int	    GetHWIndexCount();	

		//! \brief This lets you get the material id for this mesh.
		/*! This lets you associate the mesh with a material.  We only allow one material per mesh.	*/
		DllExport DWORD	GetMatID();	
		//! \brief This lets you set the material id for this mesh.
		/*!   This lets you get the associate material for this mesh.  We only allow one material per mesh.	*/
		DllExport void	SetMatID(DWORD matID);	

		//! \brief  This lets you set the IHWDrawMesh asssociated with this mesh.  
		/*!  The IHWDrawMesh contains the low level hardware buffers specific to the current device ie Direct9, 10, OGL etc*/
		DllExport void	SetGFXMesh(IHWSubMesh *msh);
		//! \brief  This lets you get the IHWDrawMesh asssociated with this mesh.
		/*! The IHWDrawMesh contains the low level hardware buffers specific to the current device ie Direct9, 10, OGL etc*/
		DllExport IHWSubMesh* GetGFXMesh();

		//! \brief This allows you to update the buffers
		/*! This allows you to change the position and normals of the mesh without recreating the hwmesh.		
		\param vertexList This is a pointer of all the vertices on the mesh
		\param gfxNormalList This is a pointer of all the gfx normals on the mesh
		\param faceNormalList This is a pointer of all the face normals on the mesh
		\param changedVerts This is a bitarray of the changed verts
		\param count This is number of vertices in vertex List\n\n
		*/
		DllExport void UpdateVertexBuffer(Point3 *vertexList,Point3 *gfxNormalList, Point3 *faceNormalList, BitArray &changedVerts, int count);


		

	private:
		DWORD					mMatID;				//the matid associated with this chunk.  We only allow one material per chunk
		HWVertexBuffer			*mHWVertexBuffer;	// our vertex buffer
		Tab<HWIndexBuffer*>		mPrimitives;	// our array of index buffers


		IHWSubMesh				*mHWDrawMesh;		//pointer to the class that contains our graphics hardware specific buffers

	};



	//! \brief  This is a container class of our sub meshes
	/*!  This class is a container for all our sub mesh.  We separate each mesh by matid and vertex buffer size
	*/
	class HWTupleMeshContainer : public IHWDrawMesh
	{
	public:
		//! \brief Constructor		
		DllExport HWTupleMeshContainer();
		//! \brief Destructor
		DllExport ~HWTupleMeshContainer();	

		//! \brief  This frees all the sub meshes  and sets the sub mesh count to 0
		DllExport void FreeMeshes();	

		//! \brief Returns the number of sub meshes
		DllExport unsigned int Count();
		//! \brief Returns a specific sub mesh
		DllExport GFX_MESH::HWTupleMesh *Get(unsigned int id);
		//! \brief Appends a sub mesh to the list
		/*! The container will manage the life of the pointer
			\param hwMesh is the mesh to append to the end of the list
		*/
		DllExport void Append(GFX_MESH::HWTupleMesh *hwMesh);



		//! \brief Draws all meshes that match the attribute
		/*!  
			\param gw the graphics window to draw to
			\param attribute the mat id to check
		*/
		DllExport void Draw(GraphicsWindow *gw, DWORD attribute);

		//! \brief Lets you update the vertex buffers
		/*!  Allows you to change the position and normals of the mesh without recreating the hw mesh		
		\param vertexList This is a pointer of all the vertices on the mesh
		\param gfxNormalList This is a pointer of all the gfx normals on the mesh
		\param faceNormalList This is a pointer of all the face normals on the mesh
		\param changedVerts This is a bitarray of all the changed verts
		\param count This is number of vertices in vertex List
		\param vertMapList This is mapping that lets the HW vertex look up which vertex it belongs to
		\param normalsMapList This is mapping that lets the HW vertex look up which normal it belongs to.  If this 
			value is negative it is made positive and then face normallist is used to lookup the normal		
			This occurs when there is no smoothing group.
		*/
		DllExport void UpdateVertexBuffer(Point3 *vertexList,Point3 *gfxNormalList, Point3 *faceNormalList, BitArray &changedVerts, int count,
			unsigned int *vertMapList, int *normalsMapList);

		//! \brief  Releases this class pointer		
		 DllExport void Release();

	private:
		//new hw mesh representation
		Tab<GFX_MESH::HWTupleMesh *> mHWTupleMesh; //this is an array of all our sub mesh. a sub mesh are faces thathave the same mat ID
	};





};
