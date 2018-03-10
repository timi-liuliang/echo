/**********************************************************************
 *<
	FILE: IKHierarchy.h

	DESCRIPTION:  Geometrical representation of the ik problem. Note that
				  this file should not dependent on Max SDK, except for
				  some math classes, such as Matrix3, Point3, etc.

	CREATED BY: Jianmin Zhao

	HISTORY: created 16 March 2000

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "maxheap.h"
#include "coreexp.h"
#include "point2.h"
#include "point3.h"
#include "maxtypes.h"
#include "matrix3.h"

// forward declarations
class IIKChainControl;

namespace IKSys
{
//--------------
/*! \sa  Class Point3, <a href="ms-its:3dsmaxsdk.chm::/ik_inverse_kinematics.html">Inverse Kinematics</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class is defined in <b>IKHierarchy.h</b> and provides the functionality
that, given a unit axis, which is to be substituted by the end effector (EE)
Axis, produces a unit vector, which will be interpreted as the normal to a
plane. For more details see the section on Inverse Kinematics.  */
  class ZeroPlaneMap : public MaxHeapOperators {
  public:
	/*! \remarks The ZeroPlaneMap call operator.
	\par Parameters:
	<b>const Point3\& EEAxis</b>\n\n
	The end effector axis.
	\return  The unit vector described above. */
	virtual Point3 operator()(const Point3& EEAxis) const =0;
	/*! \remarks Destructor.
	\par Default Implementation:
	<b>{ }</b> */
	virtual ~ZeroPlaneMap() {}
  };

const Interface_ID kGoalID(0x53937e2, 0x2be92941);
const Interface_ID kHIIKGoalID(0x2497c0e, 0x376f602a);
const Interface_ID kSplineIKGoalID(0x4ee7cd9, 0x68a54886);

//! Id for ISplineIKGoal2 interface
const Interface_ID IID_SPLINE_IKGOAL2(0x163a2f78, 0xb3027de);

enum SAParentSpace {
	kSAInGoal,
	kSAInStartJoint,
	kSANotApplicable = 1000
};

