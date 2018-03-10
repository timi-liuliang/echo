/**********************************************************************
 *<
	FILE: IIKSys.h

	DESCRIPTION:  Interfaces into IK sub-system classes/functions. 

	CREATED BY: Jianmin Zhao

	HISTORY: created 3 April 2000

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "iFnPub.h"
#include "INodeTab.h"

// Class ids
#define IKCONTROL_CLASS_ID Class_ID(0xe6f5815, 0x717f99a4)
#define IKCHAINCONTROL_CLASS_ID Class_ID(0x78724378, 0x8a4fd9)
#define GET_IKCHAIN_CD (GetCOREInterface()->GetDllDir().ClassDir().FindClass(CTRL_MATRIX3_CLASS_ID, IKCHAINCONTROL_CLASS_ID))

#define IK_FP_INTERFACE_ID Interface_ID(0x5b734601, 0x7c7c7ece)
#define IKCHAIN_FP_INTERFACE_ID Interface_ID(0x5a5e7cbe, 0x55367776)

#define GET_IK_OPS_INTERFACE ((IKCmdOps*)GetCOREInterface(IK_FP_INTERFACE_ID))
#define GET_IKCHAIN_FP_INTERFACE ((IKChainActions*)GET_IKCHAIN_CD->GetInterface(IKCHAIN_FP_INTERFACE_ID))

/*! \sa  : Class IKChainActions, Class IIKChainControl\n\n
\par Description:
<b>This class is only available in release 5 or later.</b>  The program
interface to functions of the IK system. To obtain a pointer to the interface,
use\n\n
 <b>IKCmdOps* iksys = GetCoreInterface(IK_FP_INTERFACE_ID);</b>\n\n
It should be non-null if the plugin of the IK system is loaded.\n\n
    */
class IKCmdOps : public FPStaticInterface {
public:
  /*! \remarks This method assigns an IK solver from nodes start to end. To be
  successful, following conditions have to be met: <ol> <li> Start must be an
  ancestor of end in the node hierarchy; </li> <li> All TM controllers along
  the chain must be replaceable; </li> <li> The 3rd argument, solver, must be a
  name of a plugin solver. </li> </ol>  \n\n
  Upon success, it will create an IK chain node that contains these pieces of
  information plus the IK goal and returns a pointer to it. Returning non-null
  pointer indicates failure.\n\n
  Example:\n\n
  	\code
	INode* ikchainNode = iksys->CreateIKChain(bone1, bone4, _M("HIIKSolver"));
	\endcode 
    */
  virtual INode* CreateIKChain(INode* start, INode* end, const MCHAR* solver) =0;
  /*! \remarks Returns the number of IK solvers that have been loaded in.\n\n
    */
  virtual int SolverCount() const =0;
  /*! \remarks Returns the internal name of the i-th solver. This name is used
  in <b>CreateIKChain()</b> or the script.\n\n
    */
  virtual MSTR SolverName(int) const =0;
  /*! \remarks Returns the UI name of the i-th solver. The UI names appear in
  the solver list on the Motion Panel, or the Animation menu. They are localized.
  */
  virtual MSTR SolverUIName(int) const =0;
};

// class IKCmdOpsEx
//! \brief This interface extends interface IKCmdOps by providing access to notification suspension for the IK system.

//! This class contains functionality for suspending and resuming IK system notification
//! and for inquiring its status.  If you have a large number of node parenting, you can
//! expect a performance gain if you suspend IK link notifications.  Be sure to match
//! any call to SuspendLinkNotify() with one to ResumeLinkNotify() since they are nestable.
//! You get this interface by calling GET_IKEX_OPS_INTERFACE
class IKCmdOpsEx : public IKCmdOps {
public:
  /*! \remarks Temporarily suspend the IK link notification mechanism.\n\n
    */
	virtual void SuspendLinkNotify() = 0;
  /*! \remarks Resume the IK link notification mechanism.\n\n
    */
	virtual void ResumeLinkNotify() = 0;
  /*! \remarks Returns the status of the IK link notification mechanism.\n\n
    */
	virtual bool IsLinkNotifySuspended() const = 0;
};

