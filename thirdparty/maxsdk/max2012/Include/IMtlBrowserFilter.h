 /**********************************************************************
 
	FILE:           IMtlBrowserFilter.h

	DESCRIPTION:    Public interface for filtering the contents of the
					material/map browser.

	CREATED BY:     Daniel Levesque, Discreet

	HISTORY:        Created 23 February 2003

 *>	Copyright (c) 2003, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "ifnpub.h"
#include "baseinterface.h"
#include "GetCOREInterface.h"
// forward declarations
class IMtlBrowserFilter;
class MtlBase;
class ClassDesc;

#define IMTLBROWSERFILTER_MANAGER_INTERFACEID Interface_ID(0x371b4b10, 0x6c715bbc)
#define IMTLBROWSERFILTER_INTERFACEID Interface_ID(0x1d1f513b, 0x6f315a24)
#define ISUBMTLMAP_BROWSERFILTER_INTERFACEID Interface_ID(0x17304fc3, 0x1ad25941)

//==============================================================================
// class IMtlBrowserFilter_Manager
/*! \sa Class IMtlBrowserFilter, Class ISubMtlMap_BrowserFilter
	\par Description
	This is the manager for the material and texture map browser filters. 
	It is used to add, remove and access filters for the browser. */
//==============================================================================
class IMtlBrowserFilter_Manager : public FPStaticInterface {

public:

	/*! \brief Adds a filter to the list.
		\param filter - A new filter to add to the browser
		\return false if filter could not be added because duplicate. */
	virtual bool AddFilter(IMtlBrowserFilter& filter) = 0;

	/*! \brief Remove a filter from the list. 
		\param filter The filter to be removed
		\return false if filter could not be removed because not found. */
	virtual bool RemoveFilter(IMtlBrowserFilter& filter) = 0;

	/*! \brief Query the list of filters currently assigned
		\return the number of applied filters */
	virtual int GetNumberFilters() = 0;
	
	/*! \brief Query the list of filters currently assigned
		\param i - the index of the filter to return
		\return The filter at index i */
	virtual IMtlBrowserFilter* GetFilter(unsigned int i) = 0;
	
	/*! \brief Returns whether the i-th filter is enabled */
	virtual bool FilterEnabled(unsigned int i) = 0;
	
	/*! \brief Enables/disables the i-th filter */
	virtual void EnableFilter(unsigned int i, bool enable) = 0;

	/*! \brief Get the name of the i-th filter */
	virtual const MCHAR* GetFilterName(unsigned int i) = 0;

	//! \brief Run the given material/map through all the filters in the list.
	/*! Returns true if and only if all filters accept to include that material/map.
		\param mtlBase - the Material to be filtered
		\param flags - See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_material_browser_flags.html">List of Material Browser Flags</a>.
		\return true if every filter accepts mtlBase */
	virtual bool Include(MtlBase& mtlBase, DWORD flags) = 0;


	//! \brief Run the given ClassDesc through all the filters in the list.
	/*!	Returns true if and only if all filters accept to include that ClassDesc.
		\param classDesc - The material/map class descriptor to be filtered
		\param flags - See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_material_browser_flags.html">List of Material Browser Flags</a>.
		\return true if every filter accepts mtlBase */
	virtual bool Include(ClassDesc& classDesc, DWORD flags) = 0;
};

inline IMtlBrowserFilter_Manager* Get_IMtlBrowserFilter_Manager() {

	return static_cast<IMtlBrowserFilter_Manager*>(GetCOREInterface(IMTLBROWSERFILTER_MANAGER_INTERFACEID));
};