// A LinkChain consists of a RootLink and a number of Links.
// A RootLink consists of a rotation plus a rigidExtend. It transforms
// like this:
//	To_Coordinate_Frame = rigidExtend * rotXYZ * From_Coordinate_Frame.
// where rotXYZ = Rot_x(rotXYZ[0]) * Rot_y(rotXYZ[1]) * Rot_z(rotXYZ[2]).
// 
// * Note that not all the x, y, and z, are degrees of freedom. Only
// Active() ones are. We put the whole rotation here so that some
// solver may choose to use it as a full rotation and then clamp the
// result to the permissible range.
// 
// * LinkMatrix(bool include_rot) returns rigidExtend if include_rot is
// false and returns the whole matrix from the From_Coordinate_Fram to
// To_Coordinate_Frame, i.e., rigidExtend*rotXYZ.rotXYZ are not all degrees of freedom. Only the active ones are. 
//
// * Matrix3& ApplyLinkMatrix(Matrix3& mat, bool) applies the LinkMatrix() to
// the input matrix from the left, i.e., mat = LinkMatrix(bool)*mat,
// and returns the reference to the input matrix.
//
// * Matrix3& RotateByAxis(Matrix3&, unsigned i) pre-applies the
// rotation about x, y, or z (corresponding to i=0,1,or 2).
// Therefore, starting with the identity matrix, mat,
//	ApplyLinkMatrix(
//		RotateByAxis(
//			RotateByAxis(
//				RotateByAxis(mat, 2),
//				1),
//			0),
//		false)
//  should equal to LinkMatrix(true).
//
/*! \sa  <a href="class_i_k_sys_1_1_link_chain.html">Class LinkChain</a>,
<a href="class_i_k_sys_1_1_link.html">Class Link</a>,
Class Matrix3, Class Point3, <a href="ms-its:3dsmaxsdk.chm::/ik_inverse_kinematics.html">Inverse Kinematics</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
A RootLink consists of a rotation plus a rigidExtend. It transforms like
this:\n\n
<b>To_Coordinate_Frame = rigidExtend * rotXYZ * From_Coordinate_Frame</b>\n\n
where <b>rotXYZ = Rot_x(rotXYZ[0]) * Rot_y(rotXYZ[1]) *
Rot_z(rotXYZ[2])</b>.\n\n
Note that not all the x, y, and z, are degrees of freedom. Only Active() ones
are. We put the whole rotation here so that some solver may choose to use it as
a full rotation and then clamp the result to the permissible range.
\par Data Members:
<b>Point3 rotXYZ;</b>\n\n
The xyz rotation.\n\n
<b>Point3 initXYZ;</b>\n\n
Corresponds to the <b>PrefRotation()</b> of the Start Joint.\n\n
<b>Point3 llimits;</b>\n\n
The lower limits.\n\n
<b>Point3 ulimits;</b>\n\n
The upper limits.\n\n
<b>Matrix3 rigidExtend;</b>\n\n
The rigid extents.\n\n
private:\n\n
<b>unsigned flags;</b>\n\n
The root link flags.  */
  class RootLink : public MaxHeapOperators {
  public:
	  /*! \remarks Constructor.
	  \par Default Implementation:
	  <b>{ }</b> */
	RootLink():flags(7){} // x,y,z, are all active. No joint limits.
	Point3		rotXYZ;
	Point3		initXYZ;
	Point3		llimits;
	Point3		ulimits;
	Matrix3		rigidExtend;
	/*! \remarks This method informs the Solver whether a particular component
	is active.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.
	\return  TRUE if the specified component is active, otherwise FALSE.
	\par Default Implementation:
	<b>{ return flags\&(1\<\<i)?true:false; }</b> */
	bool		GetActive(unsigned i) const { return flags&(1<<i)?true:false;}
	/*! \remarks This method informs the Solver whether a particular component
	has its lower limits active.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.
	\return  TRUE if the specified component is active, otherwise FALSE.
	\par Default Implementation:
	<b>{ return flags\&(1\<\<(i+3))?true:false; }</b> */
	bool		GetLLimited(unsigned i) const { return flags&(1<<(i+3))?true:false;}
	/*! \remarks This method informs the Solver whether a particular component
	has its upper limits active.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.
	\return  TRUE if the specified component is active, otherwise FALSE.
	\par Default Implementation:
	<b>{ return flags\&(1\<\<(i+6))?true:false; }</b> */
	bool		GetULimited(unsigned i) const { return flags&(1<<(i+6))?true:false;}
	
	/*! \remarks This method will pre-apply the rotation about the x, y, or z
	axis. Therefore, starting with the identity matrix <b>mat</b>,\n\n
	<b>ApplyLinkMatrix(</b>\n\n
	<b>RotateByAxis(</b>\n\n
	<b>RotateByAxis(</b>\n\n
	<b>RotateByAxis(mat, 2),</b>\n\n
	<b>1),</b>\n\n
	<b>0),</b>\n\n
	<b>false)</b>\n\n
	should equal to <b>LinkMatrix(true)</b>.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively. */
	CoreExport Matrix3&	RotateByAxis(Matrix3& mat, unsigned i) const;
	/*! \remarks This method returns the link matrix just defined if the
	argument is TRUE.
	\par Parameters:
	<b>bool include_rot</b>\n\n
	TRUE to return the link matrix, FALSE to return <b>RigidExtend</b>.
	\return  The link matrix, otherwise it simply returns <b>RigidExtend</b>.
	*/
	CoreExport Matrix3	LinkMatrix(bool include_rot) const;
	/*! \remarks This methods applies the LinkMatrix() to the input matrix, or
	<b>mat = mat * LinkMatrix(include_rot)</b>
	\par Parameters:
	<b>Matrix3\& mat</b>\n\n
	The input matrix.\n\n
	<b>bool include_rot</b>\n\n
	When applying the DOF part, or rotation part, to a matrix, this will take
	place one at a time by calling <b>RootLink::RotateByAxis()</b>. If you want
	to apply the whole link, while already having applied the rotation part,
	you would need to set this flag to FALSE.
	\return  The reference to the input matrix, <b>mat</b>. */
	CoreExport Matrix3&	ApplyLinkMatrix(Matrix3& mat, bool include_rot) const;
	// Set methods:
	//
	/*! \remarks This method allows you to activate or deactivate a particular
	component.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.\n\n
	<b>bool s</b>\n\n
	TRUE to activate, FALSE to deactivate. */
	CoreExport void	SetActive(unsigned i, bool s);
	/*! \remarks This method allows you to activate or deactivate a particular
	component's lower limits.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.\n\n
	<b>bool s</b>\n\n
	TRUE to activate, FALSE to deactivate. */
	CoreExport void	SetLLimited(unsigned i, bool s);
	/*! \remarks This method allows you to activate or deactivate a particular
	component's upper limits.
	\par Parameters:
	<b>unsigned i</b>\n\n
	The component, 0 through 3 for x, y, and z, respectively.\n\n
	<b>bool s</b>\n\n
	TRUE to activate, FALSE to deactivate. */
	CoreExport void	SetULimited(unsigned i, bool s);
  private:
	unsigned	flags;
  };

