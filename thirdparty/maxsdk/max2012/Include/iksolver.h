/**********************************************************************
*<
FILE: IKSolver.h

DESCRIPTION:  IK Solver Class definition

CREATED BY: Jianmin Zhao

HISTORY: created 16 March 2000

*>	Copyright (c) 1994, All Rights Reserved.
**********************************************************************/
#pragma once

#include "plugapi.h"
#include "IKHierarchy.h"
// forward declarations

/*! \sa  Class BaseInterfaceServer,  <a href="class_i_k_sys_1_1_zero_plane_map.html">Class ZeroPlaneMap</a>, 
<a href="class_i_k_sys_1_1_link_chain.html">Class LinkChain</a>,
<a href="ms-its:3dsmaxsdk.chm::/ik_inverse_kinematics.html">Inverse Kinematics</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the base class that IK Solver plugins should derive
from.\n\n
The IK solver is a pure mathematical function and does not hold state, but
instead just solves a given, self-contained, mathematical problem, e.g. the
plugin solver does not have influence on when IK is invoked and what an IK
problem is (what is the goal and what are the joints, etc.), but contributes to
IK by providing an answer on how to solve. Structurally, it is independent of
the SDK and, hence, can be built independently, except for some theoretically
independent math library. See the Inverse Kinematics section for more detailed
information.  */
class IKSolver  : public BaseInterfaceServer {
public:
	typedef unsigned	ReturnCondition;
	enum ConditionBit {
		bLimitReached =			0x00000001,
		bLimitClamped =			0x00000002,
		bMaxIterationReached =	0x00000004,
		// The first eight bits are reserved for mild condition.
		// They are still considered successful.
		bGoalTooCloseToEE =		0x00000100,
		bInvalidArgument =		0x00000200,
		bInvalidInitialValue =	0x00000400
	};
	// Plugins derived from this class are supposed to have this super class id.
	/*! \remarks Plugins derived from this class are supposed to have
	<b>IK_SOLVER_CLASS_ID</b>as their super class ID. This method should not be
	overridden.
	\par Default Implementation:
	<b>{ return IK_SOLVER_CLASS_ID; }</b> */
	virtual SClass_ID SuperClassID() {return IK_SOLVER_CLASS_ID;}
	/*! \remarks Returns the class ID of the IK Solver plugin. */
	virtual Class_ID ClassID() =0;
	/*! \remarks This method returns the class name of the IK Solver plugin. This name will
	appear in the solver list from which users can pick or assign IK chains.
	\par Parameters:
	<b>MSTR\& s</b>\n\n
	The class name string.
	\par Default Implementation:
	<b>{ s = MSTR(_M("IKSolver")); }</b> */
	virtual void GetClassName(MSTR& s) { s= MSTR(_M("IKSolver")); }  
	/*! \remarks Destructor. */
	virtual ~IKSolver(){}

	// History independent solver does need time input.
	/*! \remarks At a specific point in time, the history dependent solver will reach
	solutions not only based the state of the goal at the time, but also its
	previous states (i.e. history dependent). On the contrary, the history
	independent solver does its job based on the state of the goal just at the
	time. The procedural implication is that, when the goal is changed at time t,
	the IK system would have to invalidate joints at time t for the history
	independent solver, and at all times that are greater or equal to t for the
	history dependent solver. In R4, only history dependent solvers are used by
	the IK system.
	\return  TRUE if the IK Solver is history dependent, otherwise FALSE. */
	virtual bool		IsHistoryDependent() const =0;
	/*! \remarks When two IK chains overlap, i.e., there is a joint belonging to both IK
	chains, some solvers are able to negotiate between the possibly contending
	goals and some are not. This method indicates if the IK Solver does a single
	chain only. For those IK Solvers that can only solve one chain at a time, the
	IK system will pass to the solvers one chain at a time in a definitive order.
	In R4, only solvers that "do one chain only" are used.
	\return  TRUE if the IK Solver does only one chain, otherwise FALSE. */
	virtual bool		DoesOneChainOnly() const =0;
	// Interactive solver does need initial pose. It needs current pose.
	/*! \remarks This method indicates whether the IK Solver is a controller or an interactive
	manipulation tool. In the former, the relationship between the goal and the
	joints are permanent: joints are completely controlled by the goal. In the
	latter, the relationship is transient, existing only during interactive
	manipulation. In the end, IK solutions are registered at each joint, mostly
	likely as key-frames, and it no longer matters how joints have got their
	joint angles. Only non-interactive, or controller, IK solvers are supported
	in R4. Note that Interactive solvers do not need an initial pose, instead it
	needs a current pose.
	\return  TRUE if the IK Solver is an interactive tool, otherwise FALSE. */
	virtual bool		IsInteractive() const =0;
	/*! \remarks This method indicates whether the IK Solver intends to use the sliding joint
	(translational degrees of freedom) of the IK chain.
	\return  TRUE if the sliding joint of the IK chain is used, otherwise FALSE.
	*/
	virtual bool		UseSlidingJoint() const =0;
	/*! \remarks This method indicates whether the IK Solver intends to use the swivel angle
	parameter of the IK chain.
	\return  TRUE if the swivel angle of the IK chain is used, otherwise FALSE.
	*/
	virtual bool		UseSwivelAngle() const =0;
	// Solutions of an analytic solver is not dependent on Pos/Rot threshold
	// or MaxInteration number.
	/*! \remarks This method determines whether the IK Solver is analytic or needs to go
	through iterations. Solutions of an analytic IK Solver are not dependent on
	position and rotation thresholds or a maximum number of iterations.
	\return  TRUE if the IK Solver is analytic, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false; }</b> */
	virtual bool		IsAnalytic() const { return false; }
	// The result will be simply clamped into joint limits by the ik system
	// if the solver does not do joint limits.
	/*! \remarks This method determines whether the IK Solver handles root joint limits. If
	the IK Solver does not do joint limits, the result will be simply clamped
	into joint limits by the IK system.
	\return  TRUE if the IK Solver does root joint limits, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false;}</b> */
	virtual bool		DoesRootJointLimits() const { return false;}
	/*! \remarks This method determines whether the IK Solver handles joint limits. If the IK
	Solver does not do joint limits, the result will be simply clamped into joint
	limits by the IK system.
	\return  TRUE if the IK Solver does joint limits, otherwise FALSE.
	\par Default Implementation:
	<b>{ return false;}</b> */
	virtual bool		DoesJointLimitsButRoot() const { return false;}

