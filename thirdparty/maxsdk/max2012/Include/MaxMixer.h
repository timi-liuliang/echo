/*****************************************************************************

	FILE: MaxMixer.h

	DESCRIPTION: New Mixer interfaces for the new max mix functionality.
	
	CREATED BY: Michael Zyracki

	Copyright (c) 2004 Autodesk Inc., All Rights Reserved.


*****************************************************************************/
#pragma once

#include "maxheap.h"

// forward declarations
class Control;

#ifdef BLD_MIXER
#define MIXExport __declspec( dllexport )
#else
#define MIXExport __declspec( dllimport )
#endif


//!\brief The ClassID for the main class that holds all of the mixer info
#define MASTERMAXCLIP_CLASS_ID	Class_ID(0x56e36460, 0x57c34da)

//!\name ClassID's for the mixer slave controls.
//@{
//!\brief The various slave control class ids for the mixer slaves that replace the ordinary max controllers once the object goes into the mixer.
#define SLAVE_CONTROL_CLASS_ID  Class_ID(0x32fd7fa2, 0x67a62aae)
#define SLAVEFLOAT_CONTROL_CLASS_ID   Class_ID(0xed63317, 0x1a55054b)
#define SLAVEPOS_CONTROL_CLASS_ID	Class_ID(0x77d776ae, 0x26e16054)
#define SLAVEROTATION_CONTROL_CLASS_ID	Class_ID(0x611b5ad7, 0x7485235)
#define SLAVESCALE_CONTROL_CLASS_ID		Class_ID(0x5f712fd0, 0x376a57a8)
#define SLAVEPOINT3_CONTROL_CLASS_ID	Class_ID(0x68233163, 0x3e7802ee)
#define SLAVEPOINT4_CONTROL_CLASS_ID	Class_ID(0x531e2dda, 0x67926815)
#define SLAVEMATRIX3_CONTROL_CLASS_ID	Class_ID(0x20fa45e0, 0x58ef2ced)

//!\brief INTERNAL USE  Interface for the slave
#define I_MIXSLAVEINTERFACE	 0x4606111e

//!\brief INTERNAL USE
class IMixSlave: public MaxHeapOperators
{
public:
	MIXExport virtual Control* GetProxyControl() const =0;

};
//@}
