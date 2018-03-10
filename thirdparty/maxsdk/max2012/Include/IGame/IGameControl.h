/**********************************************************************
 *<
	FILE: IGameControl.h

	DESCRIPTION: Controller interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameControl.h
\brief High level access to 3ds Max's controllers, including Biped, constraints and list controllers

<b> Point4 controller access is only available for 3ds Max 6.0 and above</b>
*/
#pragma once

#include "..\maxheap.h"
#include "..\istdplug.h"
#include "IGameProperty.h"
#include "IConversionManager.h"
// forward declarations
class IGameNode;


//! Various IGame Controller types
/*! These controllers types are used to define the type of controller being queried.
*/
enum IGameControlType{
	 IGAME_POS,						/*!<Position Controller*/	
	 IGAME_ROT,						/*!<Rotation Controller*/	
	 IGAME_SCALE,					/*!<Scale Controller*/
	 IGAME_FLOAT,					/*!<Float Controller*/
	 IGAME_POINT3,					/*!<Point3 Controller*/
	 IGAME_TM,						/*!<Used for sampling the node transformation matrix.  This is the only time this control
										type can be used*/		
	 IGAME_EULER_X,					/*!<Euler X controller*/
	 IGAME_EULER_Y,					/*!<Euler Y controller*/
	 IGAME_EULER_Z,					/*!<Euler Z controller*/	

	 IGAME_POINT4,					/*!<Point4 based controllers - <b><i>available from 3ds max 6.0</i></b>*/

	 IGAME_POS_X,					/*!<Independent Position X Controller*/
	 IGAME_POS_Y,					/*!<Independent Position Y Controller*/
	 IGAME_POS_Z,					/*!<Independent Position Z Controller*/
};




class IGameConstraint;
class GMatrix;

//! A generic animation key wrapper class
/*! A generic TCB key class for IGame
*/
class IGameTCBKey: public MaxHeapOperators {
	public:	
		//! Access to basic TCB data
		/*! This provides access to the Tension, continuity, bias and easein/out properties of a TCB Key
		*/
		float tens, cont, bias, easeIn, easeOut;
		//! Float based value 
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float fval;
		//! Point3 based value 
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Point4 based value
		/*! This would be accessed when using the IGameControlType::IGAME_POINT4 specifiers
		*/
		Point4 p4val;
		//! Ang Axis based value 
		/*! This would be accessed when using the IGameControlType::IGAME_ROT specifier
		*/
		AngAxis aval;
		//! Scale based value 
		/*! This would be accessed when using the IGameControlType::IGAME_SCALE specifier
		*/
		ScaleValue sval;
	};

//! A generic animation key wrapper class
/*! A generic Bezier Key class for IGame
*/
class IGameBezierKey: public MaxHeapOperators {
	public:
		//! Float based In and out tangents
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float fintan, fouttan;

		//! Float based value 
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float fval;
		//! Float based tangent lengths
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float finLength, foutLength;

		//! Point3 based In and out tangents
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pintan, pouttan;
		
		//! Point3 based value 
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Point3 based tangent lengths
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pinLength, poutLength;
		//! Quaternion based value 
		/*! This would be accessed when using the IGameControlType::IGAME_ROT specifier
		*/
		Quat qval;
		//! Scale based value 
		/*! This would be accessed when using the IGameControlType::IGAME_SCALE specifier
		*/
		ScaleValue sval;

		//! Point4 based In and out tangents
		/*! This would be accessed when using the IGameControlType::IGAME_POINT4 specifier
		*/
		Point4 p4intan, p4outtan;

		//! Point4 based tangent lengths
		/*! This would be accessed when using IGameControlType::IGAME_POINT4 specifier
		*/
		Point4 p4inLength, p4outLength;

		//! Point4 based value
		/*! This would be accessed when using the IGameControlType::IGAME_POINT4 specifier
		*/
		Point4 p4val;

	};

