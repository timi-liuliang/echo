/**********************************************************************
 *<
	FILE:  marketDefaults.h

	DESCRIPTION:  Market Specific Defaults API

	CREATED BY: Cleve Ard

	HISTORY: created 5/19/2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "coreexp.h"
#include "maxheap.h"
#include "strclass.h"
#include "maxtypes.h"
#include "plugapi.h"
#include "ParamDimension.h"
#include "point3.h"
#include "point4.h"

// forward declarations
class MarketDefaults;
class MarketDefaults13;
class Point3;
class Point4;
class ReferenceTarget;
class Mtl;
class Texmap;

// Get the market default settings. We can handle multiple
// files, so third parties can add there own market defaults.
// Name should be the name, without and extension, for the
// default to be retrieved. NULL will retrieve the defaults
// for Autodesk/Discreet settings.
CoreExport MarketDefaults* GetMarketDefaults(const MCHAR* name = NULL);

//!	Get a filename that is in the current MarketDefaults set.
/*! If the file doesn't exist in the MarketDefaults set and
	does exits in the factoryDefaults for the MarketDefaults
	set it is copied to the MarketDefault set.
	\param[in] name The filename (without) path of the file
	whose read location is requested.
	\see IPathConfigMgr::IsUsingProfileDirectories()
	\see IPathConfigMgr::IsUsingRoamingProfiles()
	\return The full path of the market defaults file requested.
 */
CoreExport MSTR GetMarketDefaultsFileName(const MCHAR* name = NULL);

//! \brief Get the appropriate write location for a market defaults file
/*! Get the appropriate write location for a market defaults file
	This value will depend on whether the application is configured
	to work with User Profile directories.
	\see IPathConfigMgr::IsUsingProfileDirectories()
	\see IPathConfigMgr::IsUsingRoamingProfiles()
	\return The path to which market default files should be written.
 */
CoreExport MSTR GetMarketDefaultsWriteLocation();

//! \brief This class manages the default settings of a range of 3ds Max features. 
/*! \par Description:
	A set of default settings can be saved under a certain name. 
	The different sets are then presented by 3ds Max to users to 
	choose from. 3rd party plug-ins can add their own deafults to 
	the current set of settings, but cannot create their own sets.
*/
class MarketDefaults: public MaxHeapOperators {
public:
	typedef bool (*FloatValidator)(float&);
	typedef bool (*IntValidator)(int&);
	typedef bool (*Point3Validator)(Point3&);
	typedef bool (*Point4Validator)(Point4&);
	typedef bool (*StringValidator)(MSTR&);
	typedef bool (*AnimatableValidator)(Animatable*&);
	typedef bool (*ClassIDValidator)(Class_ID&);

	// This class generates range checkers for values. 
	template<class T, int low, int high> class Range;

	CoreExport virtual ~MarketDefaults();

	// This function rejects NULL animatable objects
	CoreExport static bool CheckNULL(Animatable*& obj);

	// This function rejects Null strings
	CoreExport static bool CheckNULL(MSTR& str);

	// This function rejects Class_ID(0, 0) and
	// Class_ID(STANDIN_CLASS_ID, 0)
	CoreExport static bool CheckNULL(Class_ID& id);

	// High level access to defaults. These return the default
	// value, based on the class ID of the object and the name
	// of the setting. A default value is supplied, to be used
	// when the setting name is not present for the class ID.
	// The validator and dimension are only used for setting
	// names that are found. The dimension is used to convert
	// the UI values in the settings file to internal values
	// before the validator is called. The validator may change
	// the value. It returns true if the retrieved value is to
	// be used, or false if the default value is to be used.
	virtual int GetInt(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		int					defaultValue,
		IntValidator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	int GetTime(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		int					defaultValue,
		IntValidator		validator = NULL
	)
	{
		return GetInt(objectSuperClass, objectClass, name, defaultValue, validator, DIM_TIME);
	}

	virtual float GetFloat(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	float GetAngle(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_ANGLE);
	}

	float GetPercentage(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_PERCENT);
	}

