/**********************************************************************
 
	FILE: iSkinPose.h

	DESCRIPTION:  Public interface for setting and getting a special,
	non-animated, transformation pose, SkinPose.

	CREATED BY: Jianmin Zhao, Discreet

	HISTORY: - created November 09, 2001

 *>	Copyright (c) 1998-2001, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "iFnPub.h"
#include "maxtypes.h"
#include "euler.h"
#include "control.h"
#include "inode.h"
#include "trig.h"

#define SKINPOSE_INTERFACE Interface_ID(0x66f226de, 0x37ff3962)

/*!  
\par Description:
Interface class for setting and getting a special,\n\n
non-animated, transformation pose, SkinPose.\n\n
  */
class ISkinPose : public FPMixinInterface {
public:
	// Utilities:
	/*! \remarks Method to obtain the interface pointer for a given INode.\n\n

	\par Parameters:
	<b>INode\& n</b>\n\n
	The node having the transformation pose.\n\n

	\return  <b>ISkinPose*</b>\n\n
	A pointer to this interface class.\n\n
	  */
	static ISkinPose* GetISkinPose(INode& n) {
		return static_cast<ISkinPose*>(n.GetInterface(SKINPOSE_INTERFACE)); }

	/*! \remarks Method to obtain the position part of the transformation
	pose.\n\n

	\return  <b>Point3</b>\n\n
	The position of the pose expressed as 3-vector (Point3).\n\n
	  */
	virtual Point3	SkinPos() const =0;
	/*! \remarks Method to obtain the rotation part of the transformation
	pose.\n\n

	\return  <b>RotationValue</b>\n\n
	The rotation of the pose expressed as an RotationValue, which can be
	quaternion or Euler angle type.\n\n
	  */
	virtual RotationValue SkinRot() const=0;
	/*! \remarks Method to obtain the scale part of the transformation
	pose.\n\n

	\return  <b>ScaleValue</b>\n\n
	The scale of the pose expressed as possibly unequal values along the
	principal axes of a coordinate system whose orientation is defined by a
	quaternion.\n\n
	  */
	virtual ScaleValue SkinScale() const =0;
	/*! \remarks Method to set the pose position to a given point.\n\n

	\par Parameters:
	<b>Point3</b>\n\n
	The desired point for the pose.\n\n
	  */
	virtual void	SetSkinPos(const Point3&) =0;
	/*! \remarks Method to set the pose rotation to a given value.\n\n

	\par Parameters:
	<b>RotationValue</b>\n\n
	The rotation of the pose expressed as an Euler angle or quaternion
	type.\n\n
	  */
	virtual void	SetSkinRot(const RotationValue&) =0;
	// Set the skin rotation via Euler angles:
	/*! \remarks Method to set the pose rotation to a given Euler angle.\n\n

	\par Parameters:
	<b>Point3</b>\n\n
	The rotation of the pose expressed as an Euler angle vector.\n\n
	  */
	virtual void	SetSkinRot(const Point3&) =0;
	/*! \remarks Method to set the potentially differing pose scale
	factors.\n\n

	\par Parameters:
	<b>Point3</b>\n\n
	The point containing the scale factors x, y, and z.\n\n
	  */
	virtual void	SetSkinScaleFactors(const Point3&) =0;
	/*! \remarks Method to set the orientation of the scale factor axes.\n\n

	\par Parameters:
	<b>Quat\&</b>\n\n
	The quaternion specifying the orientation of the scale factor axes.\n\n
	  */
	virtual void	SetSkinScaleOrient(const Quat&) =0;