// A Link is a 1-dof rotation followed by a rigidExtend. The dof
// axis is specified by dofAxis. It is always active.
// 
// * LinkMatrix(true) == rigidExtend * Rotation(dofAxis, dofValue).
//   LinkMatrix(false) == rigidExtend.
//
// * Matrix3& ApplyLinkMatrix(Matrix3& mat, bool) pre-applies the
// LinkMatrix(bool) to the input matrix, mat.
//
// * A typical 3-dof (xyz) joint is decomposed into three links. z and
// y dofs don't have rigid extension, called NullLink(). Let's use
//		++o
// to denote NullLink() and
//		---o
// to denote !NullLink(). Then, a 3-dof joint will be decomposed into
// three Links, as:
//		---o++o++o
//         x  y  z
//
// * For an xyz rotation joint, if y is not active (Active unchecked),
// then y will be absorbed into the z-link, as:
//		---o---o
//         x   z
// In this case, the z-link is not NullLink(). But its length is
// zero. It is called ZeroLengh() link.
//
/*! \sa  <a href="class_i_k_sys_1_1_link_chain.html">Class LinkChain</a>,
<a href="class_i_k_sys_1_1_root_link.html">Class RootLink</a>,
Class Matrix3, Class Point3, <a href="ms-its:3dsmaxsdk.chm::/ik_inverse_kinematics.html">Inverse Kinematics</a>\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents a single link in the link chain. A Link is a single
degree of freedom rotation followed by a rigidExtend. The DOF axis is specified
by dofAxis. It is always active.
\par Data Members:
<b>DofAxis dofAxis;</b>\n\n
The variable part of a Link is of one degree of freedom. It can be
translational or rotational. One of the following; <b>TransX, TransY, TransZ,
RotX, RotY</b>, or <b>RotZ</b>.\n\n
<b>float dofValue;</b>\n\n
The current value with regard to the degree of freedom.\n\n
<b>float initValue;</b>\n\n
The initial value.\n\n
<b>Point2 limits;</b>\n\n
The constrained lower and upper limits. [0] for the lower limit and [1] for the
upper limit.\n\n
private:\n\n
<b>Matrix3 rigidExtend;</b>\n\n
The rigid extents.\n\n
<b>byte llimited : 1;</b>\n\n
Lower limit flag.\n\n
<b>byte ulimited : 1;</b>\n\n
Upper limit flag.  */
  class Link : public MaxHeapOperators {
  public:
	  /*! \remarks Constructor.
	  \par Default Implementation:
	  <b>{ }</b> */
	Link():rigidExtend(0),dofAxis(RotZ){}
	/*! \remarks Destructor.
	\par Default Implementation:
	<b>{ if (rigidExtend) delete rigidExtend; rigidExtend = 0; }</b> */
	CoreExport ~Link();
	enum DofAxis {
	  TransX,
	  TransY,
	  TransZ,
	  RotX,
	  RotY,
	  RotZ
	};
	DofAxis		dofAxis;
	float		dofValue;
	float		initValue;
	Point2		limits;
	/*! \remarks This method checks whether the link is a null-link. When
	TRUE, the rigid extend is logically an identity matrix.
	\par Default Implementation:
	<b>{ return rigidExtend?false:true; }</b> */
	bool		NullLink() const {return rigidExtend?false:true;}
	/*! \remarks This method checks whether the link has no length. When TRUE,
	it is a pure rotation matrix. */
	bool		ZeroLength() const {
	  return NullLink() ? true :
		(rigidExtend->GetIdentFlags() & POS_IDENT) ? true : false; }
	/*! \remarks This method checks whether the degree of freedom is further
	constrained by lower limits. TRUE if constrained, otherwise FALSE.
	\par Default Implementation:
	<b>{ return llimited?true:false; }</b> */
	bool		LLimited() const { return llimited?true:false; }
	/*! \remarks This method checks whether the degree of freedom is further
	constrained by upper limits. TRUE if constrained, otherwise FALSE.
	\par Default Implementation:
	<b>{ return ulimited?true:false; }</b> */
	bool		ULimited() const { return ulimited?true:false; }
	
	/*! \remarks This mehod returns the matrix contribution by the degrees of
	freedom. Either it is a pure rotation or a pure translation, of one axis.
	The following identity holds;\n\n
	<b>LinkMatrix(true)</b> == <b>LinkMatrix(false) * DofMatrix()</b> */
	CoreExport	Matrix3		DofMatrix() const;
	/*! \remarks This method allows you to apply a matrix, <b>mat</b>, by the
	<b>DofMatrix()</b> so that <b>mat = mat * DofMatrix().</b>
	\par Parameters:
	<b>Matrix3\& mat</b>\n\n
	The matrix to multiply by the DOF matrix.
	\return  A reference to the matrix argument. */
	CoreExport	Matrix3&	DofMatrix(Matrix3& mat) const;	
	/*! \remarks This method returns the link matrix just defined if the
	argument is TRUE.
	\par Parameters:
	<b>bool include_dof = true</b>\n\n
	TRUE to return the link matrix, FALSE to return <b>RigidExtend</b>.
	\return  The link matrix, otherwise it simply returns <b>RigidExtend</b>.
	*/
	CoreExport	Matrix3		LinkMatrix(bool include_dof =true) const;
	/*! \remarks This methods applies the LinkMatrix() to the input matrix.
	\par Parameters:
	<b>Matrix3\& mat</b>\n\n
	The input matrix.\n\n
	<b>bool include_dof = true</b>\n\n
	When applying the DOF part, or rotation part, to a matrix, this will take
	place one at a time by calling <b>RootLink::RotateByAxis()</b>. If you want
	to apply the whole link, while already having applied the rotation part,
	you would need to set this flag to FALSE.
	\return  The reference to the input matrix, <b>mat</b>. */
	CoreExport	Matrix3&	ApplyLinkMatrix(Matrix3& mat, bool include_dof =true) const;
	
	// Set methods:
	//
	/*! \remarks This method allows you to activate or deactivate the lower
	limits.
	\par Parameters:
	<b>bool s</b>\n\n
	TRUE to activate, FALSE to deactivate. */
	void		SetLLimited(bool s) { llimited = s?1:0; }
	/*! \remarks This method allows you to activate or deactivate the upper
	limits.
	\par Parameters:
	<b>bool s</b>\n\n
	TRUE to activate, FALSE to deactivate. */
	void		SetULimited(bool s) { ulimited = s?1:0; }
	
	/*! \remarks This method allows you to set the RigidEtend matrix.
	\par Parameters:
	<b>const Matrix3\& mat</b>\n\n
	The rigid extend matrix you wish to set. */
	CoreExport	void		SetRigidExtend(const Matrix3& mat);

  private:
	Matrix3*	rigidExtend;
	byte		llimited : 1;
	byte		ulimited : 1;
  };

