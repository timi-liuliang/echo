/**********************************************************************
 *<
	FILE: IGameObject.h

	DESCRIPTION: Object interfaces for IGame

	CREATED BY: Neil Hazzard, Discreet

	HISTORY: created 02/02/02

	IGame Version: 1.122

 *>	Copyright (c) 2002, All Rights Reserved.
 **********************************************************************/
/*!\file IGameObject.h
\brief IGame supported Object Interfaces.
*/
#pragma once

#include "..\maxheap.h"
#include "IGameProperty.h"
#include "IGameStd.h"
#include "..\point2.h"

// forward declarations
class IGameNode;
class IGameModifier;
class IGameSkin;
class IGameMaterial;
class Object;
class Box3;
class Mesh;
class Color;
class ShapeObject;

//!Simple wrapper for 3ds Max objects
/*! This is heart of the interaction with 3ds Max objects.  3ds Max objects are converted to IGame objects to provide
simpler interfaces.  Specific support is given to Meshes, Splines, Helper (including bones) and Cameras.  Any object 
that is not supported by IGame directly is supported as a generic object, so that properties can be extracted easily.  At the present time IGame only 
supports Geom Objects that can be converted to a Tri Object. 
\n
A note about XRef items.  IGame will search out XRef Objects and store them as IGameXrefObject.  External file names and proxies can be accessed
by casting IGameObject* to IGameXrefObject*
\n
Some of the interfaces returned by IGameObject require that the nodes be parsed by IGame first.
\sa IGameNode, IGameMesh, IGameLight, IGameCamera, IGameSupportObject
\sa IExportEntity
*/
class IGameObject : public IExportEntity
{
	Tab <IGameModifier *> gameModTab;

protected:
	INode * gameNode;
	Object * gameObject;
	void SetNode (INode * n);
public:
	//!Default constructor
	IGameObject();
	
	//! Various IGame Object types
	/*! These are the objects supported by IGame
	*/
	enum ObjectTypes{
		IGAME_UNKNOWN,	/*!< An unknown object*/
		IGAME_LIGHT,	/*!< A Light Object */
		IGAME_MESH,		/*!< A Mesh Object */
		IGAME_SPLINE,	/*!< A Spline Object */
		IGAME_CAMERA,	/*!< A Camera Object */
		IGAME_HELPER,	/*!< A Helper Object */
		IGAME_BONE,		/*!< A Bone Object */
		IGAME_IKCHAIN,	/*!< An IK Chain Object*/
		IGAME_XREF      /*!< A XRef Object*/
	};

	//! Various 3ds Max Object types
	/*! These are 3ds Max object - Developers can use this to cast the Object pointer to one of the relevent 3ds Max classes
	*/
	enum MaxType{
		IGAME_MAX_UNKNOWN,	/*!< An unknown object*/
		IGAME_MAX_GEOM,		/*!< A Geom Object*/
		IGAME_MAX_SHAPE,	/*!< A Shape Object*/
		IGAME_MAX_LIGHT,	/*!< A Light Object*/
		IGAME_MAX_CAMERA,	/*!< A Camera Object*/
		IGAME_MAX_BONE,		/*!< A Bone Object*/
		IGAME_MAX_HELPER,	/*!< A Helper Object*/
	};

	//! The bounding box of the object
	/*! 
	\param bb The Box3 to receive the value
	*/
	IGAMEEXPORT void GetBoundingBox(Box3 & bb);

	//!Check if object renderable
	/*!
	\return TRUE if it is renderable
	*/
	IGAMEEXPORT bool IsRenderable();

	//! The IGame object type
	/*! Return the Type of IGameObject it represents
	\return The object type. Returned value corresponds to IGameObject::ObjectTypes Enum
	*/
	virtual ObjectTypes GetIGameType() =0;
	
	//! Access to the actual 3ds Max object
	/*! Access to the actual 3ds Max object, if further direct access needed
	\return A pointer to a max Object class
	*/
	IGAMEEXPORT Object * GetMaxObject();

	//! The 3ds Max object type
	/*! This lets the developer know what type of object - Light, Geom, Helper etc...
	This is different from the IGame types, as these may group more than one type of 3ds Max object together.
	The information can be used to cast Object returned from IGameObject::GetMaxObject to the appropriate class.
	\return The type of max object.  This is defined as the IGameObject::MaxType enum
	*/
	IGAMEEXPORT MaxType GetMaxType() ;
	
	//!The number of modifiers active on the object
	/*! 
	\return The number of modifiers
	*/
	IGAMEEXPORT int GetNumModifiers();

	//!Access to IGame modifier
	/*! Get a pointer to the IGame representation of the modifier
	\param index The index of the modifier to return
	\return A pointer to the IGameModifier
	*/
	IGAMEEXPORT IGameModifier * GetIGameModifier(int index);

