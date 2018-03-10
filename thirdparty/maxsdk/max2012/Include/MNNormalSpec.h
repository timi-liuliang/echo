/**********************************************************************
 *<
	FILE: MNNormalSpec.h

	DESCRIPTION:  User-specifiable normals for MNMeshes - Luna task 747

	CREATED BY: Steve Anderson

	HISTORY: created January 2002

 *>	Copyright (c) 2002 Autodesk, Inc., All Rights Reserved.
 **********************************************************************/
#pragma once

#include "export.h"
#include "maxheap.h"
#include "baseinterface.h"
#include "ipipelineclient.h"
#include "bitarray.h"
#include "mncommon.h"
#include "box3.h"

// forward declarations
class MNFace;
class MNMesh;
class Point3;
class GraphicsWindow;
class HitRegion;
class SubObjHitList;

#define MN_NORMAL_SPEC_INTERFACE Interface_ID(0x7b7c2c5f, 0xf94260f)

/*! <b>class MNNormalFace</b>   This class is available in release 5.0 and
later only.\n\n
This class is a face used to store specified normal information for a given
face. An array of these faces is used as a data member of class MNNormalSpec,
in much the same way as an MNMapFace array is a data member of class MNMap.
*/
class MNNormalFace: public MaxHeapOperators {
	int mDegree;
	int *mpNormalID;
	BitArray mSpecified;

public:
	/*! \remarks Constructor - initializes mDegree to 0, mpNormalID to
	NULL.\n\n
	  */
	MNNormalFace () : mDegree(0), mpNormalID(NULL) { }
	/*! \remarks Constructor - initializes to the degree specified.\n\n
	  */
	DllExport MNNormalFace (int degree);
	/*! \remarks Destructor - frees data members.\n\n
	  */
	~MNNormalFace () { Clear(); }
	/*! \remarks Initializes data members. This is useful in situations where
	the default constructor may not have been properly called, such as\n\n
	MNNormalFace *fc = new MNNormalFace[10];\n\n
	It should not be called on an MNNormalFace with data allocated, or that
	memory will leak.\n\n
	  */
	DllExport void Init();
	/*! \remarks Frees all allocated data, sets degree to 0.\n\n
	  */
	DllExport void Clear();

	// Data accessors
	/*! \remarks Returns the face's degree.\n\n
	  */
	int GetDegree() { return mDegree; }
	/*! \remarks Sets the face's degree.*/
	DllExport void SetDegree (int degree);
	// Low-level - do not use mSpecified data!
	/*! \remarks Accessor for normal in a particular corner of the face.\n\n

	\par Parameters:
	<b>int corner</b>\n\n
	The (zero-based) index of the corner of the face. (A quad face has corners
	0, 1, 2, and 3.)\n\n

	\return  Index of normal (in parent MNNormalSpec's normal array), or -1 if
	"corner" is out of range.\n\n
	  */
	int GetNormalID(int corner) { return (mpNormalID && (corner<mDegree)) ? mpNormalID[corner] : -1; }
	/*! \remarks Sets the normal ID used in a particular corner.\n\n

	\par Parameters:
	<b>int corner</b>\n\n
	The (zero-based) index of the corner of the face. (A quad face has corners 0,
	1, 2, and 3.)\n\n
	<b>int norm</b>\n\n
	The index of the normal (in the parent MNNormalSpec's normal array).*/
	void SetNormalID (int corner, int norm) { if (mpNormalID && (corner<mDegree)) mpNormalID[corner] = norm; }

	///  \brief  Return a pointer to the internal array of normal IDs.  This array has length == GetDegree().  
	int *GetNormalIDArray () { return mpNormalID; }

	/*! \remarks Indicates whether the normal used in a particular corner is
	specified or not.\n\n

	\par Parameters:
	<b>int corner</b>\n\n
	The (zero-based) index of the corner of the face. (A quad face has\n\n
	corners 0, 1, 2, and 3.)\n\n
	  */
	bool GetSpecified (int corner) { return (mpNormalID && (corner<mDegree) && mSpecified[corner]) ? true : false; }
	/*! \remarks Controls whether the normal used in a particular corner of
	the face is specified or not.\n\n

	\par Parameters:
	<b>int corner</b>\n\n
	The (zero-based) index of the corner of the face. (A quad face has corners
	0, 1, 2, and 3.)\n\n

	\par Parameters:
	<b>bool value</b>\n\n
	Whether the corner should have a specified (true) or unspecified (false)
	normal.\n\n
	  */
	void SetSpecified (int corner, bool value=true) { if (mpNormalID && (corner<mDegree)) mSpecified.Set (corner, value); }

