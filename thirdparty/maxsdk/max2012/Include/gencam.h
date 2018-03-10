/**********************************************************************
 *<
	FILE: gencamera.h

	DESCRIPTION:  Defines General-Purpose cameras

	CREATED BY: Tom Hudson

	HISTORY: created 5 December 1995

 *>	Copyright (c) 1995, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "object.h"

// Camera types
#define FREE_CAMERA 0
#define TARGETED_CAMERA 1
#define PARALLEL_CAMERA 2

#define NUM_CAM_TYPES 2

/*! \sa  Class CameraObject, Class Interval, Class Control.\n\n
\par Description:
This class describes a generic camera object. It is used as a base class for
creating plug-in cameras. Methods of this class are used to get and set
properties of the camera. All methods of this class are virtual.  */
#pragma warning(push)
#pragma warning(disable:4239 4100)

class GenCamera: public CameraObject {			   
	public:
		/*! \remarks Creates a new generic camera object.
		\par Parameters:
		<b>int type</b>\n\n
		Nonzero if the camera has a target; otherwise 0.
		\return  A pointer to a new instance of the specified light type. */
		virtual GenCamera *NewCamera(int type)=0;
		/*! \remarks Sets if the camera cone is displayed in the viewports.
		\par Parameters:
		<b>int s</b>\n\n
		Nonzero to display the camera cone; otherwise 0. */
		virtual void SetConeState(int s)=0;
		/*! \remarks Returns TRUE if the camera cone is displayed in the
		viewports; otherwise FALSE. */
		virtual int GetConeState()=0;
		/*! \remarks Sets if the camera has a horizon line displayed.
		\par Parameters:
		<b>int s</b>\n\n
		Nonzero to display the horizon line; otherwise 0. */
		virtual void SetHorzLineState(int s)=0;
		/*! \remarks Returns TRUE if the camera has a horizon line displayed;
		otherwise FALSE. */
		virtual int GetHorzLineState()=0;
		/*! \remarks Enables or disables the camera. If enabled the camera may
		be displayed, hit tested, etc.
		\par Parameters:
		<b>int enab</b>\n\n
		Nonzero to enable; zero to disable. */
		virtual void Enable(int enab)=0;
		/*! \remarks Sets the controller for the field-of-view parameter.
		\par Parameters:
		<b>Control *c</b>\n\n
		Points to the controller to set.
		\return  Returns TRUE if set; otherwise FALSE. */
		virtual BOOL SetFOVControl(Control *c)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the Field-Of-View type of the camera.
		\par Parameters:
		<b>int ft</b>\n\n
		One of the following values:\n\n
		<b>FOV_W</b>\n\n
		Width-related FOV\n\n
		<b>FOV_H</b>\n\n
		Height-related FOV\n\n
		<b>FOV_D</b>\n\n
		Diagonal-related FOV */
		virtual void  SetFOVType(int ft)=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns the Field-Of-View type of the camera. One of the following
		values:\n\n
		<b>FOV_W</b>\n\n
		Width-related FOV\n\n
		<b>FOV_H</b>\n\n
		Height-related FOV\n\n
		<b>FOV_D</b>\n\n
		Diagonal-related FOV */
		virtual int GetFOVType()=0;
		/*! \remarks Returns the controller for the field-of-view parameter.
		*/
		virtual Control *GetFOVControl()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Returns one of the following values to indicate the camera type:\n\n
		<b>FREE_CAMERA</b> (No Target)\n\n
		<b>TARGETED_CAMERA</b> (Target / Look At Controller)\n\n
		<b>PARALLEL_CAMERA</b> (Orthographic Camera) */
		virtual	int  Type()=0;
		/*! \remarks This method is available in release 3.0 and later
		only.\n\n
		Sets the type of camera.
		\par Parameters:
		<b>int tp</b>\n\n
		One of the following types:\n\n
		<b>FREE_CAMERA</b> (No Target)\n\n
		<b>TARGETED_CAMERA</b> (Target / Look At Controller)\n\n
		<b>PARALLEL_CAMERA</b> (Orthographic Camera) */
		virtual void SetType(int tp)=0;

		virtual void SetDOFEnable(TimeValue t, BOOL onOff) {}
		virtual BOOL GetDOFEnable(TimeValue t, Interval& valid = Interval(0,0)) { return 0; }
		virtual void SetDOFFStop(TimeValue t, float fs) {}
		virtual float GetDOFFStop(TimeValue t, Interval& valid = Interval(0,0)) { return 1.0f; }
	};
#pragma warning(pop)
