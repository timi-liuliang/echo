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


//#ifdef PX_COOKING

/*
*	This code computes volume integrals needed to compute mass properties of polyhedral bodies.
*	Based on public domain code by Brian Mirtich.
*/
#include "VolumeIntegration.h"
#include "PxSimpleTriangleMesh.h"
#include "PxMemory.h"
#include "PsUserAllocated.h"
#include "PsMathUtils.h"

namespace physx
{

namespace
{

	class VolumeIntegrator
	{
		public:
			VolumeIntegrator(PxSimpleTriangleMesh mesh, PxF64	mDensity);
			~VolumeIntegrator();
			bool	computeVolumeIntegrals(PxIntegrals& ir);
		private:
			struct Normal
				{
				PxVec3 normal;
				PxF32 w;
				};
			
			struct Face
				{
				PxF64 Norm[3];
				PxF64	w;
				PxU32	Verts[3];
				};
			
			// Data structures
			PxF64				mMass;					//!< Mass
			PxF64				mDensity;				//!< Density
			PxSimpleTriangleMesh mesh;	
			//Normal	*			faceNormals;			//!< temp face normal data structure
			
			
			
			
			unsigned int		mA;						//!< Alpha
			unsigned int		mB;						//!< Beta
			unsigned int		mC;						//!< Gamma
			
			// Projection integrals
			PxF64				mP1;
			PxF64				mPa;					//!< Pi Alpha
			PxF64				mPb;					//!< Pi Beta
			PxF64				mPaa;					//!< Pi Alpha^2
			PxF64				mPab;					//!< Pi AlphaBeta
			PxF64				mPbb;					//!< Pi Beta^2
			PxF64				mPaaa;					//!< Pi Alpha^3
			PxF64				mPaab;					//!< Pi Alpha^2Beta
			PxF64				mPabb;					//!< Pi AlphaBeta^2
			PxF64				mPbbb;					//!< Pi Beta^3
			
			// Face integrals
			PxF64				mFa;					//!< FAlpha
			PxF64				mFb;					//!< FBeta
			PxF64				mFc;					//!< FGamma
			PxF64				mFaa;					//!< FAlpha^2
			PxF64				mFbb;					//!< FBeta^2
			PxF64				mFcc;					//!< FGamma^2
			PxF64				mFaaa;					//!< FAlpha^3
			PxF64				mFbbb;					//!< FBeta^3
			PxF64				mFccc;					//!< FGamma^3
			PxF64				mFaab;					//!< FAlpha^2Beta
			PxF64				mFbbc;					//!< FBeta^2Gamma
			PxF64				mFcca;					//!< FGamma^2Alpha
			
			// The 10 volume integrals
			PxF64				mT0;					//!< ~Total mass
			PxF64				mT1[3];					//!< Location of the center of mass
			PxF64				mT2[3];					//!< Moments of inertia
			PxF64				mTP[3];					//!< Products of inertia
			
			// Internal methods
			//				bool				Init();
			PxVec3				computeCenterOfMass();
			void				computeInertiaTensor(PxF64* J);
			void				computeCOMInertiaTensor(PxF64* J);
			void				computeFaceNormal(Face & f, PxU32 * indices);
			
			void				computeProjectionIntegrals(const Face& f);
			void				computeFaceIntegrals(const Face& f);
	};

