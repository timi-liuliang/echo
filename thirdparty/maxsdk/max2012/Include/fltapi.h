//-----------------------------------------------------------------------------
// -------------------
// File	....:	fltapi.h
// -------------------
// Author...:	Gus Grubba
// Date	....:	September 1995
//
// History	.:	Sep, 07 1995 -	Started
//
//-----------------------------------------------------------------------------
#pragma once
#include "maxheap.h"
#include <WTypes.h>
#include "strbasic.h"

// forward declarations
class Bitmap;
class DllDir;
class Interface;

class FLTInterface: public MaxHeapOperators {

	public:

		virtual HINSTANCE	AppInst				()	= 0;
		virtual HWND		AppWnd				()	= 0;
		virtual DllDir*		AppDllDir			()	= 0;
		virtual const MCHAR* GetDir				(int i) = 0;
		virtual int			 GetMapDirCount	()	= 0;
		virtual const MCHAR* GetMapDir			(int i) = 0;
		virtual BOOL		 CreatePreview    (HWND,Bitmap**,int,int,int,float,Bitmap**,DWORD) = 0;
		virtual Interface*   GetMaxInterface	( ) = 0;

};