//! A generic animation key wrapper class
/*! A generic Linear Key class for IGame
*/
class IGameLinearKey: public MaxHeapOperators  {
	public:
		//! Float based value 
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float fval;
		//! Point3 based value 
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Quaternion based value 
		/*! This would be accessed when using the IGameControlType::IGAME_ROT specifier
		*/
		Quat qval;
		//! Scale based value 
		/*! This would be accessed when using the IGameControlType::IGAME_SCALE specifier
		*/
		ScaleValue sval;
	};


//! A generic animation key wrapper class
/*! A generic Sample Key class for IGame.  This is used for unknown controllers or controllers that
simply need to be sampled, this can includes Biped
*/
class IGameSampleKey: public MaxHeapOperators {
	public:
		//! Point3 based value 
		/*! This would be accessed when using the IGameControlType::IGAME_POS or IGameControlType::IGAME_POINT3 specifiers
		*/
		Point3 pval;
		//! Point4 based value 
		/*! This would be accessed when using the IGameControlType::IGAME_POINT4 specifier
		*/
		Point4 p4val;
		//! Float based value 
		/*! This would be accessed when using the IGameControlType::IGAME_FLOAT specifier
		*/
		float fval;
		//! Quaternion based value 
		/*! This would be accessed when using the IGameControlType::IGAME_ROT specifier
		*/
		Quat qval;
		//! Scale based value
		/*! This would be accessed when using the IGameControlType::IGAME_SCALE specifier
		*/
		ScaleValue sval;
		//! GMatrix based value
		/*! This would be accessed when using the IGameControlType::IGAME_TM specifier
		*/
		GMatrix gval;

};


//!Main animation key container
/*! A simple container class for direct Key access of all the available Key types
*/
class IGameKey: public MaxHeapOperators
{
	public:
		//! The time the key was set
		TimeValue t;
		//! Flags various selection states for the key.
		DWORD flags;
		//!The TCB Keys
		/*! This key access would be used if you used one of the IGameControl::GetTCBKeys methods
		*/
		IGameTCBKey tcbKey;
		//!The Bezier Keys
		/*! This key access would be used if you used one of the IGameControl::GetBezierKeys methods
		*/
		IGameBezierKey bezierKey;
		//!The Linear Keys
		/*! This key access would be used if you used one of the IGameControl::GetLinearKeys methods
		*/		
		IGameLinearKey linearKey;

		//!The Sampled Keys
		/*! This key access would be used if you used one of the IGameControl::GetSampledKeys methods
		*/		
		IGameSampleKey sampleKey;
};

/*!\var typedef  Tab<IGameKey> IGameKeyTab
\brief A Tab of IGameKey. Uses 3ds Max Template class Tab
*/
typedef  Tab<IGameKey> IGameKeyTab;



//!A simple access class for controllers.
/*! IGameControl provides a simplified access to the various key frame controllers used throughout 3ds Max.  In 3ds Max a controller
needs to be queried for the key interface and then its class ID checked before casting to the appropriate Key class.  This class
provide the developer with all the keys based on the key type being asked for.  As the game engine may only support certain type 
of max controllers it is far more efficient for a developer to ask IGame for all the Bezier Postion keys then to check with 3ds Max for 
the controller type.  This class also provides direct support for Euler Rotation controllers.  The developer can use IGameControl::GetControlType
to see if the rotation is Euler and can then can use IGameControlType::IGAME_EULER_X in the appropriate control access type to retrieve the keys.
\n
In 3ds Max some controllers such as TCB, Linear, and Bezier support direct access to their keys.  Other controllers are more private
and usually base them selves on a float or Point3 controller.  If there is no direct access then sampling is the easiest choice.  IGame
supports two types - Full and Quick.  Full samples across the full animation range, whilst Quick only samples where keys are found.  The 
limitation of Quick, is that it does not support IGameControlType::IGAME_TM or controllers that do not set keys.

\sa GMatrix
\sa IGameProperty
*/
class IGameControl: public MaxHeapOperators
{
	public:
        //! Various 3ds Max controller types
		enum MaxControlType{
			IGAME_UNKNOWN,					/*!<An unknown controller type*/
			IGAME_MAXSTD,					/*!<A Standard 3ds Max key frame controller*/
			IGAME_BIPED,					/*!<A Biped Controller*/
			IGAME_EULER,					/*!<An Euler Controller*/
			IGAME_ROT_CONSTRAINT,			/*!<A Rotation constraint*/
			IGAME_POS_CONSTRAINT,			/*!<A Position constraint*/
			IGAME_LINK_CONSTRAINT,			/*!<A Link Constraint*/
			IGAME_LIST,						/*!<A List Controller*/
			IGAME_INDEPENDENT_POS,			/*!<An Independent Position Controller*/
			IGAME_MASTER,                  /*!<Master Point Controller*/	

		};
		
