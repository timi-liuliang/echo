/**********************************************************************
 *<
	FILE: iEditNormals.h

	DESCRIPTION:   Edit Normals Modifier SDK Interface

	CREATED BY: Steve Anderson

	HISTORY: created January 2002

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
#pragma once

#include "iFnPub.h"
#include "point3.h"
// forward declarations
class MNMesh;
class MNNormalSpec;

// Edit Normals Selection Levels - selection is always some set of normals
// (vertex/face selLevels are just rules for hit testing - select normals by vertex, or by face.)
enum editNormalSelLevel
{
	EN_SL_OBJECT,
	EN_SL_NORMAL,
	EN_SL_VERTEX,
	EN_SL_EDGE,
	EN_SL_FACE
};

// Edit Normals Parameters:
enum editNormalParameters
{
	en_display_length,
	en_ignore_backfacing,
	en_select_by,
	en_show_handles
};

#define EDIT_NORMALS_MOD_INTERFACE Interface_ID(0x2b572ad6,0x7cf86ae2)

// "Mixin" Interface methods:
enum editNormalsModMethods { enfn_get_sel_level, enfn_set_sel_level, enfn_move,
	enfn_rotate, enfn_break, enfn_unify, enfn_reset, enfn_specify,
	enfn_make_explicit, enfn_copy, enfn_paste,
	enfn_select, enfn_get_selection, enfn_set_selection,
	enfn_convert_vertex_selection, enfn_convert_edge_selection,
	enfn_convert_face_selection, enfn_get_num_normals,
	enfn_get_num_faces, enfn_get_normal, enfn_set_normal,
	enfn_get_normal_explicit, enfn_set_normal_explicit,
	enfn_get_face_degree, enfn_get_normal_id, enfn_set_normal_id,
	enfn_get_num_vertices, enfn_get_vertex_id, enfn_get_vertex,
	enfn_get_num_edges, enfn_get_edge_id, enfn_get_edge_vertex,
	enfn_get_face_edge_side, enfn_get_edge_face, enfn_get_edge_normal,
	enfn_get_face_normal_specified, enfn_set_face_normal_specified,
	enfn_rebuild_normals, enfn_recompute_normals, enfn_average,
	enfn_average_global, enfn_average_two };

// Enum of enums, for methods which accept enum parameters
enum editNormalsModEnums { enprop_sel_level };

/*! \sa  Class MNMesh , Class MNNormalSpec\n\n
class IEditNormalsMod : public FPMixinInterface\n\n

\par Description:
This class is available in release 5.0 and later only.\n\n
This class is an interface used by the scripter and the SDK to access the Edit
Normals modifier. See the documentation for that modifier for background on the
normals and the basic operations like Break and Unify.\n\n
All but the last two of these methods are available via the scripter with
commands like:\n\n
numNormals = $.modifiers[#Edit_Normals].EditNormalsMod.GetNumNormals ()\n\n
$.modifiers[#Edit_Normals].EditNormalsMod.SetSelection #{1..numNormals}\n\n
$.modifiers[#Edit_Normals].EditNormalsMod.Unify ().\n\n
  */
#pragma warning(push)
#pragma warning(disable:4100)
class IEditNormalsMod : public FPMixinInterface
{
public:
	#pragma warning(push)
	#pragma warning(disable:4238)
	BEGIN_FUNCTION_MAP
		// Selection Level Access
		PROP_FNS(enfn_get_sel_level, EnfnGetSelLevel, enfn_set_sel_level, EnfnSetSelLevel, TYPE_INT);

		// Transforms
		FNT_1(enfn_move, TYPE_bool, EnfnMove, TYPE_POINT3_BR);
		FNT_1(enfn_rotate, TYPE_bool, EnfnRotate, TYPE_QUAT_BR);

