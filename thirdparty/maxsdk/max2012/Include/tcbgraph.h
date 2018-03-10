/**********************************************************************
 *<
	FILE: TCBGraph.h

	DESCRIPTION:  UI gadget to display TCB graph

	CREATED BY: Rolf Berteig

	HISTORY: created 2/12/96

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include <WTypes.h>
#include "coreexp.h"
#include "maxheap.h"

// The class name of the UI gadget
#define TCBGRAPHCLASS _M("TCBGraph")

// Send this message to the graph control with lParam
// pointing to a TCBGraphParams structure to set the
// garph parameters
#define WM_SETTCBGRAPHPARAMS	(WM_USER + 0xbb34)

/*! \sa  <a href="ms-its:3dsmaxsdk.chm::/ui_custom_controls.html">Custom
Controls</a>.\n\n
\par Description:
The TCB Graph control displays a tension/continuity/bias graph in the
control.\n
\n \image html "tcbgraph.gif"
If you are going to use the TCB Graph control you must initialize it by
calling\n\n
<b>void InitTCBGraph(HINSTANCE hInst);</b>\n\n
from <b>DLLMain()</b>\n\n
The value to use in the Class field of the Custom Control Properties dialog is:
<b>TCBGraph</b>\n\n
Send this message to the graph control with <b>lParam</b> pointing to a
<b>TCBGraphParams</b> structure to set the graph parameters.\n\n
<b>WM_SETTCBGRAPHPARAMS</b>\n\n
For example:\n\n
<b>TCBGraphParams gp;</b>\n\n
<b>gp.tens = 0.0f; gp.bias = 0.0f;gp.cont = 0.0f;</b>\n\n
<b>gp.easeFrom = 0.0f; gp.easeTo = 0.0f;</b>\n\n
<b>HWND hGraph = GetDlgItem(hDlg, IDC_TCB_GRAPH);</b>\n\n
<b>EnableWindow(hGraph, TRUE);</b>\n\n
<b>SendMessage(hGraph,WM_SETTCBGRAPHPARAMS,0,(LPARAM)\&gp);</b>\n\n
<b>UpdateWindow(hGraph);</b>\n\n
Note that this control is not derived from <b>ICustControl</b> and thus does
not have <b>Enable()</b>, <b>Disable()</b>, etc. methods.
\par Data Members:
<b>float tens, cont, bias, easeFrom, easeTo;</b>\n\n
The tension, continuity, bias, ease from and ease to parameters. Each value may
range from 0.0 to 1.0. */
class TCBGraphParams: public MaxHeapOperators {
	public:
		float tens, cont, bias, easeFrom, easeTo;
	};

CoreExport void InitTCBGraph(HINSTANCE hInst);

