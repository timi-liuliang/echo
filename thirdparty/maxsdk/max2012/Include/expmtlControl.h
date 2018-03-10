/**********************************************************************
 *<
	FILE: expmtlControl.h

	DESCRIPTION: Classes to extend materials and shaders. These class
				 provide control over whether the exposure control
				 is to process the shaded values of the material,
				 and whether the material should use the exposure
				 control to convert self-illumination, reflection and
				 refraction from RGB to scaled physical values.

	CREATED BY: Cleve Ard

	HISTORY:

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ifnpub.h"
#include "imtl.h"

// =====================================================================
// Implementation classes
/*! \defgroup ExposureMaterialControlImpClass ExposureMaterialControl: How to Implement
  \par Group Classes: 
	Class ExposureMaterialControl, Class ExposureMaterialControlImp, Class ExposureMaterialControlDesc, Class AddExposureMaterialControl
  \sa Class Mtl, Class ClassDesc2

  \par Description
  These classes are helpers for material developers that want to
  add ExposureMaterialControl to a material.

  \par Implementation:
  Step 1: Derive the Material from both ExposureMaterialControl
  and ExposureMaterialControlImp. There are two ways to do this
  depending on whether you are using a common base class for several
  materials, or whether you are implementing a single material.\n\n
  If you are implementing a single material that was:
  \code
     class MyMaterial : public Mtl
  \endcode
  the do this:
  \code
     class MyMaterial : public ExposureMaterialControlImp<
                                   MyMaterial,
                                   AddExposureMaterialControl<Mtl> >
  \endcode
  this derives MyMaterial pretty directly from both ExposureMaterialControlImp
  and ExposureMaterialControl.\n\n
  If you are implementing several materials from a common base class:
  \code
     class MyBase : public Mtl
     class MyMaterial1 : public MyBase
     class MyMaterial2 : public MyBase
  \endcode
  then do this:
  \code
     class MyBase : public Mtl, public ExposureMaterialControl
     class MyMaterial1 : public ExposureMaterialControlImp<MyMaterial1, MyBase>
     class MyMaterial2 : public ExposureMaterialControlImp<MyMaterial2, MyBase>
  \endcode
  Each material you want to add ExposureMaterialControl to that has a
  class descriptor should be derived from ExposureMaterialControlImp
  separately. This allows MAX Script to connect up the interface with
  the class.\n\n
  Step 2: Add these lines to each material class that you derive from
  ExposureMaterialControlImp:
  \code
     typedef ExposureMaterialControlImp<MyMaterial,
                 AddExposureMaterialControl<Mtl> > BaseClass;
     static ExposureMaterialControlDesc msExpMtlControlDesc;
  \endcode
  or
  \code
     typedef ExposureMaterialControlImp<MyMaterial1, MyBase> BaseClass;
     static ExposureMaterialControlDesc msExpMtlControlDesc;
  \endcode
  Chose the typedef based on the base class of your material.\n\n
  The definition of msExpMtlControlDesc should look like:
  \code
     ExposureMaterialControlDesc MyMaterial::msExpMtlControlDesc(myMaterialClassDesc,
         IDS_EXPOSURE_MATERIAL_CONTROL,
         IDS_NO_EXPOSURE,
         IDS_INVERTSELFILLUM,
         IDS_INVERTREFLECT,
        IDS_INVERTREFRACT
     );
  \endcode
  myMaterialClassDesc is the class descriptor that is used to create
  MyMaterial. The rest of the parameters are string ids in your resource
  file: \n\n
  <b>IDS_EXPOSURE_MATERIAL_CONTROL</b> is the desription string
  for the interface and should be a localization of
  Exposure Material Control.\n
  <b>IDS_NO_EXPOSURE</b> is the name of the No Exposure Control property.\n
  <b>IDS_INVERTSELFILLUM</b> is the name of the Exposure Control Invert SelfIllum property.\n
  <b>IDS_INVERTREFLECT</b> is the name of the Exposure Control Invert Reflect property.\n
  <b>IDS_INVERTREFRACT</b> is the name of the Exposure Control Invert Refract property.\n\n
  Step 3: Modify LocalRequirements, Requirements and GetRequirements.
  Or isNoExposure() in with your other material requirement flags.
  If the class is a material always modify LocalRequirements and
  also modify Requirements if you override it and don't call
  LocalRequirements to get the local flags. In shaders for StdMat2
  you need to modify GetRequirements.\n\n
  Step 4: Modify GetInterface. If you are already using
  GetInterface(Interface_Id id), then you need to call the
  ExposureMaterialControlImp implementation when you get and id
  that you don't recognize. If you setup the BaseClass typedef use:
  \code
     return BaseClass::GetInterface(id);
  \endcode
  Step 5: Modify your save and load code to save and load the
  values in the interface. Save and Load methods are provided.
  You should Save the interface in a separate chunk.\n\n
  Step 6: Modify your Shade method. This is an example of how
  you can do that.\n\n
  \code
    if (sc.globContext != NULL && sc.globContext->pToneOp != NULL) {
         if (isInvertSelfIllum())
             sc.globContext->pToneOp->RGBToScaled(selfIllumOut);
         if (isInvertReflect())
             sc.globContext->pToneOp->RGBToScaled(reflIllumOut);
         if (isInvertRefract())
             sc.globContext->pToneOp->RGBToScaled(transIllumOut);
     }
  \endcode
  Step 7: Modify the Clone method for the material to copy the
  interface values to the cloned material. I used:
  \code
      mnew->ExposureMaterialControl::operator=(*this);
  \endcode
*/