		// Operations
		FN_3(enfn_break, TYPE_bool, EnfnBreakNormals, TYPE_BITARRAY, TYPE_INODE, TYPE_bool);
		FN_3(enfn_unify, TYPE_bool, EnfnUnifyNormals, TYPE_BITARRAY, TYPE_INODE, TYPE_bool);
		FN_2(enfn_reset, TYPE_bool, EnfnResetNormals, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_specify, TYPE_bool, EnfnSpecifyNormals, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_make_explicit, TYPE_bool, EnfnMakeNormalsExplicit, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_copy, TYPE_bool, EnfnCopyNormal, TYPE_INDEX, TYPE_INODE);
		FN_2(enfn_paste, TYPE_bool, EnfnPasteNormal, TYPE_BITARRAY, TYPE_INODE);
		FN_4(enfn_average, TYPE_bool, EnfnAverageNormals, TYPE_bool, TYPE_FLOAT, TYPE_BITARRAY, TYPE_INODE);
		FN_2(enfn_average_global, TYPE_bool, EnfnAverageGlobalNormals, TYPE_bool, TYPE_FLOAT);
		FN_4(enfn_average_two, TYPE_bool, EnfnAverageTwoNormals, TYPE_INODE, TYPE_INDEX, TYPE_INODE, TYPE_INDEX);

		// Selection set access
		FN_1(enfn_get_selection, TYPE_BITARRAY, EnfnGetSelection, TYPE_INODE);
		FN_2(enfn_set_selection, TYPE_bool, EnfnSetSelection, TYPE_BITARRAY_BR, TYPE_INODE);
		FN_4(enfn_select, TYPE_bool, EnfnSelect, TYPE_BITARRAY_BR, TYPE_bool, TYPE_bool, TYPE_INODE);
		VFN_3(enfn_convert_vertex_selection, EnfnConvertVertexSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);
		VFN_3(enfn_convert_edge_selection, EnfnConvertEdgeSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);
		VFN_3(enfn_convert_face_selection, EnfnConvertFaceSelection, TYPE_BITARRAY_BR, TYPE_BITARRAY_BR, TYPE_INODE);

		// Normal access
		FN_1(enfn_get_num_normals, TYPE_INT, EnfnGetNumNormals, TYPE_INODE);
		FNT_2(enfn_get_normal, TYPE_POINT3, EnfnGetNormal, TYPE_INDEX, TYPE_INODE);
		VFNT_3(enfn_set_normal, EnfnSetNormal, TYPE_INDEX, TYPE_POINT3_BR, TYPE_INODE);
		FN_2(enfn_get_normal_explicit, TYPE_bool, EnfnGetNormalExplicit, TYPE_INDEX, TYPE_INODE);
		VFN_3(enfn_set_normal_explicit, EnfnSetNormalExplicit, TYPE_INDEX, TYPE_bool, TYPE_INODE);