#define GET_IKEX_OPS_INTERFACE ((IKCmdOpsEx*)GetCOREInterface(IK_FP_INTERFACE_ID))

/*! \sa  : Class IKCmdOps, Class IIKChainControl\n\n
class IKChainActions : public FPStaticInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>  The program
interface to actions on IK chain nodes, which <b>IKCmdOps::CreateIKChain()</b>
returns. An macro is defined to obtain a pointer to the interface:\n\n
 <b>IKChainActions* ikchainAction = GET_IKCHAIN_FP_INTERFACE;</b>\n\n
It should be a valid pointer provided the IK system is successfully loaded.\n\n
The methods are mainly designed to connect to the UI and script. However, they
can be called programmatically as well. The action methods, those that return
<b>FPStatus</b>, assume that a unique IK chain node is currently selected and
apply action to it. Before an action method is called, <b>IsSnapEnabled()</b>
must be called to test whether a unique IK chain node is being selected. Such
as:\n\n
\code
if (ikchainAction->IsSnapEnabled())
	ikchainAction->SetPreferredAngles();
\endcode  
    */
class IKChainActions : public FPStaticInterface {
public:
  /*! \remarks It applies <b>IKSnapAction()</b> if IK Enabled is currently
  off, and <b>FKSnapAction()</b> if IK Enabled is on.\n\n
    */
  virtual FPStatus SnapAction() =0;
  /*! \remarks Applies IK Snapping, which sets IK goal and other parameters,
  such as the swivel angle, according to the current state of the bone chain.
  Moreover, it invokes <b>SetPreferredAngles().</b>\n\n
    */
  virtual FPStatus IKSnapAction() =0;
  /*! \remarks Applies FK Snapping, which assigns the currently active joint
  angles to the corresponding FK angles. Moreover, it invokes
  <b>SetPreferredAngles().</b>\n\n
    */
  virtual FPStatus FKSnapAction() =0;
  virtual BOOL IsSnapEnabled() =0;
  /*! \remarks Toggles IK Enabled state. If the AutoSnap parameter of the IK
  chain is true, it will invoke <b>SnapAction()</b> before toggling the Enabled
  state.\n\n
    */
  virtual FPStatus ToggleEnabled() =0;
  /*! \remarks It sets the preferred angles of all bones on the chain to the
  corresponding joint angles currently in force, which can be FK or IK
  depending the state of Enabled.\n\n
    */
  virtual FPStatus SetPreferredAngles() =0;
  /*! \remarks It assigns the preferred angles to the corresponding FK angles
  and then turn off IK Enabled. */
  virtual FPStatus AssumePreferredAngles() =0;
};

namespace IKSys {
  class ZeroPlaneMap;
}
//
// IIKChainControl
//
class IKSolver;
/*! \sa  : Class IKCmdOps, Class IKChainActions\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
The interface class to TM controller that the IK chain node employs. Suppose
node is an IK chain node, following code obtains the interface pointer:\n\n
\code
IIKChainControl* ikchain = (IIKChainControl*) node->GetTMController()->GetInterface(I_IKChainControl);
\endcode
From this pointer, we can get all properties pertinent to an IK Chain.\n\n
   */
class IIKChainControl: public MaxHeapOperators {
public:
  // Reference index:
  //
  enum {
	kPBlockRef = 0,	// ParamBlock
	kGoalTMRef,		// Matrix3 controller
	kEndJointRef,	// INode
	kEnableRef,		// Bool (float) controller
	kStartJointRef, // INode
	kOwningNodeRef, // Reference on a NodeMonitor that holds an indirect reference on the owning Node
	kLastRef
  };
  // Parameter block index:
  //
  enum {
	kParamBlock,
	kNumParamBlocks
  };
  // Paramter index of parameters in param-block of index kParamBlock:
  //
  enum {
	kStartJoint,	// INode, referenced by kStartJointRef
	kEndJoint,		// INode, referenced by kEndJointRef
	kSolverName,	// String
	kAutoEnable,	// BOOL
	kSwivel,		// Angle
	kPosThresh,		// Float
	kRotThresh,		// Float
	kIteration,		// Integer
	kEEDisplay,		// BOOL
	kEESize,		// Float
	kGoalDisplay,	// BOOL
	kGoalSize,		// Float
	kVHDisplay,		// BOOL
	kVHSize,		// Float
	kVHLength,		// Float
	kSolverDisplay,	// BOOL
	kAutoSnap,		// BOOL
	kVHUseTarget,	// BOOL
	kVHTarget,		// INode
	kSAParent,		// RadioBtn_Index
	kLastHIIKParam
  };