	float GetWorld(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_WORLD);
	}

	float GetColorChannel(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		float				defaultValue,
		FloatValidator		validator = NULL
	)
	{
		return GetFloat(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR255);
	}

	virtual Point3 GetPoint3(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Point3&		defaultValue,
		Point3Validator		validator = NULL,
		int					dimension = DIM_NONE
	) = 0;

	Point3 GetRGBA(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Point3&		defaultValue,
		Point3Validator		validator = NULL
	)
	{
		return GetPoint3(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR255);
	}

	virtual Point4 GetPoint4(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Point4&		defaultValue,
		Point4Validator		validator = NULL,
		int					dimension = DIM_NONE
		) = 0;

	Point4 GetFRGBA(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Point4&		defaultValue,
		Point4Validator		validator = NULL
		)
	{
		return GetPoint4(objectSuperClass, objectClass, name, defaultValue, validator, DIM_COLOR);
	}

	virtual MSTR GetString(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const MCHAR*		defaultValue,
		StringValidator		validator = NULL
	) = 0;

	virtual Class_ID GetClassID(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Class_ID&		defaultID,
		ClassIDValidator	validator = NULL
	) = 0;

	virtual Animatable* CreateInstance(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		SClass_ID			superClass,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	) = 0;
	
	CoreExport ReferenceTarget* CreateRefTarget(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		SClass_ID			superClass,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	);

	CoreExport Mtl* CreateMtl(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	);

	CoreExport Texmap* CreateTexmap(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Class_ID&		defaultInstance,
		AnimatableValidator	validator = NULL
	);

	// This is the low level interface into the defaults.
	// This interface lets you retrive raw values directly from the
	// defaults.
	virtual bool SectionExists(const MCHAR* section) = 0;
	virtual bool SectionExists(SClass_ID sid, const Class_ID& id) = 0;
	virtual void SetSectionName(const MCHAR* section) = 0;
	virtual void SetSectionName(SClass_ID sid, const Class_ID& id) = 0;
	virtual const MCHAR* GetSectionName() = 0;
	virtual void DeleteSection(const MCHAR* section) = 0;
	virtual void DeleteSection(SClass_ID sid, const Class_ID& id) = 0;

	virtual bool KeyExists(const MCHAR* key) = 0;
	virtual bool DeleteKey(const MCHAR* key) = 0;

	virtual bool PutInt(
		const MCHAR*	key,
		int				val
	) = 0;
	virtual bool PutIntArray(
		const MCHAR*	key,
		int				ct,
		const int*		array
	) = 0;
	virtual bool PutFloat(
		const MCHAR*	key,
		float val
	) = 0;
	virtual bool PutFloatArray(
		const MCHAR*	key,
		int				ct,
		const float*	array
	) = 0; 
	virtual bool PutString(
		const MCHAR*	key,
		const MCHAR*	str
	) = 0;

	virtual bool GetInt(
		const MCHAR*	key,
		int&			val
	) = 0;
	virtual bool GetIntArray(
		const MCHAR*	key,
		int&			ct,
		int*			array,
		int				arrayCt
	) = 0;
	virtual bool GetFloat(
		const MCHAR*	key,
		float&			val
	) = 0;
	virtual bool GetFloatArray(
		const MCHAR*	key,
		int&			ct,
		float*			array,
		int				arrayCt
	) = 0;
	virtual bool GetString(
		const MCHAR*	key,
		MCHAR*			buf,
		int				bufSize
	) = 0;
};

// Get the market default settings for R13.
CoreExport MarketDefaults13* GetMarketDefaults13(const MCHAR* name = NULL);

// This class is the extension for R13 with new type.
class MarketDefaults13 : public MarketDefaults
{
public:
	typedef bool (*Point2Validator)(Point2&);

	virtual Point2 GetPoint2(
		SClass_ID			objectSuperClass,
		const Class_ID&		objectClass,
		const MCHAR*		name,
		const Point2&		defaultValue,
		Point2Validator		validator = NULL,
		int					dimension = DIM_NONE
		) = 0;
};



// This class generates range checkers for values. 
template<class T, int low, int high> class MarketDefaults::Range : public MaxHeapOperators
{
public:
	// Return whether val is in the range including the endpoints
	static bool CheckII(T& val);

	// Return whether val is in the range excluding the low endpoint
	static bool CheckEI(T& val);

	// Return whether val is in the range excluding the high endpoint
	static bool CheckIE(T& val);

	// Return whether val is in the range excluding the endpoints
	static bool CheckEE(T& val);

	// Correct val to fall within the givin range including the endpoints
	static bool Correct(T& val);

	// Correct val if it is above the given range. Fail if it is below.
	static bool CorrectHigh(T& val);

	// Correct val if it is below the given range. Fail if it is above.
	static bool CorrectLow(T& val);
};

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CheckII(T& val)
{
	return val >= T(low) && val <= T(high);
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CheckEI(T& val)
{
	return val > T(low) && val <= T(high);
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CheckIE(T& val)
{
	return val >= T(low) && val < T(high);
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CheckEE(T& val)
{
	return val > T(low) && val < T(high);
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::Correct(T& val)
{
	if (val < T(low))
		val = T(low);
	else if (val > T(high))
		val = T(high);
	return true;
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CorrectHigh(T& val)
{
	if (val < T(low))
		return false;
	if (val > T(high))
		val = T(high);
	return true;
}

template<class T, int low, int high> bool MarketDefaults::Range<T, low, high>::CorrectLow(T& val)
{
	if (val > T(high))
		return false;
	if (val < T(low))
		val = T(low);
	return true;
}

