/**********************************************************************
 *<
	FILE:			IColorCorrectionMgr.h

	DESCRIPTION: This interface manages the correction that can be made to the colors in MAX. see Desc below.
											 
	CREATED BY:	Chloe Mignot	

	HISTORY: created on jan 18th, 2005		

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"


//! \brief The interface ID for class IColorCorrectionMgr
#define COLORCORRECTIONMGR_INTERFACE Interface_ID(0x2c65369, 0x10be38b0)


//! \brief This interface manages the correction that can be made to the colors.

/*! The differences between IColorCorrectionMgr and GammaManager is the number of channels corrected and the correction types (Autodesk LUT, gamma).
 IColorCorrectionMgr manages 3 channels, so that all Autodesk LUTs can be loaded.
 GammaManager can still be used to change or access the display gamma value or to apply gamma correction on a color 
 BUT it is now highly recommended to use IColorCorrectionMgr.  Only the use of IColorCorrectionMgr will provide the use of Autodesk LUTs.
 Here is a short description of how this class may be used:
 \li First, it manages the color correction mode which can be one of the following:  kLUT, kGAMMA, kNONE
 \li The color correction mode can be modified/accessed by GetColorCorrectionMode/SetColorCorrectionMode methods (see description).
 \li The Autodesk LUT can be modified/accessed by GetLUT/SetLUT .
 \li The display Gamma value can be modified/accessed by GetGamma/SetGamma.
*/

class IColorCorrectionMgr : public FPStaticInterface
{
public:
		//! \brief Describes the color correction mode.
		/*! Only one correction mode can be active at any moment */
		enum CorrectionMode {
			kLUT, //!< %Color correction using Autodesk LUT
			kGAMMA,		   //!< %Color correction using gamma correction
			kNONE          //!< No color correction is made 
		};

		enum ColorChannel {
			kRED_C,  //!< Red channel
			kGREEN_C,//!< Green channel
			kBLUE_C  //!< Blue channel
		};


		//! \brief This method reads and parses the LUT associated with the file name.
		/*! This method can be called in ANY color correction mode,
		 and this action will NOT change the color correction mode.
		 When the color correction mode will be kLUT, 
		 the last LUT set here will be the one used.
		 \param[in] file - new LUT file name
		 \return - false if there was an error reading or parsing the file
		*/
		CoreExport virtual BOOL SetLUT(const MCHAR *file)=0;

		//! \brief Returns the name of the current LUT file
		/*! This method can be called in ANY color correction mode,
		 and this action will NOT change the color correction mode.
		 This will return the current LUT file name in any color correction mode.
		 \return file - the current LUT file name
		 */
		CoreExport virtual MCHAR* GetLUT()const=0;
 
		//! \brief This method sets the current color correction mode.
		/*! \param[in] mode - the new color correction mode ( kGAMMA, kLUT or kNONE)
		    \see IColorCorrectionMgr::GetColorCorrectionPrefMode()
		*/
		CoreExport virtual void SetColorCorrectionMode(const CorrectionMode mode)=0;

		//! \brief This method returns the current color correction mode 
		/*! \return - the current color correction mode ( kGAMMA, kLUT or kNONE)
		*/
		CoreExport virtual CorrectionMode GetColorCorrectionMode() const =0;

		//! \brief This method sets the preferred mode used when color correction is enabled.
		/*! This sets the UI default but does not change the currently active mode.
		\param[in] mode - the preferred mode used when color correction is enabled (kGAMMA or kLUT)
		*/
		CoreExport virtual void SetColorCorrectionPrefMode(const CorrectionMode mode)=0;

		//! \brief This method returns the preferred mode used when color correction is enabled.
		/*! This returns the UI default, ordinarily the most recent enabled mode (kGAMMA or kLUT) selected by the user.
		\return - the preferred mode used when color correction is enabled (kGAMMA or kLUT)
		*/
		CoreExport virtual CorrectionMode GetColorCorrectionPrefMode() const =0;


		//! \brief Correct 8 bit color value
		/*! If the color correction mode is set to kNONE, then the color itself is returned.
		 \param[in] b - value of the color to correct in 16 bits
		 \param[in] c - indicates the channel corresponding to the value to be corrected (kRED_C, kGREEN_C or kBLUE_C)
		 \return - corrected value in 8 bits
		*/
		virtual UBYTE ColorCorrect16(const UWORD b, ColorChannel c) const= 0;

			
		//! \brief Correct 8 bit color value
		/*! If the color correction mode is set to kNONE, then the color itself is returned.
		 \param[in] b - value of the color to correct in 8 bits
		 \param[in] c - indicates the channel corresponding to the value to be corrected (kRED_C, kGREEN_C or kBLUE_C)
		 \return - corrected value in 8 bits
		*/
		virtual UBYTE ColorCorrect8(const UBYTE b, ColorChannel c) const= 0;
		

		//! \brief Correct RGB color. All 3 channels (kRED_C, kGREEN_C or kBLUE_C) are corrected.
		/*! If the color correction mode is set to kNONE, then the color itself is returned.
		 \param[in] col - color to correct
		 \return - corrected value of the given color
		*/
		virtual COLORREF ColorCorrect8RGB(const DWORD col)  const = 0;

		//! \brief This method sets the display gamma correction value to a new value
		/*! This method can be called in ANY color correction mode,
		 and this action will NOT change the color correction mode.
		 This new gamma value will equally be applied to each of the three channels
		 The gamma value must be a float value between 0.01 and 5, 
		 if the newGamma value isn't the resulting value with still be in this range
		 \param[in] newGamma - The new gamma value 
		*/
		CoreExport virtual void SetGamma(float newGamma)=0;

		//! \brief This method returns the display gamma correction value currently stored 
		/*! This method can be called in ANY color correction mode,
		 and this action will NOT change the color correction mode.
		 The returned gamma value is the one equally applied to each of the three channels
		 \return - current display gamma correction value (a value between 0.01 and 5)
		*/
		CoreExport virtual float GetGamma()const=0;
	
		
};