	//! The IGameObject constructor
	/*!
	\param *node  The 3ds Max node to initialise
	*/
	IGAMEEXPORT IGameObject(INode  * node);

	//!Check if Object casts shadows
	/*!
	\return TRUE if object casts shadows
	*/
	IGAMEEXPORT bool CastShadows();
	
	//!Check if object skinned
	/*!Is object skinned with either Physique of Skin
	\return TRUE if skinned
	*/
	IGAMEEXPORT bool IsObjectSkinned();

	//!Access to the Skin interface
	/*! 
	\return A pointer to IGameSkin or NULL if not present
	*/
	IGAMEEXPORT IGameSkin * GetIGameSkin();

	//!Check if XRef object
	/*! Indentify XRef objects
	\return TRUE if it is an XRef Object
	*/
	IGAMEEXPORT bool IsObjectXRef();

	//! Access to the ObjectTM
	/*! Get the ObjectTM.  This is the matrix needed to calculate world space
	\return A matrix containing the Object TM.  Used to calculate world space.
	*/
	IGAMEEXPORT GMatrix GetIGameObjectTM();

	//! Extract the 3ds Max data into IGame data
	/*! Some extraction processes are time and memory consuming.  This method allows the developer to specify when 
	they want the data to be converted - this prevents any unwanted data being converted.  This is important for 
	IGameMesh class - if you are just after parameter data, you don't want the whole vertex array being sorted.  Calling
	this tells the object that you want the data be converted.
	\return It will return FALSE when data has not been converted, this object should not be exported.  Usually this is 
	due to a standin or in the case of a GeomObject, it can't be converted to a Tri Object.
	*/
	virtual bool InitializeData(){return false;}

	virtual ~IGameObject() = 0;
};

//!Simple wrapper for light objects
/*! An IGame Wrapper around 3ds Max's lights.  This is a generic interface for all the lights
*/
class IGameLight : public IGameObject  {
public:
	//! Various Light types used by 3ds Max
	enum LightType{
		IGAME_OMNI,		/*!< Omnidirectional Light*/
		IGAME_TSPOT,	/*!< Targeted Spot Light*/
		IGAME_DIR,		/*!< Directional Light*/
		IGAME_FSPOT,	/*!< Free spot Light*/
		IGAME_TDIR,		/*!< Targeted Directional Light*/
		IGAME_UNKNOWN,	/*!< An unknown light type*/
	};

	//! Get the Light Color Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightColor()=0;

	//! Get the Light Multiplier Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightMultiplier()=0;

	//! Get the Light Attenuation End Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAttenEnd()=0;

	//! Get the Light Attenuation Start Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAttenStart()=0;

	//! Get the Light Falloff Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightFallOff()=0;

	//! Get the Light Hot spot Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightHotSpot()=0;

	//! Get the Light Aspect Ratio Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightAspectRatio()=0;
	
	//! Get the Light Decay Start Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetLightDecayStart()=0;
	
	//! Get the Light type as defined in the UI
	/*!
	\return The Light Type. Returned value corresponds to IGameLight::LightType Enum
	*/
	virtual LightType GetLightType()=0;


	//! Get the Light overshoot
	/*! 
	\return TRUE if the Light supports overshoot
	*/
	virtual int GetLightOvershoot()=0;

	//! Get the Decay Type of the Light
	/*! 
	\return  The Decay Type as integer
	\n
	0 - None\n
	1 - Inverse\n
	2 - Inverse Square\n
	*/
	virtual int GetLightDecayType()=0;

	//! Get the shape of the Light
	/*! 
	\return  The shape can be one of the following
	\n
	RECT_LIGHT\n
	CIRCLE_LIGHT\n
	*/
	virtual int GetSpotLightShape()=0;

		//! Get the Target of the Light
	/*! If the light is of type Spot light then this provides access to the target
	\return A pointer to IGameNode for the target.  Return NULL for non target lights
	*/
	virtual IGameNode * GetLightTarget()=0;

	//! Check if Light is On 
	/*! 
	\return True if the Light is on
	*/
	virtual bool IsLightOn() = 0;

	//! Check Exclude List
	/*! Determines whether the exclude list actually maintains a list that is infact included by the light
	\return TRUE if the light maintains an included list
	*/
	virtual bool IsExcludeListReversed()=0;

	//! Get the number of excluded nodes from the Light
	/*! This list contains nodes that should not be included in lighting
	calculations.  It can also contain a list of only those lights that SHOULD be included.  This all depends on the state 
	of IsExcludedListReversed.
	\return The total number of excluded nodes
	*/
	virtual int GetExcludedNodesCount() = 0;

	//!Get the excluded node based on the index pass in
	/*! 
	\param index The index of the node to access
	\return An IGameNode pointer for the excluded node
	*/
	virtual IGameNode * GetExcludedNode(int index) = 0;



};

