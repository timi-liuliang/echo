/* this ALWAYS GENERATED file contains the definitions for the interfaces */
#pragma once

/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Apr 19 12:18:33 1999
 */
/* Compiler settings for ..\..\..\CSLib\Src\csapi.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


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

#ifndef __ICsSession_FWD_DEFINED__
#define __ICsSession_FWD_DEFINED__
typedef interface ICsSession ICsSession;
#endif 	/* __ICsSession_FWD_DEFINED__ */


#ifndef __ICsDictionary_FWD_DEFINED__
#define __ICsDictionary_FWD_DEFINED__
typedef interface ICsDictionary ICsDictionary;
#endif 	/* __ICsDictionary_FWD_DEFINED__ */


#ifndef __ICsCoordConvert_FWD_DEFINED__
#define __ICsCoordConvert_FWD_DEFINED__
typedef interface ICsCoordConvert ICsCoordConvert;
#endif 	/* __ICsCoordConvert_FWD_DEFINED__ */


#ifndef __ICsDatumShift_FWD_DEFINED__
#define __ICsDatumShift_FWD_DEFINED__
typedef interface ICsDatumShift ICsDatumShift;
#endif 	/* __ICsDatumShift_FWD_DEFINED__ */


#ifndef __ICsCoordsys_FWD_DEFINED__
#define __ICsCoordsys_FWD_DEFINED__
typedef interface ICsCoordsys ICsCoordsys;
#endif 	/* __ICsCoordsys_FWD_DEFINED__ */


#ifndef __ICsDef_FWD_DEFINED__
#define __ICsDef_FWD_DEFINED__
typedef interface ICsDef ICsDef;
#endif 	/* __ICsDef_FWD_DEFINED__ */


#ifndef __ICsMentorDef_FWD_DEFINED__
#define __ICsMentorDef_FWD_DEFINED__
typedef interface ICsMentorDef ICsMentorDef;
#endif 	/* __ICsMentorDef_FWD_DEFINED__ */


#ifndef __ICsCoordsysDef_FWD_DEFINED__
#define __ICsCoordsysDef_FWD_DEFINED__
typedef interface ICsCoordsysDef ICsCoordsysDef;
#endif 	/* __ICsCoordsysDef_FWD_DEFINED__ */


#ifndef __ICsDatum_FWD_DEFINED__
#define __ICsDatum_FWD_DEFINED__
typedef interface ICsDatum ICsDatum;
#endif 	/* __ICsDatum_FWD_DEFINED__ */


#ifndef __ICsEllipsoidDef_FWD_DEFINED__
#define __ICsEllipsoidDef_FWD_DEFINED__
typedef interface ICsEllipsoidDef ICsEllipsoidDef;
#endif 	/* __ICsEllipsoidDef_FWD_DEFINED__ */


#ifndef __ICsDatumDef_FWD_DEFINED__
#define __ICsDatumDef_FWD_DEFINED__
typedef interface ICsDatumDef ICsDatumDef;
#endif 	/* __ICsDatumDef_FWD_DEFINED__ */


#ifndef __ICsCategoryDef_FWD_DEFINED__
#define __ICsCategoryDef_FWD_DEFINED__
typedef interface ICsCategoryDef ICsCategoryDef;
#endif 	/* __ICsCategoryDef_FWD_DEFINED__ */


#ifndef __ICsEnumName_FWD_DEFINED__
#define __ICsEnumName_FWD_DEFINED__
typedef interface ICsEnumName ICsEnumName;
#endif 	/* __ICsEnumName_FWD_DEFINED__ */


#ifndef __ICsEnumDword_FWD_DEFINED__
#define __ICsEnumDword_FWD_DEFINED__
typedef interface ICsEnumDword ICsEnumDword;
#endif 	/* __ICsEnumDword_FWD_DEFINED__ */


#ifndef __ICsDefSet_FWD_DEFINED__
#define __ICsDefSet_FWD_DEFINED__
typedef interface ICsDefSet ICsDefSet;
#endif 	/* __ICsDefSet_FWD_DEFINED__ */


#ifndef __ICsSelector_FWD_DEFINED__
#define __ICsSelector_FWD_DEFINED__
typedef interface ICsSelector ICsSelector;
#endif 	/* __ICsSelector_FWD_DEFINED__ */


#ifndef __ICsMentorSelector_FWD_DEFINED__
#define __ICsMentorSelector_FWD_DEFINED__
typedef interface ICsMentorSelector ICsMentorSelector;
#endif 	/* __ICsMentorSelector_FWD_DEFINED__ */


#ifndef __ICsEnum_FWD_DEFINED__
#define __ICsEnum_FWD_DEFINED__
typedef interface ICsEnum ICsEnum;
#endif 	/* __ICsEnum_FWD_DEFINED__ */


#ifndef __ICsEnumMentor_FWD_DEFINED__
#define __ICsEnumMentor_FWD_DEFINED__
typedef interface ICsEnumMentor ICsEnumMentor;
#endif 	/* __ICsEnumMentor_FWD_DEFINED__ */


#ifndef __ICsEllipsoidDefSet_FWD_DEFINED__
#define __ICsEllipsoidDefSet_FWD_DEFINED__
typedef interface ICsEllipsoidDefSet ICsEllipsoidDefSet;
#endif 	/* __ICsEllipsoidDefSet_FWD_DEFINED__ */


#ifndef __ICsEllipsoidSelector_FWD_DEFINED__
#define __ICsEllipsoidSelector_FWD_DEFINED__
typedef interface ICsEllipsoidSelector ICsEllipsoidSelector;
#endif 	/* __ICsEllipsoidSelector_FWD_DEFINED__ */


#ifndef __ICsEnumEllipsoid_FWD_DEFINED__
#define __ICsEnumEllipsoid_FWD_DEFINED__
typedef interface ICsEnumEllipsoid ICsEnumEllipsoid;
#endif 	/* __ICsEnumEllipsoid_FWD_DEFINED__ */


#ifndef __ICsDatumDefSet_FWD_DEFINED__
#define __ICsDatumDefSet_FWD_DEFINED__
typedef interface ICsDatumDefSet ICsDatumDefSet;
#endif 	/* __ICsDatumDefSet_FWD_DEFINED__ */


#ifndef __ICsDatumSelector_FWD_DEFINED__
#define __ICsDatumSelector_FWD_DEFINED__
typedef interface ICsDatumSelector ICsDatumSelector;
#endif 	/* __ICsDatumSelector_FWD_DEFINED__ */


#ifndef __ICsEnumDatum_FWD_DEFINED__
#define __ICsEnumDatum_FWD_DEFINED__
typedef interface ICsEnumDatum ICsEnumDatum;
#endif 	/* __ICsEnumDatum_FWD_DEFINED__ */


#ifndef __ICsCoordsysDefSet_FWD_DEFINED__
#define __ICsCoordsysDefSet_FWD_DEFINED__
typedef interface ICsCoordsysDefSet ICsCoordsysDefSet;
#endif 	/* __ICsCoordsysDefSet_FWD_DEFINED__ */


#ifndef __ICsCoordsysSelector_FWD_DEFINED__
#define __ICsCoordsysSelector_FWD_DEFINED__
typedef interface ICsCoordsysSelector ICsCoordsysSelector;
#endif 	/* __ICsCoordsysSelector_FWD_DEFINED__ */


#ifndef __ICsEnumCoordsys_FWD_DEFINED__
#define __ICsEnumCoordsys_FWD_DEFINED__
typedef interface ICsEnumCoordsys ICsEnumCoordsys;
#endif 	/* __ICsEnumCoordsys_FWD_DEFINED__ */


#ifndef __ICsCategoryDefSet_FWD_DEFINED__
#define __ICsCategoryDefSet_FWD_DEFINED__
typedef interface ICsCategoryDefSet ICsCategoryDefSet;
#endif 	/* __ICsCategoryDefSet_FWD_DEFINED__ */


#ifndef __ICsCategorySelector_FWD_DEFINED__
#define __ICsCategorySelector_FWD_DEFINED__
typedef interface ICsCategorySelector ICsCategorySelector;
#endif 	/* __ICsCategorySelector_FWD_DEFINED__ */


#ifndef __ICsEnumCategory_FWD_DEFINED__
#define __ICsEnumCategory_FWD_DEFINED__
typedef interface ICsEnumCategory ICsEnumCategory;
#endif 	/* __ICsEnumCategory_FWD_DEFINED__ */


#ifndef __ICsEnumUnit_FWD_DEFINED__
#define __ICsEnumUnit_FWD_DEFINED__
typedef interface ICsEnumUnit ICsEnumUnit;
#endif 	/* __ICsEnumUnit_FWD_DEFINED__ */


#ifndef __ICsEnumProjection_FWD_DEFINED__
#define __ICsEnumProjection_FWD_DEFINED__
typedef interface ICsEnumProjection ICsEnumProjection;
#endif 	/* __ICsEnumProjection_FWD_DEFINED__ */


#ifndef __ICsUnitInfo_FWD_DEFINED__
#define __ICsUnitInfo_FWD_DEFINED__
typedef interface ICsUnitInfo ICsUnitInfo;
#endif 	/* __ICsUnitInfo_FWD_DEFINED__ */


#ifndef __ICsProjectionInfo_FWD_DEFINED__
#define __ICsProjectionInfo_FWD_DEFINED__
typedef interface ICsProjectionInfo ICsProjectionInfo;
#endif 	/* __ICsProjectionInfo_FWD_DEFINED__ */


#ifndef __ICsDatumInfo_FWD_DEFINED__
#define __ICsDatumInfo_FWD_DEFINED__
typedef interface ICsDatumInfo ICsDatumInfo;
#endif 	/* __ICsDatumInfo_FWD_DEFINED__ */


#ifndef __ICsEllipsoidInfo_FWD_DEFINED__
#define __ICsEllipsoidInfo_FWD_DEFINED__
typedef interface ICsEllipsoidInfo ICsEllipsoidInfo;
#endif 	/* __ICsEllipsoidInfo_FWD_DEFINED__ */


#ifndef __ICsDictionaryUtility_FWD_DEFINED__
#define __ICsDictionaryUtility_FWD_DEFINED__
typedef interface ICsDictionaryUtility ICsDictionaryUtility;
#endif 	/* __ICsDictionaryUtility_FWD_DEFINED__ */


#ifndef __ICsPersistStream_FWD_DEFINED__
#define __ICsPersistStream_FWD_DEFINED__
typedef interface ICsPersistStream ICsPersistStream;
#endif 	/* __ICsPersistStream_FWD_DEFINED__ */


#ifndef __CsSession_FWD_DEFINED__
#define __CsSession_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsSession CsSession;
#else
typedef struct CsSession CsSession;
#endif /* __cplusplus */

#endif 	/* __CsSession_FWD_DEFINED__ */


#ifndef __CsEllipsoidDictionary_FWD_DEFINED__
#define __CsEllipsoidDictionary_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEllipsoidDictionary CsEllipsoidDictionary;
#else
typedef struct CsEllipsoidDictionary CsEllipsoidDictionary;
#endif /* __cplusplus */

#endif 	/* __CsEllipsoidDictionary_FWD_DEFINED__ */


#ifndef __CsDatumDictionary_FWD_DEFINED__
#define __CsDatumDictionary_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDatumDictionary CsDatumDictionary;
#else
typedef struct CsDatumDictionary CsDatumDictionary;
#endif /* __cplusplus */

#endif 	/* __CsDatumDictionary_FWD_DEFINED__ */


#ifndef __CsCoordsysDictionary_FWD_DEFINED__
#define __CsCoordsysDictionary_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCoordsysDictionary CsCoordsysDictionary;
#else
typedef struct CsCoordsysDictionary CsCoordsysDictionary;
#endif /* __cplusplus */

#endif 	/* __CsCoordsysDictionary_FWD_DEFINED__ */


#ifndef __CsCategoryDictionary_FWD_DEFINED__
#define __CsCategoryDictionary_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCategoryDictionary CsCategoryDictionary;
#else
typedef struct CsCategoryDictionary CsCategoryDictionary;
#endif /* __cplusplus */

#endif 	/* __CsCategoryDictionary_FWD_DEFINED__ */


#ifndef __CsCoordConvert_FWD_DEFINED__
#define __CsCoordConvert_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCoordConvert CsCoordConvert;
#else
typedef struct CsCoordConvert CsCoordConvert;
#endif /* __cplusplus */

#endif 	/* __CsCoordConvert_FWD_DEFINED__ */


#ifndef __CsDatumShift_FWD_DEFINED__
#define __CsDatumShift_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDatumShift CsDatumShift;
#else
typedef struct CsDatumShift CsDatumShift;
#endif /* __cplusplus */

#endif 	/* __CsDatumShift_FWD_DEFINED__ */


#ifndef __CsCoordsys_FWD_DEFINED__
#define __CsCoordsys_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCoordsys CsCoordsys;
#else
typedef struct CsCoordsys CsCoordsys;
#endif /* __cplusplus */

#endif 	/* __CsCoordsys_FWD_DEFINED__ */


#ifndef __CsCoordsysDef_FWD_DEFINED__
#define __CsCoordsysDef_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCoordsysDef CsCoordsysDef;
#else
typedef struct CsCoordsysDef CsCoordsysDef;
#endif /* __cplusplus */

#endif 	/* __CsCoordsysDef_FWD_DEFINED__ */


#ifndef __CsDatum_FWD_DEFINED__
#define __CsDatum_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDatum CsDatum;
#else
typedef struct CsDatum CsDatum;
#endif /* __cplusplus */

#endif 	/* __CsDatum_FWD_DEFINED__ */


#ifndef __CsEllipsoidDef_FWD_DEFINED__
#define __CsEllipsoidDef_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEllipsoidDef CsEllipsoidDef;
#else
typedef struct CsEllipsoidDef CsEllipsoidDef;
#endif /* __cplusplus */

#endif 	/* __CsEllipsoidDef_FWD_DEFINED__ */


#ifndef __CsDatumDef_FWD_DEFINED__
#define __CsDatumDef_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDatumDef CsDatumDef;
#else
typedef struct CsDatumDef CsDatumDef;
#endif /* __cplusplus */

#endif 	/* __CsDatumDef_FWD_DEFINED__ */


#ifndef __CsCategoryDef_FWD_DEFINED__
#define __CsCategoryDef_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCategoryDef CsCategoryDef;
#else
typedef struct CsCategoryDef CsCategoryDef;
#endif /* __cplusplus */

#endif 	/* __CsCategoryDef_FWD_DEFINED__ */


#ifndef __CsEnumCategoryCoordsys_FWD_DEFINED__
#define __CsEnumCategoryCoordsys_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumCategoryCoordsys CsEnumCategoryCoordsys;
#else
typedef struct CsEnumCategoryCoordsys CsEnumCategoryCoordsys;
#endif /* __cplusplus */

#endif 	/* __CsEnumCategoryCoordsys_FWD_DEFINED__ */


#ifndef __CsEnumDword_FWD_DEFINED__
#define __CsEnumDword_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDword CsEnumDword;
#else
typedef struct CsEnumDword CsEnumDword;
#endif /* __cplusplus */

#endif 	/* __CsEnumDword_FWD_DEFINED__ */


#ifndef __CsEllipsoidDefSet_FWD_DEFINED__
#define __CsEllipsoidDefSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEllipsoidDefSet CsEllipsoidDefSet;
#else
typedef struct CsEllipsoidDefSet CsEllipsoidDefSet;
#endif /* __cplusplus */

#endif 	/* __CsEllipsoidDefSet_FWD_DEFINED__ */


#ifndef __CsEnumEllipsoid_FWD_DEFINED__
#define __CsEnumEllipsoid_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumEllipsoid CsEnumEllipsoid;
#else
typedef struct CsEnumEllipsoid CsEnumEllipsoid;
#endif /* __cplusplus */

#endif 	/* __CsEnumEllipsoid_FWD_DEFINED__ */


#ifndef __CsDatumDefSet_FWD_DEFINED__
#define __CsDatumDefSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDatumDefSet CsDatumDefSet;
#else
typedef struct CsDatumDefSet CsDatumDefSet;
#endif /* __cplusplus */

#endif 	/* __CsDatumDefSet_FWD_DEFINED__ */


#ifndef __CsEnumDatum_FWD_DEFINED__
#define __CsEnumDatum_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDatum CsEnumDatum;
#else
typedef struct CsEnumDatum CsEnumDatum;
#endif /* __cplusplus */

#endif 	/* __CsEnumDatum_FWD_DEFINED__ */


#ifndef __CsCoordsysDefSet_FWD_DEFINED__
#define __CsCoordsysDefSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCoordsysDefSet CsCoordsysDefSet;
#else
typedef struct CsCoordsysDefSet CsCoordsysDefSet;
#endif /* __cplusplus */

#endif 	/* __CsCoordsysDefSet_FWD_DEFINED__ */


#ifndef __CsEnumCoordsys_FWD_DEFINED__
#define __CsEnumCoordsys_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumCoordsys CsEnumCoordsys;
#else
typedef struct CsEnumCoordsys CsEnumCoordsys;
#endif /* __cplusplus */

#endif 	/* __CsEnumCoordsys_FWD_DEFINED__ */


#ifndef __CsCategoryDefSet_FWD_DEFINED__
#define __CsCategoryDefSet_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsCategoryDefSet CsCategoryDefSet;
#else
typedef struct CsCategoryDefSet CsCategoryDefSet;
#endif /* __cplusplus */

#endif 	/* __CsCategoryDefSet_FWD_DEFINED__ */


#ifndef __CsEnumCategory_FWD_DEFINED__
#define __CsEnumCategory_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumCategory CsEnumCategory;
#else
typedef struct CsEnumCategory CsEnumCategory;
#endif /* __cplusplus */

#endif 	/* __CsEnumCategory_FWD_DEFINED__ */


#ifndef __CsEnumUnit_FWD_DEFINED__
#define __CsEnumUnit_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumUnit CsEnumUnit;
#else
typedef struct CsEnumUnit CsEnumUnit;
#endif /* __cplusplus */

#endif 	/* __CsEnumUnit_FWD_DEFINED__ */


#ifndef __CsEnumProjection_FWD_DEFINED__
#define __CsEnumProjection_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumProjection CsEnumProjection;
#else
typedef struct CsEnumProjection CsEnumProjection;
#endif /* __cplusplus */

#endif 	/* __CsEnumProjection_FWD_DEFINED__ */


#ifndef __CsUnitInfo_FWD_DEFINED__
#define __CsUnitInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsUnitInfo CsUnitInfo;
#else
typedef struct CsUnitInfo CsUnitInfo;
#endif /* __cplusplus */

#endif 	/* __CsUnitInfo_FWD_DEFINED__ */


#ifndef __CsProjectionInfo_FWD_DEFINED__
#define __CsProjectionInfo_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsProjectionInfo CsProjectionInfo;
#else
typedef struct CsProjectionInfo CsProjectionInfo;
#endif /* __cplusplus */

#endif 	/* __CsProjectionInfo_FWD_DEFINED__ */


#ifndef __CsDictionaryUtility_FWD_DEFINED__
#define __CsDictionaryUtility_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsDictionaryUtility CsDictionaryUtility;
#else
typedef struct CsDictionaryUtility CsDictionaryUtility;
#endif /* __cplusplus */

#endif 	/* __CsDictionaryUtility_FWD_DEFINED__ */


#ifndef __CsEnumDictEllipsoid_FWD_DEFINED__
#define __CsEnumDictEllipsoid_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDictEllipsoid CsEnumDictEllipsoid;
#else
typedef struct CsEnumDictEllipsoid CsEnumDictEllipsoid;
#endif /* __cplusplus */

#endif 	/* __CsEnumDictEllipsoid_FWD_DEFINED__ */


#ifndef __CsEnumDictDatum_FWD_DEFINED__
#define __CsEnumDictDatum_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDictDatum CsEnumDictDatum;
#else
typedef struct CsEnumDictDatum CsEnumDictDatum;
#endif /* __cplusplus */

#endif 	/* __CsEnumDictDatum_FWD_DEFINED__ */


#ifndef __CsEnumDictCoordsys_FWD_DEFINED__
#define __CsEnumDictCoordsys_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDictCoordsys CsEnumDictCoordsys;
#else
typedef struct CsEnumDictCoordsys CsEnumDictCoordsys;
#endif /* __cplusplus */

#endif 	/* __CsEnumDictCoordsys_FWD_DEFINED__ */


#ifndef __CsEnumDictCategory_FWD_DEFINED__
#define __CsEnumDictCategory_FWD_DEFINED__

#ifdef __cplusplus
typedef class CsEnumDictCategory CsEnumDictCategory;
#else
typedef struct CsEnumDictCategory CsEnumDictCategory;
#endif /* __cplusplus */

#endif 	/* __CsEnumDictCategory_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_csapi_0000 */
/* [local] */ 