// A LinkChain consists of a RootLink and LinkCount() of Links.
// 
// * parentMatrix is where the root joint starts with respect to the
// world. It should not concern the solver. Solvers should derive their
// solutions in the parent space.
//
// * goal is represented in the parent space, i.e.,
//		goal_in_world = goal * parentMatrix
//
// * Bone(): The Link of index i may be a NullLink(). Bone(i) gives
// the index j so that j >= i and LinkOf(j).NullLink() is false. If j
// >= LinkCount() means that the chain ends up with NullLink().
//
// * PreBone(i) gives the index, j, so that j < i and LinkOf(j) is not
// NullLink(). For the following 3-dof joint:
//		---o++o++o---o
//            i
// Bone(i) == i+1, and PreBone(i) == i-2. Therefore, degrees of
// freedom of LinkOf(i) == Bone(i) - PreBone(i).
// 
// * A typical two bone chain with elbow being a ball joint has this
// structure:
//		---o++o++o---O
//         2  1  0   rootLink
// It has 3 links in addition to the root link.
//
// * A two-bone chain with the elbow being a hinge joint has this
// structure:
//		---o---O
//         0   rootLink
// It has one link. Geometrically, the axis of LinkOf(0) should be
// perpendicular to the two bones.
//
// * The matrix at the end effector is
//		End_Effector_matrix == LinkOf(n-1).LinkMatrix(true) * ... *
//			LinkOf(0).LinkMatrix(true) * rootLink.LinkMatrix(true).
//
// * swivelAngle, chainNormal, and defaultZeroMap concerns solvers that
// answer true to IKSolver::UseSwivelAngle().
//
// * chainNormal is the normal to the plane that is intrinsic to the
// chain when it is constructed. It is represented in the object space
// of the root joint.
//
// * A zero-map is a map that maps the end effector axis (EEA) to a
// plane normal perpendicular to the EEA. The IK System will provide a
// default one to the solver. However, a solver may choose to use its
// own.
//
// * Given the swivelAngle, the solver is asked to adjust the rotation
// at the root joint, root_joint_rotation, so that:
// (A)  EEA stays fixed
// (B)	chainNormal * root_joint_rotation
//		== zeroMap(EEA) * RotationAboutEEA(swivelAngle)
// By definition, zeroMap(EEA) is always perpendicular to EEA. At the
// initial pose, chainNormal is also guarranteed to be perpendicular
// to zeroMap(EEA). When it is not, root_joint_rotation has to
// maintain (A) absolutely and satisfy (B) as good as it is possible.
//
/*!  
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
This class abstracts the data structure that the IK system pass to the plugin
solver. With it, an IK problem becomes a pure mathematical one to the plugin
solver.\n\n
A <b>LinkChain</b> starts with a <b>rootLink</b>, followed by a number of 1D
link. The rootLink has a whole rotation (c.f. class <b>RootLink</b> in
ikHierarch.h), as represented by Euler angles of order XYZ, rotXYZ. These
angles may be marked as active or not. The <b>RootLink</b> exposes the whole
rotation to allow the solver to handle the start joint specially.\n\n
The next 1D link is to be pivoted and aligned at the reference frame that is
offset from the rotation of the RootLink by <b>"rigidExtend"</b>.
*/
  class LinkChain : public MaxHeapOperators {
  public:
	LinkChain():links(0),linkCount(0) {}
	CoreExport  LinkChain(unsigned lc);
	CoreExport  virtual		~LinkChain();
	virtual void* GetInterface(ULONG i) const { UNUSED_PARAM(i); return NULL; }
	/*! \remarks This <b>LinkChain</b> is originally cut off from a transformation
	space quantified by this matrix. Specifically, it is the matrix that
	accumulates all transformations from the root of the world to the position
	component of the start joint.\n\n
	  */
	Matrix3		parentMatrix;
	RootLink	rootLink;
	/*! \remarks The i-th 1D link. The next link is pivoted and aligned at\n\n
	<b> LinkOf(i).LinkMatrix(true) * LinkOf(i-1).LinkMatrix(true) ... *
	LinkOf(0).LinkMatrix(true) * rootLink.LinkMatrix(true)</b>\n\n
	This is the extremity after the i-th link. The position of the end
	effector, in particular, is the extremity of the last link. */
	const Link&	LinkOf(unsigned i) const {return links[i];}
	/*! \remarks Please refer to the above entry for full explanation.\n\n
	  */
	Link&		LinkOf(unsigned i) {return links[i];}
	/*! \remarks The number of 1D links following the rootLink.\n\n
	  */
	unsigned	LinkCount() const { return linkCount; }
	/*! \remarks A 1D link comprises a degree of freedom, which can be
	rotational or prismatic (sliding), and an offset transfromation,
	"rigidExtend". If the length of "rigidExtend" is zero, the next 1D link
	comes from same joint of this link. PreBone(i) returns the first link that
	precedes the i-th link that has non-zero "rigidExtend." In other words,
	LinkOf(PreBone(i) + 1) starts a new joint that includes i-th link as one of
	its degrees of freedom. PreBone(i) is always less then i.\n\n
	  */
	CoreExport	int			PreBone(unsigned i) const;
	/*! \remarks <b>Bone(i)</b> returns the first index, j, such that <b>j \>=
	i and LinkOf(j).ZeroLength()</b> false. This is the last link of the bone
	that includes i-th link as a degree of freedom.\n\n
	  */
	CoreExport	unsigned	Bone(unsigned i) const;

	/*! \remarks Returns an interface pointer to the goal. The actual type of
	goal can be queried from the interface. (They are documented
	separately.)\n\n
	  */
	BaseInterface* GetIKGoal() { return ikGoal; }
	/*! \remarks They are used to set and release the goal. The plugin solver
	should not worried about it. The IK system will use them to the set and release
	the goal.\n\n
	  */
	void SetIKGoal(BaseInterface* ikgoal) { ikGoal = ikgoal; }
	/*! \remarks Please refer to <b>SetIKGoal()</b> for full explanation. */
	CoreExport	void ReleaseIKGoal();
  protected:
	CoreExport	void SetLinkCount(unsigned lc);
  private:
	Link*		links;
	unsigned	linkCount;
	BaseInterface* ikGoal;
  };

