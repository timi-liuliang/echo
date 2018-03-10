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


#ifndef PX_PHYSICS_EXTENSIONS_INERTIATENSOR_H
#define PX_PHYSICS_EXTENSIONS_INERTIATENSOR_H

#include "CmPhysXCommon.h"
#include "PxGeometry.h"
#include "PxBoxGeometry.h"
#include "PxSphereGeometry.h"
#include "PxCapsuleGeometry.h"
#include "PxConvexMeshGeometry.h"
#include "PxConvexMesh.h"
#include "PxMat33.h"
#include "PsMathUtils.h"
#include "PsInlineArray.h"
#include "PxMathUtils.h"

namespace physx
{
namespace Ext
{
	class MassProps
	{
	public:
		// mass props of geometry at density 1, mass & inertia scale linearly with density

		MassProps(const PxGeometry& geometry): 
			mCom(0)
		{
			switch(geometry.getType())
			{
			case PxGeometryType::eSPHERE:
				{
					const PxSphereGeometry& s = static_cast<const PxSphereGeometry&>(geometry);
					mMass = (4.0f/3.0f) * PxPi * s.radius * s.radius * s.radius;
					mInertia = PxMat33::createDiagonal(PxVec3(2.0f/5.0f * mMass * s.radius * s.radius));
					break;
				}

			case PxGeometryType::eBOX:
				{
					const PxBoxGeometry& b = static_cast<const PxBoxGeometry&>(geometry);
					mMass = b.halfExtents.x * b.halfExtents.y * b.halfExtents.z * 8.0f;
					PxVec3 d2 = b.halfExtents.multiply(b.halfExtents);
					mInertia = PxMat33::createDiagonal(PxVec3(d2.y+d2.z, d2.x+d2.z, d2.x+d2.y)) * (mMass * 2.0f/3.0f);
					break;
				}

			case PxGeometryType::eCAPSULE:
				{
					const PxCapsuleGeometry& c = static_cast<const PxCapsuleGeometry&>(geometry);
					PxReal r = c.radius, h = c.halfHeight;
					mMass = ((4.0f/3.0f) * r + 2 * c.halfHeight) * PxPi * r * r;

					PxReal a = r*r*r * (8.0f/15.0f) + h*r*r * (3.0f/2.0f) + h*h*r * (4.0f/3.0f) + h*h*h * (2.0f/3.0f);
					PxReal b = r*r*r * (8.0f/15.0f) + h*r*r;				
					mInertia = PxMat33::createDiagonal(PxVec3(b,a,a) * PxPi * r * r);

					break;
				}

			case PxGeometryType::eCONVEXMESH:
				{
					const PxConvexMeshGeometry& c = static_cast<const PxConvexMeshGeometry&>(geometry);
					c.convexMesh->getMassInformation(mMass, mInertia, mCom);
					
					const PxMeshScale& s = c.scale;
					mMass *= (s.scale.x * s.scale.y * s.scale.z);
					mCom = s.rotation.rotate(s.scale.multiply(s.rotation.rotateInv(mCom)));
					mInertia = scaleInertia(mInertia, s.rotation, s.scale);
					break;
				}
			case PxGeometryType::eHEIGHTFIELD:
			case PxGeometryType::ePLANE:
			case PxGeometryType::eTRIANGLEMESH:
			case PxGeometryType::eINVALID:
			case PxGeometryType::eGEOMETRY_COUNT:
			default:
				break;
			}
		}

		MassProps(PxReal mass, const PxMat33& inertia, const PxVec3& com):
		  mInertia(inertia), mCom(com), mMass(mass)	{}

		MassProps operator*(PxReal scale)					const
		{
			return MassProps(mMass * scale, mInertia * scale, mCom);
		}

		static MassProps sum(MassProps* props, PxTransform* tm, PxU32 count)
		{
			PxReal	mass = 0;
			PxVec3	com(0);
			PxMat33 inertia = PxMat33(PxZero);
			
			Ps::InlineArray<PxVec3, 16> comArray; 
			comArray.reserve(count);

			for(PxU32 i=0;i<count;i++)
			{
				mass += props[i].mMass;
				comArray[i] = tm[i].transform(props[i].mCom);
				com += comArray[i] * props[i].mMass;
			}
			com /= mass;

			for(PxU32 i=0;i<count;i++)
				inertia += translateInertia(rotateInertia(props[i].mInertia, tm[i].q), props[i].mMass, com - comArray[i]);

			return MassProps(mass, inertia, com);
		}

