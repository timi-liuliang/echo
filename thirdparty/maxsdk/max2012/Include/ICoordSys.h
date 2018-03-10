/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Jul 25 11:57:48 1999
 */
/* Compiler settings for ICoordSys.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#pragma once

/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IGcsBitmap_FWD_DEFINED__
#define __IGcsBitmap_FWD_DEFINED__
typedef interface IGcsBitmap IGcsBitmap;
#endif 	/* __IGcsBitmap_FWD_DEFINED__ */


#ifndef __IGcsPointTab_FWD_DEFINED__
#define __IGcsPointTab_FWD_DEFINED__
typedef interface IGcsPointTab IGcsPointTab;
#endif 	/* __IGcsPointTab_FWD_DEFINED__ */


#ifndef __IGcsNodeTab_FWD_DEFINED__
#define __IGcsNodeTab_FWD_DEFINED__
typedef interface IGcsNodeTab IGcsNodeTab;
#endif 	/* __IGcsNodeTab_FWD_DEFINED__ */


#ifndef __IGcsTriObject_FWD_DEFINED__
#define __IGcsTriObject_FWD_DEFINED__
typedef interface IGcsTriObject IGcsTriObject;
#endif 	/* __IGcsTriObject_FWD_DEFINED__ */


#ifndef __IGcsSplineShape_FWD_DEFINED__
#define __IGcsSplineShape_FWD_DEFINED__
typedef interface IGcsSplineShape IGcsSplineShape;
#endif 	/* __IGcsSplineShape_FWD_DEFINED__ */


#ifndef __IGcsTransform_FWD_DEFINED__
#define __IGcsTransform_FWD_DEFINED__
typedef interface IGcsTransform IGcsTransform;
#endif 	/* __IGcsTransform_FWD_DEFINED__ */


#ifndef __IGcsUI_FWD_DEFINED__
#define __IGcsUI_FWD_DEFINED__
typedef interface IGcsUI IGcsUI;
#endif 	/* __IGcsUI_FWD_DEFINED__ */


#ifndef __IGcsLoad_FWD_DEFINED__
#define __IGcsLoad_FWD_DEFINED__
typedef interface IGcsLoad IGcsLoad;
#endif 	/* __IGcsLoad_FWD_DEFINED__ */


#ifndef __IGcsSave_FWD_DEFINED__
#define __IGcsSave_FWD_DEFINED__
typedef interface IGcsSave IGcsSave;
#endif 	/* __IGcsSave_FWD_DEFINED__ */


#ifndef __IGcsSession_FWD_DEFINED__
#define __IGcsSession_FWD_DEFINED__
typedef interface IGcsSession IGcsSession;
#endif 	/* __IGcsSession_FWD_DEFINED__ */


#ifndef __GcsSession_FWD_DEFINED__
#define __GcsSession_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsSession GcsSession;
#else
typedef struct GcsSession GcsSession;
#endif /* __cplusplus */

#endif 	/* __GcsSession_FWD_DEFINED__ */


#ifndef __GcsPointTab_FWD_DEFINED__
#define __GcsPointTab_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsPointTab GcsPointTab;
#else
typedef struct GcsPointTab GcsPointTab;
#endif /* __cplusplus */

#endif 	/* __GcsPointTab_FWD_DEFINED__ */


#ifndef __GcsNodeTab_FWD_DEFINED__
#define __GcsNodeTab_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsNodeTab GcsNodeTab;
#else
typedef struct GcsNodeTab GcsNodeTab;
#endif /* __cplusplus */

#endif 	/* __GcsNodeTab_FWD_DEFINED__ */


#ifndef __GcsTriObject_FWD_DEFINED__
#define __GcsTriObject_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsTriObject GcsTriObject;
#else
typedef struct GcsTriObject GcsTriObject;
#endif /* __cplusplus */

#endif 	/* __GcsTriObject_FWD_DEFINED__ */


#ifndef __GcsSplineShape_FWD_DEFINED__
#define __GcsSplineShape_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsSplineShape GcsSplineShape;
#else
typedef struct GcsSplineShape GcsSplineShape;
#endif /* __cplusplus */

#endif 	/* __GcsSplineShape_FWD_DEFINED__ */


#ifndef __GcsLoad_FWD_DEFINED__
#define __GcsLoad_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsLoad GcsLoad;
#else
typedef struct GcsLoad GcsLoad;
#endif /* __cplusplus */

#endif 	/* __GcsLoad_FWD_DEFINED__ */


#ifndef __GcsSave_FWD_DEFINED__
#define __GcsSave_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsSave GcsSave;
#else
typedef struct GcsSave GcsSave;
#endif /* __cplusplus */

#endif 	/* __GcsSave_FWD_DEFINED__ */


#ifndef __GcsBitmap_FWD_DEFINED__
#define __GcsBitmap_FWD_DEFINED__

#ifdef __cplusplus
typedef class GcsBitmap GcsBitmap;
#else
typedef struct GcsBitmap GcsBitmap;
#endif /* __cplusplus */

#endif 	/* __GcsBitmap_FWD_DEFINED__ */


/* header files for imported files */
#include "csapi.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_ICoordSys_0000 */
/* [local] */ 