	///  \brief  Set all normals on this face to be specified. 
	void SpecifyAll (bool value=true) { if (value) mSpecified.SetAll(); else mSpecified.ClearAll(); }
	/*! \remarks Specifies that a particular corner of the face should use a
	given normal index.\n\n

	\par Parameters:
	<b>int corner</b>\n\n
	The (zero-based) index of the corner of the face. (A quad face has corners
	0, 1, 2, and 3.)\n\n

	\par Parameters:
	<b>int norm</b>\n\n
	The index of the normal (in the parent MNNormalSpec's normal array).\n\n
	  */
	DllExport void SpecifyNormalID (int corner, int norm);

	DllExport void MakePoly (int degree, int *pNormalID);
	/*!   \remarks Inserts space for more normals into an existing face.
	(Designed to be used in conjunction with MNFace::Insert, to increase the
	size of a polygon and add new vertices \& normals.)
	\par Parameters:
	<b>int pos</b>\n\n
	The location within the face where the new normals will be added.\n\n

	\par Parameters:
	<b>int num=1</b>\n\n
	The number of new normals to add.\n\n
	  */
	DllExport void Insert (int pos, int num=1);
	/*!   \remarks Deletes normals from this map face. (Designed to be
	used in conjunction with MNFace::Delete, to decrease the size of a polygon
	and remove vertices \& normals.).
	\par Parameters:
	<b>int pos</b>\n\n
	The location within the face where the new normals will be deleted.\n\n

	\par Parameters:
	<b>int num=1</b>\n\n
	The number of new normals to delete.\n\n
	  */
	DllExport void Delete (int pos, int num=1);
	/*!   \remarks Deletes normals from this map face. (Designed to be
	used in conjunction with MNFace::Delete, to decrease the size of a polygon
	and remove vertices \& normals.).
	\par Parameters:
	<b>int newstart</b>\n\n
	The new first normal\n\n
	  */
	DllExport void RotateStart (int newstart);
	/*!   \remarks Reverses order of normals, effectively inverting the
	face. (Designed to be used in conjunction with MNFace::Flip.)\n\n
	  */
	DllExport void Flip ();	// Reverses order of verts.  0 remains start.

	/*!   \remarks Typical = operator - calls SetDegree to make this face
	the same size as "from", then copies the specification data and the
	normalIDs.\n\n
	  */
	DllExport MNNormalFace & operator= (const MNNormalFace & from);
	/*!   \remarks Sets the degree of this NormalFace to that of the
	MNFace given.\n\n
	  */
	DllExport MNNormalFace & operator= (const MNFace & from);
	/*!   \remarks "Shallow-copies" data from "from". This is dangerous
	to use - the pointer to the normal ID array is the same in both faces after
	this is called.\n\n
	It's typically used by MNNormal::FAlloc, when resizing the whole face
	array.\n\n
	  */
	DllExport void ShallowTransfer (MNNormalFace & from);
	/*!   \remarks Uses "DebugPrint" to output information about this
	MNNormalFace to the Debug buffer in DevStudio. Output is formatted as
	follows: suppose we have a 5-sided face, with normals {4,5,6,7,8}, and only
	the normals in corners 1 and 2 (i.e. normals 5 and 6) are specified.\n\n
	MNDebugPrint (true) would generate:\n\n
	_4 5 6 _7 _8\n\n
	MNDebugPrint (false) would generate:\n\n
	_ 5 6 _ _\n\n
	This is mainly used as part of MNNormalSpec::MNDebugPrint.\n\n
	  */
	DllExport void MNDebugPrint (bool printAll=false);

	/*!   \remarks Called by the system. Saves the face's data to the
	stream given.\n\n
	  */
	DllExport IOResult Save (ISave *isave);
	/*!   \remarks Called by the system. Loads the face's data from the
	stream given. */
	DllExport IOResult Load (ILoad *iload);
};

// Class MNNormalSpec flags:
#define MNNORMAL_NORMALS_BUILT 0x01
#define MNNORMAL_NORMALS_COMPUTED 0x02
#define MNNORMAL_DISPLAY_HANDLES 0x04
#define MNNORMAL_FACE_ANGLES 0x08	// Used face angles last time we computed normals

// Default length for normal display
#define MNNORMAL_LENGTH_DEFAULT 10.0f