//!Simple wrapper for camera objects
/*! An IGame Wrapper around 3ds Max's cameras.  This is a generic interface for all the cameras
*/
class IGameCamera : public IGameObject {

public:

	//! Get the Camera Field of View Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraFOV()=0;

	//! Get the Camera Far Clip plane Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraFarClip()=0;

	//! Get the Camera Near Clip plane Data
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraNearClip()=0;

	//! Get the Camera Target Distance
	/*!
	\return A pointer to IGameProperty
	*/
	virtual IGameProperty * GetCameraTargetDist()=0;

	//! Get the Camera Target
	/*! If the camera is target camera then this provides access to the target
	\return a pointer to IGameNode for the target.  Return NULL for non target cameras
	*/
	virtual IGameNode *  GetCameraTarget()=0;


};



//!Simple extension to the 3ds Max Face class
/*!Simple class to store extended data about the face.  The indexing works as a regular
3ds Max Face but mirroring has been taken account of, in the construction.
*/

class FaceEx: public MaxHeapOperators
{
public:

	//! Index into the vertex array
	DWORD vert[3];
	//! Index into the standard mapping channel
	DWORD texCoord[3];
	//! Index into the normal array 
	DWORD norm[3];
	//! Index into the vertex color array
	DWORD color[3];
	//! Index into the vertex illumination array
	DWORD illum[3];
	//! Index into the vertex alpha array
	DWORD alpha[3];
	//! The smoothing group
	DWORD smGrp;
	//! The material ID of the face
	int matID;
	//! Additional flags
	DWORD flags;
	//! Index of corresponding face in the original mesh
	int meshFaceIndex;
	//! Index into edge visibility array.  
	/*! 1 for visible, 0 if the edge is invisible.*/
	DWORD edgeVis[3];  


};

//!Simple wrapper for tri mesh objects
/*! An IGame wrapper around the standard 3ds Max Mesh class.  It provides unified support for Vertex colors and normals
Mirroring is taken into account so the data you retrieve is swapped.\n
Many of the geometry lookups used by IGameObject use the 3ds Max Template Class Tab.  You can use the Tab returned to find out whether the call 
was successful as the Tab count would be greater then zero.\n
In 3ds Max 6.0 and above version of IGame,  Tangent space is calculated.  For this to work, each face <b>must</b> be a member of a smoothing group.
*/
class IGameMesh: public IGameObject {
public:
	
	//! Initialize Binormal and Tangents data <b><i> NEW for 3DXI V 2.2 </i></b>
	/*!  Binormal and Tangents can be initialized and accessed separately from other mesh data
	see IGameObject::InitializeData() for more details 
	To initialize entire mesh data (including Binormals and Tangents) IGameObject::InitializeData() should be called 
	\return true if successful, false otherwise
	*/
	virtual bool InitializeBinormalData()=0;
	
	//! Get number of Vertices
	/*! The total number of vertices found in the mesh
	\return The number of Vertices
	*/
	virtual int GetNumberOfVerts()=0;

	//! Get number of Texture Vertices
	/*! The total number of Texture vertices found in the mesh
	\return The number of TextureVertices
	*/	
	virtual int GetNumberOfTexVerts()=0;

	//!Get the actual Vertex
	/*! Get the vertex at the specified index.  This is in the World Space Coordinate System
	\param index The index of the vertex 
	\param ObjectSpace Defines what space the vertex is in.  This defaults to World space to be compatible 
	with previous versions	
	\return A Point3 representing the position of the vertex
	*/
	virtual Point3 GetVertex(int index, bool ObjectSpace = false)=0;

	//!Get the actual Vertex
	/*! Get the vertex at the specified index.  This is in the World Space Coordinate System
	\param index The index of the vertex 
	\param &vert A Point3 to receive the data
	\param ObjectSpace Defines what space the vertex is in.  This defaults to World space to be compatible 
	with previous versions
	\return TRUE if successful
	*/
	virtual bool GetVertex(int index, Point3 & vert, bool ObjectSpace = false) = 0;

	//!Get the actual Texture Vertex
	/*! Get the Texture vertex at the specified index
	\param index The index of the Texture vertex 
	\return A Point2 representing the Texture vertex
	*/
	virtual Point2 GetTexVertex(int index)=0;

	//!Get the actual Texture Vertex
	/*! Get the Texture vertex at the specified index
	\param index The index of the Texture vertex 
	\param &tex A Point2 to receive the data.
	\return TRUE if successful
	*/
	virtual bool GetTexVertex(int index, Point2 & tex) = 0;

	//! [METHOD IS RETIRED]  Specify whether normals are calculated based on face angles
	/*! To tell IGame to calculate normals based on a weight made from the angle of the edges at the vertex, the developer
	needs to call this <b>before</b> IGameObject::InitializeData() is called.  The default is not to use weighted normals
	which is also the default in 3ds Max 4.0
	*/
	virtual void SetUseWeightedNormals() = 0;
	