	/*! \remarks Method to determine if the pose position is enabled.\n\n

	\return  <b>bool</b>\n\n
	If true, the pose position is enabled.\n\n
	If false, the pose position is disabled.\n\n
	  */
	virtual bool	IsSkinPosEnabled() const =0;
	/*! \remarks Method to determine if the pose rotation is enabled.\n\n

	\return  <b>bool</b>\n\n
	If true, the pose rotation is enabled.\n\n
	If false, the pose rotation is disabled.\n\n
	  */
	virtual bool	IsSkinRotEnabled() const =0;
	/*! \remarks Method to determine if the pose scale is enabled.\n\n

	\return  <b>bool</b>\n\n
	If true, the pose scale is enabled.\n\n
	If false, the pose scale is disabled.\n\n
	  */
	virtual bool	IsSkinScaleEnabled() const =0;
	/*! \remarks Member function yielding a Boolean whether the node is in the
	Skin Pose mode. In this mode, the node will assume the skin pose, subject
	to "enabled" flags of the three (position, rotation, and scale) parts, as
	its transformation, rather than from the normal channel, the transform
	controller.\n\n

	\return  <b>bool</b>\n\n
	If true, the node transformation is in the skin pose mode.\n\n
	If false, it is in the normal mode.\n\n
	  */
	virtual bool	SkinPoseMode() const =0;
	/*! \remarks Sets the state of the pose position transform.\n\n

	\par Parameters:
	<b>bool</b>\n\n
	If true, the pose position transform is enabled.\n\n
	If false, the pose position transform is disabled.\n\n
	  */
	virtual void	EnableSkinPos(bool) =0;
	/*! \remarks Sets the state of the pose rotation transform.\n\n

	\par Parameters:
	<b>bool</b>\n\n
	If true, the pose rotation transform is enabled.\n\n
	If false, the pose rotation transform is disabled.\n\n
	  */
	virtual void	EnableSkinRot(bool) =0;
	/*! \remarks Sets the state of the pose scale transform.\n\n

	\par Parameters:
	<b>bool</b>\n\n
	If true, the pose scale transform is enabled.\n\n
	If false, the pose scale transform is disabled.\n\n
	  */
	virtual void	EnableSkinScale(bool) =0;
	/*! \remarks Puts the node transform in the skin pose or normal mode.\n\n

	\par Parameters:
	<b>bool</b>\n\n
	If true, the node transform is put in the skin pose mode.\n\n
	If false, the node transform resumes to the normal mode.\n\n
	  */
	virtual void	SetSkinPoseMode(bool) =0;

	/*! \remarks Sets the state of all three non-animated skin pose
	transforms, subject to the "enabled" flags, to the animated normal pose at
	a particular time.\n\n

	\par Parameters:
	<b>TimeValue</b>\n\n
	Time at which the animated normal pose is used as the target to set the
	skin pose transforms.\n\n
	  */
	virtual void	SetSkinPose(TimeValue) =0;
	/*! \remarks Sets the state of all three animated normal pose transforms,
	subject to the "enabled" flags, at a particular time, to the non-animated
	skin pose transforms.\n\n

	\par Parameters:
	<b>TimeValue</b>\n\n
	Time at which to set the normal pose transforms.\n\n
	  */
	virtual void	AssumeSkinPose(TimeValue) =0;
	/*! \remarks This is a utility method used to set value to the node
	transform. According to whether it is in the skin pose mode and the three
	"enabled" flags, it will set value to the TM controller or the skin pose
	transforms.\n\n

	\par Parameters:
	<b>TimeValue</b>\n\n
	Time at which to set the pose transform.\n\n
	<b>SetXFormPacket\&</b>\n\n
	Controller values for the transform.\n\n
	  */
	virtual void	TMSetValue(TimeValue, SetXFormPacket&) =0;