typedef /* [public] */ 
enum __MIDL___MIDL_itf_ICoordSys_0000_0001
    {	eGcsNew	= 0,
	eGcsContainer	= 1,
	eGcsData	= 2
    }	GcsChunkType;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_ICoordSys_0000_0002
    {	eGcsMax	= 0,
	eGcsMat	= 1
    }	GcsFileIOType;

typedef /* [public] */ 
enum __MIDL___MIDL_itf_ICoordSys_0000_0003
    {	eGcsFont	= 0,
	eGcsScene	= 1,
	eGcsImport	= 2,
	eGcsExport	= 3,
	eGcsHelp	= 4,
	eGcsExpression	= 5,
	eGcsPreview	= 6,
	eGcsImage	= 7,
	eGcsSound	= 8,
	eGcsPlugCfg	= 9,
	eGcsMaxStart	= 10,
	eGcsVPost	= 11,
	eGcsDrivers	= 12,
	eGcsAutoBack	= 13,
	eGcsMatLib	= 14,
	eGcsScripts	= 15,
	eGcsStartUpScripts	= 16,
	eGcsUI	= 17,
	eGcsMaxRoot	= 18
    }	GcsDirType;



extern RPC_IF_HANDLE __MIDL_itf_ICoordSys_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ICoordSys_0000_v0_0_s_ifspec;

#ifndef __IGcsBitmap_INTERFACE_DEFINED__
#define __IGcsBitmap_INTERFACE_DEFINED__

/* interface IGcsBitmap */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsBitmap;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("189618F0-F8FD-11d2-AA0F-0060B057DAFB")
    IGcsBitmap : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateNewBitmap( 
            /* [in] */ ULONG bmc,
            /* [in] */ ULONG mtl) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateBitmap( 
            /* [in] */ BSTR name1,
            /* [in] */ BSTR name2) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetName( 
            /* [out] */ BSTR __RPC_FAR *bstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ BSTR bstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGeoData( 
            /* [in] */ ULONG name,
            /* [out] */ ULONG __RPC_FAR *data) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTransform( 
            /* [out] */ ULONG __RPC_FAR *matrix) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTransform( 
            /* [in] */ ULONG matrix) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsBitmapVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsBitmap __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsBitmap __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateNewBitmap )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ ULONG bmc,
            /* [in] */ ULONG mtl);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateBitmap )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ BSTR name1,
            /* [in] */ BSTR name2);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IGcsBitmap __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *bstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ BSTR bstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGeoData )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ ULONG name,
            /* [out] */ ULONG __RPC_FAR *data);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransform )( 
            IGcsBitmap __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *matrix);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTransform )( 
            IGcsBitmap __RPC_FAR * This,
            /* [in] */ ULONG matrix);
        
        END_INTERFACE
    } IGcsBitmapVtbl;

    interface IGcsBitmap
    {
        CONST_VTBL struct IGcsBitmapVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsBitmap_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsBitmap_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsBitmap_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsBitmap_CreateNewBitmap(This,bmc,mtl)	\
    (This)->lpVtbl -> CreateNewBitmap(This,bmc,mtl)

#define IGcsBitmap_CreateBitmap(This,name1,name2)	\
    (This)->lpVtbl -> CreateBitmap(This,name1,name2)

#define IGcsBitmap_GetName(This,bstrName)	\
    (This)->lpVtbl -> GetName(This,bstrName)

#define IGcsBitmap_SetName(This,bstrName)	\
    (This)->lpVtbl -> SetName(This,bstrName)

#define IGcsBitmap_GetGeoData(This,name,data)	\
    (This)->lpVtbl -> GetGeoData(This,name,data)

#define IGcsBitmap_GetTransform(This,matrix)	\
    (This)->lpVtbl -> GetTransform(This,matrix)

#define IGcsBitmap_SetTransform(This,matrix)	\
    (This)->lpVtbl -> SetTransform(This,matrix)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_CreateNewBitmap_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [in] */ ULONG bmc,
    /* [in] */ ULONG mtl);


void __RPC_STUB IGcsBitmap_CreateNewBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_CreateBitmap_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [in] */ BSTR name1,
    /* [in] */ BSTR name2);


void __RPC_STUB IGcsBitmap_CreateBitmap_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_GetName_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *bstrName);


void __RPC_STUB IGcsBitmap_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_SetName_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [in] */ BSTR bstrName);


void __RPC_STUB IGcsBitmap_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_GetGeoData_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [in] */ ULONG name,
    /* [out] */ ULONG __RPC_FAR *data);


void __RPC_STUB IGcsBitmap_GetGeoData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_GetTransform_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *matrix);


void __RPC_STUB IGcsBitmap_GetTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsBitmap_SetTransform_Proxy( 
    IGcsBitmap __RPC_FAR * This,
    /* [in] */ ULONG matrix);


void __RPC_STUB IGcsBitmap_SetTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsBitmap_INTERFACE_DEFINED__ */


#ifndef __IGcsPointTab_INTERFACE_DEFINED__
#define __IGcsPointTab_INTERFACE_DEFINED__

