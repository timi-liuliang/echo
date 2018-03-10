/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  
/*----------------------------------------------------------------------
		Copyright (c) 2004 Open Dynamics Framework Group
					www.physicstools.org
		All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, are permitted provided
		that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, this list of conditions
		and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright notice,
		this list of conditions and the following disclaimer in the documentation
		and/or other materials provided with the distribution.

		Neither the name of the Open Dynamics Framework Group nor the names of its contributors may
		be used to endorse or promote products derived from this software without specific prior written permission.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES,
		INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
		DISCLAIMED. IN NO EVENT SHALL THE INTEL OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
		EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
		LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
		IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
		THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

#include "foundation/PxPreprocessor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


#include <stdarg.h>

#include "hulllib.h"
#include "PxMemory.h"
#include "PsAlloca.h"

using namespace physx;
using namespace hullLibArray;

namespace physx
{
	//****************************************************
	//** VECMATH.H
	//****************************************************
	#ifndef PLUGIN_3DSMAX
	#define PI (3.1415926535897932384626433832795f)
	#endif

	#define DEG2RAD (PI / 180.0f)
	#define RAD2DEG (180.0f / PI)

	template <class T>
	void Swap(T &a,T &b) 
	{
		T tmp = a;
		a=b;
		b=tmp;
	}

	template <class T>
	T Max(const T &a,const T &b) 
	{
		return (a>b)?a:b;
	}

	template <class T>
	T Min(const T &a,const T &b) 
	{
		return (a<b)?a:b;
	}

	//----------------------------------

	class int3  
	{
	public:
		int x,y,z;
		int3(){}
		int3(int _x,int _y, int _z){x=_x;y=_y;z=_z;}
		const int& operator[](int i) const
		{
			return reinterpret_cast<const int*>(this)[i];
		}
		int& operator[](int i)
		{
			return reinterpret_cast<int*>(this)[i];
		}
	};

	class int4
	{
	public:
		int x,y,z,w;
		int4(){}
		int4(int _x,int _y, int _z,int _w){x=_x;y=_y;z=_z;w=_w;}
		const int& operator[](int i) const
		{
			return reinterpret_cast<const int*>(this)[i];
		}
		int& operator[](int i)
		{
			return reinterpret_cast<int*>(this)[i];
		}
	};

	//--------- 3D ---------

	class float3
	{
		public:
		float x,y,z;
		float3(){x=0;y=0;z=0;}
		float3(float _x,float _y,float _z){x=_x;y=_y;z=_z;}
		//operator float *() { return &x;};
		float& operator[](int i)
		{
			PX_ASSERT(i>=0&&i<3);

			return reinterpret_cast<float*>(this)[i];
		}
		const float& operator[](int i) const
		{
			PX_ASSERT(i>=0&&i<3);

			return reinterpret_cast<const float*>(this)[i];
		}
	#	ifdef PLUGIN_3DSMAX
		float3(const Point3 &p):x(p.x),y(p.y),z(p.z){}
		operator Point3(){return *((Point3*)this);}
	#	endif
	};

	inline int operator==( const float3 &a, const float3 &b ) { return (a.x==b.x && a.y==b.y && a.z==b.z); }
	inline int operator!=( const float3 &a, const float3 &b ) { return (a.x!=b.x || a.y!=b.y || a.z!=b.z); }

	class float3x3
	{
		public:
		float3 x,y,z;  // the 3 rows of the Matrix
		float3x3(){}
		float3x3(float xx,float xy,float xz,float yx,float yy,float yz,float zx,float zy,float zz):x(xx,xy,xz),y(yx,yy,yz),z(zx,zy,zz){}
		float3x3(float3 _x,float3 _y,float3 _z):x(_x),y(_y),z(_z){}
		float3&       operator[](int i)       {PX_ASSERT(i>=0&&i<3);return (&x)[i];}
		const float3& operator[](int i) const {PX_ASSERT(i>=0&&i<3);return (&x)[i];}
		float&        operator()(int r, int c)       {PX_ASSERT(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
		const float&  operator()(int r, int c) const {PX_ASSERT(r>=0&&r<3&&c>=0&&c<3);return ((&x)[r])[c];}
	}; 

	//-------- 4D Math --------

	class float4
	{
	public:
		float x,y,z,w;
		float4(){x=0;y=0;z=0;w=0;}
		float4(float _x,float _y,float _z,float _w){x=_x;y=_y;z=_z;w=_w;}
		float4(const float3 &v,float _w){x=v.x;y=v.y;z=v.z;w=_w;}
		//operator float *() { return &x;};
		float& operator[](int i)
		{
			PX_ASSERT(i>=0&&i<4);

			return reinterpret_cast<float*>(this)[i];
		}
		const float& operator[](int i) const
		{
			PX_ASSERT(i>=0&&i<4);

			return reinterpret_cast<const float*>(this)[i];
		}
		const float3& xyz() const { return *((float3*)this);}
		float3&       xyz()       { return *((float3*)this);}
	};

	//------- Plane ----------

	class Plane
	{
		public:
		float3	normal;
		float	dist;   // distance below origin - the D from plane equasion Ax+By+Cz+D=0
				Plane(const float3 &n,float d):normal(n),dist(d){}
				Plane():normal(),dist(0){}

		float   distance(const float3& point) const { return (point.x*normal.x + point.y*normal.y + point.z*normal.z) + dist; }
	};

	inline int operator==(const Plane& a, const Plane& b)	{ return (a.normal==b.normal && a.dist==b.dist); }

	struct ExpandPlane
	{
		Plane		mPlane;
		int			mAdjacency[3];   // 1 - 0, 2 - 0, 2 - 1
		int			mExpandPoint;
		float		mExpandDistance;		
		int			mIndices[3];
		int			mTrisIndex;

		ExpandPlane()
		{
			for (int i = 0; i < 3; i++)
			{
				mAdjacency[i]  = -1;
				mIndices[i] = -1;
			}

			mExpandDistance = -FLT_MAX;
			mExpandPoint = -1;			
			mTrisIndex = -1;
		}
	};

	//*****************************************************
	// ** VECMATH.CPP
	//*****************************************************

// suppress warnings about unused functions in GCC compliant environments
#if defined(PX_GNUC) || defined(__GCC__) || defined(PX_GHS)
#define UNUSED_FUNCTION __attribute__((unused))
#else
#define UNUSED_FUNCTION
#endif

static float sqr(float a) {return a*a;}

	//------------ float3 (3D) --------------

UNUSED_FUNCTION PX_FORCE_INLINE static float3 operator+( const float3& a, const float3& b )
{
	return float3(a.x+b.x, a.y+b.y, a.z+b.z); 
}

UNUSED_FUNCTION static float3 operator-( const float3& a, const float3& b )
{
	return float3( a.x-b.x, a.y-b.y, a.z-b.z ); 
}

UNUSED_FUNCTION static float3 operator-( const float3& v ) 
{
	return float3( -v.x, -v.y, -v.z ); 
}

UNUSED_FUNCTION static float3 operator*( const float3& v, float s ) 
{
	return float3( v.x*s, v.y*s, v.z*s ); 
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3 operator*( float s, const float3& v )
{
	return float3( v.x*s, v.y*s, v.z*s ); 
}

UNUSED_FUNCTION static float3 operator/( const float3& v, float s )
{ 
	return v*(1.0f/s); 
}

UNUSED_FUNCTION static float dot( const float3& a, const float3& b ) 
{
	return a.x*b.x + a.y*b.y + a.z*b.z; 
}

UNUSED_FUNCTION static float3 cross( const float3& a, const float3& b )
{
	return float3(	a.y*b.z - a.z*b.y,
					a.z*b.x - a.x*b.z,
					a.x*b.y - a.y*b.x );
}

UNUSED_FUNCTION static float3& operator+=( float3& a , const float3& b )
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

UNUSED_FUNCTION static float3& operator-=( float3& a , const float3& b ) 
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

UNUSED_FUNCTION static float3& operator*=(float3& v , float s ) 
{
	v.x *= s;
	v.y *= s;
	v.z *= s;
	return v;
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3& operator/=(float3& v , float s ) 
{
	float sinv = 1.0f / s;
	v.x *= sinv;
	v.y *= sinv;
	v.z *= sinv;
	return v;
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3 vabs(const float3 &v) 
{
	return float3(fabsf(v.x),fabsf(v.y),fabsf(v.z));
}

static float magnitude( const float3& v )
{
	return sqrtf(sqr(v.x) + sqr( v.y)+ sqr(v.z));
}

static float3 normalize( const float3 &v )
{
	// this routine, normalize, is ok, provided magnitude works!!
	float d=magnitude(v);
	if (d==0) 
	{
		printf("Cant normalize ZERO vector\n");
		PX_ASSERT(0);// yes this could go here
		d=0.1f;
	}
	d = 1/d;
	return float3(v.x*d,v.y*d,v.z*d);
}

static float3 VectorMin(const float3 &a,const float3 &b)
{
	return float3(Min(a.x,b.x),Min(a.y,b.y),Min(a.z,b.z));
}
static float3 VectorMax(const float3 &a,const float3 &b)
{
	return float3(Max(a.x,b.x),Max(a.y,b.y),Max(a.z,b.z));
}

	//------------ float3x3 ---------------
static float Determinant(const float3x3 &m)
{
	return  m.x.x*m.y.y*m.z.z + m.y.x*m.z.y*m.x.z + m.z.x*m.x.y*m.y.z 
			 -m.x.x*m.z.y*m.y.z - m.y.x*m.x.y*m.z.z - m.z.x*m.y.y*m.x.z ;
}

static float3x3 Inverse(const float3x3 &a)
{
	float3x3 b;
	float d=Determinant(a);
	PX_ASSERT(d!=0);
	for(int i=0;i<3;i++) 
		{
		for(int j=0;j<3;j++) 
				{
			int i1=(i+1)%3;
			int i2=(i+2)%3;
			int j1=(j+1)%3;
			int j2=(j+2)%3;
			// reverse indexs i&j to take transpose
			b[j][i] = (a[i1][j1]*a[i2][j2]-a[i1][j2]*a[i2][j1])/d;
		}
	}
	// Matrix check=a*b; // Matrix 'check' should be the identity (or close to it)
	return b;
}

UNUSED_FUNCTION static float3x3 Transpose( const float3x3& m ) 
{
	return float3x3( float3(m.x.x,m.y.x,m.z.x),
					float3(m.x.y,m.y.y,m.z.y),
					float3(m.x.z,m.y.z,m.z.z));
}

UNUSED_FUNCTION static float3 operator*(const float3& v , const float3x3 &m ) 
{ 
	return float3((m.x.x*v.x + m.y.x*v.y + m.z.x*v.z), 
					(m.x.y*v.x + m.y.y*v.y + m.z.y*v.z), 
					(m.x.z*v.x + m.y.z*v.y + m.z.z*v.z));
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3 operator*(const float3x3 &m,const float3& v  ) 
{ 
	return float3(dot(m.x,v),dot(m.y,v),dot(m.z,v));
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 operator*( const float3x3& a, const float3x3& b ) 
{ 
	return float3x3(a.x*b,a.y*b,a.z*b);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 operator*( const float3x3& a, const float& s ) 
{ 
	return float3x3(a.x*s, a.y*s ,a.z*s); 
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 operator/( const float3x3& a, const float& s ) 
{ 
	float t=1/s;
	return float3x3(a.x*t, a.y*t ,a.z*t); 
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 operator+( const float3x3& a, const float3x3& b ) 
{
	return float3x3(a.x+b.x, a.y+b.y, a.z+b.z);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 operator-( const float3x3& a, const float3x3& b ) 
{
	return float3x3(a.x-b.x, a.y-b.y, a.z-b.z);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 &operator+=( float3x3& a, const float3x3& b ) 
{
	a.x+=b.x;
	a.y+=b.y;
	a.z+=b.z;
	return a;
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 &operator-=( float3x3& a, const float3x3& b ) 
{
	a.x-=b.x;
	a.y-=b.y;
	a.z-=b.z;
	return a;
}

UNUSED_FUNCTION PX_FORCE_INLINE static float3x3 &operator*=( float3x3& a, const float& s ) 
{
	a.x*=s;
	a.y*=s;
	a.z*=s;
	return a;
}

static float3 ThreePlaneIntersection(const Plane &p0,const Plane &p1, const Plane &p2){
	float3x3 mp =Transpose(float3x3(p0.normal,p1.normal,p2.normal));
	float3x3 mi = Inverse(mp);
	float3 b(p0.dist,p1.dist,p2.dist);
	return -b * mi;
}

	//--------------- 4D ----------------

UNUSED_FUNCTION PX_FORCE_INLINE static int operator==( const float4 &a, const float4 &b ) 
{
	return (a.x==b.x && a.y==b.y && a.z==b.z && a.w==b.w); 
}

UNUSED_FUNCTION PX_FORCE_INLINE static float4 operator*( const float4 &v, float s) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float4 operator*( float s, const float4 &v) 
{
	return float4(v.x*s,v.y*s,v.z*s,v.w*s);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float4 operator+( const float4 &a, const float4 &b) 
{
	return float4(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w);
}

UNUSED_FUNCTION PX_FORCE_INLINE static float4 operator-( const float4 &a, const float4 &b) 
{
	return float4(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w);
}

	//--------- utility functions -------------

static float3 TriNormal(const float3 &v0, const float3 &v1, const float3 &v2)
{
	// return the normal of the triangle
	// inscribed by v0, v1, and v2
	float3 cp=cross(v1-v0,v2-v1);
	float m=magnitude(cp);
	if(m==0) return float3(1.f,0,0);
	return cp*(1.0f/m);
}

	//****************************************************
	// HULL.H source code goes here
	//****************************************************
	class PHullResult
	{
	public:

		PHullResult()
		{
			mVcount		= 0;
			mIndexCount	= 0;
			mFaceCount	= 0;
			mVertices	= 0;
			mIndices	= 0;
		}

		PxU32	mVcount;
		PxU32	mIndexCount;
		PxU32	mFaceCount;
		float*	mVertices;
		PxU32*	mIndices;
	};

	//*****************************************************
	// HULL.cpp source code goes here
	//*****************************************************

	#define REAL3 float3
	#define REAL  float

	#define COPLANAR   (0)
	#define UNDER      (1)
	#define OVER       (2)
	#define SPLIT      (OVER|UNDER)
	#define PAPERWIDTH (0.001f)
	#define VOLUME_EPSILON (1e-20f)

	float planetestepsilon = PAPERWIDTH;

	#if STANDALONE
	class ConvexH 
	#else
	class ConvexH : public Ps::UserAllocated
	#endif
	{
	  public:
		class HalfEdge
		{
		  public:
			short ea;         // the other half of the edge (index into edges list)
			PxU8 v;  // the vertex at the start of this edge (index into vertices list)
			PxU8 p;  // the facet on which this edge lies (index into facets list)
			HalfEdge(){}
			HalfEdge(short _ea, PxU8 _v, PxU8 _p):ea(_ea),v(_v),p(_p){}
		};
		Array<REAL3> vertices;
		Array<HalfEdge> edges;
		Array<Plane>  facets;
		ConvexH(int vertices_size,int edges_size,int facets_size);
	};

	typedef ConvexH::HalfEdge HalfEdge;

	ConvexH::ConvexH(int vertices_size,int edges_size,int facets_size)
		:vertices(vertices_size)
		,edges(edges_size)
		,facets(facets_size)
	{
		vertices.count=vertices_size;
		edges.count   = edges_size;
		facets.count  = facets_size;
	}

	static ConvexH* ConvexHDup(ConvexH* src)
	{
	#if STANDALONE
		ConvexH *dst = new ConvexH(src->vertices.count,src->edges.count,src->facets.count);
	#else
		ConvexH *dst = PX_NEW(ConvexH)(src->vertices.count,src->edges.count,src->facets.count);
	#endif

		PxMemCopy(dst->vertices.element,src->vertices.element,sizeof(float3)*src->vertices.count);
		PxMemCopy(dst->edges.element,src->edges.element,sizeof(HalfEdge)*src->edges.count);
		PxMemCopy(dst->facets.element,src->facets.element,sizeof(Plane)*src->facets.count);
		return dst;
	}

static int PlaneTest(const Plane& p, const REAL3& v)
{
	REAL a = dot(v,p.normal)+p.dist;
	int flag = (a>planetestepsilon)?OVER:((a<-planetestepsilon)?UNDER:COPLANAR);
	return flag;
}

	class VertFlag 
	{
	public:
		PxU8 planetest;
		PxU8 junk;
		PxU8 undermap;
		PxU8 overmap;
	};
	class EdgeFlag 
	{
	public:
		//PxU8 planetest;
		//PxU8 fixes;
		short undermap;
		//short overmap;
	};
	class PlaneFlag 
	{
	public:
		PxU8 undermap;
		//PxU8 overmap;
	};
	class Coplanar{
	public:
		unsigned short ea;
		PxU8 v0;
		PxU8 v1;
	};

static int AssertIntact(ConvexH& convex)
{
	int i;
	int estart=0;
	for(i=0;i<convex.edges.count;i++) {
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		int inext = i+1;
		if(inext>= convex.edges.count || convex.edges[inext].p != convex.edges[i].p) {
			inext = estart;
		}
		PX_ASSERT(convex.edges[inext].p == convex.edges[i].p);
		//HalfEdge &edge = convex.edges[i];
		int nb = convex.edges[i].ea;
		PX_ASSERT(nb!=255);
		if(nb==255 || nb==-1) return 0;
		PX_ASSERT(nb!=-1);
		PX_ASSERT(i== convex.edges[nb].ea);
		// Check that the vertex of the next edge is the vertex of the adjacent half edge.
		// Otherwise the two half edges are not really adjacent and we have a hole.
		PX_ASSERT(convex.edges[nb].v == convex.edges[inext].v);
		if(!(convex.edges[nb].v == convex.edges[inext].v)) return 0;
	}
	for(i=0;i<convex.edges.count;i++) {
		PX_ASSERT(COPLANAR==PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v]));
		if(COPLANAR!=PlaneTest(convex.facets[convex.edges[i].p],convex.vertices[convex.edges[i].v])) return 0;
		if(convex.edges[estart].p!= convex.edges[i].p) {
			estart=i;
		}
		int i1 = i+1;
		if(i1>= convex.edges.count || convex.edges[i1].p != convex.edges[i].p) {
			i1 = estart;
		}
		int i2 = i1+1;
		if(i2>= convex.edges.count || convex.edges[i2].p != convex.edges[i].p) {
			i2 = estart;
		}
		if(i==i2) continue; // i sliced tangent to an edge and created 2 meaningless edges
		REAL3 localnormal = TriNormal(convex.vertices[convex.edges[i ].v],
									   convex.vertices[convex.edges[i1].v],
									   convex.vertices[convex.edges[i2].v]);
		//PX_ASSERT(dot(localnormal,convex.facets[convex.edges[i].p].normal)>0);//Commented out on Stan Melax' advice
		if(dot(localnormal,convex.facets[convex.edges[i].p].normal)<=0)return 0;
	}
	return 1;
}

static ConvexH* test_cube()
{
#if STANDALONE
	ConvexH *convex = new ConvexH(8,24,6);
#else
	ConvexH *convex = PX_NEW_TEMP(ConvexH)(8,24,6);
#endif
	convex->vertices[0] = REAL3(0,0,0);
	convex->vertices[1] = REAL3(0,0,1.f);
	convex->vertices[2] = REAL3(0,1.f,0);
	convex->vertices[3] = REAL3(0,1.f,1.f);
	convex->vertices[4] = REAL3(1.f,0,0);
	convex->vertices[5] = REAL3(1.f,0,1.f);
	convex->vertices[6] = REAL3(1.f,1.f,0);
	convex->vertices[7] = REAL3(1.f,1.f,1.f);

	convex->facets[0] = Plane(REAL3(-1.f,0,0),0);
	convex->facets[1] = Plane(REAL3(1.f,0,0),-1.f);
	convex->facets[2] = Plane(REAL3(0,-1.f,0),0);
	convex->facets[3] = Plane(REAL3(0,1.f,0),-1.f);
	convex->facets[4] = Plane(REAL3(0,0,-1.f),0);
	convex->facets[5] = Plane(REAL3(0,0,1.f),-1.f);

	convex->edges[0 ] = HalfEdge(11,0,0);
	convex->edges[1 ] = HalfEdge(23,1,0);
	convex->edges[2 ] = HalfEdge(15,3,0);
	convex->edges[3 ] = HalfEdge(16,2,0);

	convex->edges[4 ] = HalfEdge(13,6,1);
	convex->edges[5 ] = HalfEdge(21,7,1);
	convex->edges[6 ] = HalfEdge( 9,5,1);
	convex->edges[7 ] = HalfEdge(18,4,1);

	convex->edges[8 ] = HalfEdge(19,0,2);
	convex->edges[9 ] = HalfEdge( 6,4,2);
	convex->edges[10] = HalfEdge(20,5,2);
	convex->edges[11] = HalfEdge( 0,1,2);

	convex->edges[12] = HalfEdge(22,3,3);
	convex->edges[13] = HalfEdge( 4,7,3);
	convex->edges[14] = HalfEdge(17,6,3);
	convex->edges[15] = HalfEdge( 2,2,3);

	convex->edges[16] = HalfEdge( 3,0,4);
	convex->edges[17] = HalfEdge(14,2,4);
	convex->edges[18] = HalfEdge( 7,6,4);
	convex->edges[19] = HalfEdge( 8,4,4);
	
	convex->edges[20] = HalfEdge(10,1,5);
	convex->edges[21] = HalfEdge( 5,5,5);
	convex->edges[22] = HalfEdge(12,7,5);
	convex->edges[23] = HalfEdge( 1,3,5);
	
	return convex;
}

static ConvexH* ConvexHMakeCube(const REAL3& bmin, const REAL3& bmax)
{
	ConvexH* convex = test_cube();
	convex->vertices[0] = REAL3(bmin.x,bmin.y,bmin.z);
	convex->vertices[1] = REAL3(bmin.x,bmin.y,bmax.z);
	convex->vertices[2] = REAL3(bmin.x,bmax.y,bmin.z);
	convex->vertices[3] = REAL3(bmin.x,bmax.y,bmax.z);
	convex->vertices[4] = REAL3(bmax.x,bmin.y,bmin.z);
	convex->vertices[5] = REAL3(bmax.x,bmin.y,bmax.z);
	convex->vertices[6] = REAL3(bmax.x,bmax.y,bmin.z);
	convex->vertices[7] = REAL3(bmax.x,bmax.y,bmax.z);

	convex->facets[0] = Plane(REAL3(-1.f,0,0), bmin.x);
	convex->facets[1] = Plane(REAL3(1.f,0,0), -bmax.x);
	convex->facets[2] = Plane(REAL3(0,-1.f,0), bmin.y);
	convex->facets[3] = Plane(REAL3(0,1.f,0), -bmax.y);
	convex->facets[4] = Plane(REAL3(0,0,-1.f), bmin.z);
	convex->facets[5] = Plane(REAL3(0,0,1.f), -bmax.z);
	return convex;
}

static ConvexH* ConvexHCrop(ConvexH& convex, const Plane& slice)
{
	static const int invalidIndex = (PxU8)-1;

	PxU16 i;

	PxU16 vertcountunder=0; // Running count of the vertices UNDER the slicing plane.

	PX_ASSERT(convex.edges.count<480);

	// Arrays of mapping information associated with features in the input convex.
	// edgeflag[i].undermap  - output index of input edge convex->edges[i]
	// vertflag[i].undermap  - output index of input vertex convex->vertices[i]
	// vertflag[i].planetest - the side-of-plane classification of convex->vertices[i]
	// planeflag[i].undermap - output index of input plane convex->planes[i]
	// (There are other members but they are unused.)
	EdgeFlag  edgeflag[512];
	VertFlag  vertflag[256];

	// Lists of output features. Populated during clipping.
	// Coplanar edges have one sibling in tmpunderedges and one in coplanaredges.
	// coplanaredges holds the sibling that belong to the new polygon created from slicing.
	HalfEdge  tmpunderedges[512];  // The output edge list.
	Plane	  tmpunderplanes[128]; // The output plane list.
	Coplanar  coplanaredges[512];  // The coplanar edge list.

	int coplanaredges_num=0; // Running count of coplanar edges.

	// Created vertices on the slicing plane (stored for output after clipping).
	Array<REAL3> createdverts;
	
	// Logical OR of individual vertex flags.
	int convexClipFlags = 0;
	
	// Classify each vertex against the slicing plane as OVER | COPLANAR | UNDER.
	// OVER     - Vertex is over (outside) the slicing plane. Will not be output.
	// COPLANAR - Vertex is on the slicing plane. A copy will be output.
	// UNDER    - Vertex is under (inside) the slicing plane. Will be output.
	// We keep an array of information structures for each vertex in the input convex.
	// vertflag[i].undermap  - The (computed) index of convex->vertices[i] in the output.
	//                         invalidIndex for OVER vertices - they are not output.
	//                         initially invalidIndex for COPLANAR vertices - set later.
	// vertflag[i].overmap   - Unused - we don't care about the over part.
	// vertflag[i].planetest - The classification (clip flag) of convex->vertices[i].
	for(i=0;i<convex.vertices.count;i++) 
	{
		int vertexClipFlag = PlaneTest(slice,convex.vertices[i]);
		switch (vertexClipFlag)
		{
			case OVER:
			case COPLANAR:
				vertflag[i].undermap = invalidIndex; // Initially invalid for COPLANAR
				vertflag[i].overmap  = invalidIndex;
				break;
			case UNDER:
				vertflag[i].undermap = Ps::to8(vertcountunder++);
				vertflag[i].overmap  = invalidIndex;
				break;
			default:
				PX_ASSERT(0);
				return NULL;
		}
		vertflag[i].planetest = PxU8(vertexClipFlag);
		convexClipFlags |= vertexClipFlag;
	}

	// Check special case: everything UNDER or COPLANAR.
	// This way we know we wont end up with silly faces / edges later on.
	if ( (convexClipFlags & OVER) == 0 ) 
	{
		// Just return a copy of the same convex.
		return ConvexHDup(&convex);
	}

	PxU16 under_edge_count = 0; // Running count of output edges.
	PxU16 underplanescount = 0; // Running count of output planes.

	// Clipping Loop
	// =============
	//
	// for each plane
	//
	//    for each edge
	//
	//       if first UNDER & second !UNDER
	//          output current edge -> tmpunderedges
	//          if we have done the sibling
	//             connect current edge to its sibling
	//             set vout = first vertex of sibling
	//          else if second is COPLANAR
	//             if we havent already copied it
	//                copy second -> createdverts
	//             set vout = index of created vertex
	//          else
	//             generate a new vertex -> createdverts
	//             set vout = index of created vertex
	//          if vin is already set and vin != vout (non-trivial edge)
	//             output coplanar edge -> tmpunderedges (one sibling)
	//             set coplanaredge to new edge index (for connecting the other sibling)
	//
	//       else if first !UNDER & second UNDER
	//          if we have done the sibling
	//             connect current edge to its sibling
	//             set vin = second vertex of sibling (this is a bit of a pain)
	//          else if first is COPLANAR
	//             if we havent already copied it
	//                copy first -> createdverts
	//             set vin = index of created vertex
	//          else
	//             generate a new vertex -> createdverts
	//             set vin = index of created vertex
	//          if vout is already set and vin != vout (non-trivial edge)
	//             output coplanar edge -> tmpunderedges (one sibling)
	//             set coplanaredge to new edge index (for connecting the other sibling)
	//          output current edge -> tmpunderedges
	//
	//       else if first UNDER & second UNDER
	//          output current edge -> tmpunderedges
	//
	//    next edge
	//
	//    if part of current plane was UNDER
	//       output current plane -> tmpunderplanes
	//
	//    if coplanaredge is set
	//       output coplanar edge -> coplanaredges
	// 
	// next plane
	// 

	// Indexing is a bit tricky here:
	// 
	// e0           - index of the current edge
	// e1           - index of the next edge
	// estart       - index of the first edge in the current plane
	// currentplane - index of the current plane
	// enextface    - first edge of next plane

	int e0 = 0;
	
	for(int currentplane=0; currentplane<convex.facets.count; currentplane++) 
	{
		
		int estart = e0;
		int enextface = 0xffffffff;
		int e1 = e0 + 1;
		
		PxU8 vout= invalidIndex;
		PxU8 vin = invalidIndex;
		
		int coplanaredge = invalidIndex;
		
		// Logical OR of individual vertex flags in the current plane.
		int planeside = 0;

		do{

			// Next edge modulo logic
			if(e1 >= convex.edges.count || convex.edges[e1].p!=currentplane) 
			{
				enextface = e1;
				e1=estart;
			}
			
			HalfEdge& edge0 = convex.edges[e0];
			HalfEdge& edge1 = convex.edges[e1];
			HalfEdge& edgea = convex.edges[edge0.ea];

			planeside |= vertflag[edge0.v].planetest;

			if (vertflag[edge0.v].planetest == UNDER && vertflag[edge1.v].planetest != UNDER) 
			{
				// first is UNDER, second is COPLANAR or OVER
				
				// Output current edge.
				edgeflag[e0].undermap = short(under_edge_count);
				tmpunderedges[under_edge_count].v = vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = PxU8(underplanescount);
				PX_ASSERT(tmpunderedges[under_edge_count].v != invalidIndex);

				if (edge0.ea < e0) 
				{
					// We have already done the sibling.
					// Connect current edge to its sibling.
					PX_ASSERT(edgeflag[edge0.ea].undermap != invalidIndex);
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = short(under_edge_count);
					// Set vout = first vertex of (output, clipped) sibling.
					vout = tmpunderedges[edgeflag[edge0.ea].undermap].v;
				}
				else if (vertflag[edge1.v].planetest == COPLANAR)
				{
					// Boundary case.
					// We output coplanar vertices once.
					if (vertflag[edge1.v].undermap == invalidIndex)
					{
						createdverts.Add(convex.vertices[edge1.v]);
						// Remember the index so we don't output it again.
						vertflag[edge1.v].undermap = Ps::to8(vertcountunder++);
					}
					vout = vertflag[edge1.v].undermap;
				}
				else 
				{
					// Add new vertex.
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					vout = Ps::to8(vertcountunder++);
				}
				
				// We added an edge, increment the counter
				under_edge_count++;

				if (vin != invalidIndex && vin != vout)
				{
					// We already have vin and a non-trivial edge
					// Output coplanar edge
					PX_ASSERT(vout != invalidIndex);
					coplanaredge = under_edge_count;
					tmpunderedges[under_edge_count].v = vout;
					tmpunderedges[under_edge_count].p = PxU8(underplanescount);
					tmpunderedges[under_edge_count].ea = invalidIndex;
					under_edge_count++;
				}
			}
			else if(vertflag[edge0.v].planetest != UNDER && vertflag[edge1.v].planetest == UNDER) 
			{
				// First is OVER or COPLANAR, second is UNDER.

				if (edge0.ea < e0) 
				{
					// We have already done the sibling.
					// We need the second vertex of the sibling.
					// Which is the vertex of the next edge in the adjacent poly.
					int nea = edgeflag[edge0.ea].undermap + 1;
					int p = tmpunderedges[edgeflag[edge0.ea].undermap].p;
					if (nea >= under_edge_count || tmpunderedges[nea].p != p)
					{
						// End of polygon, next edge is first edge
						nea -= 2;
						while (nea > 0 && tmpunderedges[nea-1].p == p) 
							nea--;
					}
					vin = tmpunderedges[nea].v;
					PX_ASSERT(vin < vertcountunder);
				}
				else if (vertflag[edge0.v].planetest == COPLANAR)
				{
					// Boundary case.
					// We output coplanar vertices once.
					if (vertflag[edge0.v].undermap == invalidIndex)
					{
						createdverts.Add(convex.vertices[edge0.v]);
						// Remember the index so we don't output it again.
						vertflag[edge0.v].undermap = Ps::to8(vertcountunder++);
					}
					vin = vertflag[edge0.v].undermap;
				}
				else 
				{
					// Add new vertex.
					Plane &p0 = convex.facets[edge0.p];
					Plane &pa = convex.facets[edgea.p];
					createdverts.Add(ThreePlaneIntersection(p0,pa,slice));
					vin = Ps::to8(vertcountunder++);
				}
				
				if (vout != invalidIndex && vin != vout)
				{
					// We have been in and out, Add the coplanar edge
					coplanaredge = under_edge_count;
					tmpunderedges[under_edge_count].v = vout;
					tmpunderedges[under_edge_count].p = Ps::to8(underplanescount);
					tmpunderedges[under_edge_count].ea = invalidIndex;
					under_edge_count++;
				}

				// Output current edge.
				tmpunderedges[under_edge_count].v = vin;
				tmpunderedges[under_edge_count].p = Ps::to8(underplanescount);
				edgeflag[e0].undermap = (short)under_edge_count;
				
				if(edge0.ea < e0) 
				{
					// We have already done the sibling.
					// Connect current edge to its sibling.
					PX_ASSERT(edgeflag[edge0.ea].undermap != invalidIndex);
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = (short)under_edge_count;
				}
				
				PX_ASSERT(edgeflag[e0].undermap == under_edge_count);
				under_edge_count++;
			}
			else if (vertflag[edge0.v].planetest == UNDER && vertflag[edge1.v].planetest == UNDER)
			{
				// Both UNDER

				// Output current edge.
				edgeflag[e0].undermap = (short)under_edge_count;
				tmpunderedges[under_edge_count].v = vertflag[edge0.v].undermap;
				tmpunderedges[under_edge_count].p = Ps::to8(underplanescount);
				if(edge0.ea < e0) {
					// We have already done the sibling.
					// Connect current edge to its sibling.
					PX_ASSERT(edgeflag[edge0.ea].undermap != invalidIndex);
					tmpunderedges[under_edge_count].ea = edgeflag[edge0.ea].undermap;
					tmpunderedges[edgeflag[edge0.ea].undermap].ea = (short)under_edge_count;
				}
				under_edge_count++;
			}

			e0 = e1;
			e1++; // do the modulo at the beginning of the loop

		} while(e0 != estart) ;
		
		e0 = enextface;
		
		if(planeside & UNDER) 
		{
			// At least part of current plane is UNDER.
			// Output current plane.
			tmpunderplanes[underplanescount] = convex.facets[currentplane];
			underplanescount++;
		}

		if(coplanaredge != invalidIndex) 
		{
			// We have a coplanar edge.
			// Add to coplanaredges for later processing.
			// (One sibling is in place but one is missing)
			PX_ASSERT(vin  != invalidIndex);
			PX_ASSERT(vout != invalidIndex);
			PX_ASSERT(coplanaredge != 511);
			coplanaredges[coplanaredges_num].ea = PxU8(coplanaredge);
			coplanaredges[coplanaredges_num].v0 = vin;
			coplanaredges[coplanaredges_num].v1 = vout;
			coplanaredges_num++;
		}

		// Reset coplanar edge infos for next poly
		vin = invalidIndex;
		vout = invalidIndex;
		coplanaredge = invalidIndex;
	}

	// Add the new plane to the mix:
	if(coplanaredges_num > 0) 
	{
		tmpunderplanes[underplanescount++] = slice;
	}

	// Sort the coplanar edges in winding order.
	for(i=0;i<coplanaredges_num-1;i++) 
	{
		if(coplanaredges[i].v1 != coplanaredges[i+1].v0) 
		{
			int j = 0;
			for(j=i+2;j<coplanaredges_num;j++) 
			{
				if(coplanaredges[i].v1 == coplanaredges[j].v0) 
				{
					Coplanar tmp = coplanaredges[i+1];
					coplanaredges[i+1] = coplanaredges[j];
					coplanaredges[j] = tmp;
					break;
				}
			}
			if(j>=coplanaredges_num)
			{
				// PX_ASSERT(j<coplanaredges_num);
				return NULL;
			}
		}
	}

	// PT: added this line to fix DE2904
	if(!vertcountunder)
		return NULL;

	// Create the output convex.
#if STANDALONE
	ConvexH *punder = new ConvexH(vertcountunder,under_edge_count+coplanaredges_num,underplanescount);
#else
	ConvexH *punder = PX_NEW_TEMP(ConvexH)(vertcountunder,under_edge_count+coplanaredges_num,underplanescount);
#endif

	ConvexH &under = *punder;

	// Copy UNDER vertices
	int k=0;
	for(i=0;i<convex.vertices.count;i++) 
	{
		if(vertflag[i].planetest == UNDER)
		{
			under.vertices[k++] = convex.vertices[i];
		}
	}

	// Copy created vertices
	i=0;
	while(k<vertcountunder) 
	{
		under.vertices[k++] = createdverts[i++];
	}
	
	PX_ASSERT(i==createdverts.count);

	// Add the coplanar edge siblings that belong to the new polygon (coplanaredges).
	for(i=0;i<coplanaredges_num;i++) 
	{
		under.edges[under_edge_count+i].p  = PxU8(underplanescount-1);
		under.edges[under_edge_count+i].ea = (short)coplanaredges[i].ea;
		tmpunderedges[coplanaredges[i].ea].ea = under_edge_count+i;
		under.edges[under_edge_count+i].v  = coplanaredges[i].v0;
	}

	// Copy the output edges and output planes.
	PxMemCopy(under.edges.element,tmpunderedges,sizeof(HalfEdge)*under_edge_count);
	PxMemCopy(under.facets.element,tmpunderplanes,sizeof(Plane)*underplanescount);

	return punder;
}

static const float minadjangle = 3.0f;  // in degrees  - result wont have two adjacent facets within this angle of each other.
static int candidateplane(Plane* planes, int planes_count, ConvexH* convex, float epsilon)
{
	int p =-1;
	REAL md=0;
	int i,j;
	float maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(i=0;i<planes_count;i++)
	{
		float d=0;
		float dmax=0;
		float dmin=0;
		for(j=0;j<convex->vertices.count;j++)
		{
			dmax = Max(dmax,dot(convex->vertices[j],planes[i].normal)+planes[i].dist);
			dmin = Min(dmin,dot(convex->vertices[j],planes[i].normal)+planes[i].dist);
		}
		float dr = dmax-dmin;
		if(dr<planetestepsilon) dr=1.0f; // shouldn't happen.
		d = dmax /dr;
		if(d<=md) continue;
		for(j=0;j<convex->facets.count;j++)
		{
			if(planes[i]==convex->facets[j]) 
			{
				d=0;continue;
			}
			if(dot(planes[i].normal,convex->facets[j].normal)>maxdot_minang)
			{
				for(int k=0;k<convex->edges.count;k++)
				{
					if(convex->edges[k].p!=j) continue;
					if(dot(convex->vertices[convex->edges[k].v],planes[i].normal)+planes[i].dist<0)
					{
						d=0; // so this plane wont get selected.
						break;
					}
				}
			}
		}
		if(d>md)
		{
			p=i;
			md=d;
		}
	}
	return (md>epsilon)?p:-1;
}

template<class T>
inline int maxdir(const T *p,int count,const T &dir)
{
	PX_ASSERT(count);
	int m=0;
	for(int i=1;i<count;i++)
	{
		if(dot(p[i],dir)>dot(p[m],dir)) m=i;
	}
	return m;
}

// finds the max vertex along given direction with respect to allowed vertices
template<class T>
int maxdirfiltered(const T *p,int count,const T &dir, bool* tempNotAllowed)
{
	PX_ASSERT(count);
	int m=-1;
	for(int i=0;i<count;i++) if(!tempNotAllowed[i])
	{
		if(m==-1 || dot(p[i],dir)>dot(p[m],dir)) m=i;
	}
	PX_ASSERT(m!=-1);
	return m;
} 

static float3 orth(const float3& v)
{
	float3 a=cross(v,float3(0,0,1.f));
	float3 b=cross(v,float3(0,1.f,0));
	return normalize((magnitude(a)>magnitude(b))?a:b);
}

// finds the max vertex along given direction
template<class T>
int maxdirsterid(const T *p,int count,const T &dir,Array<int> &allow)
{
	// if the found vertex does not get hit by a slightly rotated ray, it
	// may not be the extreme we are looking for. Therefore it is marked
	// as disabled for the direction search and different candidate is chosen.
	PX_ALLOCA(tempNotAllowed,bool,count);
	PxMemSet(tempNotAllowed,0,count*sizeof(bool));
	
	int m=-1;
	while(m==-1)
	{
		m = maxdirfiltered(p,count,dir,tempNotAllowed);
		if(allow[m]==3) return m;
		T u = orth(dir);
		T v = cross(u,dir);
		int ma=-1;
		for(float x = 0.0f ; x<= 360.0f ; x+= 45.0f)
		{
			float s0 = sinf(DEG2RAD*(x));
			float c0 = cosf(DEG2RAD*(x));
			int mb = maxdirfiltered(p,count,dir+(u*s0+v*c0)*0.025f,tempNotAllowed);
			if(ma==m && mb==m)
			{
				allow[m]=3;
				return m;
			}
			if(ma!=-1 && ma!=mb)
			{
				int mc = ma;
				for(float xx = x-40.0f ; xx <= x ; xx+= 5.0f)
				{
					float s = sinf(DEG2RAD*(xx));
					float c = cosf(DEG2RAD*(xx));
					int md = maxdirfiltered(p,count,dir+(u*s+v*c)*0.025f,tempNotAllowed);
					if(mc==m && md==m)
					{
						allow[m]=3;
						return m;
					}
					mc=md;
				}
			}
			ma=mb;
		}
		tempNotAllowed[m]=true;
		m=-1;
	}
	PX_ASSERT(0);
	return m;
} 

static int above(float3* vertices, const int3& t, const float3& p, float epsilon)
{
	float3 n=TriNormal(vertices[t[0]],vertices[t[1]],vertices[t[2]]);
	return (dot(n,p-vertices[t[0]]) > epsilon); // EPSILON???
}

static int hasvert(const int3& t, int v)
{
	return (t[0]==v || t[1]==v || t[2]==v) ;
}

class Tri;
static void deleteTri(Tri* tri, Array<Tri*>* mTris);

	#if STANDALONE
	class Tri : public int3
	#else
	class Tri : public int3, public Ps::UserAllocated
	#endif
	{
	public:
		int3 n;
		int id;
		int vmax;
		float rise;
		Tri(int a, int b, int c, Array<Tri*>* mTris) : int3(a, b, c), n(-1,-1,-1)
		{
			id = mTris->count;
			mTris->Add(this);
			vmax=-1;
			rise = 0.0f;
		}
		int& neib(int a, int b);

		protected:
		~Tri()
		{
		}

		friend /*static*/ void deleteTri(Tri* tri, Array<Tri*>* mTris);
	};