	// Derived methods. They are all inline'd at the end of the file.
	//
	// SkinRotAngles() returns Euler angles of order XYZ.
	/*! \remarks Method to obtaion the Euler angles of the pose rotation.\n\n

	\return  <b>Point3</b>\n\n
	The Euler angles about x, y, and z.\n\n
	  */
	Point3			SkinRotAngles() const;
	/*! \remarks Method to obtain the scale factors of the pose
	transformation.\n\n

	\return  <b>Point3</b>\n\n
	The scale factors along the x, y, and z axes.\n\n
	  */
	Point3			SkinScaleFactors() const;
	/*! \remarks Method to obtain the scale factor orientation of the pose
	transformation.\n\n

	\return  <b>Quat</b>\n\n
	The axis and angle of the scale factor orientation.\n\n
	  */
	Quat			SkinScaleOrient() const;
	/*! \remarks Method to set the scale values for the pose transform.\n\n

	\par Parameters:
	<b>ScaleValue\&</b>\n\n
	The scale of the pose expressed as possibly unequal values along the principal
	axes of a coordinate system whose orientation is defined by a quaternion.*/
	void			SetSkinScale(const ScaleValue& sv);
	/*! \remarks Method to set the rotation angles for the pose transform.\n\n

	\par Parameters:
	<b>Point3\&</b>\n\n
	The Euler angles of the pose rotation.\n\n
	  */
	void			SetSkinRotAngles(const Point3&);
	/*! \remarks Method to determine if the pose position component is enabled
	and the node is in the skin pose mode.\n\n

	\return  <b>bool</b>\n\n
	If true, the position of the node transform will come from the skin
	pose.\n\n
	If false, the position of the node transform comes from the normal TM
	controller.\n\n
	  */
	bool			ShowSkinPos() const;
	/*! \remarks Method to determine if the pose rotation component is enabled
	and the node is in the skin pose mode.\n\n

	\return  <b>bool</b>\n\n
	If true, the rotation of the node transform will come from the skin
	pose.\n\n
	If false, the rotation of the node transform comes from the normal TM
	controller.\n\n
	  */
	bool			ShowSkinRot() const;
	/*! \remarks Method to determine if the pose scale component is enabled
	and the node is in the skin pose mode.\n\n

	\return  <b>bool</b>\n\n
	If true, the scale of the node transform will come from the skin pose.\n\n
	If false, the scale of the node transform comes from the normal TM
	controller.\n\n
	  */
	bool			ShowSkinScale() const;
	/*! \remarks Method to determine if one or more pose components; position,
	rotation, or scale are enabled.\n\n

	\return  <b>bool</b>\n\n
	If true, one or more components are enabled.\n\n
	If false, none of the components are enabled.\n\n
	  */
	bool			IsACompEnabled() const;

	/*! \remarks Method to indicate that a post transform component has
	changed from the default and needs to be saved.\n\n

	\return  <b>bool</b>\n\n
	If true, a pose component has changed.\n\n
	If false, no pose component has changed.\n\n
	  */
	virtual bool	 	NeedToSave() const =0;
	/*! \remarks Method to write pose data to a file.\n\n

	\par Parameters:
	<b>ISave*</b>\n\n
	Pointer for use in calling write methods.\n\n

	\return  <b>IOResult</b>\n\n
	If IO_OK, the method succeeded.\n\n
	If IO_ERROR, the method was unsuccessful.\n\n
	  */
	virtual IOResult	Save(ISave*) const =0;
	/*! \remarks Method to read pose data from a file.\n\n

	\par Parameters:
	<b>ILoad*</b>\n\n
	Pointer for use in calling read methods.\n\n

	\return  <b>IOResult</b>\n\n
	If IO_OK, the method succeeded.\n\n
	If IO_ERROR, the method was unsuccessful.\n\n
	  */
	virtual IOResult	Load(ILoad*) =0;
	/*! \remarks Method to copy data members from an existing ISkinPose
	instance to the current one.\n\n

	\par Parameters:
	<b>ISkinPose\&</b>\n\n
	Reference to instance of this class to copy from.\n\n
	  */
	virtual void	 	Copy(const ISkinPose&) =0;

	// Used by implementation class:
	/*! \remarks Determines whether this is a const object of a particular
	subclass derived from ISkinPose. It is used for the internal implementation
	purpose.\n\n

	\par Parameters:
	<b>void*</b>\n\n
	Pointer to the subclass identifier to test.\n\n

	\return  <b>void*</b>\n\n
	Const pointer to subclass.\n\n
	  */
	virtual const void* ObjectOf(void*) const =0;
	/*! \remarks Determines whether this is an object of a particular subclass
	derived from ISkinPose. It is used for the internal implementation
	purpose.\n\n

	\par Parameters:
	<b>void*</b>\n\n
	Pointer to the subclass identifier to test.\n\n

	\return  <b>void*</b>\n\n
	Pointer to subclass.\n\n
	  */
	virtual		  void*	ObjectOf(void*) =0;