	//! Euler Orderings
	/*! These are the rotation orders for an Euler Controller
	*/
		enum EulerOrder{
			XYZ,	/*!<XYZ Ordering*/
			XZY,	/*!<XZY Ordering*/
			YZX,	/*!<YZX Ordering*/
			YXZ,	/*!<YXZ Ordering*/
			ZXY,	/*!<ZXY Ordering*/
			ZYX,	/*!<ZYX Ordering*/
			XYX,	/*!<XYX Ordering*/
			YZY,	/*!<YZY Ordering*/
			ZXZ,	/*!<ZXZ Ordering*/
			BAD		/*!<If this is not a Euler Controller*/
		};
		
		//! \brief Destructor 
		virtual ~IGameControl() {;}

		//! Return the Bezier Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with the Key data.  The 
		keys should be accessed in Tab maintained by IGameKey::bezierKey.
		\param &gameKeyTab The Tab to receive the data
		\param Type The controller type (based on Transform style) to query.  This can be one of the following\n
		IGAME_POS\n
		IGAME_ROT\n
		IGAME_SCALE\n
		IGAME_FLOAT\n
		IGAME_POINT3\n
		IGAME_EULER_X\n
		IGAME_EULER_Y\n
		IGAME_EULER_Z\n
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetBezierKeys(IGameKeyTab &gameKeyTab,IGameControlType Type)=0;

		//! Return the Linear Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with data
		\param &gameKeyTab The tab to receive the data
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetLinearKeys(IGameKeyTab &gameKeyTab, IGameControlType Type)=0;

		//! Return the TCB Keys
		/*! IGameControl will check the appropriate control and fill the IGameKeyTab with data
		\param &gameKeyTab The tab to receive the data
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return TRUE is the controller was accessed successfully.
		*/
		virtual bool GetTCBKeys(IGameKeyTab &gameKeyTab, IGameControlType Type)=0;

		//! Return the Sampled Keys
		/*! IGameControl will sample the control based on the type supplied.  It will sample the node TM , float or point3 
		controllers.  The TM sample will be in the Coord System that defined when initialising IGame.  This method
		will sample the controller across the complete animation range.  The method of access can be provided (absolute and
		relative) It is important to read the 3ds Max sdk docs on Control::GetValue to understand the usage when used with non IGameControlType::IGAME_TM
		controllers.  IGame will still however fill out the respective structures even when the 3ds Max sdk docs mention Matrix3 access
		It is set to Relative as default, as this was the default for IGameControlType::IGAME_TM  usage before the method changed.  If you are sampling 
		anything else than IGameControlType::IGAME_TM this should be set to false to behave like the 3ds Max default and IGame before this change.
		\param &sample The tab to receive the data
		\param frameRate This is the number frames that the controller will be sampled at.  It will be converted to Ticks internally
		\param Type The controller type to query.  This can be any of the standard type but also include  IGAME_TM
		\param Relative This defines whether the controller is sampled for relative or absolute values.  It defaults to 
		relative true. Please read the max sdk section on Control::GetValue for details on the internal usage.
		\return TRUE if the controller was accessed successfully.
		*/
		virtual bool GetFullSampledKeys(IGameKeyTab &sample, int frameRate, IGameControlType Type, bool Relative = true) =0;

