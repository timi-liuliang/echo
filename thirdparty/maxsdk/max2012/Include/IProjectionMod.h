/**********************************************************************
 *<
	FILE: IProjectionMod.h

	DESCRIPTION: Interface for Projection Modifier

	CREATED BY:  Michael Russo

	HISTORY:     02-23-2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/

#pragma once

#include "maxheap.h"
#include "ObjectWrapper.h"
// forward declarations
class IProjectionModData;

#pragma warning(push)
#pragma warning(disable:4100)

#define PROJECTIONMOD_CLASS_ID			Class_ID(0x166c301c, 0x19f90c49)
#define IPROJECTIONMOD_INTERFACE_ID		Interface_ID(0x6b231b96, 0xb26a72)
#define IPROJECTIONMOD10_INTERFACE_ID	Interface_ID(0x5a9803fa, 0x40216bb5)
#define IPROJECTIONMODDATA_INTERFACE_ID	Interface_ID(0x341e47bc, 0x1f923a2f)

//=============================================================================
//
//	Class IProjectionMod
/*! \sa Class IProjectionModData, Class IProjectionModType
	\par Description:
	provides an interface to the Projection Modifier. This interface gives access to the geometry
	selections, cage settings, validity checks, and projection types.
*/
//=============================================================================
class IProjectionMod : public FPMixinInterface
{
public:
	// IProjectionMod

	// Geometry Selection Methods
	enum { PM_SL_OBJECT, PM_SL_VERT, PM_SL_FACE, PM_SL_ELEM };

	// Access to Geom Selections by Node/Object Index

	//! \brief Number of Nodes in geometry selection.
	//! \return int number of objects.
	virtual int					NumObjects() = 0;

	//! \brief Get Object Node by Index.
	//! \param iIndex - zero based index into number of objects
	virtual INode*				GetObjectNode(int iIndex) = 0;

	//! \brief Get Object Name by Index.
	//! \param iIndex - zero based index into number of objects
	virtual const MCHAR*		GetObjectName(int iIndex) = 0;

	//! \brief Get Object Selection Level by Index.
	//! \param iIndex - zero based index into number of objects
	//! \return enum PM_SL_OBJECT, PM_SL_FACE, or PM_SL_ELEM
	virtual int					GetObjectSelLevel(int iIndex) = 0;

	//! \brief Delete Object by Index.
	//! \param iIndex - zero based index into number of objects
	virtual void				DeleteObjectNode(int iIndex) = 0;

	// Access to Geom Selections by Geom Selections

	//! \brief Number of Geometry Selection.
	//! \return int number of geometry selections.
	virtual int					NumGeomSels() = 0;

	//! \brief Get Geometry Selection Name by Index.
	//! \param iSelIndex - zero based index into number of geometry selections
	virtual const MCHAR*		GetGeomSelName( int iSelIndex ) = 0;

	//! \brief Get Geometry Selection Selection Level by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \return enum PM_SL_OBJECT, PM_SL_FACE, or PM_SL_ELEM
	virtual int					GetGeomSelSelLevel( int iSelIndex ) = 0;

	//! \brief Get number of nodes in Geometry Selection by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \return int number of nodes
	virtual int					NumGeomSelNodes( int iSelIndex ) = 0;

	//! \brief Get Node in Geometry Selection by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \param[in] iNodeIndex zero based index into number of nodes in geometry selections
	virtual INode*				GetGeomSelNode( int iSelIndex, int iNodeIndex ) = 0;

	//! \brief Get Geometry Selection Map Proportions by Index.
	//! Used to offset the proportion this geometry selection will occupy in the generated bake element
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \return float proportion adjustment
	virtual void				SetGeomSelMapProportion( int iSelIndex, float fProportion ) = 0;

	//! \brief Delete node from Geometry Selection by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \param[in] iNodeIndex zero based index into number of nodes in geometry selections
	virtual float				GetGeomSelMapProportion( int iSelIndex ) = 0;

	//! \brief Delete Geometry Selection by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	virtual void				DeleteGeomSel( int iSelIndex ) = 0;

