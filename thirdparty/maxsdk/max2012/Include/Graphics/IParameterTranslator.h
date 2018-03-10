//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "../baseinterface.h"
#include "../strclass.h"

/** Interface ID of class IParameterTranslator. */
#define IPARAMETER_TRANSLATOR_INTERFACE_ID Interface_ID(0x31230111, 0x1c385ce3)

namespace MaxSDK { namespace Graphics {

	/** Facilitates translation of material and texture map parameter values to 
	their shaders representation.
	Materials and texture maps can use programmable shaders to describe their 
	appearance when displayed in the Nitrous viewport and rendered with Quicksilver.
	Programmable shaders allow for a more realistic and complex visual appearance
	than ISimpleMaterial does. Note that 3ds Max 2012 only supports MetaSL shaders.
	See IShaderManager for more details.

	The Nitrous graphics system handles automatic translation of Paramblock2 based 
	material and texture map parameters given that their names and types match those 
	of the shader. IParameterTranslator allows plug-ins to control this translation process.
	Nitrous queries the plug-in for the IParameterTranslator interface by passing
	IPARAMETER_TRANSLATOR_INTERFACE_ID to its overwrite of Animatable::GetInterface(Interface_ID),
	and uses it instead of its own automated translation. Nitrous automatically 
	detects when a plug-in's parameters, reference structure or animatable structure
	changes, and will call the methods on this interface to re-translate the plug-in's 
	parameters to the shader.

	The following code snippet illustrates how a material plug-in may implement IParameterTranslator:
	\code
	#include <.\graphics\IParameterTranslator.h>
	// A material plug-in that has a shader and needs to control the parameter translation process
	class MyMtlPlugin : public MtlBase, public MaxSDK::Graphics::IParameterTranslator {
		// Parameter block definition omitted for brevity
	public:
		virtual bool GetParameterValue(
		const TimeValue t, 
		const MCHAR* shaderParamName, 
		void* value, 
		MaxSDK::Graphics::IParameterTranslator::ShaderParameterType shaderParamType) {
			if (_tcscmp(_M("myFloatShaderParam"), shaderParamName) == 0) {
				DbgAssert(ShaderParameterTypeFloat == shaderParamType);
				float floatParam = // Get float parameter value from parameter block
				*(float*)value = floatParam;
				return true;
			}
			else if (_tcscmp(_M("myIntShaderParam"), shaderParamName) == 0) {
				DbgAssert(ShaderParameterTypeInt == shaderParamType);
				*(int*)value = 1; // the value of this parameter is constant
				return true;
			}
			// etc
			DbgAssert(false && _M("Unexpected shader parameter"));
			return false;
		}
		virtual bool GetShaderInputParameterName(
			MaxSDK::Graphics::IParameterTranslator::SubMtlBaseType shaderType, 
			int subMtlBaseIndex, 
			MSTR& shaderParamName) {
				if (shaderType == SubTexmap && 1 == subMtlBaseIndex) {
					shaderParamName = // name of input shader parameter corresponding to sub texture map slot 1
					return true;
				}
				return false;
			}
		virtual bool OnPreParameterTranslate() { 
			// Nothing to pre-process or initialize 
			return true; 
		};
	}
	\endcode
	*/
	class IParameterTranslator : public BaseInterface
	{
	public:
		/** Shader parameter types.*/
		enum ShaderParameterType
		{
			ShaderParameterTypeUnknown = 0,/*!< unknown parameter type*/
			ShaderParameterTypeFloat,	   /*!< float type*/
			ShaderParameterTypeInt,		   /*!< int type*/
			ShaderParameterTypeBool,	   /*!< BOOL type*/
			ShaderParameterTypeColor,	   /*!< AColor type*/
			ShaderParameterTypePoint2,	   /*!< Point2 type*/
			ShaderParameterTypePoint3,	   /*!< Point3 type*/
			ShaderParameterTypePoint4,	   /*!< Point4 type*/
			ShaderParameterTypeTexture,	   /*!< MSTR type*/
		};

		/** The type of plug-in's "sub" slot. */
		enum SubMtlBaseType
		{
			SubMaterial, /*!< sub material type*/
			SubTexmap,	 /*!< sub texmap type*/
		};

		/** Retrieves the interface id of IParameterTranslator.
		*/
		virtual Interface_ID GetID() { return IPARAMETER_TRANSLATOR_INTERFACE_ID; }

		/** Retrieves the plug-in's parameter value that corresponds to a given shader parameter.
		\param [in] t The time at which to retrieve the plug-in's parameter value
		\param [in] shaderParamName The name of the shader parameter. The name is not case sensitive.
		\param [out] value Pointer to a data structure that will store the plug-in's 
		parameter value. This parameter will need to be cast to one of the supported
		types - see ShaderParameterType
		\param [in] type The type of the shader parameter
		\return Returns true if the plug-in parameter value was retrieved successfully, 
		false otherwise.
		*/
		virtual bool GetParameterValue(
			const TimeValue t, 
			const MCHAR* shaderParamName, 
			void* value, 
			ShaderParameterType type) = 0 ;

		/** Retrieves the shader input parameter name for a given "sub" slot of the plug-in.
		The plug-in need to "compute" the name of the input shader parameter that 
		corresponds to a given "sub" slot identified by its slot index and type.
		Nitrous will connect the shader corresponding to material or texture map at 
		that "sub" slot with the shader input parameter that has the name this method
		generates.
		\param [in] type The type of plug-in's "sub" slot
		\param [in] subMtlBaseIndex The index of the "sub" slot within the plug-ins list of slots
		\param [out] shaderInputParamName The shader input parameter name 
		\return true if the shader name was successfully generated, false otherwise.
		*/
		virtual bool GetShaderInputParameterName(SubMtlBaseType type, int subMtlBaseIndex, MSTR& shaderInputParamName) =0 ;

		/** One time setup before any parameter is translated.
		Called once before the parameter list is parsed. This could be used to 
		make sure all parameters are valid before being translated.
		\return If it returns false, the translation does not take place.
		*/
		virtual bool OnPreParameterTranslate() = 0 ;
	};


} } // end namespace