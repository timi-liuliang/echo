/**********************************************************************
 *<
	FILE: gport2.h

	DESCRIPTION: Enhancement of gport

	CREATED BY: Chloe Mignot

	HISTORY:

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "gport.h"
#include "buildver.h"

//! \brief This class adds extends GPort interface.
/*! Two methods are added :
 \li setColorCorrectedState : when set to TRUE, the color picker will be affected by color correction,
 when set to FALSE, the color picker will not be affected by color correction.
 \li getColorCorrectedState : returns TRUE if the color picker is set to be affected by color correction,returns FALSE otherwise.
*/
class GPort2 : public GPort
{
public:

	//!GPort2 constructor.
	GPort2(){mIsColorCorrected = FALSE;}

	//! This method returns the current color correction state of the color picker.
	//! \return - The method returns TRUE if the color picker is set to be affected by color correction , and returns FALSE otherwise.
	BOOL getColorCorrectedState(){
#ifndef NO_LUT_INTEGRATION
		return mIsColorCorrected;
#else //NO_LUT_INTEGRATION
		return TRUE;
#endif
	}

	//! This method sets the current color correction state of the color picker to a new value.
	//! This does NOT affect the current color correction mode.
	//! \param[in] isCorrected - When set to TRUE, the color picker will be affected by color correction , when set to FALSE , the color picker will not be affected by color correction.
	void setColorCorrectedState(BOOL isCorrected){
#ifndef NO_LUT_INTEGRATION
		mIsColorCorrected = isCorrected;
#else //NO_LUT_INTEGRATION
		mIsColorCorrected = TRUE;
#endif
	}

private:
	//! Flag for color correction managing of the color picker
	BOOL mIsColorCorrected;
};

extern CoreExport GPort2* GetGPort2();

