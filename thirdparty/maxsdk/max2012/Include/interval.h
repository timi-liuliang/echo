/**********************************************************************
 *<
	FILE: interval.h

	DESCRIPTION:  Defines TimeValue and Interval Classes

	CREATED BY: Rolf Berteig

	HISTORY: created 13 September 1994
             950818 - Added methods for setting start/end individually (gus)

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#pragma once
#include "maxheap.h"
#include "coreexp.h"
#include "maxtypes.h"

/*! \sa  Advanced Topics sections on
<a href="ms-its:3dsmaxsdk.chm::/start_conceptual_overview.html#heading_08">Intervals</a>
and <a href="ms-its:3dsmaxsdk.chm::/anim_time_functions.html">Time</a>.\n\n
\par Description:
An Interval is a class that represents a length of time. It has two private
data members, start and end that are each TimeValues. A TimeValue is a single
instant in time. For more explanation see the Advanced Topics section on
<a href="ms-its:3dsmaxsdk.chm::/start_conceptual_overview.html#heading_08">Intervals</a>.
All the methods of this class are implemented by the system.\n\n
Definitions:\n\n
<b>#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)</b>\n\n
<b>#define NEVER Interval(TIME_NegInfinity, TIME_NegInfinity)</b>  */
class Interval: public MaxHeapOperators {
	private:
		TimeValue	start;
		TimeValue	end;

	public:		
		/* 
		 Constructors:
		 */		
		/*! \remarks Constructor that assigns both the start and end times of
		the interval. If TimeValue <b>e</b> is less than\n\n
		TimeValue <b>s</b> the values are swapped before they are assigned.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Specifies the start time.\n\n
		<b>TimeValue e</b>\n\n
		Specifies the end time. */
		CoreExport Interval( TimeValue s, TimeValue e );
		/*! \remarks Constructor that returns an <b>EMPTY</b> interval, i.e.
		having a start and end time equal to <b>TIME_NegInfinity</b> */
		Interval() { SetEmpty(); } 

		/*! \remarks Checks for equality between two Intervals.
		\return  Nonzero if the intervals are equal; otherwise 0. */
		int operator==( const Interval& i ) { return( i.start==start && i.end==end ); }
		/*! \remarks Return Nonzero if the TimeValue passed is greater than or
		equal to the start value and less than or equal to the end value and
		not equal to <b>TIME_NegInfinity</b>. Returns 0 otherwise.
		\par Parameters:
		<b>const TimeValue t</b>
		\return  Nonzero if the TimeValue passed is greater than or equal to
		the start value and less than or equal to the end value and not equal
		to <b>TIME_NegInfinity</b>; otherwise 0. */
		CoreExport int InInterval(const TimeValue t) const;
		/*! \remarks Returns nonzero if the interval passed is contained
		within the interval; otherwise 0.\n\n
		return InInterval( interval.Start() ) \&\& InInterval( interval.End()
		);
		\par Parameters:
		<b>const Interval interval</b>\n\n
		The interval to check.
		\return  Returns nonzero if the interval passed is contained within the
		interval; otherwise 0. */
		int InInterval(const Interval interval) const { return InInterval( interval.Start() ) && InInterval( interval.End() ); }
		/*! \remarks Returns 1 if the interval is <b>EMPTY</b>, i.e. has a
		start and end time equal to <b>TIME_NegInfinity</b>. Returns 0
		otherwise. */
		int Empty() { return (start == TIME_NegInfinity) && (end == TIME_NegInfinity); }

		/*! \remarks Sets the start and end times for the interval.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Start time for the interval.\n\n
		<b>TimeValue e</b>\n\n
		End time for the interval. */
		void Set       ( TimeValue s, TimeValue e ) { start = s; end = e; }
		/*! \remarks Sets the start value only.
		\par Parameters:
		<b>TimeValue s</b>\n\n
		Start time for the interval. */
		void SetStart  ( TimeValue s )              { start = s;          }
		/*! \remarks Sets the end value only.
		\par Parameters:
		<b>TimeValue e</b>\n\n
		End time for the interval. */
		void SetEnd    ( TimeValue e )              { end = e;            }

		/*! \remarks Sets the interval to be <b>EMPTY</b>, i.e. having a start
		and end time equal to <b>TIME_NegInfinity</b>. */
		void SetEmpty() { start = TIME_NegInfinity; end = TIME_NegInfinity; }
		/*! \remarks Sets the interval to be <b>FOREVER</b>, i.e. have a start
		time equal <b>TIME_NegInfinity</b> and end time equal to
		<b>TIME_PosInfinity</b>. */
		void SetInfinite() { start = TIME_NegInfinity; end = TIME_PosInfinity; }
		/*! \remarks Sets both the start and end times to the time passed. */
		void SetInstant(const TimeValue t) { start = end = t; }
		/*! \remarks Returns the start time of the interval. */
		TimeValue Start() const { return start; }
		/*! \remarks Returns the end time of the interval. */
		TimeValue End() const { return end; }
		/*! \remarks Implemented by the System.\n\n
		Returns the duration of the interval (end points included).
		\par Operators:
		*/
		TimeValue Duration() const { return end-start+TimeValue(1); } // end points included

		// intersection of intervals
		/*! \remarks Intersects <b>Interval</b> and i. The interval will have
		a start time of the greater of the two interval start times, and an end
		value which is the lesser of the two end values. If the end time is
		less than the start time, both the start and end times are set to
		<b>TIME_NegInfinity</b>.
		\return  An Interval that is the intersection of the intervals. */
		CoreExport Interval operator&(const Interval i) const;
		/*! \remarks This updates the invoking interval so it will have a
		start time of the greater of the two interval start times, and an end
		value which is the lesser of the two end values.\n\n
		<b>return (*this = (*this\&i));</b> */
		Interval& operator&=(const Interval i) { return (*this = (*this&i)); }
		/*! \remarks Expands the <b>Interval</b> to include the TimeValue.\n\n
		<b>if (t\<start) start=t; if (t\>end) end=t; return *this;</b> */
		Interval& operator+=(const TimeValue t) { if (t<start) start=t; if (t>end) end=t; return *this; }
};

#define FOREVER Interval(TIME_NegInfinity, TIME_PosInfinity)
#define NEVER Interval(TIME_NegInfinity, TIME_NegInfinity)



