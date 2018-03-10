/******************************************************************************
 *<
	FILE: bipexp.h
				  
	DESCRIPTION:  Export Interface Functionality for BIped

	CREATED BY: Susan Amkraut with a lot of help from John Chadwick

	HISTORY: created June 2, 1998

 *>     Copyright (c) Unreal Pictures, Inc. 1997, 1998 All Rights Reserved.
 *******************************************************************************/
#pragma once

#include <WTypes.h>
#include "..\maxheap.h"

#ifdef BLD_BIPED
#define BIPExport __declspec( dllexport )
#else
#define BIPExport __declspec( dllimport )
#endif



// This is the interface ID for a Biped Controller Interface
#define I_BIPINTERFACE	0x00100101

// These are subanim numbers for the center of mass (root) controller
#define VERTICAL_SUBANIM	0
#define HORIZONTAL_SUBANIM	1
#define ROTATION_SUBANIM	2


// These are the Class ID defines for Biped Controllers.

// this is the class for all biped controllers except the root and the footsteps
#define BIPSLAVE_CONTROL_CLASS_ID Class_ID(0x9154,0)

// this is the class for the center of mass, biped root controller ("Bip01")
#define BIPBODY_CONTROL_CLASS_ID  Class_ID(0x9156,0) 

// this is the class for the biped footstep controller ("Bip01 Footsteps")
#define FOOTPRINT_CLASS_ID Class_ID(0x3011,0)        

// this is the class for biped geometric objects
#define SKELOBJ_CLASS_ID Class_ID(0x9125, 0)

// this is the class for the biped master controller
#define BIPED_CLASS_ID Class_ID(0x9155, 0)


// Using the Biped Export Interface
// 
//  1.	Find the Biped Controller you wish to export from (see the sample code).
//
//	2.	Given  a Biped controller c, get the Biped Export Interface:
//		IBipedExport *BipIface = (BipedExport *)c->GetInterface(I_BIPINTERFACE);		
//
// At the moment this interface only allows you to remove the non uniform scale.
// This allows you to export cleaner transform information from a biped.
// In the future, more virtual functions will be added, and you'll be able
// to get more information about the biped via this interface.

// Sample Code
//
// // Get a biped node - in this case the root
// INode *node = ip->GetINodeByName("Bip01");
//
// if (node)
// {
//     // Get the node's transform control
//     Control *c = node->GetTMController();
//
//     // You can test whether or not this is a biped controller with the following pseudo code:
//     if ((c->ClassID() == BIPSLAVE_CONTROL_CLASS_ID) ||
//         (c->ClassID() == BIPBODY_CONTROL_CLASS_ID) 
//         (c->ClassID() == FOOTPRINT_CLASS_ID))
//     {
//
//         // Get the Biped Export Interface from the controller 
//         IBipedExport *BipIface = (IBipedExport *) c->GetInterface(I_BIPINTERFACE);
//
//         // Remove the non uniform scale
//         BipIface->RemoveNonUniformScale(1);

//         // Begin or End figure mode
//         BipIface->BeginFigureMode(1);
//         BipIface->EndFigureMode(1);
//     }
// }


// BipedExport: This class can be returned by calling the method GetInterface() from a Biped controller
// Given controller *c points to a Biped controller, then:
// IBipedExport *BipIface = (IBipedExport *) (c->GetInterface(I_BIPINTERFACE));
// will return the interface for this Biped Controller, else returns NULL.
#pragma warning(push)
#pragma warning(disable:4100)
class IBipedExport: public MaxHeapOperators
{
	public:
		
		BIPExport virtual ~IBipedExport() {}

        // For the BipedExport you got from the center of mass (root) controller, send in: 
        // VERTICAL_SUBANIM, HORIZONTAL_SUBANIM, and ROTATION_SUBANIM 
        // to get information for those tracks.
        // For other BipedExports this method is irrelevant.
		// The SetSubAnim method is actually included for future releases, 
		// when more methods will be added to the IBipedExport class.
		// Then you could call these methods for the three center of mass subanims.
		// It is not presently useful.
        BIPExport virtual void SetSubAnim (int i) {};

		// call this from any IBipedExport instance to remove or restore non uniform scaling
		BIPExport virtual void RemoveNonUniformScale(BOOL onOFF) {};

        // call these to begin and end figure mode
		BIPExport virtual void BeginFigureMode(int redraw) {};
		BIPExport virtual void EndFigureMode(int redraw) {};
};

#pragma warning(pop)

