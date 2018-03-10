//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface for Biped for Max 10
// AUTHOR: Susan Amkraut - created March 26, 2007
//***************************************************************************/

#pragma once

#include "biped9Api.h"


/*! \defgroup BipedInterface10 Biped Interface 10
Max 10 Biped export interface
*/
//@{
#define I_BIPMASTER10	0x9170    //!< Biped interface identifier
#define GetBipMaster10Interface(anim) ((IBipMaster10*)(anim)->GetInterface(I_BIPMASTER10))	//!< Gets the Biped interface
//@}

//! \brief Interface into the biped master controller for Max 10.
/*! This contains functions supporting new biped features in Max 10, in particular Xtras.
*/
class IBipMaster10: public IBipMaster9 
{
	public:

	/*! \defgroup BipedObjectRetargetOptions BIP file load max object retargeting options 
	The following are flags used to describe how to retarget max objects loaded from a BIP file */
//@{
	enum {
			kRETARGET_NONE=1,	//!< The position of the objects in the BIP file are not changed
			kRETARGET_HEIGHT=2,	//!< The position of the objects in the BIP file adapt to the size of the target biped based on height
			kRETARGET_LIMB=3	//!< The position of the objects in the BIP file adapt to the size of the target biped based on limb sizes
	};
//@}

	/*! \defgroup BipedSaveOptions BIP file controller and max object save options
	The following are flags used to describe how to save controllers and max objects to BIP files */
//@{
enum {
	kSAVE_NONE=0,		//!< Nothing will be saved
	kSAVE_AUTOMATIC=1,	//!< Only automatically computed items will be saved
	kSAVE_SPECIFIED=2	//!< Only the items specified in the array associated with this parameter will be saved
	};
//@}

	/*! \defgroup BipedLoadOptions BIP file controller and max object load options
	The following are flags used to describe how to load controllers and max objects from BIP files */
//@{
enum {
	kLOAD_NONE=0,		//!< Nothing will be loaded
	kLOAD_ALL=1,			//!< All items in the file will be loaded
	kLOAD_SPECIFIED=2	//!< Only the items specified in the array associated with this parameter and existing in the file will be loaded
	};
//@}

	/*! \name Xtras
	Xtras are extra FK appendages on a biped.  You can have a maximum of \ref MAXXTRAS Xtras.
	The following functions may be used to create, delete, and modify biped Xtras, and to paste Xtra copies.
	*/
//@{
	//! \brief Creates an Xtra with the specified number of links and base name.  
	/*! The base of the Xtra will be attached to the biped's center of mass.
	\pre must be in figure mode
	\see BeginModes(BMODE_FIGURE)
	\param[in] nlinks - the number of links in the new Xtra, between 1 and 25
	\param[in] name - the base name of the Xtra.  
	! The name can be the empty string, and names do not have to be unique.
	\return the base node of the new Xtra if the creation was successful, NULL if the creation was unsuccessful
	*/
	virtual INode *CreateXtra(int nlinks, const MCHAR *name) = 0;

	//! \brief Creates Xtras which are the opposite of the specified Xtra and its children, if desired
	/*! The new Xtras will have the mirrored position, orientation, and hierarchy of the original Xtras.  Xtras which already have opposites are ignored.
	\pre must be in figure mode
	\see BeginModes(BMODE_FIGURE)
	\param[in] xtra - an Xtra node
	\param[in] doChildren - if true, opposites of all the Xtra's children are created in a matching hierarchy
	\return the base node of the opposite of the Xtra sent in if the creation was successful, NULL if the creation was unsuccessful
	*/
	virtual INode *CreateXtraOpposite(INode& xtra, bool doChildren) = 0;

	//! \brief Gets the opposite Xtra
	//! \param[in] xtra - an Xtra node
	//! \return the opposite Xtra if there is one, NULL if there is no opposite
	virtual INode *GetXtraOpposite(INode& xtra) = 0;

	//! \brief Deletes an Xtra
	//! \pre must be in figure mode
	//! \see BeginModes(BMODE_FIGURE)
	//! \param[in] xtrabase - the base node of the Xtra to delete
	//! \return true if the deletion was successful, false if not
	virtual bool DeleteXtra(INode& xtraBase) = 0;

	//! \brief Attaches an Xtra to a parent
	//! \pre must be in figure mode
	//! \see BeginModes(BMODE_FIGURE)
	//! \param[in] xtrabase - the base node of the Xtra to attach
	//! \param[in] parent - the node which will become the Xtra's parent.  The parent must be part of the same biped as the Xtra.
	//! \return true if the attach was successful, false if not
	virtual bool AttachXtra(INode& xtraBase, INode& parent) = 0;

	//! \brief Sets the base name of an Xtra, and uses that to set the names of all the nodes in the Xtra.  
	/*! The first node in the Xtra will be called "name", the second "name01", the next "name02", and so forth.
	\param[in] xtrabase - the base node of the Xtra
	\param[in] name - the base name of the Xtra
	*/
	virtual void SetXtraName(INode& xtraBase, const MCHAR *name) = 0;

	//! \brief Gets the base name of an Xtra
	//! \param[in] xtrabase - the base node of the Xtra
	//! \return the base name of the Xtra
	virtual const MCHAR* GetXtraName(INode& xtraBase) = 0;

	//! \brief Pastes a copy of an Xtra posture onto each Xtra in the tab
	//! \param[in] CP - a biped copy of type posture, which must be a copy of a single Xtra
	//! \param[in] nodes - a tab of Xtras to paste to.  
	//! The copied posture will be pasted onto the entire Xtra track, regardless of which links from the Xtra are in the tab.
	//! \param[in] KeyPasteType - method to paste TCB/IK values, one of the following: PST_DEF (default), PST_COPY (copied), PST_INTERP (interpolated).  
	//! KeyPasteType is only relevant if you are in auto key mode.
	//! \return true if the paste was successful, false if the paste was unsuccessful
	virtual bool PastePostureToXtras(IBipedCopy *CP, INodeTab& nodes, int KeyPasteType) = 0;