int& Tri::neib(int a, int b)
{
	static int er=-1;
	for(int i=0;i<3;i++) 
	{
		int i1=(i+1)%3;
		int i2=(i+2)%3;
		if((*this)[i]==a && (*this)[i1]==b) return n[i2];
		if((*this)[i]==b && (*this)[i1]==a) return n[i2];
	}
	PX_ASSERT(0);
	return er;
}

static void deleteTri(Tri* tri, Array<Tri*>* mTris)
{
	PX_ASSERT((*mTris)[tri->id]==tri);
	(*mTris)[tri->id] = NULL;
	delete tri;
}

HullLibrary::HullLibrary()
{
	mTris=(Array<Tri*>*)PX_ALLOC(sizeof(Array<Tri*>), PX_DEBUG_EXP("Array<Tri*>"));
	PX_PLACEMENT_NEW(mTris,Array<Tri*>);
}

HullLibrary::~HullLibrary()
{
	mTris->~Array<Tri*>();
	PX_FREE(mTris);
}

void HullLibrary::b2bfix(Tri* s, Tri* t)
{
	for(int i=0;i<3;i++) 
	{
		const int i1=(i+1)%3;
		const int i2=(i+2)%3;
		const int a = (*s)[i1];
		const int b = (*s)[i2];
		PX_ASSERT((*mTris)[s->neib(a,b)]->neib(b,a) == s->id);
		PX_ASSERT((*mTris)[t->neib(a,b)]->neib(b,a) == t->id);
		(*mTris)[s->neib(a,b)]->neib(b,a) = t->neib(b,a);
		(*mTris)[t->neib(b,a)]->neib(a,b) = s->neib(a,b);
	}
}

