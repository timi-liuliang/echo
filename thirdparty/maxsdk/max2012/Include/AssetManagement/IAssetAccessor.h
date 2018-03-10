/* -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

FILE: IPathAccessor.h

DESCRIPTION: abstract classes for menus

CREATED BY: David Cunnnigham

HISTORY: created February 18, 2005

Copyright (c) 2005, All Rights Reserved

// -----------------------------------------------------------------------------
// -------------------------------------------------------------------------- */

#pragma once

#include "..\baseinterface.h"
#include "..\Path.h"
#include "AssetType.h"
#include "..\AssetEnumCallback.h"

// forward declarations

namespace MaxSDK
{
	namespace AssetManagement
	{
		class AssetUser;
	}
}


//! \brief A callback interface for declaring and modifying an asset reference.

/*! A callback class used in conjunction with IEnumAuxAssetsCallback to declare
    an asset used by this application component.  Accessor methods provide
    high-level information about the assets, and one mutator method - SetPath, 
    is used at the time of declaration to retarget an asset.
    Classes implementing this interface need not worry about maintaining
    a persistent link to this interface.  It should only be instantiated within the
    scope of a call to the client's EnumAuxFiles function.  When the asset is declared,
    at that point the enumerating code may choose to retarget the asset.  
\code 

class PointCacheAssetAccessor : public IAssetAccessor	{
	public:

	PointCacheAssetAccessor(PointCacheBase* aPointCache);

	// path accessor functions
	virtual const MCHAR* GetPath() const ;

	virtual void SetPath(const MSTR& aNewPath) ;

	// asset client information
	virtual int GetAssetType() const ;

protected:
	PointCacheBase* mPointCache;
};

PointCacheAssetAccessor::PointCacheAssetAccessor(PointCacheBase* aPointCache) : 
	mPointCache(aPointCache)
{

}

const MCHAR* PointCacheAssetAccessor::GetPath() const	{
	MCHAR *fname;
	Interval iv;
	mPointCache->pblock->GetValue(pb_cache_file,0,fname,iv);
	return fname;
}


void PointCacheAssetAccessor::SetPath(const MSTR& aNewPath)	{
	mPointCache->pblock->SetValue(pb_cache_file, 0, const_cast<MCHAR*>(aNewPath.data()));
}

int PointCacheAssetAccessor::GetAssetType() const	{
	return IAssetAccessor::kAnimationAsset;
}

void PointCacheBase::EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags) 
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/11/03

	if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
		PointCacheAssetAccessor accessor(this);
		const MCHAR* test = accessor.GetPath();
		if(test && test[0] != _M('\0'))	{
			IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
			callback->DeclareAsset(accessor);		
			//  NOTE:  callback object can call SetPath on
   			//  on the accessor within the scope of this call
		}
	}
	else	{
		// do normal enum business
	}
} 
\endcode
Notice that the accessor does not need to survive beyond the scope which contains the call to

\code
callback->DeclareAsset(IAssetAccessor& anAccessor);
\endcode

<b>Clients must implement GetPath() and SetPath(), and GetAssetType().  The asset type must be 
one of the types declared as AssetType.</b>

A client wishing to declare a new category of assets can do so by returning kOtherAsset
as a type and implementing the GetAssetDesc() and GetAssetTypeIcon() functions.

Finally, other methods exist that allow a client to return varying amounts of information about
an asset.  See methods below for details.
*/
class IAssetAccessor : public InterfaceServer {

public:

	/*!  enum AssetType has been moved to iAsset.h. 
	See iAsset.h for the various assets types supported by this accessor. */
	
	//! \brief Gets the AssetUser pointing on the asset.
	
	//! Gets the full path of the asset, as declared by the asset.
	//! \return the asset's AssetUser
	virtual MaxSDK::AssetManagement::AssetUser GetAsset() const = 0;