//
// A convenience class that helps to iterate over the LinkChain on
// the basis of joint.
// Joint is defined as follows.
// (A) The RootLink is a rotaional joint.
// (B) A consecutive series of Link's of the same type
// (sliding v. rotational) of which only the last Link may have
// NullLink() being false.
// Steps to iterate, supposing linkChain is a LinkChain:
// * IterJoint iter(linkChain); -- make an iterator.
// * iter.InitJointAngles(); -- Set all the link variables to initial values.
// * iter.SetSkipSliding(true); -- If you want to skip sliding joints.
//		If your solver does not use sliding joint, they won't be present
// 		in the linkChain and there is no need to call it.
// * iter.Begin(true/false); -- Begin the iteration and this is the first
// 		joint. Pass true as the argument if you want the first joint to be
//		place in the world, according to linkChain.parentMatrix.
// * iter.GetJointType(); -- Is this joint rotational or sliding?
// * iter.DofCount(); -- How many degrees of freedom does it have?
// * iter.GetJointAxes(); -- Axes of each degree of freedom. It is represented
//		as char[3]. The usual xyz joint is "xyz". If this 1D of y, it is
//		"__y", 2D of xy as "_xy", etc.
// * iter.ProximalFrame(); -- The base reference frame of this joint. The
//		joint axes are relative to it.
// * iter.DistalFrame(); -- The reference frame that the joint brings the
//		base frame to.
// * iter.Pivot() -- The pivot of this joint.
// * iter.DistalEnd() -- The end point the rigid link attached to this joint.
//		If this is not the last joint, it is the pivot of the next joint.
// * iter.SetJointAngles(ang); -- Assign "ang" to joint angles. ang is a
//		Point3. If this is not a 3D joint, values are retrieved from ang
//		by name. For "_zx" joint, for instance, ang.z will be assigned to
//		the z-axis, and ang.x will be assigned to the x-axis.
//		After it is called, DistalFrame() and DistalEnd() will be updated.
// * iter.Next(); -- This brings to the next joint, if returns true. It is
//		the last joint if returns false.
// 
/*!  
\par Description:
<b>This class is only available in release 5 or later.</b>\n\n
<b>LinkChain</b> breaks joints, except for the root joint, of several degrees
of freedom into 1 degree of freedom link. Sometimes, joints are more
convenient. This is a helper class that helps to iterate over a
<b>LinkChain</b> on the basis of joints. Suppose <b>linkChain</b> is a
<b>LinkChain</b>, following code demonstrates how to iterate over
joints.\n\n
\code
IterJoint iter(linkChain);
iter.Begin(false);	// The first pivot starts at (0,0,0). To start from linkChain.parentMatrix, use iter.Begin(true).
do
{
	Point3 pivot_of_this_joint = iter.Pivot();
	Point3 pivot_of_next_joint = iter.DistalEnd();
	...
} while (iter.Next());
\endcode  */
class IterJoint : public MaxHeapOperators
{
public:
	struct JointAxes : public MaxHeapOperators {
		char& operator[](int i) { return mAxes[i]; }
		char mAxes[3];
	};
	enum JointType
	{
		SlidingJoint,
		RotationalJoint
	};

