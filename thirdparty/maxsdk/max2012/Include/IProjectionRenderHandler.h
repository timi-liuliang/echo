/**********************************************************************
 *<
	FILE:			ProjectionRenderHandler.h

	DESCRIPTION:	Render time functionality for projection mapping

	CREATED BY:		Michaelson Britt

	HISTORY:		05-01-2004

 *>	Copyright (c) 2004, All Rights Reserved.
 **********************************************************************/


#pragma once

#include "maxheap.h"
#include "istdplug.h"
#include "iparamb2.h"
#include "iparamm2.h"

//-----------------------------------------------------------------------------
// Types

class IMeshWrapper;
class IProjectionRenderMgr;


struct GenTri: public MaxHeapOperators {
	int v0, v1, v2;
};

struct GenTriPoints: public MaxHeapOperators {
	Point3 v0, v1, v2;
};

struct TangentBasis: public MaxHeapOperators {
	Point3 uBasis, vBasis;
};


//===========================================================================
//
// Class IProjectionRenderHandler
//
//===========================================================================

class IProjectionRenderHandler: public MaxHeapOperators {
	public:
		struct ProjectionResult: public MaxHeapOperators {
			RenderInstance*	inst;
			IMeshWrapper*	mesh; //CAMERA SPACE mesh wrapper for the inst
			float			dist;
			int				faceIndex;
			Point3			faceBary;
			BOOL			isBackfacing;
			int				err; //used when Project() returns FALSE
		};

		enum { ERR_NONE=0,
			ERR_RAYMISS,	//ray did not hit a reference model
			ERR_NONSELFACE,	//the working model face is not in the active selection, skipped
		};


		virtual ~IProjectionRenderHandler() {}
		virtual void		DeleteThis() = 0;

		virtual int			RenderBegin(TimeValue t, ULONG flags=0) = 0;
		virtual int			RenderEnd(TimeValue t) = 0;

		// NOTE: Init() may require several seconds to calculate sorting structures for the reference models
		virtual BOOL		Init( RenderInstance* workingModelInst, RenderGlobalContext* rgc ) = 0;
		virtual BOOL		Valid() = 0; //true if initialized, false otherwise
		// Reference & working model info
		virtual void				GetReferenceModels( Tab<RenderInstance*>& refModels ) = 0;
		virtual IMeshWrapper*		GetWorkingModelMesh() = 0;
		virtual RenderInstance*		GetWorkingModelInst() = 0;
		virtual void				GetWorkingModelMask( BitArray& mask ) = 0;

		// Do the projection from working model to reference model
		virtual BOOL		Project( int faceIndex, Point3& faceBary, Point3& faceNorm, ProjectionResult& result ) = 0;
};


// Manager class to create render handler instances

#define IPROJECTIONRENDERMGR_INTERFACE_ID Interface_ID(0x45390e61, 0x42de3b37)
inline IProjectionRenderMgr* GetIProjectionRenderMgr()
{return (IProjectionRenderMgr*)GetCOREInterface(IPROJECTIONRENDERMGR_INTERFACE_ID);}

class IProjectionRenderMgr : public FPStaticInterface {
	public:
		virtual IProjectionRenderHandler* CreateProjectionRenderHandler() = 0;
};


//===========================================================================
//
// Class IMeshWrapper
// An object defined as a set of triangles
//
//===========================================================================


//FIXME: hopefully this can become a subclass of ObjectWrapper
class IMeshWrapper: public MaxHeapOperators {
	public:
		virtual ~IMeshWrapper() {;}
		virtual void		DeleteThis() = 0;

		virtual int			NumTriangles() = 0;
		// For a given triangle, find the face number, and the index of the triangle within the face
		virtual void		Translate( int rawTriIndex, int& faceIndex, int& faceTriIndex ) = 0;

		// Transform affects the geometry & geom normals, but not the mapping data
		virtual Matrix3		GetTM() = 0;
		virtual void		SetTM( Matrix3 tm ) = 0;

		// Geom triangles & verts
		virtual GenTri		GetTri( int triIndex ) = 0;
		virtual void		GetTriPoints( int triIndex, GenTriPoints& triPoints ) = 0;
		virtual Point3		GetVert( int vertIndex ) = 0;
		virtual int			NumVerts() = 0;

		// Map triangles & verts
		virtual BOOL		GetChannelSupport( int mapChannel ) = 0;
		virtual GenTri		GetMapTri( int triIndex, int mapChannel ) = 0;
		virtual void		GetMapTriPoints( int triIndex, int mapChannel, GenTriPoints& triPoints ) = 0;
		virtual Point3		GetMapVert( int vertIndex, int mapChannel ) = 0;
		virtual int			NumMapVerts( int mapChannel ) = 0;
	
		// Normals
		virtual Point3		GetNormal( int triIndex ) = 0;
		virtual TangentBasis GetTangentBasis( int triIndex, int mapChannel ) = 0;
		virtual Point3		GetVertexNormal( int vertIndex, DWORD smGroup ) = 0;
		virtual TangentBasis GetVertexTangentBasis( int vertIndex, DWORD smGroup, int mapChannel ) = 0;
		virtual Point3		GetMapNormal( int triIndex, int mapChannel ) = 0; //UVW space

		// Face properties
		virtual MtlID		GetMtlID( int triIndex ) = 0;
		virtual DWORD		GetSmoothingGroup( int triIndex ) = 0;

		// Helpers
		virtual void		GetPoint( int triIndex, const Point3& triBary, Point3& point ) = 0;
		virtual void		GetMapPoint( int triIndex, const Point3& triBary, int mapChannel, Point3& uvw ) = 0;
		virtual void		GetNormal( int triIndex, const Point3& triBary, Point3& point ) = 0;
		virtual void		GetTangentBasis( int triIndex, const Point3& triBary, int mapChannel, TangentBasis& tangentBasis ) = 0;
};


