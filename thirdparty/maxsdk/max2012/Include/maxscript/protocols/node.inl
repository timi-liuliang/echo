/*	
 *		MAX_node_protocol.h - def_generics for the operations on MAX node objects
 *
 *		see def_abstract_generics.h for more info.
 *
 *	
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
/* node transforms */

	def_node_generic				(move,					"move");
	def_node_generic				(scale,					"scale");
	def_node_generic				(rotate,				"rotate");

/* node ops */

	use_generic						(copy,					"copy");
	def_node_generic				(reference,				"reference");
	def_node_generic				(instance,				"instance");
	def_visible_primitive			(delete,				"delete");		// a prim so I can control collection traversal
	def_node_generic_debug_ok		(isDeleted,				"isDeleted");
	def_node_generic				(addModifier,			"addModifier");
	def_node_generic				(deleteModifier,		"deleteModifier");
	def_node_generic				(collapseStack,			"collapseStack");
	def_node_generic				(bindSpaceWarp,			"bindSpaceWarp");
	def_node_generic_debug_ok		(intersects,			"intersects");
//	def_node_generic				(dependsOn,				"dependsOn");

	def_node_generic				(instanceReplace,		"instanceReplace");
	def_node_generic				(referenceReplace,		"referenceReplace");
	def_node_generic				(snapShot,				"snapShot");

	def_visible_generic_debug_ok	(getModContextTM,		"getModContextTM");
	def_visible_generic_debug_ok	(getModContextBBoxMin,	"getModContextBBoxMin");
	def_visible_generic_debug_ok	(getModContextBBoxMax,	"getModContextBBoxMax");
	def_visible_primitive_debug_ok	(validModifier,			"validModifier");
	def_visible_generic_debug_ok	(canConvertTo,			"canConvertTo");
	def_node_generic				(convertTo,				"convertTo");
	def_node_generic				(flagForeground,		"flagForeground");

	def_visible_primitive			(AttachObjects, 		"AttachObjects");

	/* node state */
	
	def_node_generic				(hide,					"hide");
	def_node_generic				(unhide,				"unhide");
	def_node_generic				(unfreeze,				"unfreeze");
	def_node_generic				(freeze,				"freeze");
	def_node_generic				(select,				"select");
	def_node_generic				(deselect,				"deselect");
	def_visible_primitive			(clearSelection,		"clearSelection");
	def_node_generic				(selectmore,			"selectmore");
	def_visible_primitive			(group,					"group");
	def_visible_primitive			(ungroup,				"ungroup");
	def_visible_primitive			(explodeGroup,			"explodeGroup");
	def_visible_primitive			(attachNodesToGroup,	"attachNodesToGroup");
	def_visible_primitive			(detachNodesFromGroup,	"detachNodesFromGroup");

/* object xrefs */
	def_visible_generic				( updateXRef,			"updateXRef");

/* utilities */
	
	def_node_generic_debug_ok		(intersectRay,			"intersectRay");
	def_node_generic_debug_ok		(printstack,			"printstack");
	def_visible_primitive			(uniqueName,			"uniqueName");

/* user prop access */
	
	def_node_generic_debug_ok		( getUserProp,			"getUserProp");
	def_node_generic				( setUserProp,			"setUserProp");
	def_node_generic_debug_ok		( getUserPropBuffer,	"getUserPropBuffer");
	def_node_generic				( setUserPropBuffer,	"setUserPropBuffer");
	
