/**********************************************************************
 *<
	FILE: midiman.h

	DESCRIPTION: A manager for MIDI

	CREATED BY: Rolf Berteig

	HISTORY: 6/7/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include <WTypes.h>
#include <mmsystem.h>
#include <mmreg.h>


// This is the same function type that is passed to the Win32 API midiInOpen()
// except that it returns a DWORD instead of void.
typedef DWORD(*MIDI_IN_PROC)(HMIDIIN hMidiIn,UINT wMsg, DWORD dwInstance,DWORD dwParam1, DWORD dwParam2);

// Return values from MIDI_IN_PROC
#define MIDIPROC_PROCESSED		1			// The message was processed and so no other callbacks should be called.
#define MIDIPROC_NOTPROCESSED	0			// The message was not processed.

// These can be called to open/close/start/stop the midi in device.
// The MIDI device can be opened more than once... it will only actually
// close when Close() is called as many times as Open().
CoreExport MMRESULT MIDIMan_Open(MIDI_IN_PROC proc,DWORD dwInstance,int priority=0);
CoreExport MMRESULT MIDIMan_Close(MIDI_IN_PROC proc,DWORD dwInstance);
CoreExport MMRESULT MIDIMan_Start();
CoreExport MMRESULT MIDIMan_Stop();
CoreExport BOOL MIDIMan_IsOpened();
CoreExport BOOL MIDIMan_IsStarted();

// To temporary close and stop the MIDI device use these methods. These
// will preserve the state of the Open/Close Start/Stop stack.
CoreExport void MIDIMan_Suspend();
CoreExport void MIDIMan_Resume();

// Macros to pull out peices of param1
#define MIDI_CHANNEL(a)				((a)&0x0f)
#define MIDI_EVENT(a)				((a)&0xf0)
#define MIDI_NOTENUMBER(a)			(((a)&0xff00)>>8)
#define MIDI_VELOCITY(a)			(((a)&0xff0000)>>16)
#define MIDI_PITCHBEND(a)			(((a)&0xff0000)>>16)
#define MIDI_NOTEFLOAT(a,low,high)	(float((a)-(low))/float((high)-(low)))
#define MIDI_VELFLOAT(a)            (float(a)/127.0f)
#define MIDI_BENDFLOAT(a)			(float(a)/127.0f)

// MIDI events
#define MIDI_NOTE_ON		0x90
#define MIDI_NOTE_OFF		0x80
#define MIDI_PITCH_BEND		0xe0
#define MIDI_CONTROLCHANGE	0xb0



