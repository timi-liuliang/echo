#pragma once
#include "maxheap.h"
#include "iparamm.h"
#include "iparamm2.h"
#include "Simpobj.h"

#pragma warning(push)
#pragma warning(disable:4100)

class DynamHelperObject : public SimpleObject, IParamArray
{
public:
	virtual	INode *GetEndNode1(){return NULL;}
	virtual	INode *GetEndNode2(){return NULL;}
	virtual	Point3 ApplyAtEnd1(TimeValue t){return Point3(0.0f,0.0f,0.0f);}
	virtual	Point3 ApplyAtEnd2(TimeValue t){return Point3(0.0f,0.0f,0.0f);}
	virtual	Point3 Force(TimeValue t,TimeValue dt){return Point3(0.0f,0.0f,0.0f);}
};

struct FlectForces : public MaxHeapOperators
{
  TimeValue t1,t2;
  Point3 FlectForce,ApplyAt;
  int Num;
};

class DynamModObject : public SimpleWSMObject2
{ 
public:
	virtual FlectForces ForceData(TimeValue t)
	{
		FlectForces f1;
		f1.t1 = 0;
		f1.t2 = 0;
		f1.FlectForce = Point3(0.0f,0.0f,0.0f);
		f1.ApplyAt    = Point3(0.0f,0.0f,0.0f);
		return f1;
	}

	int	NumParamBlocks() { return 1; }
	IParamBlock2* GetParamBlock(int i) { return pblock2; }
	IParamBlock2* GetParamBlockByID(BlockID id) { return (pblock2->ID() == id) ? pblock2 : NULL; }
};

#pragma warning(pop)

