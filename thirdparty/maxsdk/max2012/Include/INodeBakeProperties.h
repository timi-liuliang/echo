 /**********************************************************************
 
	FILE:           INodeBakeProperties.h

	DESCRIPTION:    Public interface for setting and getting a node's
                    texture baking properties.

	CREATED BY:		Kells Elmquist

	HISTORY:		created 15 december 2001

 *>	Copyright (c) 2001, All Rights Reserved.
 **********************************************************************/


#pragma once

#include "ifnpub.h"
#include "ipoint2.h"
#include "box2.h"
// forward declarations
class MaxBakeElement;

// The interface ID for class INodeBakeProperties
#define NODE_BAKE_PROPERTIES_INTERFACE Interface_ID(0x3e5d0f38, 0x37e500ac)

// by default use this channel for all baking ops
#define DEFAULT_BAKE_CHANNEL	3
#define DEFAULT_N_DILATIONS		2

//==============================================================================
// class INodeBakeProperties
/*! \par Description:
	This class defines an interface for accessing a node's texture
	baking properties.\n\n
	An instance of this interface can be retrieved using the following line of
	code (assuming 'node' is of type INode*):
	\code
		static_cast<INodeBakeProperties*>(pNode->GetInterface(NODE_BAKE_PROPERTIES_INTERFACE))
	\endcode
	Description of the node's texture baking properties:


	\par General Properties:\n
	Enable:\n
	\t	Texture baking is enabled for this object\n
	Baking Channel:\n
	\t	Flattening & baking use this uv mapping channel for this object\n
	List of Baking Render Elements:\n
	\t	each object has a list of render elements for output
*/
//==============================================================================
class INodeBakeProperties : public FPMixinInterface {

public:

	// General properties
	virtual BOOL GetBakeEnabled() const = 0;
	virtual void SetBakeEnabled( BOOL isExcluded ) = 0;

	// mapping channel to use for baking Object level
	virtual int  GetBakeMapChannel() const = 0;
	virtual void SetBakeMapChannel( int mapChannel ) = 0;

	// number of dilations after rendering, affects seaming
	virtual int  GetNDilations() const = 0;
	virtual void SetNDilations( int nDilations ) = 0;

	virtual int  GetBakeFlags() const = 0;
	virtual void SetBakeFlags( int flags ) = 0;

	virtual float GetSurfaceArea() const = 0;
	virtual void  SetSurfaceArea( float area ) = 0;

	virtual float GetAreaScale() const = 0;
	virtual void  SetAreaScale( float scale ) = 0;

	// bake render elements
	virtual int GetNBakeElements() const = 0;
	virtual MaxBakeElement* GetBakeElement( int nElement ) = 0;

	//! \brief returns a Tab object holding the list of Texture Bake elements assigned to the node. 
	/*! Developers should not try to set BakeElements by altering the contents of this Tab;
		use the methods AddBakeElement() and RemoveBakeElement() instead 
		\returns a Tab object holding the list of Texture Bake elements assigned to the node. */
	virtual Tab<MaxBakeElement*> GetBakeElementArray() = 0;

	virtual BOOL AddBakeElement( MaxBakeElement* pEle ) = 0;
	virtual BOOL RemoveBakeElement( MaxBakeElement* pEle ) = 0;
	virtual BOOL RemoveBakeElementByName( char * name ) = 0;
	virtual BOOL RemoveBakeElementByIndex( int index ) = 0;
	virtual void RemoveAllBakeElements() = 0;

	// reset params to default, toss render elements
	virtual void ResetBakeProps() = 0;

	// enabled & has some elements & sz not 0
	virtual BOOL GetEffectiveEnable() = 0;

	// largest size of enabled baking elements
	virtual IPoint2 GetRenderSize() = 0;

	virtual FBox2 GetActiveRegion() =0;
	virtual void SetActiveRegion(FBox2 region) = 0;
};


//==============================================================================
// class INodeBakeProjProperties
//
// Properties related to projection mapping with texture baking
//
//==============================================================================

#define NODE_BAKE_PROJ_PROPERTIES_INTERFACE Interface_ID(0x59d919a5, 0x6fb90a85)

class INodeBakeProjProperties : public FPMixinInterface {
public:
	//Enabled
	virtual BOOL	GetEnabled() = 0;
	virtual void	SetEnabled( BOOL b ) = 0;

