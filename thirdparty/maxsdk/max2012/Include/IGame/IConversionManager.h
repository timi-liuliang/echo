/**********************************************************************
 *<
	FILE: IConveriosnManager.h

	DESCRIPTION:	Tools to convert from one coordinate system to another

	CREATED BY:		Neil Hazzard

	HISTORY:		10|12|2002

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IConversionManager.h
\brief IGame Coordinate conversion Interfaces.
*/

#pragma once

#include "..\maxheap.h"
#include "IGameStd.h"

//!A User definable Coordinate System
/*! The developer can use this to define the Coordinate System that they are using.  Rotation specifies whether
it is a Right or Left handed system.  The Axis define which way the primary axis point.  This will mean that the 
data extracted is converted correctly, and the winding order is correct for Left and Right handed systems.
\n
In 3ds Max this could be defined as
\n
<pre>
UserCoord = {
1,	//Right Handed
1,	//X axis goes right
4,	//Y Axis goes in
2,	//Z Axis goes up.
1,	//U Tex axis is left
0,  //V Tex axis is Up
}
</pre>
	
*/
struct UserCoord{
	//! Handedness
	/*! 0 specifies Left Handed, 1 specifies Right Handed.
	*/
	int rotation;	

	//! The X axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int xAxis;
	//! The Y axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int yAxis;
	//! The Z axis 
	/*! It can be one of the following values 0 = left, 1 = right, 2 = Up, 3 = Down, 4 = in, 5 = out.
	*/
	int zAxis;

	//! The U Texture axis 
	/*! It can be one of the following values 0 = left, 1 = right
	*/
	int uAxis;

	//! The V Texture axis
	/*! It can be one of the following values 0 = Up, 1 = down
	*/
	int vAxis;

};

//! A developer can use this class to define what Coord Systems IGame exports the data in
/*! IGame will convert data from the standard 3ds Max RH Z up system to any defined system.  At the moment direct support
for DirectX and OpenGL are provided.  This means that all Matrix and vertex data will have been converted ready to use
on your target system.
\n The coordinate system should set up straight after initialising IGame.  The default is to provide everything in 3ds Max
native formats.
*/

class IGameConversionManager: public MaxHeapOperators
{
public:
	//! The supported Coordinate Systems
	/*! These are used to tell IGame how to format the data
	*/
	enum CoordSystem{
		IGAME_MAX,	/*!<Max RH Z up & +Y*/
		IGAME_D3D,	/*!<DirectX LH Y up & +Z*/
		IGAME_OGL,	/*!<OpenGL RH Y up & -Z*/
		IGAME_USER  /*!<User defined Coord System*/
	};

	//!Set IGame up for the Coordinate System you are wanting the data to be present in
	/*! The default system is the 3ds MAX system.
	\param Coord  The Coordinate system to use	If Coord is IGAME_USER then you must set
	this data via SetUserCoordSystem
	*/
	virtual void SetCoordSystem(CoordSystem Coord) =0;

	//!Get the Coordinate System
	/*! The default system is the 3ds MAX system. 
	\return The current Coordinate System used,  one of the IGameConversionManager::CoordSystem
	*/
	virtual CoordSystem GetCoordSystem() =0;


	//!Set the User defined Coordinate system, if the CoordSystem has been defined as IGAME_USER
	/*! Allow a user definable Coordinate System.  See comments above.
	\param UC THe data to define the system
	*/
	virtual void SetUserCoordSystem(UserCoord UC) =0;

};


//!External access to the conversion manager
IGAMEEXPORT IGameConversionManager * GetConversionManager();


