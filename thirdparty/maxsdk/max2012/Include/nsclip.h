/**********************************************************************
 *<
	FILE: nsclip.h

	DESCRIPTION: Clip board for named selection sets

	CREATED BY: Rolf Berteig

	HISTORY: 1/9/97

 *>	Copyright (c) 1997, All Rights Reserved.
 **********************************************************************/

#pragma once
#include "maxheap.h"
#include "strclass.h"
// forward declarations
class BitArray;
class ShapeVSel;
class ShapeSSel;
class ShapePSel;

// The mesh select modifier and the editable tri object need
// to be able to share the clipboard for named selection sets,
// however they are in two separate DLLs. So I've provided a 
// place in CORE for the clipboard. Note that if 3rd parties
// run into a similar problem they could always create their
// own shared DLL, I just used CORE because it was handy.

/*! \sa  Class BitArray, Template Class Tab.\n\n
\par Description:
This class enables the Mesh Select Modifier and the Editable TriObject to share
clipboard data for named selection sets.
\par Data Members:
<b>MSTR name;</b>\n\n
The name of the clipboard.\n\n
<b>Tab\<BitArray*\> sets;</b>\n\n
This table of BitArray pointers stores the selected state for the vertex, edge
and face states of the mesh.  */
class MeshNamedSelClip: public MaxHeapOperators {
	public:
		MSTR name;
		Tab<BitArray*> sets;

		/*! \remarks Constructor. The name is set to the name passed. */
		MeshNamedSelClip(MSTR &n) {name=n;}
		/*! \remarks Destructor. The sets are deleted. */
		CoreExport ~MeshNamedSelClip();
	};

#define CLIP_VERT	1
#define CLIP_FACE	2
#define CLIP_EDGE	3

/*! \remarks This global function sets the specified named selection
clipboard.
\par Parameters:
<b>MeshNamedSelClip *clip</b>\n\n
Points to the named selection clipboard to set.\n\n
<b>int which</b>\n\n
Specifies which clipboard to set. One of the following values:\n\n
<b>CLIP_VERT</b>\n\n
<b>CLIP_FACE</b>\n\n
<b>CLIP_EDGE</b> */
CoreExport void SetMeshNamedSelClip(MeshNamedSelClip *clip, int which);
/*! \remarks This global function returns a pointer to the specified named
selection clipboard.
\par Parameters:
<b>int which</b>\n\n
Specifies which clipboard to get. One of the following values:\n\n
<b>CLIP_VERT</b>\n\n
<b>CLIP_FACE</b>\n\n
<b>CLIP_EDGE</b> */
CoreExport MeshNamedSelClip *GetMeshNamedSelClip(int which);

// Here is the equivalent for patches

/*! \sa  Class BitArray, Template Class Tab.\n\n
\par Description:
This class provides a storage and access mechanism for a named patch selection
clipboard.
\par Data Members:
<b>MSTR name;</b>\n\n
The name of the clipboard.\n\n
<b>Tab\<BitArray*\> sets;</b>\n\n
This table is a  */
class PatchNamedSelClip: public MaxHeapOperators {
	public:
		MSTR name;
		Tab<BitArray*> sets;

		/*! \remarks Constructor. The name is set to the name passed. */
		PatchNamedSelClip(MSTR &n) {name=n;}
		/*! \remarks Destructor. The sets are deleted. */
		CoreExport ~PatchNamedSelClip();
	};

#define CLIP_P_VERT		1
#define CLIP_P_PATCH	2
#define CLIP_P_EDGE		3
#define CLIP_P_HANDLE	4

/*! \remarks This global function sets the specified named selection
clipboard.
\par Parameters:
<b>PatchNamedSelClip *clip</b>\n\n
Points to the named selection clipboard to set.\n\n
<b>int which</b>\n\n
Specifies which clipboard to set. One of the following values:\n\n
<b>CLIP_P_VERT</b>\n\n
<b>CLIP_P_PATCH</b>\n\n
<b>CLIP_P_EDGE</b> */
CoreExport void SetPatchNamedSelClip(PatchNamedSelClip *clip, int which);
/*! \remarks This global function returns a pointer to the specified named
selection clipboard.
\par Parameters:
<b>int which</b>\n\n
Specifies which clipboard to get. One of the following values:\n\n
<b>CLIP_P_VERT</b>\n\n
<b>CLIP_P_PATCH</b>\n\n
<b>CLIP_P_EDGE</b> */
CoreExport PatchNamedSelClip *GetPatchNamedSelClip(int which);