/* interface IGcsPointTab */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsPointTab;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD70-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsPointTab : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AppendPoint( 
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Count( 
            /* [out] */ ULONG __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPoint( 
            /* [in] */ ULONG lPoint,
            /* [out] */ float __RPC_FAR *x,
            /* [out] */ float __RPC_FAR *y,
            /* [out] */ float __RPC_FAR *z) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetPoint( 
            /* [in] */ ULONG lPoint,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsPointTabVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsPointTab __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsPointTab __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsPointTab __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppendPoint )( 
            IGcsPointTab __RPC_FAR * This,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Count )( 
            IGcsPointTab __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *plCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPoint )( 
            IGcsPointTab __RPC_FAR * This,
            /* [in] */ ULONG lPoint,
            /* [out] */ float __RPC_FAR *x,
            /* [out] */ float __RPC_FAR *y,
            /* [out] */ float __RPC_FAR *z);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPoint )( 
            IGcsPointTab __RPC_FAR * This,
            /* [in] */ ULONG lPoint,
            /* [in] */ float x,
            /* [in] */ float y,
            /* [in] */ float z);
        
        END_INTERFACE
    } IGcsPointTabVtbl;

    interface IGcsPointTab
    {
        CONST_VTBL struct IGcsPointTabVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsPointTab_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsPointTab_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsPointTab_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsPointTab_AppendPoint(This,x,y,z)	\
    (This)->lpVtbl -> AppendPoint(This,x,y,z)

#define IGcsPointTab_Count(This,plCount)	\
    (This)->lpVtbl -> Count(This,plCount)

#define IGcsPointTab_GetPoint(This,lPoint,x,y,z)	\
    (This)->lpVtbl -> GetPoint(This,lPoint,x,y,z)

#define IGcsPointTab_SetPoint(This,lPoint,x,y,z)	\
    (This)->lpVtbl -> SetPoint(This,lPoint,x,y,z)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsPointTab_AppendPoint_Proxy( 
    IGcsPointTab __RPC_FAR * This,
    /* [in] */ float x,
    /* [in] */ float y,
    /* [in] */ float z);


void __RPC_STUB IGcsPointTab_AppendPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsPointTab_Count_Proxy( 
    IGcsPointTab __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *plCount);


void __RPC_STUB IGcsPointTab_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsPointTab_GetPoint_Proxy( 
    IGcsPointTab __RPC_FAR * This,
    /* [in] */ ULONG lPoint,
    /* [out] */ float __RPC_FAR *x,
    /* [out] */ float __RPC_FAR *y,
    /* [out] */ float __RPC_FAR *z);


void __RPC_STUB IGcsPointTab_GetPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsPointTab_SetPoint_Proxy( 
    IGcsPointTab __RPC_FAR * This,
    /* [in] */ ULONG lPoint,
    /* [in] */ float x,
    /* [in] */ float y,
    /* [in] */ float z);


void __RPC_STUB IGcsPointTab_SetPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsPointTab_INTERFACE_DEFINED__ */


#ifndef __IGcsNodeTab_INTERFACE_DEFINED__
#define __IGcsNodeTab_INTERFACE_DEFINED__

/* interface IGcsNodeTab */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsNodeTab;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD71-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsNodeTab : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AppendNode( 
            /* [in] */ INT_PTR iNode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Count( 
            /* [out] */ ULONG __RPC_FAR *plCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetNode( 
            /* [in] */ ULONG lNode,
            /* [out] */ INT_PTR __RPC_FAR *piNode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsNodeTabVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsNodeTab __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsNodeTab __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsNodeTab __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AppendNode )( 
            IGcsNodeTab __RPC_FAR * This,
            /* [in] */ INT_PTR iNode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Count )( 
            IGcsNodeTab __RPC_FAR * This,
            /* [out] */ ULONG __RPC_FAR *plCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetNode )( 
            IGcsNodeTab __RPC_FAR * This,
            /* [in] */ ULONG lNode,
            /* [out] */ INT_PTR __RPC_FAR *piNode);
        
        END_INTERFACE
    } IGcsNodeTabVtbl;

    interface IGcsNodeTab
    {
        CONST_VTBL struct IGcsNodeTabVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsNodeTab_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsNodeTab_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsNodeTab_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsNodeTab_AppendNode(This,iNode)	\
    (This)->lpVtbl -> AppendNode(This,iNode)

#define IGcsNodeTab_Count(This,plCount)	\
    (This)->lpVtbl -> Count(This,plCount)

#define IGcsNodeTab_GetNode(This,lNode,piNode)	\
    (This)->lpVtbl -> GetNode(This,lNode,piNode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsNodeTab_AppendNode_Proxy( 
    IGcsNodeTab __RPC_FAR * This,
    /* [in] */ INT_PTR iNode);


void __RPC_STUB IGcsNodeTab_AppendNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsNodeTab_Count_Proxy( 
    IGcsNodeTab __RPC_FAR * This,
    /* [out] */ ULONG __RPC_FAR *plCount);


void __RPC_STUB IGcsNodeTab_Count_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsNodeTab_GetNode_Proxy( 
    IGcsNodeTab __RPC_FAR * This,
    /* [in] */ ULONG lNode,
    /* [out] */ INT_PTR __RPC_FAR *piNode);


void __RPC_STUB IGcsNodeTab_GetNode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsNodeTab_INTERFACE_DEFINED__ */


#ifndef __IGcsTriObject_INTERFACE_DEFINED__
#define __IGcsTriObject_INTERFACE_DEFINED__

/* interface IGcsTriObject */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsTriObject;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD72-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsTriObject : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMesh( 
            /* [in] */ int iMesh) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVertices( 
            /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetVertices( 
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsTriObjectVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsTriObject __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsTriObject __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsTriObject __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMesh )( 
            IGcsTriObject __RPC_FAR * This,
            /* [in] */ int iMesh);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVertices )( 
            IGcsTriObject __RPC_FAR * This,
            /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVertices )( 
            IGcsTriObject __RPC_FAR * This,
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints);
        
        END_INTERFACE
    } IGcsTriObjectVtbl;

    interface IGcsTriObject
    {
        CONST_VTBL struct IGcsTriObjectVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsTriObject_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsTriObject_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsTriObject_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsTriObject_SetMesh(This,iMesh)	\
    (This)->lpVtbl -> SetMesh(This,iMesh)

#define IGcsTriObject_GetVertices(This,ppPoints)	\
    (This)->lpVtbl -> GetVertices(This,ppPoints)

#define IGcsTriObject_SetVertices(This,pPoints)	\
    (This)->lpVtbl -> SetVertices(This,pPoints)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTriObject_SetMesh_Proxy( 
    IGcsTriObject __RPC_FAR * This,
    /* [in] */ int iMesh);


void __RPC_STUB IGcsTriObject_SetMesh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTriObject_GetVertices_Proxy( 
    IGcsTriObject __RPC_FAR * This,
    /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints);


void __RPC_STUB IGcsTriObject_GetVertices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTriObject_SetVertices_Proxy( 
    IGcsTriObject __RPC_FAR * This,
    /* [in] */ IGcsPointTab __RPC_FAR *pPoints);


void __RPC_STUB IGcsTriObject_SetVertices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsTriObject_INTERFACE_DEFINED__ */


#ifndef __IGcsSplineShape_INTERFACE_DEFINED__
#define __IGcsSplineShape_INTERFACE_DEFINED__

/* interface IGcsSplineShape */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsSplineShape;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD73-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsSplineShape : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSpline( 
            /* [in] */ int iSpline) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetVertices( 
            /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetVertices( 
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsSplineShapeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsSplineShape __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsSplineShape __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsSplineShape __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSpline )( 
            IGcsSplineShape __RPC_FAR * This,
            /* [in] */ int iSpline);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVertices )( 
            IGcsSplineShape __RPC_FAR * This,
            /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetVertices )( 
            IGcsSplineShape __RPC_FAR * This,
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints);
        
        END_INTERFACE
    } IGcsSplineShapeVtbl;

    interface IGcsSplineShape
    {
        CONST_VTBL struct IGcsSplineShapeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsSplineShape_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsSplineShape_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsSplineShape_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsSplineShape_SetSpline(This,iSpline)	\
    (This)->lpVtbl -> SetSpline(This,iSpline)

#define IGcsSplineShape_GetVertices(This,ppPoints)	\
    (This)->lpVtbl -> GetVertices(This,ppPoints)

#define IGcsSplineShape_SetVertices(This,pPoints)	\
    (This)->lpVtbl -> SetVertices(This,pPoints)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSplineShape_SetSpline_Proxy( 
    IGcsSplineShape __RPC_FAR * This,
    /* [in] */ int iSpline);


void __RPC_STUB IGcsSplineShape_SetSpline_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSplineShape_GetVertices_Proxy( 
    IGcsSplineShape __RPC_FAR * This,
    /* [out] */ IGcsPointTab __RPC_FAR *__RPC_FAR *ppPoints);


void __RPC_STUB IGcsSplineShape_GetVertices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSplineShape_SetVertices_Proxy( 
    IGcsSplineShape __RPC_FAR * This,
    /* [in] */ IGcsPointTab __RPC_FAR *pPoints);


void __RPC_STUB IGcsSplineShape_SetVertices_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsSplineShape_INTERFACE_DEFINED__ */


#ifndef __IGcsTransform_INTERFACE_DEFINED__
#define __IGcsTransform_INTERFACE_DEFINED__

/* interface IGcsTransform */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsTransform;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD74-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsTransform : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformStart( 
            /* [in] */ BSTR bstrSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformEnd( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Transform( 
            /* [out] */ float __RPC_FAR *x,
            /* [out] */ float __RPC_FAR *y,
            /* [out] */ float __RPC_FAR *z) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformPoints( 
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformNodes( 
            /* [in] */ IGcsNodeTab __RPC_FAR *pNodeTab) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformMesh( 
            /* [in] */ IGcsTriObject __RPC_FAR *pMesh) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE TransformShape( 
            /* [in] */ IGcsSplineShape __RPC_FAR *pShape) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InvTransformStart( 
            /* [in] */ BSTR bstrSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InvTransformEnd( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsTransformVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsTransform __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsTransform __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformStart )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ BSTR bstrSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformEnd )( 
            IGcsTransform __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Transform )( 
            IGcsTransform __RPC_FAR * This,
            /* [out] */ float __RPC_FAR *x,
            /* [out] */ float __RPC_FAR *y,
            /* [out] */ float __RPC_FAR *z);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformPoints )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ IGcsPointTab __RPC_FAR *pPoints);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformNodes )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ IGcsNodeTab __RPC_FAR *pNodeTab);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformMesh )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ IGcsTriObject __RPC_FAR *pMesh);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *TransformShape )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ IGcsSplineShape __RPC_FAR *pShape);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvTransformStart )( 
            IGcsTransform __RPC_FAR * This,
            /* [in] */ BSTR bstrSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InvTransformEnd )( 
            IGcsTransform __RPC_FAR * This);
        
        END_INTERFACE
    } IGcsTransformVtbl;

    interface IGcsTransform
    {
        CONST_VTBL struct IGcsTransformVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsTransform_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsTransform_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsTransform_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsTransform_TransformStart(This,bstrSource)	\
    (This)->lpVtbl -> TransformStart(This,bstrSource)

#define IGcsTransform_TransformEnd(This)	\
    (This)->lpVtbl -> TransformEnd(This)

#define IGcsTransform_Transform(This,x,y,z)	\
    (This)->lpVtbl -> Transform(This,x,y,z)

#define IGcsTransform_TransformPoints(This,pPoints)	\
    (This)->lpVtbl -> TransformPoints(This,pPoints)

#define IGcsTransform_TransformNodes(This,pNodeTab)	\
    (This)->lpVtbl -> TransformNodes(This,pNodeTab)

#define IGcsTransform_TransformMesh(This,pMesh)	\
    (This)->lpVtbl -> TransformMesh(This,pMesh)

#define IGcsTransform_TransformShape(This,pShape)	\
    (This)->lpVtbl -> TransformShape(This,pShape)

#define IGcsTransform_InvTransformStart(This,bstrSource)	\
    (This)->lpVtbl -> InvTransformStart(This,bstrSource)

#define IGcsTransform_InvTransformEnd(This)	\
    (This)->lpVtbl -> InvTransformEnd(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformStart_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ BSTR bstrSource);


void __RPC_STUB IGcsTransform_TransformStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformEnd_Proxy( 
    IGcsTransform __RPC_FAR * This);


void __RPC_STUB IGcsTransform_TransformEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_Transform_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [out] */ float __RPC_FAR *x,
    /* [out] */ float __RPC_FAR *y,
    /* [out] */ float __RPC_FAR *z);


