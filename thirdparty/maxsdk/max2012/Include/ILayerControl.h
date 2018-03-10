/*==============================================================================
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Interface to Animation Layer Interfaces
// AUTHOR: Michael Zyracki created 2006
//***************************************************************************/

#pragma once

#include "ifnpub.h"
#include "iparamb2.h"
#include "XMLAnimTreeEntry.h"


/*! \internal !*/
#define ANIMLAYERMGR_COMBOBOX_ID   50612
#define ANIMLAYERMGR_EDIT_ID   50613
#define ANIMLAYERMGR_SPINNER_ID   50614

//! The class ID of the singleton that resides in trackview that holds of the animation layer objects
#define MASTERLAYERCONTROLMANAGER_CLASS_ID	Class_ID(0xf4871a5, 0x781f1430)

//! \brief Interface ID for the IAnimLayerControlManager Interface
//! \see IAnimLayerControlManager 
#define IANIMLAYERCONTROLMANAGER_INTERFACE Interface_ID(0xe343ef5, 0x62944bc2)


//! \brief The interface to the functionality for animation layers.

//! This class contains key functionality for creating and manipulation animation layer.s
//! You get this interface by calling  static_cast<IAnimLayerControlManager*>(GetCOREInterface(IANIMLAYERCONTROLMANAGER_INTERFACE ))
class IAnimLayerControlManager  : public FPStaticInterface {

public:

	//! \name Enums
	//@{

	//! \brief Enum for where to create the individual layer controllers when calling EnableAnimLayers
	
	//! \see IAnimLayerControlManager::EnableAnimLayers
	//! ePos means that we will create a layer around the position controller, eRot means that we will create a layer around the rotation controller,
	//! eScale means that we will create a layer around the scale controller, eIk means that we will create layers within an IK node,
	//! eObject means that we will create layers around the object controllers, eCA means that we will create layers around custom attributes,
	//! eMod means that we will create layers around modifier controllers, eMat means that we will create layers around material controllers,
	//! eOther means that we will create layers around any other controller.
	enum Filter{ePos= 0x1, eRot = 0x2, eScale = 0x4,eIK = 0x8, eObject = 0x10,
	eCA = 0x20, eMod = 0x40, eMat = 0x80, eOther = 0x100};

	//! \brief Enum for specifying what type of controller you want a non keyable controller to collapse to when you collapse onto it.

	//!\see IAnimLayerControlManager::SetCollapseControllerType
	//! eBezier means that you will either collapse onto a Bezier controller or a Euler controller.
	//! eLinear means that you will either collapse onto a Linear controller or a TCB controller.
	//! eDefault means that you will collapse onto the default controller for it's type.
	enum ControllerType {eBezier=0x0, eLinear, eDefault};
	//@}

	//! \name Dialog Functions
	//@{

	//! \brief Brings up or hides the main Anim Layers Manager Dialog.  
	
	//! This funciton will bring up or hide the main Anim Layers manager toolbar.
	//! \param[in] show  If true, then show the toolbar, otherwise hide it.
	virtual void ShowAnimLayersManagerToolbar(bool show) =0;

	//! \brief This refreshes the UI in the Anim Layers Manager Dialog, if it's open.
	virtual void RefreshAnimLayersManagerToolbar() =0;

	//! \brief Brings up the Enable Animation Layers Dialog.

	//! This function will bring up a dialog where a user can specify where they want to enable layer animation by creating
	//! layer controllers where specified in the dialog.
	//! \param[in] nodeTab The nodes that the results of the filters chosen in the enable animation layers dialog will apply too.
	virtual int  EnableAnimLayersDlg(Tab<INode *> &nodeTab) =0;
	//@}

	//! \name Core Functionality
	//@{

	//! \brief Enable animation layers on specified nodes given particular filters.

	//! This function enables animations layers on specified nodes by creating layer controllers
	//! as specified by the filter parameter. Note that this function will only create layer controllers,
	//! it doesn' delete layer controllers if it's not specified in the filter.
	/*! \param[in] nodeTab The nodes where the layer controllers will be created.
	//! \param[in] filter Where the layer controllers will be created. \see IAnimLayerControlManager::Filter
	//! \return Returns how many layer controllers where made on the specified nodes.
	!*/
	virtual int  EnableAnimLayers(Tab<INode *> &nodeTab,DWORD filter) =0;
	

	//! Returns whether or not the specified animatible will be able to be converted to a layer control or not.
	//! \param[in] anim The animatable that we want to convert to a layer controller.
	//! \param[in] client The parent of the animatalbe we want to replace.
	//! \param[in] subNum The 'anim' animatable subanim's number on the client.
	//! \return Returns true if we can replace it with a layer controller,else returns false.
	virtual BOOL CanEnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client,int subNum) =0;

	//! This function will enable a layer on the specified anim in the TrackViewPick parameter. If other layers exist on the node
	//! that the layer exists on, then the layer control will sync up with those layers.
	//! \param[in] anim The animatable that we want to replace with a layer controller.
	//! \param[in] client The parent of the animatalbe we want to replace.
	//! \param[in] subnum The 'anim' animatable subanim's number on the client.
	//! \return Returns whether or not we succesful enabled a layer here. Some reasons why we may not be successful is if a
	//! a child subanim is already a layer controller(no nested layers) or the controller is a subanim of a position,rotation or scale,
	//! controller.
	virtual BOOL EnableAnimLayer(ReferenceTarget *anim, ReferenceTarget *client, int subnum) =0;

	//! Return the total number of layers.
	//! \return The total number of layers.
	virtual int	 GetLayerCount()=0;

	//! This function returns which layers exist on the passed in nodes. Note that a layer may only exist on
	//! some, not all of the nodes.
	//! \param[in] nodeTab The nodes where we are checking for layers.
	//! \param[out] layers  The indices of which layers exist on these nodes.
	virtual void GetNodesLayers(Tab<INode *> &nodeTab,Tab<int> &layers)=0;

	//! Sets the layer active
	//! \param[in] index The index to set active. If the index is out of range then nothing is set active.
	virtual void SetLayerActive(int index)=0;

	//! Sets the active layer only on the nodes that are passed in.  Also if that layer doesn't exist on a node
	//! then nothing occurs.
	//! \param[in] index The index to set active. 
	//! \param[in] nodeTab  The nodes whose layer will be set to active.
	virtual void SetLayerActiveNodes(int index,Tab<INode *> &nodeTab)=0;

	//! Adds a layer to the specified nodes. If a layer with that name doesn't exist, a layer with that name is created, 
	//! otherwise the layer with that name will be used to add to those layers. If a node already has that layer, then
	//! nothing happens.
	//! \param[in] name The name of the layer to add to the nodes.
	//! \param[in] nodeTab The nodes which will have the named layer added to them.
	//! \param[in] useActiveControllerType  If true, then the controller type in the active layer will be used to create the new layer,
	//! otherwise we will use the default controller type.
	virtual void AddLayer(MSTR &name, Tab<INode *> &nodeTab, bool useActiveControllerType)=0;

	//! Adds a layer to the specified nodes, by bringing up a dialog that lets you specify the layers name and 
	//! what controller class the new layer will be.
	//! \param[in] nodeTab The nodes which will have the named layer added to them.
	virtual void AddLayerDlg(Tab<INode *> &nodeTab)=0;


	//! Gets which layers are active on the passed in set of nodes.
	//! \param[in] nodeTab The nodes which we are checking for active layers
	//! \param[out] layers The layers that are active on these nodes
	virtual void	 GetActiveLayersNodes(Tab<INode *> &nodeTab,Tab<int> &layers)=0;


	//! Gets which nodes in the scene have the active layer on it.
	//! \param[out] nodeTab The nodes which have the active layer on it.  Note that if we have multiple active layers, then
	//! we will get all of the nodes in those active layers.
	virtual void	 GetNodesActiveLayer(Tab<INode *> &nodeTab)=0;

	//! Delete this layer. Note that this completely deletes the layer, including any weight control, from the system and any nodes where it exists.
	//! \param[in] index The layer to delete. Note that we can't delete the first (0th) layer.
	virtual void DeleteLayer(int index)=0;

	//! Delete this layer from the specified nodes. Note that this only deletes the layer from these nodes, not the system or other nodes.
	//! \param[in] index The layer to delete.  Note that we can't delete the first (0th) layer.
	//! \param[in] nodeTab The nodes where the layer will be deleted.  Nothing happens if the layer doesn't exist on a node.
	virtual void DeleteLayerNodes(int index,Tab<INode *> &nodeTab)=0;

	//! Copy this layer to a buffer so that it may be copied.
	//! \param[in] index The layer to copy.
	//! \param[in] nodeTab The nodes where the layer will be copy.  Nothing happens if the layer doesn't exist on a node.
	virtual void CopyLayerNodes(int index,Tab<INode *> &nodeTab)=0;

	//! Paste the active copied layer at this index on these nodes.  If no layer is copied on a node, then nothing will be pasted.
	//! \param[in] index Where to paste the copied layer over the existing layer.  If the index is -1 or greater than the max number of layers already present, it will paste at the end.
	//! \param[in] nodeTab The nodes where the layer will be pasted.  Nothing happens if the layer doesn't exist on that node.
	virtual void PasteLayerNodes(int index,Tab<INode *> &nodeTab)=0;

	//! Set the layer name at the specified index. If the name already exists, the name won't get changed.
	//! \param[in] index Where to change the name of the layer.
	//! \param[in] name The new name.
	virtual void SetLayerName(int index, MSTR name)=0;

	//! Get the name of the specified layer.
	//! \param[in] index Where to get the name of the layer.
	//! \return Returns the name of the specified layer. If the index is out of bounds, then an empty string is returned.
	virtual MSTR GetLayerName(int index)=0;

	//! Get the layer weight for the specified layer at the specified time.
	//! \param[in] index Where to get the layer weight.
	//! \param[in] t At what time to get the layer weight.
	//! \return Returns the layer weight. 
	virtual float GetLayerWeight(int index,TimeValue t)=0;

	//! Set the layer weight for the specified layer at the speicifed time.
	//! \param[in] index Where to set the layer weight.
	//! \param[in] t At what time to set the layer weight.
	//! \param[in] weight The weight to set. 
	virtual void SetLayerWeight(int index, TimeValue t, float weight)=0;

	//! Get the layer weight control for the specified layer.
	//! \param[in] index Where to get the layer weight.
	//! \return Returns the layer weight control. 
	virtual Control * GetLayerWeightControl(int index)=0;

	//! Set the layer weight control for the specified layer.
	//! \param[in] index Where to set the layer weight.
	//! \param[in] c The weight control we want to set at that index. It needs to be a float controller.
	//! \return Returns whether or not we were able to set the weight control or not.
	virtual bool SetLayerWeightControl(int index, Control *c)=0;


	//! Get whether or not a particular layer is muted or not.
	//! \param[in] index Which layer to see if it's muted.
	//! \return Whether or not the layer is muted or not.
	virtual bool GetLayerMute(int index)=0;

	//! Set whether or not a particular layer is muted or not.
	//! \param[in] index Which layer to mute or not.
	//! \param[in] mute The mute value for the specified layer. 
	virtual void SetLayerMute(int index, bool mute)=0;

	//! Get whether or not a particular layer's output is muted or not.
	//! \param[in] index Which layer to see if it's muted.
	//! \return Whether or not the layer is muted or not.
	virtual bool GetLayerOutputMute(int index)=0;

	//! Set whether or not a particular layer output is muted or not.
	//! \param[in] index Which layer output to mute or not.
	//! \param[in] mute The output mute value for the specified layer. 
	virtual void SetLayerOutputMute(int index, bool mute)=0;

	//! Get whether or not a particular layer is locked. A locked layer cannot be animated
	//! nor collapsed, or pasted over and an object with a locked layer cannot get disabled.
	//! \param[in] index Which layer to see if it's locked.
	//! \return Whether or not the layer is locked or not.
	virtual bool GetLayerLocked(int index)=0;

	//! Set whether or not a particular layer is locked. A locked layer cannot be animated
	//! nor collapsed, or pasted over and an object with a locked layer cannot get disabled.
	//! \param[in] index Which layer output to lock or unlock.
	//! \param[in] locked the locked value for the specified layer. If true the layer is locked
	//! if false it is unlocked.
	virtual void SetLayerLocked(int index, bool locked)=0;


	//! Collapse the layer at the specified index so that it gets deleted and it's keys and values are baked down to the previous layer.  
	//! \param[in] index Where to collapse.  The index must be greater than zero since we can't collapse the first layer.
	//! \param[in] nodeTab The nodes where the layer will be collapsed.  Nothing happens if the layer doesn't exist on that node.
	virtual void CollapseLayerNodes(int index,Tab<INode*> &nodeTab)=0;


	//! Disables and removes any layers on  the specified nodes, by replacing the layer controller that exists
	//! with the first, base layer.  Note that it will disable layer controllers that only have one layer.
	//! \param[in] nodeTab The nodes where layers will be disabled.  Nothing happens if no layers exists or the layer has more than one layer.
	virtual void DisableLayerNodes(Tab<INode*> &nodeTab)=0;


	//! Brings up the Animation Layer Properties Dialog, which lets you set different animation layer properties,
	//! such as whether or not we should automatically mute layers greater than the active one.
	virtual void AnimLayerPropertiesDlg() = 0;

	//! Sets whether or not trackview will only show the current active layer or all of the layers for all of the layer controllers
	//!\ param[in] val If true only active layers will be shown in trackview.
	virtual void SetFilterActiveOnlyTrackView(bool val)=0;

		
	//! Get whether or not trackview will only show the current active layer or all of the layers for all of the layer controllers.
	//! \return Returns whether or trackview will only show the current active layer or all of the layers.
	virtual bool GetFilterActiveOnlyTrackView()=0;

	//! Sets whether or not we will automatically mute layers greater than the active layer.
	//!\ param[in] val If true, automatically mute layers greater than the active layer.
	virtual void SetJustUpToActive(bool v)=0;

	//! Get whether or not we will automatically mute layers greater than the active layer.
	//! \return Returns whether or not we will automatically mute layers greater than the active layer.
	virtual  bool GetJustUpToActive()=0;

	//! Sets the controller type we will collapse non keyable controllers onto.
	//! \param[in] type  The type of controller to collapse too, either Bezier, Linear or Default.
	virtual  void SetCollapseControllerType(IAnimLayerControlManager::ControllerType type)=0;

	//! Get the controller type we will collapse non keyable controllers onto.
	//! \return  Returns the type of controller to collapse non keyable controller onto.  Either Bezier, Linear or Default.
	virtual IAnimLayerControlManager::ControllerType GetCollapseControllerType() = 0;

	//! Sets whether or not we will collapse a controller per frame or we will try to just collapse the keys of the two controllers
	//! that are being merged.  We can only collape the keys if the two controllers are of the same class and are both keyable.
	//! \param[in] v If true it will always collapse per frame, otherwise it will try to only collapse onto keys if it can.
	virtual void SetCollapsePerFrame(bool v) =0;

	//! Gets whether or not we will collapse a controller per frame or we will try to just collapse the keys of the two controllers
	//! that are being merged.  We can only collape the keys if the two controllers are of the same class and are both keyable.
	//! \return If true it will always collapse per frame, otherwise it will try to only collapse onto keys if it can.
	virtual bool GetCollapsePerFrame() =0;

	//! Sets whether or not if collapsing per frame we use the active range, or a specified range. \see IAnimLayerControlManager::SetCollapseRange.
	//! \param[in] v  If true, when we collapse per frame we use the active range in the scene, otherwise we use a specified range.
	virtual void SetCollapsePerFrameActiveRange(bool v) =0;

	//! Gets whether or not if collapsing per frame we use the active range, or a specified range. \see IAnimLayerControlManager::GetCollapseRange.
	//! \return  If true, when we collapse per frame we use the active range in the scene, otherwise we use a specified range.
	virtual bool GetCollapsePerFrameActiveRange() =0;

	//! Sets the collapse range, if we aren't collapsing over the active range interval. \see IAnimLayerControlManager::SetCollapsePerFrameActiveRange.
	//! \param[in] range The range we want to collapse a frame per key over.
	virtual void SetCollapseRange(Interval range) =0;

	//! Gets the collapse range, if we aren't collapsing over the active range interval. \see IAnimLayerControlManager::GetCollapsePerFrameActiveRange.
	//! \return The range we want to collapse a frame per key over.
	virtual Interval GetCollapseRange() =0;
	//@}

	/*! \internal !*/
	virtual void SetUpCustUI(ICustToolbar *toolbar,int id,HWND hWnd, HWND hParent)=0;

	/*! \internal !*/
	virtual MCHAR *GetComboToolTip() =0;

};


//! Class IDs of the Animation Layer Controls. 
#define FLOATLAYER_CONTROL_CLASS_ID		Class_ID(0xba900de, 0x64230ed4)
#define POINT3LAYER_CONTROL_CLASS_ID	Class_ID(0x602b0ddf, 0x455b1b11)
#define POSLAYER_CONTROL_CLASS_ID		Class_ID(0x29b938ee, 0x7fe83f9c)
#define ROTLAYER_CONTROL_CLASS_ID		Class_ID(0x6d7f1859, 0x3f3f0f5e)
#define SCALELAYER_CONTROL_CLASS_ID		Class_ID(0x300e6319, 0x194e7d61)
#define POINT4LAYER_CONTROL_CLASS_ID	Class_ID(0x753822fd, 0xedf52d8)
#define LAYEROUTPUT_CONTROL_CLASS_ID	Class_ID(0x5b6e7ba7, 0x692e4477)

//! \brief The interface to the functionality for animation layers.

//! This class contains key functionality for creating and manipulation animation layer.s
//! You get this interface by calling  static_cast<IAnimLayerControlManager*>(GetCOREInterface(IANIMLAYERCONTROLMANAGER_INTERFACE )


//! \brief Interface ID for the ILayerControl Interface
//! \see ILayerControl
#define LAYER_CONTROLLER_INTERFACE Interface_ID(0x434d7627, 0xa25e6e26)

//! \brief Get the LayerControl Interface 
//! \see ILayerControl
#define GetILayerControlInterface(cd) \
		static_cast<ILayerControl*> ((cd)->GetInterface(LAYER_CONTROLLER_INTERFACE))


//! \brief ILayerControl provides API access to layer control functionality.
/*! The interface is returned by calling GetILayerControlInterface.
*/
class ILayerControl :  public FPMixinInterface {
	public:
		
		enum{ getNumLayers, setLayerActive, getLayerActive,  copyLayer,pasteLayer,
			  deleteLayer, count, setLayerActive_prop, getLayerActive_prop,getLayerName, setLayerName,
			  getSubCtrl, getLayerWeight,setLayerWeight,getLayerMute, setLayerMute,collapseLayer,
			  disableLayer, getLayerLocked, setLayerLocked};
		// FUNCTION_PUBLISHING		
		// Function Map for Function Publish System 
		//***********************************
#pragma warning(push)
#pragma warning(disable:4238)
		BEGIN_FUNCTION_MAP
			FN_0		(getNumLayers,	TYPE_INT,	GetLayerCount				);
			VFN_1		(setLayerActive,				SetLayerActive,		TYPE_INDEX	);
			FN_0		(getLayerActive,	TYPE_INDEX,	GetLayerActive					);
			VFN_1		(copyLayer,					CopyLayer,		TYPE_INDEX	);
			VFN_1		(pasteLayer,				PasteLayer,		TYPE_INDEX	);
			VFN_1		(deleteLayer,				DeleteLayer,		TYPE_INDEX	);
			FN_1		(getLayerName,	  TYPE_TSTR_BV,	GetLayerName,		TYPE_INDEX	);
			VFN_2		(setLayerName,					SetLayerName,		TYPE_INDEX, TYPE_STRING );
			RO_PROP_FN	(count,					GetLayerCount,	TYPE_INT	); 
			PROP_FNS	(getLayerActive_prop, GetLayerActive, setLayerActive_prop, SetLayerActive, TYPE_INDEX	); 
			FN_1		(getSubCtrl,	TYPE_CONTROL,	GetSubCtrl,	TYPE_INDEX	);
			FN_2		(getLayerWeight,TYPE_FLOAT, GetLayerWeight,TYPE_INDEX,TYPE_TIMEVALUE );
		    VFN_3		(setLayerWeight,SetLayerWeight,	TYPE_INDEX,TYPE_TIMEVALUE,TYPE_FLOAT );
			FN_1		(getLayerMute,TYPE_bool, GetLayerMute,TYPE_INDEX );
		    VFN_2		(setLayerMute,SetLayerMute,	TYPE_INDEX,TYPE_bool );
			VFN_1		(collapseLayer,				CollapseLayer,		TYPE_INDEX	);
			VFN_0		(disableLayer, DisableLayer);
			FN_1		(getLayerLocked,TYPE_bool, GetLayerLocked,TYPE_INDEX );
		    VFN_2		(setLayerLocked,SetLayerLocked,	TYPE_INDEX,TYPE_bool );
	END_FUNCTION_MAP
#pragma warning(pop)
		FPInterfaceDesc* GetDesc();    // <-- must implement 
		//End of Function Publishing system code 
		//***********************************

		//! Return the total number of layers on this layer control
		//! \return The total number of layers.
		virtual int	 GetLayerCount()=0;

		//! Set the layer specified by the index to be active.  Note that this also sets this layer to be active on other layers on
		//! the same node that this layer is on, since layers on a node are synchronized.
		//! \param[in] index The layer to set active.
		virtual void SetLayerActive(int index)=0;

		//! Returns which layer is active.
		//! \return The active layer.
		virtual int	 GetLayerActive()=0;

		//! Delete the specified layer.  Note since layers are synchronized on a node, other layers on the same node will also get deleted.
		//! \param[in] index The layer to delete. 
		virtual void DeleteLayer(int index)=0;

		//! Copy the specified layer so that it may be pasted.  Note since layers are synchronized on a node, other layers on the same node will also get cut.
		//! \param[in] index The layer to cut.
		virtual void CopyLayer(int index)=0;
			
		//! Paste the copied layer to the specified index.  Note since layers are synchronized on a node, other layers on the same node will also get pasted.
		//! If no layer is copied, then nothing happens.
		//! \param[in] index Where the pasted layer will go.
		virtual void PasteLayer(int index)=0;

		//! Set the name of the specified layer.
		//! \param[in] index Which layer to rename.
		//! \param[in] name The new name of the layer. Remember layer names are unique so if a layer exists with this name, nothing happens.
		virtual void SetLayerName(int index, MSTR name)=0;

		//! Get the name of the specified layer.
		//! \param[in] index Which layer's name to get.
		//! \return Returns the layer's name. If the index is invalid, returns an empty string.
		virtual MSTR GetLayerName(int index)=0;
	

		//! Get the layer weight for the specified layer at the specified time.
		//! \param[in] index Where to get the layer weight.
		//! \param[in] t At what time to get the layer weight.
		//! \return Returns the layer weight. 
		virtual float GetLayerWeight(int index,TimeValue t)=0;

		//! Set the layer weight for the specified layer at the speicifed time.
		//! \param[in] index Where to set the layer weight.
		//! \param[in] t At what time to set the layer weight.
		//! \param[in] weight The weight to set. 
		virtual void SetLayerWeight(int index, TimeValue t, float weight)=0;

		//! Get whether or not a particular layer is muted or not.
		//! \param[in] index Which layer to see if it's muted.
		//! \return Whether or not the layer is muted or not.
		virtual bool GetLayerMute(int index)=0;

		//! Set whether or not a particular layer is muted or not.
		//! \param[in] index Which layer to mute or not.
		//! \param[in] mute The mute value for the specified layer. 
		virtual void SetLayerMute(int index, bool mute)=0;

		//! Get a subcontroller from its index. 
		//! \return - a pointer to a Control, or NULL if the index is invalid.
		//! \param[in] in_index Index of the list controller subcontroller
		virtual Control*	GetSubCtrl(int in_index) const =0;

		//! Collapse the layer at the specified index so that it gets deleted and it's keys and values are baked down to the previous layer.  
		//! \param[in] index Where to collapse.  The index must be greater than zero since we can't collapse the first layer.
		virtual void CollapseLayer(int index)=0;

		//! Disables and remove the layer controller, and replace it with the first, base layer.
		//! Note that doing this may delete the layer control also, thus making this interface invalid.
        //! Nothing will happen if the layer controller has more than one layer.
		virtual void DisableLayer()=0;

		//! Get whether or not the layer is locked. A locked layer cannot be animated
		//! nor collapsed, or pasted over and an object with a locked layer cannot get disabled.
		//! \param[in] index Which layer to see if it's locked.
		//! \return Whether or not the layer is locked or not.
		virtual bool GetLayerLocked(int index)=0;

		//! Set whether or not a particular layer is locked. A locked layer cannot be animated
		//! nor collapsed,  or pasted over and an object with a locked layer cannot get disabled.
		//! \param[in] locked the locked value for the specified layer. If true the layer is locked
		//! if false it is unlocked.	
		//! \param[in] index Which layer output to lock or unlock.	
		virtual void SetLayerLocked(int index,bool locked)=0;


};


