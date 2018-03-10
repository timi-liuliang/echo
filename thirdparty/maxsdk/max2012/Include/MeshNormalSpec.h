/**********************************************************************
 *<
	FILE: MeshNormalSpec.h

	DESCRIPTION:  User-specifiable normals for Triangle Meshes

	CREATED BY: Steve Anderson

	HISTORY: created July 2002 during SIGGraph

 *>	Copyright (c) 2002 Autodesk, Inc., All Rights Reserved.
 **********************************************************************/

// Necessary prior inclusions...?

#pragma once
#include "export.h"
#include "maxheap.h"
#include "baseinterface.h"
#include "ipipelineclient.h"
#include "bitarray.h"
#include "box3.h"

// forward declarations
class ISave;
class ILoad;
class Point3;
class Mesh;
class GraphicsWindow;
class HitRegion;
class SubObjHitList;

#define MESH_NORMAL_SPEC_INTERFACE Interface_ID(0xa4e770b, 0x47aa3cf9)

///  \brief 
///  <b>See Also: </b>class MeshNormalSpec
///
/// This class is available in release 6.0 and later only.
/// 
/// This class is used to store specified normal information for a
/// particular face in a Mesh. An array of these faces is used in
/// class MeshNormalSpec to match vertex normals to vertices.
///
/// Each MeshNormalFace contains a bit indicating whether each corner
/// is specified, and a Normal ID for each specified corner.
/// 
class MeshNormalFace: public MaxHeapOperators {
	int mNormalID[3];
	UBYTE mSpecified;

public:
	/// \brief  Constructor - sets all corners to "unspecified" 
	MeshNormalFace () : mSpecified(0x0) { }

	/// \brief  Resets all corners to "unspecified". 
	DllExport void Clear();

	/// \brief  Accessor for normal in a particular corner of the face. 
	/// \param corner  The (zero-based) index of the corner of the face 
	/// \return  The index of the normal (in the owner MeshNormalSpec's normal array)
	// used in that corner, or -1 for error 
	int GetNormalID(int corner) { return ((corner>-1) && (corner<3)) ? mNormalID[corner] : -1; }

	/// \brief  Setter for the normal used in a particular corner of the face 
	/// \param corner  The (zero-based) index of the corner of the face 
	/// \param norm  The index of the normal (in the owner MeshNormalSpec's normal array).
	void SetNormalID (int corner, int norm) { if ((corner>-1) && (corner<3)) mNormalID[corner] = norm; }

	/// \brief return an array of 3 normal IDs
	/// \return an array of 3 normal IDs
	int *GetNormalIDArray () { return mNormalID; }

	///  \brief  Indicates whether the normal used in a particular corner is specified or not 
	bool GetSpecified (int corner) { return ((corner>-1) && (corner<3) && (mSpecified & (1<<corner))) ? true : false; }

	///  \brief  Sets the normal used by the corner to be specified or not. 
	DllExport void SetSpecified (int corner, bool value=true);

	///  \brief  Sets the normal used by the corner to be specified, and to use the particular normal given. 
	DllExport void SpecifyNormalID (int corner, int norm);

	///  \brief  Sets this face to have all its normals specified.  
	void SpecifyAll (bool value=true) { mSpecified = value ? 7 : 0; }

	///  \brief  Reverses the order of the normals.  Corner 0 is unaffected, but corner 1 is switched
	/// with corner 2.
	DllExport void Flip ();

	///  \brief  Standard = operator. 
	DllExport MeshNormalFace & operator= (const MeshNormalFace & from);

	///  \brief  Using DebugPrint, output information about this MeshNormalFace.  If "printAll" is true,
	/// it will print normal IDs for both specified and unspecified normals; otherwise it'll print only the
	/// specified ones.  Typical output for a partially specified face: "_7 3 _5" (printAll==true) or
	/// "_ 3 _" (printAll==false).  A newline is printed at the end. 
	DllExport void MyDebugPrint (bool printAll=false);

	///  \brief  Save to file 
	DllExport IOResult Save (ISave *isave);

	///  \brief  Load from file 
	DllExport IOResult Load (ILoad *iload);
};

// Class MeshNormalSpec flags:
#define MESH_NORMAL_NORMALS_BUILT 0x01
#define MESH_NORMAL_NORMALS_COMPUTED 0x02
#define MESH_NORMAL_DISPLAY_HANDLES 0x04
#define MESH_NORMAL_FACE_ANGLES 0x08	// Used face angles last time we computed normals
#define MESH_NORMAL_MODIFIER_SUPPORT 0x20	// When modifying mesh, indicates whether the current modifier supports these Normals.