	//! \brief Delete node from Geometry Selection by Index.
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \param[in] iNodeIndex zero based index into number of nodes in geometry selections
	virtual void				DeleteGeomSelNode( int iSelIndex, int iNodeIndex ) = 0;

	//! \brief Retrieve the IProjectionModData for the specified node.
	//! Access to ModData for selection set and cage information
	//! \param[in] pNode INode that contains Projection Modifier
	virtual IProjectionModData*	GetProjectionModData(INode *pNode) = 0;

	// Modifier Access

	//! \brief Is this a valid node for geometry selections.
	//! \param[in] pNode INode*
	//! \return bool true if this is a valid geometry selection object
	virtual bool				IsValidObject( INode *pNode ) = 0;

	//! \brief Add Object Level Node.
	//! \param[in] pNode INode*
	//! \return bool true if node was successfully added
	virtual bool				AddObjectNode( INode *pNode ) = 0;

	//! \brief Delete all geometry selections
	virtual void				DeleteAll() = 0;

	//! \brief Enable/Disable the visibility of the geometry selection nodes.
	//! \param[in] bVisible bool true/false
	virtual void				SetGeomSelNodesVisibility( bool bVisible ) = 0;

	//! \brief Get the visibility of the geometry selection nodes.
	//! If their visibility state varies, the majority state will be returned.
	//! \return bool true if the nodes are visible
	virtual bool				GetGeomSelNodesVisibility() = 0;

	//! \brief Refresh the projection type list.
	virtual void				UpdateProjectionTypeList() = 0;

	//! \brief Triggers an auto wrap cage.
	//! This will destroy the current cage and generate a new one based on the current geometry selections.
	virtual void				AutoWrapCage() = 0;

	//! \brief Resets the cage.
	//! This will destroy the current cage and return all verts back to their original/zero-offset location.
	virtual void				ResetCage() = 0;

	//! \brief Pushes the cage.
	//! This will work on the selected verts. If no verts are selected, the entire cage will be affected.
	//! \param[in] fAmount float pushes the cage by specified amount.
	virtual void				PushCage(float fAmount) = 0;

	//! \brief Get list of material IDs by Object Index.
	//! \param[in] pNode INode* that contains the projection modifier
	//! \param[in] iIndex zero based index into number of objects
	//! \param[out] mtlIDs - Tab< int> array of material IDs
	//! \return int number of material IDs
	virtual int					fpGetObjectSelMtlIDs(INode *pNode, int iIndex, Tab<int> &mtlIDs) = 0;

	//! \brief Get list of material IDs by Object Index.
	//! \param[in] pNode INode* that contains the projection modifier
	//! \param[in] iIndex zero based index into number of objects
	//! \param[out] mtlIDs - Tab< int> array of material IDs
	//! \return int number of material IDs
	virtual int					fpGetGeomSelMtlIDs(INode *pNode, int iSelIndex, Tab<int> &mtlIDs) = 0;

	//! \brief Get Selected Face surface area by Geometry Selection Index.
	//! \param[in] pNode INode* that contains the projection modifier
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \return float surface areas of faces
	virtual float				fpGetGeomSelFaceArea(INode *pNode, int iSelIndex) = 0;

	//! \brief Check for duplicate Material IDs within Geometry Selections
	//! Duplicate material IDs can create unwanted results with certain Project Mapping settings in RTT.
	//! \param[in] pNode INode* that contains the projection modifier
	//! \param[out] bitFaceDups - bitarray of face duplicates
	//! \return int Number of faces with duplicates
	virtual	int					fpCheckDuplicateMatIDs( INode *pNode, BitArray &bitFaceDups ) = 0;

	//! \brief Check for duplicate face selections within Geometry Selections
	//! Duplicate face selections can create unwanted results with certain Project Mapping settings in RTT.
	//! \param[in] pNode INode* that contains the projection modifier
	//! \param[out] bitFaceDups - bitarray of face duplicates
	//! \return int Number of faces with duplicates
	virtual	int					fpCheckDuplicateSels( INode *pNode, BitArray &bitFaceDups ) = 0;

