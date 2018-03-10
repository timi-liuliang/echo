//
// Copyright 2010 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form.   
//
//

#pragma once

#include "./SmartHandle.h"
#include "./GeometryEnums.h"
#include "../containers/Array.h"

class Mesh;
class MNMesh;

namespace MaxSDK { namespace Graphics {

/** A renderable plugin object could be consist of several render items. Each 
render item belongs to a category. Render items that belongs to different category 
will be visible when the node and viewport has specific settings. 
*/
enum RenderItemCategory
{
	/** Solid Mesh, it could be: vertex color mesh or general mesh. This will 
	be displayed in any viewport that requires smooth mesh.
	*/
	RenderItemCategorySolidMesh              = 0x00000001, 

	/** Diagonal Mesh. This is the dotted edge lines when user enabled "show 
	edges" of a node and enable the wireframe mode of viewport.
	*/
	RenderItemCategoryDiagonalMesh           = 0x00000002, 

	/** Edge mesh, this layer includes the edges. This will be displayed in 
	viewport that has "wireframe" or "hidden line" on. 
	*/
	RenderItemCategoryEdgeMesh               = 0x00000004, 

	/** Vertex ticks mesh. This will be displayed when the node has "Vertex Ticks" 
	display option on or the vertex tick sub-object be selected.
	*/
	RenderItemCategorySubObjectVertexTicks   = 0x00000008, 

	/** Sub object edge mesh. The difference between this kind of render item 
	and the edge mesh render item is that this kind of render item highlight 
	selected edges, and only be displayed when the node is selected.
	*/
	RenderItemCategorySubObjectEdgeMesh      = 0x00000010, 

	/** Other sub objects. This will be treated as gizmos and will only be 
	displayed when the node is selected.
	*/
	RenderItemCategoryOtherSubObjects        = 0x00000020, 

	/** Gizmos part. This layer includes all other renderable items, like slice 
	plane, mouse helper, etc. This kind of render items will be displayed in 
	all viewports.
	*/
	RenderItemCategoryGizmo                  = 0x00000040, 
};

/** This is the base wrapper class for all viewport render items. A render item 
is the primary drawable element that holds the geometry. A plugin object that 
wants to be displayed in the viewport needs to create and maintain a set of 
concrete derived render items like TupleMeshRenderItem and expose those render 
items to max via the IObjectDisplay interface. 
\sa Class IObjectDisplay, IGeometryDisplay
*/
class RenderItemHandle : public SmartHandle
{
public:
	GraphicsDriverAPI RenderItemHandle();
	GraphicsDriverAPI RenderItemHandle(const RenderItemHandle& from);
	GraphicsDriverAPI RenderItemHandle& operator = (const RenderItemHandle& from);
	GraphicsDriverAPI virtual ~RenderItemHandle();

	/** Sets the category of the render item.
	\remarks Each render item belongs to a category. category controls the visibility 
	of the render item at a specific render category pass. For example: 
	Wireframe pass. For more information please see enum RenderItemCategory.

	By default the category of a render item is RenderItemCategoryGizmo.
	\param category The category of this render item.
	*/
	GraphicsDriverAPI void SetCategory(RenderItemCategory category);

	/** Gets the category of the render item.
	\return the category of the render item.
	*/
	GraphicsDriverAPI RenderItemCategory GetCategory() const;

	/** \deprecated Deprecated in 3ds Max 2012. No replacement.
	Set custom material for render item.
	By default, render item uses the material from node. When a custom material 
	is set, the render item will use the set material.
	\param hMaterial The material handle.
	*/
	MAX_DEPRECATED GraphicsDriverAPI void SetCustomMaterial(const BaseMaterialHandle& hMaterial);

	/** \deprecated Deprecated in 3ds Max 2012. No replacement.
	Get custom material for render item.
	\return the custom material.
	*/
	MAX_DEPRECATED GraphicsDriverAPI BaseMaterialHandle GetCustomMaterial() const;

};

/** This is a utility class. It is used for storing a set of render items.
*/
typedef MaxSDK::Array<RenderItemHandle> RenderItemHandleArray;

/** Generate a set of render items from an input mesh and viewport requirements.
\remarks Usually plugins use this function in their UpdateDisplay(). If plugins maintain 
	instances of Mesh, then they should use this function to convert those mesh instances 
	to render items (RenderItemHandle).
\param renderItems the container that accept newly generated render items.
\param pMaxMesh the input mesh to be converted to render items.
\param renderItemCategoryFlags defines the visible render item categories. It is 
	a combination of RenderItemCategory
\param requiredStreams defines the required mesh channel. This parameter defines 
	which channel (for example: normal channel, texture coordinate channel, etc.) 
	will be added to the final render item.
*/
GraphicsDriverAPI void GenerateRenderItems(
	RenderItemHandleArray& renderItems, 
	Mesh* pMaxMesh, 
	unsigned long renderItemCategoryFlags, 
	const MaterialRequiredStreams& requiredStreams);

/** Generate a set of render items from an input MNMesh and viewport requirements.
\remarks Usually plugins use this function in their UpdateDisplay(). If plugins maintain 
	instances of MNMesh, then they should use this function to convert those MNMesh 
	instances to render items (RenderItemHandle). Converting MNMesh to Mesh via 
	MNMesh::OutToTri() and then use the Mesh version of GenerateRenderItems is possible 
	but not recommended because it causes unnecessary conversions and is very slow.
\param renderItems the container that accept newly generated render items.
\param pMaxMesh the input MNMesh to be converted to render items.
\param renderItemCategoryFlags defines the visible render item categories. It is 
	a combination of RenderItemCategory
\param requiredStreams defines the required mesh channel. This parameter defines 
	which channel (for example: normal channel, texture coordinate channel, etc.) 
	will be added to the final render item.
*/
GraphicsDriverAPI void GenerateRenderItems(
	RenderItemHandleArray& renderItems, 
	MNMesh* pMaxMesh, 
	unsigned long renderItemCategoryFlags, 
	const MaterialRequiredStreams& requiredStreams);
} } // end namespace