void HullLibrary::removeb2b(Tri* s, Tri* t)
{
	b2bfix(s,t);
	deleteTri(s, mTris);
	deleteTri(t, mTris);
}

void HullLibrary::checkit(Tri* t) const
{
	PX_ASSERT((*mTris)[t->id]==t);
	for(int i=0;i<3;i++)
	{
		const int i1=(i+1)%3;
		const int i2=(i+2)%3;
		const int a = (*t)[i1];
		const int b = (*t)[i2];
		PX_ASSERT(a!=b);
		PX_ASSERT( (*mTris)[t->n[i]]->neib(b,a) == t->id);
		PX_UNUSED(a);
		PX_UNUSED(b);
	}
}

void HullLibrary::extrude(Tri* t0, int v)
{
	int3 t= *t0;
	int n = mTris->count;
#if STANDALONE
	Tri* ta = new Tri(v, t[1], t[2], mTris);
#else
	Tri* ta = PX_NEW_TEMP(Tri)(v, t[1], t[2], mTris);
#endif
	ta->n = int3(t0->n[0],n+1,n+2);
	(*mTris)[t0->n[0]]->neib(t[1],t[2]) = n+0;
#if STANDALONE
	Tri* tb = new Tri(v, t[2], t[0], mTris);
#else
	Tri* tb = PX_NEW_TEMP(Tri)(v, t[2], t[0], mTris);
#endif
	tb->n = int3(t0->n[1],n+2,n+0);
	(*mTris)[t0->n[1]]->neib(t[2],t[0]) = n+1;
#if STANDALONE
	Tri* tc = new Tri(v, t[0], t[1], mTris);
#else
	Tri* tc = PX_NEW_TEMP(Tri)(v, t[0], t[1], mTris);
#endif
	tc->n = int3(t0->n[2],n+0,n+1);
	(*mTris)[t0->n[2]]->neib(t[0],t[1]) = n+2;
	checkit(ta);
	checkit(tb);
	checkit(tc);
	if(hasvert(*(*mTris)[ta->n[0]],v)) removeb2b(ta,(*mTris)[ta->n[0]]);
	if(hasvert(*(*mTris)[tb->n[0]],v)) removeb2b(tb,(*mTris)[tb->n[0]]);
	if(hasvert(*(*mTris)[tc->n[0]],v)) removeb2b(tc,(*mTris)[tc->n[0]]);
	deleteTri(t0, mTris);
}