typedef /* [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0001
    {	eCsOk	= 0,
	eCsWarning	= 1,
	eCsError	= 2,
	eCsFatal	= 3,
	eCsInvalidArgument	= 4,
	eCsNotFound	= 5,
	eCsDuplicate	= 6,
	eCsReadOnly	= 7,
	eCsProtected	= 8,
	eCsLocking	= 9,
	eCsMismatch	= 10,
	eCsMentor	= 11,
	eCsErrorOpenDictionary	= 12,
	eCsNotReady	= 13,
	eCsFileErr	= 14,
	eCsUnknownErr	= 15
    }	CsErr;

typedef /* [public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0002
    {	kCsClosed	= 0,
	kCsOpenForRead	= 1,
	kCsOpenForWrite	= 2
    }	CsOpenStatus;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0003
    {	kCsUnitTypeUnknown	= 0,
	kCsUnitTypeLinear	= 1,
	kCsUnitTypeAngular	= 2
    }	CsUnitType;

typedef /* [public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0004
    {	kCsUnitMeter	= 1,
	kCsUnitFoot	= 2,
	kCsUnitInch	= 3,
	kCsUnitIFoot	= 4,
	kCsUnitClarkeFoot	= 5,
	kCsUnitIInch	= 6,
	kCsUnitCentimeter	= 7,
	kCsUnitKilometer	= 8,
	kCsUnitYard	= 9,
	kCsUnitSearsYard	= 10,
	kCsUnitMile	= 11,
	kCsUnitIYard	= 12,
	kCsUnitIMile	= 13,
	kCsUnitKnot	= 14,
	kCsUnitNautM	= 15,
	kCsUnitLat66	= 16,
	kCsUnitLat83	= 17,
	kCsUnitDecimeter	= 18,
	kCsUnitMillimeter	= 19,
	kCsUnitDekameter	= 20,
	kCsUnitHectometer	= 21,
	kCsUnitGermanMeter	= 22,
	kCsUnitCaGrid	= 23,
	kCsUnitClarkeChain	= 24,
	kCsUnitGunterChain	= 25,
	kCsUnitBenoitChain	= 26,
	kCsUnitSearsChain	= 27,
	kCsUnitClarkeLink	= 28,
	kCsUnitGunterLink	= 29,
	kCsUnitBenoitLink	= 30,
	kCsUnitSearsLink	= 31,
	kCsUnitRod	= 32,
	kCsUnitPerch	= 33,
	kCsUnitPole	= 34,
	kCsUnitFurlong	= 35,
	kCsUnitRood	= 36,
	kCsUnitCapeFoot	= 37,
	kCsUnitBrealey	= 38,
	kCsUnitDegree	= 1001,
	kCsUnitGrad	= 1002,
	kCsUnitGrade	= 1003,
	kCsUnitMapInfo	= 1004,
	kCsUnitMil	= 1005,
	kCsUnitMinute	= 1006,
	kCsUnitRadian	= 1007,
	kCsUnitSecond	= 1008,
	kCsUnitDecisec	= 1009,
	kCsUnitCentisec	= 1010,
	kCsUnitMillisec	= 1011,
	kCsUnitUnknown	= 0
    }	CsUnit;

typedef /* [public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0005
    {	kCsPrjAlber	= 4,
	kCsPrjAzmea	= 11,
	kCsPrjAzmed	= 7,
	kCsPrjBipolar	= 31,
	kCsPrjBonne	= 24,
	kCsPrjCassini	= 22,
	kCsPrjEckert4	= 25,
	kCsPrjEckert6	= 26,
	kCsPrjEdcnc	= 12,
	kCsPrjEdcyl	= 20,
	kCsPrjGnomonic	= 19,
	kCsPrjGoode	= 28,
	kCsPrjHom1uv	= 1281,
	kCsPrjHom1xy	= 1282,
	kCsPrjHom2uv	= 1283,
	kCsPrjHom2xy	= 1284,
	kCsPrjLL	= 1,
	kCsPrjLm1sp	= 36,
	kCsPrjLm2sp	= 37,
	kCsPrjLmblg	= 38,
	kCsPrjLmtan	= 8,
	kCsPrjMiller	= 13,
	kCsPrjMndotl	= 41,
	kCsPrjMndott	= 42,
	kCsPrjModpc	= 10,
	kCsPrjMollweid	= 27,
	kCsPrjMrcat	= 6,
	kCsPrjMstero	= 15,
	kCsPrjNeacyl	= 29,
	kCsPrjNzealand	= 16,
	kCsPrjOblqM	= 5,
	kCsPrjOrtho	= 18,
	kCsPrjOstro	= 34,
	kCsPrjPlycn	= 9,
	kCsPrjPstro	= 33,
	kCsPrjRskew	= 1285,
	kCsPrjRskewc	= 1286,
	kCsPrjRobinson	= 23,
	kCsPrjSinus	= 17,
	kCsPrjSotrm	= 43,
	kCsPrjSstro	= 35,
	kCsPrjSwiss	= 32,
	kCsPrjTeacyl	= 30,
	kCsPrjTm	= 3,
	kCsPrjTrmrs	= 45,
	kCsPrjVdgrntn	= 21,
	kCsPrjWccsl	= 39,
	kCsPrjWccst	= 40,
	kCsPrjUtm	= 44,
	kCsPrjUnknown	= 0
    }	CsProjection;

typedef /* [public][public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0006
    {	kCsDtConvNone	= 0,
	kCsDtConvMolodensky	= 1,
	kCsDtConvMReg	= 2,
	kCsDtConvBursa	= 3,
	kCsDtConvNAD27	= 4,
	kCsDtConvNAD83	= 5,
	kCsDtConvWGS84	= 6,
	kCsDtConvWGS72	= 7,
	kCsDtConvHPGN	= 8,
	kCsDtConv7Param	= 9,
	kCsDtConvLclgrf	= 99
    }	CsDatumConvert;

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0007
    {	kCsPrjLogTypeNone	= 0,
	kCsPrjLogTypeLng	= 1,
	kCsPrjLogTypeLat	= 2,
	kCsPrjLogTypeAzm	= 3,
	kCsPrjLogTypeAngd	= 4,
	kCsPrjLogTypeCmplxc	= 5,
	kCsPrjLogTypeZnbr	= 6,
	kCsPrjLogTypeHsns	= 7,
	kCsPrjLogTypeGhgt	= 8,
	kCsPrjLogTypeElev	= 9
    }	CsPrjLogicalType;

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0008
    {	kCsPrjFormatNone	= 0,
	kCsPrjFormatLng	= 57507,
	kCsPrjFormatLat	= 41155,
	kCsPrjFormatAngd	= 32931,
	kCsPrjFormatCoef	= 9
    }	CsPrjFormatType;

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_csapi_0000_0009
    {	kCsPrjParamNotUsed	= 0,
	kCsPrjParamCntmer	= 1,
	kCsPrjParamNstdpll	= 2,
	kCsPrjParamSstdpll	= 3,
	kCsPrjParamStdpll	= 4,
	kCsPrjParamGcp1lng	= 5,
	kCsPrjParamGcp1lat	= 6,
	kCsPrjParamGcp2lng	= 7,
	kCsPrjParamGcp2lat	= 8,
	kCsPrjParamGcplng	= 9,
	kCsPrjParamGcplat	= 10,
	kCsPrjParamGcazm	= 11,
	kCsPrjParamYaxisaz	= 12,
	kCsPrjParamEstdmer	= 13,
	kCsPrjParamNparall	= 14,
	kCsPrjParamSparall	= 15,
	kCsPrjParamP1lng	= 16,
	kCsPrjParamP1lat	= 17,
	kCsPrjParamP2lng	= 18,
	kCsPrjParamP2lat	= 19,
	kCsPrjParamAdp1p2	= 20,
	kCsPrjParamAdsp1	= 21,
	kCsPrjParamAdsp2	= 22,
	kCsPrjParamCmplxan	= 23,
	kCsPrjParamCmplxbn	= 24,
	kCsPrjParamWestll	= 25,
	kCsPrjParamEastll	= 26,
	kCsPrjParamUtmzn	= 27,
	kCsPrjParamHsns	= 28,
	kCsPrjParamGhgt	= 29,
	kCsPrjParamAelev	= 30
    }	CsPrjParamType;

typedef struct  CCsPointTAG
    {
    double x;
    double y;
    }	CCsPoint;

typedef struct  CCsPoint3dTAG
    {
    double x;
    double y;
    double z;
    }	CCsPoint3d;


















extern RPC_IF_HANDLE __MIDL_itf_csapi_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_csapi_0000_v0_0_s_ifspec;

#ifndef __ICsSession_INTERFACE_DEFINED__
#define __ICsSession_INTERFACE_DEFINED__

/* interface ICsSession */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsSession;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C00EA40-0B84-11D2-9DE2-080009ACE18E")
    ICsSession : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSysDefaultDictionaryInfo( 
            /* [out] */ BSTR __RPC_FAR *ppDir,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
            /* [out] */ BSTR __RPC_FAR *ppDatumDict,
            /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
            /* [out] */ BSTR __RPC_FAR *ppCategoryDict) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDictionaryDir( 
            /* [out] */ BSTR __RPC_FAR *ppDir) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDictionaryDir( 
            /* [in] */ const BSTR kpDirPath,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoidFileName( 
            /* [out] */ BSTR __RPC_FAR *ppFile) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatumFileName( 
            /* [out] */ BSTR __RPC_FAR *ppFile) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCoordsysFileName( 
            /* [out] */ BSTR __RPC_FAR *ppFile) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCategoryFileName( 
            /* [out] */ BSTR __RPC_FAR *ppFile) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDictionaryFileNames( 
            /* [in] */ const BSTR kpEllipsoidDict,
            /* [in] */ const BSTR kpDatumDict,
            /* [in] */ const BSTR kpCoordsysDict,
            /* [in] */ const BSTR kpCategoryDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetPathsToDefault( 
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenDictionaries( 
            /* [in] */ CsOpenStatus status,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CloseDictionaries( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenMode( 
            /* [out] */ CsOpenStatus __RPC_FAR *pStatus) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConverter( 
            /* [out] */ ICsCoordConvert __RPC_FAR *__RPC_FAR *ppConvert,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatumShift( 
            /* [out] */ ICsDatumShift __RPC_FAR *__RPC_FAR *ppShift,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetProtectionMode( 
            /* [in] */ short sMode) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetProtectionMode( 
            /* [out] */ short __RPC_FAR *psMode) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsSessionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsSession __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsSession __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsSession __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSysDefaultDictionaryInfo )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDir,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
            /* [out] */ BSTR __RPC_FAR *ppDatumDict,
            /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
            /* [out] */ BSTR __RPC_FAR *ppCategoryDict);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDictionaryDir )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDir);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDictionaryDir )( 
            ICsSession __RPC_FAR * This,
            /* [in] */ const BSTR kpDirPath,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoidFileName )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppFile);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatumFileName )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppFile);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordsysFileName )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppFile);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCategoryFileName )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppFile);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDictionaryFileNames )( 
            ICsSession __RPC_FAR * This,
            /* [in] */ const BSTR kpEllipsoidDict,
            /* [in] */ const BSTR kpDatumDict,
            /* [in] */ const BSTR kpCoordsysDict,
            /* [in] */ const BSTR kpCategoryDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetPathsToDefault )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenDictionaries )( 
            ICsSession __RPC_FAR * This,
            /* [in] */ CsOpenStatus status,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CloseDictionaries )( 
            ICsSession __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenMode )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ CsOpenStatus __RPC_FAR *pStatus);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConverter )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ ICsCoordConvert __RPC_FAR *__RPC_FAR *ppConvert,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatumShift )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ ICsDatumShift __RPC_FAR *__RPC_FAR *ppShift,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtectionMode )( 
            ICsSession __RPC_FAR * This,
            /* [in] */ short sMode);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProtectionMode )( 
            ICsSession __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psMode);
        
        END_INTERFACE
    } ICsSessionVtbl;

    interface ICsSession
    {
        CONST_VTBL struct ICsSessionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsSession_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsSession_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsSession_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsSession_GetSysDefaultDictionaryInfo(This,ppDir,ppEllipsoidDict,ppDatumDict,ppCoordsysDict,ppCategoryDict)	\
    (This)->lpVtbl -> GetSysDefaultDictionaryInfo(This,ppDir,ppEllipsoidDict,ppDatumDict,ppCoordsysDict,ppCategoryDict)

#define ICsSession_GetDictionaryDir(This,ppDir)	\
    (This)->lpVtbl -> GetDictionaryDir(This,ppDir)

#define ICsSession_SetDictionaryDir(This,kpDirPath,pErr)	\
    (This)->lpVtbl -> SetDictionaryDir(This,kpDirPath,pErr)

#define ICsSession_GetEllipsoidFileName(This,ppFile)	\
    (This)->lpVtbl -> GetEllipsoidFileName(This,ppFile)

#define ICsSession_GetDatumFileName(This,ppFile)	\
    (This)->lpVtbl -> GetDatumFileName(This,ppFile)

#define ICsSession_GetCoordsysFileName(This,ppFile)	\
    (This)->lpVtbl -> GetCoordsysFileName(This,ppFile)

#define ICsSession_GetCategoryFileName(This,ppFile)	\
    (This)->lpVtbl -> GetCategoryFileName(This,ppFile)

#define ICsSession_SetDictionaryFileNames(This,kpEllipsoidDict,kpDatumDict,kpCoordsysDict,kpCategoryDict,pErr)	\
    (This)->lpVtbl -> SetDictionaryFileNames(This,kpEllipsoidDict,kpDatumDict,kpCoordsysDict,kpCategoryDict,pErr)

#define ICsSession_SetPathsToDefault(This,pErr)	\
    (This)->lpVtbl -> SetPathsToDefault(This,pErr)

#define ICsSession_OpenDictionaries(This,status,pErr)	\
    (This)->lpVtbl -> OpenDictionaries(This,status,pErr)

#define ICsSession_CloseDictionaries(This)	\
    (This)->lpVtbl -> CloseDictionaries(This)

#define ICsSession_OpenMode(This,pStatus)	\
    (This)->lpVtbl -> OpenMode(This,pStatus)

#define ICsSession_GetConverter(This,ppConvert,pErr)	\
    (This)->lpVtbl -> GetConverter(This,ppConvert,pErr)

#define ICsSession_GetDatumShift(This,ppShift,pErr)	\
    (This)->lpVtbl -> GetDatumShift(This,ppShift,pErr)

#define ICsSession_SetProtectionMode(This,sMode)	\
    (This)->lpVtbl -> SetProtectionMode(This,sMode)

#define ICsSession_GetProtectionMode(This,psMode)	\
    (This)->lpVtbl -> GetProtectionMode(This,psMode)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetSysDefaultDictionaryInfo_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppDir,
    /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
    /* [out] */ BSTR __RPC_FAR *ppDatumDict,
    /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
    /* [out] */ BSTR __RPC_FAR *ppCategoryDict);


void __RPC_STUB ICsSession_GetSysDefaultDictionaryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetDictionaryDir_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppDir);


void __RPC_STUB ICsSession_GetDictionaryDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_SetDictionaryDir_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [in] */ const BSTR kpDirPath,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_SetDictionaryDir_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetEllipsoidFileName_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppFile);


void __RPC_STUB ICsSession_GetEllipsoidFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetDatumFileName_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppFile);


void __RPC_STUB ICsSession_GetDatumFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetCoordsysFileName_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppFile);


void __RPC_STUB ICsSession_GetCoordsysFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetCategoryFileName_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppFile);


void __RPC_STUB ICsSession_GetCategoryFileName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_SetDictionaryFileNames_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [in] */ const BSTR kpEllipsoidDict,
    /* [in] */ const BSTR kpDatumDict,
    /* [in] */ const BSTR kpCoordsysDict,
    /* [in] */ const BSTR kpCategoryDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_SetDictionaryFileNames_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_SetPathsToDefault_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_SetPathsToDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_OpenDictionaries_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [in] */ CsOpenStatus status,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_OpenDictionaries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_CloseDictionaries_Proxy( 
    ICsSession __RPC_FAR * This);


void __RPC_STUB ICsSession_CloseDictionaries_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_OpenMode_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ CsOpenStatus __RPC_FAR *pStatus);


void __RPC_STUB ICsSession_OpenMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetConverter_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ ICsCoordConvert __RPC_FAR *__RPC_FAR *ppConvert,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_GetConverter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetDatumShift_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ ICsDatumShift __RPC_FAR *__RPC_FAR *ppShift,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsSession_GetDatumShift_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_SetProtectionMode_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [in] */ short sMode);


void __RPC_STUB ICsSession_SetProtectionMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSession_GetProtectionMode_Proxy( 
    ICsSession __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *psMode);


void __RPC_STUB ICsSession_GetProtectionMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsSession_INTERFACE_DEFINED__ */


#ifndef __ICsDictionary_INTERFACE_DEFINED__
#define __ICsDictionary_INTERFACE_DEFINED__

/* interface ICsDictionary */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDictionary;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("0220E18E-3D34-11D2-9022-0060B01AA6ED")
    ICsDictionary : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Open( 
            /* [in] */ BSTR kpPath,
            /* [in] */ CsOpenStatus status,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Close( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPath( 
            /* [out] */ BSTR __RPC_FAR *ppPath) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE OpenMode( 
            /* [out] */ CsOpenStatus __RPC_FAR *pStatus) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDictionaryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDictionary __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDictionary __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDictionary __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Open )( 
            ICsDictionary __RPC_FAR * This,
            /* [in] */ BSTR kpPath,
            /* [in] */ CsOpenStatus status,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Close )( 
            ICsDictionary __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPath )( 
            ICsDictionary __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppPath);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *OpenMode )( 
            ICsDictionary __RPC_FAR * This,
            /* [out] */ CsOpenStatus __RPC_FAR *pStatus);
        
        END_INTERFACE
    } ICsDictionaryVtbl;

    interface ICsDictionary
    {
        CONST_VTBL struct ICsDictionaryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDictionary_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDictionary_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDictionary_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDictionary_Open(This,kpPath,status,pErr)	\
    (This)->lpVtbl -> Open(This,kpPath,status,pErr)

#define ICsDictionary_Close(This)	\
    (This)->lpVtbl -> Close(This)

#define ICsDictionary_GetPath(This,ppPath)	\
    (This)->lpVtbl -> GetPath(This,ppPath)

#define ICsDictionary_OpenMode(This,pStatus)	\
    (This)->lpVtbl -> OpenMode(This,pStatus)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionary_Open_Proxy( 
    ICsDictionary __RPC_FAR * This,
    /* [in] */ BSTR kpPath,
    /* [in] */ CsOpenStatus status,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionary_Open_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionary_Close_Proxy( 
    ICsDictionary __RPC_FAR * This);


void __RPC_STUB ICsDictionary_Close_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionary_GetPath_Proxy( 
    ICsDictionary __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppPath);


void __RPC_STUB ICsDictionary_GetPath_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionary_OpenMode_Proxy( 
    ICsDictionary __RPC_FAR * This,
    /* [out] */ CsOpenStatus __RPC_FAR *pStatus);


void __RPC_STUB ICsDictionary_OpenMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDictionary_INTERFACE_DEFINED__ */


#ifndef __ICsCoordConvert_INTERFACE_DEFINED__
#define __ICsCoordConvert_INTERFACE_DEFINED__

/* interface ICsCoordConvert */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCoordConvert;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C00EA42-0B84-11D2-9DE2-080009ACE18E")
    ICsCoordConvert : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSourceCoordsys( 
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsSrc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDestinationCoordsys( 
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsDst) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCoordsys( 
            /* [in] */ ICsCoordsys __RPC_FAR *pSrc,
            /* [in] */ ICsCoordsys __RPC_FAR *pDst,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCoordsysDefs( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pSrc,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pDst,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDomainCheck( 
            /* [out] */ BOOL __RPC_FAR *pbDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDomainCheck( 
            /* [in] */ BOOL bDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatumCheck( 
            /* [out] */ BOOL __RPC_FAR *pbDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDatumCheck( 
            /* [in] */ BOOL bDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Convert( 
            /* [in] */ double dSrcX,
            /* [in] */ double dSrcY,
            /* [out] */ double __RPC_FAR *pdDstX,
            /* [out] */ double __RPC_FAR *pdDstY,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ConvertArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Convert3dArrayAs2d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValidSourcePoint( 
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValidDestinationPoint( 
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCoordConvertVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCoordConvert __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCoordConvert __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSourceCoordsys )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsSrc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDestinationCoordsys )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsDst);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCoordsys )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ ICsCoordsys __RPC_FAR *pSrc,
            /* [in] */ ICsCoordsys __RPC_FAR *pDst,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCoordsysDefs )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pSrc,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pDst,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDomainCheck )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDomainCheck )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ BOOL bDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatumCheck )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDatumCheck )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ BOOL bDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Convert )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ double dSrcX,
            /* [in] */ double dSrcY,
            /* [out] */ double __RPC_FAR *pdDstX,
            /* [out] */ double __RPC_FAR *pdDstY,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ConvertArray )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Convert3dArrayAs2d )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValidSourcePoint )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValidDestinationPoint )( 
            ICsCoordConvert __RPC_FAR * This,
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsCoordConvertVtbl;

    interface ICsCoordConvert
    {
        CONST_VTBL struct ICsCoordConvertVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCoordConvert_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCoordConvert_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCoordConvert_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCoordConvert_GetSourceCoordsys(This,ppCsSrc)	\
    (This)->lpVtbl -> GetSourceCoordsys(This,ppCsSrc)

#define ICsCoordConvert_GetDestinationCoordsys(This,ppCsDst)	\
    (This)->lpVtbl -> GetDestinationCoordsys(This,ppCsDst)

#define ICsCoordConvert_SetCoordsys(This,pSrc,pDst,pErr)	\
    (This)->lpVtbl -> SetCoordsys(This,pSrc,pDst,pErr)

#define ICsCoordConvert_SetCoordsysDefs(This,pSrc,pDst,pErr)	\
    (This)->lpVtbl -> SetCoordsysDefs(This,pSrc,pDst,pErr)

#define ICsCoordConvert_GetDomainCheck(This,pbDoCheck)	\
    (This)->lpVtbl -> GetDomainCheck(This,pbDoCheck)

#define ICsCoordConvert_SetDomainCheck(This,bDoCheck)	\
    (This)->lpVtbl -> SetDomainCheck(This,bDoCheck)

#define ICsCoordConvert_GetDatumCheck(This,pbDoCheck)	\
    (This)->lpVtbl -> GetDatumCheck(This,pbDoCheck)

#define ICsCoordConvert_SetDatumCheck(This,bDoCheck)	\
    (This)->lpVtbl -> SetDatumCheck(This,bDoCheck)

#define ICsCoordConvert_Convert(This,dSrcX,dSrcY,pdDstX,pdDstY,pErr)	\
    (This)->lpVtbl -> Convert(This,dSrcX,dSrcY,pdDstX,pdDstY,pErr)

#define ICsCoordConvert_ConvertArray(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> ConvertArray(This,dwSize,points,pErr)

#define ICsCoordConvert_Convert3dArrayAs2d(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> Convert3dArrayAs2d(This,dwSize,points,pErr)

#define ICsCoordConvert_IsValidSourcePoint(This,dX,dY,pbIsValid,pErr)	\
    (This)->lpVtbl -> IsValidSourcePoint(This,dX,dY,pbIsValid,pErr)

#define ICsCoordConvert_IsValidDestinationPoint(This,dX,dY,pbIsValid,pErr)	\
    (This)->lpVtbl -> IsValidDestinationPoint(This,dX,dY,pbIsValid,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_GetSourceCoordsys_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsSrc);


void __RPC_STUB ICsCoordConvert_GetSourceCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_GetDestinationCoordsys_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppCsDst);


void __RPC_STUB ICsCoordConvert_GetDestinationCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_SetCoordsys_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ ICsCoordsys __RPC_FAR *pSrc,
    /* [in] */ ICsCoordsys __RPC_FAR *pDst,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_SetCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_SetCoordsysDefs_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pSrc,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pDst,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_SetCoordsysDefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_GetDomainCheck_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbDoCheck);


void __RPC_STUB ICsCoordConvert_GetDomainCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_SetDomainCheck_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ BOOL bDoCheck);


void __RPC_STUB ICsCoordConvert_SetDomainCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_GetDatumCheck_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbDoCheck);


void __RPC_STUB ICsCoordConvert_GetDatumCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_SetDatumCheck_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ BOOL bDoCheck);