  /*! \defgroup splineIKParams Spline IK Parameters
  Provides the PB2 parameter IDs particular to the SplineIK solver.
   */
  //@{
  enum SplineIKParams {
	  // Paramter ids (of PB2) of Spline IK, i.e. that on which
	  // GoalInterfaceID() == IKSys::kSplineIKGoalID.
	  //
	  // Following parameter ids are inherited from the HIIK 
	  // kStartJoint,
	  // kEndJoint,
	  // kSolverName,
	  // kAutoEnable,
	  // kEEDisplay,
	  // kEESize,
	  // kGoalDisplay,
	  // kGoalSize,
	  // kSolverDisplay,
	  // kAutoSnap,
	  //
	  // Spline IK paramters start here:
	      /*! Associated with the "Pick Shape" button on the UI. This is also the "goal" for the
	      IK chain. */
	  kPickShape = kLastHIIKParam,	//!< Contains the Spline node which the IK chain follows. 
		  kTwistHStartDisplay,		//!< Controls display of the Start Twist Handle manipulator; default: OFF
		  kTwistHEndDisplay,		//!< Controls display of the EndTwist Handle manipulator; default: OFF
		  kTwistHStartSize,			//!< Spinner and editbox for the size of the Start Twist Handle manipulator; default size: 1.0
		  kTwistHEndSize,			//!< Spinner and editbox for the size of the End Twist Handle manipulator; default size: 1.0
		  kTwistHStartLength,		//!< Spinner and editbox for the length of the Start Twist Handle manipulator; default length: 75.0
		  kTwistHEndLength,			//!< Spinner and editbox for the length of the End Twist Handle manipulator; default length: 75.0
		  kTwistHStartAngle,		//!< Spinner and editbox for the Start Twist Handle manipulator angle; default angle: 0.0
		  kTwistHEndAngle,			//!< Spinner and editbox for the End Twist Handle manipulator angle; default angle: 0.0
//		  kTwistHStartTarget,
//		  kTwistHEndTarget,
//		  kTwistHUseStartTarget,
//		  kTwistHUseEndTarget,
		  
//parameters below are in the first pop-up dialog box "Spline IK SOlver"
		  /*! When the user checks "Auto Create Spline" checkbox, a spline with # of knot points
		  equal to the # of bones. The spline passes through all the bone pivot
		  points. If "Auto Create Spline" checkbox is unchecked, no spline is created. In this
		  case, to use splineIK, the user needs to open the UI (motion panel), click on "Pick
		  Shape" pushbutton and select an already existing spline. Then the user can assign the
		  SplineIKControl modifier or go to the sub-object level and animate the vertices of the
		  spline. */		  
		  kAutoSplineCreate,		//!< For the checkbox "Auto Create Spline" default: on
		  /*! Allows the user to use a Bezier Spline, NURBS Point curve or NURBS CV curve. */
		  kSplineTypeChoice,		//!< For the set of three radiobuttons called "Curve Type"; default: Bezier Spline
		  /*! The user can change the default number of knots during the creation process.
		  Available only for Bezier Spline curves. The default is equal to the number of joints in
		  the IKChain. Minimum is 2 knots. Internally a 'normalize spline' modifier is applied to
		  the spline. Note that as you decrease the knotCount, the resulting spline may have a
		  significantly different shape, and consequently the IKChain might move. */
		  kSplineKnotCount,			//!< For the checkbox "Number of Spline Knots"; Default: equal to chain DOF.
		  /*! If "Create Helpers" checkbox is checked, it is assumed that the user wants to
		  automatically assign the SplineIKControl modifier. So the modifier is assigned in
		  agreement with the helper properties specified in the pop-up. If the checkbox is
		  un-checked no modifier is assigned. */
		  kCreateHelper,			//!< For the checkbox "Create Helper"; default: ON
		  /*! If checked, attaches position constraint to the rootbone of the IKchain to constrain
		  it to the first helper. Note that the "position" of the rootbone is outside the scope of
		  the IK. In addition, makes all helpers children to Helper#1, i.e., knot#1. Helper#1 can
		  be position constrained or linked to another object. And, individual helpers can
		  be moved and rotated without any other helper being affect. If unchecked, all helpers
		  are independent -- not linked to any other helper -- so that they can be moved and
		  rotated without any other helper being affect. */
		  kLinktoRootNode,			//!< For the checkbox "Link to Root Node"; default: ON
		  kHelpersize,				//!< Display Option for Point Helper; default: 20.0
		  kHelperCentermarker,		//!< Display Option for Point Helper; default: OFF
		  kHelperAxisTripod,		//!< Display Option for Point Helper; default: OFF
		  kHelperCross,				//!< Display Option for Point Helper; default: OFF
		  kHelperBox,				//!< Display Option for Point Helper; default: ON
		  kHelperScreensize,		//!< Display Option for Point Helper;
		  kHelperDrawontop,			//!< Display Option for Point Helper; default: ON
		  kUpnode,					//!< Display Option for Point Helper;
	  	  kUseUpnode,				//!< Display Option for Point Helper;
	  kLastSplineIKParam			//!< 
  };
  //@}

