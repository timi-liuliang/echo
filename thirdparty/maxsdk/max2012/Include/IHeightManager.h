
 /**********************************************************************
 *<
	FILE: HeightManager.h

	DESCRIPTION: Height Manager

	CREATED BY:	Nikolai Sander

	HISTORY: Created 1 January, 2005,

 *>	Copyright (c) 1998, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "GetCOREInterface.h"

//! This interface gives access to the HeightManager. The HeightManager has a list of heights for 
//! Metric, Imperial and Custom Units. All methods work on the currently selected display unit type
//! If Generic is selected, the System unit will be considered in regards to which height-list to use.
//! All Heights are stored in the 3dsmax.ini/3dsviz.ini file.
class IHeightManager : public FPStaticInterface {
	public:
		//! Brings up a dialog, letting the user customize (add, delete, set current) the heights.
		virtual void CustomizeHeights()=0;
		//! Gets the height for the given index
		//! \param[in] index - Index to get the height for.
		//! \return height for the given index
		virtual float GetHeight(int index)=0;
		//! Gets the height, respecting the current unit setting in string form
		//! \param[in] index - Index to get the height for.
		//! \return height, respecting the current unit setting in string form
		virtual MCHAR *GetHeightString(int index)=0;
		//! Gets the index of the current height
		//! \return index of the current height. If no height is active, return value is -1
		virtual int GetCurrentHeightIndex()=0;
		//! Sets the index of the current height
		//! \param[in] index the current height should be set to. If no height should be active set parameter to -1
		virtual void SetCurrentHeightIndex(int index)=0;
		//! Gets the number of heights
		//! \return number of heights
		virtual int GetNumHeights()=0;
		
	}; 
//! HeightManager interface ID
#define IHEIGHTMAN_INTERFACE   Interface_ID(0x78632529, 0xc179d875)

//! Convenience method to retrieve the HeightManager interface
inline IHeightManager* GetHeightManagerInterface() { return (IHeightManager*)GetCOREInterface(IHEIGHTMAN_INTERFACE); }