	#define X 0u
	#define Y 1u
	#define Z 2u

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Constructor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VolumeIntegrator::VolumeIntegrator(PxSimpleTriangleMesh mesh_, PxF64	density)
	{
		mDensity	= density;
		mMass		= 0.0;
		this->mesh	= mesh_;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Destructor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VolumeIntegrator::~VolumeIntegrator()
	{
	}

	void VolumeIntegrator::computeFaceNormal(Face & f, PxU32 * indices)
	{
		const PxU8 * vertPointer = (const PxU8 *)mesh.points.data;

		//two edges
		PxVec3 d1 = (*(PxVec3 *)(vertPointer + mesh.points.stride * indices[1] )) - (*(PxVec3 *)(vertPointer + mesh.points.stride * indices[0] ));
		PxVec3 d2 = (*(PxVec3 *)(vertPointer + mesh.points.stride * indices[2] )) - (*(PxVec3 *)(vertPointer + mesh.points.stride * indices[1] ));


		PxVec3 normal = d1.cross(d2);

		normal.normalize();

		f.w = - normal.dot(		(*(PxVec3 *)(vertPointer + mesh.points.stride * indices[0] ))	); 

		f.Norm[0] = normal.x;
		f.Norm[1] = normal.y;
		f.Norm[2] = normal.z;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes volume integrals for a polyhedron by summing surface integrals over its faces.
	*	\param		ir	[out] a result structure.
	*	\return		true if success
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool VolumeIntegrator::computeVolumeIntegrals(PxIntegrals& ir)
	{
		// Clear all integrals
		mT0 = mT1[X] = mT1[Y] = mT1[Z] = mT2[X] = mT2[Y] = mT2[Z] = mTP[X] = mTP[Y] = mTP[Z] = 0;

		Face f;
		const PxU8 * trigPointer = (const PxU8 *)mesh.triangles.data;
		for(PxU32 i=0;i<mesh.triangles.count;i++, trigPointer += mesh.triangles.stride)
			{

			if (mesh.flags & PxMeshFlag::e16_BIT_INDICES)
				{
				f.Verts[0] = ((PxU16 *)trigPointer)[0];
				f.Verts[1] = ((PxU16 *)trigPointer)[1];
				f.Verts[2] = ((PxU16 *)trigPointer)[2];
				}
			else
				{
				f.Verts[0] = ((PxU32 *)trigPointer)[0];
				f.Verts[1] = ((PxU32 *)trigPointer)[1];
				f.Verts[2] = ((PxU32 *)trigPointer)[2];
				}

			if (mesh.flags & PxMeshFlag::eFLIPNORMALS)
				{
				PxU32 t = f.Verts[1];
				f.Verts[1] = f.Verts[2];
				f.Verts[2] = t;
				}

			//compute face normal:
			computeFaceNormal(f,f.Verts);

			// Compute alpha/beta/gamma as the right-handed permutation of (x,y,z) that maximizes |n|
			PxF64 nx = fabs(f.Norm[X]);
			PxF64 ny = fabs(f.Norm[Y]);
			PxF64 nz = fabs(f.Norm[Z]);
			if (nx > ny && nx > nz) mC = X;
			else mC = (ny > nz) ? Y : Z;
			mA = (mC + 1) % 3;
			mB = (mA + 1) % 3;

			// Compute face contribution
			computeFaceIntegrals(f);

			// Update integrals
			mT0 += f.Norm[X] * ((mA == X) ? mFa : ((mB == X) ? mFb : mFc));

			mT1[mA] += f.Norm[mA] * mFaa;
			mT1[mB] += f.Norm[mB] * mFbb;
			mT1[mC] += f.Norm[mC] * mFcc;

			mT2[mA] += f.Norm[mA] * mFaaa;
			mT2[mB] += f.Norm[mB] * mFbbb;
			mT2[mC] += f.Norm[mC] * mFccc;

			mTP[mA] += f.Norm[mA] * mFaab;
			mTP[mB] += f.Norm[mB] * mFbbc;
			mTP[mC] += f.Norm[mC] * mFcca;
			}

		mT1[X] /= 2; mT1[Y] /= 2; mT1[Z] /= 2;
		mT2[X] /= 3; mT2[Y] /= 3; mT2[Z] /= 3;
		mTP[X] /= 2; mTP[Y] /= 2; mTP[Z] /= 2;

		// Fill result structure
		ir.COM = computeCenterOfMass();
		computeInertiaTensor((PxF64*)ir.inertiaTensor);
		computeCOMInertiaTensor((PxF64*)ir.COMInertiaTensor);
		ir.mass = mMass;
		return true;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes the center of mass.
	*	\return		The center of mass.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	PxVec3 VolumeIntegrator::computeCenterOfMass()
	{
		// Compute center of mass
		PxVec3 COM(0.0f, 0.0f, 0.0f);
		if(mT0!=0.0f)
			{
			COM.x = float(mT1[X] / mT0);
			COM.y = float(mT1[Y] / mT0);
			COM.z = float(mT1[Z] / mT0);
			}
		return COM;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Setups the inertia tensor relative to the origin.
	*	\param		it	[out] the returned inertia tensor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VolumeIntegrator::computeInertiaTensor(PxF64* it)
	{
		PxF64 J[3][3];

		// Compute inertia tensor
		J[X][X] = mDensity * (mT2[Y] + mT2[Z]);
		J[Y][Y] = mDensity * (mT2[Z] + mT2[X]);
		J[Z][Z] = mDensity * (mT2[X] + mT2[Y]);

		J[X][Y] = J[Y][X] = - mDensity * mTP[X];
		J[Y][Z] = J[Z][Y] = - mDensity * mTP[Y];
		J[Z][X] = J[X][Z] = - mDensity * mTP[Z];

		PxMemCopy(it, J, 9*sizeof(PxF64));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Setups the inertia tensor relative to the COM.
	*	\param		it	[out] the returned inertia tensor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VolumeIntegrator::computeCOMInertiaTensor(PxF64* it)
	{
		PxF64 J[3][3];

		mMass = mDensity * mT0;

		const PxVec3 COM = computeCenterOfMass();
		const PxVec3 MassCOM(static_cast<PxF32>(mMass) * COM); 
		const PxVec3 MassCOM2(MassCOM.x * COM.x, MassCOM.y * COM.y, MassCOM.z * COM.z); 

		// Compute initial inertia tensor
		computeInertiaTensor((PxF64*)J);

		// Translate inertia tensor to center of mass
		// Huyghens' theorem:
		// Jx'x' = Jxx - m*(YG^2+ZG^2)
		// Jy'y' = Jyy - m*(ZG^2+XG^2)
		// Jz'z' = Jzz - m*(XG^2+YG^2)
		// XG, YG, ZG = new origin
		// YG^2+ZG^2 = dx^2
		J[X][X] -= MassCOM2.y + MassCOM2.z; 
		J[Y][Y] -= MassCOM2.z + MassCOM2.x; 
		J[Z][Z] -= MassCOM2.x + MassCOM2.y; 
		
		// Huyghens' theorem:
		// Jx'y' = Jxy - m*XG*YG
		// Jy'z' = Jyz - m*YG*ZG
		// Jz'x' = Jzx - m*ZG*XG
		// ### IS THE SIGN CORRECT ?
		J[X][Y] = J[Y][X] += MassCOM.x * COM.y; 
		J[Y][Z] = J[Z][Y] += MassCOM.y * COM.z; 
		J[Z][X] = J[X][Z] += MassCOM.z * COM.x; 

		PxMemCopy(it, J, 9*sizeof(PxF64));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes integrals over a face projection from the coordinates of the projections vertices.
	*	\param		f	[in] a face structure.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VolumeIntegrator::computeProjectionIntegrals(const Face& f)
	{
		mP1 = mPa = mPb = mPaa = mPab = mPbb = mPaaa = mPaab = mPabb = mPbbb = 0.0;

		const PxU8* vertPointer = (const PxU8*)mesh.points.data;
		for(PxU32 i=0;i<3;i++)
		{
			PxVec3& p0 = (*(PxVec3 *)(vertPointer + mesh.points.stride * (f.Verts[i]) ));
			PxVec3& p1 = (*(PxVec3 *)(vertPointer + mesh.points.stride * (f.Verts[(i+1) % 3]) ));


			PxF64 a0 = p0[mA];
			PxF64 b0 = p0[mB];
			PxF64 a1 = p1[mA];
			PxF64 b1 = p1[mB];

			PxF64 da = a1 - a0;				// DeltaA
			PxF64 db = b1 - b0;				// DeltaB

			PxF64 a0_2 = a0 * a0;				// Alpha0^2
			PxF64 a0_3 = a0_2 * a0;			// ...
			PxF64 a0_4 = a0_3 * a0;

			PxF64 b0_2 = b0 * b0;
			PxF64 b0_3 = b0_2 * b0;
			PxF64 b0_4 = b0_3 * b0;

			PxF64 a1_2 = a1 * a1;
			PxF64 a1_3 = a1_2 * a1; 

			PxF64 b1_2 = b1 * b1;
			PxF64 b1_3 = b1_2 * b1;

			PxF64 C1 = a1 + a0;

			PxF64 Ca = a1*C1 + a0_2;
			PxF64 Caa = a1*Ca + a0_3;
			PxF64 Caaa = a1*Caa + a0_4;

			PxF64 Cb = b1*(b1 + b0) + b0_2;
			PxF64 Cbb = b1*Cb + b0_3;
			PxF64 Cbbb = b1*Cbb + b0_4;

			PxF64 Cab = 3*a1_2 + 2*a1*a0 + a0_2;
			PxF64 Kab = a1_2 + 2*a1*a0 + 3*a0_2;

			PxF64 Caab = a0*Cab + 4*a1_3;
			PxF64 Kaab = a1*Kab + 4*a0_3;

			PxF64 Cabb = 4*b1_3 + 3*b1_2*b0 + 2*b1*b0_2 + b0_3;
			PxF64 Kabb = b1_3 + 2*b1_2*b0 + 3*b1*b0_2 + 4*b0_3;

			mP1		+= db*C1;
			mPa		+= db*Ca;
			mPaa	+= db*Caa;
			mPaaa	+= db*Caaa;
			mPb		+= da*Cb;
			mPbb	+= da*Cbb;
			mPbbb	+= da*Cbbb;
			mPab	+= db*(b1*Cab + b0*Kab);
			mPaab	+= db*(b1*Caab + b0*Kaab);
			mPabb	+= da*(a1*Cabb + a0*Kabb);
		}

		mP1		/= 2.0;
		mPa		/= 6.0;
		mPaa	/= 12.0;
		mPaaa	/= 20.0;
		mPb		/= -6.0;
		mPbb	/= -12.0;
		mPbbb	/= -20.0;
		mPab	/= 24.0;
		mPaab	/= 60.0;
		mPabb	/= -60.0;
	}

	#define		SQR(x)			((x)*(x))						//!< Returns x square
	#define		CUBE(x)			((x)*(x)*(x))					//!< Returns x cube

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes surface integrals over a polyhedral face from the integrals over its projection.
	*	\param		f	[in] a face structure.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void VolumeIntegrator::computeFaceIntegrals(const Face& f)
	{
		computeProjectionIntegrals(f);

		PxF64 w = f.w;
		const PxF64* n = f.Norm;
		PxF64 k1 = 1 / n[mC];
		PxF64 k2 = k1 * k1;
		PxF64 k3 = k2 * k1;
		PxF64 k4 = k3 * k1;

		mFa = k1 * mPa;
		mFb = k1 * mPb;
		mFc = -k2 * (n[mA]*mPa + n[mB]*mPb + w*mP1);

		mFaa = k1 * mPaa;
		mFbb = k1 * mPbb;
		mFcc = k3 * (SQR(n[mA])*mPaa + 2*n[mA]*n[mB]*mPab + SQR(n[mB])*mPbb + w*(2*(n[mA]*mPa + n[mB]*mPb) + w*mP1));

		mFaaa = k1 * mPaaa;
		mFbbb = k1 * mPbbb;
		mFccc = -k4 * (CUBE(n[mA])*mPaaa + 3*SQR(n[mA])*n[mB]*mPaab 
			+ 3*n[mA]*SQR(n[mB])*mPabb + CUBE(n[mB])*mPbbb
			+ 3*w*(SQR(n[mA])*mPaa + 2*n[mA]*n[mB]*mPab + SQR(n[mB])*mPbb)
			+ w*w*(3*(n[mA]*mPa + n[mB]*mPb) + w*mP1));

		mFaab = k1 * mPaab;
		mFbbc = -k2 * (n[mA]*mPabb + n[mB]*mPbbb + w*mPbb);
		mFcca = k3 * (SQR(n[mA])*mPaaa + 2*n[mA]*n[mB]*mPaab + SQR(n[mB])*mPabb + w*(2*(n[mA]*mPaa + n[mB]*mPab) + w*mPa));
	}

	/*
	*	This code computes volume integrals needed to compute mass properties of polyhedral bodies.
	*	Based on public domain code by David Eberly.
	*/

	class VolumeIntegratorEberly 
	{
	public:
		VolumeIntegratorEberly(const PxSimpleTriangleMesh& mesh, PxF64	mDensity);
		~VolumeIntegratorEberly();
		bool	computeVolumeIntegrals(PxIntegrals& ir, const PxVec3& origin);
	private:

		void	subexpressions(PxF64 w0,PxF64 w1,PxF64 w2,PxF64& f1,PxF64& f2,PxF64& f3,PxF64& g0,PxF64& g1,PxF64& g2);

	private:
		VolumeIntegratorEberly& operator=(const VolumeIntegratorEberly&);
		const PxSimpleTriangleMesh&	mMesh;
		PxF64					mMass;
		PxF64					mDensity;			
	};

	void VolumeIntegratorEberly::subexpressions(PxF64 w0,PxF64 w1,PxF64 w2,PxF64& f1,PxF64& f2,PxF64& f3,PxF64& g0,PxF64& g1,PxF64& g2)
	{
		PxF64 temp0 = w0+w1; 
		f1 = temp0+w2; 
		PxF64 temp1 = w0*w0; 
		PxF64 temp2 = temp1+w1*temp0;
		f2 = temp2+w2*f1; 
		f3 = w0*temp1+w1*temp2+w2*f2;
		g0 = f2+w0*(f1+w0); 
		g1 = f2+w1*(f1+w1); 
		g2 = f2+w2*(f1+w2);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Constructor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VolumeIntegratorEberly::VolumeIntegratorEberly(const PxSimpleTriangleMesh& mesh, PxF64	density)
		: mMesh(mesh), mMass(0), mDensity(density)
	{
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Destructor.
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	VolumeIntegratorEberly::~VolumeIntegratorEberly()
	{
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/**
	*	Computes volume integrals for a polyhedron by summing surface integrals over its faces.
	*	\param		ir	[out] a result structure.
	*	\param		origin [in] the origin of the mesh vertices. All vertices will be shifted accordingly prior to computing the volume integrals.
					Can improve accuracy, for example, if the centroid is used in the case of a convex mesh. Note: the returned inertia will not be relative to this origin but relative to (0,0,0).
	*	\return		true if success
	*/
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	bool VolumeIntegratorEberly::computeVolumeIntegrals(PxIntegrals& ir, const PxVec3& origin)
	{
		const PxF64 mult[10] = {1.0f/6.0f,1.0f/24.0f,1.0f/24.0f,1.0f/24.0f,1.0f/60.0f,1.0f/60.0f,1.0f/60.0f,1.0f/120.0f,1.0f/120.0f,1.0f/120.0f};
		PxF64 intg[10] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f}; // order: 1, x, y, z, x^2, y^2, z^2, xy, yz, zx

		const PxU8 * trigPointer = (const PxU8 *)mMesh.triangles.data;
		for(PxU32 i=0;i<mMesh.triangles.count;i++, trigPointer += mMesh.triangles.stride)
		{
			PxU32 i0 = 0;
			PxU32 i1 = 0;
			PxU32 i2 = 0;

			if (mMesh.flags & PxMeshFlag::e16_BIT_INDICES)
			{
				i0 = ((PxU16 *)trigPointer)[0];
				i1 = ((PxU16 *)trigPointer)[1];
				i2 = ((PxU16 *)trigPointer)[2];
			}
			else
			{
				i0 = ((PxU32 *)trigPointer)[0];
				i1 = ((PxU32 *)trigPointer)[1];
				i2 = ((PxU32 *)trigPointer)[2];
			}

			if (mMesh.flags & PxMeshFlag::eFLIPNORMALS)
			{
				PxU32 t = i1;
				i1 = i2;
				i2 = t;
			}

			const PxU8 * vertPointer = (const PxU8 *)mMesh.points.data;
			PxVec3 p0 = (*(PxVec3 *)(vertPointer + mMesh.points.stride * i0)) - origin;
			PxVec3 p1 = (*(PxVec3 *)(vertPointer + mMesh.points.stride * i1)) - origin;
			PxVec3 p2 = (*(PxVec3 *)(vertPointer + mMesh.points.stride * i2)) - origin;

			PxF64 x0 = p0.x; PxF64 y0 = p0.y; PxF64 z0 = p0.z;
			PxF64 x1 = p1.x; PxF64 y1 = p1.y; PxF64 z1 = p1.z;
			PxF64 x2 = p2.x; PxF64 y2 = p2.y; PxF64 z2 = p2.z;

			// get edges and cross product of edges
			PxF64 a1 = x1-x0; PxF64 b1 = y1-y0; PxF64 c1 = z1-z0; 
			PxF64 a2 = x2-x0; PxF64 b2 = y2-y0; PxF64 c2 = z2-z0;
			PxF64 d0 = b1*c2-b2*c1; PxF64 d1 = a2*c1-a1*c2; PxF64 d2 = a1*b2-a2*b1;

			// compute integral terms
			PxF64 f1x; PxF64 f2x; PxF64 f3x; PxF64 g0x; PxF64 g1x; PxF64 g2x;
			PxF64 f1y; PxF64 f2y; PxF64 f3y; PxF64 g0y; PxF64 g1y; PxF64 g2y;
			PxF64 f1z; PxF64 f2z; PxF64 f3z; PxF64 g0z; PxF64 g1z; PxF64 g2z;

			subexpressions(x0,x1,x2,f1x,f2x,f3x,g0x,g1x,g2x);
			subexpressions(y0,y1,y2,f1y,f2y,f3y,g0y,g1y,g2y);
			subexpressions(z0,z1,z2,f1z,f2z,f3z,g0z,g1z,g2z);

			// update integrals
			intg[0] += d0*f1x;
			intg[1] += d0*f2x; intg[2] += d1*f2y; intg[3] += d2*f2z;
			intg[4] += d0*f3x; intg[5] += d1*f3y; intg[6] += d2*f3z;
			intg[7] += d0*(y0*g0x+y1*g1x+y2*g2x);
			intg[8] += d1*(z0*g0y+z1*g1y+z2*g2y);
			intg[9] += d2*(x0*g0z+x1*g1z+x2*g2z);
		}
		for (PxU32 i = 0; i < 10; i++)
		{
			intg[i] *= mult[i];
		}

		ir.mass = mMass = intg[0];
		// center of mass
		ir.COM.x = PxReal(intg[1]/mMass);
		ir.COM.y = PxReal(intg[2]/mMass);
		ir.COM.z = PxReal(intg[3]/mMass);

		// inertia tensor relative to the provided origin parameter
		ir.inertiaTensor[0][0] = intg[5]+intg[6];
		ir.inertiaTensor[1][1] = intg[4]+intg[6];
		ir.inertiaTensor[2][2] = intg[4]+intg[5];
		ir.inertiaTensor[0][1] = ir.inertiaTensor[1][0] = -intg[7];
		ir.inertiaTensor[1][2] = ir.inertiaTensor[2][1] = -intg[8];
		ir.inertiaTensor[0][2] = ir.inertiaTensor[2][0] = -intg[9];		

		// inertia tensor relative to center of mass
		ir.COMInertiaTensor[0][0] = ir.inertiaTensor[0][0] -mMass*(ir.COM.y*ir.COM.y+ir.COM.z*ir.COM.z);
		ir.COMInertiaTensor[1][1] = ir.inertiaTensor[1][1] -mMass*(ir.COM.z*ir.COM.z+ir.COM.x*ir.COM.x);
		ir.COMInertiaTensor[2][2] = ir.inertiaTensor[2][2] -mMass*(ir.COM.x*ir.COM.x+ir.COM.y*ir.COM.y);
		ir.COMInertiaTensor[0][1] = ir.COMInertiaTensor[1][0] = (ir.inertiaTensor[0][1] +mMass*ir.COM.x*ir.COM.y);
		ir.COMInertiaTensor[1][2] = ir.COMInertiaTensor[2][1] = (ir.inertiaTensor[1][2] +mMass*ir.COM.y*ir.COM.z);
		ir.COMInertiaTensor[0][2] = ir.COMInertiaTensor[2][0] = (ir.inertiaTensor[0][2] +mMass*ir.COM.z*ir.COM.x);

		// inertia tensor relative to (0,0,0)
		if (!origin.isZero())
		{
			PxVec3 sum = ir.COM + origin;
			ir.inertiaTensor[0][0] -= mMass*((ir.COM.y*ir.COM.y+ir.COM.z*ir.COM.z) - (sum.y*sum.y+sum.z*sum.z));
			ir.inertiaTensor[1][1] -= mMass*((ir.COM.z*ir.COM.z+ir.COM.x*ir.COM.x) - (sum.z*sum.z+sum.x*sum.x));
			ir.inertiaTensor[2][2] -= mMass*((ir.COM.x*ir.COM.x+ir.COM.y*ir.COM.y) - (sum.x*sum.x+sum.y*sum.y));
			ir.inertiaTensor[0][1] = ir.inertiaTensor[1][0] = ir.inertiaTensor[0][1] + mMass*((ir.COM.x*ir.COM.y) - (sum.x*sum.y));
			ir.inertiaTensor[1][2] = ir.inertiaTensor[2][1] = ir.inertiaTensor[1][2] + mMass*((ir.COM.y*ir.COM.z) - (sum.y*sum.z));
			ir.inertiaTensor[0][2] = ir.inertiaTensor[2][0] = ir.inertiaTensor[0][2] + mMass*((ir.COM.z*ir.COM.x) - (sum.z*sum.x));
			ir.COM = sum;
		}

		return true;
	}
} // namespace

// Wrapper
bool computeVolumeIntegrals(const PxSimpleTriangleMesh& mesh, PxReal density, PxIntegrals& integrals)
{
	VolumeIntegrator v(mesh, density);
	return v.computeVolumeIntegrals(integrals);
}

// Wrapper
bool computeVolumeIntegralsEberly(const PxSimpleTriangleMesh& mesh, PxReal density, PxIntegrals& integrals, const PxVec3& origin)
{
	VolumeIntegratorEberly v(mesh, density);
	v.computeVolumeIntegrals(integrals, origin);

	return true;
}

}

//#endif
