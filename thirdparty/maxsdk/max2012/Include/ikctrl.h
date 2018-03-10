/**********************************************************************
 *<
	FILE:  ikctrl.h

	DESCRIPTION:  Inverse Kinematics Controllers

	CREATED BY:  Rolf Berteig

	HISTORY: 3-1-97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "ref.h"
#include "control.h"

#define IKMASTER_CLASSID		Class_ID(0xa91004be,0x9901fe83)
#define IKSLAVE_CLASSID			Class_ID(0xbe380a31,0x310dc9e4)

/*! \sa  Class ReferenceTarget, Class IKSlaveControl, Class Control.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
The IK Controller requires that you use the Bones system. When you create the
bones, a slave IK controller is assigned to each bone. All of the slave IK
controllers in a single hierarchy are, in turn, controlled by a master IK
controller. This class provides two methods to work with the master controller.
To get an interface to this class call <b>GetInterface(I_MASTER);</b>on the
controller in question. If the return value is non-NULL you can cast the
pointer to an instance of this class.\n\n
For an example the use of this class see
<b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>.  */
class IKMasterControl : public ReferenceTarget {
	public:
		Class_ID			ClassID() {return IKMASTER_CLASSID;}
		SClass_ID			SuperClassID() {return REF_TARGET_CLASS_ID;}

		/*! \remarks Adds the specified node to the list of slave nodes
		maintained.
		\par Parameters:
		<b>INode *node</b>\n\n
		The node to add. */
		virtual void		AddSlaveNode(INode *node)=0;
		/*! \remarks Returns a pointer to the IK master object. */
		virtual void		*GetMasterBase()=0;

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the position threshold. If the UI for the master controller is not
		visible while this method is called, the screen is not redrawn to
		reflect the changes. After calling this method you should therefore
		call <b>Interface::RedrawViews()</b>.
		\par Parameters:
		<b>float t</b>\n\n
		The value to set. */
		virtual void		SetPosThresh(float t)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the rotation threshold. If the UI for the master controller is not
		visible while this method is called, the screen is not redrawn to
		reflect the changes. After calling this method you should therefore
		call <b>Interface::RedrawViews()</b>.
		\par Parameters:
		<b>float t</b>\n\n
		The value to set. */
		virtual void		SetRotThresh(float t)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the iterations value. If the UI for the master controller is not
		visible while this method is called, the screen is not redrawn to
		reflect the changes. After calling this method you should therefore
		call <b>Interface::RedrawViews()</b>.
		\par Parameters:
		<b>int i</b>\n\n
		The value to set. */
		virtual void		SetIterations(int i)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the start time. If the UI for the master controller is not visible
		while this method is called, the screen is not redrawn to reflect the
		changes. After calling this method you should therefore call
		<b>Interface::RedrawViews()</b>.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		The time to set. */
		virtual void		SetStartTime(TimeValue s)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the end time. If the UI for the master controller is not visible
		while this method is called, the screen is not redrawn to reflect the
		changes. After calling this method you should therefore call
		<b>Interface::RedrawViews()</b>.
		\par Parameters:
		<b>TimeValue e</b>\n\n
		The time to set. */
		virtual void		SetEndTime(TimeValue e)=0;

		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the position threshold. */
		virtual float		GetPosThresh()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the rotation threshold. */
		virtual float		GetRotThresh()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the iterations setting. */
		virtual int			GetIterations()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the start time. */
		virtual TimeValue	GetStartTime()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the end time. */
		virtual TimeValue	GetEndTime()=0;

		virtual void RemoveIKChainControllers(TimeValue t)=0;
	};

/*! \sa  Class Control, Class IKMasterControl, Class Point3,  Class Quat.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
The IK Controller requires that you use the Bones system. When you create the
bones, a slave IK controller is assigned to each bone. All of the slave IK
controllers in a single hierarchy are, in turn, controlled by a master IK
controller. This class provides access to the slave controller parameters.\n\n
If you have a pointer to a TM controller you may test the Class_ID against
<b>IKSLAVE_CLASSID</b> to determine if it is an IK Slave Controller.\n\n
For an example the use of this class see
<b>/MAXSDK/SAMPLES/OBJECTS/BONES.CPP</b>.  */
class IKSlaveControl : public Control {
	public:
		Class_ID ClassID() {return IKSLAVE_CLASSID;}
		SClass_ID SuperClassID() {return CTRL_MATRIX3_CLASS_ID;}

		/*! \remarks Returns a pointer to the <b>IKMasterControl</b> that
		manages this IK slave controller. */
		virtual IKMasterControl *GetMaster()=0;
		/*! \remarks Sets the specified degree of freedom to on or off.
		\par Parameters:
		<b>int which</b>\n\n
		Specifies which degree of freedom to modify. Pass a value between 1 and
		6. The first 3 DOFs are position and the next 3 are rotation.\n\n
		<b>BOOL onOff</b>\n\n
		TRUE for on; FALSE for off. */
		virtual void SetDOF(int which,BOOL onOff)=0;
		/*! \remarks Sets the initial position to the value passed.
		\par Parameters:
		<b>Point3 pos</b>\n\n
		The initial position to set. */
		virtual void SetInitPos(Point3 pos)=0;
		/*! \remarks Sets the initial rotation to the value passed.
		\par Parameters:
		<b>Point3 rot</b>\n\n
		The initial rotation to set. */
		virtual void SetInitRot(Point3 rot)=0;
		/*! \remarks For any IK solution you explicitly move one object. 3ds
		Max uses IK calculations to move and rotate all other objects in the
		kinematic chain to react to the object you moved. The object that you
		move is the end effector. There are two types of end effectors;
		Position and Rotation. This method create or deletes an end effector
		for position or rotation or both.
		\par Parameters:
		<b>BOOL onOff</b>\n\n
		Controls if the end effector is on or off (creates or deletes the end
		effector controller). TRUE to create; FALSE to delete.\n\n
		<b>DWORD which</b>\n\n
		Specifies which end effector(s) to modify. Set the low order bit for
		position, the second bit for rotation, or set both for position and
		rotation.\n\n
		<b>Point3 pos</b>\n\n
		The initial position set at time 0 for the position controller
		(CTRL_ABSOLUTE).\n\n
		<b>Quat rot</b>\n\n
		The initial rotation set at time 0 for the rotation controller
		(CTRL_ABSOLUTE). */
		virtual void MakeEE(BOOL onOff,DWORD which,Point3 pos,Quat rot)=0;
	};

/*! \remarks This global function creates a new IK master controller. */
CoreExport IKMasterControl *CreateIKMasterControl();
/*! \remarks This global function creates and returns a pointer to a new IK
slave controller by specifying the master controller and a node to which the
slave controller is assigned.
\par Parameters:
<b>IKMasterControl *master</b>\n\n
Points to the master controller to use.\n\n
<b>INode *slaveNode</b>\n\n
Points to the slave node to use. */
CoreExport IKSlaveControl *CreateIKSlaveControl(IKMasterControl *master,INode *slaveNode);


