#ifndef __ECHO_FRUSTUM_H__
#define __ECHO_FRUSTUM_H__

#include "Box.h"
#include "Plane.h"
#include "Sphere.h"

namespace Echo
{

	class Frustum
	{
	public:

		/*
		4-------------7
		|\           /|
		| \         / |
		|  0-------3  |
		|  |       |  |
		6--|-------|--5
	   	 \ |       | /
		  \|       |/
		   2-------1
		*/

		enum Corner
		{
			CORNER_NLT	= 0,		//!< near left top
			CORNER_NRB	= 1,		//!< near right bottom
			CORNER_NLB	= 2,		//!< near left bottom
			CORNER_NRT	= 3,		//!< near right top

			CORNER_FLT	= 4,		//!< far left top
			CORNER_FRB	= 5,		//!< far right bottom
			CORNER_FLB	= 6,		//!< far left bottom
			CORNER_FRT	= 7,		//!< far right top
			CORNER_NUM,				//!< num
		};

		enum FrustumPlane
		{
			FP_NEAR		= 0,
			FP_FAR		= 1,
			FP_LEFT		= 2,
			FP_RIGHT	= 3,
			FP_TOP		= 4,
			FP_BOTTOM	= 5,
		};

	public:
		inline Frustum()
		{
			memset(m_corners, 0, sizeof(m_corners));
		}

		inline Frustum(const Vector3& vNLT, const Vector3& vNRB, const Vector3& vNLB, const Vector3& vNRT,
			const Vector3& vFLT, const Vector3& vFRB, const Vector3& vFLB, const Vector3& vFRT)
		{
			m_corners[CORNER_NLT] = vNLT;
			m_corners[CORNER_NRB] = vNRB;
			m_corners[CORNER_NLB] = vNLB;
			m_corners[CORNER_NRT] = vNRT;
			m_corners[CORNER_FLT] = vFLT;
			m_corners[CORNER_FRB] = vFRB;
			m_corners[CORNER_FLB] = vFLB;
			m_corners[CORNER_FRT] = vFRT;

			m_planes[FP_NEAR].set(vNLT, vNRB, vNLB);
			m_planes[FP_FAR].set(vFRT, vFLB, vFRB);
			m_planes[FP_LEFT].set(vFLT, vNLB, vFLB);
			m_planes[FP_RIGHT].set(vNRT, vFRB, vNRB);
			m_planes[FP_TOP].set(vFLT, vNRT, vNLT);
			m_planes[FP_BOTTOM].set(vNLB, vFRB, vFLB);
		}

		inline Frustum(const Frustum &src)
		{
			memcpy(&m_corners, &(src.m_corners), sizeof(m_corners));
			memcpy(&m_planes, &(src.m_planes), sizeof(m_planes));
		}

		inline void transform(const Matrix4& mat)
		{
			for (int i=0; i<CORNER_NUM; ++i)
			{
				m_corners[i] = m_corners[i] * mat;
			}
			m_planes[FP_NEAR].set(m_corners[CORNER_NLT], m_corners[CORNER_NRB], m_corners[CORNER_NLB]);
			m_planes[FP_FAR].set(m_corners[CORNER_FRT], m_corners[CORNER_FLB], m_corners[CORNER_FRB]);
			m_planes[FP_LEFT].set(m_corners[CORNER_FLT], m_corners[CORNER_NLB], m_corners[CORNER_FLB]);
			m_planes[FP_RIGHT].set(m_corners[CORNER_NRT], m_corners[CORNER_FRB], m_corners[CORNER_NRB]);
			m_planes[FP_TOP].set(m_corners[CORNER_FLT], m_corners[CORNER_NRT], m_corners[CORNER_NLT]);
			m_planes[FP_BOTTOM].set(m_corners[CORNER_NLB], m_corners[CORNER_FRB], m_corners[CORNER_FLB]);
		}

		// ¹¹½¨AABB
		void buildAABB( Box& oAABB) const;

	public:
		inline const Frustum& operator = (const Frustum& src)
		{
			memcpy(&m_corners, &(src.m_corners), sizeof(m_corners));
			return *this;
		}

		inline bool operator == (const Frustum& src) const
		{
			return memcmp(m_corners, src.m_corners, sizeof(m_corners)) == 0;
		}

		inline bool operator != (const Frustum& src) const
		{
			return memcmp(m_corners, src.m_corners, sizeof(m_corners)) != 0;
		}

		inline Vector3& operator [] (Corner corner)
		{
			return m_corners[corner];
		}

		inline const Vector3& operator [] (Corner corner) const
		{
			return m_corners[corner];
		}

		inline Plane& operator [] (FrustumPlane plane)
		{
			return m_planes[plane];
		}
	
		inline const Plane& operator [] (FrustumPlane plane) const
		{
			return m_planes[plane];
		}

	public:
		inline Vector3* getCorners() const
		{
			return m_corners;
		}
		
		inline bool isVisible(const Vector3& point) const
		{
			// Check if the point is inside all six planes of the view frustum.
			for(int i = 0; i < 6; ++i) 
			{
				if ((Vector3::Dot(m_planes[i].n, point) + m_planes[i].d) < 0.0f)
					return false;
			}

			return true;
		}

		// ¼ì²â
		bool isVisible(const Box& box) const;
		
		inline bool isVisible(const Sphere& sphere) const
		{
			// Check if the radius of the sphere is inside the view frustum.
			for(int i = 0; i < 6; ++i) 
			{
				if ((Vector3::Dot(m_planes[i].n, sphere.c) + m_planes[i].d) < -sphere.r)
					return false;
			}

			return true;
		}

		bool intersect(const Box& box) const;

		bool include(const Box& box) const;

	protected:
		mutable Vector3	m_corners[8];
		mutable Plane	m_planes[6];
	};

}

#endif