/**********************************************************************
 *<
	FILE: imblur.h

	DESCRIPTION:  Defines Interface for Image Motion Blur.  This interface is
		implemented in the Effect plugin MotionBlur.dlv, which must be present to use it.
		This interface is does NOT support scripting, only direct calling.

	CREATED BY: Dan Silva

	HISTORY: created 7 March 2000

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "iFnPub.h"
#include "iparamb2.h"

// forward declarations
class Bitmap;
class CheckAbortCallback;

// Class ID for the motion blur Effect
#define MBLUR_CLASS_ID Class_ID(0x86c92d3, 0x601af384) 

// Interface ID for the motion blur function interface ( IMBOps) 
#define IMB_INTERFACE Interface_ID(0x2A3764C1,0x9C96F51)

// Use this to get ClassDesc2 that implements interface.
#define GET_MBLUR_CD (ClassDesc2*)GetCOREInterface()->GetDllDir().ClassDir().FindClass(RENDER_EFFECT_CLASS_ID, MBLUR_CLASS_ID)

// Use this to get IMBOps interface from the ClassDesc2.
#define GetIMBInterface(cd) (IMBOps *)(cd)->GetInterface(IMB_INTERFACE)

// Flags values
#define IMB_TRANSP  1  // controls whether motion blur works through transparency.  Setting it to 0 saves memory, runs faster.
 
//! \brief The interface for Image Motion Blur. 
/*! This interface is implemented in the Effect plug-in MotionBlur.dlv, which 
	must be present to use it. This interface is does NOT support scripting, 
	only direct calling. The sample code below shows how this is done:
	\code
	ClassDesc2* mbcd = GET_MBLUR_CD;
	if (mbcd) {
		IMBOps* imb = GetIMBInterface(mbcd);
		imb->ApplyMotionBlur(bm, &imbcb, 1.2f);
	}
	\endcode
	\sa  Class FPStaticInterface,  Class Bitmap, Class CheckAbortCallback */
class IMBOps: public FPStaticInterface {
	public: 
	/*! \remarks Sets the channels required for the image motion blur.
	\par Parameters:
	<b>ULONG flags=0</b>\n\n
	The following flag may be set:\n\n
	<b>IMB_TRANSP</b>\n\n
	Controls whether motion blur works through transparency. Setting it to 0
	saves memory, runs faster. */
	virtual ULONG ChannelsRequired(ULONG flags=0)=0;
	/*! \remarks Applies the motion blur process to the specified bitmap.
	\par Parameters:
	<b>Bitmap *bm</b>\n\n
	The bitmap to apply the motion blur to.\n\n
	<b>CheckAbortCallback *progCallback=NULL</b>\n\n
	A pointer to a callback, allowing an abort check during the progress.\n\n
	<b>float duration=1.0f</b>\n\n
	The motion blur duration.\n\n
	<b>ULONG flags=IMB_TRANSP</b>\n\n
	The following flag may be set:\n\n
	<b>IMB_TRANSP</b>\n\n
	Controls whether motion blur works through transparency. Setting it to 0
	saves memory, runs faster.\n\n
	<b>Bitmap* extraBM=NULL</b>\n\n
	If the <b>extraBM</b> bitmap is supplied, then that is used as the target
	color bitmap, but the gbuffer information still comes from the other, main,
	bitmap. this is used to apply motion blur to render lements.
	\return  TRUE if success, otherwise FALSE. */
	virtual int ApplyMotionBlur(Bitmap *bm, CheckAbortCallback *progCallback=NULL,  
		float duration=1.0f,  ULONG flags=IMB_TRANSP, Bitmap *extraBM=NULL)=0;
	};