//==============================================================================
// class IMtlBrowserFilter
/*! \sa Class IMtlBrowserFilter_Manager, Class ISubMtlMap_BrowserFilter

	\par Description
	This is the base class for all mtl/map browser filters.
	This public interface provides developers the ability to filter the contents of
	the material/map browser based on certain criteria of the material or texture.\n\n
	With the introduction of mental ray, certain materials or textures were unusable by the 
	scanline renderer or even as sub materials. This meant a system was needed to filter 
	out unsupported materials. A material that hosts sub materials may require a specific 
	type of sub material, e.g. a material that supported displacement evaluation. The material 
	can then filter the list for only materials that support displacement. \n\n
	The system is made of a manager (class IMtlBrowserFilter_Manager) and a filter 
	(class IMtlBrowserFilter). The mtl/texmap browser queries the manager, before adding any 
	mtl/texmap to its list, to determine whether each item should be included or filtered out.

	\par Implementation 
	All a developer has to do to access this feature is to implement a filter, 
	derived from class IMtlBrowserFilter, and add it to the filter manager. */
//==============================================================================
class IMtlBrowserFilter : public BaseInterface {

public:

	/*! \brief Returns the name of the filter */
	virtual const MCHAR* FilterName() = 0;

	/*! \brief Returns if this filter is enabled.  The filter manager will not call disabled filters.
		\return true if this filter should be applied */
	virtual bool Enabled() = 0;

	/*! \brief Used to enable/disable this filter.
		\param enable - if false, disable this filter by returning false from Enabled */
	virtual void Enable(bool enable) = 0;

	/*! \brief Called when this filter is added to the manager */
	virtual void Registered() = 0;

	/*! \brief Called when a filter is removed from the manager */
	virtual void Unregistered() = 0;

	/*! \brief Returns whether the given material/map should be included in the browser,
		when browsing for existing materials/maps.
		\param mtlBase - The material/texmap to be filtered
		\param flags - See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_material_browser_flags.html">List of Material Browser Flags</a>.
		\return true if this material should be included in the browser */
	virtual bool Include(MtlBase& mtlBase, DWORD flags) = 0;

	/*! \brief Returns whether the given material/map should be included in the browser,
		when browsing for 'new'.
		\param classDesc - The material/map class descriptor to be filtered
		\param flags - See the <a href="ms-its:listsandfunctions.chm::/idx_R_list_of_material_browser_flags.html">List of Material Browser Flags</a>.
		\return true if this material should be included in the browser */
	virtual bool Include(ClassDesc& classDesc, DWORD flags) = 0;

	// -- from BaseInterface
	virtual Interface_ID GetID();
};

//==============================================================================
// class ISubMtlMap_BrowserFilter

/*! \sa Class IMtlBrowserFilter_Manager, Class IMtlBrowserFilter

	\par Description:
	A Mtl/Texmap plugin which wishes to specify filters for its submaps/submaterials
	may implement this interface. The material editor will use the filters returned
	by this interface when browsing for sub materials or texmaps. */
//==============================================================================
class ISubMtlMap_BrowserFilter : public BaseInterface {

public:

	/*! \brief Returns a filter for the i-th sub-texmap (indices are identical to
		those used with class ISubMap).
		\param i - The index of the filter to return
		\return The filter at index i */
	virtual IMtlBrowserFilter* GetSubMapFilter(unsigned int i) = 0;

	//! \brief Returns a filter for the i-th sub-material.
	/*! The (indices are identical to those used with class Mtl). 
			Note that this is only useful for Mtl plugins.
		Texmap plugins should return NULL.
		\param i - The index of the filter to return
		\return The filter at index i */
	virtual IMtlBrowserFilter* GetSubMtlFilter(unsigned int i) = 0;
	
	// -- from BaseInterface
	virtual Interface_ID GetID();

};

inline ISubMtlMap_BrowserFilter* Get_ISubMtlMap_BrowserFilter(InterfaceServer* iserver) {

	return static_cast<ISubMtlMap_BrowserFilter*>(iserver->GetInterface(ISUBMTLMAP_BROWSERFILTER_INTERFACEID));
}

//==============================================================================
// class IMtlBrowserFilter inlined methods
//==============================================================================

inline Interface_ID IMtlBrowserFilter::GetID() {

	return IMTLBROWSERFILTER_INTERFACEID;
}

//==============================================================================
// class ISubMapBrowserFilter inlined methods
//==============================================================================

inline Interface_ID ISubMtlMap_BrowserFilter::GetID() {

	return ISUBMTLMAP_BROWSERFILTER_INTERFACEID;
}