void __RPC_STUB IGcsTransform_Transform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformPoints_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ IGcsPointTab __RPC_FAR *pPoints);


void __RPC_STUB IGcsTransform_TransformPoints_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformNodes_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ IGcsNodeTab __RPC_FAR *pNodeTab);


void __RPC_STUB IGcsTransform_TransformNodes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformMesh_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ IGcsTriObject __RPC_FAR *pMesh);


void __RPC_STUB IGcsTransform_TransformMesh_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_TransformShape_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ IGcsSplineShape __RPC_FAR *pShape);


void __RPC_STUB IGcsTransform_TransformShape_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_InvTransformStart_Proxy( 
    IGcsTransform __RPC_FAR * This,
    /* [in] */ BSTR bstrSource);


void __RPC_STUB IGcsTransform_InvTransformStart_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsTransform_InvTransformEnd_Proxy( 
    IGcsTransform __RPC_FAR * This);


void __RPC_STUB IGcsTransform_InvTransformEnd_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsTransform_INTERFACE_DEFINED__ */


#ifndef __IGcsUI_INTERFACE_DEFINED__
#define __IGcsUI_INTERFACE_DEFINED__

/* interface IGcsUI */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsUI;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD76-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsUI : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DoChangeDialog( 
            /* [in] */ HWND hDlg,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE DoDefineDialog( 
            /* [in] */ HWND hDlg,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsUIVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsUI __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsUI __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsUI __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoChangeDialog )( 
            IGcsUI __RPC_FAR * This,
            /* [in] */ HWND hDlg,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *DoDefineDialog )( 
            IGcsUI __RPC_FAR * This,
            /* [in] */ HWND hDlg,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } IGcsUIVtbl;

    interface IGcsUI
    {
        CONST_VTBL struct IGcsUIVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsUI_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsUI_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsUI_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsUI_DoChangeDialog(This,hDlg,ppNewSys,pbResult)	\
    (This)->lpVtbl -> DoChangeDialog(This,hDlg,ppNewSys,pbResult)

#define IGcsUI_DoDefineDialog(This,hDlg,ppNewSys,pbResult)	\
    (This)->lpVtbl -> DoDefineDialog(This,hDlg,ppNewSys,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsUI_DoChangeDialog_Proxy( 
    IGcsUI __RPC_FAR * This,
    /* [in] */ HWND hDlg,
    /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB IGcsUI_DoChangeDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsUI_DoDefineDialog_Proxy( 
    IGcsUI __RPC_FAR * This,
    /* [in] */ HWND hDlg,
    /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppNewSys,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB IGcsUI_DoDefineDialog_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsUI_INTERFACE_DEFINED__ */


#ifndef __IGcsLoad_INTERFACE_DEFINED__
#define __IGcsLoad_INTERFACE_DEFINED__

/* interface IGcsLoad */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsLoad;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD77-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsLoad : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenChunk( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CurChunkID( 
            /* [out] */ USHORT __RPC_FAR *psID) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CloseChunk( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Read( 
            /* [in] */ int iBuf,
            /* [in] */ ULONG lBytes,
            /* [out] */ ULONG __RPC_FAR *plRead) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ReadWStringChunk( 
            /* [out] */ BSTR __RPC_FAR *pbstrString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ReadCStringChunk( 
            /* [out] */ BSTR __RPC_FAR *pbstrString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetObsolete( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetILoadPointer( 
            /* [in] */ INT_PTR iPointer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsLoadVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsLoad __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsLoad __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsLoad __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenChunk )( 
            IGcsLoad __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CurChunkID )( 
            IGcsLoad __RPC_FAR * This,
            /* [out] */ USHORT __RPC_FAR *psID);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseChunk )( 
            IGcsLoad __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Read )( 
            IGcsLoad __RPC_FAR * This,
            /* [in] */ int iBuf,
            /* [in] */ ULONG lBytes,
            /* [out] */ ULONG __RPC_FAR *plRead);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadWStringChunk )( 
            IGcsLoad __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReadCStringChunk )( 
            IGcsLoad __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetObsolete )( 
            IGcsLoad __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetILoadPointer )( 
            IGcsLoad __RPC_FAR * This,
            /* [in] */ INT_PTR iPointer);
        
        END_INTERFACE
    } IGcsLoadVtbl;

    interface IGcsLoad
    {
        CONST_VTBL struct IGcsLoadVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsLoad_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsLoad_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsLoad_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsLoad_OpenChunk(This)	\
    (This)->lpVtbl -> OpenChunk(This)

#define IGcsLoad_CurChunkID(This,psID)	\
    (This)->lpVtbl -> CurChunkID(This,psID)

#define IGcsLoad_CloseChunk(This)	\
    (This)->lpVtbl -> CloseChunk(This)

#define IGcsLoad_Read(This,iBuf,lBytes,plRead)	\
    (This)->lpVtbl -> Read(This,iBuf,lBytes,plRead)

#define IGcsLoad_ReadWStringChunk(This,pbstrString)	\
    (This)->lpVtbl -> ReadWStringChunk(This,pbstrString)

#define IGcsLoad_ReadCStringChunk(This,pbstrString)	\
    (This)->lpVtbl -> ReadCStringChunk(This,pbstrString)

#define IGcsLoad_SetObsolete(This)	\
    (This)->lpVtbl -> SetObsolete(This)

#define IGcsLoad_SetILoadPointer(This,iPointer)	\
    (This)->lpVtbl -> SetILoadPointer(This,iPointer)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_OpenChunk_Proxy( 
    IGcsLoad __RPC_FAR * This);


void __RPC_STUB IGcsLoad_OpenChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_CurChunkID_Proxy( 
    IGcsLoad __RPC_FAR * This,
    /* [out] */ USHORT __RPC_FAR *psID);


void __RPC_STUB IGcsLoad_CurChunkID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_CloseChunk_Proxy( 
    IGcsLoad __RPC_FAR * This);


void __RPC_STUB IGcsLoad_CloseChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_Read_Proxy( 
    IGcsLoad __RPC_FAR * This,
    /* [in] */ int iBuf,
    /* [in] */ ULONG lBytes,
    /* [out] */ ULONG __RPC_FAR *plRead);


void __RPC_STUB IGcsLoad_Read_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_ReadWStringChunk_Proxy( 
    IGcsLoad __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrString);


void __RPC_STUB IGcsLoad_ReadWStringChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_ReadCStringChunk_Proxy( 
    IGcsLoad __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrString);


void __RPC_STUB IGcsLoad_ReadCStringChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_SetObsolete_Proxy( 
    IGcsLoad __RPC_FAR * This);


void __RPC_STUB IGcsLoad_SetObsolete_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsLoad_SetILoadPointer_Proxy( 
    IGcsLoad __RPC_FAR * This,
    /* [in] */ INT_PTR iPointer);


void __RPC_STUB IGcsLoad_SetILoadPointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsLoad_INTERFACE_DEFINED__ */


#ifndef __IGcsSave_INTERFACE_DEFINED__
#define __IGcsSave_INTERFACE_DEFINED__

/* interface IGcsSave */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsSave;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD78-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsSave : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE BeginChunk( 
            /* [in] */ USHORT sId) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EndChunk( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Write( 
            /* [in] */ int iBuf,
            /* [in] */ ULONG lBytes,
            /* [out] */ ULONG __RPC_FAR *plWrit) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE WriteWString( 
            /* [in] */ BSTR bstrString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE WriteCString( 
            /* [in] */ BSTR bstrString) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetISavePointer( 
            /* [in] */ INT_PTR iPointer) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsSaveVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsSave __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsSave __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *BeginChunk )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ USHORT sId);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EndChunk )( 
            IGcsSave __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ int iBuf,
            /* [in] */ ULONG lBytes,
            /* [out] */ ULONG __RPC_FAR *plWrit);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteWString )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ BSTR bstrString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteCString )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ BSTR bstrString);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetISavePointer )( 
            IGcsSave __RPC_FAR * This,
            /* [in] */ INT_PTR iPointer);
        
        END_INTERFACE
    } IGcsSaveVtbl;

    interface IGcsSave
    {
        CONST_VTBL struct IGcsSaveVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsSave_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsSave_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsSave_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsSave_BeginChunk(This,sId)	\
    (This)->lpVtbl -> BeginChunk(This,sId)

#define IGcsSave_EndChunk(This)	\
    (This)->lpVtbl -> EndChunk(This)

#define IGcsSave_Write(This,iBuf,lBytes,plWrit)	\
    (This)->lpVtbl -> Write(This,iBuf,lBytes,plWrit)

#define IGcsSave_WriteWString(This,bstrString)	\
    (This)->lpVtbl -> WriteWString(This,bstrString)

#define IGcsSave_WriteCString(This,bstrString)	\
    (This)->lpVtbl -> WriteCString(This,bstrString)

#define IGcsSave_SetISavePointer(This,iPointer)	\
    (This)->lpVtbl -> SetISavePointer(This,iPointer)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_BeginChunk_Proxy( 
    IGcsSave __RPC_FAR * This,
    /* [in] */ USHORT sId);


void __RPC_STUB IGcsSave_BeginChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_EndChunk_Proxy( 
    IGcsSave __RPC_FAR * This);


void __RPC_STUB IGcsSave_EndChunk_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_Write_Proxy( 
    IGcsSave __RPC_FAR * This,
    /* [in] */ int iBuf,
    /* [in] */ ULONG lBytes,
    /* [out] */ ULONG __RPC_FAR *plWrit);


void __RPC_STUB IGcsSave_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_WriteWString_Proxy( 
    IGcsSave __RPC_FAR * This,
    /* [in] */ BSTR bstrString);


void __RPC_STUB IGcsSave_WriteWString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_WriteCString_Proxy( 
    IGcsSave __RPC_FAR * This,
    /* [in] */ BSTR bstrString);


void __RPC_STUB IGcsSave_WriteCString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSave_SetISavePointer_Proxy( 
    IGcsSave __RPC_FAR * This,
    /* [in] */ INT_PTR iPointer);


void __RPC_STUB IGcsSave_SetISavePointer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsSave_INTERFACE_DEFINED__ */


#ifndef __IGcsSession_INTERFACE_DEFINED__
#define __IGcsSession_INTERFACE_DEFINED__

/* interface IGcsSession */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IGcsSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("90F5DD79-E2F6-11d2-A9FE-0060B057DAFB")
    IGcsSession : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGlobalCoordSystem( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGlobalCoordSystemByName( 
            /* [in] */ BSTR bstrSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGlobalCoordSystem( 
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppCoordSys) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGlobalCoordSystemName( 
            /* [out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGlobalCoordSystemDescription( 
            /* [out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FindCoordSystem( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys,
            /* [out] */ BOOL __RPC_FAR *pbFound) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsEnabled( 
            /* [out] */ BOOL __RPC_FAR *pbEnabled) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetEnabled( 
            /* [in] */ BOOL bEnable) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Load( 
            /* [in] */ IGcsLoad __RPC_FAR *pLoad) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Save( 
            /* [in] */ IGcsSave __RPC_FAR *pSave) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTransformer( 
            /* [out] */ IGcsTransform __RPC_FAR *__RPC_FAR *ppTransformer) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUI( 
            /* [out] */ IGcsUI __RPC_FAR *__RPC_FAR *ppUi) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGcsSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IGcsSession __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IGcsSession __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGlobalCoordSystem )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGlobalCoordSystemByName )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ BSTR bstrSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGlobalCoordSystem )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppCoordSys);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGlobalCoordSystemName )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGlobalCoordSystemDescription )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FindCoordSystem )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys,
            /* [out] */ BOOL __RPC_FAR *pbFound);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEnabled )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbEnabled);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEnabled )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ BOOL bEnable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ IGcsLoad __RPC_FAR *pLoad);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            IGcsSession __RPC_FAR * This,
            /* [in] */ IGcsSave __RPC_FAR *pSave);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransformer )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ IGcsTransform __RPC_FAR *__RPC_FAR *ppTransformer);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUI )( 
            IGcsSession __RPC_FAR * This,
            /* [out] */ IGcsUI __RPC_FAR *__RPC_FAR *ppUi);
        
        END_INTERFACE
    } IGcsSessionVtbl;

    interface IGcsSession
    {
        CONST_VTBL struct IGcsSessionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGcsSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGcsSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGcsSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGcsSession_SetGlobalCoordSystem(This,pCoordSys)	\
    (This)->lpVtbl -> SetGlobalCoordSystem(This,pCoordSys)

#define IGcsSession_SetGlobalCoordSystemByName(This,bstrSource)	\
    (This)->lpVtbl -> SetGlobalCoordSystemByName(This,bstrSource)

#define IGcsSession_GetGlobalCoordSystem(This,ppCoordSys)	\
    (This)->lpVtbl -> GetGlobalCoordSystem(This,ppCoordSys)

#define IGcsSession_GetGlobalCoordSystemName(This,pbstrName)	\
    (This)->lpVtbl -> GetGlobalCoordSystemName(This,pbstrName)

#define IGcsSession_GetGlobalCoordSystemDescription(This,pbstrName)	\
    (This)->lpVtbl -> GetGlobalCoordSystemDescription(This,pbstrName)

#define IGcsSession_FindCoordSystem(This,pCoordSys,pbFound)	\
    (This)->lpVtbl -> FindCoordSystem(This,pCoordSys,pbFound)

#define IGcsSession_IsEnabled(This,pbEnabled)	\
    (This)->lpVtbl -> IsEnabled(This,pbEnabled)

#define IGcsSession_SetEnabled(This,bEnable)	\
    (This)->lpVtbl -> SetEnabled(This,bEnable)

#define IGcsSession_Load(This,pLoad)	\
    (This)->lpVtbl -> Load(This,pLoad)

#define IGcsSession_Save(This,pSave)	\
    (This)->lpVtbl -> Save(This,pSave)

#define IGcsSession_GetTransformer(This,ppTransformer)	\
    (This)->lpVtbl -> GetTransformer(This,ppTransformer)

#define IGcsSession_GetUI(This,ppUi)	\
    (This)->lpVtbl -> GetUI(This,ppUi)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_SetGlobalCoordSystem_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys);