void __RPC_STUB ICsCoordConvert_SetDatumCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_Convert_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ double dSrcX,
    /* [in] */ double dSrcY,
    /* [out] */ double __RPC_FAR *pdDstX,
    /* [out] */ double __RPC_FAR *pdDstY,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_Convert_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_ConvertArray_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_ConvertArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_Convert3dArrayAs2d_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_Convert3dArrayAs2d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_IsValidSourcePoint_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ double dX,
    /* [in] */ double dY,
    /* [out] */ BOOL __RPC_FAR *pbIsValid,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_IsValidSourcePoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordConvert_IsValidDestinationPoint_Proxy( 
    ICsCoordConvert __RPC_FAR * This,
    /* [in] */ double dX,
    /* [in] */ double dY,
    /* [out] */ BOOL __RPC_FAR *pbIsValid,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordConvert_IsValidDestinationPoint_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCoordConvert_INTERFACE_DEFINED__ */


#ifndef __ICsDatumShift_INTERFACE_DEFINED__
#define __ICsDatumShift_INTERFACE_DEFINED__

/* interface ICsDatumShift */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatumShift;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C00EA44-0B84-11D2-9DE2-080009ACE18E")
    ICsDatumShift : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ ICsDatum __RPC_FAR *pSrcDatum,
            /* [in] */ ICsDatum __RPC_FAR *pDstDatum,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitDefs( 
            /* [in] */ ICsDatumDef __RPC_FAR *pSrcDef,
            /* [in] */ ICsDatumDef __RPC_FAR *pDstDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Shift( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdLongitude,
            /* [out] */ double __RPC_FAR *pdLatitude,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ShiftArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Shift3dArrayAs2d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumShiftVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatumShift __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatumShift __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ ICsDatum __RPC_FAR *pSrcDatum,
            /* [in] */ ICsDatum __RPC_FAR *pDstDatum,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitDefs )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *pSrcDef,
            /* [in] */ ICsDatumDef __RPC_FAR *pDstDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shift )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdLongitude,
            /* [out] */ double __RPC_FAR *pdLatitude,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShiftArray )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Shift3dArrayAs2d )( 
            ICsDatumShift __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsDatumShiftVtbl;

    interface ICsDatumShift
    {
        CONST_VTBL struct ICsDatumShiftVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatumShift_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatumShift_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatumShift_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatumShift_Init(This,pSrcDatum,pDstDatum,pErr)	\
    (This)->lpVtbl -> Init(This,pSrcDatum,pDstDatum,pErr)

#define ICsDatumShift_InitDefs(This,pSrcDef,pDstDef,pErr)	\
    (This)->lpVtbl -> InitDefs(This,pSrcDef,pDstDef,pErr)

#define ICsDatumShift_Shift(This,dLongitude,dLatitude,pdLongitude,pdLatitude,pErr)	\
    (This)->lpVtbl -> Shift(This,dLongitude,dLatitude,pdLongitude,pdLatitude,pErr)

#define ICsDatumShift_ShiftArray(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> ShiftArray(This,dwSize,points,pErr)

#define ICsDatumShift_Shift3dArrayAs2d(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> Shift3dArrayAs2d(This,dwSize,points,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumShift_Init_Proxy( 
    ICsDatumShift __RPC_FAR * This,
    /* [in] */ ICsDatum __RPC_FAR *pSrcDatum,
    /* [in] */ ICsDatum __RPC_FAR *pDstDatum,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumShift_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumShift_InitDefs_Proxy( 
    ICsDatumShift __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *pSrcDef,
    /* [in] */ ICsDatumDef __RPC_FAR *pDstDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumShift_InitDefs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumShift_Shift_Proxy( 
    ICsDatumShift __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdLongitude,
    /* [out] */ double __RPC_FAR *pdLatitude,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumShift_Shift_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumShift_ShiftArray_Proxy( 
    ICsDatumShift __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumShift_ShiftArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumShift_Shift3dArrayAs2d_Proxy( 
    ICsDatumShift __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumShift_Shift3dArrayAs2d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatumShift_INTERFACE_DEFINED__ */


#ifndef __ICsCoordsys_INTERFACE_DEFINED__
#define __ICsCoordsys_INTERFACE_DEFINED__

/* interface ICsCoordsys */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCoordsys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C00EA46-0B84-11D2-9DE2-080009ACE18E")
    ICsCoordsys : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCoordsysDef( 
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatum( 
            /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppDef) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoidDef( 
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitFromSession( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValid( 
            /* [out] */ BOOL __RPC_FAR *pbIsValid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitGeodetic( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsDatum __RPC_FAR *pDatum,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitCartographic( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDomainCheck( 
            /* [out] */ BOOL __RPC_FAR *pbDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDomainCheck( 
            /* [in] */ BOOL bDoCheck) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValidXY( 
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValidLatLong( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ToLatLong( 
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ double __RPC_FAR *pdLongitude,
            /* [out] */ double __RPC_FAR *pdLatitude,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ToLatLongArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ToLatLong3dArrayAs2d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FromLatLong( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdX,
            /* [out] */ double __RPC_FAR *pdY,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FromLatLongArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FromLatLong3dArrayAs2d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConvergence( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdConvergence,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConvergenceArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dConvergences[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetConvergenceArray3d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScale( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleArray3d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleH( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScaleH,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleHArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleHArray3d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleK( 
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScaleK,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleKArray( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleKArray3d( 
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppClone) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCoordsysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCoordsys __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCoordsys __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordsysDef )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatum )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppDef);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoidDef )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitFromSession )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitGeodetic )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsDatum __RPC_FAR *pDatum,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitCartographic )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDomainCheck )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDomainCheck )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ BOOL bDoCheck);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValidXY )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValidLatLong )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ BOOL __RPC_FAR *pbIsValid,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ToLatLong )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [out] */ double __RPC_FAR *pdLongitude,
            /* [out] */ double __RPC_FAR *pdLatitude,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ToLatLongArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ToLatLong3dArrayAs2d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FromLatLong )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdX,
            /* [out] */ double __RPC_FAR *pdY,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FromLatLongArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FromLatLong3dArrayAs2d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConvergence )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdConvergence,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConvergenceArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dConvergences[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetConvergenceArray3d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScale )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleArray3d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScales[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleH )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScaleH,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleHArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleHArray3d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleK )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ double dLongitude,
            /* [in] */ double dLatitude,
            /* [out] */ double __RPC_FAR *pdScaleK,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleKArray )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleKArray3d )( 
            ICsCoordsys __RPC_FAR * This,
            /* [in] */ DWORD dwSize,
            /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
            /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsCoordsys __RPC_FAR * This,
            /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppClone);
        
        END_INTERFACE
    } ICsCoordsysVtbl;

    interface ICsCoordsys
    {
        CONST_VTBL struct ICsCoordsysVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCoordsys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCoordsys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCoordsys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCoordsys_GetCoordsysDef(This,ppDef)	\
    (This)->lpVtbl -> GetCoordsysDef(This,ppDef)

#define ICsCoordsys_GetDatum(This,ppDef)	\
    (This)->lpVtbl -> GetDatum(This,ppDef)

#define ICsCoordsys_GetEllipsoidDef(This,ppDef)	\
    (This)->lpVtbl -> GetEllipsoidDef(This,ppDef)

#define ICsCoordsys_InitFromSession(This,pCoordsysDef,pSession,pErr)	\
    (This)->lpVtbl -> InitFromSession(This,pCoordsysDef,pSession,pErr)

#define ICsCoordsys_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsCoordsys_InitGeodetic(This,pCoordsysDef,pDatum,pErr)	\
    (This)->lpVtbl -> InitGeodetic(This,pCoordsysDef,pDatum,pErr)

#define ICsCoordsys_InitCartographic(This,pCoordsysDef,pEllipsoidDef,pErr)	\
    (This)->lpVtbl -> InitCartographic(This,pCoordsysDef,pEllipsoidDef,pErr)

#define ICsCoordsys_GetDomainCheck(This,pbDoCheck)	\
    (This)->lpVtbl -> GetDomainCheck(This,pbDoCheck)

#define ICsCoordsys_SetDomainCheck(This,bDoCheck)	\
    (This)->lpVtbl -> SetDomainCheck(This,bDoCheck)

#define ICsCoordsys_IsValidXY(This,dX,dY,pbIsValid,pErr)	\
    (This)->lpVtbl -> IsValidXY(This,dX,dY,pbIsValid,pErr)

#define ICsCoordsys_IsValidLatLong(This,dLongitude,dLatitude,pbIsValid,pErr)	\
    (This)->lpVtbl -> IsValidLatLong(This,dLongitude,dLatitude,pbIsValid,pErr)

#define ICsCoordsys_ToLatLong(This,dX,dY,pdLongitude,pdLatitude,pErr)	\
    (This)->lpVtbl -> ToLatLong(This,dX,dY,pdLongitude,pdLatitude,pErr)

#define ICsCoordsys_ToLatLongArray(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> ToLatLongArray(This,dwSize,points,pErr)

#define ICsCoordsys_ToLatLong3dArrayAs2d(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> ToLatLong3dArrayAs2d(This,dwSize,points,pErr)

#define ICsCoordsys_FromLatLong(This,dLongitude,dLatitude,pdX,pdY,pErr)	\
    (This)->lpVtbl -> FromLatLong(This,dLongitude,dLatitude,pdX,pdY,pErr)

#define ICsCoordsys_FromLatLongArray(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> FromLatLongArray(This,dwSize,points,pErr)

#define ICsCoordsys_FromLatLong3dArrayAs2d(This,dwSize,points,pErr)	\
    (This)->lpVtbl -> FromLatLong3dArrayAs2d(This,dwSize,points,pErr)

#define ICsCoordsys_GetConvergence(This,dLongitude,dLatitude,pdConvergence,pErr)	\
    (This)->lpVtbl -> GetConvergence(This,dLongitude,dLatitude,pdConvergence,pErr)

#define ICsCoordsys_GetConvergenceArray(This,dwSize,points,dConvergences,pErr)	\
    (This)->lpVtbl -> GetConvergenceArray(This,dwSize,points,dConvergences,pErr)

#define ICsCoordsys_GetConvergenceArray3d(This,dwSize,points,dScales,pErr)	\
    (This)->lpVtbl -> GetConvergenceArray3d(This,dwSize,points,dScales,pErr)

#define ICsCoordsys_GetScale(This,dLongitude,dLatitude,pdScale,pErr)	\
    (This)->lpVtbl -> GetScale(This,dLongitude,dLatitude,pdScale,pErr)

#define ICsCoordsys_GetScaleArray(This,dwSize,points,dScales,pErr)	\
    (This)->lpVtbl -> GetScaleArray(This,dwSize,points,dScales,pErr)

#define ICsCoordsys_GetScaleArray3d(This,dwSize,points,dScales,pErr)	\
    (This)->lpVtbl -> GetScaleArray3d(This,dwSize,points,dScales,pErr)

#define ICsCoordsys_GetScaleH(This,dLongitude,dLatitude,pdScaleH,pErr)	\
    (This)->lpVtbl -> GetScaleH(This,dLongitude,dLatitude,pdScaleH,pErr)

#define ICsCoordsys_GetScaleHArray(This,dwSize,points,dScalesH,pErr)	\
    (This)->lpVtbl -> GetScaleHArray(This,dwSize,points,dScalesH,pErr)

#define ICsCoordsys_GetScaleHArray3d(This,dwSize,points,dScalesH,pErr)	\
    (This)->lpVtbl -> GetScaleHArray3d(This,dwSize,points,dScalesH,pErr)

#define ICsCoordsys_GetScaleK(This,dLongitude,dLatitude,pdScaleK,pErr)	\
    (This)->lpVtbl -> GetScaleK(This,dLongitude,dLatitude,pdScaleK,pErr)

#define ICsCoordsys_GetScaleKArray(This,dwSize,points,dScalesK,pErr)	\
    (This)->lpVtbl -> GetScaleKArray(This,dwSize,points,dScalesK,pErr)

#define ICsCoordsys_GetScaleKArray3d(This,dwSize,points,dScalesK,pErr)	\
    (This)->lpVtbl -> GetScaleKArray3d(This,dwSize,points,dScalesK,pErr)

#define ICsCoordsys_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetCoordsysDef_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef);


void __RPC_STUB ICsCoordsys_GetCoordsysDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetDatum_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppDef);


void __RPC_STUB ICsCoordsys_GetDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetEllipsoidDef_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef);


void __RPC_STUB ICsCoordsys_GetEllipsoidDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_InitFromSession_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
    /* [in] */ ICsSession __RPC_FAR *pSession,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_InitFromSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_IsValid_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbIsValid);


void __RPC_STUB ICsCoordsys_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_InitGeodetic_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
    /* [in] */ ICsDatum __RPC_FAR *pDatum,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_InitGeodetic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_InitCartographic_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pCoordsysDef,
    /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_InitCartographic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetDomainCheck_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbDoCheck);


void __RPC_STUB ICsCoordsys_GetDomainCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_SetDomainCheck_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ BOOL bDoCheck);


void __RPC_STUB ICsCoordsys_SetDomainCheck_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_IsValidXY_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dX,
    /* [in] */ double dY,
    /* [out] */ BOOL __RPC_FAR *pbIsValid,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_IsValidXY_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_IsValidLatLong_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ BOOL __RPC_FAR *pbIsValid,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_IsValidLatLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_ToLatLong_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dX,
    /* [in] */ double dY,
    /* [out] */ double __RPC_FAR *pdLongitude,
    /* [out] */ double __RPC_FAR *pdLatitude,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_ToLatLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_ToLatLongArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_ToLatLongArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_ToLatLong3dArrayAs2d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_ToLatLong3dArrayAs2d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_FromLatLong_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdX,
    /* [out] */ double __RPC_FAR *pdY,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_FromLatLong_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_FromLatLongArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_FromLatLongArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_FromLatLong3dArrayAs2d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][out][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_FromLatLong3dArrayAs2d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetConvergence_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdConvergence,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetConvergence_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetConvergenceArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dConvergences[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetConvergenceArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetConvergenceArray3d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScales[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetConvergenceArray3d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScale_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdScale,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScales[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleArray3d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScales[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleArray3d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleH_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdScaleH,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleH_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleHArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleHArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleHArray3d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScalesH[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleHArray3d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleK_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ double dLongitude,
    /* [in] */ double dLatitude,
    /* [out] */ double __RPC_FAR *pdScaleK,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleK_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleKArray_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleKArray_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_GetScaleKArray3d_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [in] */ DWORD dwSize,
    /* [size_is][in] */ CCsPoint3d __RPC_FAR points[  ],
    /* [size_is][out] */ double __RPC_FAR dScalesK[  ],
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsys_GetScaleKArray3d_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsys_Clone_Proxy( 
    ICsCoordsys __RPC_FAR * This,
    /* [out] */ ICsCoordsys __RPC_FAR *__RPC_FAR *ppClone);


void __RPC_STUB ICsCoordsys_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCoordsys_INTERFACE_DEFINED__ */


#ifndef __ICsDef_INTERFACE_DEFINED__
#define __ICsDef_INTERFACE_DEFINED__

/* interface ICsDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("926B2FD6-0D18-11d2-9DE2-080009ACE18E")
    ICsDef : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetName( 
            /* [out] */ BSTR __RPC_FAR *ppName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalName( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsValid( 
            /* [out] */ BOOL __RPC_FAR *pbIsValid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsUsable( 
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsSameAs( 
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        END_INTERFACE
    } ICsDefVtbl;

    interface ICsDef
    {
        CONST_VTBL struct ICsDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_GetName_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppName);


void __RPC_STUB ICsDef_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_SetName_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDef_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_IsLegalName_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsDef_IsLegalName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_IsValid_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbIsValid);


void __RPC_STUB ICsDef_IsValid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_IsUsable_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [in] */ ICsSession __RPC_FAR *pSession,
    /* [out] */ BOOL __RPC_FAR *pbIsUsable);


void __RPC_STUB ICsDef_IsUsable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_IsSameAs_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [in] */ ICsDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbIsSame);


void __RPC_STUB ICsDef_IsSameAs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDef_Clone_Proxy( 
    ICsDef __RPC_FAR * This,
    /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);


void __RPC_STUB ICsDef_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDef_INTERFACE_DEFINED__ */


#ifndef __ICsMentorDef_INTERFACE_DEFINED__
#define __ICsMentorDef_INTERFACE_DEFINED__

/* interface ICsMentorDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsMentorDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("926B2FD7-0D18-11d2-9DE2-080009ACE18E")
    ICsMentorDef : public ICsDef
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [out] */ BSTR __RPC_FAR *ppDesc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetDescription( 
            /* [in] */ const BSTR kpDesc,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalDescription( 
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetGroup( 
            /* [out] */ BSTR __RPC_FAR *ppGroup) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGroup( 
            /* [in] */ const BSTR kpGroup,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalGroup( 
            /* [in] */ const BSTR kpGroup,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSource( 
            /* [out] */ BSTR __RPC_FAR *ppSource) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSource( 
            /* [in] */ const BSTR kpSource,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalSource( 
            /* [in] */ const BSTR kpSource,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsProtected( 
            /* [out] */ BOOL __RPC_FAR *pbIsProtected) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAge( 
            /* [out] */ short __RPC_FAR *psAge) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetProtectMode( 
            /* [in] */ BOOL bIsProtected) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsEncrypted( 
            /* [out] */ BOOL __RPC_FAR *pbIsEncrypted) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetEncryptMode( 
            /* [in] */ BOOL bIsEncrypted) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsMentorDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsMentorDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsMentorDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDesc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalDescription )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGroup )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppGroup);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGroup )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalGroup )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSource )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSource )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalSource )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsProtected )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAge )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psAge);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtectMode )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ BOOL bIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEncrypted )( 
            ICsMentorDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEncryptMode )( 
            ICsMentorDef __RPC_FAR * This,
            /* [in] */ BOOL bIsEncrypted);
        
        END_INTERFACE
    } ICsMentorDefVtbl;

    interface ICsMentorDef
    {
        CONST_VTBL struct ICsMentorDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsMentorDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsMentorDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsMentorDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsMentorDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsMentorDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsMentorDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsMentorDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsMentorDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsMentorDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsMentorDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)


#define ICsMentorDef_GetDescription(This,ppDesc)	\
    (This)->lpVtbl -> GetDescription(This,ppDesc)

#define ICsMentorDef_SetDescription(This,kpDesc,pErr)	\
    (This)->lpVtbl -> SetDescription(This,kpDesc,pErr)

#define ICsMentorDef_IsLegalDescription(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalDescription(This,kpDesc,pbIsLegal)

#define ICsMentorDef_GetGroup(This,ppGroup)	\
    (This)->lpVtbl -> GetGroup(This,ppGroup)

#define ICsMentorDef_SetGroup(This,kpGroup,pErr)	\
    (This)->lpVtbl -> SetGroup(This,kpGroup,pErr)

#define ICsMentorDef_IsLegalGroup(This,kpGroup,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalGroup(This,kpGroup,pbIsLegal)

#define ICsMentorDef_GetSource(This,ppSource)	\
    (This)->lpVtbl -> GetSource(This,ppSource)

#define ICsMentorDef_SetSource(This,kpSource,pErr)	\
    (This)->lpVtbl -> SetSource(This,kpSource,pErr)

#define ICsMentorDef_IsLegalSource(This,kpSource,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalSource(This,kpSource,pbIsLegal)

#define ICsMentorDef_IsProtected(This,pbIsProtected)	\
    (This)->lpVtbl -> IsProtected(This,pbIsProtected)

#define ICsMentorDef_GetAge(This,psAge)	\
    (This)->lpVtbl -> GetAge(This,psAge)

#define ICsMentorDef_SetProtectMode(This,bIsProtected)	\
    (This)->lpVtbl -> SetProtectMode(This,bIsProtected)

#define ICsMentorDef_IsEncrypted(This,pbIsEncrypted)	\
    (This)->lpVtbl -> IsEncrypted(This,pbIsEncrypted)

#define ICsMentorDef_SetEncryptMode(This,bIsEncrypted)	\
    (This)->lpVtbl -> SetEncryptMode(This,bIsEncrypted)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_GetDescription_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppDesc);


void __RPC_STUB ICsMentorDef_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_SetDescription_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpDesc,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsMentorDef_SetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_IsLegalDescription_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpDesc,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsMentorDef_IsLegalDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_GetGroup_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppGroup);


void __RPC_STUB ICsMentorDef_GetGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_SetGroup_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpGroup,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsMentorDef_SetGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_IsLegalGroup_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpGroup,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsMentorDef_IsLegalGroup_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_GetSource_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppSource);


void __RPC_STUB ICsMentorDef_GetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_SetSource_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpSource,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsMentorDef_SetSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_IsLegalSource_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ const BSTR kpSource,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsMentorDef_IsLegalSource_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_IsProtected_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbIsProtected);


void __RPC_STUB ICsMentorDef_IsProtected_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_GetAge_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *psAge);


void __RPC_STUB ICsMentorDef_GetAge_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_SetProtectMode_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ BOOL bIsProtected);


void __RPC_STUB ICsMentorDef_SetProtectMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_IsEncrypted_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbIsEncrypted);


void __RPC_STUB ICsMentorDef_IsEncrypted_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorDef_SetEncryptMode_Proxy( 
    ICsMentorDef __RPC_FAR * This,
    /* [in] */ BOOL bIsEncrypted);


void __RPC_STUB ICsMentorDef_SetEncryptMode_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsMentorDef_INTERFACE_DEFINED__ */


#ifndef __ICsCoordsysDef_INTERFACE_DEFINED__
#define __ICsCoordsysDef_INTERFACE_DEFINED__