	//! Get number of normals
	/*! The total number of normals found in the mesh
	\return The number of normals
	*/
	virtual int GetNumberOfNormals()=0;

	//!Get the Vertex Index associated with the Normal <b><i> NEW for 3DXI V 2.2 </i></b>
	/*! Get the Vertex Index associated with the Normal index.
	\param index The index of the normal in the flat list 
	\return Vertex Index associated with the Normal inex
	*/
	virtual int GetNormalVertexIndex (int index)=0;
	
	//!Get the actual normal
	/*! Get the normal at the specified index.  
	\param index The index of the normal in the flat list 
	\param ObjectSpace defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	\return A Point3 representing the normal
	*/
	virtual Point3 GetNormal(int index, bool ObjectSpace = false)=0;

	//!Get the actual normal
	/*! Get the normal at the specified index in the flat list
	\param index The index of the normal
	\param &norm A Point3 to receive the data.
	\param ObjectSpace Defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	\return TRUE if successful
	*/
	virtual bool GetNormal(int index, Point3 & norm, bool ObjectSpace = false) = 0;

	//!Get the normal based on face and corner
	/*!
	\param faceIndex The index into the face array
	\param corner The vertex whose normal is requested
	\param &norm A point3 to receive the data
	\param ObjectSpace Defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	\return TRUE if successful
	*/
	virtual bool GetNormal(int faceIndex, int corner, Point3 & norm, bool ObjectSpace = false) = 0;

	//!Get the normal based on face and corner
	/*!
	\param face  The pointer of the face whose normals are to be accessed
	\param corner The vertex whose normal is requested
	\param ObjectSpace Defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	\return A Point3 containing the normal
	*/
	virtual Point3 GetNormal ( FaceEx * face, int corner, bool ObjectSpace = false) = 0;

	//!Get the normal based on face and corner
	/*!
	\param face The pointer of the face whose normals are to be accessed
	\param corner The vertex whose normal is requested
	\param &norm A point3 to receive the data
	\param ObjectSpace Defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	\return TRUE if successful
	*/
	virtual bool GetNormal(FaceEx * face, int corner, Point3 &norm, bool ObjectSpace = false) =0;
	
	//!Get the normal based on face and corner
	/*!
	\param faceIndex The index into the face array
	\param corner The vertex whose normal is requested
	\return A Point3 containing the normal
	\param ObjectSpace Defines what space the normal is in.  This defaults to World space to be compatible 
	with previous versions
	*/
	virtual Point3 GetNormal(int faceIndex, int corner, bool ObjectSpace = false) = 0;
	
	//! Get number of Illuminated Vertices
	/*! The total number of Illuminated Vertices found in the mesh
	\return The number of Illuminated Vertices
	*/
	virtual int GetNumberOfIllumVerts()=0;

	//! Get number of Alpha Vertices
	/*! The total number of Alpha Vertices found in the mesh
	\return The number of Alpha Vertices
	*/
	virtual int GetNumberOfAlphaVerts()=0;

    //! Get number of  Vertex Colors
	/*! The total number of Vertex Colors found in the mesh
	\return The number of Vertex Colors
	*/	
	virtual int GetNumberOfColorVerts()=0;

	//!Get the actual Color Vertex
	/*! Get the color vertex at the specified index
	\param index The index of the color vertex 
	\return A Point3 representing the color of the vertex.  Return Point3(-1,-1,-1) if the index is invalid
	*/
	virtual Point3 GetColorVertex(int index)=0;

	//!Get the actual Color Vertex
	/*! Get the color vertex at the specified index
	\param index The index of the color vertex 
	\param &col A Point3 to receive the color data
	\return TRUE if successful
	*/
	virtual bool GetColorVertex(int index, Point3 & col) = 0;

	//!Get the actual Alpha Vertex
	/*! Get the Alpha vertex at the specified index
	\param index The index of the Alpha vertex 
	\return A float representing the Alpha value of the vertex.  Return -1 if the index is invalid 
	*/
	virtual float  GetAlphaVertex(int index)=0;


	//!Get the actual Alpha Vertex
	/*! Get the Alpha vertex at the specified index
	\param index The index of the Alpha vertex 
	\param &alpha A float to receive the value
	\return TRUE if successful
	*/
	virtual bool GetAlphaVertex(int index, float & alpha) = 0;

	//!Get the actual Illuminated Vertex
	/*! Get the Illuminated vertex at the specified index
	\param index The index of the Illuminated vertex 
	\return A float representing the Illuminated value of the vertex.  Return -1 if the index is invalid
	*/
	virtual float  GetIllumVertex(int index)=0;
	
