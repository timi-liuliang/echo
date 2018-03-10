//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
#pragma once

/*! \defgroup InterfaceIDList List of interface IDs supported by different plugin types
List of interface IDs to be used with void* Animatable::GetInterface(ULONG id)
Checking whether an animatable implements a certain interface is the 
recommended way for determining if that animatable can be used as the type
represented by that interface.\n
For example, given an Animatable* anim, one can find out whether it can be used as
a controller by simply asking for the I_CONTROL interface. In order to simplify the
task of querying for the most common interfaces, use one of the predefined macros, such as 
\code
#define GetControlInterface(anim) ((Control*)anim->GetInterface(I_CONTROL)).
\endcode
Example of using the predefined macros:
\code
Animatable* anim = NULL;
// Code that initializes anim ... 
Control *c = GetControlInterface(anim);
\endcode
\note These interfaces do NOT need to be released with Animatable::ReleaseInterface().
\note The ids of interfaces supported by 3rd party classes derived from Animatable, 
should be greater than I_USERINTERFACE	
*/
//@{
#define I_CONTROL			0x00001010 //!< An Animatable supporting this interface is a Control
#define I_IKCONTROL			0x00001020 //!< An Animatable supporting this interface is an IIKControl
#define I_IKCHAINCONTROL	0x00001030 //!< An Animatable supporting this interface is an IIKChainControl
#define I_WIRECONTROL		0x00001040 //!< An Animatable supporting this interface is an IBaseWireControl
#define I_SCRIPTCONTROL		0x00001050 //!< An Animatable supporting this interface is an IBaseScriptControl
#define I_MASTER			0x00001060 //!< An Animatable supporting this interface is a ReferenceTarget
#define I_EASELIST			0x00001070 //!< An Animatable supporting this interface is an EaseCurveList
#define I_MULTLIST			0x00001080 //!< An Animatable supporting this interface is a MultCurveList
#define I_BASEOBJECT		0x00001090 //!< An Animatable supporting this interface is a BaseObject 
#define I_WSMOBJECT			0x000010a0 //!< An Animatable supporting this interface is a WSMObject
#define I_DERIVEDOBJECT		0x000010b0 //!< An Animatable supporting this interface is an IDerivedObject
#define I_OBJECT			0x000010c0 //!< An Animatable supporting this interface is an Object 
#define I_PARTICLEOBJ		0x000010d0 //!< An Animatable supporting this interface is a ParticleObject
#define I_LIGHTOBJ			0x000010e0 //!< An Animatable supporting this interface is a LightObject
#define I_SIMPLEPARTICLEOBJ	0x000010f0 //!< An Animatable supporting this interface is a SimpleParticle
#define I_KEYCONTROL		0x00001100 //!< An Animatable supporting this interface is an IKeyControl
#define I_KEYCONTROL2		0x00001110 //!< An Animatable supporting this interface is an IKeyControl2
#define I_SETKEYCONTROL		0x00001120 //!< An Animatable supporting this interface is an ISetKey
#define I_SYSTEM_XREF		0x00001130 //!< An Animatable supporting this interface is an ISystemXRef

#define I_TEXTOBJECT		0x00001140 //!< An Animatable supporting this interface is an ITextObject
#define I_WAVESOUND			0x00001150 //!< An Animatable supporting this interface is an IWaveSound
#ifdef _SUBMTLASSIGNMENT
#define I_SUBMTLAPI			0x00001160 //!< An Animatable supporting this interface is an ISubMtlAPI
#endif

#define	I_NEWPARTPOD		0x00001170 //!< An Animatable supporting this interface is a ParticlePodObj
#define	I_NEWPARTSOURCE		0x00001180 //!< An Animatable supporting this interface is a BasicSourceObj
#define	I_NEWPARTOPERATOR	0x00001190 //!< An Animatable supporting this interface is an IOperatorInterface
#define	I_NEWPARTTEST		0x000011a0 //!< An Animatable supporting this interface is an ITestInterface. This is an obsolete interface.

#define I_MESHSELECT		0x000011b0 //!< An Animatable supporting this interface is an IMeshSelect
#define I_MESHSELECTDATA	0x000011c0 //!< An Animatable supporting this interface is an IMeshSelectData
#define I_MAXSCRIPTPLUGIN	0x000011d0 //!< An Animatable supporting this interface is a MSPlugin
#define I_MESHDELTAUSER		0x000011e0 //!< An Animatable supporting this interface is a MeshDeltaUser
#define I_MESHDELTAUSERDATA	0x000011f0 //!< An Animatable supporting this interface is a MeshDeltaUserData
#define I_SPLINESELECT		0x00001200 //!< An Animatable supporting this interface is an ISplineSelect
#define I_SPLINESELECTDATA	0x00001210 //!< An Animatable supporting this interface is an ISplineSelectData
#define I_SPLINEOPS			0x00001220 //!< An Animatable supporting this interface is an ISplineOps
#define I_PATCHSELECT		0x00001230 //!< An Animatable supporting this interface is an IPatchSelect
#define I_PATCHSELECTDATA	0x00001240 //!< An Animatable supporting this interface is an IPatchSelectData
#define I_PATCHOPS			0x00001250 //!< An Animatable supporting this interface is an IPatchOps
#define I_SUBMAP			0x00001260 //!< An Animatable supporting this interface is an ISubMap
#define I_MITRANSLATOR		0x00001270 //!< unused - interface to max connection to mental ray
#define I_MENTALRAY			0x00001280 //!< An Animatable supporting this interface is a mrItem*