/* interface ICsCoordsysDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCoordsysDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("3C00EA4A-0B84-11D2-9DE2-080009ACE18E")
    ICsCoordsysDef : public ICsMentorDef
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetErrors( 
            /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLocation( 
            /* [out] */ BSTR __RPC_FAR *ppLoc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetLocation( 
            /* [in] */ const BSTR kpLoc,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalLocation( 
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCntrySt( 
            /* [out] */ BSTR __RPC_FAR *ppCntrySt) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCntrySt( 
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalCntrySt( 
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsGeodetic( 
            /* [out] */ BOOL __RPC_FAR *pbIsGeodetic) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetGeodetic( 
            /* [in] */ const BSTR kpDatumName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCartographic( 
            /* [in] */ const BSTR kpEllipsoidName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatumName( 
            /* [out] */ BSTR __RPC_FAR *ppDatum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoidName( 
            /* [out] */ BSTR __RPC_FAR *ppEllipsoid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetProjection( 
            /* [out] */ CsProjection __RPC_FAR *pPrjType) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetProjection( 
            /* [in] */ CsProjection prjType,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUnit( 
            /* [out] */ CsUnit __RPC_FAR *pUnit) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetUnit( 
            /* [in] */ CsUnit unit,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParam( 
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdValue,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetParam( 
            /* [in] */ DWORD dwIndex,
            /* [in] */ double dValue,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOriginLongitude( 
            /* [out] */ double __RPC_FAR *pdOrgLng) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOriginLongitude( 
            /* [in] */ double dOrgLng) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOriginLatitude( 
            /* [out] */ double __RPC_FAR *pdOrgLat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOriginLatitude( 
            /* [in] */ double dOrgLat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOffsets( 
            /* [out] */ double __RPC_FAR *pdXOffset,
            /* [out] */ double __RPC_FAR *pdYOffset) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOffsets( 
            /* [in] */ double dXOffset,
            /* [in] */ double dYoffset) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetScaleReduction( 
            /* [out] */ double __RPC_FAR *pdSclRed) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetScaleReduction( 
            /* [in] */ double dSclRed) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMapScale( 
            /* [out] */ double __RPC_FAR *pdMapScale) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetMapScale( 
            /* [in] */ double dMapScale) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetZeroes( 
            /* [out] */ double __RPC_FAR *pdXZero,
            /* [out] */ double __RPC_FAR *pdYZero) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetZeroes( 
            /* [in] */ double dXZero,
            /* [in] */ double dYZero) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLatLongBounds( 
            /* [out] */ double __RPC_FAR *pdLonMin,
            /* [out] */ double __RPC_FAR *pdLatMin,
            /* [out] */ double __RPC_FAR *pdLonMax,
            /* [out] */ double __RPC_FAR *pdLatMax) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetLatLongBounds( 
            /* [in] */ double dLonMin,
            /* [in] */ double dLatMin,
            /* [in] */ double dLonMax,
            /* [in] */ double dLatMax) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CancelLatLongBounds( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetXYBounds( 
            /* [out] */ double __RPC_FAR *pdXMin,
            /* [out] */ double __RPC_FAR *pdYMin,
            /* [out] */ double __RPC_FAR *pdXMax,
            /* [out] */ double __RPC_FAR *pdYMax) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetXYBounds( 
            /* [in] */ double dXMin,
            /* [in] */ double dYMin,
            /* [in] */ double dXMax,
            /* [in] */ double dYMax) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CancelXYBounds( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetQuadrant( 
            /* [out] */ short __RPC_FAR *psQuad) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetQuadrant( 
            /* [in] */ short sQuad,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCoordsysDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCoordsysDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCoordsysDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDesc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalDescription )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGroup )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppGroup);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGroup )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalGroup )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSource )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSource )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalSource )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsProtected )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAge )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psAge);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtectMode )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ BOOL bIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEncrypted )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEncryptMode )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ BOOL bIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrors )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLocation )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppLoc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLocation )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpLoc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalLocation )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCntrySt )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppCntrySt);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCntrySt )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalCntrySt )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsGeodetic )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsGeodetic);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGeodetic )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDatumName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCartographic )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ const BSTR kpEllipsoidName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatumName )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDatum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoidName )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProjection )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ CsProjection __RPC_FAR *pPrjType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProjection )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnit )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ CsUnit __RPC_FAR *pUnit);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetUnit )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParam )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdValue,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetParam )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ DWORD dwIndex,
            /* [in] */ double dValue,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOriginLongitude )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdOrgLng);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOriginLongitude )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dOrgLng);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOriginLatitude )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdOrgLat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOriginLatitude )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dOrgLat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOffsets )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdXOffset,
            /* [out] */ double __RPC_FAR *pdYOffset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOffsets )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dXOffset,
            /* [in] */ double dYoffset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetScaleReduction )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdSclRed);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetScaleReduction )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dSclRed);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMapScale )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdMapScale);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMapScale )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dMapScale);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetZeroes )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdXZero,
            /* [out] */ double __RPC_FAR *pdYZero);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetZeroes )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dXZero,
            /* [in] */ double dYZero);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLatLongBounds )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdLonMin,
            /* [out] */ double __RPC_FAR *pdLatMin,
            /* [out] */ double __RPC_FAR *pdLonMax,
            /* [out] */ double __RPC_FAR *pdLatMax);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLatLongBounds )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dLonMin,
            /* [in] */ double dLatMin,
            /* [in] */ double dLonMax,
            /* [in] */ double dLatMax);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelLatLongBounds )( 
            ICsCoordsysDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetXYBounds )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdXMin,
            /* [out] */ double __RPC_FAR *pdYMin,
            /* [out] */ double __RPC_FAR *pdXMax,
            /* [out] */ double __RPC_FAR *pdYMax);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetXYBounds )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ double dXMin,
            /* [in] */ double dYMin,
            /* [in] */ double dXMax,
            /* [in] */ double dYMax);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CancelXYBounds )( 
            ICsCoordsysDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetQuadrant )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psQuad);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetQuadrant )( 
            ICsCoordsysDef __RPC_FAR * This,
            /* [in] */ short sQuad,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsCoordsysDefVtbl;

    interface ICsCoordsysDef
    {
        CONST_VTBL struct ICsCoordsysDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCoordsysDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCoordsysDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCoordsysDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCoordsysDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsCoordsysDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsCoordsysDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsCoordsysDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsCoordsysDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsCoordsysDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsCoordsysDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)


#define ICsCoordsysDef_GetDescription(This,ppDesc)	\
    (This)->lpVtbl -> GetDescription(This,ppDesc)

#define ICsCoordsysDef_SetDescription(This,kpDesc,pErr)	\
    (This)->lpVtbl -> SetDescription(This,kpDesc,pErr)

#define ICsCoordsysDef_IsLegalDescription(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalDescription(This,kpDesc,pbIsLegal)

#define ICsCoordsysDef_GetGroup(This,ppGroup)	\
    (This)->lpVtbl -> GetGroup(This,ppGroup)

#define ICsCoordsysDef_SetGroup(This,kpGroup,pErr)	\
    (This)->lpVtbl -> SetGroup(This,kpGroup,pErr)

#define ICsCoordsysDef_IsLegalGroup(This,kpGroup,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalGroup(This,kpGroup,pbIsLegal)

#define ICsCoordsysDef_GetSource(This,ppSource)	\
    (This)->lpVtbl -> GetSource(This,ppSource)

#define ICsCoordsysDef_SetSource(This,kpSource,pErr)	\
    (This)->lpVtbl -> SetSource(This,kpSource,pErr)

#define ICsCoordsysDef_IsLegalSource(This,kpSource,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalSource(This,kpSource,pbIsLegal)

#define ICsCoordsysDef_IsProtected(This,pbIsProtected)	\
    (This)->lpVtbl -> IsProtected(This,pbIsProtected)

#define ICsCoordsysDef_GetAge(This,psAge)	\
    (This)->lpVtbl -> GetAge(This,psAge)

#define ICsCoordsysDef_SetProtectMode(This,bIsProtected)	\
    (This)->lpVtbl -> SetProtectMode(This,bIsProtected)

#define ICsCoordsysDef_IsEncrypted(This,pbIsEncrypted)	\
    (This)->lpVtbl -> IsEncrypted(This,pbIsEncrypted)

#define ICsCoordsysDef_SetEncryptMode(This,bIsEncrypted)	\
    (This)->lpVtbl -> SetEncryptMode(This,bIsEncrypted)


#define ICsCoordsysDef_GetErrors(This,ppEnum)	\
    (This)->lpVtbl -> GetErrors(This,ppEnum)

#define ICsCoordsysDef_GetLocation(This,ppLoc)	\
    (This)->lpVtbl -> GetLocation(This,ppLoc)

#define ICsCoordsysDef_SetLocation(This,kpLoc,pErr)	\
    (This)->lpVtbl -> SetLocation(This,kpLoc,pErr)

#define ICsCoordsysDef_IsLegalLocation(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalLocation(This,kpDesc,pbIsLegal)

#define ICsCoordsysDef_GetCntrySt(This,ppCntrySt)	\
    (This)->lpVtbl -> GetCntrySt(This,ppCntrySt)

#define ICsCoordsysDef_SetCntrySt(This,kpCntrySt,pErr)	\
    (This)->lpVtbl -> SetCntrySt(This,kpCntrySt,pErr)

#define ICsCoordsysDef_IsLegalCntrySt(This,kpCntrySt,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalCntrySt(This,kpCntrySt,pbIsLegal)

#define ICsCoordsysDef_IsGeodetic(This,pbIsGeodetic)	\
    (This)->lpVtbl -> IsGeodetic(This,pbIsGeodetic)

#define ICsCoordsysDef_SetGeodetic(This,kpDatumName,pErr)	\
    (This)->lpVtbl -> SetGeodetic(This,kpDatumName,pErr)

#define ICsCoordsysDef_SetCartographic(This,kpEllipsoidName,pErr)	\
    (This)->lpVtbl -> SetCartographic(This,kpEllipsoidName,pErr)

#define ICsCoordsysDef_GetDatumName(This,ppDatum)	\
    (This)->lpVtbl -> GetDatumName(This,ppDatum)

#define ICsCoordsysDef_GetEllipsoidName(This,ppEllipsoid)	\
    (This)->lpVtbl -> GetEllipsoidName(This,ppEllipsoid)

#define ICsCoordsysDef_GetProjection(This,pPrjType)	\
    (This)->lpVtbl -> GetProjection(This,pPrjType)

#define ICsCoordsysDef_SetProjection(This,prjType,pErr)	\
    (This)->lpVtbl -> SetProjection(This,prjType,pErr)

#define ICsCoordsysDef_GetUnit(This,pUnit)	\
    (This)->lpVtbl -> GetUnit(This,pUnit)

#define ICsCoordsysDef_SetUnit(This,unit,pErr)	\
    (This)->lpVtbl -> SetUnit(This,unit,pErr)

#define ICsCoordsysDef_GetParam(This,dwIndex,pdValue,pErr)	\
    (This)->lpVtbl -> GetParam(This,dwIndex,pdValue,pErr)

#define ICsCoordsysDef_SetParam(This,dwIndex,dValue,pErr)	\
    (This)->lpVtbl -> SetParam(This,dwIndex,dValue,pErr)

#define ICsCoordsysDef_GetOriginLongitude(This,pdOrgLng)	\
    (This)->lpVtbl -> GetOriginLongitude(This,pdOrgLng)

#define ICsCoordsysDef_SetOriginLongitude(This,dOrgLng)	\
    (This)->lpVtbl -> SetOriginLongitude(This,dOrgLng)

#define ICsCoordsysDef_GetOriginLatitude(This,pdOrgLat)	\
    (This)->lpVtbl -> GetOriginLatitude(This,pdOrgLat)

#define ICsCoordsysDef_SetOriginLatitude(This,dOrgLat)	\
    (This)->lpVtbl -> SetOriginLatitude(This,dOrgLat)

#define ICsCoordsysDef_GetOffsets(This,pdXOffset,pdYOffset)	\
    (This)->lpVtbl -> GetOffsets(This,pdXOffset,pdYOffset)

#define ICsCoordsysDef_SetOffsets(This,dXOffset,dYoffset)	\
    (This)->lpVtbl -> SetOffsets(This,dXOffset,dYoffset)

#define ICsCoordsysDef_GetScaleReduction(This,pdSclRed)	\
    (This)->lpVtbl -> GetScaleReduction(This,pdSclRed)

#define ICsCoordsysDef_SetScaleReduction(This,dSclRed)	\
    (This)->lpVtbl -> SetScaleReduction(This,dSclRed)

#define ICsCoordsysDef_GetMapScale(This,pdMapScale)	\
    (This)->lpVtbl -> GetMapScale(This,pdMapScale)

#define ICsCoordsysDef_SetMapScale(This,dMapScale)	\
    (This)->lpVtbl -> SetMapScale(This,dMapScale)

#define ICsCoordsysDef_GetZeroes(This,pdXZero,pdYZero)	\
    (This)->lpVtbl -> GetZeroes(This,pdXZero,pdYZero)

#define ICsCoordsysDef_SetZeroes(This,dXZero,dYZero)	\
    (This)->lpVtbl -> SetZeroes(This,dXZero,dYZero)

#define ICsCoordsysDef_GetLatLongBounds(This,pdLonMin,pdLatMin,pdLonMax,pdLatMax)	\
    (This)->lpVtbl -> GetLatLongBounds(This,pdLonMin,pdLatMin,pdLonMax,pdLatMax)

#define ICsCoordsysDef_SetLatLongBounds(This,dLonMin,dLatMin,dLonMax,dLatMax)	\
    (This)->lpVtbl -> SetLatLongBounds(This,dLonMin,dLatMin,dLonMax,dLatMax)

#define ICsCoordsysDef_CancelLatLongBounds(This)	\
    (This)->lpVtbl -> CancelLatLongBounds(This)

#define ICsCoordsysDef_GetXYBounds(This,pdXMin,pdYMin,pdXMax,pdYMax)	\
    (This)->lpVtbl -> GetXYBounds(This,pdXMin,pdYMin,pdXMax,pdYMax)

#define ICsCoordsysDef_SetXYBounds(This,dXMin,dYMin,dXMax,dYMax)	\
    (This)->lpVtbl -> SetXYBounds(This,dXMin,dYMin,dXMax,dYMax)

#define ICsCoordsysDef_CancelXYBounds(This)	\
    (This)->lpVtbl -> CancelXYBounds(This)

#define ICsCoordsysDef_GetQuadrant(This,psQuad)	\
    (This)->lpVtbl -> GetQuadrant(This,psQuad)

#define ICsCoordsysDef_SetQuadrant(This,sQuad,pErr)	\
    (This)->lpVtbl -> SetQuadrant(This,sQuad,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetErrors_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsCoordsysDef_GetErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetLocation_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppLoc);


void __RPC_STUB ICsCoordsysDef_GetLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetLocation_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpLoc,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_IsLegalLocation_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpDesc,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsCoordsysDef_IsLegalLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetCntrySt_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppCntrySt);


void __RPC_STUB ICsCoordsysDef_GetCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetCntrySt_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpCntrySt,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_IsLegalCntrySt_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpCntrySt,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsCoordsysDef_IsLegalCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_IsGeodetic_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbIsGeodetic);


void __RPC_STUB ICsCoordsysDef_IsGeodetic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetGeodetic_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpDatumName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetGeodetic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetCartographic_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ const BSTR kpEllipsoidName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetCartographic_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetDatumName_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppDatum);


void __RPC_STUB ICsCoordsysDef_GetDatumName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetEllipsoidName_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppEllipsoid);


void __RPC_STUB ICsCoordsysDef_GetEllipsoidName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetProjection_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ CsProjection __RPC_FAR *pPrjType);


void __RPC_STUB ICsCoordsysDef_GetProjection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetProjection_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetProjection_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetUnit_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ CsUnit __RPC_FAR *pUnit);


void __RPC_STUB ICsCoordsysDef_GetUnit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetUnit_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetUnit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetParam_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ DWORD dwIndex,
    /* [out] */ double __RPC_FAR *pdValue,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_GetParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetParam_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ DWORD dwIndex,
    /* [in] */ double dValue,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetOriginLongitude_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdOrgLng);


void __RPC_STUB ICsCoordsysDef_GetOriginLongitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetOriginLongitude_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dOrgLng);


void __RPC_STUB ICsCoordsysDef_SetOriginLongitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetOriginLatitude_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdOrgLat);


void __RPC_STUB ICsCoordsysDef_GetOriginLatitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetOriginLatitude_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dOrgLat);


void __RPC_STUB ICsCoordsysDef_SetOriginLatitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetOffsets_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdXOffset,
    /* [out] */ double __RPC_FAR *pdYOffset);


void __RPC_STUB ICsCoordsysDef_GetOffsets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetOffsets_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dXOffset,
    /* [in] */ double dYoffset);


void __RPC_STUB ICsCoordsysDef_SetOffsets_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetScaleReduction_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdSclRed);


void __RPC_STUB ICsCoordsysDef_GetScaleReduction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetScaleReduction_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dSclRed);


void __RPC_STUB ICsCoordsysDef_SetScaleReduction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetMapScale_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdMapScale);


void __RPC_STUB ICsCoordsysDef_GetMapScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetMapScale_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dMapScale);


void __RPC_STUB ICsCoordsysDef_SetMapScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetZeroes_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdXZero,
    /* [out] */ double __RPC_FAR *pdYZero);


void __RPC_STUB ICsCoordsysDef_GetZeroes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetZeroes_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dXZero,
    /* [in] */ double dYZero);


void __RPC_STUB ICsCoordsysDef_SetZeroes_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetLatLongBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdLonMin,
    /* [out] */ double __RPC_FAR *pdLatMin,
    /* [out] */ double __RPC_FAR *pdLonMax,
    /* [out] */ double __RPC_FAR *pdLatMax);


void __RPC_STUB ICsCoordsysDef_GetLatLongBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetLatLongBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dLonMin,
    /* [in] */ double dLatMin,
    /* [in] */ double dLonMax,
    /* [in] */ double dLatMax);


void __RPC_STUB ICsCoordsysDef_SetLatLongBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_CancelLatLongBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This);


void __RPC_STUB ICsCoordsysDef_CancelLatLongBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetXYBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdXMin,
    /* [out] */ double __RPC_FAR *pdYMin,
    /* [out] */ double __RPC_FAR *pdXMax,
    /* [out] */ double __RPC_FAR *pdYMax);


void __RPC_STUB ICsCoordsysDef_GetXYBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetXYBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ double dXMin,
    /* [in] */ double dYMin,
    /* [in] */ double dXMax,
    /* [in] */ double dYMax);


void __RPC_STUB ICsCoordsysDef_SetXYBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_CancelXYBounds_Proxy( 
    ICsCoordsysDef __RPC_FAR * This);


void __RPC_STUB ICsCoordsysDef_CancelXYBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_GetQuadrant_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [out] */ short __RPC_FAR *psQuad);


void __RPC_STUB ICsCoordsysDef_GetQuadrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDef_SetQuadrant_Proxy( 
    ICsCoordsysDef __RPC_FAR * This,
    /* [in] */ short sQuad,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDef_SetQuadrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCoordsysDef_INTERFACE_DEFINED__ */


#ifndef __ICsDatum_INTERFACE_DEFINED__
#define __ICsDatum_INTERFACE_DEFINED__

/* interface ICsDatum */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4E01DB0-0D13-11D2-9DE2-080009ACE18E")
    ICsDatum : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatumDef( 
            /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoidDef( 
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE InitFromSession( 
            /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Init( 
            /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppClone) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatum __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatum __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatum __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatumDef )( 
            ICsDatum __RPC_FAR * This,
            /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoidDef )( 
            ICsDatum __RPC_FAR * This,
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitFromSession )( 
            ICsDatum __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            ICsDatum __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsDatum __RPC_FAR * This,
            /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppClone);
        
        END_INTERFACE
    } ICsDatumVtbl;

    interface ICsDatum
    {
        CONST_VTBL struct ICsDatumVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatum_GetDatumDef(This,ppDef)	\
    (This)->lpVtbl -> GetDatumDef(This,ppDef)

#define ICsDatum_GetEllipsoidDef(This,ppDef)	\
    (This)->lpVtbl -> GetEllipsoidDef(This,ppDef)

#define ICsDatum_InitFromSession(This,pDatumDef,pSession,pErr)	\
    (This)->lpVtbl -> InitFromSession(This,pDatumDef,pSession,pErr)

#define ICsDatum_Init(This,pDatumDef,pEllipsoidDef,pErr)	\
    (This)->lpVtbl -> Init(This,pDatumDef,pEllipsoidDef,pErr)

#define ICsDatum_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatum_GetDatumDef_Proxy( 
    ICsDatum __RPC_FAR * This,
    /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef);


void __RPC_STUB ICsDatum_GetDatumDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatum_GetEllipsoidDef_Proxy( 
    ICsDatum __RPC_FAR * This,
    /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef);


void __RPC_STUB ICsDatum_GetEllipsoidDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatum_InitFromSession_Proxy( 
    ICsDatum __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
    /* [in] */ ICsSession __RPC_FAR *pSession,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatum_InitFromSession_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatum_Init_Proxy( 
    ICsDatum __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *pDatumDef,
    /* [in] */ ICsEllipsoidDef __RPC_FAR *pEllipsoidDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatum_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatum_Clone_Proxy( 
    ICsDatum __RPC_FAR * This,
    /* [out] */ ICsDatum __RPC_FAR *__RPC_FAR *ppClone);


void __RPC_STUB ICsDatum_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatum_INTERFACE_DEFINED__ */


#ifndef __ICsEllipsoidDef_INTERFACE_DEFINED__
#define __ICsEllipsoidDef_INTERFACE_DEFINED__

/* interface ICsEllipsoidDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEllipsoidDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4E01DB3-0D13-11D2-9DE2-080009ACE18E")
    ICsEllipsoidDef : public ICsMentorDef
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEquatorialRadius( 
            /* [out] */ double __RPC_FAR *pdRadius) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPolarRadius( 
            /* [out] */ double __RPC_FAR *pdRadius) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFlatteningRatio( 
            /* [out] */ double __RPC_FAR *pdFlat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEccentricity( 
            /* [out] */ double __RPC_FAR *pdEccent) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetRadii( 
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dPolarRadius,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FlatToEccent( 
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEccent) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EccentToFlat( 
            /* [in] */ double dEccent,
            /* [out] */ double __RPC_FAR *pdFlat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EquatorialFromPolarFlat( 
            /* [in] */ double dPolarRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEquatorialRadius) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PolarFromEquatorialFlat( 
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdPolarRadius) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEllipsoidDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEllipsoidDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEllipsoidDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDesc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalDescription )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGroup )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppGroup);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGroup )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalGroup )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSource )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSource )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalSource )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsProtected )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAge )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psAge);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtectMode )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ BOOL bIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEncrypted )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEncryptMode )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ BOOL bIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEquatorialRadius )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdRadius);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetPolarRadius )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdRadius);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFlatteningRatio )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdFlat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEccentricity )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdEccent);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetRadii )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dPolarRadius,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FlatToEccent )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEccent);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EccentToFlat )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ double dEccent,
            /* [out] */ double __RPC_FAR *pdFlat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EquatorialFromPolarFlat )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ double dPolarRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEquatorialRadius);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PolarFromEquatorialFlat )( 
            ICsEllipsoidDef __RPC_FAR * This,
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdPolarRadius);
        
        END_INTERFACE
    } ICsEllipsoidDefVtbl;

    interface ICsEllipsoidDef
    {
        CONST_VTBL struct ICsEllipsoidDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEllipsoidDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEllipsoidDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEllipsoidDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEllipsoidDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsEllipsoidDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsEllipsoidDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsEllipsoidDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsEllipsoidDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsEllipsoidDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsEllipsoidDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)


