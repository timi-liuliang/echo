//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// FILE:        notetrck.h
// DESCRIPTION: Note track plug-in class
// AUTHOR:      Rolf Berteig
// HISTORY:     created July 20, 1995
//**************************************************************************/
#pragma once
#include "maxheap.h"
#include "ref.h"
#include "plugapi.h"
#include "AnimPropertyID.h"
#include "TrackClipObject.h"

class NoteTrack : public ReferenceTarget {
	public:
		SClass_ID SuperClassID() {return SClass_ID(NOTETRACK_CLASS_ID);}
		RefResult AutoDelete() {return REF_SUCCEED;}
	};

class NoteAnimProperty : public AnimProperty {
	public:		
		NoteTrack *note;
		DWORD ID() {return PROPID_NOTETRACK;}

		NoteAnimProperty(NoteTrack *n=NULL) {note = n;}
		~NoteAnimProperty() {if (note) note->DeleteMe();}
	};


/*! \remarks This global function creates and returns a pointer to a new Note Track.
*/
CoreExport NoteTrack *NewDefaultNoteTrack();




// Standard note track plug-in class definitions:

// Note Key Flags
#define NOTEKEY_SELECTED	(1<<0)
#define NOTEKEY_LOCKED		(1<<1)
#define NOTEKEY_FLAGGED		(1<<2)

/*! \sa  Class NoteKeyTab, Class DefNoteTrack, Class Animatable.\n\n
\par Description:
This class contains the data for a single note of a Note track in Track %View.
This includes the time, text and flags for the notes.
\par Data Members:
<b>TimeValue time;</b>\n\n
The time of the note.\n\n
<b>MSTR note;</b>\n\n
The text of the note.\n\n
<b>DWORD flags;</b>\n\n
The note flags. One or more of the following values:\n\n
<b>NOTEKEY_SELECTED</b>\n\n
The key is selected.\n\n
<b>NOTEKEY_LOCKED</b>\n\n
The key is locked.\n\n
<b>NOTEKEY_FLAGGED</b>\n\n
The key is flagged.  */
class NoteKey: public MaxHeapOperators {
	public:
		TimeValue time;
		MSTR note;
		DWORD flags;
		
		/*! \remarks Constructor. The data members are initialized to the
		values passed. */
		NoteKey(TimeValue t,const MSTR &n,DWORD f=0) {time=t;note=n;flags=f;}
		/*! \remarks Constructor. The data members are initialized from the
		NoteKey passed. */
		NoteKey(NoteKey& n) {time=n.time;note=n.note;flags=n.flags;}

		/*! \remarks Sets the specified flags.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The flags to set. */
		void SetFlag(DWORD mask) { flags|=(mask); }
		/*! \remarks Clears (sets to zero) the specified flags.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The flags to clear. */
		void ClearFlag(DWORD mask) { flags &= ~(mask); }
		/*! \remarks Tests the specified flags. Returns TRUE if set; otherwise
		FALSE.
		\par Parameters:
		<b>DWORD mask</b>\n\n
		The flags to test. */
		BOOL TestFlag(DWORD mask) { return(flags&(mask)?1:0); }
	};

/*! \sa  Class NoteKey, Template Class Tab, Class DefNoteTrack, Class Animatable.\n\n
\par Description:
This class is table of pointers to <b>NoteKey</b> objects which store data
about a Note Track in Track %View. See <b>Template Class Tab</b> for details on
manipulating this table.  */
class NoteKeyTab : public Tab<NoteKey*> {
	public:
		/*! \remarks Destructor. Deletes all the keys in the table. */
		~NoteKeyTab() {Clear();}
		/*! \remarks Deletes all the keys in the table. */
		CoreExport void Clear();
		/*! \remarks Deletes the specified key.
		\par Parameters:
		<b>int i</b>\n\n
		The zero based index of the key to delete. */
		CoreExport void DelKey(int i);
		/*! \remarks Assignment operator.
		\par Parameters:
		<b>NoteKeyTab \&keys</b>\n\n
		The table of keys to assign. */
		CoreExport NoteKeyTab &operator=(NoteKeyTab &keys);
		/*! \remarks This method is used internally to sort the keys by time.
		\par Operators:
		*/
		CoreExport void KeysChanged();
	};

