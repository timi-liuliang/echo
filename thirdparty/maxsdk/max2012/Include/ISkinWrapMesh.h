#pragma once

#include "ifnpub.h"

#define MESHDEFORMPW_CLASS_ID	Class_ID(0x22b7bd09, 0x673ac5cf)
#define MESHDEFORMPW_INTERFACE Interface_ID(0xDE21A34f, 0x8A43E3D2)

/*! IMeshDeformPWMod is the interface into the Skin Wrap(Mesh) modifier */
class IMeshDeformPWMod : public FPMixinInterface
{
public:
	FPInterfaceDesc* GetDesc(); 

	/*! \brief This selects the control point cage vertices
	\param whichWrapMesh - this is which wrap mesh you are getting the selection from
						  each wrap mesh keeps it own selection list
	\param selList - the bitarray representing the selection
	\param updateView - whether to update the view and the modifier */
	virtual void	SelectVertices(int whichWrapMesh, BitArray *selList, BOOL updateViews)=0;
	
	/*! \brief This returns the current selected control points on the cage
	\param whichWrapMesh - this is which wrap mesh you are setting the selection to
							  each wrap mesh keeps it own selection list */
	virtual BitArray *GetSelectedVertices(int whichWrapMesh)=0;

	/*! \brief This returns the number of control points on the cage
	\param whichWrapMesh - this is which wrap mesh you are getting the number of control pointss from */
	virtual int GetNumberControlPoints(int whichWrapMesh)=0;

	//! \brief This returns the local scale factor for a control point.
	/*! The total influence area is equal to the control points distance * global distance * local scale
	\param whichWrapMesh - this is which wrap mesh you are getting the info from
	\param index - this is which control point you want to get */
	virtual Point3 *GetPointScale(int whichWrapMesh,int index)=0;
	
	/*! \brief This lets you set a control points local scale
	\param whichWrapMesh - this is which wrap mesh you are setting the info to
	\param index - this is the index of the point you want to set
	\param scale - this is the scale of the point */
	virtual void SetPointScale(int whichWrapMesh,int index, Point3 scale)=0;

	/*! \brief this returns the strength of a control point
	\param whichWrapMesh - this is which wrap mesh you are getting the info from
	\param index - this is the index of the control point you want to get */
	virtual float GetPointStr(int whichWrapMesh,int index)=0;
	
	/*! \brief This lets you set the strength of a control point
	\param whichWrapMesh - this is which wrap mesh you are setting the info to
	\param index - this is the index of the control point you want to set
	\param str - this is the strength  */
	virtual void SetPointStr(int whichWrapMesh,int index, float str)=0;

	/*! \brief This returns the initial tm of the control point
	\param whichWrapMesh - this is which wrap mesh you are getting the info from
	\param index - the control point index */
	virtual Matrix3 GetPointInitialTM(int whichWrapMesh,int index)=0;
	/*! \brief This returns the current tm of the control point
	\param index - the control point index */
	virtual Matrix3 GetPointCurrentTM(int whichWrapMesh,int index)=0;
	/*! \brief This returns the size of the envelope of a control point
	\param whichWrapMesh - this is which wrap mesh you are getting the info from
	\param index - the control point index */
	virtual float GetPointDist(int whichWrapMesh,int index) = 0;
	/*! \brief this is the vertex that forms the x axis, the z axis is the normal
	\param whichWrapMesh - this is which wrap mesh you are getting the info from
	\param index - the control point index */
	virtual int GetPointXVert(int whichWrapMesh,int index) = 0;


	//! \brief This mirrors the current selected control points.
	/*! This is identical to pushing the Mirro button in the UI */
	virtual void MirrorSelectedVerts()=0;

	//! \brief This bakes the control point data into the app data of the node that is the control mesh.
	/*! This is identical to pushing the Bake button in the UI */
	virtual void BakeControlPoints()=0;

	//! \brief This retrieves the control point data from the app data of the node that is the control mesh.  
	/*! This is identical to pushing the Retrieve button in the UI */
	virtual void RetreiveControlPoints()=0;

	/*! \brief This forces the modifier to resample itself. This will force all weights to be recomputed */
	virtual void Resample()=0;

	/*! \brief same as Resample */
	virtual void SetResampleModContext() = 0;
	
	//! \brief Update the neighbouring weight lists on selection change.
	/*! The system keep tracks of a potential weight lists by using neighbor data.
	Any time a selection is changed this potential weight list needs to be updated
	use this function to update that list.  
	This should be called after any control point selection change. */
	virtual void SetRebuildNeighborDataModContext() = 0;

	/*! \brief This forces the selected control points to have their weights rebuilt
	this should be called when you change the str/scaling etc of a control point */
	virtual void SetRebuildSelectedWeightDataModContext() = 0;

	/*! \brief returns the number of deformed vertices
	\param node - the node that owns the local data */
	virtual int NumberOfVertices(INode *node) = 0;

	/*! \brief this returns the number of weights of a vertex
	\param node - the node that owns the local data
	\param vindex - the vertex index that you want to get the number of weights from */
	virtual int VertNumberWeights(INode *node, int vindex) = 0;

	/*! \brief this returns a particular weight of a vertex
	\param node - the node that owns the local data
	\param vindex - the vertex index that you want to get the weight
	\param windex - the weight index you want to get */
	virtual float VertGetWeight(INode *node, int vindex, int windex) = 0;

	/*! \brief this returns a particular distance of a vertex
	\param node - the node that owns the local data
	\param vindex - the vertex index that you want to get the weight
	\param windex - the weight index you want to get */
	virtual float VertGetDistance(INode *node, int vindex, int windex) = 0;

	/*! \brief this returns the control point that owns this weight
	\param node - the node that owns the local data
	\param vindex - the vertex index that you want to get the weight
	\param windex - the weight index you want to get */
	virtual int VertGetControlPoint(INode *node, int vindex, int windex) = 0;


	/*! \brief this returns the wrap node that owns this weight
	\param node - the node that owns the local data
	\param vindex - the vertex index that you want to get the weight
	\param windex - the weight index you want to get */
	virtual int VertGetWrapNode(INode *node, int vindex, int windex) = 0;

	//! \brief This forces the modifier to reset itself.  
	/*! This is identical to pressing the reset button
	in the UI.  This will force all weights and param space to be recomputed */
	virtual void Reset()=0;

	//! \brief This takes the weighting generated from a wrap modifier and turns it into a skin modifier.
	/*! This requires that the modifier not be instances and all the wrap objects be driven by skin
	\param node - the node that owns the local data
	\param silent - this supresses any warning/error message boxes */
	virtual void ConvertToSkin(BOOL silent)=0;
};