// Default length for normal display
#define MESH_NORMAL_LENGTH_DEFAULT 10.0f

///  \brief 
/// This class is an interface used to store user-specified normals
/// (such as those created in the Edit Normals modifier). These normals have very
/// limited pipeline support. They are used for viewport display, but not
/// for rendering.
///
/// The MeshNormalSpec contains three types of normals:
/// - Unspecified - these are the usual normals that are computed from
///   smoothing groups. All normals are unspecified by default.
///
/// - Specified - these are normals that are intended for use by particular
///   corners of particular faces, without regard to smoothing groups. For 
///   instance, you can create a box, apply Edit Normals, select a group of 
///   normals at a particular vertex, and click "Unify". Now those three 
///   faces are told to specifically use that one unified normal, and they 
///   ignore their smoothing groups at that vertex (which would normally 
///   tell them they should each have their own normal). 
///
/// - Explicit - these are normals that are set to particular values.
///   For instance, if the user wants to use the Edit Normals Move or Rotate 
///   commands to set a normal to something other than its default value, 
///   it has to be made explicit, so it won't be recomputed based on the 
///   face normals. All explicit normals are also considered to be specified..
/// 
///  \remark 
/// This class is available in release 6.0 and later only.
/// 
class MeshNormalSpec : public IPipelineClient {
private:
	int mNumNormalAlloc, mNumFaceAlloc;
	int mNumNormals, mNumFaces;
	MeshNormalFace *mpFace;
	Point3 *mpNormal;
	BitArray mNormalExplicit;	// Indicates whether mpNormal[i] is explicit or computed from face normals.

	// Display and selection data:
	BitArray mNormalSel;
	float mDisplayLength;
	DWORD mFlags;

	// We also maintain a pointer to the parent Mesh
	// (NOTE that the Mesh MUST keep this pointer updated at all times!)
	Mesh *mpParent;

public:
	MeshNormalSpec () : mpFace(NULL), mpNormal(NULL), mNumNormalAlloc(0),
		mNumFaceAlloc(0), mNumNormals(0), mNumFaces(0), mpParent(NULL), mFlags(0),
		mDisplayLength(MESH_NORMAL_LENGTH_DEFAULT) { }
	~MeshNormalSpec () { ClearAndFree (); }

	///  \brief  Sets flags in the MeshNormalSpec.  Generally, the only flag that clients should set
	/// is the MESH_NORMAL_MODIFIER_SUPPORT flag.  See GetFlag for more details. 
	void SetFlag (DWORD fl, bool val=true) { if (val) mFlags |= fl; else mFlags &= ~fl; }

	///  \brief  Clears the flags given.  See GetFlag for more details on MeshNormalSpec flags. 
	void ClearFlag (DWORD fl) { mFlags &= ~fl; }

	///  \brief  Gets flags in the MeshNormalSpec.  
	///  \remarks MeshNormalSpec supports the following flags:
	/// MESH_NORMAL_NORMALS_BUILT
	///   Indicates that non-specified normals have been constructed using
	///   smoothing groups. If not set, non-specified normals may be invalid.
	/// MESH_NORMAL_NORMALS_COMPUTED 
	///   Indicates that non-explicit normals have been computed using geometrically
	///   computed face normals. (If not set, only explicit normals may be assumed
	///   to be pointing the right direction.)
	/// MESH_NORMAL_MODIFIER_SUPPORT
	///   This flag should be set by modifiers that want to support the edited normals.
	///   See class TriObjectNormalModifier for more details.  If this flag is not specifically
	///   set in a modifier that alters PART_GEOM or PART_TOPO of a TriObject, any specified 
	///   normals will be cleared after the modifier is evaluated.
	/// 
	bool GetFlag (DWORD fl) const { return (mFlags & fl) ? true : false; }

	// Initialization, allocation:
	
	/// \brief Initializes all data members - do not use if already allocated!
	DllExport void Initialize ();
	DllExport bool NAlloc (int num, bool keep=TRUE);
	/// \brief shrinks allocation down to actual number of normals.
	DllExport void NShrink ();
	DllExport bool FAlloc (int num, bool keep=TRUE);
	DllExport void FShrink ();
	/// \brief Deletes everything.
	DllExport void Clear ();
	/// \brief Deletes everything, frees all memory
	DllExport void ClearAndFree ();	

