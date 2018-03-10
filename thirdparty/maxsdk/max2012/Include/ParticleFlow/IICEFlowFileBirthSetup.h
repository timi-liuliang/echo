//*****************************************************************************/
// Copyright 2010 Autodesk, Inc.  All rights reserved. 
// 
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may not
// be disclosed to, copied or used by any third party without the prior written
// consent of Autodesk, Inc.
//*****************************************************************************/
#pragma once

#include "..\ifnpub.h"
#include "..\GetCOREInterface.h"

// --- Interface IDs
#define ICEFLOWPARTICLE_FILEBIRTHSETUP_INTERFACE Interface_ID(0x5455499b, 0x3be722d6)

/*! \brief Factory for the Particle Flow creation from a OneClick plugin.
This interface is implemented by 3ds Max. 
*/
class IICEFlowFileBirthSetup : public FPStaticInterface
{
public:
	/*! \brief Sets a new file path to the nCache file to be read by the File Birth object.
	* \param [in] _node The File Birth node that will receive the new file path (new nCache).
	* \param [in] _path The path of the nCache file that will be connected to this construct.
	* \return true if we could set the property, false otherwise.
	*/
	virtual bool    SetPathProperty( INode * _node, const MCHAR * _path ) = 0;

	/*! \brief Retrieves the single instance of the Daylight system factory
	*/
	static IICEFlowFileBirthSetup* GetInstance()	{
		return static_cast<IICEFlowFileBirthSetup*>(GetCOREInterface(ICEFLOWPARTICLE_FILEBIRTHSETUP_INTERFACE));
	}
};

/*@}*/