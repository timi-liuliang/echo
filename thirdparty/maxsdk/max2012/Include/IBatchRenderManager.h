 /**********************************************************************
 
	FILE:			IBatchRenderManager.h

	DESCRIPTION:	Public interface to the batch renderer

	CREATED BY:		Christer Janson, Autodesk, Inc.

	HISTORY:		Created November 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "Noncopyable.h"
#include "iFnPub.h"
#include "assetmanagement\AssetUser.h"
#include "GetCOREInterface.h"
#include "FileEnumConstants.h"

// forward declarations
class BitmapInfo;
class AssetEnumCallback;

//! \brief This class represent one entry in the list of batch render views.
/*! You can use the properties in this class to read or change the parameters for each batch render view. */
class IBatchRenderView : public FPMixinInterface , public MaxSDK::Util::Noncopyable
{
public:
	//! \brief Get the camera that the batch render view is rendering.
	/*! \return A pointer to the camera node if the view has a valid camera node, NULL otherwise.
	     When NULL, the current viewport will be used for rending */
	virtual INode*			GetCamera() = 0;
	//! \brief Set the camera that the batch render view is rendering.
	/*! \param[in] node - The camera node to use, or NULL to render from the current viewport
	    \return true if the operation was successful, false if an error occurred */
	virtual bool			SetCamera(INode* node) = 0;
	//! \brief Get the width of the rendering output.
	/*! Applicable only when the preset override is enabled.
	    \return The width of the output
	    \see IBatchRenderView::GetOverridePreset */
	virtual int				GetWidth() = 0;
	//! \brief Set the width of the rendering output.
	/*! Applicable only when the preset override is enabled.
	    \param[in] width - The width of the rendered output in pixels
	    \return true if the operation was successful, false if an error occurred
	    \see IBatchRenderView::GetOverridePreset */
	virtual bool			SetWidth(int width) = 0;
	//! \brief Get the height of the rendering output.
	/*! Applicable only when the preset override is enabled.
	    \return The height of the output
	    \see IBatchRenderView::GetOverridePreset */
	virtual int				GetHeight() = 0;
	//! \brief Set the height of the rendering output.
	/*! Applicable only when the preset override is enabled.
	    \param[in] height - The height of the rendered output in pixels
	    \return true if the operation was successful, false if an error occurred
	    \see IBatchRenderView::GetOverridePreset */
	virtual bool			SetHeight(int height) = 0;
	//! \brief Get the start frame of the segment to render.
	/*! Applicable only when the preset override is enabled.
	    \return The time used for the start frame
	    \see IBatchRenderView::GetOverridePreset */
	virtual TimeValue		GetStart() = 0;
	//! \brief Set the start frame of the segment to render.
	/*! Applicable only when the preset override is enabled.
	    \param[in] start - The time of the first frame to render
	    \see IBatchRenderView::GetOverridePreset */
	virtual void			SetStart(TimeValue start) = 0;
	//! \brief Get the end frame of the segment to render.
	/*! Applicable only when the preset override is enabled.
	    \return the time used for the end frame
	    \see IBatchRenderView::GetOverridePreset */
	virtual TimeValue		GetEnd() = 0;
	//! \brief Set the end frame of the segment to render.
	/*! Applicable only when the preset override is enabled.
	    \param[in] end - The time of the last frame to render
	    \see IBatchRenderView::GetOverridePreset */
	virtual void			SetEnd(TimeValue end) = 0;
	//! \brief Get the name of the batch render view.
	/*! \return The name of the batch render view */
	virtual const MCHAR*	GetName() = 0;
	//! \brief Set the name of the batch render view.
	/*! Batch render views must be unique in the same scene.
	    \param[in] name - The new name of the batch render view
	    \return true if the operation was successful, false otherwise. */
	virtual bool			SetName(const MCHAR* name) = 0;
	//! \brief Return the render preset used by the batch render view
	/*! \return The filename of the render preset */
	virtual const MaxSDK::AssetManagement::AssetUser& GetPresetFile() = 0;
	//! \brief Set the filename of the render preset used for the batch render view
	/*! \param[in] preset - The filename of the render preset */
	virtual void			SetPresetFile(const MaxSDK::AssetManagement::AssetUser& preset) = 0;
	//! \brief Get the fully qualified path to the render preset
	/*! \param[out] path - The fully qualified path of the render preset
	    \return true if successful, otherwise false */
	virtual bool			GetFullPresetPath(MSTR& path) = 0;
	//! \brief Get the scene state used by the batch render view
	/*! \return The scene state used by the batch render view, or an empty string in case no scene state is used. */
	virtual const MCHAR*	GetSceneState() = 0;
	//! \brief Set the scene state to be used by the batch render view
	/*! \param[in] sceneState - The name of the scene state */
	virtual void			SetSceneState(const MCHAR* sceneState) = 0;
	//! \brief Check if the batch render view is enabled
	/*! \return true if the batch render view is enabled, false otherwise */
	virtual bool			GetEnabled() = 0;
	//! \brief Enable or disable the batch render view
	/*! \param[in] enabled - Set to true to enable the batch render the view, false to disable it */
	virtual void			SetEnabled(bool enabled) = 0;
	//! \brief Get the parameters of the output file for the batch render view
	/*! \return An object representing the output parameters (filename, filetype, etc...) for the batch render view */
	virtual BitmapInfo*		GetFileOutputInfo() = 0;
	//! \brief Set the parameters of the output file for the batch render view
	/*! \param[in] bi - A BitmapInfo object to use when saving the file */
	virtual void			SetFileOutputInfo(BitmapInfo* bi) = 0;
	//! \brief Get the pixel ascpect ratio used for the rendering.
	/*! Applicable only when the preset override is enabled.
	    \return The pixel aspect ratio
	    \see IBatchRenderView::GetOverridePreset */
	virtual float			GetAspect() = 0;
	//! \brief Set the pixel aspect ratio used for the rendering
	/*! Applicable only when the preset override is enabled.
	    \param[in] pixelAspect - The pixel aspect ratio
	    \see IBatchRenderView::GetOverridePreset */
	virtual void			SetAspect(float pixelAspect) = 0;
	//! \brief Check if the render preset will be overridden for this batch render view.
	/*! If this flag is 'false', the value of the width, height, start, end and aspect parameters will be
	    taken from the render preset, or the current render settings if no preset is used. 
	    Only if this flag is true will these settings be taken from the values in this batch render view
	    \return true if the settings in this batch render view will be used, false if the render preset or current render settings will be used. */
	virtual bool			GetOverridePreset() = 0;
	//! \brief Set whether or not the render preset will be overridden for this batch render view.
	/*! If this flag is 'false', the value of the width, height, start, end and aspect parameters will be
	    taken from the render preset, or the current render settings if no preset is used. 
	    Only if this flag is true will these settings be taken from the values in this batch render view
	    \param[in] override - true to use the settings in this batch render view, false to use the current render settings or render preset. */
	virtual void			SetOverridePreset(bool override) = 0;