  enum SAParentSpace {
	kSAInGoal,
	kSAInStartJoint
  };


  //Spline IK Specific methods.
  //CAUTION: The following methods, although meaningful only to 
  // SplineIK solver are also accessible to HIIK and IKLimb solvers. 
  // We have to implement them for these latter IKSolvers, with some
  // reasonable return values as return, so that they don't crash.

  virtual float		TwistHStartAngle(TimeValue, Interval&)=0;
  virtual float		TwistHEndAngle(TimeValue, Interval&)=0;

  // IK chain delimiters
  /*! \remarks The start joint, end joint of the chain, and the node to that
  this IK chain controller belongs.\n\n
    */
  virtual INode*		StartJoint() const =0;
  /*! \remarks Please refer to StartJoint().\n\n
    */
  virtual INode*		EndJoint() const =0;
  /*! \remarks Please refer to StartJoint().\n\n
    */
  virtual INode*		GetNode() const =0;

  // Vector Handle
  // InitPlane/InitEEAxis are the normal and End Effector Axis at the
  // initial pose. They are represented in the parent space.
  // ChainNormal is the InitPlane represented in the object space:
  // InitPlane() == ChainNormal * StartJoint()->InittialRotation()
  /*! \remarks Preferred angles are used to start the IK iteration. Hence, the
  pose when joint angles assume the preferred angles is also called the Initial
  Pose in the context of IK. It is animatable, meaning that at different time,
  IK may start iteration from different poses.\n\n
  Let's call the plane that the joints of an IK chain the (IK) solver plane.
  <b>InitPlane()</b> and <b>InitEEAxis()</b> returns the normal to the solver
  plane and the axis from the start joint to the end joint (end-effector axis)
  at the initial (preferred angle) pose, at time of input argument. They are
  represented in the parent space of the start joint. <b>ChainNormal()</b>
  returns the normal in the object space, as\n\n
   <b>InitPlane() == ChainNormal() * startIKCont-\>PrefRotation()</b>\n\n
  where\n\n
   <b>startIKCont =
  (IIKControl*)StartJoint()-\>GetController()-\>GetInterface(I_IKControl);</b>\n\n
  Furthermore, <b>ChainNormal()</b> takes as an input argument a validity
  interval that will be intersected by the validity interval of the chain
  normal.\n\n
    */
  virtual Point3		ChainNormal(TimeValue t, Interval& valid) =0;
  /*! \remarks Please refer to ChainNormal(). */
  virtual Point3		InitPlane(TimeValue t) =0;
  /*! \remarks Please refer to ChainNormal(). */
  virtual Point3		InitEEAxis(TimeValue t) =0;
  /*! \remarks Please refer to ChainNormal(). */
  virtual float			InitChainLength(TimeValue t) =0;
  /*! The swivel angle at time. The validity interval, valid, will be
  intersected.\n\n
    */
  virtual float			SwivelAngle(TimeValue, Interval&)=0;
  /*! The zero plane is the plane that, at each "start joint to end joint" axis,
  is used as a reference plane with regard to that the swivel angle is defined.
  The zero plane map maps an axis to a plane normal. The IK system offers as a
  possible default via <b>DefaultZeroPlaneMap()</b>.\n\n
    */
  virtual const IKSys::ZeroPlaneMap*
  						DefaultZeroPlaneMap(TimeValue t) =0;
  /*! Whether the zero plane is defined in the parent space of the start joint,
  <b>kSAInStartJoint</b>, or in the space of the IK goal, <b>kSAInGoal</b>.\n\n
    */
  virtual SAParentSpace SwivelAngleParent() const =0;
  
