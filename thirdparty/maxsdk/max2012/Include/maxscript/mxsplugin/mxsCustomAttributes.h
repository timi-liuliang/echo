/*	
 *		MSCustAttrib.h - MAXScript scriptable custom attributes MAX objects
 *
 *			Copyright (c) Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

// ---------- scripter Custom Attribute classes -------------------

#include <map>

#include "..\..\custattrib.h"
#include "..\..\ICustAttribContainer.h"
#include "mxsplugin.h"

#define I_SCRIPTEDCUSTATTRIB	0x000010C1

#define MSCUSTATTRIB_CHUNK	0x0110
#define MSCUSTATTRIB_NAME_CHUNK	0x0010


// special subclass for holding custom attribute definitions
// these can be applied to any existing object, adding a CustAttrib to it
//   instances of MSCustAttrib (an MSPlugin subclass) refer to CustAttribDefs for metadata
visible_class (MSCustAttribDef)

class MSCustAttribDef;
typedef std::map<Class_ID, MSCustAttribDef*> MSCustAttribDefMap;
typedef MSCustAttribDefMap::iterator MSCustAttribDefMapIter;
typedef MSCustAttribDefMap::const_iterator MSCustAttribDefMapConstIter;
typedef MSCustAttribDefMap::value_type MSCustAttribDefMapValue;


class MSCustAttribDef : public MSPluginClass
{
public:
	// Map of existing scripted attribute defs, indexed on the ClassID.  This is used in redefining attributes
	ScripterExport static MSCustAttribDefMap ms_attrib_defs;	

	Value*			defData;			// persistent definition data, used by the scripter attribute editor
	MSTR			source;				// extracted definition source, stored persistently with the def & automatically recompiled on 
										// reload

	ScripterExport	MSCustAttribDef(Class_ID& attrib_id);
	ScripterExport	~MSCustAttribDef();

	// definition and redefinition
	static MSCustAttribDef* intern(Class_ID& attrib_id);
	ScripterExport void		init(int local_count, Value** inits, HashTable* local_scope, HashTable* handlers, Array* pblock_defs, Array* iremap_param_names, Array* rollouts, CharStream* source);

	// MAXScript required
//	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSCustAttribDef)) ? 1 : Value::is_kind_of(c); } // LAM: 2/23/01
	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSCustAttribDef)) ? 1 : MSPluginClass::is_kind_of(c); }
#	define			is_attribute_def(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(MSCustAttribDef))
	void			collect() { delete this; }
	void			gc_trace();
	void			sprin1(CharStream* s);

	bool			is_custAttribDef() { return true; }
	ScripterExport	MSCustAttribDef* unique_clone();

	// from Value 
	Value*			apply(Value** arg_list, int count, CallContext* cc=NULL) { return Value::apply(arg_list, count, cc); }  // CustAttribDef's are not applyable

	// scene I/O
	static IOResult	save_custattrib_defs(ISave* isave);
	static IOResult	load_custattrib_defs(ILoad* iload);

	// ClassDesc delegates
	RefTargetHandle	Create(BOOL loading);

	Value*			get_property(Value** arg_list, int count);
	Value*			set_property(Value** arg_list, int count);

	def_property ( name );
};


#pragma warning(push)
#pragma warning(disable:4239)

// MSCustAttrib - instances contain individual custom attribute blocks 
//   that are added to customized objects.
class MSCustAttrib : public MSPlugin, public CustAttrib, public ISubMap
{
public:
	IObjParam*				cip;		// ip for any currently open command panel dialogs
	static MSAutoMParamDlg* masterMDlg; // master dialog containing all scripted rollout
	IMtlParams*				mip;		// ip for any open mtlEditor panel dlgs
	MSTR					name;		// name of the custom attribute

	ScripterExport	MSCustAttrib() : cip(NULL), mip(NULL) { }
	ScripterExport	MSCustAttrib(MSCustAttribDef* pc, BOOL loading);
	ScripterExport	~MSCustAttrib() { DeleteAllRefsFromMe(); }

	// Needed to solve ambiguity between Collectable's operators and MaxHeapOperators
	using Collectable::operator new;
	using Collectable::operator delete;

	ScripterExport void	sprin1(CharStream* s);

	// From MSPlugin
	ScripterExport	HWND			AddRollupPage(HINSTANCE hInst, MCHAR *dlgTemplate, DLGPROC dlgProc, MCHAR *title, LPARAM param=0,DWORD flags=0, int category=ROLLUP_CAT_CUSTATTRIB - 1);
	ScripterExport	void			DeleteRollupPage(HWND hRollup);
	ScripterExport	IRollupWindow*  GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegates in MSCustAttribs 

	// from CustAttrib
	const MCHAR*			GetName()	{ return name; } 
	ScripterExport	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams *imp);

	void			SetName(const MCHAR* newName)	{ name = newName ; }

	// From Animatable
    
    using CustAttrib::GetInterface;

	void			GetClassName(MSTR& s) { s = MSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	MSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	BOOL			CanCopyAnim() { return FALSE; }

	ScripterExport	void*			GetInterface(ULONG id);
	ScripterExport	void			DeleteThis();
	ScripterExport	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	ScripterExport	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						if (!(pc->mpc_flags & MPC_REDEFINITION))
							return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
						else
							return REF_SUCCEED;
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
protected:
	virtual void	SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count())
							pblocks.SetCount(i+1); 
						pblocks[i] = (IParamBlock2*)rtarg; 
					}
public:
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap);
	
	ScripterExport	IOResult		Save(ISave *isave);
    ScripterExport	IOResult		Load(ILoad *iload);

	// from ISubMap
	ScripterExport	int				NumSubTexmaps();
	ScripterExport	Texmap*			GetSubTexmap(int i);
	ScripterExport	void			SetSubTexmap(int i, Texmap *m);
	ScripterExport	MSTR			GetSubTexmapSlotName(int i);
	int				MapSlotType(int i) { UNUSED_PARAM(i); return MAPSLOT_TEXTURE; }
	MSTR			GetSubTexmapTVName(int i) { return GetSubTexmapSlotName(i); }
	ReferenceTarget *GetRefTarget() { return this; }
};

#pragma warning(pop)

// ref enumerator to find an owner for this Custom Attribute
class FindCustAttribOwnerDEP : public DependentEnumProc 
{
public:		
	ReferenceMaker* rm;
	CustAttrib* targ_ca;
	FindCustAttribOwnerDEP(CustAttrib* ca) { rm = NULL; targ_ca = ca; }
	int proc(ReferenceMaker* rmaker)
	{
		if (rmaker == targ_ca)
			return DEP_ENUM_CONTINUE;

		// russom - 08/27/04 - 579271
		// Make sure we only eval real dependencies.
		// Note: We might need to also add a SKIP for restore class ids, but that
		// is not part of this defect fix.
		if( !rmaker->IsRealDependency(targ_ca) ) 
			return DEP_ENUM_SKIP;

		if (rmaker->ClassID() == CUSTATTRIB_CONTAINER_CLASS_ID )
		{
			ICustAttribContainer* cac = (ICustAttribContainer*)rmaker;
			Animatable* owner = cac->GetOwner();
			if (owner)
			{
				SClass_ID sid = owner->SuperClassID();
				if (sid != MAKEREF_REST_CLASS_ID  && sid != MAXSCRIPT_WRAPPER_CLASS_ID && sid != DELREF_REST_CLASS_ID)
				{
					for (int j = 0; j < cac->GetNumCustAttribs(); j++)
					{
						CustAttrib* ca = cac->GetCustAttrib(j);
						if (ca == targ_ca)
						{
							rm = (ReferenceMaker*)owner;
							return DEP_ENUM_HALT;
						}
					}
				}
			}
		}
		return DEP_ENUM_SKIP; // only need to look at immediate dependents
	}

};