	// Due to FPMixinInterface:
	/*! \remarks Method to obtain the function publishing interface
	description.\n\n

	\return  <b>FPInterfaceDesc *</b>\n\n
	Pointer to the interface descriptor. */
	FPInterfaceDesc* GetDesc() { return GetDescByID(SKINPOSE_INTERFACE); }

	// Function publishings:
	//
	enum FuncID {
		kSkinPosGet, kSkinPosSet,
		kSkinRotGet, kSkinRotSet,
		kSkinScaleGet, kSkinScaleSet,
		kSkinScaleOrientGet, kSkinScaleOrientSet,
		// When SkinPosEnabled is false, SkinPoseMode, SetSkinPose, and
		// AssumeSkinPose will disregard the position component.
		// Similar to SkinRotEnabled and SkinScaleEnabled.
		kSkinPosEnabledGet, kSkinPosEnabledSet,
		kSkinRotEnabledGet, kSkinRotEnabledSet,
		kSkinScaleEnabledGet, kSkinScaleEnabledSet,
		kSkinPoseModeGet, kSkinPoseModeSet,
		kSetSkinPose,
		kAssumeSkinPose
	};
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		PROP_FNS(kSkinPosGet, SkinPos, kSkinPosSet, SetSkinPos, TYPE_POINT3_BV)
		PROP_FNS(kSkinRotGet, SkinRotAngles, kSkinRotSet, SetSkinRotAngles, TYPE_POINT3_BV)
		PROP_FNS(kSkinScaleGet, SkinScaleFactors, kSkinScaleSet, SetSkinScaleFactors, TYPE_POINT3_BV)
		PROP_FNS(kSkinScaleOrientGet, SkinScaleOrient, kSkinScaleOrientSet, SetSkinScaleOrient, TYPE_QUAT_BV)
		PROP_FNS(kSkinPosEnabledGet, IsSkinPosEnabled, kSkinPosEnabledSet, EnableSkinPos, TYPE_bool)
		PROP_FNS(kSkinRotEnabledGet, IsSkinRotEnabled, kSkinRotEnabledSet, EnableSkinRot, TYPE_bool)
		PROP_FNS(kSkinScaleEnabledGet, IsSkinScaleEnabled, kSkinScaleEnabledSet, EnableSkinScale, TYPE_bool)
		PROP_FNS(kSkinPoseModeGet, SkinPoseMode, kSkinPoseModeSet, SetSkinPoseMode, TYPE_bool)
		VFNT_0(kSetSkinPose, SetSkinPose)
		VFNT_0(kAssumeSkinPose, AssumeSkinPose)
	END_FUNCTION_MAP
	#pragma warning(pop)
};

// inlines for derived methods
//
inline Point3 ISkinPose::SkinRotAngles() const
{
	return SkinRot().Euler() * RAD_TO_DEG;
}

inline Point3 ISkinPose::SkinScaleFactors() const
{
	return SkinScale().s;
}

inline Quat ISkinPose::SkinScaleOrient() const
{
	return SkinScale().q;
}

inline void ISkinPose::SetSkinScale(const ScaleValue& sv)
{
	SetSkinScaleFactors(sv.s);
	SetSkinScaleOrient(sv.q);
}

inline void	ISkinPose::SetSkinRotAngles(const Point3& p)
{
	SetSkinRot(p * DEG_TO_RAD);
}

inline bool ISkinPose::ShowSkinPos() const
{
	return SkinPoseMode() && IsSkinPosEnabled();
}

inline bool ISkinPose::ShowSkinRot() const
{
	return SkinPoseMode() && IsSkinRotEnabled();
}

inline bool ISkinPose::ShowSkinScale() const
{
	return SkinPoseMode() && IsSkinScaleEnabled();
}

inline bool ISkinPose::IsACompEnabled() const
{
	return (IsSkinPosEnabled() || IsSkinRotEnabled() || IsSkinScaleEnabled());
}

