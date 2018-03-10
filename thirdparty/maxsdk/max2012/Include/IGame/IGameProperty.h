/**********************************************************************
 *<
	FILE: IGameProperty.h

	DESCRIPTION: IGameProperty interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "..\maxheap.h"
#include "..\strbasic.h"
#include "..\maxtypes.h"
#include "..\BuildWarnings.h"
#include "..\iparamb2Typedefs.h"

// forward declarations
class IParamBlock;
class IParamBlock2;
class Point3;
class Point4;

/*!\file IGameProperty.h
\brief IParamBlock and IParamBlock2 property access.

All properties found by IGame are stored as an IGameProperty.  This gives developers a unified way of accessing
IParamBlock and IParamBlock2 based properties used in 3ds Max
*/



//! Property types
enum PropType{
	IGAME_UNKNOWN_PROP,	/*!<Unkown property Type*/
	IGAME_FLOAT_PROP,	/*!<Property of Type float*/
	IGAME_POINT3_PROP,	/*!<Property of Type Point3*/
	IGAME_INT_PROP,		/*!<Property of Type int*/
	IGAME_STRING_PROP,  /*!<Property of Type MCHAR*/
	IGAME_POINT4_PROP,	/*!<Property of Type Point4*/
};

class IGameControl;

//!Main property definition
/*! IGameProperty provides a wrapper around the standard 3ds Max ParamBlock system.  It works for both 
IParamBlock and IParamBlock2, all paramblocks parameters are supported directly.  It provides access to IGameControl and also data access 
for floats, ints, strings, etc. It performs the type checking, so the Paramblock system will not assert in case of the wrong data type requested.
The access to User Properties is provided by look up from the IGameProp.xml file.  
*/

class IGameProperty: public MaxHeapOperators
{

public:
	//! \brief Destructor 
	virtual ~IGameProperty() {;}

	//!The name of the Property
	/*! The name as defined in MAXScript or User Property defined in IGameProp.xml file
	\return The parameter name
	*/
	virtual const MCHAR * GetName() =0;

	//! Check if the parameter is a IParamBlock2 based
	/*!
	\return TRUE if IParamBlock2, FALSE if IParamBlock
	*/
	virtual bool IsPBlock2()=0;
	
	//! Check if the parameter animated
	/*!  Use this check to determing whether or not to access the controller
	\return TRUE if animated
	*/
	virtual bool IsPropAnimated()=0;

	//! The controller for the Property
	/*!
	\return A pointer to IGameControl
	*/
	virtual IGameControl * GetIGameControl() =0;

	//! Direct access to the IParamBlock2 
	/*! 
	\return a pointer to IParamBlock2
	*/
	virtual IParamBlock2 * GetMaxParamBlock2() =0;

	//! Direct access to the IParamBlock 
	/*! 
	\return a pointer to IParamBlock
	*/
	virtual IParamBlock * GetMaxParamBlock() =0;

	//! Check if the parameter is IParamBlock or IParamBlock2 based
	/*! Specifies whether this parameter is based on either IParamBlock or IParamBlock2.  This is useful as some IGameProperties
	are based on Non paramblocks.  For example node/user data is accessed as an IGame Property but in 3ds Max has no Paramblock representation
	\return TRUE if it is based on a either IParamBlock or IParamBlock2.
	*/
	virtual bool IsParamBlock()=0;

	//! The data type of the Property
	/*! This is used to find out the data type of the property, so the correct GetPropertyValue method can be used
	\return The data type.  Returned value corresponds to PropType enumeration.
	\sa PropType
	*/
	virtual PropType GetType() =0;

	//! Check if User Parameter has a entry in the IGameProp.xml file
	/*! All parameter blocks parameters are supported directly. Use this check to decide 
	whether a User Property is defined in the IGameProp.xml file
	\return True if User Parameter has a entry in the IGameProp.xml file
	*/
	virtual bool IsParameterSupported() = 0;

	//! Deprecated in 3ds Max 2012. Use GetParamIndex or GetParamID instead.
	MAX_DEPRECATED int GetParamBlockIndex();

	//! The index of the parameter
	/*! Replacement for the old GetParamBlockIndex method. 
	This method gets the actual index of the parameter as used by the Parameter block system. The Parameter block
	system treats ParamID's and their related indexes very differently. Therefore incorrectly mixing Parameter integer index's and
	and ParamID's can cause very subtle and difficult to find bugs. Previous implementations of the old 
	GetParamBlockIndex method returned a ParamID cast to an int which was incorrect (If the data was based off of Parameter Block 2). 
	Fixing it would cause silent behavior changes which may have gone unnoticed. Therefore this method
	was introduced to forcefully clarify the difference between accessing the ParamID versus the index of the Parameter. 
	\return The index in the parameter block */
	virtual int GetParamIndex() = 0;

	//! The ParamID of the parameter
	/*! This replaces the old method GetParamBlockIndex which previously had incorrect behavior. 
	\return The ParamID of the parameter, not the index. */
	virtual ParamID GetParamID() = 0;
	

