/*********************************************************************
 *<
	FILE: sceneTraversal.h

	DESCRIPTION: Interface for plugins controlling display traversal

	CREATED BY:	John Hutchinson

	HISTORY: November 5 2003

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "maxtypes.h"
#include "baseinterface.h"

// forward declarations
class NodeIterator;
class ViewExp;
class INode;

#define SCENETRAVERSAL_INTERFACE Interface_ID(0x35380d8c, 0x48e54b8c)

// An interface for controlling the display traversal of the scene
class ISceneTraversal : public BaseInterface
{
public:

	enum IteratorContext
	{
		kDisplay = 0,
		kHitTest = 1
	};

	virtual NodeIterator *GetIterator(ViewExp* vpt, IteratorContext context) = 0;
};

class NodeIterator: public MaxHeapOperators
{
public:
	virtual ~NodeIterator() {;}
	virtual void DeleteThis()=0;	// used to implement { delete this; }
	virtual void Reset() = 0;
	virtual INode *next() = 0;
};