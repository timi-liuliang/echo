//*****************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/
/*==============================================================================

  file:     imrLightTranslation.h

  author:   Daniel Levesque

  created:  27 Jan 2006

  description:

    Defition of the mental ray light translation interface.

  modified:	

==============================================================================*/
#pragma once

#include "..\maxheap.h"
#include "..\baseinterface.h"
#include "..\coreexp.h"
#include "..\maxtypes.h"

// Forward declarations
class imrTranslation;
class INode;
class Interval;
class Texmap;

//==============================================================================
// class imrLightTranslation
//
//! \brief An interface used to translate custom lights to mental ray.
/*!
	This interface makes it possible for 3rd party light plugins to specify their
	own shaders and properties for use with mental ray. This interface is basically
	used to fill up the miLight structure (see the mental ray documentation for
	details on the miLight structure). Any miLight parameter which is not supported
	by this interface is queried on class LightObject instead. The shadow options,
	for example, are retrieved through the shadow generator plugin assigned to the light.
*/
class imrLightTranslation : public BaseInterface {

public:

	//! Returns the interface ID of this class.
	CoreExport static Interface_ID GetInterfaceID();

	/*! Queries a pointer to this class from an interface server.
		\return A valid pointer to an instance of this class if the given interface server
		derives from it; NULL otherwise. */
	CoreExport static imrLightTranslation* GetInterface(InterfaceServer& iserver);

	//! Identifies the type of light to be used in the mental ray API.
	enum LightType {
		//! Same as miLIGHT_ORIGIN in the mental ray API.
		kLightType_Origin,
		//! Same as miLIGHT_DIRECTION in the mental ray API.
		kLightType_Directional,
		//! Same as miLIGHT_DIRECTION with miLight::miLIGHT_DIRECTION in the mental ray API.
		kLightType_DirectionalWithOrigin,
		//! Same as miLIGHT_SPOT in the mental ray API.
		kLightType_Spot
	};

	//! Identifies the type of area light to be created.
	//! miTYPE_OBJECT is not yet supported.
	enum AreaType {
		//! Same as miLIGHT_NONE in the mental ray API.
		kAreaType_None,
		//! Same as miLIGHT_RECTANGLE in the mental ray API.
		kAreaType_Rectangle,
		//! Same as miLIGHT_DISC in the mental ray API.
		kAreaType_Disc,
		//! Same as miLIGHT_SPHERE in the mental ray API.
		kAreaType_Sphere,
		//! Same as miLIGHT_CYLINDER in the mental ray API.
		kAreaType_Cylinder,
		//! Same as miLIGHT_USER in the mental ray API.
		kAreaType_User
	};

	//! A simple vector structure, equivalent to miVector in the mental ray API.
	struct VectorStruct : public MaxHeapOperators {
		float x, y, z;
	};

	//! Describes an area primitive, equivalent to miLight::primitive in the mental ray API.
	union AreaPrimitive {
		//! Same as miLight_rectangle in the mental ray API.
		struct AreaLight_rectangle : public MaxHeapOperators {
			VectorStruct edge_u;
			VectorStruct edge_v;
		} rectangle;
		//! Same as miLight_disc in the mental ray API.
		struct AreaLight_disc : public MaxHeapOperators {
			VectorStruct normal;
			float radius;
		} disc;
		//! Same as miLight_sphere in the mental ray API.
		struct AreaLight_sphere : public MaxHeapOperators {
			float radius;
		} sphere;
		//! Same as miLight_cylinder in the mental ray API.
		struct AreaLight_cylinder : public MaxHeapOperators {
			VectorStruct axis;
			float radius;
		} cylinder;
	};

	//@{
	//! \brief Return the light and emitter shaders to be used for this light.
	/*!	These methods will usually return one of:
		-# An instance to a mental ray shader Texmap (i.e. a Texmap* to an actual mental ray
		shader, which implements imrShader*). Such an instance can be created by iterating
		through the list of Texmap plugins loaded by 3ds Max, and looking for the internal name
		that matches with the shader definition.
		-# An instance to a Texmap that implements imrShaderTranslation. This is usually
		preferable as it allows the plugin developer to implement custom translation for the
		shader.
		\param[in] node - The light node for which this shader is created, in case the shader
		has dependencies on that node.
		\return The light and photon emitter shaders to be used with this light; the photon
		emitter shader may be NULL if the light does not support photon emission.
	*/
	virtual Texmap* GetLightShader(INode& node) = 0;
	virtual Texmap* GetPhotonEmitterShader(INode& node) = 0;
	//@}

	// Tells the translator whether this light species a custom flux. If this returns false,
	// then the translator will attempt to calculate the flux based on the light type and its intensity.

	//! \brief Used by the translator to determine whether this light calculates its own flux.
	/*!	If this returns false, the translator attempts to calculate the flux based on the light type,
		its intensity and other parameters.
		If this returns true, GetFlux() will be used to retrieve the flux. */
	virtual bool HasCustomFlux(INode& node) = 0;


	//! \brief Returns the flux to be used with this light for GI and caustics.
	/*!	If HasCustomFlux() returns true and GetPhotonEmitterShader() returns non-null, 
		then this method needs to return a valid flux value. Otherwise, this method should
		simply return 0.	
		The flux is used to calculate the energy used with GI and caustic, in lumen. The formula is: 
		energy = PI * (flux * globalEnergyMultiplier * lightColor * globalLightLevel) / meterScale / physicalScale 
		- For point lights, the flux is calculated as: intensity_in_candelas * solid_angle_in_steradians, 
		IF the intensity is constant.
		- For point lights with non-constant intensity (e.g. spotlights with falloff), 
		the flux is an integration of the intensity (in candelas) over the sphere.
		- For directional lights, the flux is: intensity_in_lux * area_covered_by_directional_light, 
		and the directional light must shoot photons for and only for that area.
		\param[in] node - The light node with which this light is associated.
		\param[in] translationInterface - The interface for the mental ray translator.
		\param[in] t - The time value at which the flux is to be calculated.
		\param[inout] valid - The validity interval of the light. The validity interval of the flux
		calculation should be INTERSECTED with the incoming interval.
		\return The flux to be used with this light.
	*/
	virtual float GetFlux(INode& node, imrTranslation& translationInterface, TimeValue t, Interval& valid) = 0;

	/*!	Returns the type of light to be created in mental ray.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual LightType GetLightType(TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns the type of area light to be created in mental ray.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual AreaType GetAreaType(TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns the description of the area primitive to be created in mental ray.
		Will only be called if the area light type is different than "none". Implement an
		empty methods if the light is not an area light.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual void GetAreaPrimitive(AreaPrimitive& areaPrimitive, TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns the number of area samples to be used for this area light.
		The number of samples is equivalent to miLight::samples_u * miLight::samples_v.
		If this light is not an area light, return 0.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual short GetAreaSamples(TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns the number of area samples to be used once the trance depth given in "low_level is reached.
		This value is equivalent to miLight::low_samples_u * miLight::low_samples_v.		
		Return 0 if this is not an area light.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual short GetAreaLowSamples(TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns the trace depth level at which the number of area samples is changed to "low_samples".
		Return 0 to disable this feature and always use "samples".
		Return 0 if this is not an area light.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual short GetAreaLowLevel(TimeValue t, Interval& validity) = 0;

	/*!	\brief Returns whether this area light is visible.
		Return false if this is not an area light.
		\param[in] t - The time value for which the value is queried.
		\param[in] validity - The validity interval of the value is to be intersected with this interval.
	*/
	virtual bool GetAreaVisible(TimeValue t, Interval& validity) = 0;
};