  // Solver
  /*! The solver that is assigned to this chain.\n\n
    */
  virtual IKSolver*		Solver() const =0;

  /*! Whether the solver is enabled (IK mode) or not (FK mode).\n\n
    */
  virtual bool			SolverEnabled(TimeValue, Interval* =0)=0;
  /*! When the chain is in the FK mode, IK can still be invoked when the goal
  is moved interactively if <b>CanAutoEnabled()</b> is true. It is a PB2
  parameter of index <b>kAutoEnable</b>. It is not animatable.\n\n
    */
  virtual bool			CanAutoEnable() const =0;
  /*! It is a transient state that is alive only at the time when joint angles
  are to be updated. Being true means that the Enabled state of the chain is
  off the IK is turned on by interactive manipulation.\n\n
    */
  virtual bool			AutoEnableSet() const =0;
  /*! Whether this chain is a valid one. It is valid if it is assigned a proper
  IK solver and it has valid start joint and end joint.\n\n
    */
  virtual bool			Valid() const =0;

  // Return HIIKGOAL_ID or SPLINEIKGOAL_IK
  /*! What IK goal interface this chain is prepaired for. For now, there are
  two interfaces: HI IK goal (<b>IHIIKGoal</b>) and Spline IK goal
  (<b>ISplineIKGoal</b>). An IK chain will admit of plugin solvers that support
  this goal interface (<b>IKSolver::ExpectGoal()</b>).\n\n
    */
  virtual Interface_ID	GoalInterfaceID() const =0;
  /*! It returns an interface to the goal at the time. Validity interval is
  reconciled. The parent matrix of the start joint is also returned in the
  third argument. */
  virtual BaseInterface* AcquireGoal(TimeValue, Interval&, const Matrix3& parent_of_start_joint) =0;
};

