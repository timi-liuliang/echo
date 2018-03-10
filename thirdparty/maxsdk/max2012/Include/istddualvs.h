/**********************************************************************
 *<
	FILE: IStdDualVS.h

	DESCRIPTION: Standard Dual VertexShader helper class interface definition

	CREATED BY: Nikolai Sander, Discreet

	HISTORY: created 10/11/00

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "IHardwareShader.h"
#include "ref.h"
#include "plugapi.h"

#define STD_DUAL_VERTEX_SHADER Class_ID(0x40e6b1a0, 0x549cbf06)

class VertexShaderCache: public MaxHeapOperators
{
	bool valid;
	INode *node;
public:
	VertexShaderCache(){ node = NULL; valid = false;}
	~VertexShaderCache(){}
	INode *GetNode() { return node;}
	void SetNode(INode *node) { this->node = node;}
	bool GetValid() { return valid;}
	void SetValid(bool valid) { this->valid = valid;}	
};


class IStdDualVSCallback: public MaxHeapOperators
{
public:
	virtual ReferenceTarget *GetRefTarg()=0;
	virtual VertexShaderCache *CreateVertexShaderCache()=0;
	virtual HRESULT InitValid(Mesh* mesh, INode *node)=0;
	virtual HRESULT InitValid(MNMesh* mnmesh, INode *node)=0;
	virtual void DeleteRenderMeshCache(INode * node) { UNUSED_PARAM(node); };
};

class IStdDualVS : public IVertexShader, public ReferenceMaker
{
public:
	virtual ~IStdDualVS(){};	
	virtual void SetCallback(IStdDualVSCallback *callback)=0;
	virtual int FindNodeIndex(INode *node)=0;
	virtual Class_ID ClassID(){ return STD_DUAL_VERTEX_SHADER;}
	virtual SClass_ID SuperClassID() { return REF_MAKER_CLASS_ID;}
	virtual void DeleteThis()=0;
};