		//! Return the Sampled Keys 
		/*! IGameControl will sample the control based on the type supplied.  It will sample float or point3 
		controllers.  The TM sample will be in the Coord System that you defined when initialising IGame.  This 
		method only samples the controller where a key is Set, so it will not support the IGameControlType::IGAME_TM type.  If the 
		controller does not support setting of keys, it will return false.  This method will only sample the controller
		at times where keys exist.  This is useful to limit the data where controller can not be accessed directly
		\param &sample The tab to receive the data
		\param Type The controller type to query.  This can be any of the standard type but also include  IGAME_TM
		\return TRUE if the controller was accessed successfully.
		*/
		virtual bool GetQuickSampledKeys(IGameKeyTab &sample, IGameControlType Type) =0;


		//! Return an individual IGameKey
		/*! Fills out the supplied IGameKey with the bezier data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &bezKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetBezierIGameKey(IGameControlType Type, IGameKey &bezKey, int index) =0 ;

		//! Return an individual IGameKey
		/*! Fills out the supplied IGameKey with the TCB data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &tcbKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetTCBIGameKey(IGameControlType Type, IGameKey &tcbKey, int index)=0 ;

		//! Return an individual IGameKey
		/*! Fills out the supplied IGameKey with the Linear data for the key index supplied
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\param &linearKey
		\param index The key to retrieve
		\return TRUE if successful
		*/
		virtual bool GetLinearIGameKey(IGameControlType Type, IGameKey &linearKey, int index)=0;
		
		//! Get the total number of keys for this controller
		/*! This return the total number of keys for the controller supplied, <b>WARNING:  support leaf controllers only</b>
		\param Type The controller type to query.  See IGameControl::GetBezierKeys for more info
		\return The total number of keys
		*/
		virtual int GetIGameKeyCount(IGameControlType Type)=0;

		//!Get the controller type
		/*!Retrieves what type of IGame Controller it is (based on transformation style)
		\param Type The controller to query. See IGameControl::GetBezierKeys for more info
		\return The type of controller,  It can be one of the following\n
		IGAME_UNKNOWN\n
		IGAME_MAXSTD\n
		IGAME_BIPED\n
		IGAME_ROT_CONSTRAINT\n
		IGAME_POS_CONSTRAINT\n
		IGAME_LINK_CONSTRAINT\n
		IGAME_LIST\n	
		IGAME_INDEPENDENT_POS\n	
		IGAME_MASTER\n
		*/
		virtual MaxControlType GetControlType(IGameControlType Type)=0;

		//!Get the name of the controller class
		/*!Retrieves the controller class name based on the type
		\param Type The controller to query. See IGameControl::GetBezierKeys for more info
		\param &className The class name string
		*/
		virtual void GetClassName (IGameControlType Type, MSTR &className)=0;

		//!Access to actual animation
		/*!Indicates whether the IGame Controller actually has animation (based on transformation style)
		\param Type The controller to query. See IGameControl::GetBezierKeys for more info
		\return TRUE if controller actually has animation; otherwise FALSE\n
		*/ 
		virtual bool IsAnimated(IGameControlType Type)=0;

		//!Check for sub-controllers
		/*!Indicates whether the IGame Controller is a leaf (has no any sub-controllers or references) controller (based on transformation style)
		\param Type The controller to query. See IGameControl::GetBezierKeys for more info
		\return TRUE if controller has no any sub-controllers or references; otherwise FALSE\n
		*/ 
		virtual bool IsLeaf(IGameControlType Type)=0;

		//!Access to the Constraints
		/*! If a controller has a constraint system, then this will provide access to it
		\param Type The controller to check. This can be either of\n
		IGAME_POS\n
		IGAME_ROT\n
		IGAME_TM\n
		\return A Pointer to IGameConstraint, or NULL if not available
		*/
		virtual IGameConstraint * GetConstraint(IGameControlType Type)=0;
		