		PxReal getMass()									const
		{
			return mMass;
		}

		PxVec3 getMassSpaceInertia(PxQuat& massFrame) const
		{
			return  PxDiagonalize(mInertia, massFrame);
		}

	//private:

		// rotate the inertia around the center of mass
		static PxMat33 rotateInertia(const PxMat33& inertia, const PxQuat& q)
		{
			PxMat33 m(q);
			return m * inertia * m.getTranspose();
		}

		// translate from CoM frame to a relative frame using parallel axis theorem
		static PxMat33 translateInertia(const PxMat33& inertia, const PxReal mass, const PxVec3& v)
		{
			PxMat33 s = Ps::star(v);
			return inertia + mass * s * s.getTranspose();
		}

		static PxMat33 scaleInertia(const PxMat33& inertia, const PxQuat& scaleRotation, const PxVec3& scale)
		{
			PxMat33 localInertia = rotateInertia(inertia, scaleRotation);		// rotate inertia into scaling frame
			PxVec3 diagonal(localInertia[0][0], localInertia[1][1], localInertia[2][2]);

			PxVec3 xyz2 = PxVec3(diagonal.dot(PxVec3(0.5f))) - diagonal;						// original x^2, y^2, z^2
			PxVec3 scaledxyz2 = xyz2.multiply(scale).multiply(scale);

			PxReal xx = scaledxyz2.y + scaledxyz2.z,
				   yy = scaledxyz2.z + scaledxyz2.x,
				   zz = scaledxyz2.x + scaledxyz2.y;

			PxReal xy = localInertia[0][1] * scale.x * scale.y,
			       xz = localInertia[0][2] * scale.x * scale.z,
				   yz = localInertia[1][2] * scale.y * scale.z;

			PxMat33 scaledInertia(PxVec3(xx, xy, xz),
								  PxVec3(xy, yy, yz),
								  PxVec3(xz, yz, zz));

			return rotateInertia(scaledInertia * (scale.x * scale.y * scale.z), scaleRotation.getConjugate());
		}


		PxMat33 mInertia;
		PxVec3  mCom;
		PxReal	mMass;
	};



	class InertiaTensorComputer
	{
		public:
									InertiaTensorComputer(bool initTozero = true);
									InertiaTensorComputer(const PxMat33& inertia, const PxVec3& com, PxReal mass);
									~InertiaTensorComputer();

		PX_INLINE	void			zero();																//sets to zero mass
		PX_INLINE	void			setDiagonal(PxReal mass, const PxVec3& diagonal);					//sets as a diagonal tensor
		PX_INLINE	void			rotate(const PxMat33& rot);											//rotates the mass
					void			translate(const PxVec3& t);											//translates the mass
		PX_INLINE	void			transform(const PxTransform& transform);							//transforms the mass
		PX_INLINE	void			scaleDensity(PxReal densityScale);									//scales by a density factor
		PX_INLINE	void			add(const InertiaTensorComputer& it);								//adds a mass
		PX_INLINE	void			center();															//recenters inertia around center of mass

					void			setBox(const PxVec3& halfWidths);									//sets as an axis aligned box
		PX_INLINE	void			setBox(const PxVec3& halfWidths, const PxTransform* pose);			//sets as an oriented box

					void			setSphere(PxReal radius);
		PX_INLINE	void			setSphere(PxReal radius, const PxTransform* pose);

					void			setCylinder(int dir, PxReal r, PxReal l);
		PX_INLINE	void			setCylinder(int dir, PxReal r, PxReal l, const PxTransform* pose);

					void			setCapsule(int dir, PxReal r, PxReal l);
		PX_INLINE	void			setCapsule(int dir, PxReal r, PxReal l, const PxTransform* pose);
					void			addCapsule(PxReal density, int dir, PxReal r, PxReal l, const PxTransform* pose = 0);

					void			setEllipsoid(PxReal rx, PxReal ry, PxReal rz);
		PX_INLINE	void			setEllipsoid(PxReal rx, PxReal ry, PxReal rz, const PxTransform* pose);