void __RPC_STUB IGcsSession_SetGlobalCoordSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_SetGlobalCoordSystemByName_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ BSTR bstrSource);


void __RPC_STUB IGcsSession_SetGlobalCoordSystemByName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_GetGlobalCoordSystem_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppCoordSys);


void __RPC_STUB IGcsSession_GetGlobalCoordSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_GetGlobalCoordSystemName_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IGcsSession_GetGlobalCoordSystemName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_GetGlobalCoordSystemDescription_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB IGcsSession_GetGlobalCoordSystemDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_FindCoordSystem_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordSys,
    /* [out] */ BOOL __RPC_FAR *pbFound);


void __RPC_STUB IGcsSession_FindCoordSystem_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_IsEnabled_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbEnabled);


void __RPC_STUB IGcsSession_IsEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_SetEnabled_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ BOOL bEnable);


void __RPC_STUB IGcsSession_SetEnabled_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_Load_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ IGcsLoad __RPC_FAR *pLoad);


void __RPC_STUB IGcsSession_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_Save_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [in] */ IGcsSave __RPC_FAR *pSave);


void __RPC_STUB IGcsSession_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_GetTransformer_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ IGcsTransform __RPC_FAR *__RPC_FAR *ppTransformer);


void __RPC_STUB IGcsSession_GetTransformer_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE IGcsSession_GetUI_Proxy( 
    IGcsSession __RPC_FAR * This,
    /* [out] */ IGcsUI __RPC_FAR *__RPC_FAR *ppUi);


