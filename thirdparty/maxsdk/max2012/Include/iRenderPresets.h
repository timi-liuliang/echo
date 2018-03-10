/////////////////////////////////////////////////////////////////////////
//
//
//	Render Preset Interface
//
//	Created 5/14/2003	Tom Burke
//

#pragma once

#include "maxheap.h"
#include "bitarray.h"
#include "tab.h"
#include "strbasic.h"

/*! \defgroup renderPresetCatDef Render Preset Category Defines
\brief Each set of properties that can be saved is called a Category.
\remarks For example, all the properties in the Common tab of the render dialog are
a single category. Each category has both a name and an index number. The possible
numbers range from 0 to RENDER_PRESETS_CATEGORY_COUNT. Note that some of these are
reserved for internal use.\n
Developers wishing to define custom categories should use a category number RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN
or higher.\n\n
A list of categories is represented as a bitarray; if bit N is set to true, this means the category with ID number N is present. */
//@{
#define RENDER_PRESETS_CATEGORY_COUNT				64
#define RENDER_PRESETS_CUSTOM_CATEGORY_INDEX_BEGIN	32
//@}

// return values
//
#define RENDER_PRESETS_SUCCESS				0
#define RP_ERROR_OBSOLETE_FILE				1
#define RP_ERROR_INCOMPATABLE_FILE			2
#define RP_ERROR_CATEGORY_NOT_COMPATABLE	3
#define RP_ERROR_SAVING_FILE				4
#define RP_ERROR_LOADING_FILE				5
#define RP_ERROR_CANCEL						6
#define RP_ERROR_FILE_NOT_FOUND				7


class RenderPresetsContext: public MaxHeapOperators {
public:
	RenderPresetsContext( int version, BitArray categories ) { mVersion = version; mCategories = categories; }
	BitArray GetCategories() { return mCategories; }
	int       GetVersion() { return mVersion; }

private:
	int      mVersion;
	BitArray mCategories;
};

/*! \sa Class Renderer, the <a href="ms-its:3dsMaxSDKRef.chm::/group__render_preset_cat_def.html">Render Preset Category Defines</a>
\par Description:
The Render Preset Manager allows for loading and saving of render presets through the SDK. The manager can be obtained as follows:\n\n
GetCOREInterface()->GetRenderPresetsManager(); */
class IRenderPresetsManager: public MaxHeapOperators {
public:
	enum RendererSpecifier
	{
		kProduction = 0,
		kMaterial,
		kActiveShade
	};

	enum NodeRefAction
	{
		kUnspecified = -1,
		kLoadAll = 0,
		kLoadNonNodeRefMakers = 1,
		kLoadNone = 2,
	};

	//! \brief Destructor
	virtual ~IRenderPresetsManager() {;}
  
    /*! \brief Save selected categories of render presets to the file
	\remarks This function has the same effect as selecting "Save Preset..." in 
		"Rendering->Render Setup->Preset" drop-down list in Max
	\param rendSpecifier Which renderer to save the presets of
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */
	virtual int      Save( RendererSpecifier rendSpecifier ) = 0;
	/*! \brief Save selected categories of render presets to the file
	\remarks When calling this function, the user will be presented with a dialog to select 
		which presets to save for the specified render category.
	\param rendSpecifier Which renderer to save the presets of
	\param fileName The file to save the user-selected presets to.
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */	
	virtual int      Save( RendererSpecifier rendSpecifier, const MCHAR * fileName ) = 0;
	/*! \brief Save selected categories of render presets to the file
	\param rendSpecifier Which renderer to save the presets of
	\param fileName The file to save the user-selected presets to.
	\param saveCategories A BitArray specifying the presets categories to be saved.
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */	
	virtual int      Save( RendererSpecifier rendSpecifier, const MCHAR * fileName, BitArray saveCategories ) = 0;

