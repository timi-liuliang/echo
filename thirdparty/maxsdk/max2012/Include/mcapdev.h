/**********************************************************************
 *<
	FILE: mcapdev.h

	DESCRIPTION: Motion capture device plug-in interface

	CREATED BY: Rolf Berteig

	HISTORY: May 01, 1997

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "control.h"

class IMCParamDlg;
class MCDeviceBinding;
class IRollupWindow;

// Motion capture controller class IDs
#define POS_MOTION_CLASS_ID			0xff8826de
#define ROT_MOTION_CLASS_ID			0xff7826df
#define SCALE_MOTION_CLASS_ID		0xff6826da
#define FLOAT_MOTION_CLASS_ID		0xff5826db
#define POINT3_MOTION_CLASS_ID		0xff4826dc

// If a controller has one of the above class IDs, then it
// can be cast into this class.
/*! \sa  Class Control, Class MCDeviceBinding.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
If a motion capture controller has one of the following class IDs, then it can
be cast into this class to give access to the controller's parameters.\n\n
<b>POS_MOTION_CLASS_ID</b>\n\n
<b>ROT_MOTION_CLASS_ID</b>\n\n
<b>SCALE_MOTION_CLASS_ID</b>\n\n
<b>FLOAT_MOTION_CLASS_ID</b>\n\n
<b>POINT3_MOTION_CLASS_ID</b>\n\n
Note that normally a developer will not call these methods.\n\n
All methods of this class are implemented by the system.  */
class IMCControl : public Control {
	public:
		/*! \remarks Returns TRUE if Live mode is on; otherwise FALSE. Live
		mode is invoked by pressing the 'Test' button in the Motion Capture
		user interface. */
		virtual BOOL IsLiveOn()=0;
		/*! \remarks Returns TRUE if Record mode is on; otherwise FALSE.
		Record mode is set by pressin the 'Start' button in the Motion Capture
		user interface. */
		virtual BOOL IsRecordOn()=0;
		/*! \remarks Returns the number of device bindings. */
		virtual int NumDeviceBindings()=0;
		/*! \remarks Returns a pointer to the 'i-th' device binding.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which device binding to return. */
		virtual MCDeviceBinding *GetDeviceBinding(int i)=0;
		/*! \remarks Sets the 'i-th' device bindings.
		\par Parameters:
		<b>int i</b>\n\n
		Specifies which device binding to set.\n\n
		<b>MCDeviceBinding *b</b>\n\n
		Points to the device binding to set. */
		virtual void SetDeviceBinding(int i,MCDeviceBinding *b)=0;		
		/*! \remarks This method retrieves the value based on the current
		state of the device (not the value of the controller). For instance,
		say you have done some motion capture using a joystick plugged into a
		position motion capture controller. You can call <b>GetValue()</b> on
		the controller to get its recorded value (just like a keyframe
		controller), or you can call this method to get the actual state of the
		joystick.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		Specifies the time to update the value.\n\n
		<b>void *val</b>\n\n
		Points to the value that is updated. See
		<a href="class_control.html#A_GM_ctrl_getset">Control::GetValue()</a>
		for the data types returned here.\n\n
		<b>GetSetMethod method</b>\n\n
		One of the following values:\n\n
		<b>CTRL_RELATIVE</b>\n\n
		Indicates the plug-in should apply the value of the controller to
		<b>*val</b>.\n\n
		<b>CTRL_ABSOLUTE</b>\n\n
		Indicates the controller should simply store its value in <b>*val</b>.
		*/
		virtual void GetValueLive(TimeValue t,void *val, GetSetMethod method)=0;
	};

/*! \sa  Class MCDeviceBinding, Class IMCapManager.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is an interface into the motion capture manager. This interface is passed
to plug-ins derived from class <b>MCDeviceBinding</b>. All methods of this
class are implemented by the system.  */
class IMCapManager: public MaxHeapOperators {
	public:
		/*! \remarks This method is obsolete. */
		virtual void MidiNote(int channel, int note)=0;
		/*! \remarks This method returns the current time at which it is
		called. This is an aide for devices where interaction is happening
		asynchronously (the MIDI interface is an example of this). The MIDI
		motion capture device uses a separate thread to track the MIDI
		keyboard. When the user presses a key, the MIDI device plug-in needs to
		record which key was pressed and when. It calls this method to grab the
		current 3ds Max time at which it happened. */
		virtual TimeValue GetTime()=0;
	};

