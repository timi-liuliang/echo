 /**********************************************************************
 
	FILE:			ISceneStateManager.h

	DESCRIPTION:	Public interface to the scene state manager

	CREATED BY:		Christer Janson, Autodesk, Inc.

	HISTORY:		Created November 2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "FileEnumConstants.h"
#include "GetCOREInterface.h"

class AssetEnumCallback;

#define SCENESTATE_PARTS_COUNT	10

//! \brief This class is the public interface to the scene state feature.
class ISceneStateManager : public FPStaticInterface 
{
public:
	//! \brief Definition of scene state parts.
	/*! Each part is represented by a bit in a BitArray */
	enum SceneStatePart {
		eIncludeLightProperties = 0,
		eIncludeLightTM = 1,
		eIncludeObjectProperties = 2,
		eIncludeCameraTM = 3,
		eIncludeCameraProperties = 4,
		eIncludeLayerProperties = 5,
		eIncludeLayerAssignment = 6,
		eIncludeMaterials = 7,
		eIncludeEnvironment = 8,
		eIncludeAnimationLayerProperties =9,
	};

	//! \brief Get the number of scene states stored in the scene.
	/*! \return The number of scene states stored in the scene. */
	virtual int	GetSceneStateCount() = 0;
	//! \brief Get the name of the scene state at this index.
	/*! \param[in] index - The index of the scene state
	    \return The name of the scene state, or NULL in case of an error. */
	virtual const MCHAR*	GetSceneState(int index) = 0;
	//! \brief Find the index of the scene state with this name.
	/*! \param[in] name - The name of the scene state.
	    \return The index of the scene state, or -1 in case of an error. */
	virtual int FindSceneState(const MCHAR* name) = 0;
	//! \brief Capture, or save, a scene state. All scene state parts will be saved.
	/*! \param[in] name - The name of the scene state
	    \return true if successful, otherwise false. */
	virtual bool CaptureState(const MCHAR* name) = 0;
	//! \brief Capture, or save, a scene state with a specific set of parts.
	/*! \param[in] name - The name of the scene state
	    \param[in] parts - A BitArray representing the scene state parts to save.
	    \return true if successful, otherwise false.
		\see SceneStatePart */
	virtual bool CaptureState(const MCHAR* name, BitArray parts) = 0;
	//! \brief Restore a scene state. All scene state parts included in the scene state will be restored.
	/*! \param[in] name - The name of the scene state to restore
	    \return true if successful, otherwise false. */
	virtual bool RestoreState(const MCHAR* name) = 0;
	//! \brief Restore a scene state.
	/*! \param[in] name - The name of the scene state to restore
	    \param[in] parts - A BitArray representing the scene state parts to restore.
	    \return true if successful, otherwise false.
	    \see SceneStatePart */
	virtual bool RestoreState(const MCHAR* name, BitArray parts) = 0;
	//! \brief Delete the scene state with this name
	/*! \param[in] name - The name of the scene state to delete
	    \return true if successful, otherwise false. */
	virtual bool DeleteState(const MCHAR* name) = 0;
	//! \brief Rename a scene state
	/*! \param[in] oldName - The name of the scene state to rename
	    \param[in] newName - The new name of the scene state
	    \return true if successful, otherwise false. */
	virtual bool RenameState(const MCHAR* oldName, const MCHAR* newName) = 0;
	//! \brief Get the parts that are stored in a specific scene state
	/*! \param[in] name - The name of the scene state
	    \return A bitarray representing the parts that the scene state contains.
		\see SceneStatePart */
	virtual BitArray GetSceneStateParts(const MCHAR* name) = 0;
	//! \brief Open the Scene State manager dialog.
	virtual void DoManageDialog() = 0;
	//! \brief Enumerate external files used by objects stored in the scene states
	/*! \param[in] assetEnum - The callback where external files are logged.
	    \param[in] flags - Standard EnumAuxFiles flags. */
	virtual void EnumAuxFiles(AssetEnumCallback& assetEnum, DWORD flags = FILE_ENUM_ALL) = 0;

	// Function IDs for function publishing
	enum 
	{ 
		E_GET_COUNT,
		E_GET,
		E_FIND,
		E_CAPTURE,
		E_RESTORE,
		E_DELETE,
		E_RENAME,
		E_GET_PARTS,
		E_GET_NUM_PARTS,
		E_PART_TO_INDEX,
		E_INDEX_TO_PART,
		E_CAPTURE_ALL_PARTS,
		E_RESTORE_ALL_PARTS
	}; 
};

//! \brief Scene State Manager interface ID
#define IID_ISceneStateManager Interface_ID(0xECA7BD8E, 0xD3934669)

//! \brief Returns the interface to the Scene State Manager singleton object
inline ISceneStateManager* GetSceneStateManager() { return static_cast<ISceneStateManager*>(GetCOREInterface(IID_ISceneStateManager)); }

