#pragma once

#include "maxheap.h"
#include "Simpobj.h"
#include "export.h"


enum {PLACE,SIMPLE,STANDARD,INST,META,FRAG};
enum { NTNM,DTNOM,TNOM,NOTM,DTM,TM};
class ParticleData: public MaxHeapOperators
{ public:
	 Point3 position,velocity;
	 float size,life,age;
	 BYTE type[2];
	 Matrix3 *tform;
	 Object* src;
};
struct Groupdata: public MaxHeapOperators
{ float stepSize,starttime;
	int count;
	BOOL valid;float tvalid;
};

class ParticleMatData: public MaxHeapOperators
{ public:
	unsigned short mtltbl;
	MtlID matid;
};   
class WithTex: public MaxHeapOperators
{ public:
	 BYTE bytype;
   Point3 tv;
};
class PartwithTex:public ParticleData,public WithTex
{};
class PartwithMtl:public ParticleData,public ParticleMatData
{};
class MSpin: public MaxHeapOperators
{ public:
   float mass;
   Point3 spinaxis;
	 float stretch;
};
class TexMatData: public WithTex,public ParticleMatData
{};
class ParticlePtrTab:public Tab<ParticleData *> 
{ public:
    void Clear();
		void Add(int c);
		PartwithTex *gettv(int i);
		PartwithMtl *getmtl(int i);
};
class BaseSimpleData :public ParticleData
{ public:
	float spinrate,spinphase;
	int table,sub;
};
class SimpleData :public BaseSimpleData
{ public:
	short sdtype;
	float dither;
};
class SimpleDataMat :public SimpleData,public ParticleMatData
{};
class SimpleDataTex :public SimpleData,public WithTex
{};
class SimpleDataMatTex :public SimpleDataTex,public ParticleMatData
{};
class StandardData :public SimpleData,public MSpin
{};
class StandardDataMat :public StandardData,public ParticleMatData
{};
class StandardDataTex :public StandardData,public WithTex
{};
class StandardDataMatTex :public StandardDataTex,public ParticleMatData
{};
class MetaPartData:public ParticleData
{ public:
	float mass,tension;
};
class MetaPartDataMat :public MetaPartData,public ParticleMatData
{};
class MetaPartDataTex :public MetaPartData,public WithTex
{};
class MetaPartDataMatTex :public ParticleMatData,public MetaPartDataTex
{};
class InstanceData:public BaseSimpleData,public MSpin
{ public:
		BYTE ofs[2];
    float animoffset;
    INode *objectptr;
};
class InstanceDataMat :public InstanceData,public ParticleMatData
{};
class InstanceDataTex :public InstanceData,public WithTex
{};
class InstanceDataMatTex :public InstanceDataTex,public ParticleMatData
{};
class FragmentData :public BaseSimpleData,public MSpin
{ public:
	FragmentData();
	FragmentData(BOOL nomesh);
	~FragmentData();
  Mesh *pmesh;
};
class FragmentDataMat :public FragmentData,public ParticleMatData
{	public:
	FragmentDataMat(BOOL nomesh=FALSE);
};
class FragmentDataTex :public FragmentData,public WithTex
{	public:
	FragmentDataTex(BOOL nomesh=FALSE);
};
class FragmentDataMatTex :public FragmentDataTex,public ParticleMatData
{	public:
	FragmentDataMatTex(BOOL nomesh=FALSE);
};
struct SourceInfo: public MaxHeapOperators{
	int refnum;
};
struct EffectsInfo: public MaxHeapOperators{
	int refnum;
	BOOL test;
	int trefnum,frefnum;
};
typedef Tab<INode*> InputTab;
typedef Tab<SourceInfo*> SourceDataTab;
typedef Tab<EffectsInfo*> EffectsDataTab;
#define BASEP 1
class PodObj;
typedef PodObj* PodObjPtr;

class PodObj : public SimpleObject2
{
public:	
	BOOL updatenow;
	DllExport void UpdateWrangler();
	DllExport void AddToList(INode *newnode,int i,BOOL add);
	DllExport BOOL AddSourceBinding(INode *node); 
	DllExport BOOL AddOperatorBinding(INode *node,BOOL test); 
	DllExport void AddResBinding(INode *node,int effnum,BOOL torf);
	DllExport void DeleteFromList(int nnum);
	DllExport INode *GetSource(int num);
	DllExport INode *GetEffects(int num);
	DllExport INode *GetResultT(int num);
	DllExport INode *GetResultF(int num);
	InputTab InputList;
	SourceDataTab Source;
	EffectsDataTab Effects;
	int podnum,groupnum;
	#pragma warning(push)
	#pragma warning(disable:4100)
	virtual void GetGroupStats(int gnum,Groupdata &gdata,PodObjPtr *plist) {};
	virtual void UpdatePod(float t=0.0f,float stepSize=-1.0f) {};

	using SimpleObject2::GetInterface;
	virtual void* GetInterface(ULONG id) { return SimpleObject2::GetInterface(id); }

	virtual BOOL TryBinding(INode *node) {return FALSE;}
	virtual void GetPart(ParticleData &part,int i){}
	#pragma warning(pop)
};

class ITestInterface: public MaxHeapOperators 
{
	public:
		virtual int NPTestInterface(TimeValue t,BOOL UpdatePastCollide,ParticleData *part,float dt,INode *node,int index)=0;
};

class IOperatorInterface: public MaxHeapOperators 
{
	public:
		virtual int NPOpInterface(TimeValue t,ParticleData *part,float dt,INode *node,int index)=0;
		virtual int UseOtherPod() {return -1;}
};

