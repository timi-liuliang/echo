 /**********************************************************************
 
	FILE:           IShadeContextExtension8.h

	DESCRIPTION:    API extension for ShadeContext class.
					Used specifically for querying motion information
					of shade context.

	CREATED BY:     Dwight Tomalty, Discreet

	HISTORY:        Created 23 September 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "baseinterface.h"
#include "point2.h"
#include "imtl.h"

// forward declarations
class IShadeContextExtension8;

#define ISHADECONTEXT_EXTENSION_8_INTERFACE_ID Interface_ID(0x6ea75ff6, 0x5c875b0a)

//! \brief This class is an API extension for the ShadeContext class.
/*! It is used specifically for querying motion information of shade context.*/
class IShadeContextExtension8 : public BaseInterface {

public:

    //! \brief This method returns the velocity information for shade context.
	/*! \return - This method returns a Point2 containing the velocity information for shade context.
	*/
    virtual Point2 MotionVector() = 0;
    
    // -- from BaseInterface
	//! \brief Reimplemented from BaseInterface
	virtual Interface_ID GetID();
};

inline IShadeContextExtension8* Get_IShadeContextExtension8(ShadeContext& sc) {

	return static_cast<IShadeContextExtension8*>(sc.GetInterface(ISHADECONTEXT_EXTENSION_8_INTERFACE_ID));
}

//==============================================================================
// class ISContext inlined methods
//==============================================================================

inline Interface_ID IShadeContextExtension8::GetID() {

	return ISHADECONTEXT_EXTENSION_8_INTERFACE_ID;
}