		PX_INLINE	PxVec3			getCenterOfMass()				const	{ return mG;	}
		PX_INLINE	PxReal			getMass()						const	{ return mMass;	}
		PX_INLINE	PxMat33			getInertia()					const	{ return mI;	}

		private:
					PxMat33				mI;
					PxVec3				mG;
					PxReal				mMass;
	};


	//--------------------------------------------------------------
	//
	// Helper routines
	//
	//--------------------------------------------------------------

	// Special version allowing 2D quads
	PX_INLINE PxReal volume(const PxVec3& extents)
	{
		PxReal v = 1.0f;
		if(extents.x != 0.0f)	v*=extents.x;
		if(extents.y != 0.0f)	v*=extents.y;
		if(extents.z != 0.0f)	v*=extents.z;
		return v;
	}

	// Sphere
	PX_INLINE PxReal computeSphereRatio(PxReal radius)						{ return (4.0f/3.0f) * PxPi * radius * radius * radius;	}
	PxReal computeSphereMass(PxReal radius, PxReal density)					{ return density * computeSphereRatio(radius);				}
	PxReal computeSphereDensity(PxReal radius, PxReal mass)					{ return mass / computeSphereRatio(radius);					}

	// Box
	PX_INLINE PxReal computeBoxRatio(const PxVec3& extents)					{ return volume(extents);									}
	PxReal computeBoxMass(const PxVec3& extents, PxReal density)			{ return density * computeBoxRatio(extents);				}
	PxReal computeBoxDensity(const PxVec3& extents, PxReal mass)			{ return mass / computeBoxRatio(extents);					}

	// Ellipsoid
	PX_INLINE PxReal computeEllipsoidRatio(const PxVec3& extents)			{ return (4.0f/3.0f) * PxPi * volume(extents);		}
	PxReal computeEllipsoidMass(const PxVec3& extents, PxReal density)		{ return density * computeEllipsoidRatio(extents);			}
	PxReal computeEllipsoidDensity(const PxVec3& extents, PxReal mass)		{ return mass / computeEllipsoidRatio(extents);				}

	// Cylinder
	PX_INLINE PxReal computeCylinderRatio(PxReal r, PxReal l)				{ return PxPi * r  * r * (2.0f*l);					}
	PxReal computeCylinderMass(PxReal r, PxReal l, PxReal density)			{ return density * computeCylinderRatio(r, l);				}
	PxReal computeCylinderDensity(PxReal r, PxReal l, PxReal mass)			{ return mass / computeCylinderRatio(r, l);					}

	// Capsule
	PX_INLINE PxReal computeCapsuleRatio(PxReal r, PxReal l)				{ return computeSphereRatio(r) + computeCylinderRatio(r, l);}
	PxReal computeCapsuleMass(PxReal r, PxReal l, PxReal density)			{ return density * computeCapsuleRatio(r, l);				}
	PxReal computeCapsuleDensity(PxReal r, PxReal l, PxReal mass)			{ return mass / computeCapsuleRatio(r, l);					}

	// Cone
	PX_INLINE PxReal computeConeRatio(PxReal r, PxReal l)					{ return PxPi * r * r * PxAbs(l)/3.0f;			}
	PxReal computeConeMass(PxReal r, PxReal l, PxReal density)				{ return density * computeConeRatio(r, l);					}
	PxReal computeConeDensity(PxReal r, PxReal l, PxReal mass)				{ return mass / computeConeRatio(r, l);						}

