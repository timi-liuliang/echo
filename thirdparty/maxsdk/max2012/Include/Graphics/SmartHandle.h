//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "./GraphicsDriverExport.h"
#include "./Types.h"

#include "../MaxHeap.h"
#include "../Noncopyable.h"

namespace MaxSDK { namespace Graphics {

/** This is the base wrapper class for all graphics primitive classes in max viewport: 
worlds, nodes, render items, materials, etc. It contains internal graphics object 
pointer and holds a reference to that pointer. It will release its reference to 
graphics object in its destructor.

Note this is not for generic smart handle use.
*/
class SmartHandle : public MaxSDK::Util::Noncopyable
{
public:
	/** destructor
	*/
	GraphicsDriverAPI virtual ~SmartHandle();

	/** Check if this handle is valid. User should never call member functions of 
	an invalid handle.
	*/
	GraphicsDriverAPI bool IsValid() const;
	
	/** Release the underlying graphics objects. After calling this function, any 
	function of the same SmartHandle instance should not be used.
	*/
	GraphicsDriverAPI void Release();

	/** Get the underlying graphics object id. This function is internally used only.
	*/
	GraphicsDriverAPI Identifier GetObjectID() const;

	// This function is internally used only.
	GraphicsDriverAPI void SetPointer(void* pObject);

	/** Get the underlying graphics object pointer. This function is internally used only.
	*/
	GraphicsDriverAPI void* GetPointer() const;

	// comparison operators
	inline bool operator == (const SmartHandle& rhs) const {
		return mpObject == rhs.mpObject;
	}

	inline bool operator != (const SmartHandle& rhs) const {
		return mpObject != rhs.mpObject;
	}

	inline bool operator < (const SmartHandle& rhs) const {
		return mpObject < rhs.mpObject;
	}

	inline bool operator <= (const SmartHandle& rhs) const {
		return mpObject <= rhs.mpObject;
	}

	inline bool operator > (const SmartHandle& rhs) const {
		return mpObject > rhs.mpObject;
	}

	inline bool operator >= (const SmartHandle& rhs) const {
		return mpObject >= rhs.mpObject;
	}

protected:
	// protected to keep the class from getting created directly. handle defaults to undefined
	GraphicsDriverAPI SmartHandle();

	// the object that this interface reference to. It is used to avoid accessing driver
	// every time you want to modify the object.
	void* mpObject;

private:
	SmartHandle(const SmartHandle&);
	SmartHandle& operator = (const SmartHandle&);

	void AddRef();
};

} } // end namespace
