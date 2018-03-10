//
// Copyright 2009 Autodesk, Inc.  All rights reserved. 
//
// This computer source code and related instructions and comments are the
// unpublished confidential and proprietary information of Autodesk, Inc. and
// are protected under applicable copyright and trade secret law.  They may
// not be disclosed to, copied or used by any third party without the prior
// written consent of Autodesk, Inc.
#pragma once
#include "tab.h"
#include "point3.h"

// Pre defined Tab Containers
typedef Tab<int>   IntTab;
typedef Tab<float> floatTab;
typedef Tab<float> FloatTab;
typedef Tab<DWORD> DWTab;
typedef Tab<Point3> Point3Tab;
/*! \sa  Class Tab, Class AdjEdgeList.\n\n
\par Description:
This class is simply a table of DWORDs (32-bit values.) */
class DWORDTab : public Tab<DWORD> {};