namespace IKSys {
  enum DofAxis {
	TransX = 0,	TransY,	TransZ,
	RotX, RotY,	RotZ,
	DofX = 0, DofY,	DofZ
  };
  enum JointType {
	SlidingJoint,
	RotationalJoint
  };
  struct DofSet : public MaxHeapOperators {
	DofSet() : bits(0) {}
	DofSet(const DofSet& src) : bits(src.bits) {}
	void Add(DofAxis dn) { bits |= (1 << dn); }
	void Clear(DofAxis dn) { bits &= ~(unsigned(1 << dn)); }
	int Include(DofAxis dn) const { return bits & (1 << dn); }
	int Count() const;
	unsigned bits;
  };
  inline int DofSet::Count() const
  {
   unsigned int ret;
   unsigned int b, i;
	for (b = bits, ret = 0, i = TransX; i <= RotZ; ++i) {
	  if (b == 0) break;
	  else if (b & 01u) {
		ret++;
	  }
	  b = b >> 1;
	}
	return ret;
  }
}; // namespace IKSys
//
// IIKControl
//
/*! \sa  Class INode,
Class Control, Class Point2,  Class Point3,  Class Interval, <a href="ms-its:3dsmaxsdk.chm::/ik_inverse_kinematics.html">Inverse Kinematics</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the TM controller for IK Chains and is used for nodes
that serve as IK joints. This class is closely knit together with the Class
<b>IIKChainControl</b> and form the IK system. An interface pointer to the
<b>IKControl</b> class can be obtained by using
<b>Animatable::GetInterface(I_IKCONTROL)</b>. See the Inverse Kinematics
section for more detailed information.  */
class IIKControl: public MaxHeapOperators {
public:
  typedef IKSys::DofAxis DofAxis;
  typedef IKSys::JointType JointType;
  typedef IKSys::DofSet DofSet;

