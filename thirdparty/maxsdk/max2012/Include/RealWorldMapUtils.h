/**********************************************************************
 *<
	FILE: RealWorldMapUtils.h

	DESCRIPTION:  Utilities for supporting real-world maps size parameters

	CREATED BY: Scott Morrison

	HISTORY: created Feb. 1, 2005

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "coreexp.h"
#include <WTypes.h>
#include "ifnpub.h"
// forward declarations
class Animatable;

//! Class for creating undo record for any class with a "Real-World Map Size" property.
//!
//! This is a template class where the template parameter T is the type
//! which supports the real-world map size property.  This class must have
//! a method called SetUsePhysicalScaleUVs(BOOL state).
template <class T> class RealWorldScaleRecord: public RestoreObj
{
public:		

    //! Create a real-world map size undo record
    //! \param[in] pObj - the object which supports the undo/redo toggle
    //! \param[in] oldState - the state of the real-world map size toggle to restore to.
    RealWorldScaleRecord(T* pObj, BOOL oldState)
    {
        mpObj = pObj;
        mOldState = oldState;
    }

	 /*! \brief Restores the state of the database to when this RestoreObj was 
	  * added to the hold.
	  *
     * \see RestoreObj::Restore
	  */
    void Restore(int isUndo)
    {
        if (isUndo)
            mpObj->SetUsePhysicalScaleUVs(mOldState);
    }  

	 /*! \brief Restores the database to the state prior to the last undo command.
	  *
     * \see RestoreObj::Redo
	  */
    void Redo()
    {
        mpObj->SetUsePhysicalScaleUVs(!mOldState);
    }

private:
    T* mpObj;
    BOOL mOldState;
};

//! The class ID of the real-world scale interface RealWorldMapSizeInterface.
#define RWS_INTERFACE Interface_ID(0x9ff44ef, 0x6c050704)

//! The unique instance of the real worls map size interface
extern CoreExport FPInterfaceDesc gRealWorldMapSizeDesc;

//! \brief The commong mix-in interface for setting realWorldMapSize properties on 
//! objects and modifiers
//!
//! Details follow here.
//! This class is used with multiple inheritence from a class
//! which exports a "real-world map size" toggle.  The class must
//! implement two abstract methods for setting and getting this value.
//! The class must implement the following method:
//!        BaseInterface* GetInterface(Interface_ID id) 
//!		{ 
//!			if (id == RWS_INTERFACE) 
//!				return this; 
//!			else 
//!				return FPMixinInterface::GetInterface(id);
//!		} 
//!  The client class must add this interface the first time ClassDesc::Create()
//! is called on the class's class descriptor.
//! See maxsdk/samples/objects/boxobj.cpp for an example of the use of this class.

class RealWorldMapSizeInterface: public FPMixinInterface
{
public:
    //! Gets the state of the real-world map size toggle
    //! \return the current state of the toggle
    virtual BOOL GetUsePhysicalScaleUVs() = 0;
    //! Set the real-world map size toggle
    //! \param[in] flag - the new value of the toggle
    virtual void SetUsePhysicalScaleUVs(BOOL flag) = 0;
    
    //From FPMixinInterface
	 /*! Returns a pointer to the interface metadata for this class.
	  *
     * \see FPMixinInterface::GetDesc
	  */
    FPInterfaceDesc* GetDesc() { return &gRealWorldMapSizeDesc; }
    
    
    //! Function published properties for real-world map size
    enum{ rws_getRealWorldMapSize, rws_setRealWorldMapSize};
    
    // Function Map for Function Publishing System 
    //***********************************
    BEGIN_FUNCTION_MAP
    PROP_FNS	(rws_getRealWorldMapSize, GetUsePhysicalScaleUVs, rws_setRealWorldMapSize, SetUsePhysicalScaleUVs, TYPE_BOOL	); 
    END_FUNCTION_MAP
    
    
};

//@{
//! These methods are used by many objects and modifiers to handle
//! the app data required to tag an object as using phyically scaled UVs.
//! \param[in] pAnim - The object to query for the real-world app data.
//! \return the current value of the flag stored in the app date.
CoreExport BOOL GetUsePhysicalScaleUVs(Animatable* pAnim);
//! \param[in] pAnim - The object to set for the real-world app data on.
//! \param[in] flag - the new value of the toggle to set in the app data.
CoreExport void SetUsePhysicalScaleUVs(Animatable* pAnim, BOOL flag);
//@}

//@{
//! Set/Get the property which says we should use real-world map size by default.
//! This value is stored in the market defaults INI file.
//! \return the current value of the flag from the market defaults file.
CoreExport BOOL GetPhysicalScaleUVsDisabled();
//! Set the value of this flag.
//! \param[in] disable - the new value of the flag to store in the market defaults file.
CoreExport void SetPhysicalScaleUVsDisabled(BOOL disable);
//@}