#define ICsEllipsoidDef_GetDescription(This,ppDesc)	\
    (This)->lpVtbl -> GetDescription(This,ppDesc)

#define ICsEllipsoidDef_SetDescription(This,kpDesc,pErr)	\
    (This)->lpVtbl -> SetDescription(This,kpDesc,pErr)

#define ICsEllipsoidDef_IsLegalDescription(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalDescription(This,kpDesc,pbIsLegal)

#define ICsEllipsoidDef_GetGroup(This,ppGroup)	\
    (This)->lpVtbl -> GetGroup(This,ppGroup)

#define ICsEllipsoidDef_SetGroup(This,kpGroup,pErr)	\
    (This)->lpVtbl -> SetGroup(This,kpGroup,pErr)

#define ICsEllipsoidDef_IsLegalGroup(This,kpGroup,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalGroup(This,kpGroup,pbIsLegal)

#define ICsEllipsoidDef_GetSource(This,ppSource)	\
    (This)->lpVtbl -> GetSource(This,ppSource)

#define ICsEllipsoidDef_SetSource(This,kpSource,pErr)	\
    (This)->lpVtbl -> SetSource(This,kpSource,pErr)

#define ICsEllipsoidDef_IsLegalSource(This,kpSource,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalSource(This,kpSource,pbIsLegal)

#define ICsEllipsoidDef_IsProtected(This,pbIsProtected)	\
    (This)->lpVtbl -> IsProtected(This,pbIsProtected)

#define ICsEllipsoidDef_GetAge(This,psAge)	\
    (This)->lpVtbl -> GetAge(This,psAge)

#define ICsEllipsoidDef_SetProtectMode(This,bIsProtected)	\
    (This)->lpVtbl -> SetProtectMode(This,bIsProtected)

#define ICsEllipsoidDef_IsEncrypted(This,pbIsEncrypted)	\
    (This)->lpVtbl -> IsEncrypted(This,pbIsEncrypted)

#define ICsEllipsoidDef_SetEncryptMode(This,bIsEncrypted)	\
    (This)->lpVtbl -> SetEncryptMode(This,bIsEncrypted)


#define ICsEllipsoidDef_GetEquatorialRadius(This,pdRadius)	\
    (This)->lpVtbl -> GetEquatorialRadius(This,pdRadius)

#define ICsEllipsoidDef_GetPolarRadius(This,pdRadius)	\
    (This)->lpVtbl -> GetPolarRadius(This,pdRadius)

#define ICsEllipsoidDef_GetFlatteningRatio(This,pdFlat)	\
    (This)->lpVtbl -> GetFlatteningRatio(This,pdFlat)

#define ICsEllipsoidDef_GetEccentricity(This,pdEccent)	\
    (This)->lpVtbl -> GetEccentricity(This,pdEccent)

#define ICsEllipsoidDef_SetRadii(This,dEquatorialRadius,dPolarRadius,pErr)	\
    (This)->lpVtbl -> SetRadii(This,dEquatorialRadius,dPolarRadius,pErr)

#define ICsEllipsoidDef_FlatToEccent(This,dFlat,pdEccent)	\
    (This)->lpVtbl -> FlatToEccent(This,dFlat,pdEccent)

#define ICsEllipsoidDef_EccentToFlat(This,dEccent,pdFlat)	\
    (This)->lpVtbl -> EccentToFlat(This,dEccent,pdFlat)

#define ICsEllipsoidDef_EquatorialFromPolarFlat(This,dPolarRadius,dFlat,pdEquatorialRadius)	\
    (This)->lpVtbl -> EquatorialFromPolarFlat(This,dPolarRadius,dFlat,pdEquatorialRadius)

#define ICsEllipsoidDef_PolarFromEquatorialFlat(This,dEquatorialRadius,dFlat,pdPolarRadius)	\
    (This)->lpVtbl -> PolarFromEquatorialFlat(This,dEquatorialRadius,dFlat,pdPolarRadius)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_GetEquatorialRadius_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdRadius);


void __RPC_STUB ICsEllipsoidDef_GetEquatorialRadius_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_GetPolarRadius_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdRadius);


void __RPC_STUB ICsEllipsoidDef_GetPolarRadius_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_GetFlatteningRatio_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdFlat);


void __RPC_STUB ICsEllipsoidDef_GetFlatteningRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_GetEccentricity_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdEccent);


void __RPC_STUB ICsEllipsoidDef_GetEccentricity_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_SetRadii_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [in] */ double dEquatorialRadius,
    /* [in] */ double dPolarRadius,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsEllipsoidDef_SetRadii_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_FlatToEccent_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdEccent);


void __RPC_STUB ICsEllipsoidDef_FlatToEccent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_EccentToFlat_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [in] */ double dEccent,
    /* [out] */ double __RPC_FAR *pdFlat);


void __RPC_STUB ICsEllipsoidDef_EccentToFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_EquatorialFromPolarFlat_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [in] */ double dPolarRadius,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdEquatorialRadius);


void __RPC_STUB ICsEllipsoidDef_EquatorialFromPolarFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDef_PolarFromEquatorialFlat_Proxy( 
    ICsEllipsoidDef __RPC_FAR * This,
    /* [in] */ double dEquatorialRadius,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdPolarRadius);


void __RPC_STUB ICsEllipsoidDef_PolarFromEquatorialFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEllipsoidDef_INTERFACE_DEFINED__ */


#ifndef __ICsDatumDef_INTERFACE_DEFINED__
#define __ICsDatumDef_INTERFACE_DEFINED__

/* interface ICsDatumDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatumDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4E01DB5-0D13-11D2-9DE2-080009ACE18E")
    ICsDatumDef : public ICsMentorDef
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLocation( 
            /* [out] */ BSTR __RPC_FAR *ppLoc) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetLocation( 
            /* [in] */ const BSTR kpLoc,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalLocation( 
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCntrySt( 
            /* [out] */ BSTR __RPC_FAR *ppCntrySt) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetCntrySt( 
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalCntrySt( 
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoidName( 
            /* [out] */ BSTR __RPC_FAR *ppEllipsoid) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetEllipsoidName( 
            /* [in] */ const BSTR kpEllipsoid,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetOffset( 
            /* [out] */ double __RPC_FAR *pdX,
            /* [out] */ double __RPC_FAR *pdY,
            /* [out] */ double __RPC_FAR *pdZ) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetOffset( 
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [in] */ double dZ,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetBursaWolfeTransform( 
            /* [out] */ double __RPC_FAR *pdRotationX,
            /* [out] */ double __RPC_FAR *pdRotationY,
            /* [out] */ double __RPC_FAR *pdRotationZ,
            /* [out] */ double __RPC_FAR *pdBwScale) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetBursaWolfeTransform( 
            /* [in] */ double dRotationX,
            /* [in] */ double dRotationY,
            /* [in] */ double dRotationZ,
            /* [in] */ double dBwScale,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTransformationMethod( 
            /* [out] */ CsDatumConvert __RPC_FAR *pdwConvert) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetTransformationMethod( 
            /* [in] */ CsDatumConvert convert,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatumDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatumDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDesc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetDescription )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalDescription )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetGroup )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppGroup);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGroup )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalGroup )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpGroup,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSource )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppSource);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSource )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalSource )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpSource,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsProtected )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAge )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ short __RPC_FAR *psAge);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProtectMode )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ BOOL bIsProtected);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsEncrypted )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEncryptMode )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ BOOL bIsEncrypted);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLocation )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppLoc);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetLocation )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpLoc,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalLocation )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpDesc,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCntrySt )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppCntrySt);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCntrySt )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalCntrySt )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpCntrySt,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoidName )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetEllipsoidName )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ const BSTR kpEllipsoid,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOffset )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdX,
            /* [out] */ double __RPC_FAR *pdY,
            /* [out] */ double __RPC_FAR *pdZ);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOffset )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ double dX,
            /* [in] */ double dY,
            /* [in] */ double dZ,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBursaWolfeTransform )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdRotationX,
            /* [out] */ double __RPC_FAR *pdRotationY,
            /* [out] */ double __RPC_FAR *pdRotationZ,
            /* [out] */ double __RPC_FAR *pdBwScale);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetBursaWolfeTransform )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ double dRotationX,
            /* [in] */ double dRotationY,
            /* [in] */ double dRotationZ,
            /* [in] */ double dBwScale,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTransformationMethod )( 
            ICsDatumDef __RPC_FAR * This,
            /* [out] */ CsDatumConvert __RPC_FAR *pdwConvert);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetTransformationMethod )( 
            ICsDatumDef __RPC_FAR * This,
            /* [in] */ CsDatumConvert convert,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsDatumDefVtbl;

    interface ICsDatumDef
    {
        CONST_VTBL struct ICsDatumDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatumDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatumDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatumDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatumDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsDatumDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsDatumDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsDatumDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsDatumDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsDatumDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsDatumDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)


#define ICsDatumDef_GetDescription(This,ppDesc)	\
    (This)->lpVtbl -> GetDescription(This,ppDesc)

#define ICsDatumDef_SetDescription(This,kpDesc,pErr)	\
    (This)->lpVtbl -> SetDescription(This,kpDesc,pErr)

#define ICsDatumDef_IsLegalDescription(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalDescription(This,kpDesc,pbIsLegal)

#define ICsDatumDef_GetGroup(This,ppGroup)	\
    (This)->lpVtbl -> GetGroup(This,ppGroup)

#define ICsDatumDef_SetGroup(This,kpGroup,pErr)	\
    (This)->lpVtbl -> SetGroup(This,kpGroup,pErr)

#define ICsDatumDef_IsLegalGroup(This,kpGroup,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalGroup(This,kpGroup,pbIsLegal)

#define ICsDatumDef_GetSource(This,ppSource)	\
    (This)->lpVtbl -> GetSource(This,ppSource)

#define ICsDatumDef_SetSource(This,kpSource,pErr)	\
    (This)->lpVtbl -> SetSource(This,kpSource,pErr)

#define ICsDatumDef_IsLegalSource(This,kpSource,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalSource(This,kpSource,pbIsLegal)

#define ICsDatumDef_IsProtected(This,pbIsProtected)	\
    (This)->lpVtbl -> IsProtected(This,pbIsProtected)

#define ICsDatumDef_GetAge(This,psAge)	\
    (This)->lpVtbl -> GetAge(This,psAge)

#define ICsDatumDef_SetProtectMode(This,bIsProtected)	\
    (This)->lpVtbl -> SetProtectMode(This,bIsProtected)

#define ICsDatumDef_IsEncrypted(This,pbIsEncrypted)	\
    (This)->lpVtbl -> IsEncrypted(This,pbIsEncrypted)

#define ICsDatumDef_SetEncryptMode(This,bIsEncrypted)	\
    (This)->lpVtbl -> SetEncryptMode(This,bIsEncrypted)


#define ICsDatumDef_GetLocation(This,ppLoc)	\
    (This)->lpVtbl -> GetLocation(This,ppLoc)

#define ICsDatumDef_SetLocation(This,kpLoc,pErr)	\
    (This)->lpVtbl -> SetLocation(This,kpLoc,pErr)

#define ICsDatumDef_IsLegalLocation(This,kpDesc,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalLocation(This,kpDesc,pbIsLegal)

#define ICsDatumDef_GetCntrySt(This,ppCntrySt)	\
    (This)->lpVtbl -> GetCntrySt(This,ppCntrySt)

#define ICsDatumDef_SetCntrySt(This,kpCntrySt,pErr)	\
    (This)->lpVtbl -> SetCntrySt(This,kpCntrySt,pErr)

#define ICsDatumDef_IsLegalCntrySt(This,kpCntrySt,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalCntrySt(This,kpCntrySt,pbIsLegal)

#define ICsDatumDef_GetEllipsoidName(This,ppEllipsoid)	\
    (This)->lpVtbl -> GetEllipsoidName(This,ppEllipsoid)

#define ICsDatumDef_SetEllipsoidName(This,kpEllipsoid,pErr)	\
    (This)->lpVtbl -> SetEllipsoidName(This,kpEllipsoid,pErr)

#define ICsDatumDef_GetOffset(This,pdX,pdY,pdZ)	\
    (This)->lpVtbl -> GetOffset(This,pdX,pdY,pdZ)

#define ICsDatumDef_SetOffset(This,dX,dY,dZ,pErr)	\
    (This)->lpVtbl -> SetOffset(This,dX,dY,dZ,pErr)

#define ICsDatumDef_GetBursaWolfeTransform(This,pdRotationX,pdRotationY,pdRotationZ,pdBwScale)	\
    (This)->lpVtbl -> GetBursaWolfeTransform(This,pdRotationX,pdRotationY,pdRotationZ,pdBwScale)

#define ICsDatumDef_SetBursaWolfeTransform(This,dRotationX,dRotationY,dRotationZ,dBwScale,pErr)	\
    (This)->lpVtbl -> SetBursaWolfeTransform(This,dRotationX,dRotationY,dRotationZ,dBwScale,pErr)

#define ICsDatumDef_GetTransformationMethod(This,pdwConvert)	\
    (This)->lpVtbl -> GetTransformationMethod(This,pdwConvert)

#define ICsDatumDef_SetTransformationMethod(This,convert,pErr)	\
    (This)->lpVtbl -> SetTransformationMethod(This,convert,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetLocation_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppLoc);


void __RPC_STUB ICsDatumDef_GetLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetLocation_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ const BSTR kpLoc,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_IsLegalLocation_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ const BSTR kpDesc,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsDatumDef_IsLegalLocation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetCntrySt_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppCntrySt);


void __RPC_STUB ICsDatumDef_GetCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetCntrySt_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ const BSTR kpCntrySt,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_IsLegalCntrySt_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ const BSTR kpCntrySt,
    /* [out] */ BOOL __RPC_FAR *pbIsLegal);


void __RPC_STUB ICsDatumDef_IsLegalCntrySt_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetEllipsoidName_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppEllipsoid);


void __RPC_STUB ICsDatumDef_GetEllipsoidName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetEllipsoidName_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ const BSTR kpEllipsoid,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetEllipsoidName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetOffset_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdX,
    /* [out] */ double __RPC_FAR *pdY,
    /* [out] */ double __RPC_FAR *pdZ);


void __RPC_STUB ICsDatumDef_GetOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetOffset_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ double dX,
    /* [in] */ double dY,
    /* [in] */ double dZ,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetBursaWolfeTransform_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdRotationX,
    /* [out] */ double __RPC_FAR *pdRotationY,
    /* [out] */ double __RPC_FAR *pdRotationZ,
    /* [out] */ double __RPC_FAR *pdBwScale);


void __RPC_STUB ICsDatumDef_GetBursaWolfeTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetBursaWolfeTransform_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ double dRotationX,
    /* [in] */ double dRotationY,
    /* [in] */ double dRotationZ,
    /* [in] */ double dBwScale,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetBursaWolfeTransform_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_GetTransformationMethod_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [out] */ CsDatumConvert __RPC_FAR *pdwConvert);