Tri* HullLibrary::extrudable(float epsilon) const
{
	Tri* t=NULL;
	for(int i=0;i<mTris->count;i++)
	{
		if(!t || ((*mTris)[i] && t->rise<(*mTris)[i]->rise))
		{
			t = (*mTris)[i];
		}
	}
	if(!t)
		return NULL;
	return (t->rise >epsilon)?t:NULL;
}

static bool hasVolume(float3* verts, int p0, int p1, int p2, int p3)
{
	const float3 result3 = cross(verts[p1]-verts[p0], verts[p2]-verts[p0]);
	if (magnitude(result3) < VOLUME_EPSILON && magnitude(result3) > -VOLUME_EPSILON) // Almost collinear or otherwise very close to each other
		return false;
	const float result = dot(normalize(result3), verts[p3]-verts[p0]);
	return (result > VOLUME_EPSILON || result < -VOLUME_EPSILON); // Returns true iff volume is significantly non-zero
}

static int4 FindSimplex(float3* verts, int verts_count, Array<int>& allow,const float3& minMax)
{
	float3 basisVector[3];
	basisVector[0] = float3( 1.0f, 0.02f, 0.01f);
	basisVector[1] = float3(-0.02f, 1.0f, -0.01f);
	basisVector[2] = float3( 0.01f, 0.02f, 1.0f );

	int index0 = 0;
	int index1 = 1;
	int index2 = 2;

	if(minMax.z > minMax.x && minMax.z > minMax.y)
	{
		index0 = 2;
		index1 = 0;
		index2 = 1;
	}
	else
	{
		if(minMax.y > minMax.x && minMax.y > minMax.z)
		{
			index0 = 1;
			index1 = 2;
			index2 = 0;
		}
	}

	float3 basis[3];
	basis[0] = basisVector[index0]; //float3( 0.01f, 0.02f, 1.0f );      
	int p0 = maxdirsterid(verts,verts_count, basis[0],allow);
	int	p1 = maxdirsterid(verts,verts_count,-basis[0],allow);
	basis[0] = verts[p0]-verts[p1];
	if(p0==p1 || basis[0]==float3(0,0,0)) 
		return int4(-1,-1,-1,-1);
	basis[1] = cross(basisVector[index1],basis[0]);  //cross(float3(     1, 0.02f, 0),basis[0]);
	basis[2] = cross(basisVector[index2],basis[0]); //cross(float3(-0.02f,     1, 0),basis[0]);
	basis[1] = normalize( (magnitude(basis[1])>magnitude(basis[2])) ? basis[1]:basis[2]);
	int p2 = maxdirsterid(verts,verts_count,basis[1],allow);
	if(p2 == p0 || p2 == p1)
	{
		p2 = maxdirsterid(verts,verts_count,-basis[1],allow);
	}
	if(p2 == p0 || p2 == p1) 
		return int4(-1,-1,-1,-1);
	basis[1] = verts[p2] - verts[p0];
	basis[2] = normalize(cross(basis[1],basis[0]));
	int p3 = maxdirsterid(verts,verts_count,basis[2],allow);
	if(p3==p0||p3==p1||p3==p2||!hasVolume(verts, p0, p1, p2, p3)) p3 = maxdirsterid(verts,verts_count,-basis[2],allow);
	if(p3==p0||p3==p1||p3==p2) 
		return int4(-1,-1,-1,-1);
	PX_ASSERT(!(p0==p1||p0==p2||p0==p3||p1==p2||p1==p3||p2==p3));
	if(dot(verts[p3]-verts[p0],cross(verts[p1]-verts[p0],verts[p2]-verts[p0])) <0) {Swap(p2,p3);}
	return int4(p0,p1,p2,p3);
}