/*! \sa  Class MNMESH , Class MNNormalFace\n\n
\par Description:
This class is available in release 5.0 and later only.\n\n
This class is an interface used to store user-specified normals\n\n
(as created in the Edit Normals modifier). These normals have very\n\n
limited pipeline support. They are used for viewport display, but not\n\n
for rendering.\n\n
The MNNormalSpec contains three types of normals:\n\n
- <b>Unspecified</b> - these are the usual normals that are computed from\n\n
smoothing groups. All normals are unspecified by default. \n\n
- <b>Specified</b> - these are normals that are intended for use by
particular\n\n
corners of particular faces, without regard to smoothing groups. For \n\n
instance, you can create a box, apply Edit Normals, select a group of \n\n
normals at a particular vertex, and click "Unify". Now those three \n\n
faces are told to specifically use that one unified normal, and they \n\n
ignore their smoothing groups at that vertex (which would normally \n\n
tell them they should each have their own normal). \n\n
- <b>Explicit</b> - these are normals that are set to particular values.\n\n
For instance, if the user wants to use the Edit Normals Move or
Rotate \n\n
commands to set a normal to something other than its default value, \n\n
it has to be made explicit, so it won't be recomputed based on the \n\n
face normals. All explicit normals are also considered to be specified..\n\n

\par Flags:
<b>MNNORMAL_NORMALS_BUILT</b>\n\n
Indicates that non-specified normals have been constructed using\n\n
smoothing groups. If not set, non-specified normals may be invalid.\n\n
<b>MNNORMAL_NORMALS_COMPUTED</b>\n\n
Indicates that non-explicit normals have been computed using geometrically\n\n
computed face normals. (If not set, only explicit normals may be assumed\n\n
to be pointing the right direction.)
\par Data Members:
All data members are private.\n\n
<b>int mNumNormalAlloc, mNumFaceAlloc;</b>\n\n
The current allocation length of the mpNormal and mpFace arrays.\n\n
<b>int mNumNormals, mNumFaces;</b>\n\n
The number of normals and faces in the mpNormal and mpFace arrays.\n\n
(May be less than the actual allocation above.)\n\n
<b>MNNormalFace *mpFace;</b>\n\n
The array of normal faces.\n\n
<b>Point3 *mpNormal;</b>\n\n
The array of normals, all of which should be either length 1 or (occasionally)
0.\n\n
<b>BitArray mNormalExplicit;</b>\n\n
Indicates whether mpNormal[i] is explicit or computed from face normals.\n\n
<b>BitArray mNormalSel;</b>\n\n
Current normal selection.\n\n
<b>float mDisplayLength;</b>\n\n
The length to use when displaying, hit testing, or moving normals.\n\n
<b>MNMesh *mpParent;</b>\n\n
A pointer to the "parent" MNMesh that owns this MNNormalSpec. This parent\n\n
information is required for some operations, such as display. (Such\n\n
operations should indicate below where parent information is required.)  : <br>
   */
class MNNormalSpec : public IPipelineClient, public FlagUser {
private:
	int mNumNormalAlloc, mNumFaceAlloc;
	int mNumNormals, mNumFaces;
	MNNormalFace *mpFace;
	Point3 *mpNormal;
	BitArray mNormalExplicit;	// Indicates whether mpNormal[i] is explicit or computed from face normals.

	// Display and selection data:
	BitArray mNormalSel;
	float mDisplayLength;

	// We also maintain a pointer to the parent MNMesh
	// (NOTE that the MNMesh MUST keep this pointer updated at all times!)
	MNMesh *mpParent;

public:
	/*!   \remarks Constructor. Initializes all data members.\n\n
	  */
	MNNormalSpec () : mpFace(NULL), mpNormal(NULL), mNumNormalAlloc(0),
		mNumFaceAlloc(0), mNumNormals(0), mNumFaces(0), mpParent(NULL),
		mDisplayLength(MNNORMAL_LENGTH_DEFAULT) { }
	/*!\remarks Destructor. Calls ClearAndFree().*/
	~MNNormalSpec () { ClearAndFree (); }