	IterJoint(LinkChain& lc)
		: mLinkChain(lc) , mSkipSlide(false)
	{
		// Empty ctor
	}
	
	/*! \brief Set all joint angles to the respective initial values. */
	CoreExport	void			InitJointAngles();
	
	/*! \remarks If a solver overrides "<b>bool
	IKSolver::UseSlidingJoint()</b>" to return false, the IK system
	will not include sliding joints in the <b>LinkChain</b> passed
	to the solver. If the <b>LinkChain</b> may include sliding
	joint, this method informs the iterator to skip, or not to
	skip, the sliding joint. It should be called before
	<b>Begin().</b> */
	void			SetSkipSliding(bool skip) { mSkipSlide = skip; }
	
	/*! \remarks Use <b>Begin()</b> to begin the iteration. The argument tells
	whether the pivot of the first joint starts at (0,0,0), if "in_world" is
	false, or at <b>LinkChain::parentMatrix</b> of the <b>LinkChain</b> that
	iterator works on. */
	CoreExport	void			Begin(bool in_world);

	/*! \remarks Whether this joint is rotational or sliding. */
	CoreExport	JointType		GetJointType() const;

	/*! \remarks The degrees of freedom of this joint and the axes
	of each degrees of freedom. JointAxes is string of chars. For
	3-dof joint of order XYZ, for example, the JointAxes would be
	"xyz". For 2-dof XZ joint, it would be "_xz". Leading
	underscores are used to fill up the string to 3 chars long. */
	int				DofCount() const { return mBegin == -1 ? 3: mEnd - mBegin; }

	/*! \remarks Please refer to <b>DofCount()</b> for full explanation. */
	CoreExport	JointAxes		GetJointAxes() const;