HullError HullLibrary::calchullgen(float3* verts, int verts_count, int vlimit, float areaTestEpsilon)
{
	if(verts_count <4) return QE_FAIL;
	if(vlimit==0) vlimit=1000000000;
	int j;
	float3 bmin(*verts),bmax(*verts);
	Array<int> isextreme(verts_count);
	Array<int> allow(verts_count);
	for(j=0;j<verts_count;j++) 
	{
		allow.Add(1);
		isextreme.Add(0);
		bmin = VectorMin(bmin,verts[j]);
		bmax = VectorMax(bmax,verts[j]);
	}
	float epsilon = magnitude(bmax-bmin) * 0.001f;
	const bool useAreaTest  = (areaTestEpsilon == 0.0f) ? false : true;
	const float areaEpsilon = useAreaTest ?  areaTestEpsilon*2.0f : epsilon*epsilon*0.1f;

	int4 p = FindSimplex(verts,verts_count,allow, bmax - bmin);
	if(p.x==-1) return QE_FAIL; // simplex failed

	float3 center = (verts[p[0]]+verts[p[1]]+verts[p[2]]+verts[p[3]]) /4.0f;  // a valid interior point
#if STANDALONE
	Tri *t0 = new Tri(p[2], p[3], p[1], mTris); t0->n=int3(2,3,1);
	Tri *t1 = new Tri(p[3], p[2], p[0], mTris); t1->n=int3(3,2,0);
	Tri *t2 = new Tri(p[0], p[1], p[3], mTris); t2->n=int3(0,1,3);
	Tri *t3 = new Tri(p[1], p[0], p[2], mTris); t3->n=int3(1,0,2);
#else
	Tri *t0 = PX_NEW_TEMP(Tri)(p[2], p[3], p[1], mTris); t0->n=int3(2,3,1);
	Tri *t1 = PX_NEW_TEMP(Tri)(p[3], p[2], p[0], mTris); t1->n=int3(3,2,0);
	Tri *t2 = PX_NEW_TEMP(Tri)(p[0], p[1], p[3], mTris); t2->n=int3(0,1,3);
	Tri *t3 = PX_NEW_TEMP(Tri)(p[1], p[0], p[2], mTris); t3->n=int3(1,0,2);
#endif
	isextreme[p[0]]=isextreme[p[1]]=isextreme[p[2]]=isextreme[p[3]]=1;
	checkit(t0);checkit(t1);checkit(t2);checkit(t3);

	for(j=0;j<mTris->count;j++)
	{
		Tri *t=(*mTris)[j];
		PX_ASSERT(t);
		PX_ASSERT(t->vmax<0);
		float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		t->vmax = maxdirsterid(verts,verts_count,n,allow);
		t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
		if(useAreaTest && magnitude(cross(verts[(*t)[1]]-verts[(*t)[0]],verts[(*t)[2]]-verts[(*t)[1]]))< areaEpsilon)
		{
			deleteTri(t0,mTris);
			deleteTri(t1,mTris);
			deleteTri(t2,mTris);
			deleteTri(t3,mTris);
			return QE_AREA_TEST_FAIL;
		}
	}
	Tri *te;
	vlimit-=4;
	while(vlimit >0 && ((te=extrudable(epsilon)) != NULL))
	{
		//int3 ti=*te;
		int v=te->vmax;
		PX_ASSERT(!isextreme[v]);  // wtf we've already done this vertex
		isextreme[v]=1;
		//if(v==p0 || v==p1 || v==p2 || v==p3) continue; // done these already
		j=mTris->count;
		while(j--) {
			if(!(*mTris)[j]) continue;
			int3 t=*(*mTris)[j];
			if(above(verts,t,verts[v],0.01f*epsilon))
			{
				extrude((*mTris)[j],v);
			}
		}
		// now check for those degenerate cases where we have a flipped triangle or a really skinny triangle
		j=mTris->count;
		while(j--)
		{
			if(!(*mTris)[j]) continue;
			if(!hasvert(*(*mTris)[j],v)) break;
			int3 nt=*(*mTris)[j];
			if(above(verts,nt,center,0.01f*epsilon)  || magnitude(cross(verts[nt[1]]-verts[nt[0]],verts[nt[2]]-verts[nt[1]]))< areaEpsilon)
			{
				Tri *nb = (*mTris)[(*mTris)[j]->n[0]];
				PX_ASSERT(nb);PX_ASSERT(!hasvert(*nb,v));PX_ASSERT(nb->id<j);
				extrude(nb,v);
				j=mTris->count;
			}
		}
		j=mTris->count;
		while(j--)
		{
			Tri *t=(*mTris)[j];
			if(!t) continue;
			if(t->vmax>=0) break;
			float3 n=TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
			t->vmax = maxdirsterid(verts,verts_count,n,allow);
			if(isextreme[t->vmax]) 
			{
				t->vmax=-1; // already done that vertex - algorithm needs to be able to terminate.
			}
			else
			{
				t->rise = dot(n,verts[t->vmax]-verts[(*t)[0]]);
			}
		}
		vlimit --;
	}

	if(vlimit == 0)
		return QE_VERTEX_LIMIT_REACHED;
	else
		return QE_OK;
}