#define EXPOSURE_MATERIAL_CONTROL Interface_ID(0x153b0cbc, 0x5d6b0879)

//==============================================================================
// class ExposureMaterialControl

/*! 
  \ingroup ExposureMaterialControlImpClass
  \brief This is the inteface class to implement in order to support exposure controls
  \sa Class ExposureMaterialControlImp
  \par Description:
  This class is an extension to allow maxscript to determine whether
  a material's shaded output is to be processed by a tone operator
  and whether the self-illumination, reflection or refraction channels
  are to be inverted by the tone operator so the actual rgb values
  will appear in the display.\n\n
  This class is added to materials that want to expose control over
  the results of the exposure control on the shaded output of the
  material. This is useful when a material knows the specific RGB 
  value to appear in the rendered image, and does not want this
  color to be adjusted. For example, a material might already perform 
  its own physically-based lighting calculations with handling for 
  image exposure. Or, in the case of specialty "non-physical" materials, 
  it might be inappropriate to perform exposure adjustments on the color.\n\n
  You determine whether a material allows this control by asking
  for the interface. If the interface is present, then you can
  set values. This is how you ask for the interface:
  \code
     ExposureMaterialControl* exp = GetExposureMaterialControl(mtl);
  \endcode
  A material can support this interface by deriving from this class
  and handling the GetInterface() method as follows:
  \code
  BaseInterface* GetInterface(Interface_ID id) {
	if (id == EXPOSURE_MATERIAL_CONTROL) return (ExposureMaterialControl*)(this);
	return Mtl::GetInterface(id); //If the interface ID is not recognized, call the superclass method
  }
  \endcode
  There are four properties in the interface, each with a get and
  a set method. Use the get method to get the current value and
  set to set a new value for the property.\n\n
  The properties are:\n
\t	NoExposure\n
\t	InvertSelfIllum\n
\t	InvertReflect\n
\t	InvertRefract\n
 
  Also, if you want your material's ExposureMaterialControl methods to be 
  exposed to MAXScript, you can derive from ExposureMaterialControlImp. */