	//! Access to the actual Parameter Data
	/*!
	\param &f  The float to receive the data
	\param t  The time to retrieve the value, defaulted to the static frame.  This is set by IGameScene::SetStaticFrame
	\param p  The flag indicating if percent fraction value (TYPE_PCNT_FRAC) should be converted (0.1 to 10), default:false
	\return TRUE if succesful
	*/
	virtual bool GetPropertyValue(float& f, TimeValue t = TIME_NegInfinity, bool p = false)=0;

	//! Access to the actual Parameter Data
	/*!
	\param &i  The int to receive the data
	\param t  The time to retrieve the value, defaulted to the static frame.  This is set by IGameScene::SetStaticFrame
	\return TRUE if succesful
	*/	
	virtual bool GetPropertyValue(int &i, TimeValue t=TIME_NegInfinity)=0;
	
	//! Access to the actual Parameter Data
	/*!
	\param &p  The Point3 to receive the data
	\param t  The time to retrieve the value, defaulted to the static frame.  This is set by IGameScene::SetStaticFrame
	\return TRUE if succesful
	*/	
	virtual bool GetPropertyValue(Point3 &p, TimeValue t=TIME_NegInfinity)=0;

	//! Access to the actual Parameter Data <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*!
	\param &p  The Point4 to receive the data
	\param t  The time to retrieve the value, defaulted to the static frame.  This is set by IGameScene::SetStaticFrame
	\return TRUE if succesful
	*/	
	virtual bool GetPropertyValue(Point4 &p, TimeValue t=TIME_NegInfinity)=0;

	//! Access to the actual Parameter Data  
	/*!
	\param v  The MCHAR to receive the data
	\param t  The time to retrieve the value, defaulted to the static frame.  This is set by IGameScene::SetStaticFrame
	\return TRUE if successful
	*/	
	virtual bool GetPropertyValue(const MCHAR*& v, TimeValue t=TIME_NegInfinity)=0;

};

//!Property Enumeration
/*! PropertyEnum allows to define a callback for use with EnumerateProperties.  It will be called for 
every parameter stored in the system
*/
class PropertyEnum: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~PropertyEnum() {;}

	//! The call back function
	/*! This is called for every property in the system, providing a way of stopping the enumeration if needed
	\param *prop The actual property found
	\return TRUE to stop the enumeration
	*/
	virtual bool Proc(IGameProperty* prop) = 0;
};

//! Property container
/*! This class provides an extension mechanism that IGame can use - an Entity is free to use them.
The developer can extend the properties that are "known" to IGame can be extended 
this way the property can be retrieved directly by the developer.  As it is "known" to
the developer the property type is also known and can be accessed directly
*/
#pragma warning(push)
#pragma warning(disable:4100)
class IPropertyContainer: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IPropertyContainer() {;}

	//! User Defined Property Access
	/*!Using the unique ID in the IGameProp.xml file, the user defined property can be queried directly
	\param PropID User Property ID used in the IGameProp.xml file
	\return A pointer to IGameProperty if found, NULL otherwise
	*/
	virtual IGameProperty * QueryProperty(DWORD PropID) {return NULL;}

	//! The number of Properties for the Entity
	/*!
	\return The number of properties found.  The default is 0
	*/
	virtual int GetNumberOfProperties(){return 0;}
	
	//!Direct Property Access
	/*! The property can be accessed directly from the index provided by GetNumberOfProperties().  It will return NULL for 
	"hidden" paramameters names (defined as empty string in paramblocks)
	\param index The index of the property to return
	\return A pointer to the IGameProperty.  The default is NULL
	*/
	virtual IGameProperty * GetProperty(int index) {return NULL;}

	//! Property Access
	/*!Using the name, the property can be queried directly
	\param propName Property name as defined in MAXScript or User Property name
	\return A pointer to IGameProperty if found.  The default is NULL
	*/
	virtual IGameProperty * QueryProperty(const MCHAR * propName) {return NULL;}

	//!Enumerate the Properties
	/*!All properties can be enumerated by using this method.  A user defined callback is used to provide access to 
	the properties.
	\param &Enum The callback object to use.
	*/
	virtual void EnumerateProperties(PropertyEnum & Enum) = 0;
};
#pragma warning(pop)

//!Main property access
/*!base Class used by all exporters participating in the properties system
*/
class IExportEntity: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IExportEntity() {;}

	//!Retrieve the Property Container
	/*!
	\return The PropertyContainer for this entity.
	*/
	virtual IPropertyContainer * GetIPropertyContainer(){return NULL;}

	//! Is the Entity directly supported
	/*!IGame provides direct support for certain 3ds Max objects and materials.  For instance, IGame supports the standard material and 
	bitmap texture directly, i.e provides direct API calls to get the properties.  
	\return TRUE if IGame supports the parameters for a particular ClassID directly through its API
	*/
	virtual bool IsEntitySupported() {return false;}

	//!Retrieves the name of the entity class
	/*!
	\return The name of the entity class 
	*/
	virtual MCHAR * GetClassName()=0;
};