void __RPC_STUB ICsDatumDef_GetTransformationMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDef_SetTransformationMethod_Proxy( 
    ICsDatumDef __RPC_FAR * This,
    /* [in] */ CsDatumConvert convert,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDef_SetTransformationMethod_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatumDef_INTERFACE_DEFINED__ */


#ifndef __ICsCategoryDef_INTERFACE_DEFINED__
#define __ICsCategoryDef_INTERFACE_DEFINED__

/* interface ICsCategoryDef */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCategoryDef;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4E01DB9-0D13-11D2-9DE2-080009ACE18E")
    ICsCategoryDef : public ICsDef
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ DWORD __RPC_FAR *pdwSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHasCoordsys) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clear( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCategoryDefVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCategoryDef __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCategoryDef __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalName )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbIsLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsValid )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbIsValid);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsUsable )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ ICsSession __RPC_FAR *pSession,
            /* [out] */ BOOL __RPC_FAR *pbIsUsable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsSameAs )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbIsSame);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppClone);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddCoordsys )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveCoordsys )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasCoordsys )( 
            ICsCategoryDef __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHasCoordsys);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsCategoryDef __RPC_FAR * This);
        
        END_INTERFACE
    } ICsCategoryDefVtbl;

    interface ICsCategoryDef
    {
        CONST_VTBL struct ICsCategoryDefVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCategoryDef_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCategoryDef_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCategoryDef_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCategoryDef_GetName(This,ppName)	\
    (This)->lpVtbl -> GetName(This,ppName)

#define ICsCategoryDef_SetName(This,kpName,pErr)	\
    (This)->lpVtbl -> SetName(This,kpName,pErr)

#define ICsCategoryDef_IsLegalName(This,kpName,pbIsLegal)	\
    (This)->lpVtbl -> IsLegalName(This,kpName,pbIsLegal)

#define ICsCategoryDef_IsValid(This,pbIsValid)	\
    (This)->lpVtbl -> IsValid(This,pbIsValid)

#define ICsCategoryDef_IsUsable(This,pSession,pbIsUsable)	\
    (This)->lpVtbl -> IsUsable(This,pSession,pbIsUsable)

#define ICsCategoryDef_IsSameAs(This,pDef,pbIsSame)	\
    (This)->lpVtbl -> IsSameAs(This,pDef,pbIsSame)

#define ICsCategoryDef_Clone(This,ppClone)	\
    (This)->lpVtbl -> Clone(This,ppClone)


#define ICsCategoryDef_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsCategoryDef_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsCategoryDef_AddCoordsys(This,kpName,pErr)	\
    (This)->lpVtbl -> AddCoordsys(This,kpName,pErr)

#define ICsCategoryDef_RemoveCoordsys(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveCoordsys(This,kpName,pErr)

#define ICsCategoryDef_HasCoordsys(This,kpName,pbHasCoordsys)	\
    (This)->lpVtbl -> HasCoordsys(This,kpName,pbHasCoordsys)

#define ICsCategoryDef_Clear(This)	\
    (This)->lpVtbl -> Clear(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_GetSize_Proxy( 
    ICsCategoryDef __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwSize);


void __RPC_STUB ICsCategoryDef_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_GetEnum_Proxy( 
    ICsCategoryDef __RPC_FAR * This,
    /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsCategoryDef_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_AddCoordsys_Proxy( 
    ICsCategoryDef __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDef_AddCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_RemoveCoordsys_Proxy( 
    ICsCategoryDef __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDef_RemoveCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_HasCoordsys_Proxy( 
    ICsCategoryDef __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHasCoordsys);


void __RPC_STUB ICsCategoryDef_HasCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDef_Clear_Proxy( 
    ICsCategoryDef __RPC_FAR * This);


void __RPC_STUB ICsCategoryDef_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCategoryDef_INTERFACE_DEFINED__ */


#ifndef __ICsEnumName_INTERFACE_DEFINED__
#define __ICsEnumName_INTERFACE_DEFINED__

/* interface ICsEnumName */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumName;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B4E01DBB-0D13-11D2-9DE2-080009ACE18E")
    ICsEnumName : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR *names,
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumNameVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumName __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumName __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumName __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumName __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR *names,
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumName __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumName __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumName __RPC_FAR * This,
            /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsEnumNameVtbl;

    interface ICsEnumName
    {
        CONST_VTBL struct ICsEnumNameVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumName_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumName_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumName_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumName_Next(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,names,pulFetched)

#define ICsEnumName_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumName_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumName_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumName_Next_Proxy( 
    ICsEnumName __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR *names,
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumName_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumName_Skip_Proxy( 
    ICsEnumName __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumName_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumName_Reset_Proxy( 
    ICsEnumName __RPC_FAR * This);


void __RPC_STUB ICsEnumName_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumName_Clone_Proxy( 
    ICsEnumName __RPC_FAR * This,
    /* [out] */ ICsEnumName __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumName_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumName_INTERFACE_DEFINED__ */


#ifndef __ICsEnumDword_INTERFACE_DEFINED__
#define __ICsEnumDword_INTERFACE_DEFINED__

/* interface ICsEnumDword */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumDword;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E3CD1C1-0D38-11D2-9DE3-080009ACE18E")
    ICsEnumDword : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ DWORD __RPC_FAR *defs,
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumDwordVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumDword __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumDword __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumDword __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumDword __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ DWORD __RPC_FAR *defs,
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumDword __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumDword __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumDword __RPC_FAR * This,
            /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsEnumDwordVtbl;

    interface ICsEnumDword
    {
        CONST_VTBL struct ICsEnumDwordVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumDword_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumDword_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumDword_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumDword_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumDword_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumDword_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumDword_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDword_Next_Proxy( 
    ICsEnumDword __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ DWORD __RPC_FAR *defs,
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumDword_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDword_Skip_Proxy( 
    ICsEnumDword __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumDword_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDword_Reset_Proxy( 
    ICsEnumDword __RPC_FAR * This);


void __RPC_STUB ICsEnumDword_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDword_Clone_Proxy( 
    ICsEnumDword __RPC_FAR * This,
    /* [out] */ ICsEnumDword __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumDword_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumDword_INTERFACE_DEFINED__ */


#ifndef __ICsDefSet_INTERFACE_DEFINED__
#define __ICsDefSet_INTERFACE_DEFINED__

/* interface ICsDefSet */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDefSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C504CE26-0D44-11d2-9DE3-080009ACE18E")
    ICsDefSet : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ DWORD __RPC_FAR *pdwSize) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddDef( 
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveDef( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyDef( 
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDef( 
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasDef( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsWritable( 
            /* [out] */ BOOL __RPC_FAR *pbWritable) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clear( 
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDefSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDefSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDefSet __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsDefSet __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDef )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDef )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDef )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDef )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDef )( 
            ICsDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsDefSet __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            ICsDefSet __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsDefSet __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsDefSetVtbl;

    interface ICsDefSet
    {
        CONST_VTBL struct ICsDefSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDefSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDefSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDefSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDefSet_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsDefSet_AddDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDef(This,kpDef,pErr)

#define ICsDefSet_RemoveDef(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDef(This,kpName,pErr)

#define ICsDefSet_ModifyDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDef(This,kpDef,pErr)

#define ICsDefSet_GetDef(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDef(This,kpName,ppDef,pErr)

#define ICsDefSet_HasDef(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDef(This,kpName,pbHas)

#define ICsDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsDefSet_IsWritable(This,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,pbWritable)

#define ICsDefSet_Clear(This,pErr)	\
    (This)->lpVtbl -> Clear(This,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_GetSize_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwSize);


void __RPC_STUB ICsDefSet_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_AddDef_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [in] */ ICsDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDefSet_AddDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_RemoveDef_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDefSet_RemoveDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_ModifyDef_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [in] */ ICsDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDefSet_ModifyDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_GetDef_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDefSet_GetDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_HasDef_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHas);


void __RPC_STUB ICsDefSet_HasDef_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_GetEnum_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsDefSet_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_IsWritable_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pbWritable);


void __RPC_STUB ICsDefSet_IsWritable_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDefSet_Clear_Proxy( 
    ICsDefSet __RPC_FAR * This,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDefSet_Clear_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDefSet_INTERFACE_DEFINED__ */


#ifndef __ICsSelector_INTERFACE_DEFINED__
#define __ICsSelector_INTERFACE_DEFINED__

/* interface ICsSelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsSelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1EFE013-0F71-11d2-9DE3-080009ACE18E")
    ICsSelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsSelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsSelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsSelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsSelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsSelector __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsSelectorVtbl;

    interface ICsSelector
    {
        CONST_VTBL struct ICsSelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsSelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsSelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsSelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsSelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsSelector_Eval_Proxy( 
    ICsSelector __RPC_FAR * This,
    /* [in] */ ICsDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsSelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsSelector_INTERFACE_DEFINED__ */


#ifndef __ICsMentorSelector_INTERFACE_DEFINED__
#define __ICsMentorSelector_INTERFACE_DEFINED__

/* interface ICsMentorSelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsMentorSelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1EFE015-0F71-11d2-9DE3-080009ACE18E")
    ICsMentorSelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsMentorDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsMentorSelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsMentorSelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsMentorSelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsMentorSelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsMentorSelector __RPC_FAR * This,
            /* [in] */ ICsMentorDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsMentorSelectorVtbl;

    interface ICsMentorSelector
    {
        CONST_VTBL struct ICsMentorSelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsMentorSelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsMentorSelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsMentorSelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsMentorSelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsMentorSelector_Eval_Proxy( 
    ICsMentorSelector __RPC_FAR * This,
    /* [in] */ ICsMentorDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsMentorSelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsMentorSelector_INTERFACE_DEFINED__ */


#ifndef __ICsEnum_INTERFACE_DEFINED__
#define __ICsEnum_INTERFACE_DEFINED__

/* interface ICsEnum */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1EFE016-0F71-11d2-9DE3-080009ACE18E")
    ICsEnum : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsSelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnum __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnum __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnum __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnum __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnum __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnum __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnum __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnum __RPC_FAR * This,
            /* [in] */ ICsSelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnum __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumVtbl;

    interface ICsEnum
    {
        CONST_VTBL struct ICsEnumVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnum_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnum_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnum_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnum_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_Next_Proxy( 
    ICsEnum __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_Skip_Proxy( 
    ICsEnum __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnum_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_Reset_Proxy( 
    ICsEnum __RPC_FAR * This);


void __RPC_STUB ICsEnum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_Clone_Proxy( 
    ICsEnum __RPC_FAR * This,
    /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnum_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_SetSelector_Proxy( 
    ICsEnum __RPC_FAR * This,
    /* [in] */ ICsSelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnum_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnum_NextName_Proxy( 
    ICsEnum __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnum_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnum_INTERFACE_DEFINED__ */


#ifndef __ICsEnumMentor_INTERFACE_DEFINED__
#define __ICsEnumMentor_INTERFACE_DEFINED__

/* interface ICsEnumMentor */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumMentor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1EFE019-0F71-11d2-9DE3-080009ACE18E")
    ICsEnumMentor : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsMentorDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumMentor __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsMentorSelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextDescription( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumMentorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumMentor __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumMentor __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsMentorDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumMentor __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [out] */ ICsEnumMentor __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ ICsMentorSelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextDescription )( 
            ICsEnumMentor __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumMentorVtbl;

    interface ICsEnumMentor
    {
        CONST_VTBL struct ICsEnumMentorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumMentor_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumMentor_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumMentor_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumMentor_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumMentor_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumMentor_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumMentor_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnumMentor_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnumMentor_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#define ICsEnumMentor_NextDescription(This,ulCount,descriptions,pulFetched)	\
    (This)->lpVtbl -> NextDescription(This,ulCount,descriptions,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_Next_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsMentorDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumMentor_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_Skip_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumMentor_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_Reset_Proxy( 
    ICsEnumMentor __RPC_FAR * This);


void __RPC_STUB ICsEnumMentor_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_Clone_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [out] */ ICsEnumMentor __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumMentor_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_SetSelector_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [in] */ ICsMentorSelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnumMentor_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_NextName_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumMentor_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumMentor_NextDescription_Proxy( 
    ICsEnumMentor __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumMentor_NextDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumMentor_INTERFACE_DEFINED__ */


#ifndef __ICsEllipsoidDefSet_INTERFACE_DEFINED__
#define __ICsEllipsoidDefSet_INTERFACE_DEFINED__

/* interface ICsEllipsoidDefSet */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEllipsoidDefSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E3CD1C3-0D38-11D2-9DE3-080009ACE18E")
    ICsEllipsoidDefSet : public ICsDefSet
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddEllipsoid( 
            /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveEllipsoid( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyEllipsoid( 
            /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEllipsoid( 
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasEllipsoid( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEllipsoidDefSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEllipsoidDefSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEllipsoidDefSet __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDef )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDef )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDef )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDef )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDef )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddEllipsoid )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveEllipsoid )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyEllipsoid )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEllipsoid )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasEllipsoid )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsEllipsoidDefSet __RPC_FAR * This,
            /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsEllipsoidDefSetVtbl;

    interface ICsEllipsoidDefSet
    {
        CONST_VTBL struct ICsEllipsoidDefSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEllipsoidDefSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEllipsoidDefSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEllipsoidDefSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEllipsoidDefSet_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsEllipsoidDefSet_AddDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDef(This,kpDef,pErr)

#define ICsEllipsoidDefSet_RemoveDef(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDef(This,kpName,pErr)

#define ICsEllipsoidDefSet_ModifyDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDef(This,kpDef,pErr)

#define ICsEllipsoidDefSet_GetDef(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDef(This,kpName,ppDef,pErr)

#define ICsEllipsoidDefSet_HasDef(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDef(This,kpName,pbHas)

#define ICsEllipsoidDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsEllipsoidDefSet_IsWritable(This,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,pbWritable)

#define ICsEllipsoidDefSet_Clear(This,pErr)	\
    (This)->lpVtbl -> Clear(This,pErr)


#define ICsEllipsoidDefSet_AddEllipsoid(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddEllipsoid(This,kpDef,pErr)

#define ICsEllipsoidDefSet_RemoveEllipsoid(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveEllipsoid(This,kpName,pErr)

#define ICsEllipsoidDefSet_ModifyEllipsoid(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyEllipsoid(This,kpDef,pErr)

#define ICsEllipsoidDefSet_GetEllipsoid(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetEllipsoid(This,kpName,ppDef,pErr)

#define ICsEllipsoidDefSet_HasEllipsoid(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasEllipsoid(This,kpName,pbHas)

#define ICsEllipsoidDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_AddEllipsoid_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsEllipsoidDefSet_AddEllipsoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_RemoveEllipsoid_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsEllipsoidDefSet_RemoveEllipsoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_ModifyEllipsoid_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [in] */ ICsEllipsoidDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsEllipsoidDefSet_ModifyEllipsoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_GetEllipsoid_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR *ppDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsEllipsoidDefSet_GetEllipsoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_HasEllipsoid_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHas);


void __RPC_STUB ICsEllipsoidDefSet_HasEllipsoid_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidDefSet_GetEnum_Proxy( 
    ICsEllipsoidDefSet __RPC_FAR * This,
    /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEllipsoidDefSet_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEllipsoidDefSet_INTERFACE_DEFINED__ */


#ifndef __ICsEllipsoidSelector_INTERFACE_DEFINED__
#define __ICsEllipsoidSelector_INTERFACE_DEFINED__

/* interface ICsEllipsoidSelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEllipsoidSelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E0369C95-0F68-11d2-9DE3-080009ACE18E")
    ICsEllipsoidSelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEllipsoidSelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEllipsoidSelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEllipsoidSelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEllipsoidSelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsEllipsoidSelector __RPC_FAR * This,
            /* [in] */ ICsEllipsoidDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsEllipsoidSelectorVtbl;

    interface ICsEllipsoidSelector
    {
        CONST_VTBL struct ICsEllipsoidSelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEllipsoidSelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEllipsoidSelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEllipsoidSelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEllipsoidSelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidSelector_Eval_Proxy( 
    ICsEllipsoidSelector __RPC_FAR * This,
    /* [in] */ ICsEllipsoidDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsEllipsoidSelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEllipsoidSelector_INTERFACE_DEFINED__ */


#ifndef __ICsEnumEllipsoid_INTERFACE_DEFINED__
#define __ICsEnumEllipsoid_INTERFACE_DEFINED__

/* interface ICsEnumEllipsoid */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumEllipsoid;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E3CD1C5-0D38-11D2-9DE3-080009ACE18E")
    ICsEnumEllipsoid : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsEllipsoidSelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextDescription( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumEllipsoidVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumEllipsoid __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumEllipsoid __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumEllipsoid __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ ICsEllipsoidSelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextDescription )( 
            ICsEnumEllipsoid __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumEllipsoidVtbl;

    interface ICsEnumEllipsoid
    {
        CONST_VTBL struct ICsEnumEllipsoidVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumEllipsoid_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumEllipsoid_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumEllipsoid_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumEllipsoid_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumEllipsoid_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumEllipsoid_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumEllipsoid_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnumEllipsoid_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnumEllipsoid_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#define ICsEnumEllipsoid_NextDescription(This,ulCount,descriptions,pulFetched)	\
    (This)->lpVtbl -> NextDescription(This,ulCount,descriptions,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_Next_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsEllipsoidDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumEllipsoid_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_Skip_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumEllipsoid_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_Reset_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This);


void __RPC_STUB ICsEnumEllipsoid_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_Clone_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [out] */ ICsEnumEllipsoid __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumEllipsoid_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_SetSelector_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [in] */ ICsEllipsoidSelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnumEllipsoid_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_NextName_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumEllipsoid_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumEllipsoid_NextDescription_Proxy( 
    ICsEnumEllipsoid __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumEllipsoid_NextDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumEllipsoid_INTERFACE_DEFINED__ */


#ifndef __ICsDatumDefSet_INTERFACE_DEFINED__
#define __ICsDatumDefSet_INTERFACE_DEFINED__

/* interface ICsDatumDefSet */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatumDefSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5E3CD1C7-0D38-11D2-9DE3-080009ACE18E")
    ICsDatumDefSet : public ICsDefSet
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddDatum( 
            /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveDatum( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyDatum( 
            /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetDatum( 
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasDatum( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumDefSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatumDefSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatumDefSet __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDef )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDef )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDef )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDef )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDef )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDatum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDatum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDatum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDatum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDatum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsDatumDefSet __RPC_FAR * This,
            /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsDatumDefSetVtbl;

    interface ICsDatumDefSet
    {
        CONST_VTBL struct ICsDatumDefSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatumDefSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatumDefSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatumDefSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatumDefSet_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsDatumDefSet_AddDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDef(This,kpDef,pErr)

#define ICsDatumDefSet_RemoveDef(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDef(This,kpName,pErr)

#define ICsDatumDefSet_ModifyDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDef(This,kpDef,pErr)

#define ICsDatumDefSet_GetDef(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDef(This,kpName,ppDef,pErr)

#define ICsDatumDefSet_HasDef(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDef(This,kpName,pbHas)

#define ICsDatumDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsDatumDefSet_IsWritable(This,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,pbWritable)

#define ICsDatumDefSet_Clear(This,pErr)	\
    (This)->lpVtbl -> Clear(This,pErr)


#define ICsDatumDefSet_AddDatum(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDatum(This,kpDef,pErr)

#define ICsDatumDefSet_RemoveDatum(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDatum(This,kpName,pErr)

#define ICsDatumDefSet_ModifyDatum(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDatum(This,kpDef,pErr)

#define ICsDatumDefSet_GetDatum(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDatum(This,kpName,ppDef,pErr)

#define ICsDatumDefSet_HasDatum(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDatum(This,kpName,pbHas)

#define ICsDatumDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_AddDatum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDefSet_AddDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_RemoveDatum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDefSet_RemoveDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_ModifyDatum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDefSet_ModifyDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_GetDatum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ ICsDatumDef __RPC_FAR *__RPC_FAR *ppDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDatumDefSet_GetDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_HasDatum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHas);


void __RPC_STUB ICsDatumDefSet_HasDatum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumDefSet_GetEnum_Proxy( 
    ICsDatumDefSet __RPC_FAR * This,
    /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsDatumDefSet_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatumDefSet_INTERFACE_DEFINED__ */


#ifndef __ICsDatumSelector_INTERFACE_DEFINED__
#define __ICsDatumSelector_INTERFACE_DEFINED__

/* interface ICsDatumSelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatumSelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("DAE8ACE0-0F6C-11d2-9DE3-080009ACE18E")
    ICsDatumSelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsDatumDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumSelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatumSelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatumSelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatumSelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsDatumSelector __RPC_FAR * This,
            /* [in] */ ICsDatumDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsDatumSelectorVtbl;

    interface ICsDatumSelector
    {
        CONST_VTBL struct ICsDatumSelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatumSelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatumSelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatumSelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatumSelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumSelector_Eval_Proxy( 
    ICsDatumSelector __RPC_FAR * This,
    /* [in] */ ICsDatumDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsDatumSelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatumSelector_INTERFACE_DEFINED__ */


#ifndef __ICsEnumDatum_INTERFACE_DEFINED__
#define __ICsEnumDatum_INTERFACE_DEFINED__

/* interface ICsEnumDatum */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumDatum;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7891123-0F6A-11D2-9DE3-080009ACE18E")
    ICsEnumDatum : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsDatumDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsDatumSelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextDescription( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumDatumVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumDatum __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumDatum __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsDatumDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumDatum __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ ICsDatumSelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextDescription )( 
            ICsEnumDatum __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumDatumVtbl;

    interface ICsEnumDatum
    {
        CONST_VTBL struct ICsEnumDatumVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumDatum_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumDatum_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumDatum_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumDatum_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumDatum_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumDatum_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumDatum_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnumDatum_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnumDatum_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#define ICsEnumDatum_NextDescription(This,ulCount,descriptions,pulFetched)	\
    (This)->lpVtbl -> NextDescription(This,ulCount,descriptions,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_Next_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsDatumDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumDatum_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_Skip_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumDatum_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_Reset_Proxy( 
    ICsEnumDatum __RPC_FAR * This);


void __RPC_STUB ICsEnumDatum_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_Clone_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [out] */ ICsEnumDatum __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumDatum_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_SetSelector_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [in] */ ICsDatumSelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnumDatum_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_NextName_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumDatum_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumDatum_NextDescription_Proxy( 
    ICsEnumDatum __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumDatum_NextDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumDatum_INTERFACE_DEFINED__ */


#ifndef __ICsCoordsysDefSet_INTERFACE_DEFINED__
#define __ICsCoordsysDefSet_INTERFACE_DEFINED__

/* interface ICsCoordsysDefSet */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCoordsysDefSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7891125-0F6A-11D2-9DE3-080009ACE18E")
    ICsCoordsysDefSet : public ICsDefSet
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddCoordsys( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyCoordsys( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasCoordsys( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCoordsysDefSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCoordsysDefSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCoordsysDefSet __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDef )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDef )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDef )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDef )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDef )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddCoordsys )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveCoordsys )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyCoordsys )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCoordsys )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasCoordsys )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsCoordsysDefSet __RPC_FAR * This,
            /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsCoordsysDefSetVtbl;

    interface ICsCoordsysDefSet
    {
        CONST_VTBL struct ICsCoordsysDefSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCoordsysDefSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCoordsysDefSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCoordsysDefSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCoordsysDefSet_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsCoordsysDefSet_AddDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDef(This,kpDef,pErr)

#define ICsCoordsysDefSet_RemoveDef(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDef(This,kpName,pErr)

#define ICsCoordsysDefSet_ModifyDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDef(This,kpDef,pErr)

#define ICsCoordsysDefSet_GetDef(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDef(This,kpName,ppDef,pErr)

#define ICsCoordsysDefSet_HasDef(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDef(This,kpName,pbHas)

#define ICsCoordsysDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsCoordsysDefSet_IsWritable(This,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,pbWritable)

#define ICsCoordsysDefSet_Clear(This,pErr)	\
    (This)->lpVtbl -> Clear(This,pErr)


#define ICsCoordsysDefSet_AddCoordsys(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddCoordsys(This,kpDef,pErr)

#define ICsCoordsysDefSet_RemoveCoordsys(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveCoordsys(This,kpName,pErr)

#define ICsCoordsysDefSet_ModifyCoordsys(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyCoordsys(This,kpDef,pErr)

#define ICsCoordsysDefSet_GetCoordsys(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetCoordsys(This,kpName,ppDef,pErr)

#define ICsCoordsysDefSet_HasCoordsys(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasCoordsys(This,kpName,pbHas)

#define ICsCoordsysDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_AddCoordsys_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDefSet_AddCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_RemoveCoordsys_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDefSet_RemoveCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_ModifyCoordsys_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDefSet_ModifyCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_GetCoordsys_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR *ppDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCoordsysDefSet_GetCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_HasCoordsys_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHas);


void __RPC_STUB ICsCoordsysDefSet_HasCoordsys_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysDefSet_GetEnum_Proxy( 
    ICsCoordsysDefSet __RPC_FAR * This,
    /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsCoordsysDefSet_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCoordsysDefSet_INTERFACE_DEFINED__ */


#ifndef __ICsCoordsysSelector_INTERFACE_DEFINED__
#define __ICsCoordsysSelector_INTERFACE_DEFINED__

/* interface ICsCoordsysSelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCoordsysSelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("FE4FF4F0-0F6E-11d2-9DE3-080009ACE18E")
    ICsCoordsysSelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsCoordsysDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCoordsysSelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCoordsysSelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCoordsysSelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCoordsysSelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsCoordsysSelector __RPC_FAR * This,
            /* [in] */ ICsCoordsysDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsCoordsysSelectorVtbl;

    interface ICsCoordsysSelector
    {
        CONST_VTBL struct ICsCoordsysSelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCoordsysSelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCoordsysSelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCoordsysSelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCoordsysSelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCoordsysSelector_Eval_Proxy( 
    ICsCoordsysSelector __RPC_FAR * This,
    /* [in] */ ICsCoordsysDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsCoordsysSelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCoordsysSelector_INTERFACE_DEFINED__ */


#ifndef __ICsEnumCoordsys_INTERFACE_DEFINED__
#define __ICsEnumCoordsys_INTERFACE_DEFINED__

/* interface ICsEnumCoordsys */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumCoordsys;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F7891127-0F6A-11D2-9DE3-080009ACE18E")
    ICsEnumCoordsys : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsCoordsysSelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextDescription( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumCoordsysVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumCoordsys __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumCoordsys __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumCoordsys __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ ICsCoordsysSelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextDescription )( 
            ICsEnumCoordsys __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumCoordsysVtbl;

    interface ICsEnumCoordsys
    {
        CONST_VTBL struct ICsEnumCoordsysVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumCoordsys_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumCoordsys_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumCoordsys_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumCoordsys_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumCoordsys_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumCoordsys_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumCoordsys_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnumCoordsys_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnumCoordsys_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#define ICsEnumCoordsys_NextDescription(This,ulCount,descriptions,pulFetched)	\
    (This)->lpVtbl -> NextDescription(This,ulCount,descriptions,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_Next_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsCoordsysDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumCoordsys_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_Skip_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumCoordsys_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_Reset_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This);


void __RPC_STUB ICsEnumCoordsys_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_Clone_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [out] */ ICsEnumCoordsys __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumCoordsys_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_SetSelector_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [in] */ ICsCoordsysSelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnumCoordsys_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_NextName_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumCoordsys_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCoordsys_NextDescription_Proxy( 
    ICsEnumCoordsys __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR descriptions[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumCoordsys_NextDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumCoordsys_INTERFACE_DEFINED__ */


#ifndef __ICsCategoryDefSet_INTERFACE_DEFINED__
#define __ICsCategoryDefSet_INTERFACE_DEFINED__

/* interface ICsCategoryDefSet */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCategoryDefSet;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDF16D91-0F8D-11D2-9DE3-080009ACE18E")
    ICsCategoryDefSet : public ICsDefSet
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE AddCategory( 
            /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RemoveCategory( 
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE ModifyCategory( 
            /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RenameCategory( 
            /* [in] */ const BSTR kpOldName,
            /* [in] */ const BSTR kpNewName,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetCategory( 
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE HasCategory( 
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetEnum( 
            /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCategoryDefSetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCategoryDefSet __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCategoryDefSet __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddDef )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveDef )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyDef )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ ICsDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDef )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasDef )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [out] */ ICsEnum __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsWritable )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pbWritable);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clear )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *AddCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RemoveCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ModifyCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *RenameCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpOldName,
            /* [in] */ const BSTR kpNewName,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR *ppDef,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *HasCategory )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [in] */ const BSTR kpName,
            /* [out] */ BOOL __RPC_FAR *pbHas);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetEnum )( 
            ICsCategoryDefSet __RPC_FAR * This,
            /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsCategoryDefSetVtbl;

    interface ICsCategoryDefSet
    {
        CONST_VTBL struct ICsCategoryDefSetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCategoryDefSet_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCategoryDefSet_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCategoryDefSet_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCategoryDefSet_GetSize(This,pdwSize)	\
    (This)->lpVtbl -> GetSize(This,pdwSize)

#define ICsCategoryDefSet_AddDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddDef(This,kpDef,pErr)

#define ICsCategoryDefSet_RemoveDef(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveDef(This,kpName,pErr)

#define ICsCategoryDefSet_ModifyDef(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyDef(This,kpDef,pErr)

#define ICsCategoryDefSet_GetDef(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetDef(This,kpName,ppDef,pErr)

#define ICsCategoryDefSet_HasDef(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasDef(This,kpName,pbHas)

#define ICsCategoryDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#define ICsCategoryDefSet_IsWritable(This,pbWritable)	\
    (This)->lpVtbl -> IsWritable(This,pbWritable)

#define ICsCategoryDefSet_Clear(This,pErr)	\
    (This)->lpVtbl -> Clear(This,pErr)


#define ICsCategoryDefSet_AddCategory(This,kpDef,pErr)	\
    (This)->lpVtbl -> AddCategory(This,kpDef,pErr)

#define ICsCategoryDefSet_RemoveCategory(This,kpName,pErr)	\
    (This)->lpVtbl -> RemoveCategory(This,kpName,pErr)

#define ICsCategoryDefSet_ModifyCategory(This,kpDef,pErr)	\
    (This)->lpVtbl -> ModifyCategory(This,kpDef,pErr)

#define ICsCategoryDefSet_RenameCategory(This,kpOldName,kpNewName,pErr)	\
    (This)->lpVtbl -> RenameCategory(This,kpOldName,kpNewName,pErr)

#define ICsCategoryDefSet_GetCategory(This,kpName,ppDef,pErr)	\
    (This)->lpVtbl -> GetCategory(This,kpName,ppDef,pErr)

#define ICsCategoryDefSet_HasCategory(This,kpName,pbHas)	\
    (This)->lpVtbl -> HasCategory(This,kpName,pbHas)

#define ICsCategoryDefSet_GetEnum(This,ppEnum)	\
    (This)->lpVtbl -> GetEnum(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_AddCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDefSet_AddCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_RemoveCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDefSet_RemoveCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_ModifyCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ ICsCategoryDef __RPC_FAR *kpDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDefSet_ModifyCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_RenameCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpOldName,
    /* [in] */ const BSTR kpNewName,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDefSet_RenameCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_GetCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR *ppDef,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsCategoryDefSet_GetCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_HasCategory_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [in] */ const BSTR kpName,
    /* [out] */ BOOL __RPC_FAR *pbHas);


void __RPC_STUB ICsCategoryDefSet_HasCategory_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategoryDefSet_GetEnum_Proxy( 
    ICsCategoryDefSet __RPC_FAR * This,
    /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsCategoryDefSet_GetEnum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCategoryDefSet_INTERFACE_DEFINED__ */


#ifndef __ICsCategorySelector_INTERFACE_DEFINED__
#define __ICsCategorySelector_INTERFACE_DEFINED__

/* interface ICsCategorySelector */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsCategorySelector;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E1EFE01C-0F71-11d2-9DE3-080009ACE18E")
    ICsCategorySelector : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Eval( 
            /* [in] */ ICsCategoryDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsCategorySelectorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsCategorySelector __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsCategorySelector __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsCategorySelector __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Eval )( 
            ICsCategorySelector __RPC_FAR * This,
            /* [in] */ ICsCategoryDef __RPC_FAR *pDef,
            /* [out] */ BOOL __RPC_FAR *pbResult);
        
        END_INTERFACE
    } ICsCategorySelectorVtbl;

    interface ICsCategorySelector
    {
        CONST_VTBL struct ICsCategorySelectorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsCategorySelector_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsCategorySelector_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsCategorySelector_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsCategorySelector_Eval(This,pDef,pbResult)	\
    (This)->lpVtbl -> Eval(This,pDef,pbResult)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsCategorySelector_Eval_Proxy( 
    ICsCategorySelector __RPC_FAR * This,
    /* [in] */ ICsCategoryDef __RPC_FAR *pDef,
    /* [out] */ BOOL __RPC_FAR *pbResult);


void __RPC_STUB ICsCategorySelector_Eval_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsCategorySelector_INTERFACE_DEFINED__ */


#ifndef __ICsEnumCategory_INTERFACE_DEFINED__
#define __ICsEnumCategory_INTERFACE_DEFINED__

/* interface ICsEnumCategory */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumCategory;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDF16D93-0F8D-11D2-9DE3-080009ACE18E")
    ICsEnumCategory : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSelector( 
            /* [in] */ ICsCategorySelector __RPC_FAR *pSelector) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE NextName( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumCategoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumCategory __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumCategory __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR defs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumCategory __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSelector )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [in] */ ICsCategorySelector __RPC_FAR *pSelector);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NextName )( 
            ICsEnumCategory __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ BSTR __RPC_FAR names[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        END_INTERFACE
    } ICsEnumCategoryVtbl;

    interface ICsEnumCategory
    {
        CONST_VTBL struct ICsEnumCategoryVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumCategory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumCategory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumCategory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumCategory_Next(This,ulCount,defs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,defs,pulFetched)

#define ICsEnumCategory_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumCategory_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumCategory_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#define ICsEnumCategory_SetSelector(This,pSelector)	\
    (This)->lpVtbl -> SetSelector(This,pSelector)

#define ICsEnumCategory_NextName(This,ulCount,names,pulFetched)	\
    (This)->lpVtbl -> NextName(This,ulCount,names,pulFetched)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_Next_Proxy( 
    ICsEnumCategory __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ ICsCategoryDef __RPC_FAR *__RPC_FAR defs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumCategory_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_Skip_Proxy( 
    ICsEnumCategory __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumCategory_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_Reset_Proxy( 
    ICsEnumCategory __RPC_FAR * This);


void __RPC_STUB ICsEnumCategory_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_Clone_Proxy( 
    ICsEnumCategory __RPC_FAR * This,
    /* [out] */ ICsEnumCategory __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumCategory_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_SetSelector_Proxy( 
    ICsEnumCategory __RPC_FAR * This,
    /* [in] */ ICsCategorySelector __RPC_FAR *pSelector);


void __RPC_STUB ICsEnumCategory_SetSelector_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumCategory_NextName_Proxy( 
    ICsEnumCategory __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ BSTR __RPC_FAR names[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumCategory_NextName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumCategory_INTERFACE_DEFINED__ */


#ifndef __ICsEnumUnit_INTERFACE_DEFINED__
#define __ICsEnumUnit_INTERFACE_DEFINED__

/* interface ICsEnumUnit */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumUnit;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDF16D95-0F8D-11D2-9DE3-080009ACE18E")
    ICsEnumUnit : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ CsUnit __RPC_FAR units[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumUnitVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumUnit __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumUnit __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumUnit __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumUnit __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ CsUnit __RPC_FAR units[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumUnit __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumUnit __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumUnit __RPC_FAR * This,
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsEnumUnitVtbl;

    interface ICsEnumUnit
    {
        CONST_VTBL struct ICsEnumUnitVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumUnit_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumUnit_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumUnit_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumUnit_Next(This,ulCount,units,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,units,pulFetched)

#define ICsEnumUnit_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumUnit_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumUnit_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumUnit_Next_Proxy( 
    ICsEnumUnit __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ CsUnit __RPC_FAR units[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumUnit_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumUnit_Skip_Proxy( 
    ICsEnumUnit __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumUnit_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumUnit_Reset_Proxy( 
    ICsEnumUnit __RPC_FAR * This);


void __RPC_STUB ICsEnumUnit_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumUnit_Clone_Proxy( 
    ICsEnumUnit __RPC_FAR * This,
    /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumUnit_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumUnit_INTERFACE_DEFINED__ */


#ifndef __ICsEnumProjection_INTERFACE_DEFINED__
#define __ICsEnumProjection_INTERFACE_DEFINED__

/* interface ICsEnumProjection */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEnumProjection;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("CDF16D97-0F8D-11D2-9DE3-080009ACE18E")
    ICsEnumProjection : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Next( 
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ CsProjection __RPC_FAR prjs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Skip( 
            /* [in] */ ULONG ulSkipCount) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Reset( void) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Clone( 
            /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEnumProjectionVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEnumProjection __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEnumProjection __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEnumProjection __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Next )( 
            ICsEnumProjection __RPC_FAR * This,
            /* [in] */ ULONG ulCount,
            /* [size_is][out] */ CsProjection __RPC_FAR prjs[  ],
            /* [out] */ ULONG __RPC_FAR *pulFetched);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Skip )( 
            ICsEnumProjection __RPC_FAR * This,
            /* [in] */ ULONG ulSkipCount);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Reset )( 
            ICsEnumProjection __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Clone )( 
            ICsEnumProjection __RPC_FAR * This,
            /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum);
        
        END_INTERFACE
    } ICsEnumProjectionVtbl;

    interface ICsEnumProjection
    {
        CONST_VTBL struct ICsEnumProjectionVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEnumProjection_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEnumProjection_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEnumProjection_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEnumProjection_Next(This,ulCount,prjs,pulFetched)	\
    (This)->lpVtbl -> Next(This,ulCount,prjs,pulFetched)

#define ICsEnumProjection_Skip(This,ulSkipCount)	\
    (This)->lpVtbl -> Skip(This,ulSkipCount)

#define ICsEnumProjection_Reset(This)	\
    (This)->lpVtbl -> Reset(This)

#define ICsEnumProjection_Clone(This,ppEnum)	\
    (This)->lpVtbl -> Clone(This,ppEnum)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumProjection_Next_Proxy( 
    ICsEnumProjection __RPC_FAR * This,
    /* [in] */ ULONG ulCount,
    /* [size_is][out] */ CsProjection __RPC_FAR prjs[  ],
    /* [out] */ ULONG __RPC_FAR *pulFetched);


void __RPC_STUB ICsEnumProjection_Next_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumProjection_Skip_Proxy( 
    ICsEnumProjection __RPC_FAR * This,
    /* [in] */ ULONG ulSkipCount);


void __RPC_STUB ICsEnumProjection_Skip_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumProjection_Reset_Proxy( 
    ICsEnumProjection __RPC_FAR * This);


void __RPC_STUB ICsEnumProjection_Reset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEnumProjection_Clone_Proxy( 
    ICsEnumProjection __RPC_FAR * This,
    /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsEnumProjection_Clone_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEnumProjection_INTERFACE_DEFINED__ */


#ifndef __ICsUnitInfo_INTERFACE_DEFINED__
#define __ICsUnitInfo_INTERFACE_DEFINED__

/* interface ICsUnitInfo */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsUnitInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29BBFA91-105A-11D2-9DE4-080009ACE18E")
    ICsUnitInfo : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnumLinearUnits( 
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnumAngularUnits( 
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetLinearUnitScale( 
            /* [in] */ CsUnit unit,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAngularUnitScale( 
            /* [in] */ CsUnit unit,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUnitType( 
            /* [in] */ CsUnit unit,
            /* [out] */ CsUnitType __RPC_FAR *pUnitType) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTagString( 
            /* [in] */ CsUnit unit,
            /* [out] */ BSTR __RPC_FAR *ppTag) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetAbbreviation( 
            /* [in] */ CsUnit unit,
            /* [out] */ BSTR __RPC_FAR *ppAbbrev,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsUnitInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsUnitInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsUnitInfo __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumLinearUnits )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumAngularUnits )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetLinearUnitScale )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAngularUnitScale )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ double __RPC_FAR *pdScale,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnitType )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ CsUnitType __RPC_FAR *pUnitType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTagString )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ BSTR __RPC_FAR *ppTag);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetAbbreviation )( 
            ICsUnitInfo __RPC_FAR * This,
            /* [in] */ CsUnit unit,
            /* [out] */ BSTR __RPC_FAR *ppAbbrev,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsUnitInfoVtbl;

    interface ICsUnitInfo
    {
        CONST_VTBL struct ICsUnitInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsUnitInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsUnitInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsUnitInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsUnitInfo_EnumLinearUnits(This,ppEnum)	\
    (This)->lpVtbl -> EnumLinearUnits(This,ppEnum)

#define ICsUnitInfo_EnumAngularUnits(This,ppEnum)	\
    (This)->lpVtbl -> EnumAngularUnits(This,ppEnum)

#define ICsUnitInfo_GetLinearUnitScale(This,unit,pdScale,pErr)	\
    (This)->lpVtbl -> GetLinearUnitScale(This,unit,pdScale,pErr)

#define ICsUnitInfo_GetAngularUnitScale(This,unit,pdScale,pErr)	\
    (This)->lpVtbl -> GetAngularUnitScale(This,unit,pdScale,pErr)

#define ICsUnitInfo_GetUnitType(This,unit,pUnitType)	\
    (This)->lpVtbl -> GetUnitType(This,unit,pUnitType)

#define ICsUnitInfo_GetTagString(This,unit,ppTag)	\
    (This)->lpVtbl -> GetTagString(This,unit,ppTag)

#define ICsUnitInfo_GetAbbreviation(This,unit,ppAbbrev,pErr)	\
    (This)->lpVtbl -> GetAbbreviation(This,unit,ppAbbrev,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_EnumLinearUnits_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsUnitInfo_EnumLinearUnits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_EnumAngularUnits_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [out] */ ICsEnumUnit __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsUnitInfo_EnumAngularUnits_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_GetLinearUnitScale_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ double __RPC_FAR *pdScale,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsUnitInfo_GetLinearUnitScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_GetAngularUnitScale_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ double __RPC_FAR *pdScale,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsUnitInfo_GetAngularUnitScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_GetUnitType_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ CsUnitType __RPC_FAR *pUnitType);


void __RPC_STUB ICsUnitInfo_GetUnitType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_GetTagString_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ BSTR __RPC_FAR *ppTag);


void __RPC_STUB ICsUnitInfo_GetTagString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsUnitInfo_GetAbbreviation_Proxy( 
    ICsUnitInfo __RPC_FAR * This,
    /* [in] */ CsUnit unit,
    /* [out] */ BSTR __RPC_FAR *ppAbbrev,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsUnitInfo_GetAbbreviation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsUnitInfo_INTERFACE_DEFINED__ */


#ifndef __ICsProjectionInfo_INTERFACE_DEFINED__
#define __ICsProjectionInfo_INTERFACE_DEFINED__

/* interface ICsProjectionInfo */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsProjectionInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29BBFA93-105A-11D2-9DE4-080009ACE18E")
    ICsProjectionInfo : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EnumProjections( 
            /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesParam( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesOriginLongitude( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesOriginLatitude( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesScaleReduction( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesQuadrant( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UsesOffset( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParamBounds( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdMin,
            /* [out] */ double __RPC_FAR *pdMax,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParamDefault( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdDefault,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParamLogicalType( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjLogicalType __RPC_FAR *pLogicalType,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParamFormatType( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjFormatType __RPC_FAR *pFormatType,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetParamType( 
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjParamType __RPC_FAR *pParamType,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetUnitType( 
            /* [in] */ CsProjection prjType,
            /* [out] */ CsUnitType __RPC_FAR *pUnitType) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetTagString( 
            /* [in] */ CsProjection prjType,
            /* [out] */ BSTR __RPC_FAR *ppTag) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsProjectionInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsProjectionInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsProjectionInfo __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EnumProjections )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesParam )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesOriginLongitude )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesOriginLatitude )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesScaleReduction )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesQuadrant )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *UsesOffset )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BOOL __RPC_FAR *pbUses);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamBounds )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdMin,
            /* [out] */ double __RPC_FAR *pdMax,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamDefault )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ double __RPC_FAR *pdDefault,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamLogicalType )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjLogicalType __RPC_FAR *pLogicalType,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamFormatType )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjFormatType __RPC_FAR *pFormatType,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetParamType )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [in] */ DWORD dwIndex,
            /* [out] */ CsPrjParamType __RPC_FAR *pParamType,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetUnitType )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ CsUnitType __RPC_FAR *pUnitType);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTagString )( 
            ICsProjectionInfo __RPC_FAR * This,
            /* [in] */ CsProjection prjType,
            /* [out] */ BSTR __RPC_FAR *ppTag);
        
        END_INTERFACE
    } ICsProjectionInfoVtbl;

    interface ICsProjectionInfo
    {
        CONST_VTBL struct ICsProjectionInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsProjectionInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsProjectionInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsProjectionInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsProjectionInfo_EnumProjections(This,ppEnum)	\
    (This)->lpVtbl -> EnumProjections(This,ppEnum)

#define ICsProjectionInfo_UsesParam(This,prjType,dwIndex,pbUses)	\
    (This)->lpVtbl -> UsesParam(This,prjType,dwIndex,pbUses)

#define ICsProjectionInfo_UsesOriginLongitude(This,prjType,pbUses)	\
    (This)->lpVtbl -> UsesOriginLongitude(This,prjType,pbUses)

#define ICsProjectionInfo_UsesOriginLatitude(This,prjType,pbUses)	\
    (This)->lpVtbl -> UsesOriginLatitude(This,prjType,pbUses)

#define ICsProjectionInfo_UsesScaleReduction(This,prjType,pbUses)	\
    (This)->lpVtbl -> UsesScaleReduction(This,prjType,pbUses)

#define ICsProjectionInfo_UsesQuadrant(This,prjType,pbUses)	\
    (This)->lpVtbl -> UsesQuadrant(This,prjType,pbUses)

#define ICsProjectionInfo_UsesOffset(This,prjType,pbUses)	\
    (This)->lpVtbl -> UsesOffset(This,prjType,pbUses)

#define ICsProjectionInfo_GetParamBounds(This,prjType,dwIndex,pdMin,pdMax,pErr)	\
    (This)->lpVtbl -> GetParamBounds(This,prjType,dwIndex,pdMin,pdMax,pErr)

#define ICsProjectionInfo_GetParamDefault(This,prjType,dwIndex,pdDefault,pErr)	\
    (This)->lpVtbl -> GetParamDefault(This,prjType,dwIndex,pdDefault,pErr)

#define ICsProjectionInfo_GetParamLogicalType(This,prjType,dwIndex,pLogicalType,pErr)	\
    (This)->lpVtbl -> GetParamLogicalType(This,prjType,dwIndex,pLogicalType,pErr)

#define ICsProjectionInfo_GetParamFormatType(This,prjType,dwIndex,pFormatType,pErr)	\
    (This)->lpVtbl -> GetParamFormatType(This,prjType,dwIndex,pFormatType,pErr)

#define ICsProjectionInfo_GetParamType(This,prjType,dwIndex,pParamType,pErr)	\
    (This)->lpVtbl -> GetParamType(This,prjType,dwIndex,pParamType,pErr)

#define ICsProjectionInfo_GetUnitType(This,prjType,pUnitType)	\
    (This)->lpVtbl -> GetUnitType(This,prjType,pUnitType)

#define ICsProjectionInfo_GetTagString(This,prjType,ppTag)	\
    (This)->lpVtbl -> GetTagString(This,prjType,ppTag)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_EnumProjections_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [out] */ ICsEnumProjection __RPC_FAR *__RPC_FAR *ppEnum);


void __RPC_STUB ICsProjectionInfo_EnumProjections_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesParam_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesParam_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesOriginLongitude_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesOriginLongitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesOriginLatitude_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesOriginLatitude_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesScaleReduction_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesScaleReduction_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesQuadrant_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesQuadrant_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_UsesOffset_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BOOL __RPC_FAR *pbUses);


void __RPC_STUB ICsProjectionInfo_UsesOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetParamBounds_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ double __RPC_FAR *pdMin,
    /* [out] */ double __RPC_FAR *pdMax,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsProjectionInfo_GetParamBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetParamDefault_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ double __RPC_FAR *pdDefault,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsProjectionInfo_GetParamDefault_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetParamLogicalType_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ CsPrjLogicalType __RPC_FAR *pLogicalType,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsProjectionInfo_GetParamLogicalType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetParamFormatType_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ CsPrjFormatType __RPC_FAR *pFormatType,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsProjectionInfo_GetParamFormatType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetParamType_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [in] */ DWORD dwIndex,
    /* [out] */ CsPrjParamType __RPC_FAR *pParamType,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsProjectionInfo_GetParamType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetUnitType_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ CsUnitType __RPC_FAR *pUnitType);


void __RPC_STUB ICsProjectionInfo_GetUnitType_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsProjectionInfo_GetTagString_Proxy( 
    ICsProjectionInfo __RPC_FAR * This,
    /* [in] */ CsProjection prjType,
    /* [out] */ BSTR __RPC_FAR *ppTag);


void __RPC_STUB ICsProjectionInfo_GetTagString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsProjectionInfo_INTERFACE_DEFINED__ */


#ifndef __ICsDatumInfo_INTERFACE_DEFINED__
#define __ICsDatumInfo_INTERFACE_DEFINED__

/* interface ICsDatumInfo */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDatumInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("B53DB712-8E03-11d2-9E01-080009ACE18E")
    ICsDatumInfo : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMaxOffset( 
            /* [out] */ double __RPC_FAR *pdOffset) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalOffset( 
            /* [in] */ double dOffset,
            /* [out] */ BOOL __RPC_FAR *pbLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMaxRotation( 
            /* [out] */ double __RPC_FAR *pdRotation) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalRotation( 
            /* [in] */ double dRotation,
            /* [out] */ BOOL __RPC_FAR *pbLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetBwScaleBounds( 
            /* [out] */ double __RPC_FAR *pdMinBwScale,
            /* [out] */ double __RPC_FAR *pdMaxBwScale) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalBwScale( 
            /* [in] */ double dBwScale,
            /* [out] */ BOOL __RPC_FAR *pbLegal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDatumInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDatumInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDatumInfo __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxOffset )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdOffset);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalOffset )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [in] */ double dOffset,
            /* [out] */ BOOL __RPC_FAR *pbLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMaxRotation )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdRotation);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalRotation )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [in] */ double dRotation,
            /* [out] */ BOOL __RPC_FAR *pbLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetBwScaleBounds )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdMinBwScale,
            /* [out] */ double __RPC_FAR *pdMaxBwScale);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalBwScale )( 
            ICsDatumInfo __RPC_FAR * This,
            /* [in] */ double dBwScale,
            /* [out] */ BOOL __RPC_FAR *pbLegal);
        
        END_INTERFACE
    } ICsDatumInfoVtbl;

    interface ICsDatumInfo
    {
        CONST_VTBL struct ICsDatumInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDatumInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDatumInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDatumInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDatumInfo_GetMaxOffset(This,pdOffset)	\
    (This)->lpVtbl -> GetMaxOffset(This,pdOffset)

#define ICsDatumInfo_IsLegalOffset(This,dOffset,pbLegal)	\
    (This)->lpVtbl -> IsLegalOffset(This,dOffset,pbLegal)

#define ICsDatumInfo_GetMaxRotation(This,pdRotation)	\
    (This)->lpVtbl -> GetMaxRotation(This,pdRotation)

#define ICsDatumInfo_IsLegalRotation(This,dRotation,pbLegal)	\
    (This)->lpVtbl -> IsLegalRotation(This,dRotation,pbLegal)

#define ICsDatumInfo_GetBwScaleBounds(This,pdMinBwScale,pdMaxBwScale)	\
    (This)->lpVtbl -> GetBwScaleBounds(This,pdMinBwScale,pdMaxBwScale)

#define ICsDatumInfo_IsLegalBwScale(This,dBwScale,pbLegal)	\
    (This)->lpVtbl -> IsLegalBwScale(This,dBwScale,pbLegal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_GetMaxOffset_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdOffset);


void __RPC_STUB ICsDatumInfo_GetMaxOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_IsLegalOffset_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [in] */ double dOffset,
    /* [out] */ BOOL __RPC_FAR *pbLegal);


void __RPC_STUB ICsDatumInfo_IsLegalOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_GetMaxRotation_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdRotation);


void __RPC_STUB ICsDatumInfo_GetMaxRotation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_IsLegalRotation_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [in] */ double dRotation,
    /* [out] */ BOOL __RPC_FAR *pbLegal);


void __RPC_STUB ICsDatumInfo_IsLegalRotation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_GetBwScaleBounds_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdMinBwScale,
    /* [out] */ double __RPC_FAR *pdMaxBwScale);


void __RPC_STUB ICsDatumInfo_GetBwScaleBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDatumInfo_IsLegalBwScale_Proxy( 
    ICsDatumInfo __RPC_FAR * This,
    /* [in] */ double dBwScale,
    /* [out] */ BOOL __RPC_FAR *pbLegal);


void __RPC_STUB ICsDatumInfo_IsLegalBwScale_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDatumInfo_INTERFACE_DEFINED__ */


#ifndef __ICsEllipsoidInfo_INTERFACE_DEFINED__
#define __ICsEllipsoidInfo_INTERFACE_DEFINED__

/* interface ICsEllipsoidInfo */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsEllipsoidInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("195B0032-8E07-11d2-9E01-080009ACE18E")
    ICsEllipsoidInfo : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetRadiusBounds( 
            /* [out] */ double __RPC_FAR *pdMinRadius,
            /* [out] */ double __RPC_FAR *pdMaxRadius) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalRadius( 
            /* [in] */ double dRadius,
            /* [out] */ BOOL __RPC_FAR *pbLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetFlatteningBounds( 
            /* [out] */ double __RPC_FAR *pdMinFlat,
            /* [out] */ double __RPC_FAR *pdMaxFlat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE IsLegalFlatteningRatio( 
            /* [in] */ double dFlat,
            /* [out] */ BOOL __RPC_FAR *pbLegal) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FlatToEccent( 
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEccent) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EccentToFlat( 
            /* [in] */ double dEccent,
            /* [out] */ double __RPC_FAR *pdFlat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE FlatFromRadii( 
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dPolarRadius,
            /* [out] */ double __RPC_FAR *pdFlat) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE EquatorialFromPolarFlat( 
            /* [in] */ double dPolarRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEquatorialRadius) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE PolarFromEquatorialFlat( 
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdPolarRadius) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsEllipsoidInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsEllipsoidInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsEllipsoidInfo __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetRadiusBounds )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdMinRadius,
            /* [out] */ double __RPC_FAR *pdMaxRadius);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalRadius )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dRadius,
            /* [out] */ BOOL __RPC_FAR *pbLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFlatteningBounds )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [out] */ double __RPC_FAR *pdMinFlat,
            /* [out] */ double __RPC_FAR *pdMaxFlat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *IsLegalFlatteningRatio )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dFlat,
            /* [out] */ BOOL __RPC_FAR *pbLegal);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FlatToEccent )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEccent);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EccentToFlat )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dEccent,
            /* [out] */ double __RPC_FAR *pdFlat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *FlatFromRadii )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dPolarRadius,
            /* [out] */ double __RPC_FAR *pdFlat);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *EquatorialFromPolarFlat )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dPolarRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdEquatorialRadius);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *PolarFromEquatorialFlat )( 
            ICsEllipsoidInfo __RPC_FAR * This,
            /* [in] */ double dEquatorialRadius,
            /* [in] */ double dFlat,
            /* [out] */ double __RPC_FAR *pdPolarRadius);
        
        END_INTERFACE
    } ICsEllipsoidInfoVtbl;

    interface ICsEllipsoidInfo
    {
        CONST_VTBL struct ICsEllipsoidInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsEllipsoidInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsEllipsoidInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsEllipsoidInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsEllipsoidInfo_GetRadiusBounds(This,pdMinRadius,pdMaxRadius)	\
    (This)->lpVtbl -> GetRadiusBounds(This,pdMinRadius,pdMaxRadius)

#define ICsEllipsoidInfo_IsLegalRadius(This,dRadius,pbLegal)	\
    (This)->lpVtbl -> IsLegalRadius(This,dRadius,pbLegal)

#define ICsEllipsoidInfo_GetFlatteningBounds(This,pdMinFlat,pdMaxFlat)	\
    (This)->lpVtbl -> GetFlatteningBounds(This,pdMinFlat,pdMaxFlat)

#define ICsEllipsoidInfo_IsLegalFlatteningRatio(This,dFlat,pbLegal)	\
    (This)->lpVtbl -> IsLegalFlatteningRatio(This,dFlat,pbLegal)

#define ICsEllipsoidInfo_FlatToEccent(This,dFlat,pdEccent)	\
    (This)->lpVtbl -> FlatToEccent(This,dFlat,pdEccent)

#define ICsEllipsoidInfo_EccentToFlat(This,dEccent,pdFlat)	\
    (This)->lpVtbl -> EccentToFlat(This,dEccent,pdFlat)

#define ICsEllipsoidInfo_FlatFromRadii(This,dEquatorialRadius,dPolarRadius,pdFlat)	\
    (This)->lpVtbl -> FlatFromRadii(This,dEquatorialRadius,dPolarRadius,pdFlat)

#define ICsEllipsoidInfo_EquatorialFromPolarFlat(This,dPolarRadius,dFlat,pdEquatorialRadius)	\
    (This)->lpVtbl -> EquatorialFromPolarFlat(This,dPolarRadius,dFlat,pdEquatorialRadius)

#define ICsEllipsoidInfo_PolarFromEquatorialFlat(This,dEquatorialRadius,dFlat,pdPolarRadius)	\
    (This)->lpVtbl -> PolarFromEquatorialFlat(This,dEquatorialRadius,dFlat,pdPolarRadius)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_GetRadiusBounds_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdMinRadius,
    /* [out] */ double __RPC_FAR *pdMaxRadius);


void __RPC_STUB ICsEllipsoidInfo_GetRadiusBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_IsLegalRadius_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dRadius,
    /* [out] */ BOOL __RPC_FAR *pbLegal);


void __RPC_STUB ICsEllipsoidInfo_IsLegalRadius_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_GetFlatteningBounds_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [out] */ double __RPC_FAR *pdMinFlat,
    /* [out] */ double __RPC_FAR *pdMaxFlat);


void __RPC_STUB ICsEllipsoidInfo_GetFlatteningBounds_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_IsLegalFlatteningRatio_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dFlat,
    /* [out] */ BOOL __RPC_FAR *pbLegal);


void __RPC_STUB ICsEllipsoidInfo_IsLegalFlatteningRatio_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_FlatToEccent_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdEccent);


void __RPC_STUB ICsEllipsoidInfo_FlatToEccent_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_EccentToFlat_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dEccent,
    /* [out] */ double __RPC_FAR *pdFlat);


void __RPC_STUB ICsEllipsoidInfo_EccentToFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_FlatFromRadii_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dEquatorialRadius,
    /* [in] */ double dPolarRadius,
    /* [out] */ double __RPC_FAR *pdFlat);


void __RPC_STUB ICsEllipsoidInfo_FlatFromRadii_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_EquatorialFromPolarFlat_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dPolarRadius,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdEquatorialRadius);


void __RPC_STUB ICsEllipsoidInfo_EquatorialFromPolarFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsEllipsoidInfo_PolarFromEquatorialFlat_Proxy( 
    ICsEllipsoidInfo __RPC_FAR * This,
    /* [in] */ double dEquatorialRadius,
    /* [in] */ double dFlat,
    /* [out] */ double __RPC_FAR *pdPolarRadius);


void __RPC_STUB ICsEllipsoidInfo_PolarFromEquatorialFlat_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsEllipsoidInfo_INTERFACE_DEFINED__ */


#ifndef __ICsDictionaryUtility_INTERFACE_DEFINED__
#define __ICsDictionaryUtility_INTERFACE_DEFINED__

/* interface ICsDictionaryUtility */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsDictionaryUtility;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("29BBFA95-105A-11D2-9DE4-080009ACE18E")
    ICsDictionaryUtility : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSysDefaultDictionaryInfo( 
            /* [out] */ BSTR __RPC_FAR *ppDir,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
            /* [out] */ BSTR __RPC_FAR *ppDatumDict,
            /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
            /* [out] */ BSTR __RPC_FAR *ppCategoryDict) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE SetSysDefaultDictionaryInfo( 
            /* [in] */ const BSTR kpDir,
            /* [in] */ const BSTR kpEllipsoidDict,
            /* [in] */ const BSTR kpDatumDict,
            /* [in] */ const BSTR kpCoordsysDict,
            /* [in] */ const BSTR kpCategoryDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateEllipsoidDictionary( 
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateDatumDictionary( 
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateCoordsysDictionary( 
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE CreateCategoryDictionary( 
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsDictionaryUtilityVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsDictionaryUtility __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsDictionaryUtility __RPC_FAR * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSysDefaultDictionaryInfo )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *ppDir,
            /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
            /* [out] */ BSTR __RPC_FAR *ppDatumDict,
            /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
            /* [out] */ BSTR __RPC_FAR *ppCategoryDict);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetSysDefaultDictionaryInfo )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ const BSTR kpDir,
            /* [in] */ const BSTR kpEllipsoidDict,
            /* [in] */ const BSTR kpDatumDict,
            /* [in] */ const BSTR kpCoordsysDict,
            /* [in] */ const BSTR kpCategoryDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateEllipsoidDictionary )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateDatumDictionary )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateCoordsysDictionary )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateCategoryDictionary )( 
            ICsDictionaryUtility __RPC_FAR * This,
            /* [in] */ const BSTR kpDict,
            /* [out] */ CsErr __RPC_FAR *pErr);
        
        END_INTERFACE
    } ICsDictionaryUtilityVtbl;

    interface ICsDictionaryUtility
    {
        CONST_VTBL struct ICsDictionaryUtilityVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsDictionaryUtility_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsDictionaryUtility_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsDictionaryUtility_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsDictionaryUtility_GetSysDefaultDictionaryInfo(This,ppDir,ppEllipsoidDict,ppDatumDict,ppCoordsysDict,ppCategoryDict)	\
    (This)->lpVtbl -> GetSysDefaultDictionaryInfo(This,ppDir,ppEllipsoidDict,ppDatumDict,ppCoordsysDict,ppCategoryDict)

#define ICsDictionaryUtility_SetSysDefaultDictionaryInfo(This,kpDir,kpEllipsoidDict,kpDatumDict,kpCoordsysDict,kpCategoryDict,pErr)	\
    (This)->lpVtbl -> SetSysDefaultDictionaryInfo(This,kpDir,kpEllipsoidDict,kpDatumDict,kpCoordsysDict,kpCategoryDict,pErr)

#define ICsDictionaryUtility_CreateEllipsoidDictionary(This,kpDict,pErr)	\
    (This)->lpVtbl -> CreateEllipsoidDictionary(This,kpDict,pErr)

#define ICsDictionaryUtility_CreateDatumDictionary(This,kpDict,pErr)	\
    (This)->lpVtbl -> CreateDatumDictionary(This,kpDict,pErr)

#define ICsDictionaryUtility_CreateCoordsysDictionary(This,kpDict,pErr)	\
    (This)->lpVtbl -> CreateCoordsysDictionary(This,kpDict,pErr)

#define ICsDictionaryUtility_CreateCategoryDictionary(This,kpDict,pErr)	\
    (This)->lpVtbl -> CreateCategoryDictionary(This,kpDict,pErr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_GetSysDefaultDictionaryInfo_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *ppDir,
    /* [out] */ BSTR __RPC_FAR *ppEllipsoidDict,
    /* [out] */ BSTR __RPC_FAR *ppDatumDict,
    /* [out] */ BSTR __RPC_FAR *ppCoordsysDict,
    /* [out] */ BSTR __RPC_FAR *ppCategoryDict);


void __RPC_STUB ICsDictionaryUtility_GetSysDefaultDictionaryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_SetSysDefaultDictionaryInfo_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [in] */ const BSTR kpDir,
    /* [in] */ const BSTR kpEllipsoidDict,
    /* [in] */ const BSTR kpDatumDict,
    /* [in] */ const BSTR kpCoordsysDict,
    /* [in] */ const BSTR kpCategoryDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionaryUtility_SetSysDefaultDictionaryInfo_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_CreateEllipsoidDictionary_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [in] */ const BSTR kpDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionaryUtility_CreateEllipsoidDictionary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_CreateDatumDictionary_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [in] */ const BSTR kpDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionaryUtility_CreateDatumDictionary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_CreateCoordsysDictionary_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [in] */ const BSTR kpDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionaryUtility_CreateCoordsysDictionary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsDictionaryUtility_CreateCategoryDictionary_Proxy( 
    ICsDictionaryUtility __RPC_FAR * This,
    /* [in] */ const BSTR kpDict,
    /* [out] */ CsErr __RPC_FAR *pErr);


void __RPC_STUB ICsDictionaryUtility_CreateCategoryDictionary_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsDictionaryUtility_INTERFACE_DEFINED__ */


#ifndef __ICsPersistStream_INTERFACE_DEFINED__
#define __ICsPersistStream_INTERFACE_DEFINED__

/* interface ICsPersistStream */
/* [object][unique][helpstring][uuid] */ 


EXTERN_C const IID IID_ICsPersistStream;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("620080E4-651A-11d2-9DF3-080009ACE18E")
    ICsPersistStream : public IPersist
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Load( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE Save( 
            /* [unique][in] */ IStream __RPC_FAR *pStm) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetSizeMax( 
            /* [out] */ DWORD __RPC_FAR *pdwSize) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ICsPersistStreamVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ICsPersistStream __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ICsPersistStream __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ICsPersistStream __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetClassID )( 
            ICsPersistStream __RPC_FAR * This,
            /* [out] */ CLSID __RPC_FAR *pClassID);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Load )( 
            ICsPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Save )( 
            ICsPersistStream __RPC_FAR * This,
            /* [unique][in] */ IStream __RPC_FAR *pStm);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSizeMax )( 
            ICsPersistStream __RPC_FAR * This,
            /* [out] */ DWORD __RPC_FAR *pdwSize);
        
        END_INTERFACE
    } ICsPersistStreamVtbl;

    interface ICsPersistStream
    {
        CONST_VTBL struct ICsPersistStreamVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ICsPersistStream_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ICsPersistStream_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ICsPersistStream_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ICsPersistStream_GetClassID(This,pClassID)	\
    (This)->lpVtbl -> GetClassID(This,pClassID)


#define ICsPersistStream_Load(This,pStm)	\
    (This)->lpVtbl -> Load(This,pStm)

#define ICsPersistStream_Save(This,pStm)	\
    (This)->lpVtbl -> Save(This,pStm)

#define ICsPersistStream_GetSizeMax(This,pdwSize)	\
    (This)->lpVtbl -> GetSizeMax(This,pdwSize)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsPersistStream_Load_Proxy( 
    ICsPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB ICsPersistStream_Load_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsPersistStream_Save_Proxy( 
    ICsPersistStream __RPC_FAR * This,
    /* [unique][in] */ IStream __RPC_FAR *pStm);


void __RPC_STUB ICsPersistStream_Save_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring] */ HRESULT STDMETHODCALLTYPE ICsPersistStream_GetSizeMax_Proxy( 
    ICsPersistStream __RPC_FAR * This,
    /* [out] */ DWORD __RPC_FAR *pdwSize);


void __RPC_STUB ICsPersistStream_GetSizeMax_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ICsPersistStream_INTERFACE_DEFINED__ */



#ifndef __CSAPILib_LIBRARY_DEFINED__
#define __CSAPILib_LIBRARY_DEFINED__

/* library CSAPILib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_CSAPILib;

EXTERN_C const CLSID CLSID_CsSession;

#ifdef __cplusplus

class DECLSPEC_UUID("3C00EA41-0B84-11D2-9DE2-080009ACE18E")
CsSession;
#endif

EXTERN_C const CLSID CLSID_CsEllipsoidDictionary;

#ifdef __cplusplus

class DECLSPEC_UUID("03C6E1C8-3D34-11D2-9022-0060B01AA6ED")
CsEllipsoidDictionary;
#endif

EXTERN_C const CLSID CLSID_CsDatumDictionary;

#ifdef __cplusplus

class DECLSPEC_UUID("0404DEEC-3D34-11D2-9022-0060B01AA6ED")
CsDatumDictionary;
#endif

EXTERN_C const CLSID CLSID_CsCoordsysDictionary;

#ifdef __cplusplus

class DECLSPEC_UUID("01B0D308-3D34-11D2-9022-0060B01AA6ED")
CsCoordsysDictionary;
#endif

EXTERN_C const CLSID CLSID_CsCategoryDictionary;

#ifdef __cplusplus

class DECLSPEC_UUID("0295B4C8-3D34-11D2-9022-0060B01AA6ED")
CsCategoryDictionary;
#endif

EXTERN_C const CLSID CLSID_CsCoordConvert;

#ifdef __cplusplus

class DECLSPEC_UUID("3C00EA43-0B84-11D2-9DE2-080009ACE18E")
CsCoordConvert;
#endif

EXTERN_C const CLSID CLSID_CsDatumShift;

#ifdef __cplusplus

class DECLSPEC_UUID("3C00EA45-0B84-11D2-9DE2-080009ACE18E")
CsDatumShift;
#endif

EXTERN_C const CLSID CLSID_CsCoordsys;

#ifdef __cplusplus

class DECLSPEC_UUID("3C00EA47-0B84-11D2-9DE2-080009ACE18E")
CsCoordsys;
#endif

EXTERN_C const CLSID CLSID_CsCoordsysDef;

#ifdef __cplusplus

class DECLSPEC_UUID("3C00EA4B-0B84-11D2-9DE2-080009ACE18E")
CsCoordsysDef;
#endif

EXTERN_C const CLSID CLSID_CsDatum;

#ifdef __cplusplus

class DECLSPEC_UUID("B4E01DB1-0D13-11D2-9DE2-080009ACE18E")
CsDatum;
#endif

EXTERN_C const CLSID CLSID_CsEllipsoidDef;

#ifdef __cplusplus

class DECLSPEC_UUID("B4E01DB4-0D13-11D2-9DE2-080009ACE18E")
CsEllipsoidDef;
#endif

EXTERN_C const CLSID CLSID_CsDatumDef;

#ifdef __cplusplus

class DECLSPEC_UUID("B4E01DB6-0D13-11D2-9DE2-080009ACE18E")
CsDatumDef;
#endif

EXTERN_C const CLSID CLSID_CsCategoryDef;

#ifdef __cplusplus

class DECLSPEC_UUID("B4E01DBA-0D13-11D2-9DE2-080009ACE18E")
CsCategoryDef;
#endif

EXTERN_C const CLSID CLSID_CsEnumCategoryCoordsys;

#ifdef __cplusplus

class DECLSPEC_UUID("B4E01DBC-0D13-11D2-9DE2-080009ACE18E")
CsEnumCategoryCoordsys;
#endif

EXTERN_C const CLSID CLSID_CsEnumDword;

#ifdef __cplusplus

class DECLSPEC_UUID("5E3CD1C2-0D38-11D2-9DE3-080009ACE18E")
CsEnumDword;
#endif

EXTERN_C const CLSID CLSID_CsEllipsoidDefSet;

#ifdef __cplusplus

class DECLSPEC_UUID("5E3CD1C4-0D38-11D2-9DE3-080009ACE18E")
CsEllipsoidDefSet;
#endif

EXTERN_C const CLSID CLSID_CsEnumEllipsoid;

#ifdef __cplusplus

class DECLSPEC_UUID("5E3CD1C6-0D38-11D2-9DE3-080009ACE18E")
CsEnumEllipsoid;
#endif

EXTERN_C const CLSID CLSID_CsDatumDefSet;

#ifdef __cplusplus

class DECLSPEC_UUID("5E3CD1C8-0D38-11D2-9DE3-080009ACE18E")
CsDatumDefSet;
#endif

EXTERN_C const CLSID CLSID_CsEnumDatum;

#ifdef __cplusplus

class DECLSPEC_UUID("F7891124-0F6A-11D2-9DE3-080009ACE18E")
CsEnumDatum;
#endif

EXTERN_C const CLSID CLSID_CsCoordsysDefSet;

#ifdef __cplusplus

class DECLSPEC_UUID("F7891126-0F6A-11D2-9DE3-080009ACE18E")
CsCoordsysDefSet;
#endif

EXTERN_C const CLSID CLSID_CsEnumCoordsys;

#ifdef __cplusplus

class DECLSPEC_UUID("F7891128-0F6A-11D2-9DE3-080009ACE18E")
CsEnumCoordsys;
#endif

EXTERN_C const CLSID CLSID_CsCategoryDefSet;

#ifdef __cplusplus

class DECLSPEC_UUID("CDF16D92-0F8D-11D2-9DE3-080009ACE18E")
CsCategoryDefSet;
#endif

EXTERN_C const CLSID CLSID_CsEnumCategory;

#ifdef __cplusplus

class DECLSPEC_UUID("CDF16D94-0F8D-11D2-9DE3-080009ACE18E")
CsEnumCategory;
#endif

EXTERN_C const CLSID CLSID_CsEnumUnit;

#ifdef __cplusplus

class DECLSPEC_UUID("CDF16D96-0F8D-11D2-9DE3-080009ACE18E")
CsEnumUnit;
#endif

EXTERN_C const CLSID CLSID_CsEnumProjection;

#ifdef __cplusplus

class DECLSPEC_UUID("CDF16D98-0F8D-11D2-9DE3-080009ACE18E")
CsEnumProjection;
#endif

EXTERN_C const CLSID CLSID_CsUnitInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("29BBFA92-105A-11D2-9DE4-080009ACE18E")
CsUnitInfo;
#endif

EXTERN_C const CLSID CLSID_CsProjectionInfo;

#ifdef __cplusplus

class DECLSPEC_UUID("29BBFA94-105A-11D2-9DE4-080009ACE18E")
CsProjectionInfo;
#endif

EXTERN_C const CLSID CLSID_CsDictionaryUtility;

#ifdef __cplusplus

class DECLSPEC_UUID("29BBFA96-105A-11D2-9DE4-080009ACE18E")
CsDictionaryUtility;
#endif

EXTERN_C const CLSID CLSID_CsEnumDictEllipsoid;

#ifdef __cplusplus

class DECLSPEC_UUID("8CAEBA43-3B75-11D2-9DF1-080009ACE18E")
CsEnumDictEllipsoid;
#endif

EXTERN_C const CLSID CLSID_CsEnumDictDatum;

#ifdef __cplusplus

class DECLSPEC_UUID("000AD2CE-3D34-11D2-9022-0060B01AA6ED")
CsEnumDictDatum;
#endif

EXTERN_C const CLSID CLSID_CsEnumDictCoordsys;

#ifdef __cplusplus

class DECLSPEC_UUID("0514AB00-3D34-11D2-9022-0060B01AA6ED")
CsEnumDictCoordsys;
#endif

EXTERN_C const CLSID CLSID_CsEnumDictCategory;

#ifdef __cplusplus

class DECLSPEC_UUID("0305C34E-3D34-11D2-9022-0060B01AA6ED")
CsEnumDictCategory;
#endif
#endif /* __CSAPILib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif