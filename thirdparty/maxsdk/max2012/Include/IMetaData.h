//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
/**************************************************************************
	FILE: IMetaData.h

	DESCRIPTION: Declares a public header for a function publish interface
		for creating and managing meta-data.
					- interface ID
					- FP command ID's
					- Static FP interface declaration

	AUTHOR: leonarni - created 29-AUG-2007
/***************************************************************************/

#pragma once

#include "iFnPub.h"
#include "iparamb2.h"		// PB2Value
#include "GetCOREInterface.h"
// forward declarations
class CustAttrib;

//! \brief The unique interface ID for the Meta-Data Manager.
#define IID_METADA_MANAGER Interface_ID(0xdcdd1738, 0x15ee4791)

//! \brief The interface for creating, adding, removing and keeping track of meta-data on max Animatables.
/*! Meta-data are custom attributes that contain only a parameter block.  In order to 
	create meta-data, you have to create its definition by specify the parameters first.
	This will give you a meta-data ID.  You can then use that ID to add, remove or list
	meta-data in the scene.
\see CustAttrib
*/
class IMetaDataManager : public FPStaticInterface 
{
public:

	typedef Class_ID MetaDataID;

	enum ControlAlign
	{
		eAlignLeft,
		eAlignCenter,
		eAlignRight
	};
	
	//! \brief A description for a parameter to be used in a meta-data, and its UI in an eventual rollout.
	/*! There are reasonable defaults for most of the fields.  The ones that have to be filled are:
			- m_name: the name of the parameter.  It cannot contain single quotes.  All non-alphanumerical
			characters will be replaced with underscores, but the control label will show them.
			- m_dataType: for now, supported types are limited to TYPE_STRING, TYPE_FLOAT, TYPE_BOOL and TYPE_INT
			- m_ctrlType: should be appropriate for m_dataType.  Supported types are limited to 
			TYPE_EDITBOX, TYPE_SPINNER, TYPE_SINGLECHEKBOX, TYPE_CHECKBUTTON and TYPE_SLIDER
			- m_ctrlLabel: the label for the corresponding label.  If not defined, m_name will be used
			- m_defValue: a default value for the parameter
	*/
	struct ParamDescriptor
	{
		ParamDescriptor():
			m_name(NULL),
			m_ctrlLabel(NULL),
			m_dataType(TYPE_STRING),
			m_ctrlType(TYPE_EDITBOX),
			m_width(-1),
			m_height(-1),
			m_offset(0, 0),
			m_ctrlAlign(eAlignCenter),
			m_sliderHorizontal(true),
			m_sliderTicks(0),
			m_labelOnTop(false),
			m_highlightColor(0.99f, 0.86f, 0.03f)
		{
		}

		MCHAR*		m_name;			// parameter name
		ParamType2	m_dataType;		// parameter data type
		PB2Value	m_defValue;		// default value

		// unused for now
		ULONGLONG	m_flags;		// per-parameter constructor flags (P_ANIMATABLE, P_TRANSIENT, etc.)

		// Optional fields (for integer or float parameters)
		PB2Value	m_rangeLow;		// low range value
		PB2Value	m_rangeHigh;	// high range value

		// UI
		ControlType2 m_ctrlType;	// Type of UI control
		MCHAR*		m_ctrlLabel;	// Control label
		int			m_width;
		int			m_height;
		IPoint2		m_offset;		// x and y offset in pixels, for fine-tuning control placement
		ControlAlign m_ctrlAlign;	

		bool		m_sliderHorizontal;	// If not, vertical.
		int			m_sliderTicks;		// The number of ticks drawn along the slider
		bool		m_labelOnTop;		// For editboxes
		Color		m_highlightColor;	// For checkbutton booleans; color components between 0.0f and 1.0f
	};

	//! \brief Create a meta-data definition for further use in meta-data manager functions.
	/*! The ID returned will allow the user to add or remove meta-data in the scene and
		to keep track of them.
	\param in_custAttribSetName - a name for the custom attribute set.
	\param in_rollOutName - a name for an eventual rollout.
	\param in_paramDefs - an array of descriptions for each parameter in the param block.
	\param out_errMsg - (optional): pointer to a MSTR.  If non-null and the definition 
						cannot be created, will contain a description of the reason.
	\returns  EmptyMetaDataID if for any reason the definition cannot be created:
				- invalid parameter description
				- the definitionID already exists
			  any other MetaDataID value if the definition was created successfully
	*/
	virtual MetaDataID CreateMetaDataDefinition
		(
			MCHAR*			in_custAttribSetName,
			MCHAR*			in_rollOutName,
			const Tab<ParamDescriptor>&	in_paramDefs,
			MSTR*			out_errMsg = NULL
		) = 0;

	//! \brief Create a meta-data instance and add it to the custom attribute container of an animatable.
	/*! If the meta-data already exists for an animatable, it will not get added.
	\param in_metaDataID - the meta-data ID of the custom attribute set to be created.
	\param in_animatable - the Animatable to which the custom attribute set will be added.
	\param in_rank - the insertion index of the custom attribute.  Default is to append
	\returns  the MetaData if created successfully, NULL otherwise
	*/
	virtual CustAttrib* AddMetaDataToAnimatable
		(
			const MetaDataID&	in_metaDataID,	
			Animatable&			in_animatable,
			int					in_rank = -1			// default -1 means append
		) = 0;

	//! \brief Create meta-data instances and add it to the custom attribute container of animatables.
	/*! If the meta-data already exists for an animatable, it will not get added.
	\param in_metaDataID - the meta-data ID of the custom attribute set to be created.
	\param in_animatables - an array of Animatables to which the custom attribute set will be added.
	\param out_metaData - an array of MetaData that were created successfully.
	\param in_shared - if true, a single instance will be created and shared by all animatables.
	\param in_rank - the insertion index of the custom attribute.  Default is to append
	*/
	virtual void AddMetaDataToAnimatables
		(
			const MetaDataID&		in_metaDataID,	
			const Tab<Animatable*>&	in_animatables,
			Tab<CustAttrib*>&		out_metaData,
			bool	in_shared = false,
			int		in_rank = -1			// default -1 means append
		) = 0;

	//! \brief Remove a meta-data from the custom attribute container of an animatable.
	/*! 
	\param in_metaDataID - the meta-data ID of the custom attribute to be removed.
	\param in_animatable - the Animatable from which the custom attribute set will be removed.
	\returns  true if the MetaData was removed
	*/
	virtual bool RemoveMetaDataFromAnimatable
		(
			const MetaDataID&	in_metaDataID,	
			Animatable&	in_animatable
		) = 0;

	//! \brief Returns the number of meta-data in the scene.
	virtual int MetaDataCount() const = 0;

	//! \brief Finds all owners of meta-data of a given meta-data ID
	/*! 
	\param in_ID - the meta-data ID of the meta-data.
	\param out_owners - an array of Animatables that own meta-data of the given ID.
	*/
	virtual void GetAllMetaDataOwners(const MetaDataID& in_ID, Tab<Animatable*>& out_owners) = 0;

	//! \brief Returns whether a definition exists for a given meta-data ID
	/*! 
	\param in_ID - the meta-data ID to be checked.
	\returns  true if the MetaData was defined.
	*/
	virtual bool IsMetaDataDefined(const MetaDataID& in_ID) const = 0 ;

	//! \brief Checks the existence of meta-data of a given ID on an animatable
	/*! 
	\param in_animatable - the animatable to be checked for meta-data.
	\param in_ID - the meta-data ID to be checked.
	\returns  the meta-data if it exists.
	*/
	virtual CustAttrib* GetMetaData(Animatable& in_animatable, const MetaDataID& in_ID) const = 0;

	//! \brief Finds all meta-data on an animatable
	/*! 
	\param in_animatable - the animatable to be checked for meta-data.
	\param out_metaData - an array of the meta-data on the Animatable.
	*/
	virtual void GetAllMetaData(Animatable& in_animatable, Tab<CustAttrib*>& out_metaData) const = 0;

	static IMetaDataManager* GetInstance() {
		return static_cast<IMetaDataManager*>(GetCOREInterface(IID_METADA_MANAGER)); 
	}
};

static const IMetaDataManager::MetaDataID& EmptyMetaDataID = Class_ID(0x00000000, 0x00000000);


// EOF