	// The interface ID that this solver is equipped to solve.
	virtual Interface_ID ExpectGoal() const =0;
	// RotThreshold() is not relevant to solver that does not SolveEERotation().
	// UseSwivelAngle() and SolveEERotation() cannot both be true.
	/*! \remarks This method determines whether the rotational part of the goal node will be
	used.
	\return  TRUE if the rotational part of the goal node will be used, otherwise
	FALSE to indicate that only the position of the goal node is taken as the IK
	goal while the rotation threshold will be irrelevant. */
	virtual bool		SolveEERotation() const =0;
	// A solver may have its own zero map. If so, the IK system may want
	// to know through this method.
	/*! \remarks IK Solvers may have their own Zero Plane Map. If so, they must override this
	method. The IK system will need it to perform IK snapping, which is setting
	the swivel angle based on the current pose so that the pose is consistent
	with the swivel angle. A Zero-Plane map can depend on the initial pose, which
	is when the joint angles take into account the respective preferred angles.
	In this method, <b>a0</b> is to be substituted for by the end effector axis,
	which is a unit vector, and <b>n0</b> by the solver plane normal, also a unit
	vector, when the chain is at the initial pose. The IK system will call this
	function using <b>IIKChainControl::InitEEAxis()</b> and
	<b>IIKChainControl::InitPlane()</b> for the two arguments.
	\par Parameters:
	<b>const Point3\& a0</b>\n\n
	The end effector axis unit vector.\n\n
	<b>const Point3\& n0</b>\n\n
	The solver plane normal.
	\return  A pointer to the ZeroPlaneMap.
	\par Default Implementation:
	<b>{ return NULL; }</b> */
#pragma warning(push)
#pragma warning(disable:4100)
	virtual const IKSys::ZeroPlaneMap* GetZeroPlaneMap(const Point3& a0, const Point3& n0) const { return NULL; }
#pragma warning(pop)
	/*! \remarks This method allows you to retrieve the position threshold. */
	virtual float		GetPosThreshold() const =0;
	/*! \remarks This method allows you to retrieve the rotation threshold. */
	virtual float		GetRotThreshold() const =0;
	/*! \remarks This method allows you to retrieve the maximum number of
	iterations. */
	virtual unsigned	GetMaxIteration() const =0;
	/*! \remarks This method allows you to set the position threshold.
	\par Parameters:
	<b>float</b>\n\n
	The position threshold value. */
	virtual void		SetPosThreshold(float) =0;
	/*! \remarks This method allows you to set the rotation threshold.
	\par Parameters:
	<b>float</b>\n\n
	The rotation threshold value. */
	virtual void		SetRotThreshold(float) =0;
	/*! \remarks This method allows you to set the maximum number of iterations.
	\par Parameters:
	<b>unsigned</b>\n\n
	The maximum number of iterations. */
	virtual void		SetMaxIteration(unsigned) =0;
	// The derived class should override this method if it answers true
	// to DoesOneChainOnly() and false to HistoryDependent().
	// The solver is not designed to be invoked recursively. The
	// recursion logic existing among the ik chains is taken care of by
	// the Max IK (sub-)System.
	/*! \remarks This is the method that the IK system will call when it's the time to update
	the joints according to the IK goal and other parameters. The derived class
	should override this method if <b>DoesOneChainOnly()</b> returns TRUE and
	<b>HistoryDependent()</b> returns FALSE. Note that the solver is not designed
	to be invoked recursively. The recursion logic existing among the IK chains
	is taken care of by the 3ds Max IK (sub-)System. The data structure passed to
	the Solver is transient and thus will be discarded once the solution is
	copied back to the joints. If the return condition indicates failure, (i.e.
	\> 0xff) the result will not be copied back to the joint nodes in the 3ds Max
	scene database.
	\par Parameters:
	<b>IKSys::LinkChain\&</b>\n\n
	A reference to the Link Chain.
	\return  The ReturnCondition bit-set with one or more of the following
	flags;\n\n
	<b>bLimitReached</b>\n\n
	The limit is reached.\n\n
	<b>bLimitClamped</b>\n\n
	The limit is clamped.\n\n
	<b>bMaxIterationReached</b>\n\n
	The maximum number of iterations is reached.\n\n
	<b>bGoalTooCloseToEE</b>\n\n
	The goal is too close to the end effector.\n\n
	<b>bInvalidArgument</b>\n\n
	An invalid argument is passed.\n\n
	<b>bInvalidInitialValue</b>\n\n
	An invalid initial value is passed. */
	virtual ReturnCondition Solve(IKSys::LinkChain&) =0;
};