	//!Get the actual Illuminated Vertex
	/*! Get the Illuminated vertex at the specified index
	\param index The index of the Illuminated vertex 
	\param &illum A float to receive the data
	\return TRUE if successful
	*/
	virtual bool GetIllumVertex(int index, float &illum) = 0;

	//! Get the number of faces in the mesh
	/*! The total number of faces contained in the mesh
	\return The number of faces
	*/
	virtual int GetNumberOfFaces()=0;
	
	//! Get the actual face 
	/*! The face represented by the index.  The data in FaceEx can be used to lookup into the various arrays
	\param index The index of the face to return
	\return A pointer to FaceEx, or NULL if an invalid index is passed in
	*/
	virtual FaceEx * GetFace(int index)=0;
	
	//!The number of verts in a mapping channel
	/*! Get the number of the vertices for a particular mapping channel
	\param ch The mapping channel to use
	\return The number of verts
	*/
	virtual int GetNumberOfMapVerts(int ch) = 0;

	//! Get the mapping vertex
	/*! Get the actual mapping vertex for the channel
	\param ch The channel to query
	\param index The vertex index
	\return The actual mapping data.  This will be 0 if the Mapping channel is not found
	*/
	virtual Point3 GetMapVertex(int ch, int index) = 0;

	//! Get the mapping vertex
	/*! Get the actual mapping vertex for the channel
	\param ch The channel to query
	\param index The vertex index
	\param &mVert A Point3 to receive the data
	\return TRUE if successful
	*/
	virtual bool GetMapVertex(int ch, int index, Point3 & mVert) = 0;

	//!The number of Binormals <b><i>This function is only available in 3ds Max 6.0 and above</i></b> 
	/*! Get the number of the binormals  
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return The number of binormals for specified map channel
	*/
	virtual int GetNumberOfBinormals(int mapChannel = 1) = 0;

	//!Get the actual Binormal <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*! Get the Binormal at the specified index
	\param index The index of the Binormal
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return A Point3 representing the Binormal for specified map channel
	*/
	virtual Point3 GetBinormal(int index, int mapChannel = 1)=0;

	//!Get the actual Binormal <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*! Get the Binormal at the specified index
	\param index The index of the Binormal
	\param biNormal A point3 to receive the Binormal for specified map channel
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return TRUE if successful
	*/
	virtual bool GetBinormal(int index, Point3 & biNormal, int mapChannel = 1) = 0;

	//!The number of Tangents <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*! Get the number of the Tangents 
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return The number of Tangents for specified map channel
	*/
	virtual int GetNumberOfTangents(int mapChannel = 1) = 0;

	//!Get the actual Tangent <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*! Get the Tangent at the specified index
	\param index The index of the Tangent
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return A Point3 representing the Tangent for specified map channel
	*/
	virtual Point3 GetTangent(int index, int mapChannel = 1) = 0;

	//!Get the actual Tangent <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*! Get the Tangent at the specified index
	\param index The index of the Tangent
	\param Tangent A point3 to receive the Tangent for specified map channel
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return TRUE if successful
	*/
	virtual bool GetTangent(int index, Point3 & Tangent, int mapChannel = 1) = 0;

	//! Get the active mapping channels
	/*! Extracts the active mapping channels in use by the object.  Starting with 3DXI V2.0 this returns all active channels
	including the standard ones such as Texture Coordinates, Vertex Colors, Illum, and Alpha.
	\return A tab containing the active Mapping channels.  
	*/
	virtual Tab<int> GetActiveMapChannelNum() = 0;
	
	//! Get the face index into the mapping channel array
	/*! Get the actual index into the mapping channel for the supplied face.  
	\param ch The mapping channel to use
	\param faceNum The face to use
	\param index An array of three indices to receive the indexing into the vertices
	\return TRUE if the channel was accessed correctly.  False will mean that the channel was not present.
	*/
	virtual bool GetMapFaceIndex(int ch, int faceNum, DWORD *index) = 0;
	
	//!Get all the smoothing groups found on a mesh
	/*!
	\return A tab containing the smoothing groups.  If the count is zero it means that no smoothing groups were found
	*/
	virtual Tab<DWORD> GetActiveSmgrps() = 0;

	//!Get all the material IDs found on a mesh
	/*!
	\return A tab containing the Material IDs
	*/
	virtual Tab<int> GetActiveMatIDs() = 0;

	//!Get the face for a particular smoothing group
	/*! Get all the faces belonging to a particular smoothing group
	\param smgrp The smoothing group to use
	\return A tab containing all the faces
	*/
	virtual Tab<FaceEx *> GetFacesFromSmgrp(DWORD smgrp) = 0;

