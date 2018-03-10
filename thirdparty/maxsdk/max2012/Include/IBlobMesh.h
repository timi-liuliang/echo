

#pragma once

#include "iFnPub.h"

//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define BLOBMESH_INTERFACE Interface_ID(0xDE17A66a, 0x8A41E45d)


enum {  blobmesh_addnode, blobmesh_removenode,
		blobmesh_addpfnode, blobmesh_removepfnode,
		
		blobmesh_pickmode, blobmesh_addmode,
		blobmesh_addpfmode 

		 };


/*!
The IBlobMesh class provides access to the UI controls of the Blob Mesh object;
adding and removing nodes programatically, and initiating pick mode. These methods
are exposed for MAXScript usage but also be called through the SDK. The interface can 
be obtained as follows:

\code
//assumes blobMesh points to a Blob Mesh object in the scene
IBlobMesh* iblobMesh = (IBlobMesh*)blobMesh->GetInterface(BLOBMESH_INTERFACE);
\endcode
*/
class IBlobMesh : public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP

			VFN_1(blobmesh_addnode, fnAddNode, TYPE_INODE);
			VFN_1(blobmesh_removenode, fnRemoveNode, TYPE_INODE);

			VFN_1(blobmesh_addpfnode, fnAddPFNode, TYPE_INODE);
			VFN_1(blobmesh_removepfnode, fnRemovePFNode, TYPE_INODE);

			VFN_0(blobmesh_pickmode, fnPickMode);
			VFN_0(blobmesh_addmode, fnAddMode);
			VFN_0(blobmesh_addpfmode, fnAddPFMode);

		END_FUNCTION_MAP


		FPInterfaceDesc* GetDesc();    // <-- must implement 
//note functions that start with fn are to be used with maxscript since these expect 1 based indices
		virtual void	fnAddNode(INode *node)=0;
		virtual void	fnRemoveNode(INode *node)=0;

		virtual void	fnAddPFNode(INode *node)=0;
		virtual void	fnRemovePFNode(INode *node)=0;

		virtual void	fnPickMode() = 0;
		virtual void	fnAddMode() = 0;
		virtual void	fnAddPFMode() = 0;


	};





