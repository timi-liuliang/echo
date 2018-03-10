/*! \file IParticleChannelMatrix3.h
    \brief	Channel-generic interfaces for particle channels that store matrix data.
*/
/**********************************************************************
 *<
	CREATED BY:		Chung-An Lin

	HISTORY:		created 02-03-02

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "PFExport.h"
#include "..\ifnpub.h"

// generic particle channel "Matrix3"
// interface ID
#define PARTICLECHANNELMATRIX3R_INTERFACE Interface_ID(0x74f93c0a, 0x1eb34500)
#define PARTICLECHANNELMATRIX3W_INTERFACE Interface_ID(0x74f93c0a, 0x1eb34501)

// since it's a "type" channel there is no "GetChannel" defines
//#define GetParticleChannelMatrix3RInterface(obj) ((IParticleChannelMatrix3R*)obj->GetInterface(PARTICLECHANNELMATRIX3R_INTERFACE))
//#define GetParticleChannelMatrix3WInterface(obj) ((IParticleChannelMatrix3W*)obj->GetInterface(PARTICLECHANNELMATRIX3W_INTERFACE))


class IParticleChannelMatrix3R : public FPMixinInterface
{
public:

	// function IDs Read
	enum {	kGetValue,
			kIsGlobal,
			kGetValueGlobal
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	FN_1(kGetValue, TYPE_MATRIX3_BR, GetValue, TYPE_INT);
	FN_0(kIsGlobal, TYPE_bool, IsGlobal);
	FN_0(kGetValueGlobal, TYPE_MATRIX3_BR, GetValue);

	END_FUNCTION_MAP

    /** @defgroup IParticleChannelMatrix3 IParticleChannelMatrix3.h
    *  @{
    */

    	/*! \fn virtual const Matrix3&	GetValue(int index) const = 0;
    	*  \brief Get property for particle with index
      */
	virtual const Matrix3&	GetValue(int index) const = 0;

    	/*! \fn virtual bool			IsGlobal() const = 0;
    	*  \brief Verify if the channel is global
      */
	virtual bool			IsGlobal() const = 0;

    	/*! \fn virtual const Matrix3&	GetValue() const = 0;
    	*  \brief If channel is global returns the global value
	If channel is not global returns value of the first particle
 	Returns bounding box for all particles
      */
	virtual const Matrix3&	GetValue() const = 0;

    	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMATRIX3R_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMATRIX3R_INTERFACE); }
};

class IParticleChannelMatrix3W : public FPMixinInterface
{
public:

	// function IDs Write
	enum {	kSetValue,
			kSetValueGlobal,
	};

	// Function Map for Function Publish System
	//***********************************
	BEGIN_FUNCTION_MAP

	VFN_2(kSetValue, SetValue, TYPE_INT, TYPE_MATRIX3_BR);
	VFN_1(kSetValueGlobal, SetValue, TYPE_MATRIX3_BR);

	END_FUNCTION_MAP

     /** @defgroup IParticleChannelMatrix3 IParticleChannelMatrix3.h
    *  @{
    */

   	/*! \fn virtual void	SetValue(int index, const Matrix3& v) = 0;
    	*  \brief Set property for particle with index
      */
	virtual void	SetValue(int index, const Matrix3& v) = 0;

    	/*! \fn virtual void	SetValue(const Matrix3& v) = 0;
    	*  \brief Set property for all particles at once thus making the channel global
      */
	virtual void	SetValue(const Matrix3& v) = 0;

    	/*! \fn FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMATRIX3W_INTERFACE); }
    	*  \brief 
      */
	FPInterfaceDesc* GetDesc() { return GetDescByID(PARTICLECHANNELMATRIX3W_INTERFACE); }
};

