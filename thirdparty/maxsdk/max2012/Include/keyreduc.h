/**********************************************************************
 *<
	FILE: keyreduc.h

	DESCRIPTION:  Key reduction

	CREATED BY: Rolf Berteig

	HISTORY: created 9/30/95

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "interval.h"

// forward declarations
class Control;

#define DEFULAT_KEYREDUCE_THRESHOLD		(0.5f)

// Values returned from Progress
#define KEYREDUCE_ABORT		-1		// Stops processing and undoes any key reduction
#define KEYREDUCE_STOP		0		// Stops processing, but keeps any reduction done so far
#define KEYREDUCE_CONTINUE	1		// Keeps going.

// A callback so progress can be made during key reduction
/*! \sa  <a href="ms-its:listsandfunctions.chm::/idx_R_function_applykeyreducti.html">Function
ApplyKeyReduction</a>.\n\n
\par Description:
A callback object so the progress of key reduction can be checked and
controlled. This class is used in conjunction with function
<b>ApplyKeyReduction</b>.  */
class KeyReduceStatus: public MaxHeapOperators {
	public:
		virtual ~KeyReduceStatus()	{}
		// Called once before reduction starts. 'total' is the number
		// reduction canidate keys.
		/*! \remarks This method is called once before key reduction starts. It passes in
		the total number of keys that may potentially be reduced.
		\par Parameters:
		<b>int total</b>\n\n
		The total number of reduction candidate keys. */
		virtual void Init(int total)=0;

		// Called every now and again. 'p' is the number of keys
		// processed. So % done is p/total * 100.		
		/*! \remarks This is called every so often during key reduction. The number of keys
		process so far is passed in, so the percentage complete can be
		calculated as <b>p/total * 100</b>. This method should return a value
		to indicate if processing should proceed.
		\par Parameters:
		<b>int p</b>\n\n
		The number of keys processed so far.
		\return  Return one of the following values to indicate if processing
		should continue.\n\n
		<b>KEYREDUCE_ABORT</b>\n\n
		Stops processing and undoes any key reduction.\n\n
		<b>KEYREDUCE_STOP</b>\n\n
		Stops processing, but keeps any reduction done so far.\n\n
		<b>KEYREDUCE_CONTINUE</b>\n\n
		Keeps going. */
		virtual int Progress(int p)=0;
	};

// Attempts to delete keys that lie within the given time range.
// The controller will be sampled within the range in 'step' size
// increments. After the key reduction, the controller's values
// at each step are gauranteed to be withen 'threshold' distance
// from their original values.
//
CoreExport int ApplyKeyReduction(
		Control *cont,Interval range,float thresh,TimeValue step,
		KeyReduceStatus *status);