	/*! \remarks The proximal frame is the starting reference
	frame that this joint is aligned with. The distal frame is to
	be aligned with the proximal frame of the next joint.\n\n
	<b>Pivot()</b> is the position of the proximal frame, and
	<b>DistalEnd()</b> is the position of the distal frame. The
	distance between is the length of the bone. */
	const Matrix3&	ProximalFrame() const { return mMat0; }

	/*! \remarks Please refer to <b>ProximalFrame()</b> for full
	explanation. */
	const Matrix3&	DistalFrame() const { return mMat; }

	/*! \remarks Please refer to <b>ProximalFrame()</b> for full
	explanation. */
	Point3			Pivot() const { return mMat0.GetTrans(); }
	
	/*! \remarks Please refer to <b>ProximalFrame()</b> for full
	explanation. */
	Point3			DistalEnd() const { return mMat.GetTrans(); }

	/*! \remarks Set and get joint angles. angles.x, angles.y, angles.z,
	correspond to the joint angles with regard to x-, y-, and z-axis. If the
	joint does not contain y-axis, for example, angles.y is not significant. */
	CoreExport	void			SetJointAngles(const Point3&);

    /*! \remarks Please refer to <b>SetJointAngles()</b> for full explanation. */
    CoreExport	Point3			GetJointAngles() const;

	/*! \remarks Use Next() to move to the next joint, until it returns
	false.\n\n
	  */
	CoreExport	bool			Next();

protected:
	CoreExport	void			SkipSliding();
	
private:
	// Unimplemented assignment operator
	IterJoint& operator=(const IterJoint& );
	Matrix3		mMat0;
	Matrix3		mMat;
	LinkChain&	mLinkChain;
	bool		mSkipSlide;
	short		mBegin;
	short		mEnd;
	short		mNext;
};

// IK Goals:
//
/*! \sa  : Class BaseInterface\n\n
class IIKGoal : public BaseInterface\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>  This is the
interface class that various types of IK goal will derive from. The unique
interface id will specify what type of goal this interface provides.\n\n
 */
class IIKGoal : public BaseInterface {
public:
	/*! \remarks Returns a pointer of type BaseInterface for the interface
	IIKGoal
	\par Parameters:
	<b>Interface_ID id</b>\n\n
	Input id = kGoalID(<b>0x53937e2, 0x2be92941)</b> */
	/*! \remarks Returns, Interface_ID kGoalID(0x53937e2, 0x2be92941)\n\n
	  */
	BaseInterface* GetInterface(Interface_ID id) { if (id == kGoalID) return this; else return BaseInterface::GetInterface(id); }
	/*! \remarks Returns, Interface_ID kGoalID(0x53937e2, 0x2be92941)\n\n
	  */
	Interface_ID	GetID() { return kGoalID; }
	/*! \remarks Indicates to the function-published system that this
	interface wants to be released automatically upon disuse.\n\n
	  */
	LifetimeType	LifetimeControl() { return wantsRelease; }
	/*! \remarks Manually release this interface. */
	CoreExport void	ReleaseInterface();
	/*! \remarks Delete the interface. However, <b>IT IS HEAVILY ADVISED</b>
	that one must <b>FIRST RELEASE</b> the interface <b>BEFORE</b> delete is
	called. */
	virtual			~IIKGoal() {}
};

/*! \sa  : <a href="class_i_k_sys_1_1_i_i_k_goal.html">Class IIKGoal</a>\n\n
class IHIIKGoal: public IIKGoal\n\n

\par Description:
<b>This class is only available in release 5 or later.</b>  This is an
interface to a specific type of IK goal, the HI IK goal. The interface id is
given the constant <b>kHIIKGoalID</b>.\n\n
The plugin solver that declares its goal type, with
<b>IKSolver::ExpectGoal()</b>, as <b>kHIIKGoalID</b> should expect this type of
goal from <b>LinkChain::GetIKGoal()</b>. To get the interface pointer, one
can\n\n
<b>IHIIKGoal *ikgoal =
(IHIIKGoal*)linkChain-\>GetIKGoal()-\>GetInterface(kHIIKGoalID);</b>\n\n
    */
class IHIIKGoal: public IIKGoal {
public:
	// Due to BaseInterface
	BaseInterface* GetInterface(Interface_ID id) { if (id == kHIIKGoalID) return this; else return IIKGoal::GetInterface(id); }
	Interface_ID GetID() { return kHIIKGoalID; }