	//!Get the face index for a particular smoothing group
	/*! Get all the faces belonging to a particular smoothing group as a set of indexes into the main face list
	\param smgrp The smoothing group to use
	\return A tab containing all the indexes
	*/
	virtual	Tab<int> GetFaceIndexFromSmgrp(DWORD smgrp)=0;

	//! Get the faces for a particular Material ID
	/*! Get all the faces belonging to a particular material ID
	\param matID The material ID to use
	\return A tab containing all the faces
	*/	
	virtual Tab<FaceEx *> GetFacesFromMatID(int matID) = 0;

	//!The actual material used by the Face
	/*!This will provide access to the material used by the Face whose index is passed in.  This means the mesh can be broken down
	into smaller meshes if the material is a subObject material.  This can be used in conjunction with IGameMesh::GetFacesFromMatID to rebuild a face 
	with the material assigned via a material ID.
	\param FaceNum The index of the face whose material is needed.
	\return A pointer to a material.  The is the actual material, so in the case of the SubObject material the material whose mat ID
	matches.
	*/
	virtual IGameMaterial * GetMaterialFromFace(int FaceNum) = 0;

	//!The actual material used by the Face
	/*!This will provide access to the material used by the Face.  This means the mesh can be broken down
	into smaller meshes if the material is a subObject material.  This can be used in conjunction with IGameMesh::GetFacesFromMatID to rebuild a face 
	with the material assigned via a material ID.
	\param face A pointer to the face whose material is needed.
	\return A pointer to a material.  The is the actual material, so in the case of the SubObject material the material whose mat ID
	matches.
	*/
	virtual IGameMaterial * GetMaterialFromFace(FaceEx * face) = 0;


	//! The actual 3ds Max Mesh representation
	/*! The Mesh pointer used by 3ds Max.  This allows the developer further access if required to the mesh or data structures
	\return A Mesh pointer
	*/
	virtual Mesh * GetMaxMesh()=0;


	//! \name As alternative to FaceEx structure, separate channels can be accessed directly
	//@{
	//!Get Vertex Index for specified face and corner
	/*! 
	\param faceIndex The face ID to use
    \param corner The face corner to use
	\return Vertex Index
	*/
	virtual int GetFaceVertex (int faceIndex, int corner)=0;
	
	//!Get Map Vertex Index for specified face and corner
	/*! Texture Vertex by default
	Should be used to get vertex index for Vertex Color, Alpha, Illum, etc
	\param faceIndex The face ID to use
	\param corner The face corner to use
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return Map Vertex Index
	*/
	virtual int GetFaceTextureVertex (int faceIndex, int corner, int mapChannel = 1)=0;

	//! Get Normal Vertex Index for specified face and corner
	/*!
	\param faceIndex The face ID to use
	\param corner The face corner to use
	\return Normal Vertex Index
	*/	
	virtual int GetFaceVertexNormal (int faceIndex, int corner)=0;
	
	//! Get Tangent and Binormal Vertex Index for specified face and corner
	/*! 
	\param faceIndex The face ID to use
	\param corner The face corner to use
	\param mapChannel  The valid MESH MAP channel, range (-2...99), default - Texture Coordinates
	\return Tangent and Binormal Vertex Index if successful, -1 otherwise 
	*/	
	virtual int GetFaceVertexTangentBinormal (int faceIndex, int corner, int mapChannel = 1)=0;
	
	//! Get Smoothing Group for specified face
	/*!
	\param faceIndex The face ID to use
	\return Smoothing Group value
	*/	
	virtual DWORD GetFaceSmoothingGroup (int faceIndex)=0;
	
	//! Get Material ID for specified face. 
	/*!
	\param faceIndex The face ID to use
	\return Material ID value
	*/	
	virtual int GetFaceMaterialID (int faceIndex)=0;
    
	//! Get Edge Visibility value for specified face and edge. 
	/*!
	\param faceIndex The face ID to use
	\param edge The edge to use
	\return 1 if edge visible, 0 otherwise
	*/	
	virtual int GetFaceEdgeVisibility (int faceIndex, int edge)=0;
	//@}

	//! Access the color data for the face specified.
	/*! The surface color can be obtained from the RenderedSurface interface.  Before this function can be used the Surface
	data needs to be initialised before hand.  Please see the IGameRenderedSurface for more information
	\param FaceIndex The face index whose color is being evaluated
	\param *result A pointer to a Color Array that receives the 3 vertices for the face.  This should initialised as Color res[3]
	\return TRUE is successful.  Possible errors include the object not being renderable or is hidden.
	\sa IGameRenderedSurface
	*/
	virtual bool EvaluateSurface(int FaceIndex, Color * result) = 0;


	//! [METHOD IS RETIRED] Tell IGame to create a flattened Normals Array <b><i>This function is only available in 3ds Max 6.0 and above</i></b>
	/*!IGame can create a flattened list of normals for export much like Vertices and Texture Coordinates.  However this
	operation can be expensive especially on large objects with mixed smoothing groups.  This method determines whether this is
	available for use.  <b>If it is not set, then vertex normals can only be looked up on a per face basis </b>.<br><br>
	Due to some internal changes, this method is not really needed.  However some builds of IGame did require it so it has
	been kept in.  
	*/
	virtual void SetCreateOptimizedNormalList() = 0;

	//! N-Polygon support
	//@{
	//! Get the number of polygons in the EPoly
	/*! The total number of polygons contained in the EPoly
	\return The number of polygons, zero if mesh can not be converted to EPoly
	*/
	virtual int GetNumberOfPolygons()=0;

	//! Get the number of Normals in the EPoly  <b><i> NEW for 3DXI V 2.2 </i></b>
	/*! The total number of Normalscontained in the EPoly
	\return The number of Normals, zero if mesh can not be converted to EPoly
	*/
	virtual int GetNumberOfPolygonNormals()=0;

	//! Get Polygon polygon corners
	/*! The degree (corners) for  given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\return The degree (corners) for polygon, zero if mesh can not be converted to EPoly
	*/
	virtual int GetNumberOfPolygonCorners(int PolyIndex)=0;

	//! Get Polygon vertices
	/*! The vertices ID's for given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\return The vertices ID's as an array, empty if mesh can not be converted to EPoly
	*/
	virtual Tab<INT> GetPolygonVertices(int PolyIndex)=0;


	//! Get Polygon normals
	/*! The normals ID's for given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\return The normals ID's as an array, empty if mesh can not be converted to EPoly
	*/
	virtual Tab<INT> GetPolygonNormals(int PolyIndex)=0;

	//! Get Polygon normals array  <b><i> NEW for 3DXI V 2.2 </i></b>
	/*! The normals array 
	\param PolyIndex The polygon index is being evaluated.  Default is -1, returning all normals for EPoly
	 If parameter defined (other than -1) returns normals for provided polygon only.
	\return The normals as an array, empty if mesh can not be converted to EPoly
	*/
	virtual Tab<Point3> GetPolygonNormalsArray(int PolyIndex = -1)=0;

	//! Get Polygon map vertices
	/*! The map vetrices ID's for given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\param MapChannel The map channel is being evaluated
	\return The map vertices ID's as an array, empty if mesh can not be converted to EPoly
	*/
	virtual Tab<INT> GetPolygonMapVertices(int PolyIndex, int MapChannel)=0;

	//! Get Polygon Edges Visibility
	/*! The Edges Visibility values for given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\return The Edges Visibility array (1 if edge visible, 0 otherwise), empty if mesh can not be converted to EPoly
	*/
	virtual Tab<INT> GetPolygonEgdesVisibility(int PolyIndex)=0;

	//! Get Smoothing Group for specified Polygon
	/*!
	\param PolyIndex The polygon index is being evaluated
	\return Smoothing Group value
	*/	
	virtual DWORD GetPolygonSmoothingGroup (int PolyIndex)=0;

	//! Get Material ID for specified Polygon. 
	/*!
	\param PolyIndex The polygon index is being evaluated
	\return Material ID value
	*/	
	virtual int GetPolygonMaterialID (int PolyIndex)=0;
	
	//! Get Polygon Indices based on Material ID
	/*! The Polygon ID's for given Material ID in the EPoly
	\param MatID The face index is being evaluated
	\return The vertices ID's as an array, empty if mesh can not be converted to EPoly
	*/
	virtual Tab<INT> GetPolygonIndexFromMatID(int MatID)=0;

	//! Get Polygon Material
	/*! The Material for given polygon in the EPoly
	\param PolyIndex The polygon index is being evaluated
	\return The Polygon Material, NULL if not exists
	*/
	virtual IGameMaterial* GetMaterialFromPolygon(int PolyIndex)= 0;
	//@}

};

//!Simple wrapper for spline knots
/*! An IGame wrapper for Knot information
*/
class IGameKnot: public MaxHeapOperators
{
public:
	//! Knot types
	enum KnotType{
		KNOT_AUTO,				/*!< Auto generate Knot*/
		KNOT_CORNER,			/*!< A corner knot*/
		KNOT_BEZIER,			/*!< A bezier knot*/
		KNOT_BEZIER_CORNER,		/*!< A bezier corner knot*/
	};

	//! Knot data
	enum KnotData{
		KNOT_INVEC,				/*!< The in vector*/
		KNOT_OUTVEC,			/*!< The out vector*/
		KNOT_POINT,				/*!< The actual knot position*/
	};
	//! \brief Destructor 
	virtual ~IGameKnot() {;}

	//! Get the in vector
	/*!
	\return The in vector
	*/
	virtual Point3 GetInVec()=0;
	
	//! Get the out vector
	/*!
	\return The out vector
	*/
	virtual Point3 GetOutVec()=0;