	//! Implementing classes should implement this method so that a call
	//! to this method causes the component to point to a newly declared asset.
	//! 
	//! Note: If an error occurs while repathing, the client might choose to indicate
	//! this by returning an error message via GetRetargetErrorMsg().
	//! \pre This call will only ever be made within the scope of an EnumAuxFiles
	//! call on the application component.
	//! \post The component should now be referring to the new asset path passed
	//! into this method.
	//! \param[in] aNewAssetUser An AssetUser of a new asset to refer to.  The path of the AssetUser need not 
	//! be a fully-qualified path.  
	virtual bool SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAssetUser) = 0;
	//! \brief Should return whether this is an input asset.

	//! This method should return true if the asset in question is 
	//! used as an input to a render.  An example of an asset that is
	//! not an input would be a RenderElements output asset.  
	//! \return true if the asset is an input asset
	//!
	//! Default implementation: returns true
	CoreExport virtual bool IsInputAsset() const;

	// asset client information

	//! \brief Returns the asset type for this asset declaration.

	//! Should return an asset type defined in the AssetType enum above.
	//! If the asset does not fall under one of the predefined categories, 
	//! declare the asset as kOtherAsset.  
	//!
	//!In this case, the GetAssetDesc()
	//! should return an appropriate string defining the category to which this
	//! asset belongs.
	//! \return the asset type for this declared asset
	virtual MaxSDK::AssetManagement::AssetType GetAssetType() const =0;

	//! \brief Returns an asset description string, or NULL if this is a standard asset.

	//! A component can normally return NULL for this method if the asset declaration
	//! falls under one of the pre-defined categories.  If an component declares an as
	//! kOtherAsset, then it should return an appropriate asset type description 
	//! (i.e. in the vein of Bitmap, Photometric, etc.)  This description will be used
	//! to visually group assets together.  A third-party developer can therefore define a
	//! new asset group type by consistently returning the same description string for
	//! their declared assets, and this will be reflected appropriately in the 
	//! application interface.
	//! \return NULL, or a string describing the asset type if the asset type returned
	//! for GetAssetType() is kOtherAsset
	//!
	//! Default implementation: returns NULL
	CoreExport virtual const MCHAR* GetAssetDesc() const;
	//! \brief Returns the full path of an icon file associated with a custom asset description.

	//! If the asset type returned by this accessor is kOtherAsset, and the asset description
	//! returned by GetAssetDesc() is non-NULL, then this method should be overwritten to
	//! return the full path of an icon file (*.ico) that should be used as the icon
	//! for this new asset category.  Otherwise, the result from this field is ignored.
	//! \pre GetAssetType() == kOtherAsset, and GetAssetDesc() is non-NULL
	//! \return The icon file (*.ico) that will be used as the icon to represent this
	//! new asset category.
	//!
	//! Default implementation: returns NULL
	CoreExport virtual const MCHAR* GetAssetTypeIcon() const;
	//! \brief A brief description string of the asset client.  

	//! A one or two word description of an asset client.  This description will be used to 
	//! briefly describe the source client of this asset.
	//! \return brief description string of the asset client
	//!
	//! Default implementation: returns NULL
	CoreExport virtual const MCHAR* GetAssetClientDesc() const;
	//! \brief Whether it is possible to retarget this client's asset.

	//! There are cases where an asset can be declared, but cannot be retargeted.  If
	//! this is the case for an implementing client, then \b false should be 
	//! returned here.
	//! \return Whether it is possible to retarget this client's asset
	//!
	//! Default implementation: returns true
	CoreExport virtual bool IsAssetPathWritable() const;

	//! \brief Allows a client to return an error message related to a failed SetPath attempt.

	/*! An asset client can report a SetPath error via this method.  If an error occurs during the
		SetPath call, then this method should report the error.
		A manager which calls SetPath on an IAssetAccessor should call GetRetargetErrorMsg()
		to determine whether the SetPath succeeded.  GetRetargetErrorMsg should return NULL if
		no error occurs.
		   
		\return NULL if there is no error to report, an error string if an error occured
		while repathing the asset
		Default implementation: returns NULL
	*/
	CoreExport virtual const MCHAR* GetRetargetErrorMsg() const;

};

//! \brief An extension of AssetEnumCallback that exposes an interface for providing more detailed information about an asset.