// expand hull will do following steps:
//	1. get planes from triangles that form the best hull with given vertices
//  2. compute the adjacency information for the planes
//  3. expand the planes to have all vertices inside the planes volume
//  4. compute new points by 3 adjacency planes intersections
//  5. take those points and create the hull from them
HullError HullLibrary::expandhull(float3* verts, int verts_count,float3*& verts_out, int& verts_count_out, float inflate)
{
	struct LocalTests
	{
		static bool PlaneCheck(const float3* verts_, int verts_count_, Array<ExpandPlane>& planes)
		{
			for(int i=0;i<planes.count;i++)
			{
				const ExpandPlane& expandPlane = planes[i];
				if(expandPlane.mTrisIndex != -1)
				{
					for(int j=0;j<verts_count_;j++)
					{
						const float3& vertex = verts_[j];

						PX_ASSERT(expandPlane.mPlane.distance(vertex) < 0.02f);

						if(expandPlane.mPlane.distance(vertex) > 0.02f)
						{
							return false;
						}
					}
				}
			}
			return true;
		}

	};


	Array <ExpandPlane> planes;	

	// need planes and the adjacency for the triangle
	int numPoints = 0;
	for(int i=0;i<mTris->count;i++)
	{
		ExpandPlane expandPlane;
		if((*mTris)[i])
		{
			Tri *t=(*mTris)[i];			
			Plane p;
			p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
			p.dist   = -dot(p.normal, verts[(*t)[0]]);
			expandPlane.mPlane = p;

			for (int l = 0; l < 3; l++)
			{
				if(t->n[l] > numPoints)
				{
					numPoints = t->n[l];
				}
			}

			for(int j=0;j<mTris->count;j++)
			{
				if((*mTris)[j] && i != j)
				{
					Tri *testTris=(*mTris)[j];

					int numId0 = 0;
					int numId1 = 0;
					int numId2 = 0;

					for (int k = 0; k < 3; k++)
					{
						int testI = (*testTris)[k];
						if(testI == (*t)[0] || testI == (*t)[1])
						{
							numId0++;
						}
						if(testI == (*t)[0] || testI == (*t)[2])
						{
							numId1++;
						}
						if(testI == (*t)[2] || testI == (*t)[1])
						{
							numId2++;
						}
					}

					if(numId0 == 2)
					{
						PX_ASSERT(expandPlane.mAdjacency[0] == -1);
						expandPlane.mAdjacency[0] = j;
					}
					if(numId1 == 2)
					{
						PX_ASSERT(expandPlane.mAdjacency[1] == -1);
						expandPlane.mAdjacency[1] = j;
					}
					if(numId2 == 2)
					{
						PX_ASSERT(expandPlane.mAdjacency[2] == -1);
						expandPlane.mAdjacency[2] = j;
					}
				}
			}
			
			expandPlane.mTrisIndex = i;			
		}
		planes.Add(expandPlane);
	}
	numPoints++;

	// go over the planes now and expand them	
	for(int i=0;i<verts_count;i++)
	{
		const float3& vertex = verts[i];

		for(int j=0;j<mTris->count;j++)
		{
			ExpandPlane& expandPlane = planes[j];
			if(expandPlane.mTrisIndex != -1)
			{
				float dist = expandPlane.mPlane.distance(vertex);
				if(dist > 0 && dist > expandPlane.mExpandDistance)
				{
					expandPlane.mExpandDistance = dist;
					expandPlane.mExpandPoint = i;
				}
			}
		}	
	}

	// expand the planes
	for(int i=0;i<planes.count;i++)
	{
		ExpandPlane& expandPlane = planes[i];
		if(expandPlane.mTrisIndex != -1)
		{
			if(expandPlane.mExpandPoint >= 0)
				expandPlane.mPlane.dist -= expandPlane.mExpandDistance + inflate;
		}
	}

	PX_ASSERT(LocalTests::PlaneCheck(verts,verts_count,planes));

	Array <int>	translateTable;
	Array <float3> points;
	numPoints = 0;	

	// find new triangle points and store them
	for(int i=0;i<planes.count;i++)
	{
		ExpandPlane& expandPlane = planes[i];		
		if(expandPlane.mTrisIndex != -1)
		{
			Tri *expandTri=(*mTris)[expandPlane.mTrisIndex];

			for (int j = 0; j < 3; j++)
			{
				ExpandPlane& plane1 = planes[expandPlane.mAdjacency[j]];			
				ExpandPlane& plane2 = planes[expandPlane.mAdjacency[(j + 1)%3]];	
				Tri *tri1=(*mTris)[expandPlane.mAdjacency[j]];
				Tri *tri2=(*mTris)[expandPlane.mAdjacency[(j + 1)%3]];

				int indexE = -1;
				int index1 = -1;
				int index2 = -1;
				for (int l = 0; l < 3; l++)
				{
					for (int k = 0; k < 3; k++)
					{
						for (int m = 0; m < 3; m++)
						{
							if((*expandTri)[l] == (*tri1)[k] && (*expandTri)[l] == (*tri2)[m])
							{
								indexE = l;
								index1 = k;
								index2 = m;
							}
						}
					}				
				}

				PX_ASSERT(indexE != -1);

				int foundIndex = -1;
				for (int u = 0; u < translateTable.count; u++)
				{
					if(translateTable[u] == (*expandTri)[indexE])
					{
						foundIndex = u;
						break;
					}
				}

				float3 point = ThreePlaneIntersection(expandPlane.mPlane, plane1.mPlane, plane2.mPlane);

				if(foundIndex == -1)
				{					
					expandPlane.mIndices[indexE] = numPoints;
					plane1.mIndices[index1] = numPoints;
					plane2.mIndices[index2] = numPoints;					

					points.Add(point);
					translateTable.Add((*expandTri)[indexE]);
					numPoints++;
				}
				else
				{
					if(expandPlane.mPlane.distance(points[foundIndex]) < -0.02f || plane1.mPlane.distance(points[foundIndex]) < -0.02f || plane2.mPlane.distance(points[foundIndex]) < -0.02f)
					{
						points[foundIndex] = point;
					}

					expandPlane.mIndices[indexE] = foundIndex;
					plane1.mIndices[index1] = foundIndex;
					plane2.mIndices[index2] = foundIndex;
				}

			}
		}
	}	

	verts_count_out = numPoints;
	verts_out = points.element;
	points.element = NULL;
	points.count = 0;
	points.array_size = 0;

	for(int i=0;i<mTris->count;i++)
	{
		if((*mTris)[i])
		{
			deleteTri((*mTris)[i], mTris);
		}
	}
	// please reset here, otherwise, we get a nice virtual function call (R6025) error with PxCooking library
	mTris->SetSize( 0 );

	return QE_OK;
}

HullError HullLibrary::calchull(float3* verts, int verts_count, int*& tris_out, int& tris_count,float3*& verts_out, int& verts_count_out, int vlimit, float areaEpsilon)
{
	HullError rc=calchullgen(verts,verts_count,  vlimit, areaEpsilon) ;
	if ((rc == QE_FAIL) || (rc == QE_AREA_TEST_FAIL))
		return rc;

	// if vertex limit reached expand the result to overlap all verts
	if(rc == QE_VERTEX_LIMIT_REACHED)
	{
		expandhull(verts,verts_count,verts_out, verts_count_out);

		rc=calchullgen(verts_out,verts_count_out, 256, areaEpsilon) ;
		if ((rc == QE_FAIL) || (rc == QE_AREA_TEST_FAIL))
			return rc;

		Array<int> ts;
		for(int i=0;i<mTris->count;i++)if((*mTris)[i])
		{
			for(int j=0;j<3;j++)ts.Add((*(*mTris)[i])[j]);
			//			delete (*mTris)[i];
			deleteTri((*mTris)[i], mTris);
		}
		tris_count = ts.count/3;
		tris_out   = ts.element;
		ts.element=NULL; ts.count=ts.array_size=0;
		// please reset here, otherwise, we get a nice virtual function call (R6025) error with PxCooking library
		mTris->SetSize( 0 );

		return QE_VERTEX_LIMIT_REACHED;
	}

	Array<int> ts;
	for(int i=0;i<mTris->count;i++)if((*mTris)[i])
	{
		for(int j=0;j<3;j++)ts.Add((*(*mTris)[i])[j]);
//			delete (*mTris)[i];
		deleteTri((*mTris)[i], mTris);
	}
	tris_count = ts.count/3;
	tris_out   = ts.element;
	ts.element=NULL; ts.count=ts.array_size=0;
	// please reset here, otherwise, we get a nice virtual function call (R6025) error with PxCooking library
	mTris->SetSize( 0 );
	return QE_OK;
}

static float area2(const float3 &v0,const float3 &v1,const float3 &v2)
{
	float3 cp = cross(v0-v1,v2-v0);
	return dot(cp,cp);
}

HullError HullLibrary::calchullpbev(float3* verts, int verts_count, int /*vlimit*/, Array<Plane>& planes, float bevangle)
{
	int i,j;
	Array<Plane> bplanes;
	planes.count=0;
//	extern float minadjangle; // default is 3.0f;  // in degrees  - result wont have two adjacent facets within this angle of each other.
	float maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(i=0;i<mTris->count;i++)if((*mTris)[i])
	{
		Plane p;
		Tri *t = (*mTris)[i];
		p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		p.dist   = -dot(p.normal, verts[(*t)[0]]);
		for(j=0;j<3;j++)
		{
			if(t->n[j]<t->id) continue;
			Tri *s = (*mTris)[t->n[j]];
			REAL3 snormal = TriNormal(verts[(*s)[0]],verts[(*s)[1]],verts[(*s)[2]]);
			if(dot(snormal,p.normal)>=cos(bevangle*DEG2RAD)) continue;
			REAL3 e = verts[(*t)[(j+2)%3]] - verts[(*t)[(j+1)%3]];
			REAL3 n = (e!=REAL3(0,0,0))? cross(snormal,e)+cross(e,p.normal) : snormal+p.normal;
			PX_ASSERT(n!=REAL3(0,0,0));
			if(n==REAL3(0,0,0)) return QE_FAIL;  
			n=normalize(n);
			bplanes.Add(Plane(n,-dot(n,verts[maxdir(verts,verts_count,n)])));
		}
	}
	for(i=0;i<mTris->count;i++)if((*mTris)[i])for(j=i+1;j<mTris->count;j++)if((*mTris)[i] && (*mTris)[j])
	{
		Tri *ti = (*mTris)[i];
		Tri *tj = (*mTris)[j];
		REAL3 ni = TriNormal(verts[(*ti)[0]],verts[(*ti)[1]],verts[(*ti)[2]]);
		REAL3 nj = TriNormal(verts[(*tj)[0]],verts[(*tj)[1]],verts[(*tj)[2]]);
		if(dot(ni,nj)>maxdot_minang)
		{
			// somebody has to die, keep the biggest triangle
			if( area2(verts[(*ti)[0]],verts[(*ti)[1]],verts[(*ti)[2]]) < area2(verts[(*tj)[0]],verts[(*tj)[1]],verts[(*tj)[2]]))
			{
//				delete (*mTris)[i]; (*mTris)[i]=NULL;
				deleteTri((*mTris)[i], mTris);
			}
			else
			{
//				delete (*mTris)[j]; (*mTris)[j]=NULL;
				deleteTri((*mTris)[j], mTris);
			}
		}
	}
	for(i=0;i<mTris->count;i++)if((*mTris)[i])
	{
		Plane p;
		Tri *t = (*mTris)[i];
		p.normal = TriNormal(verts[(*t)[0]],verts[(*t)[1]],verts[(*t)[2]]);
		p.dist   = -dot(p.normal, verts[(*t)[0]]);
		planes.Add(p);
	}
	for(i=0;i<bplanes.count;i++)
	{
		for(j=0;j<planes.count;j++)
		{
			if(dot(bplanes[i].normal,planes[j].normal)>maxdot_minang) break;
		}
		if(j==planes.count)
		{
			planes.Add(bplanes[i]);
		}
	}
	for(i=0;i<mTris->count;i++)if((*mTris)[i])
	{
//		delete (*mTris)[i];
		deleteTri((*mTris)[i], mTris);
	}
	mTris->count = 0; //bad place to do the tris.SetSize(0) fix, this line is executed many times, and will result in a whole lot of allocations if the array is totally cleared here
	return QE_OK;
}

static HullError overhull(Plane* planes, int planes_count, float3* verts, int verts_count, int maxplanes,
			 float3*& verts_out, int& verts_count_out, int*& faces_out, int& faces_count_out, float inflate)
{
	int i,j;
	if(verts_count <4) return QE_FAIL;
	maxplanes = Min(maxplanes,planes_count);
	float3 bmin(verts[0]),bmax(verts[0]);
	for(i=0;i<verts_count;i++) 
	{
		bmin = VectorMin(bmin,verts[i]);
		bmax = VectorMax(bmax,verts[i]);
	}
	float diameter = magnitude(bmax-bmin);
//	inflate *=diameter;   // RELATIVE INFLATION
	//bmin -= float3(inflate*2.5f,inflate*2.5f,inflate*2.5f); 
	//bmax += float3(inflate*2.5f,inflate*2.5f,inflate*2.5f); 
	// 2 is from the formula:
	// D = d*|n1+n2|/(1-n1 dot n2), where d is "inflate" and
	// n1 and n2 are the normals of two planes at bevelAngle to each other
	// for 120 degrees, D is 2d

	//bmin -= float3(inflate,inflate,inflate);
	//bmax += float3(inflate,inflate,inflate);

	for(i=0;i<planes_count;i++)
	{
		planes[i].dist -= inflate;
	}
	float3 emin = bmin; // VectorMin(bmin,float3(0,0,0));
	float3 emax = bmax; // VectorMax(bmax,float3(0,0,0));
	float epsilon  = 0.01f; // size of object is taken into account within candidate plane function.  Used to multiply here by magnitude(emax-emin) 
	planetestepsilon = magnitude(emax-emin) * PAPERWIDTH;
	// todo: add bounding cube planes to force bevel. or try instead not adding the diameter expansion ??? must think.
	// ConvexH *convex = ConvexHMakeCube(bmin - float3(diameter,diameter,diameter),bmax+float3(diameter,diameter,diameter));
	float maxdot_minang = cosf(DEG2RAD*minadjangle);
	for(j=0;j<6;j++)
	{
		float3 n(0,0,0);
		n[j/2] = (j%2)? 1.0f : -1.0f;
		for(i=0;i<planes_count;i++)
		{
			if(dot(n,planes[i].normal)> maxdot_minang)
			{
				(*((j%2)?&bmax:&bmin)) += n * (diameter*0.5f);
				break;
			}
		}
	}
	ConvexH *c = ConvexHMakeCube(REAL3(bmin),REAL3(bmax)); 
	int k;
	while(maxplanes-- && (k=candidateplane(planes,planes_count,c,epsilon))>=0)
	{
		ConvexH *tmp = c;
		c = ConvexHCrop(*tmp,planes[k]);
		if(c==NULL) {c=tmp; break;} // might want to debug this case better!!!
		if(!AssertIntact(*c)) {delete c; c=tmp; break;} // might want to debug this case better too!!!
		delete tmp;
	}

	PX_ASSERT(AssertIntact(*c));
	//return c;
	faces_out = (int*)PX_ALLOC_TEMP(sizeof(int)*(1+c->facets.count+c->edges.count), PX_DEBUG_EXP("int"));     // new int[1+c->facets.count+c->edges.count];
	faces_count_out=0;
	i=0;
	faces_out[faces_count_out++]=-1;
	k=0;
	while(i<c->edges.count)
	{
		j=1;
		while(j+i<c->edges.count && c->edges[i].p==c->edges[i+j].p) { j++; }
		faces_out[faces_count_out++]=j;
		while(j--)
		{
			faces_out[faces_count_out++] = c->edges[i].v;
			i++;
		}
		k++;
	}
	faces_out[0]=k; // number of faces.
	PX_ASSERT(k==c->facets.count);
	PX_ASSERT(faces_count_out == 1+c->facets.count+c->edges.count);
	verts_out = c->vertices.element; // new float3[c->vertices.count];
	verts_count_out = c->vertices.count;
	for(i=0;i<c->vertices.count;i++)
	{
		verts_out[i] = float3(c->vertices[i]);
	}
	c->vertices.count=c->vertices.array_size=0;	c->vertices.element=NULL;
	delete c;
	return QE_OK;
}

