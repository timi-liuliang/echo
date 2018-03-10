/*	
 *		OLEAutomation.h - OLE Automation services for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\foundation\arrays.h"
#include "..\foundation\functions.h"
#include "OLE.classids.h"

/* error scodes */

#define MS_E_EXCEPTION					MAKE_SCODE(SEVERITY_ERROR, FACILITY_ITF, 0x0200)  
#define MS_E_ILLEGAL_RETURN_VALUE       MS_E_EXCEPTION + 0x001  

/* ------- the MAXScript OLE object class factory ---------- */

class MSClassFactory : public IClassFactory
{
public:
    static IClassFactory* Create();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID iid, void** ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IClassFactory methods */
    STDMETHOD(CreateInstance)(IUnknown* pUnkOuter, REFIID iid, void** ppv);
    STDMETHOD(LockServer)(BOOL fLock);

private:
    MSClassFactory();

    unsigned long m_refs;
};

/* ---------- the MAXScript OLE object class -------------- */

class MSOLEObject : public IDispatch
{
public:
    static MSOLEObject* Create();

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void** ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(unsigned int* pcTypeInfo);
    STDMETHOD(GetTypeInfo)(unsigned int iTypeInfo, LCID lcid, ITypeInfo** ppTypeInfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR** rgszNames, unsigned int cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags,
					  DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, unsigned int* puArgErr);

	/* MSOLEObject stuff */

    unsigned long m_refs;
	static Array* exposed_fns;		// array of exposed MAXScript functions, DISPID is 1-based index in array

    MSOLEObject();

	static void install_fns(Array* fns);
};

/* ---------------- client-side classes -------------------- */

visible_class (OLEObject)

class OLEObject : public Value
{
public:
    Value*		progID;		// user-supplied progID string
	CLSID		clsid;		// CLSID of ActiveX object.
    LPDISPATCH	pdisp;		// IDispatch of ActiveX object.

				OLEObject(Value* progID, CLSID cslid, LPDISPATCH pdisp);
				OLEObject(Value* progID, LPDISPATCH pdisp);
			   ~OLEObject();
	#define		is_OLEObject(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(OLEObject))

				classof_methods (OLEObject, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void		sprin1(CharStream* s);

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	Value*		get_fn_property(Value* prop);
};

visible_class (OLEMethod)

class OLEMethod : public Function
{
public:
	OLEObject*	ole_obj;	// my OLE object
    DISPID		dispid;		// method dispatch ID

				OLEMethod() { }
				OLEMethod(MCHAR* name, OLEObject* ole_obj, DISPID mth_id);
	#define		is_OLEMethod(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(OLEMethod))

				classof_methods (OLEMethod, Function);
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count, CallContext* cc=NULL);
};

/* ---------------- SAFEARRAY wrapper class -------------------- */
applyable_class_debug_ok (SafeArrayWrapper)

// MXS Value helper class for converting between MXS Arrays and OLE SAFEARRAYs.
class SafeArrayWrapper : public Value
{
	friend SafeArrayWrapperClass;
public:
	Value*	dataArray;
	Value*	lBoundsArray;
	int		numDims;

// Creates SafeArrayWrapper with empty data and lBounds arrays.
	ScripterExport SafeArrayWrapper();
// Creates SafeArrayWrapper with data array and empty lBounds array.
// \pre dataArray must be non-null
// \param[in] dataArray The data array
// \param[in] nDims The dimensions of the data array
	ScripterExport SafeArrayWrapper(Array* dataArray, int nDims);
// Creates SafeArrayWrapper with data and lBounds array.
// \pre dataArray must be non-null
// \pre lboundsArray must be non-null
// \param[in] dataArray The data array
// \param[in] lBoundsArray The lBounds array
// \param[in] nDims The dimensions of the data and lBounds array
	ScripterExport SafeArrayWrapper(Array* dataArray, Array* lBoundsArray, int nDims);
// Creates SafeArrayWrapper from SAFEARRAY.
// \pre psa must be non-null
// \param[in] psa The SAFEARRAY to build the data and lBounds array from
// \param[in] progID user-supplied progID string of the creating object 
	ScripterExport SafeArrayWrapper(SAFEARRAY *psa, Value* progID);
// Creates OLE SAFEARRAY based on data and lBounds array.
// if dataArray is 'undefined', throws exception
// if lBoundsArray is 'undefined', lower bounds of 0 used
// if dataArray is not "square", throws exception
// \return the SAFEARRAY
	ScripterExport SAFEARRAY* to_SAFEARRAY();

// Calculates number of dimensions of input Array.
// \pre testArray must be non-null
// \param[in] testArray Array to calculate the dimensions of
// \return Number of dimensions
	ScripterExport static int		GetDataArrayDimensions(Array* testArray);
// Calculates the array size for each dimension.
// Verifies the arrays size in each dimension is consistent across array - throws runtime error if not.
// \pre testArray can be null
// \pre pCount must be non-null
// \post pCount is filled with array size for each dimension
// \param[in] testArray Array to calculate the dimensions of
// \param[in] nDims The dimensions of the Array
// \param[in,out] pCount pointer to nDims size int array. Method fills array with array size for each dimension
	ScripterExport static void		CheckDataArray(Array* testArray, int nDims, int* pCount = NULL);

#define		is_SafeArrayWrapper(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(SafeArrayWrapper))

	classof_methods (SafeArrayWrapper, Value);
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);

	/* operations */

#include "..\macros\define_implementations.h"
	use_generic( copy,		"copy");

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

};


BOOL init_MAXScript_OLE();
void uninit_OLE();

#define UNUSED(X) (X)