	void computeBoxInertiaTensor(PxVec3& inertia, PxReal mass, PxReal xlength, PxReal ylength, PxReal zlength);
	void computeSphereInertiaTensor(PxVec3& inertia, PxReal mass, PxReal radius, bool hollow);
	bool jacobiTransform(PxI32 n, PxF64 a[], PxF64 w[]);
	bool diagonalizeInertiaTensor(const PxMat33& denseInertia, PxVec3& diagonalInertia, PxMat33& rotation);

} // namespace Ext

void Ext::computeBoxInertiaTensor(PxVec3& inertia, PxReal mass, PxReal xlength, PxReal ylength, PxReal zlength)
{
	//to model a hollow block, one would have to multiply coeff by up to two.
	const PxReal coeff = mass/12;
	inertia.x = coeff * (ylength*ylength + zlength*zlength);
	inertia.y = coeff * (xlength*xlength + zlength*zlength);
	inertia.z = coeff * (xlength*xlength + ylength*ylength);

	PX_ASSERT(inertia.x != 0.0);
	PX_ASSERT(inertia.y != 0.0);
	PX_ASSERT(inertia.z != 0.0);
	PX_ASSERT(inertia.isFinite());
}


void Ext::computeSphereInertiaTensor(PxVec3& inertia, PxReal mass, PxReal radius, bool hollow)
{
	inertia.x = mass * radius * radius;
	if (hollow) 
		inertia.x *= PxReal(2 / 3.0);
	else
		inertia.x *= PxReal(2 / 5.0);

	inertia.z = inertia.y = inertia.x;
	PX_ASSERT(inertia.isFinite());
}

//--------------------------------------------------------------
//
// InertiaTensorComputer implementation
//
//--------------------------------------------------------------

Ext::InertiaTensorComputer::InertiaTensorComputer(bool initTozero)
{
	if (initTozero)
		zero();
}


Ext::InertiaTensorComputer::InertiaTensorComputer(const PxMat33& inertia, const PxVec3& com, PxReal mass) :
	mI(inertia),
	mG(com),
	mMass(mass)
{
}


Ext::InertiaTensorComputer::~InertiaTensorComputer()
{
	//nothing
}


PX_INLINE void Ext::InertiaTensorComputer::zero()
{
	mMass = 0.0f;
	mI = PxMat33(PxZero);
	mG = PxVec3(0);
}


PX_INLINE void Ext::InertiaTensorComputer::setDiagonal(PxReal mass, const PxVec3& diag)
{
	mMass = mass;
	mI = PxMat33::createDiagonal(diag);
	mG = PxVec3(0);
	PX_ASSERT(mI.column0.isFinite() && mI.column1.isFinite() && mI.column2.isFinite());
	PX_ASSERT(PxIsFinite(mMass));
}


void Ext::InertiaTensorComputer::setBox(const PxVec3& halfWidths)
{
	// Setup inertia tensor for a cube with unit density
	const PxReal mass = 8.0f * computeBoxRatio(halfWidths);
	const PxReal s =(1.0f/3.0f) * mass;

	const PxReal x = halfWidths.x*halfWidths.x;
	const PxReal y = halfWidths.y*halfWidths.y;
	const PxReal z = halfWidths.z*halfWidths.z;

	setDiagonal(mass, PxVec3(y+z, z+x, x+y) * s);
}


PX_INLINE void Ext::InertiaTensorComputer::rotate(const PxMat33& rot)
{
	//well known inertia tensor rotation expression is: RIR' -- this could be optimized due to symmetry, see code to do that in Body::updateGlobalInverseInertia
	mI = rot * mI * rot.getTranspose();
	PX_ASSERT(mI.column0.isFinite() && mI.column1.isFinite() && mI.column2.isFinite());
	//com also needs to be rotated
	mG = rot * mG;
	PX_ASSERT(mG.isFinite());
}


void Ext::InertiaTensorComputer::translate(const PxVec3& t)
{
	if (!t.isZero())	//its common for this to be zero
	{
		PxMat33 t1, t2;

		t1.column0 = PxVec3(0, mG.z, -mG.y);
		t1.column1 = PxVec3(-mG.z, 0, mG.x);
		t1.column2 = PxVec3(mG.y, -mG.x, 0);

		PxVec3 sum = mG + t;
		if (sum.isZero())
		{
			mI += (t1 * t1)*mMass;
		}
		else
		{			
			t2.column0 = PxVec3(0, sum.z, -sum.y);
			t2.column1 = PxVec3(-sum.z, 0, sum.x);
			t2.column2 = PxVec3(sum.y, -sum.x, 0);
			mI += (t1 * t1 - t2 * t2)*mMass;
		}

		//move center of mass
		mG += t;

		PX_ASSERT(mI.column0.isFinite() && mI.column1.isFinite() && mI.column2.isFinite());
		PX_ASSERT(mG.isFinite());
	}
}


PX_INLINE void Ext::InertiaTensorComputer::transform(const PxTransform& transform)
{
	rotate(PxMat33(transform.q));
	translate(transform.p);
}


PX_INLINE void Ext::InertiaTensorComputer::setBox(const PxVec3& halfWidths, const PxTransform* pose)
{
	setBox(halfWidths);
	if (pose)
		transform(*pose);

}


PX_INLINE void Ext::InertiaTensorComputer::scaleDensity(PxReal densityScale)
{
	mI *= densityScale;
	mMass *= densityScale;
	PX_ASSERT(mI.column0.isFinite() && mI.column1.isFinite() && mI.column2.isFinite());
	PX_ASSERT(PxIsFinite(mMass));
}


PX_INLINE void Ext::InertiaTensorComputer::add(const InertiaTensorComputer& it)
{
	const PxReal TotalMass = mMass + it.mMass;
	mG = (mG * mMass + it.mG * it.mMass) / TotalMass;

	mMass = TotalMass;
	mI += it.mI;
	PX_ASSERT(mI.column0.isFinite() && mI.column1.isFinite() && mI.column2.isFinite());
	PX_ASSERT(mG.isFinite());
	PX_ASSERT(PxIsFinite(mMass));
}


PX_INLINE void Ext::InertiaTensorComputer::center()
{
	PxVec3 center = -mG;
	translate(center);
}


void Ext::InertiaTensorComputer::setSphere(PxReal radius)
{
	// Compute mass of the sphere
	const PxReal m = computeSphereRatio(radius);
	// Compute moment of inertia
	const PxReal s = m * radius * radius * (2.0f/5.0f);
	setDiagonal(m,PxVec3(s,s,s));
}


PX_INLINE void Ext::InertiaTensorComputer::setSphere(PxReal radius, const PxTransform* pose)
{
	setSphere(radius);
	if (pose)
		transform(*pose);
}


void Ext::InertiaTensorComputer::setCylinder(int dir, PxReal r, PxReal l)
{
	// Compute mass of cylinder
	const PxReal m = computeCylinderRatio(r, l);

	const PxReal i1 = r*r*m/2.0f;
	const PxReal i2 = (3.0f*r*r+4.0f*l*l)*m/12.0f;

	switch(dir)
	{
	case 0:		setDiagonal(m,PxVec3(i1,i2,i2));	break;
	case 1:		setDiagonal(m,PxVec3(i2,i1,i2));	break;
	default:	setDiagonal(m,PxVec3(i2,i2,i1));	break;
	}
}


PX_INLINE void Ext::InertiaTensorComputer::setCylinder(int dir, PxReal r, PxReal l, const PxTransform* pose)
{
	setCylinder(dir, r, l);
	if (pose)
		transform(*pose);
}


void Ext::InertiaTensorComputer::setCapsule(int dir, PxReal r, PxReal l)
{
	// Compute mass of capsule
	const PxReal m = computeCapsuleRatio(r, l);

	const PxReal t  = PxPi * r * r;
	const PxReal i1 = t * ((r*r*r * 8.0f/15.0f) + (l*r*r));
	const PxReal i2 = t * ((r*r*r * 8.0f/15.0f) + (l*r*r * 3.0f/2.0f) + (l*l*r * 4.0f/3.0f) + (l*l*l * 2.0f/3.0f));

	switch(dir)
	{
	case 0:		setDiagonal(m,PxVec3(i1,i2,i2));	break;
	case 1:		setDiagonal(m,PxVec3(i2,i1,i2));	break;
	default:	setDiagonal(m,PxVec3(i2,i2,i1));	break;
	}
}


PX_INLINE void Ext::InertiaTensorComputer::setCapsule(int dir, PxReal r, PxReal l, const PxTransform* pose)
{
	setCapsule(dir, r, l);
	if (pose)
		transform(*pose);
}


void Ext::InertiaTensorComputer::setEllipsoid(PxReal rx, PxReal ry, PxReal rz)
{
	// Compute mass of ellipsoid
	const PxReal m = computeEllipsoidRatio(PxVec3(rx, ry, rz));

	// Compute moment of inertia
	const PxReal s = m * (2.0f/5.0f);

	// Setup inertia tensor for an ellipsoid centered at the origin
	setDiagonal(m,PxVec3(ry*rz,rz*rx,rx*ry)*s);
}


PX_INLINE void Ext::InertiaTensorComputer::setEllipsoid(PxReal rx, PxReal ry, PxReal rz, const PxTransform* pose)
{
	setEllipsoid(rx,ry,rz);
	if (pose)
		transform(*pose);
}

}

#endif