	//! \brief Select by Material ID
	//! \param[in] iMtlID Material ID
	virtual void				fpSelectByMtlID( int iMtlID ) = 0;
	//! \brief Select by Smoothing Group
	//! \param[in] iSG Smoothing Group
	virtual void				fpSelectBySG( int iSG ) = 0;

	//! \brief Get the number of Projection Types
	//! Projection Types are classes that load into the Projection Modifier.
	//! These plugins can use the geometry selection and cage information to perform projections
	//! of any type they wish. Examples would be projecting UVWs, vertex location, material IDs, etc.
	//! \return int Number of Projection Types created by user
	virtual int					fpGetNumProjectionTypes() = 0;

	//! \brief Get Projection Type by Index
	//! \param[in] iIndex zero based index into number of Projection Types
	//! \return ReferenceTarget* ReferenceTarget pointer of created Projection Type
	virtual ReferenceTarget*	fpGetProjectionType(int iIndex) = 0;

	//! \brief Remove Projection Type by Index
	//! \param[in] iIndex zero based index into number of Projection Types
	virtual void				fpRemoveProjectionType(int iIndex) = 0;

	//! \brief Invoke Project command for Projection Type by Index
	//! This will invoke the project command for the specified projection type.
	//! \param[in] iIndex zero based index into number of Projection Types
	virtual void				fpProject(int iIndex) = 0;

	//! \brief Invoke Project command for all Projection Types
	virtual void				fpProjectAll() = 0;

	//! \brief Get the number of Registered Projection Types
	//! Registered Projection Types are classes that load into the Projection Modifier.
	//! Instances of these Registered Projection Types are called Projection Types.
	//! \return int Number of Registered Projection Types in the system.
	virtual int					fpGetNumRegisteredProjectionTypes() = 0;

	//! \brief Create an instance of a Registered Projection Type by Index
	//! \param[in] iIndex zero based index into number of Registered Projection Types
	virtual void				fpAddRegisteredProjectionType(int iIndex) = 0;

	//! \brief Get the Class ID of a specified Registered Projection Type
	//! \param[in] iIndex zero based index into number of Registered Projection Types
	//! \param[out] classid - Tab< int> containing the two Class ID values
	virtual void				fpGetRegisteredProjectionTypeClassID(int iIndex, Tab<int> &classid) = 0;

	//! \brief Enable Show Alignment feature
	virtual void				fpShowAlignment() = 0;

	//! \brief Disable/Clear Show Alignment feature
	virtual void				fpClearAlignment() = 0;
};

//=============================================================================
//
//	Class IProjectionMod10
//
//=============================================================================

class IProjectionMod10 : public FPMixinInterface
{
public:
	virtual bool				ImportCageMesh( INode* importNode ) = 0;
	virtual bool				ExportCageMesh( MCHAR* exportNodeName ) = 0;
};

//=============================================================================
//
//	Class IProjectionModData
/*! \sa Class IProjectionMod
	\par Description:
	provides an interface to the Projection Mod Data.
*/
//=============================================================================
class IProjectionModData : public BaseInterface
{
public:

	// GenericInterface
	virtual Interface_ID	GetID() { return IPROJECTIONMODDATA_INTERFACE_ID; }

	// IProjectionModData

	//! \brief Get ObjectWrapper for the Base Object.
	//! The base object refers to the object that contains the Projection Modifier
	//! \return ObjectWrapper reference to ObjectWrapper
	virtual ObjectWrapper &GetBaseObject() = 0;	// Object with Projection Modifier

	//! \brief Get ObjectWrapper for the Cage.
	//! \return ObjectWrapper reference to ObjectWrapper
	virtual ObjectWrapper &GetCage() = 0;	    // Cage mesh

	// Access to Geom Selections by Node/Object Index

	//! \brief Get the object selection by Object Index
	//! \param[in] iIndex zero based index into number of objects
	//! \param[out] sel - bitarray of face selection
	//! \return bool true if valid bitarray was returned
	virtual bool		GetObjectSel(int iIndex, BitArray& sel) = 0;