// GetInterface IDs for new material management/particle-chunk methods - ECP
#define	I_NEWMTLINTERFACE	0x00001290 //!< An Animatable supporting this interface is an IChkMtlAPI

#define I_COMPONENT			0x000012a0 //!< An Animatable supporting this interface is an IComponent
#define I_REFARRAY			0x000012b0 //!< An Animatable supporting this interface is an IRefArray
#define I_LINKTMCTRL        0x000012c0 //!< An Animatable supporting this interface is an Link Transform Control
#define I_SUBTARGETCTRL     0x000012d0 //!< An Animatable supporting this interface is an Sub Target Control

// REQUIRES RELEASE (?)
#define I_REAGENT			0x000012e0 //!< An Animatable supporting this interface is an IReagent 
#define I_GEOMIMP			0x000012f0 //!< An Animatable supporting this interface is an IGeomImp - implementaion neutral interface to geometry caches - REQUIRES RELEASE
#define I_AGGREGATION		0x00001300 //!< An Animatable supporting this interface is an IAggregation
#define I_VALENCE			0x00001310 //!< An Animatable supporting this interface is an IValence
#define I_GUEST				0x00001320 //!< An Animatable supporting this interface is an IGuest
#define I_HOST				0x00001330 //!< An Animatable supporting this interface is an IHost
#define I_SCENEOBJECT		0x00001340 //!< An Animatable supporting this interface is an Object
#define I_MULTITANGENT		0x00001350 //!< An Animatable supporting this interface is an IAdjustMultipleTangents
#define I_SOFTSELECT		0x00001360 //!< An Animatable supporting this interface is an ISoftSelect
#define I_UNREPLACEABLECTL	0x00001370 //!< An Animatable supporting this interface is an IUnReplaceableControl - used for the exposetransform controllers
#define I_EULERCTRL			0x00001380 //!< An Animatable supporting this interface is an IEulerControl - used for euler controllers in order to get/set the xyz ordering..
#define I_LOCKED			0x00001390 //!< An Animatable supporting this interface is an ILockedTrack
#define I_LOCKED_CLIENT		0x000013a0 //!< An Animatable supporting this interface is an ILockedTrackClient

#define I_OBJECTSUPERCLASS_NOT_OBJECTCLASS	0x000013b0 //!< An Animatable supporting this interface reports a superclassid corresponding to an Object (for example, GEOMOBJECT_CLASS_ID), but creating an instance doesn't create an Object-derived object. Instead it derives just from ReferenceTarget. Examples of this are the NURBS classes RmModel and EmExternalObject.

#define I_COMPOSITESUBMTLAPI 0x000013c0 //!< An Animatable supporting this interface is an ICompositeSubMtlAPI
#define I_MTLIDSET			0x000013d0 //!< An Animatable supporting this interface is an IMtlIdSet

#define I_USERINTERFACE		0x0000ffff //!< Plug-in defined interfaces should be greater than this value

#define GetControlInterface(anim)	((Control*)(anim)->GetInterface(I_CONTROL))
#define GetObjectInterface(anim)	((BaseObject*)(anim)->GetInterface(I_BASEOBJECT))
#define GetParticleInterface(anim) 	((ParticleObject*)(anim)->GetInterface(I_PARTICLEOBJ))
#define GetKeyControlInterface(anim) ((IKeyControl*)(anim)->GetInterface(I_KEYCONTROL))
#define GetSetKeyControlInterface(anim) ((ISetKey*)(anim)->GetInterface(I_SETKEYCONTROL))
#define GetMasterController(anim) ((ReferenceTarget*)(anim)->GetInterface(I_MASTER))
#define GetTextObjectInterface(anim) ((ITextObject*)(anim)->GetInterface(I_TEXTOBJECT))
#define GetWaveSoundInterface(anim) ((IWaveSound*)(anim)->GetInterface(I_WAVESOUND))
#define GetMeshSelectInterface(anim) ((IMeshSelect*)(anim)->GetInterface(I_MESHSELECT))
#define GetMeshSelectDataInterface(anim) ((IMeshSelectData*)(anim)->GetInterface(I_MESHSELECTDATA))
#define GetMeshDeltaUserInterface(anim) ((MeshDeltaUser*)(anim)->GetInterface(I_MESHDELTAUSER))
#define GetMeshDeltaUserDataInterface(anim) ((MeshDeltaUserData*)(anim)->GetInterface(I_MESHDELTAUSERDATA))
#define GetMultiTangentInterface(anim) ((IAdjustMultipleTangents*)(anim)->GetInterface(I_MULTITANGENT))
#define GetSoftSelectInterface(anim) ((ISoftSelect*)(anim)->GetInterface(I_SOFTSELECT))
#define GetLockedTrackInterface(anim) ((ILockedTrack*)(anim)->GetInterface(I_LOCKED))
#define GetLockedTrackClientInterface(client) ((ILockedTrackClient*)(client)->GetInterface(I_LOCKED_CLIENT))
//@}