	// Initialization, allocation:
	/*!   \remarks Initializes all data members. Do not call if memory
	has already been\n\n
	allocated, or that memory will be leaked.\n\n
	  */
	DllExport void Initialize ();	// Initializes all data members - do not use if already allocated!
	/*!   \remarks Sets the size of the normal array\n\n

	\return  True if successful; false indicates a failed memory
	allocation.\n\n
	  */
	DllExport bool NAlloc (int num, bool keep=TRUE);
	/*!   \remarks Reduces the allocation size down to the actual number
	of normals.\n\n
	  */
	DllExport void NShrink ();	// shrinks allocation down to actual number of normals.
	/*!   \remarks Sets the size of the face array.\n\n

	\return  True if successful; false indicates a failed memory
	allocation.\n\n
	  */
	DllExport bool FAlloc (int num, bool keep=TRUE);
	/*!   \remarks Reduces the allocation size down to the actual number
	of faces.\n\n
	  */
	DllExport void FShrink ();
	/*!   \remarks Clears out all data, but doesn't necessarily free
	array memory.\n\n
	  */
	DllExport void Clear ();	// Deletes everything.
	/*!   \remarks Clears out all data and frees all memory.\n\n
	  */
	DllExport void ClearAndFree ();	// Deletes everything, frees all memory

	// Data access:
	// Lowest level:
	/*!   \remarks Returns the current number of faces in the
	MNNormalSpec\n\n
	  */
	int GetNumFaces () const { return mNumFaces; }
	/*!   \remarks Sets the current number of faces in the
	MNNormalSpec,\n\n
	increasing the allocation size as needed.\n\n

	\return  True if successful; false indicates a failed memory
	allocation.\n\n
	  */
	DllExport bool SetNumFaces (int numFaces);
	/*!   \remarks Returns the current number of normals in the
	MNNormalSpec\n\n
	  */
	int GetNumNormals () const { return mNumNormals; }
	/*!   \remarks Sets the current number of normals in the
	MNNormalSpec,\n\n
	increasing the allocation size as needed.\n\n

	\return  True if successful; false indicates a failed memory
	allocation.\n\n
	  */
	DllExport bool SetNumNormals (int numNormals);

	/*!   \remarks Returns the normal indicated. Since it returns a
	reference,\n\n
	you can use it as a set method as well:\n\n
	Normal(i) = Normalize (Point3(1,1,0));\n\n
	(Note that all normals should be normalized to a length of 1.)\n\n
	  */
	Point3 & Normal (int normID) const { return mpNormal[normID]; }
	/*!   \remarks Returns a pointer to the whole normal array.\n\n
	  */
	Point3 * GetNormalArray () const { return mpNormal; }
	/*!   \remarks Indicates whether a given normal is explicit or
	not.\n\n
	  */
	bool GetNormalExplicit (int normID) const { return mNormalExplicit[normID] ? true : false; }
	/*!   \remarks Sets a particular normal to be explicit or not. Note
	that if\n\n
	you make a normal non-explicit, it may need to be recomputed,\n\n
	so you may want to call ComputeNormals or at least clear the\n\n
	MNNORMAL_NORMALS_COMPUTED flag.\n\n

	\par Parameters:
	<b>int normID</b>\n\n
	The index of the normal\n\n
	<b>bool value</b>\n\n
	True to make the normal explicit; false to make it non-explicit.\n\n
	  */
	void SetNormalExplicit (int normID, bool value) { mNormalExplicit.Set (normID, value); }

	///  \brief  Set all normals in this MNNormalSpec to be explicit. 
	void SetAllExplicit (bool value=true) { if (value) mNormalExplicit.SetAll(); else mNormalExplicit.ClearAll (); }
	/*!   \remarks Returns the indicated face.\n\n
	  */
	MNNormalFace & Face(int faceID) const { return mpFace[faceID]; }
	/*!   \remarks Returns a pointer to the whole face array.\n\n
	  */
	MNNormalFace * GetFaceArray () const { return mpFace; }

	/*!   \remarks Tells the MNNormalSpec what MNMesh "owns" it.\n\n
	This "Parent" MNMesh is used in methods such as\n\n
	Display, Hit-Testing, and certain operations like\n\n
	Unify to get information about the vertices that\n\n
	normals are based on. (There's no vertex info\n\n
	in the MNNormalSpec itself.)\n\n
	If you have an isolated MNNormalSpec which doesn't\n\n
	really have an associated "parent", you can\n\n
	temporarily set this to a mesh with the right sort\n\n
	of faces and vertices, but you should clear it\n\n
	afterwards by calling SetParent (NULL). See the\n\n
	Edit Normals modifier source in\n\n
	maxsdk/SAMPLES/mesh/EditablePoly/EditNormals.cpp\n\n
	for an example of this sort of usage.\n\n
	  */
	void SetParent (MNMesh *pMesh) { mpParent = pMesh; }