  //Queries
  /*! \remarks This method allows you to determine which degrees of freedom
  are active and inactive for the translational and rotational axes.
  \par Parameters:
  <b>DofAxis</b>\n\n
  The DOF axis to check, which is one of the following <b>IKSys::TransX,
  IKSys::TransY, IKSys::TransZ, IKSys::RotX, IKSys::RotY, IKSys::RotZ</b>.
  \return  TRUE if the specified DOF axis is active, otherwise FALSE. */
  virtual bool		DofActive(DofAxis) const =0;
  /*! \remarks This method allows you to determine which degrees of freedom
  are active and inactive for the translational axes. The returned DofSet can
  be tested for <b>IKSys::DofX, IKSys::DofX,</b> and <b>IKSys::DofX</b>.
  \return  A DofSet structure containing the translational axes. */
  virtual DofSet	ActiveTrans() const =0;
  /*! \remarks This method allows you to determine which degrees of freedom
  are active and inactive for the rotational axes. The returned DofSet can be
  tested for <b>IKSys::DofX, IKSys::DofX,</b> and <b>IKSys::DofX</b>.
  \return  A DofSet structure containing the translational axes. */
  virtual DofSet	ActiveRot() const =0;
  /*! \remarks This method allows you to determine which degrees of freedom
  are active and inactive for the translational and rotational axes. The
  returned DofSet can be tested for <b>IKSys::TransX, IKSys::TransY,
  IKSys::TransZ, IKSys::RotX, IKSys::RotY, IKSys::RotZ</b>.
  \return  A DofSet structure containing the translational and rotational axes.
  */
  virtual DofSet	ActiveDofs() const =0;
  /*! \remarks This method returns a list of IK Chain nodes. Note that nn IK
  chain starts at the rotational joint of the Start Joint and ends at the
  sliding joint of the End Joint.
  \par Parameters:
  <b>JointType</b>\n\n
  The joint type, either <b>IKSys::SlidingJoint</b> or
  <b>IKSys::RotationalJoint</b>.
  \return  The node table of IK Chain nodes. */
  virtual INodeTab	IKChains(JointType) const =0;
  /*! \remarks This method allows you to check if a specific DOF axis has its
  lower bounds limited.
  \par Parameters:
  <b>DofAxis</b>\n\n
  The DOF axis to check, which is one of the following <b>IKSys::TransX,
  IKSys::TransY, IKSys::TransZ, IKSys::RotX, IKSys::RotY, IKSys::RotZ</b>.
  \return  TRUE if limited, otherwise FALSE. */
  virtual bool		DofLowerLimited(DofAxis) const =0;
  /*! \remarks This method allows you to check if a specific DOF axis has its
  upper bounds limited.
  \par Parameters:
  <b>DofAxis</b>\n\n
  The DOF axis to check, which is one of the following <b>IKSys::TransX,
  IKSys::TransY, IKSys::TransZ, IKSys::RotX, IKSys::RotY, IKSys::RotZ</b>.
  \return  TRUE if limited, otherwise FALSE. */
  virtual bool		DofUpperLimited(DofAxis) const =0;
  /*! \remarks This method allows you to check if a specific DOF axis has its
  upper and lower bounds limited.
  \par Parameters:
  <b>DofAxis</b>\n\n
  The DOF axis to check, which is one of the following <b>IKSys::TransX,
  IKSys::TransY, IKSys::TransZ, IKSys::RotX, IKSys::RotY, IKSys::RotZ</b>.
  \return  A Point2 where X and Y are the lower and upper limits, respectively.
  */
  virtual Point2	DofLimits(DofAxis) const =0;
  /*! \remarks This method allows you to retrieve the translation lower
  limits.
  \return  A Point3 where X, Y, and Z represents the actual limits. */
  virtual Point3	TransLowerLimits() const =0;
  /*! \remarks This method allows you to retrieve the translation upper
  limits.
  \return  A Point3 where X, Y, and Z represents the actual limits. */
  virtual Point3	TransUpperLimits() const =0;
  /*! \remarks This method allows you to retrieve the rotational lower limits.
  \return  A Point3 where X, Y, and Z represents the actual limits. */
  virtual Point3	RotLowerLimits() const =0;
  /*! \remarks This method allows you to retrieve the rotational upper limits.
  \return  A Point3 where X, Y, and Z represents the actual limits. */
  virtual Point3	RotUpperLimits() const =0;
  /*! \remarks The parameter that decides whether an individual degree of
  freedom is active is not animatable. There is an animatable variable of IK
  chain that decides whether the goal defined in the IK chain actually affects
  the joints it covers at a specific time. This method allows you to query
  that.
  \par Parameters:
  <b>TimeValue t</b>\n\n
  The time at which to test the joint type.\n\n
  <b>JointType jt</b>\n\n
  The joint type, either <b>IKSys::SlidingJoint</b> or
  <b>IKSys::RotationalJoint</b>.
  \return  TRUE if bound, otherwise FALSE. */
  virtual bool		IKBound(TimeValue t, JointType jt)=0;
  /*! \remarks This method allows you to obtain a pointer to the Forward
  Kinematics sub-controller. Note that the IK controller is not designed to be
  instanced. It is expected to have a unique node. */
  virtual Control*	FKSubController() const =0;
  /*! \remarks This method allows you to obtain a pointer to the node that
  holds the Forward Kinematics TM controller. */
  virtual INode*	GetNode() const =0;
  /*! \remarks This method allows you to retrieve the preferred angle of
  translation. Note that the angles are constant with regard to animation time.
  \par Parameters:
  <b>TimeValue t</b>\n\n
  The time at which to retrieve the preferred angle.\n\n
  <b>Interval\& validityInterval</b>\n\n
  The validity interval.
  \return  The X, Y, and Z, preferred angles. */
  virtual Point3	PrefPosition(TimeValue t, Interval& validityInterval) =0;
  /*! \remarks This method allows you to retrieve the preferred angle of
  rotation. Note that the angles are constant with regard to animation time.
  \par Parameters:
  <b>TimeValue t</b>\n\n
  The time at which to retrieve the preferred angle.\n\n
  <b>Interval\& validityInterval</b>\n\n
  The validity interval.
  \return  The X, Y, and Z, preferred angles. */
  virtual Point3	PrefRotation(TimeValue t, Interval& validityInterval) =0;