		// Normal face access
		FN_1(enfn_get_num_faces, TYPE_INT, EnfnGetNumFaces, TYPE_INODE);
		FN_2(enfn_get_face_degree, TYPE_INT, EnfnGetFaceDegree, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_normal_id, TYPE_INDEX, EnfnGetNormalID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_4(enfn_set_normal_id, EnfnSetNormalID, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_face_normal_specified, TYPE_bool, EnfnGetFaceNormalSpecified, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		VFN_4(enfn_set_face_normal_specified, EnfnSetFaceNormalSpecified, TYPE_INDEX, TYPE_INDEX, TYPE_bool, TYPE_INODE);

		// Vertex access
		FN_1(enfn_get_num_vertices, TYPE_INT, EnfnGetNumVertices, TYPE_INODE);
		FN_3(enfn_get_vertex_id, TYPE_INDEX, EnfnGetVertexID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FNT_2(enfn_get_vertex, TYPE_POINT3_BV, EnfnGetVertex, TYPE_INDEX, TYPE_INODE);

		// Edge access
		FN_1(enfn_get_num_edges, TYPE_INT, EnfnGetNumEdges, TYPE_INODE);
		FN_3(enfn_get_edge_id, TYPE_INDEX, EnfnGetEdgeID, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_face_edge_side, TYPE_INDEX, EnfnGetFaceEdgeSide, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_edge_vertex, TYPE_INDEX, EnfnGetEdgeVertex, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_3(enfn_get_edge_face, TYPE_INDEX, EnfnGetEdgeFace, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);
		FN_4(enfn_get_edge_normal, TYPE_INDEX, EnfnGetEdgeNormal, TYPE_INDEX, TYPE_INDEX, TYPE_INDEX, TYPE_INODE);

		// Cache rebuilders - shouldn't normally be necessary - above methods do this work as needed.
		VFN_1(enfn_rebuild_normals, EnfnRebuildNormals, TYPE_INODE);
		VFN_1(enfn_recompute_normals, EnfnRecomputeNormals, TYPE_INODE);
	END_FUNCTION_MAP
	#pragma warning(pop)

	FPInterfaceDesc *GetDesc ();

	// Selection level accessors:
	/*!   \remarks "Get" accessor for selection level - one of these
	values:\n\n
	EN_SL_OBJECT, EN_SL_NORMAL, EN_SL_VERTEX, EN_SL_EDGE, EN_SL_FACE\n\n
	*/
	virtual int EnfnGetSelLevel () { return EN_SL_OBJECT; }
	/*! \remarks "Set" accessor for selection level - one of these
	values:\n\n
	EN_SL_OBJECT, EN_SL_NORMAL, EN_SL_VERTEX, EN_SL_EDGE, EN_SL_FACE\n\n
	*/
	virtual void EnfnSetSelLevel (int selLevel) { }

	// Transform commands:
	/*!   \remarks Moves the ends of currently selected normals by the
	offset indicated - then renormalizes them to unit length.\n\n
	Note that the time is currently ignored, as the Edit Normals modifier is
	not yet animatable\n\n
	*/
	virtual bool EnfnMove (Point3& offset, TimeValue t) { return false; }
	/*!   \remarks Rotates currently selected normals by the rotation
	indicated.\n\n
	Note that the time is currently ignored, as the Edit Normals modifier is
	not yet animatable\n\n
	*/
	virtual bool EnfnRotate (Quat & rotation, TimeValue t) { return false; }

	// Operations:
	/*! \remarks Breaks the indicated normals into separate normals for each
	face.\n\n
	In 3ds Max 6, the Edit normals modifier had an average function added to
	its tool set. This allows the user to average the normals across the
	surface of the object(s). The toAverage parameter was added to this method
	to provide access to this new functionality.  <br>
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>bool toAverage=false </b>\n\n
	Default is false. When true, this parameter causes the normals to be
	averaged across the surface of the object(s). */
	/*! In 3ds Max 6, the Edit normals modifier had an average function added
	to its tool set. This  allows the user to average the normals across the
	surface of the objects(s). This method and the next four methods were added
	to an existing interface to provide access to this new functionality.
	\remarks Breaks the indicated normals into separate normals for each face.
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.) */
	virtual bool EnfnBreakNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false) { return false; }
	/*! \remarks Unifies the indicated normals so there's at most one normal
	per vertex. (Basically, causes normals to be shared across faces at a
	vertex.)\n\n
	In 3ds Max 6, the Edit normals modifier had an average function added to
	its tool set. This allows the user to average the normals across the
	surface of the objects(s). The toAverage parameter was added to this method
	to provide access to this new functionality.
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>bool toAverage=false </b>\n\n
	Default is false. When true, this parameter causes the normals to be
	averaged across the surface of the object(s). */
	virtual bool EnfnUnifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL, bool toAverage=false) { return false; }
	/*!   \remarks Makes the indicated normals completely non-explicit
	and unspecified. Generates a rebuild \& computation to determine the
	topology and direction of the newly unspecified normals.
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals
	in.\n\n
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnResetNormals (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	/*!   \remarks Specifies the normals indicated to be fixed to the
	faces they're currently used by.
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnSpecifyNormals (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	/*!   \remarks Make the indicated normals explicit, so they won't be
	based on underlying face normals.
	\par Parameters:
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnMakeNormalsExplicit (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	/*!   \remarks Copies the indicated normal into the Edit Normals
	modifier's copy/paste buffer.
	\par Parameters:
	<b>int normalID</b>\n\n
	The ID of the normal we want to copy.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnCopyNormal (int normalID, INode *pNode=NULL) { return false; }
	/*!   \remarks Pastes the normal currently in the Edit Normals
	modifier's copy/paste buffer into the normals indicated, making them
	specified and explicit.
	\par Parameters:
	<b>BitArray *normalSelection = NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is
	used.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnPasteNormal (BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	/*! \remarks Added in 3ds Max 6 SDK.
	\par Parameters:
	<b>bool useThresh=false</b>\n\n
	<b>float threshhold=0.0f</b>\n\n
	<b>BitArray *normalSelection=NULL</b>\n\n
	An optional selection set to use. If NULL, the current selection is used.
	 */
	virtual bool EnfnAverageNormals (bool useThresh=false, float threshold=0.0f, BitArray *normalSelection=NULL, INode *pNode=NULL) { return false; }
	/*! \remarks Added in 3ds Max 6 SDK.
	\par Parameters:
	<b>bool useThresh=false</b>\n\n
	<b>float threshhold=0.0f</b> */
	virtual bool EnfnAverageGlobalNormals (bool useThresh=false, float threshold=0.0f) { return false; }
	/*! \remarks Added in 3ds Max 6 SDK.
	\par Parameters:
	<b>INode *pNode1</b>\n\n
	<b>int normID1</b>\n\n
	<b>INode *pNode2</b>\n\n
	<b>int normID2</b>\n\n
	*/
	virtual bool EnfnAverageTwoNormals (INode *pNode1, int normID1, INode *pNode2, int normID2) { return false; }

	// Selection accessors
	/*!   \remarks Returns a pointer to the current selection.
	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual BitArray *EnfnGetSelection (INode *pNode=NULL) { return NULL; }
	/*!   \remarks Sets the normal selection to the selection given.
	\par Parameters:
	<b>BitArray\&selection</b>\n\n
	The desired selection\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  True if the selection was changed; false if the new selection was
	the same as the old selection.\n\n
	  */
	virtual bool EnfnSetSelection (BitArray & selection, INode *pNode=NULL) { return false; }
	/*!   \remarks Selects, deselects, or inverts the selection of the
	normals indicated.
	\par Parameters:
	<b>BitArray\&selection</b>\n\n
	The normals whose selection we are trying to change.\n\n
	<b>bool invert=false</b>\n\n
	If true, indicates that the normals in \<selection\> should have their
	selection status inverted\n\n
	<b>bool select=true</b>\n\n
	If \<invert\> is true, this is ignored. Otherwise, if true, the normals
	indicated should be selected; if false, they should be deselected.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  True if the selection changed, false otherwise.\n\n
	*/
	virtual bool EnfnSelect (BitArray & selection, bool invert=false, bool select=true, INode *pNode=NULL) { return false; }
	/*!   \remarks Converts a vertex selection into a selection of
	normals, by setting bits on normals based at selected faces.
	\par Parameters:
	<b>BitArray \& vertexSelection</b>\n\n
	The vertex selection we're converting\n\n
	<b>BitArray \& normalSelection</b>\n\n
	The output normal selection\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  True if the selection changed, false otherwise.\n\n
	*/
	virtual void EnfnConvertVertexSelection (BitArray & vertexSelection, BitArray & normalSelection, INode *pNode=NULL) { }
	/*!   \remarks Converts an edge selection into a selection of
	normals, by setting bits for normals used on either end and either side of
	selected edges.
	\par Parameters:
	<b>BitArray \&</b> edgeSelection\n\n
	The edge selection we're converting\n\n
	<b>BitArray \& normalSelection</b>\n\n
	The output normal selection\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnConvertEdgeSelection (BitArray & edgeSelection, BitArray & normalSelection, INode *pNode=NULL) { }
	/*!   \remarks Converts a face selection into a selection of normals,
	by setting bits on normals used by selected faces.
	\par Parameters:
	<b>BitArray \& faceSelection</b>\n\n
	The face selection we're converting\n\n
	<b>BitArray \& normalSelection</b>\n\n
	The output normal selection\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	  */
	virtual void EnfnConvertFaceSelection (BitArray & faceSelection, BitArray & normalSelection, INode *pNode=NULL) { }

	// Accessors for the normals:
	/*!   \remarks Returns the current number of normals.
	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetNumNormals (INode *pNode=NULL) { return 0; }
	// Direct access to the normals themselves:
	/*!   \remarks Returns a pointer to the normal indicated.
	\par Parameters:
	<b>int normalID</b>\n\n
	The index of the normal\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>TimeValue t=0</b>\n\n
	This is currently unused - but might be important if we add animation
	capabilities to Edit Normals in the future.\n\n
	*/
	virtual Point3 *EnfnGetNormal (int normalID, INode *pNode=NULL, TimeValue t=0) { return NULL; }
	/*!   \remarks Sets the indicated normal to a specific value. NOTE
	that this does not set the "explicitness" of this normal. If the normal is
	not made\n\n
	explicit, it will be restored to its default value the next time
	non-explicit normals are recomputed.
	\par Parameters:
	<b>int normalID</b>\n\n
	The index of the normal\n\n
	<b>Point3 \& direction</b>\n\n
	The desired normal direction. If not already normalized to a length of
	1,\n\n
	this method will normalize it.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>TimeValue t=0</b>\n\n
	This is currently unused - but might be important if we add animation
	capabilities to Edit Normals in the future.\n\n
	*/
	virtual void EnfnSetNormal (int normalID, Point3 &direction, INode *pNode=NULL, TimeValue t=0) { }
	// Control whether a given normal is built from smoothing groups or set to an explicit value
	/*!   \remarks Controls whether a given normal is built from
	smoothing groups or set to an explicit value (Also makes the normal
	specified for all faces using this normal.)
	\par Parameters:
	<b>int normalID</b>\n\n
	The index of the normal\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	  */
	virtual bool EnfnGetNormalExplicit (int normID, INode *pNode=NULL) { return false; }
	// (Also makes the normal specified for all faces using this normal.)
	/*!   \remarks Makes the indicated normal explicit (or not). If
	setting the normal to explicit, it will also be set to "specified" on all
	faces using it. If\n\n
	setting it to non-explicit, the modifier recomputes all non-explicit
	normals to bring it up to date.
	\par Parameters:
	<b>int normalID</b>\n\n
	The index of the normal\n\n
	<b>bool value</b>\n\n
	True to make this normal explicit, false to make it non-explicit.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnSetNormalExplicit (int normID, bool value, INode *pNode=NULL) { }

	// Normals can be used by multiple faces, even at different vertices.
	// So we require access to both face and normal information.
	// Access to the normals that are assigned to each face:
	/*!   \remarks Returns the number of faces in the normal
	specification..
	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetNumFaces (INode *pNode=NULL) { return 0; }
	/*!   \remarks Returns the degree of the face indicated. (3 for
	triangle, 4 for quad, etc.)
	\par Parameters:
	<b>int face</b>\n\n
	The desired face.  <b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetFaceDegree (int face, INode *pNode=NULL) { return 0; }
	/*!   \remarks Gets the index of the normal in the indicated corner
	of the indicated face
	\par Parameters:
	<b>int face</b>\n\n
	The desired face.\n\n
	<b>int corner</b>\n\n
	The desired corner, in the range of 0 to EnfnGetFaceDegree(face)-1.
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetNormalID (int face, int corner, INode *pNode=NULL) { return 0; }
	/*!   \remarks Sets the index of the normal in the indicated corner
	of the indicated\n\n
	face
	\par Parameters:
	<b>int face</b>\n\n
	The desired face.\n\n
	<b>int corner</b>\n\n
	The desired corner, in the range of 0 to EnfnGetFaceDegree(face)-1.\n\n
	<b>int normalID</b>\n\n
	The index of the desired normal\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnSetNormalID (int face, int corner, int normalID, INode *pNode=NULL) { }

	// Control whether a corner of a face uses an specified normal ID, or builds normals based on smoothing groups.
	/*!   \remarks Indicates whether a particular corner of a particular
	face is specified\n\n
	to use a specific normal or not.
	\par Parameters:
	<b>int face</b>\n\n
	The desired face.\n\n
	<b>int corner</b>\n\n
	The desired corner, in the range of 0 to EnfnGetFaceDegree(face)-1.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual bool EnfnGetFaceNormalSpecified (int face, int corner, INode *pNode=NULL) { return false; }
	/*!  \n\n
	  \remarks Controls whether a corner of a face uses a specific normal
	ID, or builds normals based on smoothing groups. If called to set a
	corner\n\n
	to unspecified, it generates a rebuild of nonspecified normals and a
	recomputation of nonexplicit normals at next update.
	\par Parameters:
	<b>int face</b>\n\n
	The desired face.\n\n
	<b>int corner</b>\n\n
	The desired corner, in the range of 0 to EnfnGetFaceDegree(face)-1.\n\n
	<b>bool specified</b>\n\n
	True to specify, false to set as unspecified.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnSetFaceNormalSpecified (int face, int corner, bool specified, INode *pNode=NULL) { }

	// Access to vertices - often important for proper normal handling to have access to the vertex the normal is based on.
	/*!   \remarks Returns the number of vertices in the current mesh
	cache..\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetNumVertices (INode *pNode=NULL) {return 0; }
	/*!   \remarks Returns the vertex used in a corner of a face, in the
	current mesh cache. (Useful for determining the "base" of the normal used
	in that corner of\n\n
	that face.)\n\n

	\par Parameters:
	<b>int face</b>\n\n
	The desired face.\n\n
	<b>int corner</b>\n\n
	The desired corner, in the range of 0 to EnfnGetFaceDegree(face)-1.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetVertexID (int face, int corner, INode *pNode=NULL) { return 0; }
	/*!   \remarks Returns the location of the vertex indicated (in the
	current mesh cache).\n\n

	\par Parameters:
	<b>int vertexID</b>\n\n
	The desired vertex.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>TimeValue t=0</b>\n\n
	This is currently unused - but might be important if we add animation
	capabilities to Edit Normals in the future.\n\n
	*/
	virtual Point3 EnfnGetVertex (int vertexID, INode *pNode=NULL, TimeValue t=0) { return Point3(0,0,0); }

	// Access to edges
	/*!   \remarks Returns the number of edges in the current mesh
	cache.\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetNumEdges (INode *pNode=NULL) { return 0; }
	/*!   \remarks Returns the index of the edge used on a particular
	side of a particular face, in the current mesh cache\n\n

	\par Parameters:
	<b>int faceIndex</b>\n\n
	The desired face.\n\n
	<b>int side</b>\n\n
	The desired side, in the range of 0 to EnfnGetFaceDegree(faceIndex)-1.\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetEdgeID (int faceIndex, int sideIndex, INode *pNode=NULL) { return 0; }
	/*!   \remarks Tells you which side of the face a given edge is on.
	(Can be useful for getting normal and vertex information around the
	edge.)\n\n

	\par Parameters:
	<b>int faceIndex</b>\n\n
	The desired face.\n\n
	<b>int edgeIndex</b>\n\n
	The desired edge\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  The side of the face, in the range of 0 to
	EnfnGetFaceDegree(faceIndex)-1, or -1 if the edge was not found on this
	face\n\n
	*/
	virtual int EnfnGetFaceEdgeSide (int faceIndex, int edgeIndex, INode *pNode=NULL) { return 0; }
	/*!   \remarks Returns the vertex at the end of the edge.\n\n

	\par Parameters:
	<b>int edgeIndex</b>\n\n
	The desired edge.\n\n
	<b>int end</b>\n\n
	The desired end - either 0 or 1\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual int EnfnGetEdgeVertex (int edgeIndex, int end, INode *pNode=NULL) { return 0; }
	/*!   \remarks Tells you what face is on a particular side of a
	particular edge.\n\n

	\par Parameters:
	<b>int edgeIndex</b>\n\n
	The index of the edge in the MNMesh's edge array.\n\n
	<b>int side</b>\n\n
	Indicates which side of the edge you want the face from. (Values: 0 or
	1.)\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  The index of the desired face, or -1 if there's an error or if
	there is no face on that side.\n\n
	  */
	virtual int EnfnGetEdgeFace (int edgeIndex, int side, INode *pNode=NULL) { return 0; }
	/*!   \remarks Returns the normal associated with a particular end
	and side of this edge.\n\n

	\par Parameters:
	<b>int edgeIndex</b>\n\n
	The index of the edge in the MNMesh's edge array.\n\n
	<b>int end</b>\n\n
	Indicates which end of the edge should be used. (Values: 0 or 1.)\n\n
	<b>int side</b>\n\n
	Indicates which side of the edge should be used - the edge may have
	different normals used by the faces on either side. (Values: 0 or 1.)\n\n
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n

	\return  The index of the desired normal..\n\n
	*/
	virtual int EnfnGetEdgeNormal (int edgeIndex, int end, int side, INode *pNode=NULL) { return 0; }

	// Rebuild all non-specified normals from smoothing groups
	// Note that this can change the number of normals in some cases, and often changes their order.
	/*!   \remarks Forces the modifier to rebuild all non-specified
	normals from the face smoothing groups. Note that this can change the
	number of normals in some cases, and often changes their order.\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnRebuildNormals (INode *pNode=NULL) { }
	// Recompute - computes nonexplicit normals from face normals.
	/*!   \remarks Forces the modifier to recompute all non-explicit
	normals.\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	*/
	virtual void EnfnRecomputeNormals (INode *pNode=NULL) { }

	// Direct accessors to MNMesh and MNNormalSpec classes - unpublished for now.
	/*!   \remarks Returns a pointer to the cached copy of the MNMesh
	held by the EditNormalsModData. This is a "stripped-down" copy of the last
	mesh that was output by the modifier. It contains no maps, vertex or edge
	data, or normals. It's mainly used as a temporary "parent" to the
	localdata's MNNormalSpec in operations such as Display.\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>TimeValue t=0</b>\n\n
	This is currently unused - but might be important if we add animation
	capabilities to Edit Normals in the future.\n\n
	  */
	virtual MNMesh *EnfnGetMesh (INode *pNode=NULL, TimeValue t=0) { return NULL; }
	/*!   \remarks Returns a pointer to the MNNormalSpec used by the
	EditNormalsModData. This MNNormalSpec is not part of any particular MNMesh,
	rather it's used as the local data of the EditNormalsMod to indicate what
	normals should be applied to the mesh coming up the pipe. Its "parent"
	pointer is generally set to NULL, and should be set to a mesh like the one
	you get from EnfnGetMesh before you do certain operations. (See class
	MNNormalSpec for details on which methods require an accurate "parent"
	pointer.\n\n

	\par Parameters:
	<b>INode *pNode = NULL</b>\n\n
	If the Edit Normals modifier is instanced across several nodes, this
	parameter can be used to indicate which node you mean to modify normals in.
	If you want to modify normals in all nodes, you must make a separate call
	for each node. If NULL, it assumes you want to modify normals in the
	"first" node. (This is fine if the modifier is only applied to one node, as
	is usually the case.)\n\n
	<b>TimeValue t=0</b>\n\n
	This is currently unused - but might be important if we add animation
	capabilities to Edit Normals in the future. */
	virtual MNNormalSpec *EnfnGetNormals (INode *pNode=NULL, TimeValue t=0) { return NULL; }
};
#pragma warning(pop) // C4100