    /*! \brief Load selected categories of render presets from the file
	\remarks Calling this function has the same effect as selecting "Load Preset..." in 
		"Rendering->Render Setup->Preset" drop-down list in Max
	\param rendSpecifier Which renderer to load the presets for
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */	
	virtual int      Load( RendererSpecifier rendSpecifier ) = 0;
	/*! \brief Load selected categories of render presets from the file
	\remarks Calling this function will display a dialog prompting the user to
		select which presets in fileName to load.
	\param rendSpecifier Which renderer to load the presets for
	\param fileName A path to file to load the render presets from
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */	
	virtual int      Load( RendererSpecifier rendSpecifier, const MCHAR * fileName ) = 0;
	/*! \brief Load selected categories of render presets from the file
	\remarks Calling this function will pop a dialog prompting the user to
		select which presets in fileName to load.
	\param rendSpecifier Which renderer to load the presets for
	\param fileName A path to file to load the render presets from
	\param loadCategories The preset categories to for the specified renderer
	\param nodeRefAction One of the NodeRefAction enum values
	\returns RENDER_PRESETS_SUCCESS on success, or one of the RP_ERROR values if an error occurs */	
	virtual int      Load( RendererSpecifier rendSpecifier, const MCHAR * fileName, BitArray loadCategories, NodeRefAction nodeRefAction = kUnspecified ) = 0;

	/*! \brief  Check to see if file is compatable with current renderer
	\remarks For Example: If the user saves a preset for the Default Scanline Renderer, 
		but does NOT choose the category called "Default Scanline Renderer", then the preset is
		considered incompatible with any other renderer. This is because the renderer itself will
		not be saved within the preset, so loading the preset will not change which renderer is
		active; but the render-specific parameters in the preset are not compatible with other renderers.
	\param rendSpecifier The category to check compatability for
	\param fileName The saved render preset file
	\return TRUE if the renderer currently assigned to rendSpecifier can load the presets in fileName */
	virtual int      IsFileCompatible( RendererSpecifier rendSpecifier, const MCHAR * fileName ) = 0;  

	// Convert between category index and category name for render preset files

	/*! \brief Retrieve a render presets name based on its ID
	\param fileName The file containing the preset we are looking for the name to
	\param catIndex The ID of the category saved in fileName to retrieve the name of
	\return the name of the specified render preset */
	virtual MCHAR *  MapIndexToCategory( const MCHAR * fileName, int catIndex  ) = 0;
	/*! \brief retrieve a render presets ID based on its name
	\param fileName The file containing the preset we are looking for the ID for
	\param category The name of the category saved in fileName to retrieve the ID of
	\return the ID of the specified render preset */
	virtual int      MapCategoryToIndex( const MCHAR * fileName, const MCHAR* category ) = 0;

	/*! \brief retrieve a render presets name based on its ID for the current renderers
	\param rendSpecifier Which current renderer to query for the preset category name.
	\param catIndex The ID of the render preset to retrieve the name of
	\return the name of the specified render preset */
	virtual MCHAR *  MapIndexToCategory( RendererSpecifier rendSpecifier, int catIndex  ) = 0;
	/*! \brief retrieve a render presets ID based on its name for the current renderers
	\param rendSpecifier Which current renderer to query for the preset category ID.
	\param catIndex The name of the category to retrieve the ID of
	\return the ID of the specified render preset if found, else -1 */
	virtual int      MapCategoryToIndex( RendererSpecifier rendSpecifier, const MCHAR* category ) = 0;

	/*! \brief return a list of all preset category ID's saved in a given preset file.
	\param fileName The file to query for saved preset categories */
	virtual BitArray LoadCategories( const MCHAR * fileName ) = 0; // list of categories saved in the file
	/*! \brief return a list of all preset category names saved in a given preset file.
	\param fileName The file to query for saved preset categories */
	virtual Tab<MCHAR *> LoadCategoryNames( const MCHAR * fileName ) = 0;

	/*! \brief return a list of all preset category ID's that are meaningful to the specified renderer.
	\param rendSpecifier Which current renderer to query for meaningful categories
	\return An array specifying the preset categories ID's that are applicable to the current renderer */
	virtual BitArray SaveCategories( RendererSpecifier rendSpecifier ) = 0;
	/*! \brief return a list of all preset category names that are meaningful to the specified renderer.
	\param rendSpecifier Which current renderer to query for meaningful categories
	\return An array specifying the preset categories ID's that are applicable to the current renderer */
	virtual Tab<MCHAR *> SaveCategoryNames( RendererSpecifier rendSpecifier ) = 0;

	/*! \brief Retrieve the current RenderPresetContext. 
	\remarks This is provided so that renderers can tailor their 
	save and load methods to accomodate the saving and loading of 
	any combination of render preset categories.\n\n
	This method will return NULL unless a render preset is currently being loaded or saved.
	An example usage would be to call this inside a Load() method or post-load callback, 
	and if not NULL, check for compatibility by looking at the version number and categories 
	of the preset context.
	\return the current RenderPresetContext if present, else NULL. */
	virtual RenderPresetsContext * GetContext() = 0;
};