HullError HullLibrary::overhullv(float3* verts, int verts_count, int maxplanes,
			 float3*& verts_out, int& verts_count_out, int*& faces_out, int& faces_count_out, float inflate, float bevangle, int vlimit, float areaEpsilon)
{
	if(!verts_count) return QE_FAIL;
//	extern int calchullpbev(float3 *verts,int verts_count,int vlimit, Array<Plane> &planes,float bevangle) ;
	Array<Plane> planes;
	HullError rc = calchullgen(verts,verts_count,vlimit, areaEpsilon);
	if ((rc == QE_FAIL) || (rc == QE_AREA_TEST_FAIL))
		return rc;

	// expand the hull instead of inflate code
	if(rc == QE_VERTEX_LIMIT_REACHED)
	{
		expandhull(verts,verts_count,verts_out, verts_count_out, inflate);

		rc=calchullgen(verts_out,verts_count_out, 256, areaEpsilon);
		if ((rc == QE_FAIL) || (rc == QE_AREA_TEST_FAIL))
			return rc;

		Array<int> ts;
		for(int i=0;i<mTris->count;i++)if((*mTris)[i])
		{
			for(int j=0;j<3;j++)ts.Add((*(*mTris)[i])[j]);
			//			delete (*mTris)[i];
			deleteTri((*mTris)[i], mTris);
		}
		faces_count_out = ts.count/3;
		faces_out   = ts.element;
		ts.element=NULL; ts.count=ts.array_size=0;
		// please reset here, otherwise, we get a nice virtual function call (R6025) error with PxCooking library
		mTris->SetSize( 0 );

		return QE_VERTEX_LIMIT_REACHED;
	}
	else
	{
		HullError rc1=calchullpbev(verts,verts_count,vlimit,planes,bevangle) ;
		if(rc1 == QE_FAIL) return QE_FAIL;
		return overhull(planes.element,planes.count,verts,verts_count,maxplanes,verts_out,verts_count_out,faces_out,faces_count_out,inflate);
	}
}

//*****************************************************
//*****************************************************

HullError HullLibrary::ComputeHull(PxU32 vcount, const float* vertices, PHullResult& result, PxU32 vlimit, float inflate, float areaEpsilon)
{

	int index_count;
	int* faces;
	float3* verts_out;
	int verts_count_out;

	if(inflate==0.0f)
	{
		int  *tris_out;
		int    tris_count;
		HullError ret = calchull( (float3 *) vertices, (int) vcount, tris_out, tris_count, verts_out,verts_count_out, (int)vlimit, areaEpsilon);
		if ((ret == QE_FAIL) || (ret == QE_AREA_TEST_FAIL))
			return ret;

		result.mIndexCount = (PxU32) (tris_count*3);
		result.mFaceCount  = (PxU32) tris_count;			
		result.mIndices    = (PxU32 *) tris_out;
		if(ret == QE_VERTEX_LIMIT_REACHED)
		{
			result.mVertices   = (float*) verts_out;
			result.mVcount     = (PxU32) verts_count_out;
		}
		if(ret == QE_OK)
		{
			result.mVertices   = (float*) vertices;
			result.mVcount     = (PxU32) vcount;
		}
		return QE_OK;
	}

	HullError ret = overhullv((float3*)vertices,(int)vcount,35,verts_out,verts_count_out,faces,index_count,inflate,120.0f,(int)vlimit, areaEpsilon);
	if ((ret == QE_FAIL) || (ret == QE_AREA_TEST_FAIL))
	{
		mTris->SetSize(0); //have to set the size to 0 in order to protect from a "pure virtual function call" problem
		return ret;
	}

	result.mVertices   = (float*) verts_out;
	result.mVcount     = (PxU32) verts_count_out;		

	if(ret == QE_VERTEX_LIMIT_REACHED)
	{
		result.mIndexCount = (PxU32) (index_count*3);
		result.mFaceCount  = (PxU32) index_count;			
		result.mIndices    = (PxU32 *) faces;
	}
	else
	{
		Array<int3> tris;
		int n=faces[0];
		int k=1;
		for(int i=0;i<n;i++)
		{
			int pn = faces[k++];
			for(int j=2;j<pn;j++) tris.Add(int3(faces[k],faces[k+j-1],faces[k+j]));
			k+=pn;
		}
		PX_ASSERT(tris.count == index_count-1-(n*3));
		PX_FREE_AND_RESET(faces);	// PT: I added that. Is it ok ?

		result.mIndexCount = (PxU32) (tris.count*3);
		result.mFaceCount  = (PxU32) tris.count;
		result.mIndices    = (PxU32 *) tris.element;

		tris.element=NULL; tris.count = tris.array_size=0;
		tris.SetSize(0); //have to set the size to 0 in order to protect from a "pure virtual function call" problem
	}

	return QE_OK;
}

static void ReleaseHull(PHullResult &result)
{
PX_FREE_AND_RESET(result.mIndices);	// PT: I added that. Is it ok ?
PX_FREE_AND_RESET(result.mVertices);	// PT: I added that. Is it ok ?
	result.mVcount = 0;
	result.mIndexCount = 0;
	result.mIndices = 0;
	result.mVertices = 0;
	result.mIndices  = 0;
}

//****** HULLLIB source code

static void BringOutYourDead(const float* verts, PxU32 vcount, float* overts, PxU32& ocount, PxU32* indices, unsigned indexcount)
{
	PxU32* used = PX_NEW_TEMP(PxU32)[vcount];
	memset(used,0,sizeof(PxU32)*vcount);

	ocount = 0;

	for (PxU32 i=0; i<indexcount; i++)
	{
		PxU32 v = indices[i]; // original array index

		PX_ASSERT( v < vcount );

		if ( used[v] ) // if already remapped
		{
			indices[i] = used[v]-1; // index to new array
		}
		else
		{
			indices[i] = ocount;      // new index mapping

			overts[ocount*3+0] = verts[v*3+0]; // copy old vert to new vert array
			overts[ocount*3+1] = verts[v*3+1];
			overts[ocount*3+2] = verts[v*3+2];

			ocount++; // increment output vert count

			PX_ASSERT( ocount <= vcount );

			used[v] = ocount; // assign new index remapping
		}
	}

	PX_DELETE_POD(used);
}

static void AddPoint(PxU32& vcount, float* p, float x, float y, float z)
{
	float* dest = &p[vcount*3];
	dest[0] = x;
	dest[1] = y;
	dest[2] = z;
	vcount++;
}

static float GetDist(float px, float py, float pz, const float* p2)
{
	const float dx = px - p2[0];
	const float dy = py - p2[1];
	const float dz = pz - p2[2];
	return dx*dx+dy*dy+dz*dz;
}

static bool CleanupVertices(	PxU32 svcount,
								const float* svertices,
								PxU32 stride,
								PxU32& vcount,		// output number of vertices
								float* vertices,	// location to store the results.
								float normalepsilon,
								float* scale)
{
	if ( svcount == 0 ) return false;

	#define EPSILON 0.000001f // close enough to consider two floating point numbers to be 'the same'.

	vcount = 0;

	float recip[3];

	if ( scale )
	{
		scale[0] = 1.f;
		scale[1] = 1.f;
		scale[2] = 1.f;
	}
		
	const char* vtx = (const char *) svertices;
	float center[3];
	{
		float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
		float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };


		if ( 1 )
		{
			for (PxU32 i=0; i<svcount; i++)
			{
				const float *p = (const float *) vtx;

				vtx+=stride;

				for (int j=0; j<3; j++)
				{
					if ( p[j] < bmin[j] ) bmin[j] = p[j];
					if ( p[j] > bmax[j] ) bmax[j] = p[j];
				}
			}
		}
	
		float dx = bmax[0] - bmin[0];
		float dy = bmax[1] - bmin[1];
		float dz = bmax[2] - bmin[2];

		center[0] = dx*0.5f + bmin[0];
		center[1] = dy*0.5f + bmin[1];
		center[2] = dz*0.5f + bmin[2];

		if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || svcount < 3 )
		{
			float len = FLT_MAX;

			if ( dx > EPSILON && dx < len ) len = dx;
			if ( dy > EPSILON && dy < len ) len = dy;
			if ( dz > EPSILON && dz < len ) len = dz;

			if ( len == FLT_MAX )
			{
				dx = dy = dz = 0.01f; // one centimeter
			}
			else
			{
				if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
				if ( dy < EPSILON ) dy = len * 0.05f;
				if ( dz < EPSILON ) dz = len * 0.05f;
			}

			float x1 = center[0] - dx;
			float x2 = center[0] + dx;

			float y1 = center[1] - dy;
			float y2 = center[1] + dy;

			float z1 = center[2] - dz;
			float z2 = center[2] + dz;

			AddPoint(vcount,vertices,x1,y1,z1);
			AddPoint(vcount,vertices,x2,y1,z1);
			AddPoint(vcount,vertices,x2,y2,z1);
			AddPoint(vcount,vertices,x1,y2,z1);
			AddPoint(vcount,vertices,x1,y1,z2);
			AddPoint(vcount,vertices,x2,y1,z2);
			AddPoint(vcount,vertices,x2,y2,z2);
			AddPoint(vcount,vertices,x1,y2,z2);

			return true; // return cube
		}
		else
		{
			if ( scale )
			{
				scale[0] = dx;
				scale[1] = dy;
				scale[2] = dz;

				recip[0] = 1 / dx;
				recip[1] = 1 / dy;
				recip[2] = 1 / dz;

				center[0]*=recip[0];
				center[1]*=recip[1];
				center[2]*=recip[2];
			}
		}
	}

	vtx = (const char *) svertices;

	for (PxU32 i=0; i<svcount; i++)
	{
		const float* p = (const float *)vtx;
		vtx+=stride;

		float px = p[0];
		float py = p[1];
		float pz = p[2];

		if ( scale )
		{
			px = px*recip[0]; // normalize
			py = py*recip[1]; // normalize
			pz = pz*recip[2]; // normalize
		}

		if ( 1 )
		{
			PxU32 j;

			for (j=0; j<vcount; j++)
			{
				float* v = &vertices[j*3];

				const float x = v[0];
				const float y = v[1];
				const float z = v[2];

				const float dx = fabsf(x - px );
				const float dy = fabsf(y - py );
				const float dz = fabsf(z - pz );

				if ( dx < normalepsilon && dy < normalepsilon && dz < normalepsilon )
				{
					// ok, it is close enough to the old one
					// now let us see if it is further from the center of the point cloud than the one we already recorded.
					// in which case we keep this one instead.

					const float dist1 = GetDist(px,py,pz,center);
					const float dist2 = GetDist(v[0],v[1],v[2],center);

					if ( dist1 > dist2 )
					{
						v[0] = px;
						v[1] = py;
						v[2] = pz;
					}

					break;
				}
			}

			if ( j == vcount )
			{
				float *dest = &vertices[vcount*3];
				dest[0] = px;
				dest[1] = py;
				dest[2] = pz;
				vcount++;
			}
		}
	}

	// ok..now make sure we didn't prune so many vertices it is now invalid.
	if ( 1 )
	{
		float bmin[3] = {  FLT_MAX,  FLT_MAX,  FLT_MAX };
		float bmax[3] = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		for (PxU32 i=0; i<vcount; i++)
		{
			const float *p = &vertices[i*3];
			for (int j=0; j<3; j++)
			{
				if ( p[j] < bmin[j] ) bmin[j] = p[j];
				if ( p[j] > bmax[j] ) bmax[j] = p[j];
			}
		}

		float dx = bmax[0] - bmin[0];
		float dy = bmax[1] - bmin[1];
		float dz = bmax[2] - bmin[2];

		if ( dx < EPSILON || dy < EPSILON || dz < EPSILON || vcount < 3)
		{
			float cx = dx*0.5f + bmin[0];
			float cy = dy*0.5f + bmin[1];
			float cz = dz*0.5f + bmin[2];

			float len = FLT_MAX;

			if ( dx >= EPSILON && dx < len ) len = dx;
			if ( dy >= EPSILON && dy < len ) len = dy;
			if ( dz >= EPSILON && dz < len ) len = dz;

			if ( len == FLT_MAX )
			{
				dx = dy = dz = 0.01f; // one centimeter
			}
			else
			{
				if ( dx < EPSILON ) dx = len * 0.05f; // 1/5th the shortest non-zero edge.
				if ( dy < EPSILON ) dy = len * 0.05f;
				if ( dz < EPSILON ) dz = len * 0.05f;
			}

			float x1 = cx - dx;
			float x2 = cx + dx;

			float y1 = cy - dy;
			float y2 = cy + dy;

			float z1 = cz - dz;
			float z2 = cz + dz;

			vcount = 0; // add box

			AddPoint(vcount,vertices,x1,y1,z1);
			AddPoint(vcount,vertices,x2,y1,z1);
			AddPoint(vcount,vertices,x2,y2,z1);
			AddPoint(vcount,vertices,x1,y2,z1);
			AddPoint(vcount,vertices,x1,y1,z2);
			AddPoint(vcount,vertices,x2,y1,z2);
			AddPoint(vcount,vertices,x2,y2,z2);
			AddPoint(vcount,vertices,x1,y2,z2);

			return true;
		}
	}

	return true;
}