/*! An extension of AssetEnumCallback that allows an application component to declare
    more information about an asset, and to provide a callback method for retargeting
    that asset.
    A client wishing to expose more asset information should follow the following
    code example:
\code

class PointCacheAssetAccessor : public IAssetAccessor	{
public:

	PointCacheAssetAccessor(PointCacheBase* aPointCache);

	// path accessor functions
	virtual MaxSDK::AssetManagement::AssetUser GetAsset() const ;

	virtual bool SetPath(const MaxSDK::AssetManagement::AssetUser& aNewAsset) ;

	// asset client information
	virtual int GetAssetType() const ;

protected:
	PointCacheBase* mPointCache;
};

PointCacheAssetAccessor::PointCacheAssetAccessor(PointCacheBase* aPointCache) : 
	mPointCache(aPointCache)
{

}

MaxSDK::AssetManagement::AssetUser PointCacheAssetAccessor::GetAsset() const	{
	return mPointCache->pblock->GetAssetUser(pb_cache_file);
;
}


bool PointCacheAssetAccessor::SetAsset(const MaxSDK::AssetManagement::AssetUser& aNewAsset)	{
	mPointCache->pblock->SetValue(pb_cache_file, 0, aNewAsset);
}

int PointCacheAssetAccessor::GetAssetType() const	{
	return IAssetAccessor::kAnimationAsset;
}

void PointCacheBase::EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags) 
{
	if ((flags&FILE_ENUM_CHECK_AWORK1)&&TestAFlag(A_WORK1)) return; // LAM - 4/11/03

	if(flags & FILE_ENUM_ACCESSOR_INTERFACE)	{
		PointCacheAssetAccessor accessor(this);
		if(accessor.GetAsset().GetId()!=MaxSDK::AssetManagement::kInvalidId)	{
			IEnumAuxAssetsCallback* callback = static_cast<IEnumAuxAssetsCallback*>(&nameEnum);
			callback->DeclareAsset(accessor);
		}
	}
	else	{
		// do normal enum business
	}
}
\endcode

	If the FILE_ENUM_ACCESSOR_INTERFACE is passed to the EnumAuxFiles, then it
	is safe to cast the AssetEnumCallback object to the IEnumAuxAssetsCallback interface.
	From there, an asset can declare it's own asset interface, as shown in the above
	example.  This example is taken from the <b>maxsdk\\samples\\modifiers\\pointcache</b> sample
	plugin project.
*/
class IEnumAuxAssetsCallback : public AssetEnumCallback	{
public:
	//! \brief Allows a client to implement and return an asset accessor object.

	//! An application component wishing to declare an asset should implement the
	//! IAssetAccessor interface and return an instance of that implementation
	//! via this method.  It is important to note that a link to this interface is
	//! not kept by the IEnumAuxAssetsCallback object receiving the interface.  The 
	//! interface need only be valid for the length of the DeclareAsset method call.
	//! This allows a component to pass in a stack object that will be destroyed at
	//! the end of the scope containing this method call.  
	//! See class description for a code sample.
	//! \param[in] anAccessor An asset accessor object.
	virtual void DeclareAsset(IAssetAccessor& anAccessor) =0;
	//! \brief Allows a client to declare a new asset sub-group.

	//! Allows a client to declare a new asset subgroup.  An example of a sub-group
	//! would be an xref file that has its own assets.  This method declares that we
	//! are now enumerating a sub-group of assets, meaning that declared assets will
	//! be added to this group.  It is possible to nest groups further by making 
	//! repeated calls to this function.  To end the declaration of a group, call
	//! EndGroup().
	//! \pre The asset accessor should not return NULL for a newly declared group.  It
	//! is the onus of the asset client to check that IAssetAccessor::GetPath returns a 
	//! non-null value.  If null is returned, neither DeclareGroup nor EndGroup should be called.
	//! \param[in] anAccessor An asset accessor describing the asset which holds a 
	//! a sub-group of assets.
	virtual void DeclareGroup(IAssetAccessor& anAccessor) =0;
	//! \brief Used to end the declaration of a sub-group of assets.

	//! Called when a client is done declaring a sub-group of assets.  See
	//! DeclareGroup(IAssetAccessor&) for details.
	virtual void EndGroup() =0;
};