	//! Get actual knot position
	/*!
	\return The knot position
	*/
	virtual Point3 GetKnotPoint()=0;

	//! Get the type of knot
	/*!
	\return The knot as a IGameKnot::KnotType
	*/
	virtual KnotType GetKnotType()=0;
	
	//! Get the knot controller
	/*! Get the actual IGameController for the knot - this provides access to any animated data
	\param kd The knot to access
	\return The controller for the specified knot
	*/
	virtual IGameControl * GetKnotControl(KnotData kd)=0;
};

//! A wrapper class for splines
/*! This  provides information about the actual splines making up the spline object in 3ds Max.  Access to the knot data is 
provided with this class
*/
class IGameSpline3D: public MaxHeapOperators
{
public:
	//! \brief Destructor 
	virtual ~IGameSpline3D() {;}

	//! Access the individual knot
	/*!
	\return The knot for the index passed in
	*/
	virtual IGameKnot * GetIGameKnot(int index) = 0;
	
	//! The knot count
	/*!
	\return The total number of knots in the spline
	*/
	virtual int GetIGameKnotCount()=0;

};

//!Simple wrapper for Splines
/*! An IGame wrapper around the standard 3ds Max spline object.  IGameSpline acts as a container for all the individual splines
that make up the object
*/
class IGameSpline : public IGameObject
{
public:
	//! Get the number of splines
	/*! The number of splines that make up this object
	\return The total number of splines
	*/
	virtual int GetNumberOfSplines()=0;

	//! Get an individual Spline
	/*! Get a spline based on the index.
	\param index The index of the spline to access
	\return A pointer to the spline.
	*/
	virtual IGameSpline3D * GetIGameSpline3D(int index) =0;

	//! Get the Max object
	/*!
	\return The ShapeObject used by 3ds Max
	*/
	virtual ShapeObject * GetMaxShape()=0;

};


//!Simple wrapper for IKChains
/*! An IGame wrapper around the IKChain object. 
\n
This object be used as a basis for character export.  All nodes used in the chain are maintained by the IGameIKChain
interface.  This allows animation to be exported based on whether it is in IK or FK mode.  If the IK is enabled then
the IGameControl retrieved from the IGameNode will be that of the End Effector for the chain.  If it is in FK mode then
the IGameControl from the nodes in the chain would be used for the FK calculation.
*/

class IGameIKChain : public IGameObject
{
public:

	//! Get the number of nodes that make up the chain
	/*!
	\return The number of nodes in the chain
	*/
	virtual int GetNumberofBonesinChain() =0;

	//! Access to the n'th node in the chain
	/*!
	\param index The index of the node to access
	\return An IGameNode representation of the node
	*/
	virtual IGameNode * GetIGameNodeInChain(int index) =0;

	//! Get the swivel data
	/*! The swivel data used in the IK calculation
	\return The IGameProperty for the swivel data
	*/
	virtual IGameProperty * GetSwivelData() = 0;

	//! Get the controller for the IK enable
	/*! Access to the Enabled controller - this defines whether IK or FK are used.  When IK the end effector
	is used to control the transforms, in FK the individual nodes can be positioned independent of the effector
	\return An IGameControl pointer for the Enabled controller.  This controller does not have direct access, so
	should be sampled using IGameControlType::IGAME_FLOAT
	*/
	virtual IGameControl * GetIKEnabledController() = 0;


};


//! Base class for "support" objects, such as bone, helpers dummies etc..
/*!These types of objects are really supported for their parameter access.  However a pointer
to the Mesh representation is provided if for example bone geometry is needed.  This class can be used
to check for BONES, DUMMYS etc..
*/
class IGameSupportObject : public IGameObject
{
public:
	//! Access to the mesh
	/*! If required, access to the mesh is provided
	\return A pointer to an IGameMesh object
	*/
	virtual IGameMesh * GetMeshObject() = 0;

	//!Check for 3ds Max 4.0 version of bone
	/*!
	\return TRUE if it is an 3ds Max 4.0 bone.
	*/
	virtual bool PreR4Bone() = 0;

	
};

//! An interface for an XRef Object 
class IGameXRefObject : public IGameObject
{
public:
	//!Retrieve XRef File Name
	/*!
	\return The name of XRef File
	*/
	virtual const MCHAR * GetOriginalFileName() = 0;

	
};

//! A Generic Object for IGame
/*! This object represents any object that is unknown to IGame - this could be a new pipeline object for example.
It return IGAME_UNKOWN for its IGameObject::ObjectTypes
*/
class IGameGenObject : public IGameObject
{
public:
	
	//!Retrieve the Property Container
	/*!
	\return The PropertyContainer for this entity.
	*/
	virtual IPropertyContainer * GetIPropertyContainer()=0;
};