	// Data access - higher level:
	/*!   \remarks Returns the normal used by the indicated face, in the
	indicated corner.\n\n
	  */
	DllExport Point3 & GetNormal (int face, int corner);
	/*!   \remarks Creates a new (explicit) normal and uses it in the
	indicated\n\n
	corner of the indicated face. If "normal" is not already\n\n
	normalized, this method will take care of it.\n\n
	  */
	DllExport void SetNormal (int face, int corner, Point3 & normal);
	/*!   \remarks Returns the index of the normal used in the indicated
	corner\n\n
	of the indicated face.\n\n
	  */
	DllExport int GetNormalIndex (int face, int corner);
	/*!   \remarks Sets the index of the normal used in the indicated
	corner of\n\n
	the indicated face, and marks it as specified.\n\n
	  */
	DllExport void SetNormalIndex (int face, int corner, int normalIndex);
	/*!   \remarks Creates a new normal at the end of the normal
	array.\n\n

	\par Parameters:
	<b>Point3 \& normal</b>\n\n
	The desired normal direction. Will be normalized to a length\n\n
	of 1 by the method if needed.\n\n
	<b>bool explic=true</b>\n\n
	Indicates whether the new normal should be considered explicit\n\n
	or not.\n\n
	  */
	DllExport int NewNormal (Point3 & normal, bool explic=true);

	/*!   \remarks Sets the current normal selection.\n\n
	  */
	DllExport void SetSelection (BitArray & newSelection);
	/*!   \remarks Returns the current normal selection.\n\n
	  */
	BitArray & GetSelection() { return mNormalSel; }
	/*!   \remarks Sets the current length used for normal display,
	hit-testing, and Translations.\n\n
	  */
	void SetDisplayLength (float displayLength) { mDisplayLength = displayLength; }
	/*!   \remarks Returns the current length used for normal display,
	hit-testing, and Translations.\n\n
	  */
	float GetDisplayLength () { return mDisplayLength; }

	/*!   \remarks Requires an accurate "parent" pointer (see
	SetParent).\n\n
	This method is used in conjunction with the parent MNMesh's\n\n
	CollapseDeadFaces method to keep the normal faces in synch with\n\n
	the parent MNMesh faces. It removes any normal face whose\n\n
	equivalent face in the parent mesh is considered "Dead".\n\n
	Called by MNMesh::CollapseDeadFaces, so you generally don't\n\n
	need or want to call it separately.\n\n
	  */
	DllExport void CollapseDeadFaces ();	// Requires an accurate mpParent pointer.

	// Display and hit testing - note that these require an accurate mpParent pointer.
	/*!   \remarks Requires an accurate "parent" pointer (see
	SetParent).\n\n
	Displays the current normals in the graphics window indicated.\n\n
	If "showSel" is true, selected normals are displayed in the\n\n
	usual subobject selection color.\n\n
	  */
	DllExport void Display (GraphicsWindow *gw, bool showSel);
	/*!   \remarks Requires an accurate "parent" pointer (see SetParent).
	Hit-tests on the current normals.\n\n

	\par Parameters:
	<b>GraphicsWindow *gw</b>\n\n
	The window to hit-test in.\n\n
	<b>HitRegion *hr</b>\n\n
	A hit region, typically generated by a call like MakeHitRegion(hr,type,
	crossing,4,p);\n\n
	<b>DWORD flags</b>\n\n
	Hit testing flags. Please see BaseObject::HitTest for a description of\n\n
	these flags and of the "type" and "crossing" variables used to generate\n\n
	the HitRegion.\n\n
	<b>SubObjHitList \& hitList</b>\n\n
	Where the hits get stored.\n\n

	\return  True if a hit was found; false if not.\n\n
	  */
	DllExport bool HitTest (GraphicsWindow *gw, HitRegion *hr, DWORD flags, SubObjHitList& hitList);
	/*!   \remarks Requires an accurate "parent" pointer (see SetParent).
	Computes the bounding box of the normals.\n\n

	\par Parameters:
	<b>Matrix3 tm=NULL</b>\n\n
	An optional transform for computing the bounding box in a different\n\n
	space (such as world space).\n\n
	<b>bool selectedOnly=false</b>\n\n
	Indicates whether all normals should be included in the bounding box,\n\n
	or only selected ones.\n\n
	  */
	DllExport Box3 GetBoundingBox (Matrix3 *tm=NULL, bool selectedOnly=false);

	// This method dumps all unspecified normals.  Best to use only from within CheckNormals.
	/*!   \remarks This method dumps all unspecified normals. Best to use
	only from within BuildNormals,\n\n
	since it leaves all unspecified normals in faces initialized to -1.\n\n
	  */
	DllExport void ClearNormals ();