class NoteKeyClipObject : public TrackClipObject {
	public:
		NoteKeyTab tab;

		Class_ID ClassID() {return Class_ID(NOTETRACK_CLASS_ID,0);}
		SClass_ID SuperClassID() { return NOTETRACK_CLASS_ID; }
		CoreExport void DeleteThis();

		NoteKeyClipObject(Interval iv) : TrackClipObject(iv) {}
	};

/*! \sa  Class NoteKeyTab, Class Animatable.\n\n
\par Description:
This class is available in release 3.0 and later only.\n\n
This class is 3ds Max's implementation of Note Tracks. It provides
implementation for the Animatable methods that let the keys work in Track
%View. Developers use this class to access the table of keys associated with a
track. Methods of class Animatable are available to get access to this class.
*/
class DefNoteTrack : public NoteTrack {
	public:
		NoteKeyTab keys;
		
		DefNoteTrack() {}
		DefNoteTrack(DefNoteTrack &n) {keys=n.keys;}
		DefNoteTrack& operator=(DefNoteTrack &track) {keys=track.keys;return *this;}
		CoreExport void HoldTrack();

		Class_ID ClassID() {return Class_ID(NOTETRACK_CLASS_ID,0);}

		// Tree view methods from animatable
		int NumKeys() {return keys.Count();}
		TimeValue GetKeyTime(int index) {return keys[index]->time;}
		CoreExport void MapKeys(TimeMap *map,DWORD flags );
		CoreExport void DeleteKeys( DWORD flags );
		CoreExport void CloneSelectedKeys(BOOL offset);		
		CoreExport void DeleteTime( Interval iv, DWORD flags );
		CoreExport void ReverseTime( Interval iv, DWORD flags );
		CoreExport void ScaleTime( Interval iv, float s);
		CoreExport void InsertTime( TimeValue ins, TimeValue amount );
		CoreExport void AddNewKey(TimeValue t,DWORD flags);
		CoreExport int GetSelKeyCoords(TimeValue &t, float &val,DWORD flags);
		CoreExport void SetSelKeyCoords(TimeValue t, float val,DWORD flags);
		#pragma warning(push)
		#pragma warning(disable:4100)
		CoreExport int GetTrackVSpace( int lineHeight ) {return 1;}
		CoreExport BOOL CanCopyTrack(Interval iv,DWORD flags) {return 1;}
		CoreExport BOOL CanPasteTrack(TrackClipObject *cobj,Interval iv,DWORD flags) {return cobj->ClassID()==ClassID();}
		CoreExport TrackClipObject *CopyTrack(Interval iv,DWORD flags);
		CoreExport void PasteTrack(TrackClipObject *cobj,Interval iv,DWORD flags);
		CoreExport Interval GetTimeRange(DWORD flags) ;		
		CoreExport int HitTestTrack(TrackHitTab& hits,Rect& rcHit,Rect& rcTrack,float zoom,int scroll,DWORD flags );
		CoreExport int PaintTrack(ParamDimensionBase *dim,HDC hdc,Rect& rcTrack,Rect& rcPaint,float zoom,int scroll,DWORD flags );
		CoreExport void SelectKeys( TrackHitTab& sel, DWORD flags );
		CoreExport void SelectKeyByIndex(int i,BOOL sel);
		CoreExport int NumSelKeys();
		CoreExport void FlagKey(TrackHitRecord hit);
		CoreExport int GetFlagKeyIndex();		
		CoreExport BOOL IsAnimated() {return TRUE;}
		CoreExport void EditTrackParams(TimeValue t,ParamDimensionBase *dim,MCHAR *pname,HWND hParent,IObjParam *ip,DWORD flags);
		CoreExport int TrackParamsType() {return TRACKPARAMS_KEY;}
		CoreExport BOOL SupportTimeOperations() {return TRUE;}

		CoreExport IOResult Save(ISave *isave);
		CoreExport IOResult Load(ILoad *iload);

		CoreExport void DeleteThis();
		RefResult NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget,
	         PartID& partID, RefMessage message) {return REF_SUCCEED;}
		#pragma warning(pop)
		CoreExport RefTargetHandle Clone(RemapDir &remap);
	};