	//! \brief Get the material IDs by Object Index
	//! \param[in] iIndex zero based index into number of objects
	//! \param[out] mtlIDs - Tab< int> of material IDs in selection
	//! \return int number of material IDs found
	virtual int			GetObjectSelMtlIDs(int iIndex, Tab<int> &mtlIDs) = 0;

	// Access to Geom Selections by Geom Selections

	//! \brief Get the object selection by Geometry Selection
	//! \param[in] iSelIndex zero based index into number of geometry selections.
	//! \param[out] sel - bitarray of face selection
	//! \return bool true if valid bitarray was returned
	virtual bool		GetGeomSel(int iSelIndex, BitArray& sel) = 0;

	//! \brief Get the material IDs by Geometry Selection
	//! \param[in] iSelIndex zero based index into number of geometry selections
	//! \param[out] mtlIDs - Tab< int> of material IDs in selection
	//! \return int number of material IDs found
	virtual int			GetGeomSelMtlIDs(int iSelIndex, Tab<int> &mtlIDs) = 0;

	//! \brief Get the surface area of selection by Geometry Selection
	//! \param[in] iSelIndex zero based index into number of geometry selections.
	//! \return float surface area of face selection
	virtual float		GetGeomSelFaceArea(int iSelIndex) = 0;

	//! \brief Check for duplicate Material IDs
	//! Duplicate material IDs can create unwanted results with certain Project Mapping settings in RTT.
	//! \param[out] bitFaceDups - bitarray of face duplicates
	//! \return int Number of faces with duplicates
	virtual	int			CheckDuplicateMatIDs( BitArray &bitFaceDups ) = 0;

	//! \brief Check for duplicate face selection
	//! Duplicate face selections can create unwanted results with certain Project Mapping settings in RTT.
	//! \param[out] bitFaceDups - bitarray of face duplicates
	//! \return int Number of faces with duplicates
	virtual	int			CheckDuplicateSels( BitArray &bitFaceDups ) = 0;

};


//=============================================================================
//
//	Class IProjectionModType
//
//	Class Category: ProjectionModType
/*! \sa Class IProjectionMod
	\par Description
	provides an interface to the Projection Modifier Type Plugins. Projection 
	Modifier Types are classes that load into the Projection Modifier. These 
	classes can use the geometry selection and cage information to perform 
	projections of any type they wish. Examples would be projecting UVWs, vertex 
	location, material IDs, etc.\n\n
	These classes must specify a class category of: ProjectionModType
*/
//=============================================================================
class IProjectionModType : public ReferenceTarget
{
public:

	//! \brief Sets IProjectionMod pointer.
	//! Give the Projection Mod Type a pointer back to the Projection Modifier
	//! \param[in] IProjectionMod Pointer to projection modifier
	virtual void SetIProjectionMod( IProjectionMod *pPMod ) {}

	//! \brief Determine if the Projection Mod Type can project the geometry selection.
	//! Allows the projection modifier type to tell the projection modifiers if its
	//! projection algorithm supports the current geometry selection. If it doesn't,
	//! the projection modifier will disable the Project button for this type.
	//! \param[in] tabSourceNodes Tab< INode*> an array of nodes that contain the projection modifier
	//! \param[in] iSelIndex int index into geometry selection. -1 indicates all geometry selections.
	//! \param[in] iNodeIndex int index into the geometry selection's nodes. -1 indicates all nodes within the geometry selection.
	//! \return bool return true if the projection is supported
	virtual bool CanProject(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex) { return false; }
	
	//! \brief Specifies which geometry selections and/or nodes should be projected.
	//! This method triggers the projection mod types projection algorithm for the specified geometry selections.
	//! \param[in] tabSourceNodes Tab< INode*> an array of node that contain the projection modifier
	//! \param[in] iSelIndex int index into geometry selection. -1 indicates all geometry selections.
	//! \param[in] iNodeIndex int index into the geometry selection's nodes. -1 indicates all nodes within the geometry selection.
	virtual void Project(Tab<INode*> &tabSourceNodes, int iSelIndex, int iNodeIndex) {}