	// Fills in the mpSpecNormal data by building all the unspecified normals,
	// and computing non-explicit ones.
	// Does nothing if normal faces not allocated yet!
	// Requires an accurate mpParent pointer.
	/*!   \remarks Requires an accurate "parent" pointer (see
	SetParent).\n\n
	Fills in the mpSpecNormal data by building all the unspecified normals,\n\n
	and computing non-explicit ones. Does nothing if face array is not\n\n
	allocated yet!\n\n
	  */
	DllExport void BuildNormals ();

	// This method just recomputes the directions of non-explicit normals,
	// without rebuilding the normal list.
	// Requires an accurate mpParent pointer.
	/*!   \remarks Requires an accurate "parent" pointer (see
	SetParent).\n\n
	This method just recomputes the directions of non-explicit normals,\n\n
	without rebuilding the normal list.\n\n
	  */
	DllExport void ComputeNormals ();

	// This checks our flags and calls BuildNormals, ComputeNormals as needed.
	// Requires an accurate mpParent pointer.
	/*!   \remarks Requires an accurate "parent" pointer (see
	SetParent).\n\n
	This checks our flags and calls BuildNormals or ComputeNormals as
	needed.\n\n
	  */
	DllExport void CheckNormals ();

	// operators and debug printing
	/*!   \remarks Typical = operator. Allocates arrays in this, and
	makes copies of all\n\n
	the data in "from". Does NOT copy "Parent" pointer.\n\n
	  */
	DllExport MNNormalSpec & operator= (const MNNormalSpec & from);
	/*!   \remarks This is similar to operator=, but copies only the
	specified and explicit\n\n
	information from "from". Result will need to have BuildNormals and
	ComputeNormals\n\n
	called.\n\n
	  */
	DllExport void CopySpecified (const MNNormalSpec & from);	// Like operator=, but omits unspecified.
	/*!   \remarks Adds the faces and normals from "from" to our normal
	spec, renumbering the\n\n
	normals so they don't conflict with existing ones. Called by the\n\n
	"AppendAllChannels" method below (which itself is called by
	MNMesh::operator+=).\n\n
	  */
	DllExport MNNormalSpec & operator+= (const MNNormalSpec & from);
	/*!   \remarks Uses "DebugPrint" to output information about this
	MNNormalSpece to the\n\n
	Debug buffer in DevStudio.\n\n

	\par Parameters:
	<b>bool printAll=false</b>\n\n
	If false, only explicit normals and faces using specified normals will
	be\n\n
	printed out. If true, all normals and faces will be completely printed
	out.\n\n
	Here is what the output looks like on a box with mostly default
	(non-specified)\n\n
	normals, but with one corner "Unified" into a single specified normal:\n\n
	If printAll = true, you'll see:\n\n
	MNNormalSpec Debug Output: 22 normals, 6 faces\n\n
	Normal (Non ) 0: 0.577350, -0.577350, 0.577350\n\n
	Normal (Non ) 1: 0.000000, 0.000000, -1.000000\n\n
	Normal (Non ) 2: 0.000000, 0.000000, -1.000000\n\n
	Normal (Non ) 3: 0.000000, 0.000000, -1.000000\n\n
	Normal (Non ) 4: 0.000000, 0.000000, -1.000000\n\n
	Normal (Non ) 5: 0.000000, 0.000000, 1.000000\n\n
	Normal (Non ) 6: 0.000000, 0.000000, 1.000000\n\n
	Normal (Non ) 7: 0.000000, 0.000000, 1.000000\n\n
	Normal (Non ) 8: 0.000000, -1.000000, 0.000000\n\n
	Normal (Non ) 9: 0.000000, -1.000000, 0.000000\n\n
	Normal (Non ) 10: 0.000000, -1.000000, 0.000000\n\n
	Normal (Non ) 11: 1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 12: 1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 13: 1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 14: 0.000000, 1.000000, 0.000000\n\n
	Normal (Non ) 15: 0.000000, 1.000000, 0.000000\n\n
	Normal (Non ) 16: 0.000000, 1.000000, 0.000000\n\n
	Normal (Non ) 17: 0.000000, 1.000000, 0.000000\n\n
	Normal (Non ) 18: -1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 19: -1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 20: -1.000000, 0.000000, 0.000000\n\n
	Normal (Non ) 21: -1.000000, 0.000000, 0.000000\n\n
	Normal Selection:\n\n
	0 6 13 17\n\n
	Normal faces: _ before normal index means non-specified.\n\n
	Face 0: _1 _2 _3 _4\n\n
	Face 1: _5 0 _6 _7\n\n
	Face 2: _8 _9 0 _10\n\n
	Face 3: _11 _12 _13 0\n\n
	Face 4: _14 _15 _16 _17\n\n
	Face 5: _18 _19 _20 _21\n\n
	If printAll is false, you'll just see:\n\n
	MNNormalSpec Debug Output: 22 normals, 6 faces\n\n
	Normal Selection:\n\n
	0 6 13 17\n\n
	Face 1: _ 0 _ _\n\n
	Face 2: _ _ 0 _\n\n
	Face 3: _ _ _ 0\n\n
	  */
	DllExport void MNDebugPrint (bool printAll=false);
	/*!   \remarks Performs a series of internal checks to verify that
	the normal\n\n
	data is consistent. If there are any problems, messages are\n\n
	sent out via DebugPrint.\n\n

	\par Parameters:
	<b>int numParentFaces</b>\n\n
	The number of faces in the parent MNMesh. (This method does not\n\n
	require an accurate "parent" pointer, but it does require this\n\n
	number to be accurate.\n\n

	\return  True if everything checks out ok, false if not.\n\n
	  */
	DllExport bool CheckAllData (int numParentFaces);