class ExposureMaterialControl : public FPMixinInterface {
public:
	enum {
		kGetNoExposureControl = 0,
		kSetNoExposureControl = 1,
		kGetInvertSelfIllum = 2,
		kSetInvertSelfIllum = 3,
		kGetInvertReflect = 4,
		kSetInvertReflect = 5,
		kGetInvertRefract = 6,
		kSetInvertRefract = 7
	};

	ExposureMaterialControl() :
		mNoExposure(false),
		mInvertSelfIllum(false),
		mInvertReflect(false),
		mInvertRefract(false) {}

	BEGIN_FUNCTION_MAP 
		PROP_FNS(kGetNoExposureControl, GetNoExposure,
			kSetNoExposureControl, SetNoExposure, TYPE_BOOL);
		PROP_FNS(kGetInvertSelfIllum, GetInvertSelfIllum,
			kSetInvertSelfIllum, SetInvertSelfIllum, TYPE_BOOL);
		PROP_FNS(kGetInvertReflect, GetInvertReflect,
			kSetInvertReflect, SetInvertReflect, TYPE_BOOL);
		PROP_FNS(kGetInvertRefract, GetInvertRefract,
			kSetInvertRefract, SetInvertRefract, TYPE_BOOL);
	END_FUNCTION_MAP 

	/*! \remarks Returns the NoExposure property.  If this returns true,
	the shaded output of the material is NOT processed by the exposure 
	control. When the property is false the shaded output is processed 
	by the exposure control. This is useful for materials that aren't 
	interacting with lighting that want specific RGB values to end up in the
	rendered image.
	\return true if the material should NOT be processed by the exposure control. */
	BOOL GetNoExposure() const { return mNoExposure; }

	/*! \remarks Sets the NoExposure property.  If this is set to true,
	the shaded output of the material is NOT processed by the exposure 
	control. When the property is false the shaded output is processed 
	by the exposure control. This is useful for materials that aren't 
	interacting with lighting that want specific RGB values to end up 
	in the rendered image.
	\param on - if true the material should have no exposure */
	virtual void SetNoExposure(BOOL on) { mNoExposure = on != 0; }

	/*! \remarks Gets the InvertSelfIllum property: When this property is true,
	the self-illumination portion of the shaded output is converted from RGB 
	to a physical value, so that the physical value gives the original RGB value 
	when processed by the exposure control. When the property is false
	the self-illumination value is converted by multiplying by the
	physical scale. This is useful if the self-illumination value
	is a RGB value that you want preserved, but you also want lighting
	to interact with the material.
	\return the value of Invert Self Illumination property*/
	BOOL GetInvertSelfIllum() const { return mInvertSelfIllum; }

	/*! \remarks Sets the InvertSelfIllum property: When this property is true,
	the self-illumination portion of the shaded output is converted from RGB 
	to a physical value, so that the physical value gives the original RGB value 
	when processed by the exposure control. When the property is false
	the self-illumination value is converted by multiplying by the
	physical scale. This is useful if the self-illumination value
	is a RGB value that you want preserved, but you also want lighting
	to interact with the material.
	\param on - the new value of Invert Self Illumination property*/
	virtual void SetInvertSelfIllum(BOOL on) { mInvertSelfIllum = on != 0; }

	/*! \remarks  Gets the InvertReflect property: When this property is true, 
	the reflection map portion of the shaded output is converted from RGB to 
	a physical value so that the physical value gives the original RGB value when
	processed by the exposure control. When the property is false
	the reflection map value is converted by multiplying by the
	physical scale. This is useful if the reflection value is not
	being calculated by the renderer from the scene.
	\return the state of the InvertReflect property */
	BOOL GetInvertReflect() const { return mInvertReflect; }

	/*! \remarks Sets the InvertReflect property: When this property is true, 
	the reflection map portion of the shaded output is converted from RGB to 
	a physical value so that the physical value gives the original RGB value when
	processed by the exposure control. When the property is false
	the reflection map value is converted by multiplying by the
	physical scale. This is useful if the reflection value is not
	being calculated by the renderer from the scene.
	\param on - the new state of the InvertReflect property */
	virtual void SetInvertReflect(BOOL on) { mInvertReflect = on != 0; }

