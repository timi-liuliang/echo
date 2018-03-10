//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//
#pragma  once

#include "..\export.h"
#include "..\iparamb2Typedefs.h"
#include "..\ref.h"
#include "..\ifnpub.h"
#include "..\GetCOREInterface.h"

class ReferenceTarget;

/** Interface ID of class IShaderManager. */
#define ISHADER_MANAGER_INTERFACE_ID Interface_ID(0x3fc568b8, 0xeea1adf)

/** Interface ID of class IShaderManagerCreator. */
#define ISHADER_MANAGER_CREATOR_INTERFACE_ID Interface_ID(0x56e81398, 0x7f816f4f)

namespace MaxSDK { namespace Graphics {

/** Represents the programmable shader associated with a material or texture map plug-in.
	Materials and texture maps can use programmable shaders to describe their 
	appearance when displayed in the Nitrous viewport and rendered with Quicksilver.
	Programmable shaders allow for a more realistic and complex visual appearance
	than ISimpleMaterial does. Note that 3ds Max 2012 only supports MetaSL shaders.
		
	Plug-ins do not need to derive from IShaderManager. They rather need to create
	instances of it using IShaderManagerCreator::CreateShaderManager(), one for each 
	shader they support. 3ds Max will query the plug-in for its IShaderManager via a
	request for the interface identified by ISHADER_MANAGER_INTERFACE_ID. Typically 
	a plug-in would create an instance of IShaderManager in response to this request
	in its override of Animatable::GetInterface(Interface_ID) and delete it in its 
	destructor by calling IShaderManagerCreator::DeleteShaderManager().
	The following code snippet illustrates how a plug-in manages its IShaderManager 
	instance:
	\code
	#include <graphics\IShaderManager.h>
	// A material plug-in that has a shader
	class MyMtlPlugin : public MtlBase {
	private:
		IShaderManager* mShaderManager;
	public:
		MyMtlPlugin() : mShaderManager(NULL) { 
			// It's not recommended to create the IShaderManager in the plug-in's constructor 
			// since the shader manager needs a pointer to the plug-in, which hasn't been
			// fully constructed yet. Therefore calls the shader manager may make to 
			// virtual methods of the plug-in may lead to a crash.
		}
		~MyMtlPlugin() {
			// Do not call the delete operator in mShaderManager, ask IShaderManagerCreator 
			// to destroy it properly.
			IShaderManagerCreator::GetInstance()->DeleteShaderManager(mpManager);
			mpManager = NULL;
		}

		BaseInterface* GetInterface(Interface_ID iid) {
			if (ISHADER_MANAGER_INTERFACE_ID == iid)
				// In case the plug-in may have several shaders with one IShaderManager 
				// instance for each, it will need to make sure that the IShaderManager
				// interface pointer that is returned corresponds to the currently active 
				// shader.
				return GetShaderManager();
			else
				return MtlBase::GetInterface(iid);
		}

		IShaderManager* GetShaderManager() {
			if (mpManager == NULL) {
				mpManager = IShaderManagerCreator::GetInstance()->CreateShaderManager(
					IShaderManager::ShaderTypeMetaSL, 
					_M("MetaSL shader class name"), // same class name as found in the shader file
					_M("Fully qualified shader file path") // Full path to a xmsl shader file
					this);
			}
			return mpManager;
		}
	};
	\endcode
		
	The parameters of the shader need to be fed with values from the parameters of
	the material or texture map plug-in. This process is facilitated by class 
	IParameterTranslator that plug-ins need to implement.
	\see IParameterTranslator
*/
class IShaderManager : public BaseInterface
{
public:
	/** Supported shader types
	*/
	enum ShaderType
	{
		ShaderTypeMetaSL = 0, /*!< MetaSL shader */
	};

	/** Retrieves the interface ID of IShaderManager.
	*/
	virtual Interface_ID GetID() { return ISHADER_MANAGER_INTERFACE_ID; }

	/** Retrieves the shader's type.
	*/
	virtual ShaderType GetShaderType() const =0;

	/** Retrieves the shader's class name.
	*/
	virtual const MCHAR* GetShaderClassName() const =0;

	/** Retrieves the fully qualified path of the shader file.
	*/
	virtual const MCHAR* GetShaderFile() const =0;
};

/** Factory class for IShaderManager objects. 
		Material and texture map plug-ins based on programmable shaders need to create 
		an IShaderManager for each of the shaders they use. The plug-in is responsible 
		for destroying the IShaderManager instance.
		Call IShaderManagerCreator::GetInstance() to access the sole instance of 
		IShaderManagerCreator. 
*/
class IShaderManagerCreator : public FPStaticInterface
{
public:
	/** Creates an IShaderManager object based on a programmable shader.
	Currently only MetaSL shaders are supported. A MetaSL shader's code is stored 
	in a file. The shader itself has a type and a class name.
	The plug-in is responsible for the life-time management of the IShaderManager 
	instance created by this method. 
	\param [in] type The type of the shader. Currently only MetaSL is supported.
	\param [in] className The shader's class name. 
	\param [in] shaderFileName Fully qualified file path of the file that stores the 
	shader's code. For a MetaSL shader this must be an xmsl file.
	\param[in] pRefTarget A pointer to the material or texture map plug-in the 
	IShaderManager is created for.
	\return A valid IShaderManager instance, or NULL in case of an error such as when the
	specified shader file is not found.
	See IShaderManager for a code example of how to use this method.
	*/
	virtual IShaderManager* CreateShaderManager(
		IShaderManager::ShaderType type,
		const MCHAR* className,
		const MCHAR* shaderFileName,
		ReferenceTarget * pRefTarget) = 0;

	/** Deletes an IShaderManager instance.
	Material and texture map plug-ins that created IShaderManager instances need to
	call this function in order to delete them properly. 
	\param [in] pShaderGraphManager the instance of IShaderManager.
	*/
	virtual void DeleteShaderManager(IShaderManager * pShaderGraphManager) = 0;

	/** Retrieves the sole instance of IShaderManagerCreator.
	*/
	static IShaderManagerCreator* GetInstance()
	{
		return dynamic_cast<IShaderManagerCreator*>(GetCOREInterface(ISHADER_MANAGER_CREATOR_INTERFACE_ID));
	}
};

} } // end namespace