  // DOF values
  virtual Point3	TransValues(TimeValue, Interval* =0)=0;
  virtual Point3	RotValues(TimeValue, Interval* =0)=0;
  /*! \remarks This method allows you to set the angles of translational
  joints. Note that this method does not adjust the validity interval.
  \par Parameters:
  <b>const Point3\&</b>\n\n
  The joint angles.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignTrans(const Point3&, const Interval&)=0;
  /*! \remarks This method allows you to set the angles of rotational joints.
  Note that this method does not adjust the validity interval.
  \par Parameters:
  <b>const Point3\&</b>\n\n
  The joint angles.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignRot(const Point3&, const Interval&)=0;
  /*! \remarks This method allows you to set the angles of translational
  joints. Note that this method will skip those degrees of freedom that are not
  active and that this method will not adjust the validity interval.
  \par Parameters:
  <b>const Point3\&</b>\n\n
  The joint angles.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignActiveTrans(const Point3&, const Interval&)=0;
  /*! \remarks This method allows you to set the angles of rotational joints.
  Note that this method will skip those degrees of freedom that are not active
  and that this method does not adjust the validity interval.
  \par Parameters:
  <b>const Point3\&</b>\n\n
  The joint angles.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignActiveRot(const Point3&, const Interval&)=0;
  /*! \remarks This method allows you to set the angles of translational
  joints. The active DOF's are given as the first argument of type DofSet and
  the new values are supplied as a float array whose size should be the same as
  the DofSet [DofSet::Count()].Note that this method does not adjust the
  validity interval.
  \par Parameters:
  <b>const DofSet\&</b>\n\n
  The degrees of freedom.\n\n
  <b>const float[]</b>\n\n
  The new angles you wish to set.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignActiveTrans(const DofSet&, const float[],
									  const Interval&)=0;
  /*! \remarks This method allows you to set the angles of rotational joints.
  The active DOF's are given as the first argument of type DofSet and the new
  values are supplied as a float array whose size should be the same as the
  DofSet [DofSet::Count()].Note that this method does not adjust the validity
  interval.
  \par Parameters:
  <b>const DofSet\&</b>\n\n
  The degrees of freedom.\n\n
  <b>const float[]</b>\n\n
  The new angles you wish to set.\n\n
  <b>const Interval\&</b>\n\n
  The validity interval */
  virtual void		AssignActiveRot(const DofSet&, const float[],
									const Interval&)=0;
  /*! \remarks This method allows you to set the validity interval for
  translational joint angles.
  \par Parameters:
  <b>const Interval\& valid</b>\n\n
  The validity interval. */
  virtual void		SetTransValid(const Interval& valid) =0;
  /*! \remarks This method allows you to set the validity interval for
  rotational joint angles.
  \par Parameters:
  <b>const Interval\& valid</b>\n\n
  The validity interval. */
  virtual void		SetRotValid(const Interval& valid) =0;
  /*! \remarks This method allows you to set the validity interval for both
  translational and rotational joint angles.
  \par Parameters:
  <b>const Interval\& valid</b>\n\n
  The validity interval. */
  virtual void		SetTRValid(const Interval& valid) =0;
  /*! \remarks A solver may start off the solution process with joint angles
  being set to special values, preferred angles. This method allows you to set
  the preferred angles of the translational joints.
  \par Parameters:
  <b>const Point3\& val</b>\n\n
  The preferred angles you wish to set.\n\n
  <b>TimeValue t = 0</b>\n\n
  The time at which to set them. */
  virtual void		SetPrefTrans(const Point3& val, TimeValue t =0) =0;
  /*! \remarks A solver may start off the solution process with joint angles
  being set to special values, preferred angles. This method allows you to set
  the preferred angles of the rotational joints.
  \par Parameters:
  <b>const Point3\& val</b>\n\n
  The preferred angles you wish to set.\n\n
  <b>TimeValue t = 0</b>\n\n
  The time at which to set them. */
  virtual void		SetPrefRot(const Point3& val, TimeValue t =0) =0;
  /*! \remarks A solver may start off the solution process with joint angles
  being set to special values, preferred angles. This method allows you to set
  the preferred angles of both the translational and rotational joints.
  \par Parameters:
  <b>const Point3\& trans</b>\n\n
  The preferred translational angles you wish to set.\n\n
  <b>const Point3\& rot</b>\n\n
  The preferred rotational angles you wish to set.\n\n
  <b>TimeValue t = 0</b>\n\n
  The time at which to set them. */
  virtual void		SetPrefTR(const Point3& trans, const Point3& rot,
							  TimeValue t =0) =0;
};