	/*! \remarks Gets the InvertRefract property: When this property is true,
	the refraction map portion of the shaded output is converted from RGB to
	a physical value so that the physical value gives the original RGB value when
	processed by the exposure control. When the property is false
	the refraction map value is converted by multiplying by the
	physical scale. This is useful if the reflection value is not
	being calculated by the renderer from the scene.
	\return the state of the InvertRefract property */
	BOOL GetInvertRefract() const { return mInvertRefract; }

	/*! \remarks Sets the InvertRefract property: When this property is true,
	the refraction map portion of the shaded output is converted from RGB to
	a physical value so that the physical value gives the original RGB value when
	processed by the exposure control. When the property is false
	the refraction map value is converted by multiplying by the
	physical scale. This is useful if the reflection value is not
	being calculated by the renderer from the scene.
	\param on - the new state of the InvertReflect property */
	virtual void SetInvertRefract(BOOL on) { mInvertRefract = on != 0; }

	ULONG isNoExposure() const { return mNoExposure ? MTLREQ_NOEXPOSURE : 0; }
	bool isInvertSelfIllum() const { return !mNoExposure & mInvertSelfIllum; }
	bool isInvertReflect() const { return !mNoExposure & mInvertReflect; }
	bool isInvertRefract() const { return !mNoExposure & mInvertRefract; }

	/*! \remarks Save the plugin parameters to disk.\n
	This is implemented by the system to save the exposure parameters,
	any child classes that override this function should explicitly
	call the parent classes Save.  An example of how to do this is as follows
	\code
	MyMaterial::Save(ISave* isave) {
		// Do my saving here
		isave->BeginChunk(PARENT_DATA_CHUNK);
		ExposureMaterialControl::Save(isave);
		isave->EndChunk();
	}
	\endcode
	The ExposureMaterialControl implementation is as follows */
	IOResult Save(ISave* isave);
	
	/*! \remarks Load the plugin parameters.\n
	This is implemented by the system to load the exposure parameters,
	any child classes that override this function should explicitly
	call the parent classes Load.  An example of how to do this is as follows
	\code
	MyMaterial::Load(ILoad* iload) {
		IOResult res;
		USHORT id;
		while (IO_OK==(res=iload->OpenChunk())) {
			switch(id = iload->CurChunkID())  {
				// Any other data chunks need to go here too
			case PARENT_DATA_CHUNK:
				res = ExposureMaterialControl::Load(iload);
				break;
			}
			iload->CloseChunk();
			if (res!=IO_OK) 
				return res;
		}
		return IO_OK;
	}
	\endcode
	The ExposureMaterialControl implementation is as follows */
	IOResult Load(ILoad* iload);

protected:
	enum {
		DATA_CHUNK = 0x322
	};

	IOResult write(ISave* isave, bool& b);
	IOResult read(ILoad* iload, bool& b);

	bool				mNoExposure;
	bool				mInvertSelfIllum;
	bool				mInvertReflect;
	bool				mInvertRefract;
};

inline IOResult ExposureMaterialControl::Save(ISave* isave)
{
	isave->BeginChunk(DATA_CHUNK);
	IOResult res;
	if ((res = write(isave, mNoExposure)) == IO_OK
			&& (res = write(isave, mInvertSelfIllum)) == IO_OK
			&& (res = write(isave, mInvertReflect)) == IO_OK)
		res = write(isave, mInvertRefract);
	isave->EndChunk();
	return res;
}