	//! \brief Tell the plugin to generate an initial name.
	//! \param[in] iIndex int An index value to help create a unique name. Unique names are not required.
	virtual void SetInitialName(int iIndex) {}

	//! \brief Get the plugin's display name
	//! \return MCHAR* name
	virtual const MCHAR *GetName(){return NULL;}

	//! \brief Specifies the new name based on user input.
	//! \param[in] name MCHAR* new name of plugin
	virtual void SetName(const MCHAR *name) {}
	// deprecated method - implement SetName(const MCHAR *name)
	virtual void SetName(MCHAR *name) { SetName(const_cast<const MCHAR*>(name)); }

	//! \brief Allows the projection modifier to enable or disable a projection mod type plugin
	//! Class should assume they are enabled unless notified otherwise.
	//! \param[in] enable bool true if enable
	virtual void Enable(bool enable) {}

	//! \brief Specifies if the projection mod type is currently in an editing mode.
	//! \return bool return true if in editing mode.
	virtual bool IsEditing() { return false; }

	//! \brief Forces a projection mod type plugin to end editing
	virtual void EndEditing() {}

	//! \brief Allows the projection modifier to enable or disable the ability to edit within a projection mod type plugin
	//! Class should assume they are enabled for editing unless notified otherwise.
	//! \param[in] enable bool true if enable
	virtual void EnableEditing(bool enable) {}

	//! \brief NotifyRefChanged Method Pass Through
	//! Calls to NotifyRefChanged in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa Class ReferenceMaker
	virtual RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message )	
		{ return REF_SUCCEED; }

	//! \brief Load Method Pass Through
	//! Calls to Load in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa Class ReferenceMaker::Load
    virtual IOResult Load(ILoad *iload) {return IO_OK;}
	//! \brief Save Method Pass Through
	//! Calls to Save in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa ReferenceMaker::Save
    virtual IOResult Save(ISave *isave) {return IO_OK;}

	//! \brief ModifyObject Method Pass Through
	//! Calls to ModifyObject in the Projection Modifier are passed through this method to the projection mod types.
	//! The corresponding IProjectionModData is an additional parameter.
	//! \sa Modifier::ModifyObject
	virtual void ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *inode, IProjectionModData *pPModData) {}

	//! \brief BeginEditParams Method Pass Through
	//! Calls to BeginEditParams in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa Animatable::BeginEditParams
    virtual void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev) {}
	//! \brief EndEditParams Method Pass Through
	//! Calls to EndEditParams in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa Animatable::EndEditParams
    virtual void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next) {}         

	//! \brief NotifyInputChanged Method Pass Through
	//! Calls to NotifyInputChanged in the Projection Modifier are passed through this method to the projection mod types.
	//! The corresponding IProjectionModData is an addition parameter.
	//! \sa Modifier::NotifyInputChanged
	virtual void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc, IProjectionModData *pPModData) {}

	//! \brief GetWorldBoundBox Method Pass Through
	//! Calls to GetWorldBoundBox in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::GetWorldBoundBox
	virtual void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc){}

	//! \brief Display Method Pass Through
	//! Calls to Display in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::Display
	virtual int Display(TimeValue t, GraphicsWindow *gw, Matrix3 tm ) { return 1;}

	//! \brief HitTest Method Pass Through
	//! Calls to HitTest in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::HitTest
    virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc, Matrix3 tm) {return 0;}

	//! \brief SelectSubComponent Method Pass Through
	//! Calls to SelectSubComponent in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::SelectSubComponent
    virtual void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) {}

	//! \brief Move Method Pass Through
	//! Calls to Move in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::Move
    virtual void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, Matrix3 tm, BOOL localOrigin=FALSE ) {}

	//! \brief GetSubObjectCenters Method Pass Through
    //! Calls to GetSubObjectCenters in the Projection Modifier are passed through this method to the projection mod types.
    //! \sa BaseObject::GetSubObjectCenters
    virtual void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}

	//! \brief GetSubObjectTMs Method Pass Through
	//! Calls to GetSubObjectTMs in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::GetSubObjectTMs
    virtual void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node, Matrix3 tm) {}

	//! \brief ClearSelection Method Pass Through
	//! Calls to ClearSelection in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::ClearSelection
    virtual void ClearSelection(int selLevel) {}

	//! \brief SelectAll Method Pass Through
	//! Calls to SelectAll in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::SelectAll
	virtual void SelectAll(int selLevel) {}

	//! \brief InvertSelection Method Pass Through
	//! Calls to InvertSelection in the Projection Modifier are passed through this method to the projection mod types.
	//! \sa BaseObject::InvertSelection
    virtual void InvertSelection(int selLevel) {}
	
};