#pragma warning(push)
#pragma warning(disable:4100)

// Base class for an input device
/*! \sa  Class MCDeviceBinding, Class IMCapManager, <a href="ms-its:3dsmaxsdk.chm::/anim_time_functions.html">Time</a>,
<a href="ms-its:3dsmaxsdk.chm::/ui_character_strings.html">Character Strings</a>.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This is the base class for an input device plug-in. All methods of this class
are virtual.\n\n
In terms of the motion capture system, the basic item that is plug-able is the
motion capture device. This is something like a mouse, joystick, or midi
device. Developers implement two classes, this one, <b>MCInputDevice</b>, and
<b>MCDeviceBinding</b>. There is usually only one instance of
<b>MCInputDevice</b>. This is like the virtual mouse, or the joystick. This
represents the actual device. An instance of the device binding represents an
instance where a motion capture controller has been bound to a device, i.e. the
user has picked the device and assigned it to a parameter. Thus there may be
many instances of the device binding. The device binding is part of the
reference hierarchy. The device itself doesn't usually have any parameters for
the user to adjust -- these are rather part of the device binding.\n\n
Some simple sample code for the mouse motion capture device is available in
<b>/MAXSDK/SAMPLES/MOCAP/MCDEVICE.CPP</b>.  */
class MCInputDevice: public MaxHeapOperators {
	public:
		/*! \remarks Destructor. */
		virtual ~MCInputDevice() {}
		/*! \remarks Returns the name for the input device. */
		virtual MSTR DeviceName()=0;
		/*! \remarks The motion capture utility creates a list of all the
		MCInputDevices in the system. When the user wants to pick one it will
		call this method. It returns a new instance of the
		<b>MCDeviceBinding</b> class. */
		virtual MCDeviceBinding *CreateBinding()=0;
		/*! \remarks This method is called when the user enters the utility.
		\par Parameters:
		<b>IMCapManager *im</b>\n\n
		This is an interface into the motion capture manager.
		\par Default Implementation:
		<b>{}</b> */
		virtual void UtilityStarted(IMCapManager *im) {}
		/*! \remarks This method is called when the user leaves the utility.
		\par Parameters:
		<b>IMCapManager *im</b>\n\n
		This is an interface into the motion capture manager.
		\par Default Implementation:
		<b>{}</b> */
		virtual	void UtilityStopped(IMCapManager *im) {}
		/*! \remarks This method is called when the user is in 'Record'
		(capture) mode or 'Test' mode. It is called once per millisecond. For
		instance the joystick device uses this method. To understand this
		method consider the following example:\n\n
		With MIDI you don't call a function to see if a key has been pressed or
		not -- rather it is a message based system where you're notified if
		something happens. In contrast to this is the joystick. If the user
		moves the joystick the program is not notified. Rather a developer must
		poll the joystick to get its current position. During motion capture
		one could poll the joystick at every frame to get its current position.
		However this approach leads to jittering (aliasing). The problem is
		that, on average, the joystick is providing a smooth series of values,
		but instantaneously, the values jump around a bit. So, the joystick
		motion capture plug-in implements this method to stores the values
		returned at every millisecond. Then later, when needing to sample the
		joystick at a certain time, the stored table of values can be averaged
		and this provides a level of smoothing.
		\par Parameters:
		<b>UINT tick</b>\n\n
		The time of this call in milliseconds.
		\par Default Implementation:
		<b>{}</b> */
		virtual void Cycle(UINT tick) {}		

	};