HullError HullLibrary::CreateConvexHull(const HullDesc& desc,	// describes the input request
										HullResult& result)		// contains the results
{
	HullError ret = QE_FAIL;

	PHullResult hr;

	PxU32 vcount = desc.mVcount;
	if ( vcount < 8 ) vcount = 8;

	float* vsource  = (float*)PX_ALLOC_TEMP( sizeof(float)*vcount*3, PX_DEBUG_EXP("float"));

	float scale[3];

	PxU32 ovcount;

	bool ok = CleanupVertices(desc.mVcount,desc.mVertices, desc.mVertexStride, ovcount, vsource, desc.mNormalEpsilon, scale ); // normalize point cloud, remove duplicates!

	if(ok)
	{
		if(1) // scale vertices back to their original size.
		{
			for (PxU32 i=0; i<ovcount; i++)
			{
				float* v = &vsource[i*3];
				v[0]*=scale[0];
				v[1]*=scale[1];
				v[2]*=scale[2];
			}
		}

		float skinwidth = 0;
		if ( desc.HasHullFlag(QF_SKIN_WIDTH) ) 
			skinwidth = desc.mSkinWidth;
		float areaEpsilon = desc.mAreaTestEpsilon;

		ret = ComputeHull(ovcount,vsource,hr,desc.mMaxVertices,skinwidth, areaEpsilon);

		if(ret == QE_OK)
		{
			// re-index triangle mesh so it refers to only used vertices, rebuild a new vertex table.
			float* vscratch = (float*)PX_ALLOC_TEMP(sizeof(float)*hr.mVcount*3, PX_DEBUG_EXP("float"));
			BringOutYourDead(hr.mVertices,hr.mVcount, vscratch, ovcount, hr.mIndices, hr.mIndexCount );

			ret = QE_OK;

			if(desc.HasHullFlag(QF_TRIANGLES)) // if he wants the results as triangle!
			{
				result.mPolygons          = false;
				result.mNumOutputVertices = ovcount;
				result.mOutputVertices    = PX_NEW_TEMP(float)[ovcount*3];
				result.mNumFaces          = hr.mFaceCount;
				result.mNumIndices        = hr.mIndexCount;

				result.mIndices           = PX_NEW_TEMP(PxU32)[hr.mIndexCount];

				PxMemCopy(result.mOutputVertices, vscratch, sizeof(float)*3*ovcount );

  				if(desc.HasHullFlag(QF_REVERSE_ORDER))
				{
					const PxU32* source = hr.mIndices;
					PxU32* dest = result.mIndices;

					for(PxU32 i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = source[2];
						dest[1] = source[1];
						dest[2] = source[0];
						dest+=3;
						source+=3;
					}
				}
				else
				{
					PxMemCopy(result.mIndices, hr.mIndices, sizeof(PxU32)*hr.mIndexCount);
				}
			}
			else
			{
				result.mPolygons			= true;
				result.mNumOutputVertices	= ovcount;
				result.mOutputVertices		= PX_NEW_TEMP(float)[ovcount*3];
				result.mNumFaces			= hr.mFaceCount;
				result.mNumIndices			= hr.mIndexCount+hr.mFaceCount;
				result.mIndices				= PX_NEW_TEMP(PxU32)[result.mNumIndices];

				PxMemCopy(result.mOutputVertices, vscratch, sizeof(float)*3*ovcount );

				if(1)
				{
					const PxU32* source = hr.mIndices;
					PxU32* dest = result.mIndices;
					for(PxU32 i=0; i<hr.mFaceCount; i++)
					{
						dest[0] = 3;
						if ( desc.HasHullFlag(QF_REVERSE_ORDER) )
						{
							dest[1] = source[2];
							dest[2] = source[1];
							dest[3] = source[0];
						}
						else
						{
							dest[1] = source[0];
							dest[2] = source[1];
							dest[3] = source[2];
						}

						dest+=4;
						source+=3;
					}
				}
			}
			// ReleaseHull frees memory for hr.mVertices, which can be the
			// same pointer as vsource, so be sure to set it to NULL if necessary
			if ( hr.mVertices == vsource) vsource = NULL;

			ReleaseHull(hr);

			if(vscratch)
			{
				PX_DELETE_POD(vscratch);
			}
		}
	}

	// this pointer is usually freed in ReleaseHull()
	if(vsource)
	{
		PX_DELETE_POD(vsource);
	}
	return ret;
}

HullError HullLibrary::ReleaseResult(HullResult& result) // release memory allocated for this result, we are done with it.
{
	if(result.mOutputVertices)
	{
		PX_DELETE_POD(result.mOutputVertices);
		result.mOutputVertices = 0;
	}
	if(result.mIndices)
	{
		PX_DELETE_POD(result.mIndices);
		result.mIndices = 0;
	}
	return QE_OK;
}

//==================================================================================

static float ComputeNormal(float* n, const float* A, const float* B, const float* C)
{
	const float vx = (B[0] - C[0]);
	const float vy = (B[1] - C[1]);
	const float vz = (B[2] - C[2]);

	const float wx = (A[0] - B[0]);
	const float wy = (A[1] - B[1]);
	const float wz = (A[2] - B[2]);

	const float vw_x = vy * wz - vz * wy;
	const float vw_y = vz * wx - vx * wz;
	const float vw_z = vx * wy - vy * wx;

	float mag = sqrtf((vw_x * vw_x) + (vw_y * vw_y) + (vw_z * vw_z));

	if(mag < 0.000001f)
	{
		mag = 0.0f;
	}
	else
	{
		mag = 1.0f/mag;
	}

	n[0] = vw_x * mag;
	n[1] = vw_y * mag;
	n[2] = vw_z * mag;
	return mag;
}

static void AddConvexTriangle(ConvexHullTriangleInterface* callback, const float* p1, const float* p2, const float* p3)
{
	ConvexHullVertex v1,v2,v3;

	#define TSCALE1 (1.0f/4.0f)

	v1.mPos[0] = p1[0];
	v1.mPos[1] = p1[1];
	v1.mPos[2] = p1[2];

	v2.mPos[0] = p2[0];
	v2.mPos[1] = p2[1];
	v2.mPos[2] = p2[2];

	v3.mPos[0] = p3[0];
	v3.mPos[1] = p3[1];
	v3.mPos[2] = p3[2];

	float n[3];
	ComputeNormal(n,p1,p2,p3);

	v1.mNormal[0] = n[0];
	v1.mNormal[1] = n[1];
	v1.mNormal[2] = n[2];

	v2.mNormal[0] = n[0];
	v2.mNormal[1] = n[1];
	v2.mNormal[2] = n[2];

	v3.mNormal[0] = n[0];
	v3.mNormal[1] = n[1];
	v3.mNormal[2] = n[2];

	const float *tp1 = p1;
	const float *tp2 = p2;
	const float *tp3 = p3;

	int i1 = 0;
	int i2 = 0;

	float nx = fabsf(n[0]);
	float ny = fabsf(n[1]);
	float nz = fabsf(n[2]);

	if ( nx <= ny && nx <= nz )
		i1 = 0;
	if ( ny <= nx && ny <= nz )
		i1 = 1;
	if ( nz <= nx && nz <= ny )
		i1 = 2;

	switch ( i1 )
	{
		case 0:
			if ( ny < nz )
				i2 = 1;
			else
				i2 = 2;
			break;
		case 1:
			if ( nx < nz )
				i2 = 0;
			else
				i2 = 2;
			break;
		case 2:
			if ( nx < ny )
				i2 = 0;
			else
				i2 = 1;
			break;
	}

	v1.mTexel[0] = tp1[i1]*TSCALE1;
	v1.mTexel[1] = tp1[i2]*TSCALE1;

	v2.mTexel[0] = tp2[i1]*TSCALE1;
	v2.mTexel[1] = tp2[i2]*TSCALE1;

	v3.mTexel[0] = tp3[i1]*TSCALE1;
	v3.mTexel[1] = tp3[i2]*TSCALE1;

	callback->ConvexHullTriangle(v3,v2,v1);
}

HullError HullLibrary::CreateTriangleMesh(HullResult& answer, ConvexHullTriangleInterface* iface)
{
	HullError ret = QE_FAIL;

	const float* p = answer.mOutputVertices;
	const PxU32* idx = answer.mIndices;
	PxU32 fcount = answer.mNumFaces;

	if ( p && idx && fcount )
	{
		ret = QE_OK;

		for (PxU32 i=0; i<fcount; i++)
		{
			PxU32 pcount = *idx++;

			PxU32 i1 = *idx++;
			PxU32 i2 = *idx++;
			PxU32 i3 = *idx++;

			const float *p1 = &p[i1*3];
			const float *p2 = &p[i2*3];
			const float *p3 = &p[i3*3];

			AddConvexTriangle(iface,p1,p2,p3);

			pcount-=3;
			while ( pcount )
			{
				i3 = *idx++;
				p2 = p3;
				p3 = &p[i3*3];

				AddConvexTriangle(iface,p1,p2,p3);
				pcount--;
			}

		}
	}

	return ret;
}

#ifdef HULL_NAMESPACE
};
#endif

}