//=============================================================================
//
//	Class IProjectionIntersector
//
//	Ray-intersection for projection mapping
/*! \par Description
	provides an interface to the Projection Intersector which provides ray-tracing tools intended for use by the plug-ins.
*/
//=============================================================================
class IProjectionIntersector: public MaxHeapOperators {
	public:
		IProjectionIntersector() {}
		virtual			~IProjectionIntersector() {}
		virtual	void	DeleteThis() {}

		/*! \param root - The root is normally the object on which the projection modifier is applied.
			\param cage - The cage is optional, it may be NULL or empty; 
				if supplied, the cage must have the same topology as the root. */
		virtual	BOOL	InitRoot( MNMesh* root,			MNMesh* cage,			Matrix3& objectToWorldTM ) = 0;
		/*! \param root - The root is normally the object on which the projection modifier is applied.
			\param cage - The cage is optional, it may be NULL or empty; 
				if supplied, the cage must have the same topology as the root. */
		virtual	BOOL	InitRoot( ObjectWrapper& root,	ObjectWrapper& cage,	Matrix3& objectToWorldTM ) = 0;

		/*! \return true if initialized, false when free'd */
		virtual BOOL	RootValid() = 0;

		/*! \param leaf - A leaf is normally one of the objects targeted by the projection modifier. */
		virtual BOOL	InitLeaf( MNMesh* leaf,			Matrix3& objectToWorldTM ) = 0;
		/*! \param leaf - A leaf is normally one of the objects targeted by the projection modifier. */
		virtual BOOL	InitLeaf( ObjectWrapper& leaf,	Matrix3& objectToWorldTM ) = 0;
		/*! \return true if initialized, false when free'd */
		virtual BOOL	LeafValid() = 0;

		/*! \remarks releases memory allocated by the Init functions */
		virtual void	Free() = 0;

		enum {FINDCLOSEST_CAGED=1, FINDCLOSEST_IGNOREBACKFACING=2};
		/*! \remarks Given a point in root object space, and a normal for the point (optional - may be zero),
			find the closest face, and barycentric coordinates of the point as projected onto the face
			\pre Root functions: Require the root be initialized first */
		virtual BOOL	RootClosestFace( Ray& pointAndNormal, int flags, float& dist, DWORD& rootFaceIndex, Tab<float>& rootFaceBary ) = 0;

		/*! \remarks Find an intersection on a leaf, given a ray
			\pre Require a leaf be initialized first */
		virtual BOOL	LeafIntersectRay( Ray& ray, float& dist, DWORD& leafFaceIndex, Tab<float>& leafFaceBary ) = 0;

		/*! \remarks Find an intersection on a leaf, given a point on the root mesh
			\pre Require that BOTH the root and a leaf are initialized */
		virtual BOOL	LeafIntersectRay( DWORD rootFaceIndex, Point3& rootFaceBary, float& dist, DWORD& leafFaceIndex, Tab<float>& leafFaceBary ) = 0;
};

// Manager class, creates instances of IProjectionIntersector
class IProjectionIntersectorMgr : public FPStaticInterface {
	public:
		virtual IProjectionIntersector* CreateProjectionIntersector() = 0;
};

#define IPROJECTIONINTERSECTORMGR_INTERFACE_ID Interface_ID(0x457d402d, 0x36e81f43)
#define GetIProjectionIntersectorMgr() \
	((IProjectionIntersectorMgr*)GetCOREInterface(IPROJECTIONINTERSECTORMGR_INTERFACE_ID))

#pragma warning(pop)


