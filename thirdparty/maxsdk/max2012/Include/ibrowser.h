/*	
 *	IBrowser.h - Browser Manager public interface
 *
 *		contains public interface defintions for the global manager 
 *		and individual browsers
 *
 *			Copyright © Autodesk, Inc, 2000.  John Wainwright.
 *
 */

#pragma once

#include "iFnPub.h"
#include "GetCOREInterface.h"

#ifdef BLD_BROWSER
#	define BrowserExport __declspec( dllexport )
#else
#	define BrowserExport __declspec( dllimport )
#endif

class IBrowserMgr; 
class IBrowser;

// Core interface to the new MAX browser manager

// Browser Manager interface ID
#define BROWSER_MGR_INTERFACE   Interface_ID(0x65342378, 0x4c540d3b)
inline IBrowserMgr* GetBrowserMgr() { return (IBrowserMgr*)GetCOREInterface(BROWSER_MGR_INTERFACE); }

// class IBrowserMgr
// manager public Core interface 
class IBrowserMgr : public FPStaticInterface 
{
public:
	// new browser windows
	virtual IBrowser*	NewBrowser(MCHAR* rootURL, 
							BOOL showDirectory=TRUE, BOOL showContent=TRUE,
							BOOL showToolbar=TRUE, BOOL showMenu=TRUE, 
							HWND parentWindow=NULL)=0;
	// iterate browsers
	virtual int			NumBrowsers()=0;
	virtual IBrowser*	GetBrowser(int i)=0;

	// interface function IDs 
	enum { 
		newBrowser, 
		numBrowsers, 
		getBrowser, 
	}; 

}; 

// Browser interface ID
#define BROWSER_INTERFACE_DESC   Interface_ID(0x74a22be8, 0x18e117c9)
inline FPInterfaceDesc* GetBrowserInterfaceDesc() { return (FPInterfaceDesc*)GetCOREInterface(BROWSER_INTERFACE_DESC); }

// class IBrowser
// individiual browser interface 
class IBrowser : public FPMixinInterface
{
public:
	// browser control
#ifdef USE_CUSTOM_MATNAV
	virtual void	Init()=0;
#endif
	virtual void	Close()=0;
	virtual void	ShowDirectory(BOOL onOff)=0;
	virtual void	ShowContent(BOOL onOff)=0;
	virtual void	ShowMenu(BOOL onOff)=0;
	virtual void	ShowToolbar(BOOL onOff)=0;

	// navigation 
	virtual BOOL	SetRootURL(MCHAR* rootURL)=0;
	virtual BOOL	BrowseTo(MCHAR* browseURL)=0;

	// interface function IDs 
	enum { 
#ifdef USE_CUSTOM_MATNAV	// russom - 10/18/01
		init,
#endif
		close, 
		showDirectory, 
		showContent, 
		showMenu, 
		showToolbar, 
		setRootURL, 
		browseTo, 
	}; 

	// dispatch map
	BEGIN_FUNCTION_MAP
#ifdef USE_CUSTOM_MATNAV
		VFN_0(init,			Init); 
#endif
		VFN_0(close,		Close); 
		VFN_1(showDirectory,ShowDirectory,	TYPE_BOOL); 
		VFN_1(showContent,	ShowContent,	TYPE_BOOL); 
		VFN_1(showMenu,		ShowMenu,		TYPE_BOOL); 
		VFN_1(showToolbar,	ShowToolbar,	TYPE_BOOL); 
		FN_1(setRootURL,	TYPE_BOOL, SetRootURL, TYPE_STRING); 
		FN_1(browseTo,		TYPE_BOOL, BrowseTo,   TYPE_STRING); 
	END_FUNCTION_MAP 

	FPInterfaceDesc*	GetDesc() { return GetBrowserInterfaceDesc(); }

};



