/**********************************************************************
 *<
	FILE:  soundobj.h

	DESCRIPTION:  Sound plug-in object base class

	CREATED BY:  Rolf Berteig

	HISTORY: created 2 July 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include <WTypes.h>

#pragma warning(push) 
#pragma warning(disable:4201)
#include <vfw.h>
#pragma warning(pop) 

#include "interval.h"
#include "ref.h"
#include "plugapi.h"

#define I_WAVEPAINT 94

// A SoundObject implements this interface to let the TrackBar paint a waveform.
class IWavePaint: public MaxHeapOperators {
public:
	virtual void PaintWave(HDC hdc, RECT* rect, Interval i) = 0;
	};

/*! \sa  Class ReferenceTarget, Class Animatable.\n\n
\par Description:
This is the base class for the creation of sound plug-ins. The 3ds Max user may
choose a sound plug-in using the File / Preferences... Animation Tab / Sound
Plug-In option.\n\n
There is always one sound object in the scene. A sound object's primary purpose
is to provide a sound track for the scene. The sound object also serves as a
clock that controls timing when an animation is played. This ensure the
animation is synched to the sound object. This class has methods to start and
stop the sound playing, play a specified range of the sound, and toggle the
sound on and off.\n\n
A sound plug-in can participate in Track %View by implementing the methods of
Animatable such as <b>PaintTrack()</b>. See the Advanced Topics section
<a href="ms-its:3dsmaxsdk.chm::/tview_track_view.html">Track %View</a> for
details.\n\n
Sound Object plug-ins use a Super Class ID of <b>SOUNDOBJ_CLASS_ID</b>.  */
class SoundObj : public ReferenceTarget {
	public:
		virtual SClass_ID SuperClassID() {return SClass_ID(SOUNDOBJ_CLASS_ID);}		

		/*! \remarks When the system calls this method the plug-in should loop the playing
		of sound from time <b>t0</b> to <b>t1</b> beginning at time
		<b>tStart</b>. It should continue to loop until <b>Stop()</b> is
		called.
		\par Parameters:
		<b>TimeValue tStart</b>\n\n
		The time to start playing the sound.\n\n
		<b>TimeValue t0</b>\n\n
		The loop begin range.\n\n
		<b>TimeValue t1</b>\n\n
		The loop end range.\n\n
		<b>TimeValue frameStep</b>\n\n
		The frame increment.
		\return  TRUE if the sound was played; FALSE otherwise. */
		virtual BOOL Play(TimeValue tStart,TimeValue t0,TimeValue t1,TimeValue frameStep)=0;
		/*! \remarks Implementation of this method is optional. The plug-in should play the
		amount of sound between time <b>t0</b> and <b>t1</b>. The sound should
		only be played once.
		\par Parameters:
		<b>TimeValue t0</b>\n\n
		The start time for playback.\n\n
		<b>TimeValue t1</b>\n\n
		The end time for playback. */
		virtual void Scrub(TimeValue t0,TimeValue t1)=0;
		/*! \remarks This stops the sound from playing.
		\return  The time at which the sound was stopped. */
		virtual TimeValue Stop()=0;
		/*! \remarks This returns the current time as managed by the <b>SoundObj</b>.
		\return  The current time. */
		virtual TimeValue GetTime()=0;
		/*! \remarks Returns TRUE if the sound is playing; otherwise FALSE. */
		virtual BOOL Playing()=0;
		/*! \remarks This saves the sound between the specified times to the specified file.
		\par Parameters:
		<b>PAVIFILE pfile</b>\n\n
		The file to save the sound track to.\n\n
		<b>TimeValue t0</b>\n\n
		The start of the time range to save.\n\n
		<b>TimeValue t1</b>\n\n
		The end of the time range to save. */
		virtual void SaveSound(PAVIFILE pfile,TimeValue t0,TimeValue t1)=0;
		/*! \remarks Sets the sound to mute or toggles it back on. This will be called if
		the Active checkbox is toggled for example.
		\par Parameters:
		<b>BOOL mute</b>\n\n
		Specifies if the sound should be muted. TRUE indicates the sound should
		be muted; FALSE indicates the sound should be enabled. */
		virtual void SetMute(BOOL mute)=0;
		/*! \remarks Returns TRUE if the sound is muted; otherwise FALSE. */
		virtual BOOL IsMute()=0;
	};


/*! \remarks Implemented by the System.\n\n
Returns a new default sound object. This is the standard one provided by 3ds
Max. */
CoreExport SoundObj *NewDefaultSoundObj();