void __RPC_STUB IGcsSession_GetUI_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGcsSession_INTERFACE_DEFINED__ */



#ifndef __GCSAPILib_LIBRARY_DEFINED__
#define __GCSAPILib_LIBRARY_DEFINED__

/* library GCSAPILib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_GCSAPILib;

EXTERN_C const CLSID CLSID_GcsSession;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1D5-E309-11d2-A9FE-0060B057DAFB")
GcsSession;
#endif

EXTERN_C const CLSID CLSID_GcsPointTab;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1D8-E309-11d2-A9FE-0060B057DAFB")
GcsPointTab;
#endif

EXTERN_C const CLSID CLSID_GcsNodeTab;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1D9-E309-11d2-A9FE-0060B057DAFB")
GcsNodeTab;
#endif

EXTERN_C const CLSID CLSID_GcsTriObject;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1DA-E309-11d2-A9FE-0060B057DAFB")
GcsTriObject;
#endif

EXTERN_C const CLSID CLSID_GcsSplineShape;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1DB-E309-11d2-A9FE-0060B057DAFB")
GcsSplineShape;
#endif

EXTERN_C const CLSID CLSID_GcsLoad;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1DC-E309-11d2-A9FE-0060B057DAFB")
GcsLoad;
#endif

EXTERN_C const CLSID CLSID_GcsSave;

#ifdef __cplusplus

class DECLSPEC_UUID("CEFFE1DD-E309-11d2-A9FE-0060B057DAFB")
GcsSave;
#endif

EXTERN_C const CLSID CLSID_GcsBitmap;

#ifdef __cplusplus

class DECLSPEC_UUID("189618F1-F8FD-11d2-AA0F-0060B057DAFB")
GcsBitmap;
#endif
#endif /* __GCSAPILib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long __RPC_FAR *, unsigned long            , HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  HWND_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, HWND __RPC_FAR * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long __RPC_FAR *, HWND __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif