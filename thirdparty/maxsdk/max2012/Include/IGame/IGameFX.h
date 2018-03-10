/**********************************************************************
*<
FILE: IGameFX.h

DESCRIPTION: D3D Effect interfaces for IGame

CREATED BY: Neil Hazzard,  Autodesk Ltd

HISTORY: created 06/25/05

IGame Version: 1.122

*>	Copyright (c) 2005, All Rights Reserved.
**********************************************************************/
#pragma once

#include "..\maxheap.h"
#include "..\BuildWarnings.h"
#include "..\strclass.h"

// forward declarations
class IGameNode;
class IGameProperty;
class IGameFXProperty;
class IGameFXTechnique;
class IGameFXFunction;
namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}
class Mtl;

//! Main interface for parsing a generic effect file
/*! This interface breaks down an effect file into a table of parameters, techniques and code.  It provides a non format and compiler specific 
summary of an D3D %Effect file.  This allow the developer to break up an effect for recompiling and regenerating on another platform.
*/
class IGameFX: public MaxHeapOperators
{

public:

	//! Define the source of the effect data.
	enum EffectSource{
		kFile,			/*!<The effect is defined in a file*/
		kBuffer			/*!<The effect is defined in memory*/
	};

	//! The compile time option for the profile used in the shader
	enum IGameFXProfile{
		kVSPS_20,		/*!<Vertex and Pixel shader version 2.0*/
		kVSPS_2X,		/*!<Vertex and Pixel shader version 2.A*/
		kVSPS_30		/*!<Vertex and Pixel shader version 3.0*/
	};
	//! \brief Destructor 
	virtual ~IGameFX() {;}

	//! Initialise the interface with the effect to be decompiled
	virtual bool Initialize(const MaxSDK::AssetManagement::AssetUser& effect, EffectSource input)=0;

	//! Parse the effect file, providing the host material as a source for parameter hook up
	virtual bool ParseData(Mtl * effectMtl)=0;

	virtual const MCHAR* GetError()=0;

	//! The total number of properties in the effect
	virtual int GetNumberOfProperties()=0;

	//! Access to a specific parameter.
	virtual IGameFXProperty * GetIGameFXProperty(int index)=0;

	//! The total number of techniques in the effect
	virtual int GetNumberOfTechniques()=0;

	//! Access to a specifc technique.
	virtual IGameFXTechnique* GetIGameFXTechnique(int index)=0;

	//! the total number of functions on the effect
	virtual int GetNumberOfFunctions() =0 ;

	//! Get the function from the index supplied
	virtual IGameFXFunction * GetIGameFXFunction(int index) = 0;

	//!  Access the filename of the effect used.  This will only provide a valid filename if kFile is used in Initialize
	virtual const MaxSDK::AssetManagement::AssetUser & GetEffectFile() = 0;
};


//! An interface that holds the data for an individual parameter in the D3D effect.  This will include global parameters, samplers and transform data
/*! The IGameFX interface will try to match any parameter with its host data.  For the DxMaterial this would be the paramblock that drives basic parameters.
For the DxStdMtl the parameters are driven by the host StdMtl2, but all lighting data is patched in so a "binding" node can be access.  A binding node is a max
node that drives the data.  For a light this could be position, direction, color, falloff and hotspot.  The semantic will define the usage.  A binding token will 
eventually be used to define by name where the parameter lives in the scenegraph
*/
class IGameFXProperty: public MaxHeapOperators
{
public:

	enum IGameFXPropUsage{
		IGAMEFX_GENERAL,	/*!<General property, such a a float or transform param*/
		IGAMEFX_SAMPLER,	/*!<A texture sampler, will propabaly have a pipe state*/
		IGAMEFX_LIGHT,		/*!<A light property - a binding node is used*/
		IGAMEFX_TEXTURE		/*!<A Texture property - this would be a Bitmap in max.*/
	};
	//! \brief Destructor 
	virtual ~IGameFXProperty() {;}