	// Data access:
	// Lowest level:
	int GetNumFaces () const { return mNumFaces; }
	DllExport bool SetNumFaces (int numFaces);
	int GetNumNormals () const { return mNumNormals; }
	DllExport bool SetNumNormals (int numNormals);

	Point3 & Normal (int normID) const { return mpNormal[normID]; }
	Point3 * GetNormalArray () const { return mpNormal; }
	bool GetNormalExplicit (int normID) const { return mNormalExplicit[normID] ? true : false; }
	void SetNormalExplicit (int normID, bool value) { mNormalExplicit.Set (normID, value); }

	///  \brief  Set all normals to be explicit.
	///  \param value - If true, all normals will be set to explicit.  If false, all normals will be set to non-explicit
	void SetAllExplicit (bool value=true) { if (value) mNormalExplicit.SetAll(); else mNormalExplicit.ClearAll (); }
	MeshNormalFace & Face(int faceID) const { return mpFace[faceID]; }
	MeshNormalFace * GetFaceArray () const { return mpFace; }

	void SetParent (Mesh *pMesh) { mpParent = pMesh; }

	// Data access - higher level:
	DllExport Point3 & GetNormal (int face, int corner);
	DllExport void SetNormal (int face, int corner, Point3 & normal);
	DllExport int GetNormalIndex (int face, int corner);
	DllExport void SetNormalIndex (int face, int corner, int normalIndex);
	DllExport int NewNormal (Point3 normal, bool explic=true);

	DllExport void SetSelection (BitArray & newSelection);
	BitArray & GetSelection() { return mNormalSel; }
	void SetDisplayLength (float displayLength) { mDisplayLength = displayLength; }
	float GetDisplayLength () { return mDisplayLength; }

	// Display and hit testing - note that these require an accurate mpParent pointer.
	DllExport void Display (GraphicsWindow *gw, bool showSel);
	DllExport bool HitTest (GraphicsWindow *gw, HitRegion *hr, DWORD flags, SubObjHitList& hitList);
	DllExport Box3 GetBoundingBox (Matrix3 *tm=NULL, bool selectedOnly=false);

	/// \brief This method dumps all unspecified normals.  Best to use only from within CheckNormals.
	DllExport void ClearNormals ();

	/// \brief Fills in the mpSpecNormal data by building all the unspecified normals,
	/// and computing non-explicit ones.
	/// \remarks Does nothing if normal faces not allocated yet!\n\n
	/// Requires an accurate mpParent pointer.
	DllExport void BuildNormals ();

	/// \brief This method just recomputes the directions of non-explicit normals,
	/// without rebuilding the normal list.
	/// \remarks Requires an accurate mpParent pointer.
	DllExport void ComputeNormals ();

	/// \brief This checks our flags and calls BuildNormals, ComputeNormals as needed.
	/// \remarks Requires an accurate mpParent pointer.
	DllExport void CheckNormals ();

	// operators and debug printing
	DllExport MeshNormalSpec & operator= (const MeshNormalSpec & from);
	DllExport void CopySpecified (const MeshNormalSpec & from);	// Like operator=, but omits unspecified.
	DllExport MeshNormalSpec & operator+= (const MeshNormalSpec & from);
	DllExport void MyDebugPrint (bool printAll=false);
	DllExport bool CheckAllData (int numParentFaces);

	DllExport IOResult Save (ISave *isave);
	DllExport IOResult Load (ILoad *iload);

	// From BaseInterface:
	Interface_ID GetID() {return MESH_NORMAL_SPEC_INTERFACE;}
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
	DllExport bool Transform (Matrix3 & xfm, BOOL useSel=false, BitArray *normalSelection=NULL);
	DllExport bool Translate (Point3 & translate, BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool BreakNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	// Requires an accurate mpParent pointer:
	DllExport bool UnifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL, BOOL toAverage=false);
	DllExport bool AverageNormals (BOOL useThresh=false, float threshold=0.0f, BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool SpecifyNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
	DllExport bool MakeNormalsExplicit (BOOL useSel=true, BitArray *normalSelection=NULL, bool value=true);
	DllExport bool ResetNormals (BOOL useSel=true, BitArray *normalSelection=NULL);
};