// Here are the equivalents for shapes:

/*! \sa  Class BitArray, Template Class Tab, Class ShapeVSel.\n\n
\par Description:
This class provides a storage and access mechanism for a named vertex selection
clipboard.
\par Data Members:
<b>MSTR name;</b>\n\n
The name of the clipboard.\n\n
<b>Tab\<ShapeVSel*\> sets;</b>\n\n
This table stores the selection data for the clipboard.  */
class ShapeNamedVertSelClip: public MaxHeapOperators {
	public:
		MSTR name;
		Tab<ShapeVSel*> sets;

		/*! \remarks Constructor. The name is set to the name passed. */
		ShapeNamedVertSelClip(MSTR &n) {name=n;}
		/*! \remarks Destructor. The sets are deleted. */
		CoreExport ~ShapeNamedVertSelClip();
	};

/*! \remarks This global function sets the vertex named selection clipboard.
\par Parameters:
<b>ShapeNamedVertSelClip *clip</b>\n\n
Points to the clipboard to set. */
CoreExport void SetShapeNamedVertSelClip(ShapeNamedVertSelClip *clip);
/*! \remarks This global function returns the vertex named selection
clipboard. */
CoreExport ShapeNamedVertSelClip *GetShapeNamedVertSelClip();

/*! \sa  Class BitArray, Template Class Tab, Class ShapeSSel.\n\n
\par Description:
This class provides a storage and access mechanism for a named segment
selection clipboard.
\par Data Members:
<b>MSTR name;</b>\n\n
The name of the clipboard.\n\n
<b>Tab\<ShapeSSel*\> sets;</b>\n\n
This table stores the selection data for the clipboard.  */
class ShapeNamedSegSelClip: public MaxHeapOperators {
	public:
		MSTR name;
		Tab<ShapeSSel*> sets;

		/*! \remarks Constructor. The name is set to the name passed. */
		ShapeNamedSegSelClip(MSTR &n) {name=n;}
		/*! \remarks Destructor. The sets are deleted. */
		CoreExport ~ShapeNamedSegSelClip();
	};

/*! \remarks This global function sets the named selection clipboard.
\par Parameters:
<b>ShapeNamedSegSelClip *clip</b>\n\n
Points to the clipboard to set. */
CoreExport void SetShapeNamedSegSelClip(ShapeNamedSegSelClip *clip);
/*! \remarks This global function returns a pointer to the named selection
clipboard. */
CoreExport ShapeNamedSegSelClip *GetShapeNamedSegSelClip();

/*! \sa  Class BitArray, Template Class Tab, Class ShapePSel.\n\n
\par Description:
This class provides a storage and access mechanism for a named vertex selection
clipboard.
\par Data Members:
<b>MSTR name;</b>\n\n
The name of the clipboard.\n\n
<b>Tab\<ShapePSel*\> sets;</b>\n\n
This table stores the selection data for the clipboard.  */
class ShapeNamedPolySelClip: public MaxHeapOperators {
	public:
		MSTR name;
		Tab<ShapePSel*> sets;

		/*! \remarks Constructor. The name is set to the name passed. */
		ShapeNamedPolySelClip(MSTR &n) {name=n;}
		/*! \remarks Destructor. The sets are deleted. */
		CoreExport ~ShapeNamedPolySelClip();
	};

/*! \remarks This global function sets the named selection clipboard.
\par Parameters:
<b>ShapeNamedPolySelClip *clip</b>\n\n
Points to the clipboard to set. */
CoreExport void SetShapeNamedPolySelClip(ShapeNamedPolySelClip *clip);
/*! \remarks This global function returns a pointer to the named selection
clipboard. */
CoreExport ShapeNamedPolySelClip *GetShapeNamedPolySelClip();