	//! \brief Pastes a copy of an Xtra track onto each Xtra in the tab
	//! \param[in] CP - a biped copy of type track, which must be a copy of a single Xtra
	//! \param[in] nodes - a tab of Xtras to paste to.
	//! The copied track will be pasted onto the entire Xtra track, regardless of which links from the Xtra are in the tab.
	//! \return true if the paste was successful, false if the paste was unsuccessful
	virtual bool PasteTrackToXtras(IBipedCopy *CP, INodeTab& nodes) = 0;
//@}

	/*! \name Animation Layer I/O
	The following functions may be used to load and save layers as BIP files.
	*/
//@{
	//! \brief Saves a single layer of a biped animation 
	//! \param[in] fname - the file name to save to - must be a valid path and filename
	//! \param[in] layerIndex - the layer to save.  The first layer is index one.  A zero index will save the base layer without max objects or list controllers.
	//! \see SaveBipedBaseAnimationLayer
	//! \return true if the save was successful, false if the save was unsuccessful
	virtual bool SaveBipedAnimationLayer(const MCHAR *fname, int layerIndex) = 0;

	//! \brief Loads a BIP file into a single layer of a biped animation
	/*! If more than one layer is stored in the BIP file the load will be unsuccessful.
	\pre the layer must exist
	\param[in] fname - the file to load - must be a valid path and filename
	\param[in] layerIndex - the layer into which the animation will be loaded.  The first layer is index one.
	//*   A zero index will load into the base layer without max objects or list controllers.
	\see LoadBipedBaseAnimationLayer
	\return true if the load was successful, false if the load was unsuccessful
	*/
	virtual bool LoadBipedAnimationLayer(const MCHAR *fname, int layerIndex) = 0;

	//! \brief Saves the base layer of a biped animation 
	/*! This is just like a regular BIP file save except that layers, if they exist, are not saved.
	\param[in] fname - the file name to save to - must be a valid path and filename
	\param[in] saveMaxObjects - When kSAVE_NONE, no max objects are saved.
	//* When kSAVE_AUTOMATIC, all IK, Head Target, and linked objects in the scene are saved, and the maxObjects array is ignored.
	//* When kSAVE_SPECIFIED, the max objects specified in the maxObjects array are saved.
	\param[in] maxObjects - an array of nodes in the scene, to be saved only if saveMaxObjects equals SAVE_SPECIFIED.  Biped nodes are always saved, regardless of whether they appear in this array or not.
	\param[in] saveControllers - When kSAVE_NONE, no list controllers are saved.
	//* When kSAVE_AUTOMATIC, all animation controllers assigned to the biped are saved, and the controllers array is ignored.
	//* When kSAVE_SPECIFIED, the controllers specified in the controllers array are saved.
	\param[in] controllers - an array of SaveSubAnimInfo specifying animation controllers, to be saved only if SaveListCntrls equals SAVE_SPECIFIED.  Controllers not associated with Biped nodes are ignored.
	\return true if the save was successful, false if the save was unsuccessful
	*/
	virtual bool SaveBipedBaseAnimationLayer(const MCHAR *fname, 
				int saveMaxObjects, INodeTab *maxObjects,
				int saveControllers, Tab<SaveSubAnimInfo> *controllers) = 0;

	//! \brief Loads a BIP file into the base layer of a biped animation
	/*! If multiple layers are stored in the BIP file the load will be unsuccessful.
	\param[in] fname - the file to load - must be a valid path and filename
	\param[in] MatchFile - when true, the structure of the current biped is changed to match the structure in the BIP file
	\param[in] ZeroFootHeight - when true, sets the lowest starting foot height to Z=0 
	\param[in] promptForDuplicates - When true, opens a Merge dialog for each object in the BIP file that has a duplicate in the scene.
	//* In this dialog, you can Merge, Skip, or Delete Old.  If Prompt for Duplicates is off, the objects in your scene will be automatically overwritten. 
	\param[in] retargetType - method to retarget max objects to biped size, one of the following: RETARGET_NONE, RETARGET_HEIGHT, RETARGET_LIMB
	\see RetargetType
	\param[in] scaleIKObjectSize - when true, the imported objects are scaled in size to match the target biped
	\param[in] loadMaxObjects - When kLOAD_NONE, no max objects are loaded.
	//* When kLOAD_ALL, all max objects stored in the BIP file are loaded, and the maxObjectNames array is ignored.
	//* When kLOAD_SPECIFIED, only the max objects specified in the maxObjectNames array and existing in the BIP file are loaded.
	\param[in] maxObjectNames - an array of names of nodes stored in the BIP file
	\param[in] loadControllers - When kLOAD_NONE, no list controllers are loaded.
	//* When kLOAD_ALL, all animation controllers stored in the BIP file are loaded, and the controllers array is ignored.
	//* When kLOAD_SPECIFIED, only the controllers specified in the controllers array and stored in the BIP file are loaded.
	\param[in] controllers - an array of SaveSubAnimInfo specifying animation controllers stored in the BIP file
	\return true if the load was successful, false if the load was unsuccessful
	*/
	virtual bool LoadBipedBaseAnimationLayer(const MCHAR *fname, BOOL MatchFile, BOOL ZeroFootHeight, 
				BOOL promptForDuplicates, int retargetType, BOOL scaleIKObjectSize,
				int loadMaxObjects, Tab<MCHAR *> *maxObjectNames,
				int loadControllers, Tab<LoadSubAnimInfo> *controllers) = 0;
//@}
};

