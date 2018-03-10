//**************************************************************************/
// Copyright 2009 Autodesk, Inc.  All rights reserved.
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//**************************************************************************/

#pragma once
#include "CoreExport.h"
#include "maxtypes.h"

class Interface;
class Interface7;
class Interface8;
class Interface9;
class Interface10;
class Interface11;
class Interface12;
class Interface13;
class FPInterface;

/// \defgroup SystemCoreInterfaceAccess Access to services exposed by 3ds Max's "core"
/// The following methods allow access to classes that host services exposed by 3ds Max for plug-ins to use.
//@{
CoreExport Interface*   GetCOREInterface();
CoreExport Interface7*  GetCOREInterface7();
CoreExport Interface8*  GetCOREInterface8();
CoreExport Interface9*  GetCOREInterface9();
CoreExport Interface10* GetCOREInterface10();
CoreExport Interface11* GetCOREInterface11();
CoreExport Interface12* GetCOREInterface12();
CoreExport Interface13* GetCOREInterface13();
//@}

/// \defgroup CoreInterfaceManagement Core Interface Management
/// A "core" interface is usually a singleton manager object that can be used by client 
/// code without explicitly creating an instance of a plug-in.  
/// "Core" interfaces are usually exposed to Maxscript and are derived from 
/// class FPInterfaceDesc or class FPStaticInterface and use the FP_CORE flag.
//@{
/** 
 Registers a "core" interface with 3ds Max. 
 Plug-ins can register their own "core" interfaces identified by a unique Interface_ID value.
 In order to register a "core" interface with 3ds Max, it's enough to declare an instance of a
 FPInterfaceDesc, i.e. plug-ins do not need to call RegisterCOREInterface(FPInterface* fpi).
 For more details see class FPInterfaceDesc.
 \param fpi - pointer to the "core" interface object or its descriptor
*/
CoreExport void RegisterCOREInterface(FPInterface* fpi);

/** 
 Retrieves a "core" interface based on a unique id
 \param id - Unique interface id
 \return - Pointer to the corresponding "core" interface
*/
CoreExport FPInterface* GetCOREInterface(Interface_ID id);

/**
 Returns the count of "core" interfaces registered with 3ds Max.
*/
CoreExport int NumCOREInterfaces();	

/**
 Retrieves a "core" interface based on an index value
 Note that the order of "core" interfaces is not guaranteed.
 This means that the index of a "core" interface may not be
 given by NumCOREInterfaces()-1 right after the interface has been registered.
 \param i - index of the interface to retrieve. This is a zero based index.
 \return - Pointer to the corresponding "core" interface
*/
CoreExport FPInterface* GetCOREInterfaceAt(int i);

/**
 Unregisters a "core" interface previously registered with 3ds Max.
 Note that "Core" interfaces derived from class FPInterface or a subclass of it
 are automatically unregistered when the C++ object that implements them is destroyed.
 \param fpi - Pointer to the interface to unregister
*/
CoreExport void UnregisterCOREInterface(FPInterface* fpi);
//@}

/**
 Get ID'd interface from ClassDesc for given class/sclass
*/
CoreExport FPInterface* GetInterface(SClass_ID super, Class_ID cls, Interface_ID id);