	enum 
	{ 
		E_GET_CAMERA,
		E_SET_CAMERA,
		E_GET_WIDTH,
		E_SET_WIDTH,
		E_GET_HEIGHT,
		E_SET_HEIGHT,
		E_GET_START,
		E_SET_START,
		E_GET_END,
		E_SET_END,
		E_GET_NAME,
		E_SET_NAME,
		E_GET_PRESET,
		E_SET_PRESET,
		E_GET_FULL_PRESETPATH,
		E_GET_SCENESTATE,
		E_SET_SCENESTATE,
		E_GET_ENABLED,
		E_SET_ENABLED,

		E_GET_FILENAME,
		E_SET_FILENAME,

		E_GET_ASPECT,
		E_SET_ASPECT,
		E_GET_OVERRIDE_PRESET,
		E_SET_OVERRIDE_PRESET,
	}; 
};

//! \brief This class is the manager of the batch rendering feature.
/*! You can use this class to setup or query the batch renderer. */
class IBatchRenderManager : public FPStaticInterface 
{
public:
	//! \brief Get the number of views currently configured
	/*! \return The number of batch render views */
	virtual int	GetViewCount() = 0;
	//! \brief Get an interface to a given batch render view
	/*! \param[in] i - The index of the batch render view to access
	    \return A batch render view interface, of NULL in case of an error */
	virtual IBatchRenderView* GetView(int i) = 0;
	//! \brief Find a batch render view with the given name.
	/*! \param[in] name - The name of the batch render view to find
	    \return The index of the batch render view with the given name, or NULL if none were found. */
	virtual int	FindView(const MCHAR* name) = 0;
	//! \brief Start the batch renderer. 
	virtual void Render() = 0;
	//! \brief Create a new batch render view using the given camera.
	/*! The batch render view will be added to the batch render list and have a unique name and default values for all it's parameters.
	    \param[in] camera - The camera to render with the batch render view,
		or NULL to render from the current viewport
	    \return A pointer to the newly created batch render view, or NULL if an error occurred. */
	virtual IBatchRenderView* CreateView(INode* camera) = 0;
	//! \brief Delete the given batch render view
	/*! \param[in] i - The index of the batch render view to delete
	    \return true if the entry was deleted, false if an error occurred */
	virtual bool DeleteView(int i) = 0;
	//! \brief Create a new batch render view by copying an existing one
	/*! The new view will be identical to the previous, except for its name. A new unique name will be created.
	    \param[in] i - The index of the batch render view to copy
	    \return A pointer to the newly created batch render view, or NULL in case of an error */
	IBatchRenderView*	DuplicateView(int i);
	//! \brief Returns whether or not network rendering is enabled for the batch renderer
	/*! \return true if network rendering is enabled, false otherwise */
	virtual bool GetNetRender() = 0;
	//! \brief Set whether of not to use network rendering for the batch renderer.
	/*! \param[in] b - True to use network rendering, false to render locally */
	virtual void SetNetRender(bool b) = 0;
	//! \brief Returns whether or not a separate VFB will be opened (and kept open) for each batch render view when rendered.
	/*! In 3ds Max 8 this has no effect, and is for internal use only
	    \return true if a separate VFB is used for each batch render view, false otherwise */
	virtual bool GetKeepVFB() = 0;
	//! \brief Set whether or not a separate VFB will be opened (and kept open) for each batch render view when rendered.
	/*! In 3ds Max 8 this has no effect, and is for internal use only
	    \param[in] b - Set to true to open a separate VFB for each batch render view, false to use the same VFB for all renders. */
	virtual void SetKeepVFB(bool b) = 0;
	//! \brief This will prepare the scene for a given batch render view.
	/*! This will load the preset and restore the scene state from the batch render view.
	    \param[in] batchView - The batch render view to prepare.
	    \return true if the preparation was successful and false if an error occurred. */
	virtual bool PrepareScene(IBatchRenderView* batchView) = 0;
	//! \brief This brings up the batch renderer dialog
	virtual void DoManageDialog() = 0;
	//! \brief This method may be used to enumerate all the render preset files used by Batch Render
	/*! \param[in] nameEnum - The callback, called once for each render preset.
	    \param[in] flags - Indicate what to enumerate. */
	virtual void EnumAuxFiles(AssetEnumCallback& nameEnum, DWORD flags = FILE_ENUM_ALL) = 0;

	// Function IDs for function publishing
	enum 
	{ 
		E_GET_COUNT,
		E_GET_VIEW,
		E_FIND_VIEW,
		E_CREATE_VIEW,
		E_DELETE_VIEW,
		E_DUPLICATE_VIEW,
		E_RENDER,
		E_GET_KEEP_VFB,
		E_SET_KEEP_VFB,
		E_GET_NETRENDER,
		E_SET_NETRENDER
	}; 
};

//! \brief Batch Render Manager interface ID
#define IID_IBatchRenderManager Interface_ID(0x873B70BE, 0x4BEF3BB8)
//! \brief Returns the interface to the Batch Render Manager singleton object
inline IBatchRenderManager* GetBatchRenderManager() { return static_cast<IBatchRenderManager*>(GetCOREInterface(IID_IBatchRenderManager)); }

//! \brief Batch Render View interface ID
#define IID_IBatchRenderView Interface_ID(0xB35ED920, 0xC5270C0B)