	/*!   \remarks Called by the system. Saves the MNNormalSpec to the
	buffer.\n\n
	  */
	DllExport IOResult Save (ISave *isave);
	/*!   \remarks Called by the system. Loads the MNNormalSpec from the
	buffer.\n\n
	  */
	DllExport IOResult Load (ILoad *iload);

	// From BaseInterface:
	Interface_ID GetID() {return MN_NORMAL_SPEC_INTERFACE;}
	DllExport void DeleteInterface();
	DllExport BaseInterface* GetInterface(Interface_ID id);
	DllExport BaseInterface* CloneInterface(void* remapDir = NULL);

	// --- IPipelineClient methods
	DllExport void ShallowCopy( IPipelineClient* from, ChannelMask channels );
	DllExport void DeepCopy( IPipelineClient* from, ChannelMask channels );
	DllExport void NewAndCopyChannels( ChannelMask channels );
	DllExport void FreeChannels( ChannelMask channels, int zeroOthers = 1 );
	DllExport void ZeroChannels( ChannelMask channels );
	DllExport void AppendAllChannels( IPipelineClient* from );

	// Actual operations:
	/*!   \remarks Transforms the normals. Note that since normals are
	always considered to be unit\n\n
	length vectors, scales and translations are not effective. Translations
	drop out\n\n
	because we use the VectorTransform method to transform the normals, and
	scales drop\n\n
	out because we renormalize the normals to a length of 1 afterwards.\n\n

	\par Parameters:
	<b>Matrix3 \& xfm</b>\n\n
	The desired transform.\n\n
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be translated, or only selected
	ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	The desired transform.\n\n
	<b>BOOL useSel=false</b>\n\n
	If non-NULL, this represents a selection of normals that should be used
	instead of\n\n
	the usual selection, when deciding which normals to transform.\n\n

	\return  True if something was modified. False would indicate that no
	normals were changed,\n\n
	perhaps because there are no normals in the spec or because none were
	selected.\n\n
	  */
	DllExport bool Transform (Matrix3 & xfm, BOOL useSel=false, BitArray *normalSelection=NULL);
	/*!   \remarks This is used to give a translation-like effect to
	normals. It's used in the Edit\n\n
	Normals "Move" mode. Essentially it drags the "top" of the normals by the
	amount\n\n
	given, and then renormalizes the vectors to unit length. It uses the
	current\n\n
	display length as well, so the formula is basically\n\n
	mpNormal[i] = Normalize (mpNormal[i]*mDisplayLength + translate);\n\n
	This gives a fairly natural result in Edit Normals Move.\n\n

	\par Parameters:
	<b>Point3 \& translate</b>\n\n
	The desired translation.\n\n
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be translated, or only selected
	ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used
	instead of\n\n
	the usual selection, when deciding which normals to translate.\n\n

	\return  True if something was modified. False would indicate that no
	normals were changed,\n\n
	perhaps because there are no normals in the spec or because none were
	selected.\n\n
	  */
	DllExport bool Translate (Point3 & translate, BOOL useSel=true, BitArray *normalSelection=NULL);
	/*! \remarks "Breaks" normals so that none are shared between faces.
	Broken normals are set to Specified (but not explicit.)\n\n

	\par Parameters:
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be affected, or only selected ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used instead
	of\n\n
	the usual selection, when deciding which normals to affect. (Irrelevant if\n\n
	useSel=false.)\n\n

	\return  True if something was modified. False would indicate that no normals
	were changed,\n\n
	perhaps because there are no normals present or because none were selected,\n\n
	or because selected normals were already fully broken and specified.\n\n
	If the return value is true, the MNNORMAL_NORMALS_BUILT and\n\n
	MNNORMAL_NORMALS_COMPUTED flags are cleared, because the newly broken\n\n
	normals need to be rebuilt and computed.\n\n
	  */
	DllExport bool BreakNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	// Requires an accurate mpParent pointer:
	/*!   \remarks Requires an accurate "parent" pointer (see SetParent).\n\n
	This method unifies selected normals so that there's a maximum of one\n\n
	per vertex. For instance, a default box has 3 normals at every vertex.\n\n
	You can select 2 or 3 of them and click "Unify" in Edit Normals, and\n\n
	the normals will be shared across the faces that use them. See Edit\n\n
	Normals documentation for more information.\n\n
	This method does not unify normals that are based at different vertices.\n\n
	If you want separate vertices to use the same normal, you must use more\n\n
	direct, low-level methods like SetNormalIndex.\n\n
	Unified normals are set to specified (but not explicit).\n\n

	\par Parameters:
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be affected, or only selected ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used instead
	of\n\n
	the usual selection, when deciding which normals to affect. (Irrelevant if\n\n
	useSel=false.)\n\n

	\return  True if something was modified. False would indicate that no normals
	were changed,\n\n
	perhaps because there are no normals present or because none were selected,\n\n
	or because selected normals were already fully unified and specified.\n\n
	If the return value is true, the MNNORMAL_NORMALS_BUILT and\n\n
	MNNORMAL_NORMALS_COMPUTED flags are cleared, because the newly unified\n\n
	normals need to be rebuilt and computed.\n\n
	  */
	DllExport bool UnifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	DllExport bool AverageNormals (BOOL useThresh=false, float threshold=0.0f, BOOL useSel=true, BitArray *normalSelection=NULL);
	/*!   \remarks Specifies the indicated normals. DOESN'T remove the
	explicitness of the normals.\n\n
	(That should be done separately with MakeNormalsExplicit, value=false.)
	\par Parameters:
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be affected, or only selected
	ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used
	instead of\n\n
	the usual selection, when deciding which normals to affect. (Irrelevant
	if\n\n
	useSel=false.)\n\n

	\return  True if something was modified. False would indicate that no
	normals were changed,\n\n
	perhaps because there are no normals present or because none were selected,
	or because\n\n
	they were all already specified.\n\n
	  */
	DllExport bool SpecifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
	/*!   \remarks Makes the indicated normals both specified and
	explicit.
	\par Parameters:
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be affected, or only selected
	ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used
	instead of\n\n
	the usual selection, when deciding which normals to affect. (Irrelevant
	if\n\n
	useSel=false.)\n\n
	<b>bool value=true</b>\n\n
	I Indicates whether the normals should be set to explicit, or
	non-explicit.\n\n

	\return  True if something was modified. False would indicate that no
	normals were changed,\n\n
	perhaps because there are no normals present or because none were
	selected.\n\n
	If value=false and the return value is true, the
	MNNORMAL_NORMALS_COMPUTED\n\n
	flag is cleared, because the newly nonexplicit normals need to be
	computed\n\n
	.\n\n
	  */
	DllExport bool MakeNormalsExplicit (BOOL useSel=true, BitArray *normalSelection=NULL, bool value=true);
	/*!   \remarks Resets the indicated normals to be completely
	non-explicit and non-specified.
	\par Parameters:
	<b>BOOL useSel=false</b>\n\n
	Indicates whether all normals should be affected, or only selected
	ones.\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	If non-NULL, this represents a selection of normals that should be used
	instead of\n\n
	the usual selection, when deciding which normals to affect. (Irrelevant
	if\n\n
	useSel=false.)\n\n

	\return  True if something was modified. False would indicate that no
	normals were changed,\n\n
	perhaps because there are no normals present or because none were
	selected.\n\n
	If the return value is true, the MNNORMAL_NORMALS_BUILT and\n\n
	MNNORMAL_NORMALS_COMPUTED flags are cleared, because the newly
	nonspecified\n\n
	normals need to be rebuilt and computed. */
	DllExport bool ResetNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
};