/* mesh ops -- if baseobject is a mesh */

	use_generic						( plus,					"+" );		// mesh boolean ops
	use_generic						( minus,				"-" );
	use_generic						( times,				"*" );

	def_node_generic				( convertToMesh,		"convertToMesh");  // this works on those things convertable to meshes

	def_visible_generic				( setvert,				"setvert");
	def_visible_generic_debug_ok	( getvert,				"getvert");
	def_visible_generic				( settvert,				"settvert");
	def_visible_generic_debug_ok	( gettvert,				"gettvert");
	def_visible_generic				( setvertcolor,			"setvertcolor");
	def_visible_generic_debug_ok	( getvertcolor,			"getvertcolor");
	def_visible_generic				( setnumverts,			"setnumverts");
	def_visible_generic_debug_ok	( getnumverts,			"getnumverts");
	def_visible_generic				( setnumtverts,			"setnumtverts");
	def_visible_generic_debug_ok	( getnumtverts,			"getnumtverts");
	def_visible_generic				( setnumcpvverts,		"setnumcpvverts");
	def_visible_generic_debug_ok	( getnumcpvverts,		"getnumcpvverts");
	def_visible_generic				( setnumfaces,			"setnumfaces");
	def_visible_generic_debug_ok	( getnumfaces,			"getnumfaces");
	def_visible_generic				( buildtvfaces,			"buildTVFaces");
	def_visible_generic				( buildvcfaces,			"buildVCFaces");
	def_visible_generic				( defaultvcfaces,		"defaultVCFaces");
	def_visible_generic_debug_ok	( getnormal,			"getnormal");
	def_visible_generic				( setnormal,			"setnormal");
	def_visible_generic				( setface,				"setface");
	def_visible_generic_debug_ok	( getface,				"getface");
	def_visible_generic				( settvface,			"setTVFace");
	def_visible_generic_debug_ok	( gettvface,			"getTVFace");
	def_visible_generic				( setvcface,			"setVCFace");
	def_visible_generic_debug_ok	( getvcface,			"getVCFace");
	def_visible_generic_debug_ok	( getfacenormal,		"getfacenormal");
	def_visible_generic				( setfacenormal,		"setfacenormal");
	def_visible_generic				( setfacematid,			"setfaceMatID");
	def_visible_generic_debug_ok	( getfacematid,			"getfaceMatID");
	def_visible_generic				( setfacesmoothgroup,	"setfaceSmoothGroup");
	def_visible_generic_debug_ok	( getfacesmoothgroup,	"getfaceSmoothGroup");
	def_visible_generic				( setedgevis,			"setedgevis");
	def_visible_generic_debug_ok	( getedgevis,			"getedgevis");
	def_visible_generic				( attach,				"attach");
	def_visible_generic				( detachVerts,			"detachVerts");
	def_visible_generic				( detachFaces,			"detachFaces");
	def_visible_generic				( extrudeface,			"extrudeface");
	def_visible_generic				( deletevert,			"deletevert");
	def_visible_generic				( deleteface,			"deleteface");
	def_visible_generic				( collapseface,			"collapseface");
	def_visible_generic				( setMesh,				"setMesh");
	def_visible_generic				( update,				"update");
	def_visible_generic_debug_ok	( getDisplacementMapping,		"getDisplacementMapping");
	def_visible_generic				( setDisplacementMapping,		"setDisplacementMapping");
	def_visible_generic_debug_ok	( getSubdivisionDisplacement,	"getSubdivisionDisplacement");
	def_visible_generic				( setSubdivisionDisplacement,	"setSubdivisionDisplacement");
	def_visible_generic_debug_ok	( getSplitMesh,			"getSplitMesh");
	def_visible_generic				( setSplitMesh,			"setSplitMesh");
	def_visible_generic				( displacementToPreset,	"displacementToPreset" );

	def_node_generic_debug_ok		( getVertSelection,		"getVertSelection");  // getVertSelection <node> <nodemodifier>
	def_node_generic				( setVertSelection,		"setVertSelection"); 
	def_node_generic_debug_ok		( getFaceSelection,		"getFaceSelection");  
	def_node_generic				( setFaceSelection,		"setFaceSelection");  
	def_node_generic_debug_ok		( getEdgeSelection,		"getEdgeSelection");  
	def_node_generic				( setEdgeSelection,		"setEdgeSelection");  

	def_struct_primitive			( mo_startCreate,			meshOps,	"startCreate");
	def_struct_primitive			( mo_startAttach,			meshOps,	"startAttach");
	def_struct_primitive			( mo_startExtrude,			meshOps,	"startExtrude");
	def_struct_primitive			( mo_startBevel,			meshOps,	"startBevel");
	def_struct_primitive			( mo_startChamfer,			meshOps,	"startChamfer");
	def_struct_primitive			( mo_startCut,				meshOps,	"startCut");
	def_struct_primitive			( mo_startSlicePlane,		meshOps,	"startSlicePlane");
	def_struct_primitive			( mo_startWeldTarget,		meshOps,	"startWeldTarget");
	def_struct_primitive			( mo_startFlipNormalMode, 	meshOps,	"startFlipNormalMode");
	def_struct_primitive			( mo_startDivide,			meshOps,	"startDivide");
	def_struct_primitive			( mo_startTurn,				meshOps,	"startTurn");
	def_struct_primitive			( mo_hideOp,				meshOps,	"hide");
	def_struct_primitive			( mo_unhideAllOp,			meshOps,	"unhideAll");
	def_struct_primitive			( mo_deleteOp,				meshOps,	"delete");
	def_struct_primitive			( mo_detachOp,				meshOps,	"detach");
	def_struct_primitive			( mo_weldOp,				meshOps,	"weld");
	def_struct_primitive			( mo_breakOp,				meshOps,	"break");
	def_struct_primitive			( mo_viewAlignOp,			meshOps,	"viewAlign");
	def_struct_primitive			( mo_gridAlignOp,			meshOps,	"gridAlign");
	def_struct_primitive			( mo_makePlanarOp,			meshOps,	"makePlanar");
	def_struct_primitive			( mo_collapseOp,			meshOps,	"collapse");
	def_struct_primitive			( mo_tesselateOp,			meshOps,	"tessellate");
	def_struct_primitive			( mo_explodeOp,				meshOps,	"explode");
	def_struct_primitive			( mo_sliceOp,				meshOps,	"slice");
	def_struct_primitive			( mo_removeIsolatedVertsOp,	meshOps,	"removeIsolatedVerts");
	def_struct_primitive			( mo_selectOpenEdgesOp,		meshOps,	"selectOpenEdges");
	def_struct_primitive			( mo_createShapeFromEdgesOp,meshOps,	"createShapeFromEdges");
	def_struct_primitive			( mo_flipNormalOp,			meshOps,	"flipNormal");
	def_struct_primitive			( mo_unifyNormalOp,			meshOps,	"unifyNormal");
	def_struct_primitive			( mo_visibleEdgeOp,			meshOps,	"visibleEdge");
	def_struct_primitive			( mo_invisibleEdgeOp,		meshOps,	"invisibleEdge");
	def_struct_primitive			( mo_autoEdgeOp,			meshOps,	"autoEdge");

	def_struct_primitive			( mo_showNormalOp,			meshOps,	"showNormal");
	def_struct_primitive			( mo_opAutoSmooth,			meshOps,	"autoSmooth");
	def_struct_primitive			( mo_attachList,			meshOps,	"attachList");
	def_struct_primitive			( mo_opSelectByID,			meshOps,	"selectByID");
	def_struct_primitive			( mo_opSelectBySG,			meshOps,	"selectBySG");
	def_struct_primitive			( mo_opClearAllSG,			meshOps,	"clearAllSG");
	def_struct_primitive			( mo_opSelectByColor,		meshOps,	"selectByColor");

/* poly ops -- if baseobject is a poly */

	def_node_generic				( convertToPoly,"convertToPoly");  // this works on those things convertable to polys

/* shape ops -- if baseobject is a shape */

	def_node_generic_debug_ok		( pathinterp,			"pathInterp");			// MAX path interpolation					(subcurve piecewise)
	def_node_generic_debug_ok		( lengthinterp,			"lengthInterp");		// total arclength interpolation
	def_visible_primitive			( resetlengthinterp,	"resetLengthInterp");  // clear length interp caches
	def_node_generic_debug_ok		( curvelength,			"curveLength");			// total arclength interpolation
	def_node_generic_debug_ok		( nearestpathparam,		"nearestPathParam");    // path 'u' param at nearest point along curve to obj
	def_node_generic_debug_ok		( pathtolengthparam,	"pathToLengthParam");  // give length 'u' param from path 'u' param
	def_node_generic_debug_ok		( lengthtopathparam,	"lengthToPathParam");  // give path 'u' param from length 'u' param
	def_node_generic_debug_ok		( pathtangent,			"pathTangent");			// MAX path interpolation tangent					(subcurve piecewise)
	def_node_generic_debug_ok		( lengthtangent,		"lengthTangent");		// total arclength interpolation tangent

/* bezier shape ops -- if baseobject is a bezier shape */

	def_node_generic				( convertToSplineShape,	"convertToSplineShape");  // this works on those things convertable to splineshapes
	
	def_node_generic				( addNewSpline,			"addNewSpline");		
	def_node_generic				( deleteSpline,			"deleteSpline");		
	def_node_generic_debug_ok		( numSplines,			"numSplines");
	def_node_generic				( setFirstSpline,		"setFirstSpline");
	def_node_generic				( resetShape,			"resetShape");
	def_node_generic				( updateShape,			"updateShape");
	
	def_node_generic_debug_ok		( numKnots,				"numKnots");		
	def_node_generic_debug_ok		( numSegments,			"numSegments");		
	def_node_generic_debug_ok		( isClosed,				"isClosed");		
	use_generic						( close,				"close");		
	def_node_generic				( open,					"open");		
	def_node_generic				( addKnot,				"addKnot");		
	def_node_generic				( deleteKnot,			"deleteKnot");		
	def_node_generic				( setKnotType,			"setKnotType");		
	def_node_generic_debug_ok		( getKnotType,			"getKnotType");		
	def_node_generic				( setSegmentType,		"setSegmentType");		
	def_node_generic_debug_ok		( getSegmentType,		"getSegmentType");		
	def_node_generic				( refineSegment,		"refineSegment");		
	def_node_generic				( reverse,				"reverse");		
	def_node_generic				( setFirstKnot,			"setFirstKnot");		
	def_node_generic				( setKnotPoint,			"setKnotPoint");		
	def_node_generic_debug_ok		( getKnotPoint,			"getKnotPoint");		
	def_node_generic_debug_ok		( getInVec,				"getInVec");		
	def_node_generic				( setInVec,				"setInVec");		
	def_node_generic_debug_ok		( getOutVec,			"getOutVec");		
	def_node_generic				( setOutVec,			"setOutVec");
	def_node_generic				( hideSelectedVerts,	"hideSelectedVerts");
	def_node_generic				( hideSelectedSplines,	"hideSelectedSplines");
	def_node_generic				( hideSelectedSegments,	"hideSelectedSegments");
	def_node_generic				( unhideSegments,		"unhideSegments");
	def_node_generic				( updateBindList,		"updateBindList");
	def_node_generic				( unbindKnot,			"unbindKnot");
	def_node_generic				( bindKnot,				"bindKnot");
	def_node_generic				( materialID,			"materialID");
	def_node_generic				( addAndWeld,			"addAndWeld");


	def_visible_primitive_debug_ok	( getKnotSelection,		"getKnotSelection");  // getKnotSelection <node> works only for editable splines
	def_visible_primitive			( setKnotSelection,		"setKnotSelection"); 
	def_visible_primitive_debug_ok	( getSegSelection,		"getSegSelection");  
	def_visible_primitive			( setSegSelection,		"setSegSelection");  
	def_visible_primitive_debug_ok	( getSplineSelection,	"getSplineSelection");  
	def_visible_primitive			( setSplineSelection,	"setSplineSelection");  

	def_struct_primitive			( so_startCreateLine,	splineOps,	"startCreateLine");
	def_struct_primitive			( so_startAttach,		splineOps,	"startAttach");
	def_struct_primitive			( so_startInsert,		splineOps,	"startInsert");
	def_struct_primitive			( so_startConnect,		splineOps,	"startConnect");
	def_struct_primitive			( so_startRefine,		splineOps,	"startRefine");
	def_struct_primitive			( so_startFillet,		splineOps,	"startFillet");
	def_struct_primitive			( so_startChamfer,		splineOps,	"startChamfer");
	def_struct_primitive			( so_startBind,			splineOps,	"startBind");
	def_struct_primitive			( so_startRefineConnect,splineOps,	"startRefineConnect");
	def_struct_primitive			( so_startOutline,		splineOps,	"startOutline");
	def_struct_primitive			( so_startTrim,			splineOps,	"startTrim");
	def_struct_primitive			( so_startExtend,		splineOps,	"startExtend");
	def_struct_primitive			( so_startCrossInsert,	splineOps,	"startCrossInsert");
	def_struct_primitive			( so_startBreak,		splineOps,	"startBreak");
	def_struct_primitive			( so_startUnion,		splineOps,	"startUnion");
	def_struct_primitive			( so_startSubtract,		splineOps,	"startSubtract");
	def_struct_primitive			( so_startIntersect,	splineOps,	"startIntersect");
	def_struct_primitive			( so_startCrossSection,	splineOps,	"startCrossSection");
	def_struct_primitive			( so_startCopyTangent,	splineOps,	"startCopyTangent");
	def_struct_primitive			( so_startPasteTangent,	splineOps,	"startPasteTangent");

	def_struct_primitive			( so_opHide,			splineOps,	"hide");
	def_struct_primitive			( so_opUnhideAll,		splineOps,	"unhideAll");
	def_struct_primitive			( so_opDelete,			splineOps,	"delete");
	def_struct_primitive			( so_opDetach,			splineOps,	"detach");
	def_struct_primitive			( so_opDivide,			splineOps,	"divide");
	def_struct_primitive			( so_opCycle,			splineOps,	"cycle");
	def_struct_primitive			( so_opUnbind,			splineOps,	"unbind");
	def_struct_primitive			( so_opWeld,			splineOps,	"weld");
	def_struct_primitive			( so_opMakeFirst,		splineOps,	"makeFirst");
	def_struct_primitive			( so_opAttachMultiple,	splineOps,	"attachMultiple");
	def_struct_primitive			( so_opExplode,			splineOps,	"explode");
	def_struct_primitive			( so_opReverse,			splineOps,	"reverse");
	def_struct_primitive			( so_opClose,			splineOps,	"close");
	def_struct_primitive			( so_opIntersect,		splineOps,	"intersect");
	def_struct_primitive			( so_opMirrorHoriz,		splineOps,	"mirrorHoriz");
	def_struct_primitive			( so_opMirrorVert,		splineOps,	"mirrorVert");
	def_struct_primitive			( so_opMirrorBoth,		splineOps,	"mirrorBoth");
	def_struct_primitive			( so_opSelectByID,		splineOps,	"selectByID");
	def_struct_primitive			( so_opFuse,			splineOps,	"fuse");

/* particle ops -- if baseobject is a particle system */

	def_node_generic_debug_ok		( particlecount,		"particleCount");		
	def_node_generic_debug_ok		( particlepos,			"particlePos");		
	def_node_generic_debug_ok		( particlevelocity,		"particleVelocity");		
	def_node_generic_debug_ok		( particleage,			"particleAge");	
	def_node_generic_debug_ok		( particlesize,			"particleSize");

/* patch ops */

#ifndef NO_PATCHES

	def_struct_primitive			( po_startAttach,			patchOps,	"startAttach");
	def_struct_primitive			( po_startExtrude,			patchOps,	"startExtrude");
	def_struct_primitive			( po_startBevel,			patchOps,	"startBevel");
	def_struct_primitive			( po_startBind,				patchOps,	"startBind");
	def_struct_primitive			( po_startCreate,			patchOps,	"startCreate");
	def_struct_primitive			( po_startWeldTarget,		patchOps,	"startWeldTarget");
	def_struct_primitive			( po_startFlipNormalMode,	patchOps,	"startFlipNormalMode");
	def_struct_primitive			( po_startCopyTangent,		patchOps,	"startCopyTangent");
	def_struct_primitive			( po_startPasteTangent,		patchOps,	"startPasteTangent");

	def_struct_primitive			( po_opUnbind,				patchOps,	"unbind");
	def_struct_primitive			( po_opHide,				patchOps,	"hide");
	def_struct_primitive			( po_opUnhideAll,			patchOps,	"unhideAll");
	def_struct_primitive			( po_opWeld,				patchOps,	"weld");
	def_struct_primitive			( po_opDelete,				patchOps,	"delete");
	def_struct_primitive			( po_opSubdivide,			patchOps,	"subdivide");
	def_struct_primitive			( po_opAddTri,				patchOps,	"addTri");
	def_struct_primitive			( po_opAddQuad,				patchOps,	"addQuad");
	def_struct_primitive			( po_opDetach,				patchOps,	"detach");
	def_struct_primitive			( po_opPatchSmooth,			patchOps,	"patchSmooth");

	def_struct_primitive			( po_opSelectionShrink,		patchOps,	"shrinkSelection");
	def_struct_primitive			( po_opSelectionGrow,		patchOps,	"growSelection");
	def_struct_primitive			( po_opEdgeRingSel,			patchOps,	"selectEdgeRing");
	def_struct_primitive			( po_opEdgeLoopSel,			patchOps,	"selectEdgeLoop");
	def_struct_primitive			( po_opSelectOpenEdges,		patchOps,	"selectOpenEdges");
	def_struct_primitive			( po_opBreak,				patchOps,	"break");
	def_struct_primitive			( po_opCreateShapeFromEdges,patchOps,	"createShapeFromEdges");
	def_struct_primitive			( po_opFlipNormal,			patchOps,	"flipNormal");
	def_struct_primitive			( po_opUnifyNormal,			patchOps,	"unifyNormal");
	def_struct_primitive			( po_opSelectByID,			patchOps,	"selectByID");
	def_struct_primitive			( po_opSelectBySG,			patchOps,	"selectBySG");
	def_struct_primitive			( po_opClearAllSG,			patchOps,	"clearAllSG");
	def_struct_primitive			( po_opShadedFaceToggle,	patchOps,	"toggleShadedFaces");

/* patch access */

	def_struct_primitive_debug_ok	( p_getNumVerts,		patch,	"getNumVerts");
	def_struct_primitive			( p_setNumVerts,		patch,	"setNumVerts");
	def_struct_primitive_debug_ok	( p_getNumVecs,			patch,	"getNumVecs");
	def_struct_primitive			( p_setNumVecs,			patch,	"setNumVecs");
	def_struct_primitive_debug_ok	( p_getNumPatches,		patch,	"getNumPatches");
	def_struct_primitive			( p_setNumPatches,		patch,	"setNumPatches");
	def_struct_primitive_debug_ok	( p_getNumEdges,		patch,	"getNumEdges");
	def_struct_primitive			( p_setNumEdges,		patch,	"setNumEdges");
	def_struct_primitive_debug_ok	( p_getVert,			patch,	"getVert");
	def_struct_primitive_debug_ok	( p_getVec,				patch,	"getVec");
	def_struct_primitive			( p_setVert,			patch,	"setVert");
	def_struct_primitive			( p_setVec,				patch,	"setVec");
	def_struct_primitive_debug_ok	( p_getVertVecs,		patch,	"getVertVecs");
	def_struct_primitive_debug_ok	( p_getVertPatches,		patch,	"getVertPatches");
	def_struct_primitive_debug_ok	( p_getVertEdges,		patch,	"getVertEdges");
	def_struct_primitive_debug_ok	( p_getVecVert,			patch,	"getVecVert");
	def_struct_primitive_debug_ok	( p_getVecPatches,		patch,	"getVecPatches");
	def_struct_primitive_debug_ok	( p_getEdgeVert1,		patch,	"getEdgeVert1");
	def_struct_primitive_debug_ok	( p_getEdgeVert2,		patch,	"getEdgeVert2");
	def_struct_primitive_debug_ok	( p_getEdgeVec12,		patch,	"getEdgeVec12");
	def_struct_primitive_debug_ok	( p_getEdgeVec21,		patch,	"getEdgeVec21");
	def_struct_primitive			( p_setNumMaps,			patch,	"setNumMaps");
	def_struct_primitive_debug_ok	( p_getNumMaps,			patch,	"getNumMaps");
	def_struct_primitive			( p_setMapSupport,		patch,	"setMapSupport");
	def_struct_primitive_debug_ok	( p_getMapSupport,		patch,	"getMapSupport");
	def_struct_primitive_debug_ok	( p_maxMapChannels,		patch,	"maxMapChannels");
	def_struct_primitive			( p_setNumMapVerts,		patch,	"setNumMapVerts");
	def_struct_primitive_debug_ok	( p_getNumMapVerts,		patch,	"getNumMapVerts");
	def_struct_primitive			( p_setNumMapPatches,	patch,	"setNumMapPatches");
	def_struct_primitive_debug_ok	( p_getMapVert,			patch,	"getMapVert");
	def_struct_primitive_debug_ok	( p_getMapPatch,		patch,	"getMapPatch");
	def_struct_primitive			( p_setMapVert,			patch,	"setMapVert");
	def_struct_primitive			( p_setMapPatch,		patch,	"setMapPatch");
// LAM 3/1/01 - removed following - these are messing with the graphics window material,
// not the"material" material
//	def_struct_primitive_debug_ok	( p_getMtlID,			patch,	"getMtlID");
//	def_struct_primitive			( p_setMtlID,			patch,	"setMtlID");
	def_struct_primitive_debug_ok	( p_getPatchMtlID,		patch,	"getPatchMtlID");
	def_struct_primitive			( p_setPatchMtlID,		patch,	"setPatchMtlID");
	def_struct_primitive			( p_update,				patch,	"update");
	def_struct_primitive			( p_setMeshSteps,		patch,	"setMeshSteps");
	def_struct_primitive_debug_ok	( p_getMeshSteps,		patch,	"getMeshSteps");
#ifndef NO_OUTPUTRENDERER
	def_struct_primitive			( p_setMeshStepsRender,	patch,	"setMeshStepsRender");
	def_struct_primitive_debug_ok	( p_getMeshStepsRender,	patch,	"getMeshStepsRender");
#endif // NO_OUTPUTRENDERER
	def_struct_primitive			( p_setShowInterior,	patch,	"setShowInterior");
	def_struct_primitive_debug_ok	( p_getShowInterior,	patch,	"getShowInterior");
	def_struct_primitive			( p_setAdaptive,		patch,	"setAdaptive");
	def_struct_primitive_debug_ok	( p_getAdaptive,		patch,	"getAdaptive");
	def_struct_primitive	( p_getEdges,			patch,	"getEdges");
	def_struct_primitive_debug_ok	( p_getPatches,			patch,	"getPatches");
	def_struct_primitive_debug_ok	( p_getVectors,			patch,	"getVectors");
	def_struct_primitive			( p_transform,			patch,	"transform");
	def_struct_primitive			( p_weldVerts,			patch,	"weldVerts");
	def_struct_primitive			( p_weldEdges,			patch,	"weldEdges");
	def_struct_primitive			( p_weld2Verts,			patch,	"weld2Verts");
	def_struct_primitive			( p_deletePatchParts,	patch,	"deletePatchParts");
	def_struct_primitive			( p_clonePatchParts,	patch,	"clonePatchParts");
	def_struct_primitive			( p_subdivideEdges,		patch,	"subdivideEdges");
	def_struct_primitive			( p_subdividePatches,	patch,	"subdividePatches");
	def_struct_primitive			( p_addQuadPatch,		patch,	"addQuadPatch");
	def_struct_primitive			( p_addTriPatch,		patch,	"addTriPatch");
	def_struct_primitive			( p_patchNormal,		patch,	"patchNormal");
	def_struct_primitive			( p_updatePatchNormals,	patch,	"updatePatchNormals");
	def_struct_primitive			( p_edgeNormal,			patch,	"edgeNormal");
	def_struct_primitive			( p_flipPatchNormal,	patch,	"flipPatchNormal");
	def_struct_primitive			( p_unifyNormals,		patch,	"unifyNormals");
	def_struct_primitive			( p_changeVertType,		patch,	"changeVertType");
	def_struct_primitive_debug_ok	( p_getVertType,		patch,	"getVertType");
	def_struct_primitive			( p_changePatchInteriorType,patch,	"changePatchInteriorType");
	def_struct_primitive_debug_ok	( p_getPatchInteriorType,   patch,	"getPatchInteriorType");
	def_struct_primitive_debug_ok	( p_getMesh,			patch,	"getMesh");
	def_struct_primitive			( p_autoSmooth,			patch,	"autoSmooth");
	def_struct_primitive			( p_makeQuadPatch,		patch,	"makeQuadPatch");
	def_struct_primitive			( p_makeTriPatch,		patch,	"makeTriPatch");
	def_struct_primitive			( p_interpTriPatch,		patch,	"interpTriPatch");
	def_struct_primitive			( p_interpQuadPatch,	patch,	"interpQuadPatch");
	def_struct_primitive			( p_addHook,			patch,	"addHook");
	def_struct_primitive			( p_removeHook,			patch,	"removeHook");
	def_struct_primitive			( p_updateHooks,		patch,	"updateHooks");
	def_struct_primitive			( p_hookFixTopology,	patch,	"hookFixTopology");
	def_struct_primitive_debug_ok	( p_getPatchType,		patch,	"getPatchType");

#endif // NO_PATCHES

	// xrefs
	def_struct_primitive			( update_changed_xrefs,		xrefs,"updateChangedXRefs");
	def_struct_primitive_debug_ok	( get_xref_file,			xrefs,"getXRefFile");
	def_struct_primitive_debug_ok	( get_xref_file_count,		xrefs,"getXRefFileCount");
	def_struct_primitive			( add_xref_file,			xrefs,"addNewXRefFile");
	def_struct_primitive			( delete_all_xrefs,			xrefs,"deleteAllXRefs");
	def_struct_primitive			( find_unresolved_xrefs,	xrefs,"findUnresolvedXRefs");
	def_struct_primitive			( attempt_unresolved_xrefs,	xrefs,"attemptUnresolvedXRefs");
	def_struct_primitive			( add_xref_object,			xrefs,"addNewXRefObject");

#pragma warning(pop)

