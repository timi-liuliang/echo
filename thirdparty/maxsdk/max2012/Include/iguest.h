/**********************************************************************
 *<
	FILE: IGuest.h

	DESCRIPTION: Declares Host/Guest protocol

	CREATED BY:	John Hutchinson

	HISTORY: Created April 24, 1999

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "maxheap.h"

// forward declarations
class IGeomImp;
class IHost;
class Matrix3;
class INode;
class HitRecord;
class Control;

class IGuest : public MaxHeapOperators 
{
public:
	virtual void rsvp(IHost* host, IGeomImp* return_envelope, Matrix3& tm) = 0; 
};


class IHost : public MaxHeapOperators
{
public:
	virtual void accomodate(IGeomImp* guestrep, Matrix3& tm, HitRecord* rec = NULL) = 0;
	virtual bool locate(INode* host, Control* c, Matrix3& oldP, Matrix3& newP) = 0;
};
