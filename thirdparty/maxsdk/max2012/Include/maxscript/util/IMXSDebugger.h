/*		IMXSDebugger.h - include for MAXScript debugger
*
*			CREATED BY:	Larry Minton
*
*>	Copyright (c) 2005, All Rights Reserved.
*/

#pragma once

#include "..\ScripterExport.h"
#include "..\..\ifnpub.h"

class IMXSDebugger : public FPStaticInterface
{
public:
	ScripterExport static IMXSDebugger* GetInstance(); // return a concrete instance of this class. Will be a singleton. 

	// Open debug dialog. If breakExecution is true, breaks MAX execution. If outString is specified, it is output to the debugger output window
	virtual void OpenDialog(BOOL breakExecution, MCHAR* outString = NULL, BOOL setFocus = FALSE) = 0;

	virtual BOOL IsDialogOpen() = 0; // returns true if debug dialog is open

	virtual void CloseDialog() = 0; // closes debug dialog if open, continues MAX execution if broken

	virtual HWND GetDialogHwnd() = 0; // returns HWND of dialog

	// Do a preliminary get of the clipboard data. This will cause the clipboard to contain a copy of the text
	// in the desired format. If the owner of the clipboard data is a RichEdit control, that control does the conversion. 
	// We want to do do the conversion while running a MAX thread. The debugger suspends all MAX threads and trying to 
	// do a paste would cause a hang since the control's thread might be suspended.
	virtual void UpdateClipboardData() = 0; 

	virtual void WriteString(MCHAR* str) = 0; // output string to output window
	virtual void WriteLine(MCHAR* str) = 0; // output string to output window, appends \n if needed.

	// Break MAX execution if not already broken. If outString is specified, it is output to the debugger output window
	virtual void BreakExecution(MCHAR* outString = NULL) = 0;

	virtual void ContinueExecution() = 0; // continues MAX execution if broken

	virtual BOOL IsExecutionBroken() = 0; // returns true if execution is broken

	enum state	{	idle, 
					getvar_getvar, getvar_getval, 
					setvar_getvar, setvar_eval_compile, setvar_eval, setvar_setval, 
					eval_compile, eval,
					attempting_break
				}; // the possible debugger states
	virtual state GetState() = 0; // returns state debugger is in

	virtual DWORD GetThreadID() const = 0; // returns thread ID debugger is running in

	// get/set whether MXS script throws are allowed to break execution. If true, throws can break MAX execution. Whether it will
	// depends on whether noDebugBreak:<bool> is specified on the throw, and if not the value from GetDefaultBreakOnThrow()	
	virtual BOOL GetAllowBreakOnThrow() = 0; 
	virtual BOOL SetAllowBreakOnThrow(BOOL breakExecution) = 0;	// returns previous value

	// get/set whether MXS errors break execution. 
	virtual BOOL GetBreakOnError() = 0; 
	virtual BOOL SetBreakOnError(BOOL breakExecution) = 0; // returns previous value

	// get/set whether exceptions break execution. 
	virtual BOOL GetBreakOnException() = 0;
	virtual BOOL SetBreakOnException(BOOL breakExecution) = 0; // returns previous value

	// get/set whether MXS script throws w/o noDebugBreak:<bool> break execution
	virtual BOOL GetDefaultBreakOnThrow() = 0;
	virtual BOOL SetDefaultBreakOnThrow(BOOL breakExecution) = 0; // returns previous value

	// get/set whether MXS script throws within a catch are ignored
	virtual BOOL GetIgnoreCaughtThrows() = 0;
	virtual BOOL SetIgnoreCaughtThrows(BOOL ignore) = 0; // returns previous value

	// get/set whether errors within a catch are ignored
	virtual BOOL GetIgnoreCaughtErrors() = 0;
	virtual BOOL SetIgnoreCaughtErrors(BOOL ignore) = 0; // returns previous value

	// get/set whether exceptions within a catch are ignored
	virtual BOOL GetIgnoreCaughtExceptions() = 0;
	virtual BOOL SetIgnoreCaughtExceptions(BOOL ignore) = 0; // returns previous value

	// get/set whether can break into the debugger while MAX's quietmode is true
	virtual BOOL GetEnabledInQuietMode() = 0;
	virtual BOOL SetEnabledInQuietMode(BOOL ignore) = 0; // returns previous value

	// get/set whether can break into the debugger while net rendering
	virtual BOOL GetEnabledInNetRender() = 0;
	virtual BOOL SetEnabledInNetRender(BOOL ignore) = 0; // returns previous value

	// get/set whether debug dialog is set to topmost
	virtual BOOL GetStayOnTop() = 0;
	virtual BOOL SetStayOnTop(BOOL ignore) = 0; // returns previous value

	// get/set the # milliseconds allowed for a debugger command before automatically resuming MAX execution
	virtual DWORD GetCommandTimeoutPeriod() = 0; 
	virtual DWORD SetCommandTimeoutPeriod(DWORD millisecs) = 0; // returns previous value

	// get/set the # milliseconds allowed for a break attempt before automatically resuming MAX execution and trying to break again
	virtual DWORD GetBreakTimeoutPeriod() = 0; 
	virtual DWORD SetBreakTimeoutPeriod(DWORD millisecs) = 0; // returns previous value

	// get/set the # milliseconds allowed for garbage collection occurring in a break attempt before automatically resuming MAX execution and trying to break again
	virtual DWORD GetGCTimeoutPeriod() = 0; 
	virtual DWORD SetGCTimeoutPeriod(DWORD millisecs) = 0; // returns previous value

	// get/set the # milliseconds between break attempts
	virtual DWORD GetBreakCyclePeriod() = 0; 
	virtual DWORD SetBreakCyclePeriod(DWORD millisecs) = 0; // returns previous value

	// get/set whether global constants are displayed in stack dump
	virtual BOOL GetShowGlobalConstants() = 0;
	virtual BOOL SetShowGlobalConstants(BOOL ignore) = 0; // returns previous value

	// get/set whether to show only first frame in stack dump
	virtual BOOL GetShowFirstFrameOnly() = 0;
	virtual BOOL SetShowFirstFrameOnly(BOOL firstOnly) = 0; // returns previous value

	// get/set whether to allow potentially unsafe methods to be called from debugger. 
	virtual BOOL GetAllowUnsafeMethods() const = 0;
	virtual BOOL SetAllowUnsafeMethods(BOOL allowUnsafe) = 0; // returns previous value

	// controls whether to clear the output window when the debugger breaks.
	virtual BOOL GetClearOutputOnOpen() const = 0; // CPJ added Apr 10 2006
	virtual BOOL SetClearOutputOnOpen(BOOL doClear) = 0; // returns previous value
};