// An instance of this class is created when a motion caprture controller
// binds one of its parameters to a device. The main purpose of this
// class is to store any parameters that describe the binding.
/*! \sa  Class ReferenceTarget, Class MCInputDevice, Class IMCParamDlg, Class IRollupWindow.\n\n
\par Description:
An instance of this class is created when a motion capture controller binds one
of its parameters to a device. The main purpose of this class is to store any
parameters that describe the binding.  */
class MCDeviceBinding : public ReferenceTarget {
	public:
		/*! \remarks Destructor. */
		virtual ~MCDeviceBinding() {}
		/*! \remarks Returns a pointer to the bound input device. */
		virtual MCInputDevice *GetDevice()=0;
		/*! \remarks Returns the name of the bound input device. */
		virtual MSTR BindingName()=0;
		/*! \remarks A device binding is the thing that the controller
		evaluates to get the value of the device. Everything is simply a scalar
		parameter. So for example even for a device like a mouse that has X and
		Y motion the device binding will break down into simply X or Y. This
		method is used to return the value of the device at the instant this
		method is called.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The time at which this method is called. */
		virtual float Eval(TimeValue t)=0;
		/*! \remarks Deletes this instace of the class. */
		virtual void DeleteThis()=0;		
		/*! \remarks This method is called to allow the binding to put up any
		user interface it has into the command panel via rollup pages.
		\par Parameters:
		<b>IMCParamDlg *dlg</b>\n\n
		The <b>IRollupWindow</b> data member of this class may be used to add
		the rollup page.
		\par Sample Code:
		\code
		dlg->iRoll->AppendRollup(hInstance, MAKEINTRESOURCE(IDD_MC_MOUSE),MouseDeviceDlgProc,		GetString(IDS_RB_MOUSEDEVICE), (LPARAM)dlg);
		\endcode  */
		virtual void AddRollup(IMCParamDlg *dlg)=0;
		/*! \remarks This method is called to allow the plug-in to update the
		values in its user interface rollup.
		\par Parameters:
		<b>IRollupWindow *iRoll</b>\n\n
		The interface into the command panel rollups. The <b>GetPanelDlg()</b>
		method may be used to return the window handle of the dialog and this
		<b>HWND</b> may be used to update the controls. */
		virtual void UpdateRollup(IRollupWindow *iRoll)=0;
		/*! \remarks This method is called when the binding becomes active.
		\par Parameters:
		<b>BOOL reset=TRUE</b>\n\n
		If TRUE 3ds Max is being reset; otherwise this is the first time the
		binding is becoming active.
		\par Default Implementation:
		<b>{}</b> */
		virtual void BeginActivate(BOOL reset=TRUE) {}
		/*! \remarks This method is called when the binding has been released.
		\par Default Implementation:
		<b>{}</b> */
		virtual void EndActivate() {}
		/*! \remarks This method is called 50 times per second during motion
		capture.\n\n
		To understand how this is used consider the following two situations
		for a motion capture device:\n\n
		1. The motion capture device is a joystick, and the position of
		the joystick directly maps to a range of some parameter. In this case,
		if you need to evaluate the parameter, you simply evaluate the joystick
		(inside the <b>Eval()</b> method). The position establishes the
		value.\n\n
		2. A different case is where you have a parameter at a starting
		value, and if the joystick is moved, to say the right, the value is
		incremented. If the joystick is moved to the left the value is
		decremented. In this case the value can theoretically reach any value.
		What is needed is for the value to be incremented and decremented in a
		consistent fashion. If the joystick is polled only during the
		<b>Eval()</b> method, and the value is incremented or decremented
		there, it may be a problem. If two things are using the same motion
		capture device, the value will be incremented or decremented twice
		inside <b>Eval()</b>. This will cause the value to grow or shrink twice
		as fast. If three things evaluated the same joystick it would move
		three times as fast because it would get incremented three times per
		frame. The solution is to use this method. It is called 50 times per
		second. The increments are done inside this method, and when the
		<b>Eval()</b> method is called the accumulated state is simply
		returned. This works because the method is called a fixed number of
		times per second regardless of the number of items evaluating the
		device.
		\par Parameters:
		<b>TimeValue t</b>\n\n
		The current time when this method is called.
		\par Default Implementation:
		<b>{}</b> */
		virtual void Accumulate(TimeValue t) {}

		SClass_ID SuperClassID() {return MOT_CAP_DEVBINDING_CLASS_ID;}		
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
	         PartID& partID,  RefMessage message) {return REF_SUCCEED;}

	};
#pragma warning(pop)
/*! \sa  Class ReferenceMaker, Class MCDeviceBinding, Class IRollupWindow.\n\n
\par Description:
This class is an interface to allow the plug-in to provide a user interface in
the command panel. It has two data members.
\par Data Members:
<b>MCDeviceBinding *binding;</b>\n\n
Returns a pointer to the device binding.\n\n
<b>IRollupWindow *iRoll;</b>\n\n
This is an interface into the command panel. Its methods may be used to work
with rollup pages and alter UI controls. */
class IMCParamDlg : public ReferenceMaker {
	public:
		MCDeviceBinding *binding;
		IRollupWindow *iRoll;
	};