inline IOResult ExposureMaterialControl::Load(ILoad* iload)
{
	IOResult res;
	USHORT id;
	while (IO_OK==(res=iload->OpenChunk())) {
		switch(id = iload->CurChunkID())  {
		case DATA_CHUNK:
			if ((res = read(iload, mNoExposure)) == IO_OK
					&& (res = read(iload, mInvertSelfIllum)) == IO_OK
					&& (res = read(iload, mInvertReflect)) == IO_OK)
				res = read(iload, mInvertRefract);
			break;
		}

		iload->CloseChunk();
		if (res!=IO_OK) 
			return res;
	}
	return IO_OK;
}

inline IOResult ExposureMaterialControl::write(ISave* isave, bool& b)
{
	DWORD n;
	IOResult res = isave->Write(&b, sizeof(b), &n);
	if (res != IO_OK)
		return res;
	return n == sizeof(b) ? IO_OK : IO_ERROR;
}

inline IOResult ExposureMaterialControl::read(ILoad* iload, bool& b)
{
	DWORD n;
	IOResult res = iload->Read(&b, sizeof(b), &n);
	if (res != IO_OK)
		return res;
	return n == sizeof(b) ? IO_OK : IO_ERROR;
}


inline ExposureMaterialControl* GetExposureMaterialControl(InterfaceServer* mtl)
{
	return static_cast<ExposureMaterialControl*>(
		mtl->GetInterface(EXPOSURE_MATERIAL_CONTROL));
}

//==========================================================================================

//! \ingroup ExposureMaterialControlImpClass
//! \brief This implementation of ClassDesc is to be used to implement the ExposureMaterialControl interface
class ExposureMaterialControlDesc : public FPInterfaceDesc {
public:
	ExposureMaterialControlDesc(
		ClassDesc& cd,
		int			idsDescrString,
		int			idsNoExposureString,
		int			idsInvertSelfIllumString,
		int			idsInvertReflectString,
		int			idsInvertRefractString
	) :
		FPInterfaceDesc(EXPOSURE_MATERIAL_CONTROL, _M("ExposureMaterialControl"),
				idsDescrString, &cd, FP_MIXIN,
			properties,
				ExposureMaterialControl::kGetNoExposureControl,
					ExposureMaterialControl::kSetNoExposureControl, _M("noExposureControl"),
					idsNoExposureString, TYPE_BOOL,
				ExposureMaterialControl::kGetInvertSelfIllum,
					ExposureMaterialControl::kSetInvertSelfIllum, _M("exposureControlInvertSelfIllum"),
					idsInvertSelfIllumString, TYPE_BOOL,
				ExposureMaterialControl::kGetInvertReflect,
					ExposureMaterialControl::kSetInvertReflect, _M("exposureControlInvertReflection"),
					idsInvertReflectString, TYPE_BOOL,
				ExposureMaterialControl::kGetInvertRefract,
					ExposureMaterialControl::kSetInvertRefract, _M("exposureControlInvertRefraction"),
					idsInvertRefractString, TYPE_BOOL,
			end) {}
}; 

//! \ingroup ExposureMaterialControlImpClass
//! \brief This class implements the virtual methods in ExposureMaterialControl
template<class T, class B> class ExposureMaterialControlImp : public B
{
public:
	// Visual Studio has problem with using B::GetInterface
	// This workaround is to define GetInterface(ULONG id)
	// rather than using B::GetInterface. 
	void* GetInterface(ULONG id) { return B::GetInterface(id); }

	BaseInterface* GetInterface(Interface_ID id) {
		if (id == EXPOSURE_MATERIAL_CONTROL)
			return static_cast<ExposureMaterialControl*>(this);
		return B::GetInterface(id);
	}

	FPInterfaceDesc* GetDesc() { return &T::msExpMtlControlDesc; }

private:
};

//! \ingroup ExposureMaterialControlImpClass
//! \brief This class implements a small hack to get around issues in the VS2005 compiler.
template<class B> class AddExposureMaterialControl : public B, public ExposureMaterialControl
{
public:
	using B::GetInterface;
};