		//! Get the order of Rotation
		/*! This provides a way of determining the order of rotation for Euler controllers.  This is important
		so that the rotation can be rebuilt correctly on import.\n This data is also important when accessing the
		controller keys.  You still access the Euler data bsed on X,Y and Z - but you would use the ordering to
		work out the meaning of each controller.  So if EulerOrder was ZXZ, then controller access would mean
		x=z, y=x, z=z.
		\return The order of Rotation.  This can be a value from the EulerOrder
		*/
		virtual EulerOrder GetEulerOrder()=0;

		//!Get access to the actual max controller
		/*!
		\param Type This can be either of IGameControlType
		\return The 3ds Max controller
		*/
		virtual Control * GetMaxControl(IGameControlType Type)=0;

		//! Access the list controller
		/*! Access the n'th controller from the List controller.
		\param index The index into the list controller
		\param Type The Control type to access
		\return An IGameControl interface
		*/
		virtual IGameControl * GetListSubControl(int index, IGameControlType Type)=0;

		//! Get the number of controllers maintained by the list controller
		/*! The number of controllers maintained by the list controller for the Controller type being queried
		\param Type The controller to type
		\return The number of controllers in the list controller
		*/
		virtual int GetNumOfListSubControls(IGameControlType Type)=0;


		//! Get the number of subcontroller tracks maintained by the Master Point controller
		/*! The number of subcontroller tracks maintained by the Master Point controller. This is NOT actual number of
		subcontrollers, but total number of vertices
		\return The number of subcontroller tracks in the Master point controller, zero if no vertex animation
		*/
		virtual int GetNumOfVertControls()=0;

		//! Access the Master Point subcontroller
		/*! Access the n'th subcontroller from the Master Point controller.
		\param index The index into the Master Point controller, the actual vertex index
		\return An IGameControl interface, if vertex is not animated return NULL
		*/
		virtual IGameControl * GetVertexControl(int index)=0;

};

//! Simple wrapper class for constraints
/*! A unified wrapper around the various constraints that are available in 3ds Max.  There is access to the type of constraint
in use, plus easier access to the constraints.  If further access it needed, the IPropertyContainer interface can be used 
and additions made to property file used to access other data, as the source for these constraints is available in the SDK.
*/
class IGameConstraint : public IExportEntity
{
		

public:
	//! Various 3ds Max Constraints
	/*! These are the constraints supported by IGame
	*/
	enum ConstraintType{
		IGAME_PATH,			/*!<Path Constraint*/
		IGAME_ORIENTATION,	/*!<Orientation Constraint*/
		IGAME_LOOKAT,		/*!<look At Constraint*/
		IGAME_POSITION,		/*!<Position Constraint*/
		IGAME_LINK,			/*!<A TM link constraint*/
		IGAME_UNKNOWN,		/*!<Unknown Constraint*/
	};

	//!Number of constraining Node
	/*!The number of nodes in use by the Constraint system
	\return The number of nodes
	*/
	virtual int NumberOfConstraintNodes()=0;

	//!Get the constraint Node
	/*! The actual node of the index passed in that is working in the system
	\param index The index of the node to retrieve
	\return A pointer to IGameNode
	*/
	virtual IGameNode * GetConstraintNodes(int index)=0;

	//!Get the influence of the bone
	/*! This is the weight, or influence the specified node has in the constraint system.  The index used here is the same
	as the index used in IGameConstraint::GetConstraintNodes, otherwise the weights will not match.  This has no effect for a Link Constraint
	\param nodeIndex The node index to query
	\return The weight value
	*/
	virtual float GetConstraintWeight(int nodeIndex)=0;

	//!Get the start frame for the Link constraint
	/*!This specifies when the link for the n'th node will start.
	\param index The node index
	\return The start frame for the node queried.
	*/
	virtual int GetLinkConstBeginFrame(int index) = 0;

	//! Get the type of Constraint
	/*! This defines the actual constraint being used on the controller
	\return The type of max constraint.  It can be one ConstraintType enum
	*/
	virtual ConstraintType GetConstraintType()=0;
	

};

