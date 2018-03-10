/**********************************************************************
 *<
	FILE: osnapmk.h

	DESCRIPTION:  A Class for an osnapmarker

	CREATED BY: John Hutchinson

	HISTORY: Feb 12, 1996
 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/
#pragma once
#include "coreexp.h"
#include "maxheap.h"
#include "ipoint3.h"
#include "BuildWarnings.h"
#include <WTypes.h>

// forward declarations
class GraphicsWindow;

/*! \sa  Class Osnap, Class IOsnapManager, Class GraphicsWindow, Class IPoint3.\n\n
\par Description:
This class is available in release 2.0 and later only.\n\n
This class is used for drawing Osnap markers in the viewports. The marker is
drawn as a polyline. The class maintains a cache of the points for the
polyline. There are constructors used to initialize the cache and a
<b>display()</b> method to draw the marker in the specified viewport.\n\n
The Osnap class must implement the <b>GetMarkers()</b> method which typically
returns pointers to these static instances.  */
class OsnapMarker: public MaxHeapOperators 
{
private:
	int      m_numpoints;
	IPoint3* m_ppt;
	IPoint3* m_pcache;
	IPoint3  m_cache_trans;
	int      m_cache_size;
	int*     m_edgevis;

	bool  IsCacheValidEx(IPoint3 trans, int size);
	void UpdateCache(IPoint3 trans, int size);

public:
	/*! \remarks Constructor. The cache of marker points is set to NULL and
	the number of points is set to 0. */
	CoreExport OsnapMarker();
	/*! \remarks Destructor. If any marker points have been allocated for the
	cache they are freed. */
	CoreExport ~OsnapMarker();
	/*! \remarks Constructor. This initializes the cache with the points and
	edge flags passed.
	\par Parameters:
	<b>int n</b>\n\n
	The number of points in the marker polyline.\n\n
	<b>IPoint3 *ppt</b>\n\n
	The array of points for the polyline.\n\n
	<b>int *pes</b>\n\n
	The edge state array. This is an array that indicates if the 'n-th' edge is
	one of three state:\n\n
	<b>GW_EDGE_SKIP</b>\n\n
	Nonexistent - totally invisible.\n\n
	<b>GW_EDGE_VIS</b>\n\n
	Exists and is solid.\n\n
	<b>GW_EDGE_INVIS</b>\n\n
	Exists and is hidden - shown as a dotted line.\n\n
	You may pass NULL for this array and the method will assume that the edges
	are all solid. */
	CoreExport OsnapMarker(int n, IPoint3 *ppt, int *pes);
	/*! \remarks Constructor. The marker data is initialzed from the
	<b>OsnapMarker</b> passed.
	\par Parameters:
	<b>const OsnapMarker\& om</b>\n\n
	The settings are copied from this <b>OsnapMarker</b>. */
	CoreExport OsnapMarker(const OsnapMarker& om);
	/*! \remarks Assignment operator.
	\par Parameters:
	<b>const OsnapMarker\& om</b>\n\n
	The OsnapMarker to assign. */
	CoreExport OsnapMarker& operator=(const OsnapMarker& om);
	/*! \remarks This is method is used internally to display the marker cache
	at the specified size in the specified viewport. Plugin developers need not
	call this method.
	\par Operators:
	*/
	void display(IPoint3 xyz, int markersize, GraphicsWindow *gw);
};