	//! Get the IGameProperty of the effect property
	/*! The IGameFx interface will try to extract all relevant data between the host material and the effect.  Any paramblock data that is used to drive
	the effect will be stored in an IGameProperty
	*/
	virtual IGameProperty * GetIGameProperty() =0;

	//! This defines what the property is - General means a basic parameter, Sampler means a texture sampler.  you can use this to determine whether pipe state exist
	virtual IGameFXPropUsage GetPropertyUsage()=0;

	//! Get the semantic for the parameter
	virtual const MCHAR * GetFXSemantic()=0;

	//! The number of annotations for the parameter.
	virtual int GetNumberOfFXAnnotations()=0;

	//! A key/Value pair defining the name of value of the annotation.  The High level language type id also provided.
	virtual bool GetFXAnnotation(int index, MSTR * languageType, MSTR * key, MSTR * value)=0;

	//! the default value of the parameter if supplied
	virtual const MCHAR * GetFXDefaultValue()=0;

	//! The number of render states values the parameter has.  This would typicaly be based on a Sampler.
	virtual int GetNumberofFXPipeStates()=0;

	//!The key/value pair of any pipeline states supported in the effect.  At the parameter level this will usually be a Sampler
	virtual bool GetPipeStates(int index, MSTR * key, MSTR *value)=0;

	//!The name of the property fround in the effect file
	virtual const MCHAR * GetPropertyName()=0;

	//! The type of the property e.g. float4, int, bool etc..
	virtual const MCHAR * GetPropertyType() = 0;

	//! Get the Binding Node
	virtual IGameNode * GetBindingNode() = 0;
};

//! Interface that holds per pass information.  Each technique will be made up of one or more passes.
/*! A pass will contain pipe state information and also vertex or pixel shader programs
*/
class IGameFXPass: public MaxHeapOperators{
public:
	//! \brief Destructor 
	virtual ~IGameFXPass() {;}

	//! The name of the pass
	virtual const MCHAR * GetPassName()=0;

	//!The number of annotations associated with the pass
	virtual int GetNumberOfFXAnnotations()=0;

	//! Access the key/value pair for the annotation
	virtual bool GetFXAnnotation(int index, MSTR * languageType, MSTR * key, MSTR * value)=0;

	//! the number of render pipe states
	virtual int GetNumberofFXPipeStates()=0;

	//!Acces the specific key/value pair for the state.  This would typically be render state information
	virtual bool GetPipeStates(int index, MSTR * key, MSTR *value)=0;

	//! Access the vertex shader for the current pass
	/*!
	\param entry A buffer to receive the entry name for the function
	\param code A buffer to receive the actual code fragment
	\return True if successful, or false if no code was found
	*/
	virtual bool GetVertexShader(MSTR * entry, MSTR * code)=0;
	
	//! Access the pixel shader for the current pass
	/*!
	\param entry A buffer to receive the entry name for the function
	\param code A buffer to receive the actual code fragment
	\return True if successful, or false if no code was found
	*/
	virtual bool GetPixelShader(MSTR * entry, MSTR * code)=0;

	//! Access the profile used for the current pass.
	virtual IGameFX::IGameFXProfile GetTargetProfile()=0;
};

//! An interface holding technique information for the effect.  Each technqiue is made up of one or more passes
class IGameFXTechnique: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IGameFXTechnique() {;}

	//! The number of passes making up the technique
	virtual int GetNumberOfPasses()=0;

	//!The technique name
	virtual const MCHAR * GetTechniqueName()=0;

	//!The number of annotations assigned to the technique
	virtual int GetNumberOfFXAnnotations()=0;

	//! Access the key/value pair
	virtual bool GetFXAnnotation(int index, MSTR * languageType, MSTR * key, MSTR * value)=0;

	//! Access the actual pass
	virtual IGameFXPass * GetIGameFXPass(int index)=0;
};

//! Access function data contained in the effect file
class IGameFXFunction: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IGameFXFunction() {;}

	//! Access the code that makes the function
	virtual const MCHAR * GetFunctionBody()=0;

	//! Access the name of the function
	virtual const MCHAR * GetFunctionName()=0;
};
