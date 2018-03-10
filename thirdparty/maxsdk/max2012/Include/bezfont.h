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
// FILE:        bezfont.h
// DESCRIPTION: Bezier Font Support methods
// AUTHOR:      Tom Hudson
// HISTORY:     Created 1 November 1995
//**************************************************************************/

#pragma once

#include "maxheap.h"
#include <WTypes.h>
#include "coreexp.h"
#include "strclass.h"

// Forward references
class BezFontManager;
class ClassDesc;
class DllDir;
class BezierShape;

class BezFontMetrics: public MaxHeapOperators {
    public:
	   	LONG   Height;
	    LONG   Ascent;
	    LONG   Descent;
	    LONG   InternalLeading;
	    LONG   ExternalLeading;
	    LONG   AveCharWidth;
	    LONG   MaxCharWidth;
	    LONG   Weight;
	    LONG   Overhang;
	    LONG   DigitizedAspectX;
	    LONG   DigitizedAspectY;
	    BCHAR  FirstChar;
	    BCHAR  LastChar;
	    BCHAR  DefaultChar;
	    BCHAR  BreakChar;
	    BYTE   Italic;
	    BYTE   Underlined;
	    BYTE   StruckOut;
	    BYTE   PitchAndFamily;
	    BYTE   CharSet;
	    DWORD  Flags;
	    UINT   SizeEM;
	    UINT   CellHeight;
	    UINT   AvgWidth;
		CoreExport BezFontMetrics() {}	// To Do; Fill in fields with reasonable values
		CoreExport BezFontMetrics(NEWTEXTMETRIC *from);
	};

// BezFontInfo type
#define BEZFONT_TRUETYPE	0
#define BEZFONT_OTHER		1

// BezFontInfo flags
// None currently defined

class BezFontInfo: public MaxHeapOperators {
	public:
		MSTR name;
		MSTR style;
		int type;		// See above
		DWORD flags;	// See above
		BezFontMetrics metrics;
		BezFontInfo() {}
		BezFontInfo(MSTR n, MSTR s, int t, DWORD f, BezFontMetrics &m) { name=n; style=s; type=t; flags=f; metrics=m; }
		CoreExport BezFontInfo &operator=(BezFontInfo &from);
	};

// A class for Dlls to use for info that will be sent back to them at load time
class DllData: public MaxHeapOperators {
	public:
		DllData() {}
	};

// This is a callback class which is used to process font enumerations
class BezFontEnumProc: public MaxHeapOperators {
	public:
		virtual ~BezFontEnumProc() {}
		virtual BOOL CALLBACK Entry(BezFontInfo &info, LPARAM userInfo)=0;
	};

// A special enumerator for the font manager
class BezFontMgrEnumProc: public MaxHeapOperators {
	public:
		BezFontManager *mgr;
		CoreExport BOOL CALLBACK Entry(BezFontInfo &info, LPARAM userInfo, DllData *dllData);
		void SetMgr(BezFontManager *m) { mgr = m; }
	};

// A class for listing font input dlls
class BezFontDll: public MaxHeapOperators {
	public:
		ClassDesc* dll;
		BezFontDll() { dll = NULL; }
		BezFontDll(ClassDesc* d) { dll = d; }
	};

typedef Tab<BezFontDll *> BezFontDllTab;

// A class for providing access to required Max internals
class FontMgrInterface: public MaxHeapOperators {
     public:
		virtual ~FontMgrInterface() {}
		virtual HINSTANCE	AppInst() = 0;
		virtual HWND		AppWnd() = 0;
		virtual DllDir*		AppDllDir() = 0;
    	virtual int			GetFontDirCount() = 0;
		virtual const MCHAR* GetFontDir (int i) = 0;
	};

typedef int BEZFONTHANDLE;

// A class used for listing the fonts we currently have available
class AvailableFont: public MaxHeapOperators {
	public:
		BezFontInfo info;
		DllData *dllData;
		INT_PTR dllIndex;	// The index of the DLL which provides this font (in BezFontDllTab)
		AvailableFont() { dllData = NULL; }
		AvailableFont(BezFontInfo &i, INT_PTR di, DllData *dd=NULL) { info=i; dllIndex=di; dllData=dd; }
		CoreExport ~AvailableFont();
	};

typedef Tab<AvailableFont *> AvailableFontTab;

// The basic bezier font class
class BezFont: public MaxHeapOperators {
	public:
		CoreExport BezFont() { }
		CoreExport virtual ~BezFont();
		CoreExport virtual void EnumerateFonts(BezFontMgrEnumProc& proc, LPARAM userInfo)=0;
		CoreExport virtual int OpenFont(MSTR name, DWORD flags, DllData *dllData)=0;
		CoreExport virtual void CloseFont()=0;
		CoreExport virtual BOOL BuildCharacter(UINT index, float height, BezierShape& shape, float& width, int fontShapeVersion=1)=0;
	};

// A class used to list the fonts currently open
class OpenBezFont: public MaxHeapOperators {
	public:
		int index;			 	// The index in AvailableFont
		DWORD flags;			// The style flags
		BEZFONTHANDLE handle;	// The handle we know it by
		int count;				// The number of users
		BezFont *loader;		// The loader for the font
		OpenBezFont() {}
		OpenBezFont(int i, DWORD f, BEZFONTHANDLE h, BezFont *l) { index=i; flags=f; handle=h; count=1; loader=l; }
		~OpenBezFont();
	};

typedef Tab<OpenBezFont *> OpenBezFontTab;

// This is the interface into Bezier fonts within the MAX system.
// This includes TrueType fonts and any other fonts supported via
// plugins.

// OpenFont flags
// None currently defined

class BezFontManager: public MaxHeapOperators {
	friend class BezFontMgrEnumProc;

	private:
		FontMgrInterface *iface;
		BOOL initialized;
		BezFontDllTab dllTab;
		AvailableFontTab available;
		OpenBezFontTab open;
		BezFontMgrEnumProc enumProc;			// What we use to get the available fonts
	public:
		CoreExport BezFontManager();
		CoreExport ~BezFontManager();
		CoreExport void SetInterface(FontMgrInterface *i) { iface = i; }
		CoreExport void Init();
		CoreExport void Uninit();
		CoreExport void Reinit();
		CoreExport void EnumerateFonts(BezFontEnumProc &proc, LPARAM userInfo);
		CoreExport BOOL FontExists(MSTR name);			// Returns TRUE if the font is available
		CoreExport BEZFONTHANDLE OpenFont(MSTR name, DWORD flags);
		CoreExport BOOL CloseFont(BEZFONTHANDLE handle);	// Returns TRUE if the font is still in use
		CoreExport BOOL BuildCharacter(BEZFONTHANDLE handle, UINT index, float height, BezierShape& shape, float &width, int fontShapeVersion=1);
		CoreExport FontMgrInterface *GetInterface() { return iface; }
		CoreExport BOOL GetFontInfo(MSTR name, BezFontInfo &info);
	};

extern CoreExport BezFontManager theBezFontManager;