	//ProjMod - Projection Modifier
	virtual ReferenceTarget* GetProjMod() = 0;
	virtual void	SetProjMod( ReferenceTarget* refTarg ) = 0;

	//ProjModTarg - Projection Modifier Target
	virtual MCHAR*	GetProjModTarg() = 0;
	virtual void	SetProjModTarg( MCHAR* s ) = 0;

	//CropAlpha
	virtual BOOL	GetCropAlpha() = 0;
	virtual void	SetCropAlpha( BOOL b ) = 0;

	//ProjSpace: XYZ object space, or UVW texture space
	enum { enumIdProjSpaceXYZ=0, enumIdProjSpaceUVW };
	virtual int		GetProjSpace() = 0;
	virtual void	SetProjSpace( int enum_val ) = 0;

	//NormalSpace: World Space, Screen Space, Local Space, or Tangent Space
	enum { enumIdNormalSpaceWorld=0, enumIdNormalSpaceScreen, enumIdNormalSpaceLocal, enumIdNormalSpaceTangent };
	virtual int		GetNormalSpace() = 0;
	virtual void	SetNormalSpace( int enum_val ) = 0;

	//TangentYDir: Y-Up or Y-Down
	enum { enumIdTangentDirYUp=0, enumIdTangentDirYDown};
	virtual int		GetTangentYDir() = 0;
	virtual void	SetTangentYDir( int enum_val ) = 0;

	//TangentXDir: X-Right or X_Left
	enum {enumIdTangentDirXRight=0, enumIdTangentDirXLeft };
	virtual int		GetTangentXDir() = 0;
	virtual void	SetTangentXDir( int enum_val ) = 0;

	//UseCage
	virtual BOOL	GetUseCage() = 0;
	virtual void	SetUseCage( BOOL b ) = 0;

	//RayOffset
	virtual float	GetRayOffset() = 0;
	virtual void	SetRayOffset( float f ) = 0;

	//HitResolveMode
	enum { enumIdHitResolveClosest=0, enumIdHitResolveFurthest };	
	virtual int		GetHitResolveMode() = 0;
	virtual void	SetHitResolveMode( int enum_val ) = 0;

	//HitMatchMtlID
	virtual BOOL	GetHitMatchMtlID() = 0;
	virtual void	SetHitMatchMtlID( BOOL b ) = 0;

	//HitWorkingModel
	virtual BOOL	GetHitWorkingModel() = 0;
	virtual void	SetHitWorkingModel( BOOL b ) = 0;

	//WarnRayMiss
	virtual BOOL	GetWarnRayMiss() = 0;
	virtual void	SetWarnRayMiss( BOOL b ) = 0;

	//RayMissColor
	virtual Color*	GetRayMissColor() = 0;
	virtual void	SetRayMissColor( Color* c ) = 0;

	//HeightMapMin - rays of less than this length are output as black pixels by the Height Map
	virtual float	GetHeightMapMin() = 0;
	virtual void	SetHeightMapMin( float f ) = 0;

	//HeightMapMax - rays of more than this length are output as white pixels by the Height Map
	virtual float	GetHeightMapMax() = 0;
	virtual void	SetHeightMapMax( float f ) = 0;

	//HeightBufMin - the minimum height value encountered during the last rendering
	virtual float	GetHeightBufMin() = 0;
	virtual void	SetHeightBufMin( float f ) = 0;

	//HeightBufMax - the maximum height value encountered during the last rendering
	virtual float	GetHeightBufMax() = 0;
	virtual void	SetHeightBufMax( float f ) = 0;

	//ProportionalOutput
	virtual BOOL	GetProportionalOutput() = 0;
	virtual void	SetProportionalOutput( BOOL b ) = 0;

	// mapping channel to use for baking SubObject level
	virtual int  	GetBakeMapChannel_SO() const = 0;
	virtual void 	SetBakeMapChannel_SO( int mapChannel ) = 0;

	//BakeObjectLevel
	virtual BOOL	GetBakeObjLevel() = 0;
	virtual void	SetBakeObjLevel( BOOL b ) = 0;

	//BakeSubObjLevels
	virtual BOOL	GetBakeSubObjLevels() = 0;
	virtual void	SetBakeSubObjLevels( BOOL b ) = 0;

	//useObjectBakeForMtl
	virtual BOOL	GetObjBakeToMtl() = 0;
	virtual void	SetObjBakeToMtl( BOOL b ) = 0;

};