	/*! \remarks UseVHTarget() tells whether the swivel angle is determined
	(or computed) by a point in space to that the solver plane is supposed to
	pass, or is simply provided by a number literally. If it is true, the point
	is obtained from VHTarget(). Otherwise, the angle is obtained from
	SwivelAngle(). */
	virtual bool		UseVHTarget() const =0;
	/*! \remarks See explanation in UseVHTarget. */
	virtual float	 	SwivelAngle() const =0;
	/*! \remarks See explanation in UseVHTarget. */
	virtual const Point3& VHTarget() const =0;
	/*! \remarks Whether the swivel angle is interpreted in the space in which
	the start joint lies, or in the space of the goal. */
	virtual SAParentSpace SwivelAngleParent() const =0;
	/*! \remarks The normal of the solver plane at the preferred pose,
	represented in the parent space of the root joint. It serves as the zero
	reference for the swivel angle. */
	virtual const Point3& ChainNormal() const =0;
	/*! \remarks Returns the (IK) system provided default for the Zero Plane
	Map. */
	virtual const ZeroPlaneMap* DefaultZeroMap() const =0;
	/*! \remarks The goal for the end effector. */
	virtual Matrix3& Goal() =0;
};


/*! \sa  : <a href="class_i_k_sys_1_1_i_i_k_goal.html">Class IIKGoal</a>\n\n
class ISplineIKGoal: public IIKGoal\n\n
<b>This class is only available in release 5 or later.</b>  This is an
interface to a specific type of IK goal, the spline IK goal. The interface id
is given the constant <b>kSplineIKGoalID</b>.\n\n
 */
class ISplineIKGoal: public IIKGoal {
public:
	// Due to BaseInterface
	BaseInterface* GetInterface(Interface_ID id) { if (id == kSplineIKGoalID) return this; else return IIKGoal::GetInterface(id); }
	Interface_ID GetID() { return kSplineIKGoalID; }

	/*!  */
	virtual float				StartParam()					const = 0;
	virtual float				EndParam()						const = 0;
	//watje
	/*! \remarks Returns the position of a point on a spline or NURBS curve given
	its global u-value. The point can be computed in the local or the world space
	of the curve, represented by the second BOOL argument. The third BOOL argument
	determines if we should really recompute the curve or use the cached data.\n\n
	  */
	virtual Point3				SplinePosAt(float, BOOL, BOOL = FALSE)		const = 0;
	/*! \remarks Similar to the above: returns the tangent at a point on a spline
	or NURBS curve given its global u-value. The point can be computed in the local
	or the world space of the curve, represented by the second BOOL argument.\n\n
	  */
	virtual Point3				SplineTangentAt(float, BOOL)	const = 0;
	/*! \remarks Obtained from IHIIKGoal. I don't think we are currently using
	it for the SPlineIK.\n\n
	  */
	virtual const Matrix3&		Goal()							const = 0;
	/*! \remarks Grabs the Goal node.\n\n
	  */
	virtual INode*				GetGoalNode()					const = 0;
	/*! \remarks Returns a pointer IIKChainControl.\n\n
	  */
	virtual IIKChainControl*	GetChainControl()					  = 0;
	/*! \remarks Compute the length of the spline which the IKchain conforms
	to.\n\n
	  */
	virtual float				GetSplineLength()				const = 0;
	/*! \remarks Get the angle of the Start Twist Manipulator\n\n
	  */
	virtual float				TwistHStartAngle()				const = 0;
	/*! \remarks Get the angle of the End Twist Manipulator\n\n
	  */
	virtual float				TwistHEndAngle()				const = 0;
	/*! \remarks Get the Start Joint node of the IK Chain\n\n
	  */
	virtual INode*				StartJoint()					const =	0;
	/*! \remarks Get the End Joint node of the IK Chain\n\n
	  */
	virtual INode*				EndJoint()						const =	0;
	/*! \remarks Returns a ZeroPlaneMap.\n\n
	  */
	virtual const ZeroPlaneMap* DefaultZeroMap()				const =	0;
	/*! \remarks Returns TRUE/FALSE as to whether curve is open or closed. */
	virtual BOOL				IsClosed()						const = 0;
	virtual const Matrix3&		TwistParent()					const = 0;
};

//! \brief Extension to ISplineIKGoal, the interface of a spline IK goal object.

//! Interface ISplineIKGoal2 allows access to the data of a spline IK goal for 
//! one solve.  It extends interface ISplineIKGoal
// Supported by class SplineIKGoal
class ISplineIKGoal2: public ISplineIKGoal {
public:
	Interface_ID GetID() { return IID_SPLINE_IKGOAL2; }

	//! \brief Get the solve time of the IK goal 
	//! \return - the time at which the IK is being solved
	virtual const TimeValue&	GetSolveTime()	const = 0;
};

CoreExport IterJoint::JointType DofType(Link::DofAxis axis); 

}; // namespace IKSys
