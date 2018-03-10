#ifndef __ECHO_BOX_H__
#define __ECHO_BOX_H__

#include "Sphere.h"

namespace Echo
{
	/**
	\brief AABB
	*/
	class Box
	{
	public:
		enum Extent
		{
			EXTENT_NULL,
			EXTENT_FINITE,
			EXTENT_INFINITE
		};

		/*
		   4--------7
		  /|       /|
		 / |      / |
		0--------3  |
		|  6-----|--5
		| /      | /
		|/       |/
		2--------1

		vMin (6)
		vMax (3)
		*/

		enum Corner
		{
			CORNER_NLT		= 0,		//!< near left top
			CORNER_NRB		= 1,		//!< near right bottom
			CORNER_NLB		= 2,		//!< near left bottom
			CORNER_NRT		= 3,		//!< near right top

			CORNER_FLT		= 4,		//!< far left top
			CORNER_FRB		= 5,		//!< far right bottom
			CORNER_FLB		= 6,		//!< far left bottom
			CORNER_FRT		= 7,		//!< far right top
		};

	public:
		Vector3		vMin;		//!< Min Point
		Vector3		vMax;		//!< Max Point
		Extent		mExtent;

		static const Box ZERO;

	public:
		inline Box()
		{
			reset();
		}

		inline Box(const Vector3&_vMin, const Vector3&_vMax)
			:vMin(_vMin)
			,vMax(_vMax)
		{
		}

		inline Box(const Box &box)
			:vMin(box.vMin)
			,vMax(box.vMax)
		{
		}

		inline Box(Real minX, Real minY, Real minZ, Real maxX, Real maxY, Real maxZ)
			:vMin(minX, minY, minZ)
			,vMax(maxX, maxY, maxZ)
		{
		}

	public:
		inline const Box& operator = (const Box& rhs)
		{
			vMin = rhs.vMin;
			vMax = rhs.vMax;
			return *this;
		}

		inline bool operator == (const Box& rhs) const
		{
			return (this->vMin == rhs.vMin) && (this->vMax == rhs.vMax);
		}

		inline bool operator != (const Box& rhs) const
		{
			return !(*this == rhs);
		}

	public:
		inline void reset()
		{
			vMin.x = vMin.y = vMin.z =  1e30f;
			vMax.x = vMax.y = vMax.z = -1e30f;// Math::MAX_REAL容易出现计算溢出，放弃使用
		}

		// 添加点
		inline void addPoint(const Vector3 &point)
		{
			vMax.x = Math::Max(point.x,vMax.x);
			vMax.y = Math::Max(point.y,vMax.y);
			vMax.z = Math::Max(point.z,vMax.z);

			vMin.x = Math::Min(point.x,vMin.x);
			vMin.y = Math::Min(point.y,vMin.y);
			vMin.z = Math::Min(point.z,vMin.z);
		}

		// 融合box
		inline void unionBox(const Box &box)
		{
			if (box.isValid())
			{
				Vector3::Max( vMax, vMax, box.vMax);
				Vector3::Min( vMin, vMin, box.vMin);
			}
		}
		
		inline bool isIntersected(const Box &box) const
		{
#ifdef ECHO_EDITOR_MODE
			EchoAssert(isValid());
			EchoAssert(box.isValid());
#endif

			if (box.vMax.x < vMin.x)
				return false;
			else if(box.vMax.y < vMin.y)
				return false;
			else if(box.vMax.z < vMin.z)
				return false;

			else if(box.vMin.x > vMax.x)
				return false;
			else if(box.vMin.y > vMax.y)
				return false;
			else if(box.vMin.z > vMax.z)
				return false;

			return true;
		}

		// 是否在内部
		bool isContain( const Box& box) const
		{
			if( isPointInside( box.vMin) && isPointInside( box.vMax))
				return true;

			return false;
		}

		inline void fromBSphere(const Sphere &bs)
		{
			vMin.x = bs.c.x - bs.r;
			vMin.y = bs.c.y - bs.r;
			vMin.z = bs.c.z - bs.r;

			vMax.x = bs.c.x + bs.r;
			vMax.y = bs.c.y + bs.r;
			vMax.z = bs.c.z + bs.r;
		}

		inline Vector3 getCorner(Corner corner) const
		{
			switch(corner)
			{
			case CORNER_NLT:
				return Vector3(vMin.x, vMax.y, vMax.z);
			case CORNER_NRB:
				return Vector3(vMax.x, vMin.y, vMax.z);
			case CORNER_NLB:
				return Vector3(vMin.x, vMin.y, vMax.z);
			case CORNER_NRT:
				return vMax;
			case CORNER_FLT:
				return Vector3(vMin.x, vMax.y, vMin.z);
			case CORNER_FRB:
				return Vector3(vMax.x, vMin.y, vMin.z);
			case CORNER_FLB:
				return vMin;
			case CORNER_FRT:
				return Vector3(vMax.x, vMax.y, vMin.z);
			default:
				return Vector3::INVALID;
			}
		}

		inline Vector3 getSize() const
		{
			return (vMax - vMin);
		}

		inline bool isValid() const 
		{
			return getDX()>=0.f && getDY()>=0.f && getDZ()>=0.f;
		}

		inline bool isPointInside(const Vector3& v) const
		{
			return v.x > vMin.x && v.x < vMax.x
				&& v.y > vMin.y && v.y < vMax.y
				&& v.z > vMin.z && v.z < vMax.z;
		}

		inline void move(const Vector3& offset)
		{
			vMin += offset;
			vMax += offset;
		}

		inline Real getDX() const 
		{
			return vMax.x - vMin.x;
		}

		inline Real getDY() const
		{
			return vMax.y - vMin.y;
		}

		inline Real getDZ() const
		{
			return vMax.z - vMin.z;
		}

		inline Vector3 getCenter() const
		{
			return (vMin + vMax)*0.5f;
		}

		inline Real getMaxEdgeLen() const
		{
			return Math::Max3(getDX(), getDY(), getDZ());
		}

		// 矩阵转换
		inline Box transform( const Matrix4& matrix) const
		{
			Box box;
			if (isValid())
			{
				Vector3 basePoint(vMin);
				Vector3 xLeg(vMax.x, vMin.y, vMin.z);
				Vector3 yLeg(vMin.x, vMax.y, vMin.z);
				Vector3 zLeg(vMin.x, vMin.y, vMax.z);

				basePoint = basePoint * matrix;
				xLeg = xLeg * matrix;
				yLeg = yLeg * matrix;
				zLeg = zLeg * matrix;

				xLeg -= basePoint;
				yLeg -= basePoint;
				zLeg -= basePoint;

				Vector3 farPoint(basePoint + xLeg + yLeg + zLeg);

				box.vMin = basePoint;
				box.vMax = farPoint;

				box.addPoint(basePoint + xLeg);
				box.addPoint(basePoint + yLeg);
				box.addPoint(basePoint + zLeg);
				box.addPoint(basePoint + xLeg + yLeg);
				box.addPoint(basePoint + yLeg + zLeg);
				box.addPoint(basePoint + zLeg + xLeg);
			}
	
			return box;
		}

		inline Real getDiagonalLenSqr() const
		{
			Real dx = getDX();
			Real dy = getDY();
			Real dz = getDZ();
			return dx*dx + dy*dy + dz*dz;
		}

		inline Real getDiagonalLen() const 
		{
			return Math::Sqrt(getDiagonalLenSqr());
		}

		inline void inflate(Real x, Real y, Real z) 
		{
			vMin -= Vector3(x, y, z);
			vMax += Vector3(x, y, z);
		}

		inline void deflate(Real x, Real y, Real z)
		{
			vMin += Vector3(x, y, z);
			vMax -= Vector3(x, y, z);
		}

		inline CullState	cullAABB(const Box& box) const
		{
			if (!isIntersected(box))
				return CS_OUTSIDE;
			else if (isPointInside(box.vMin) && isPointInside(box.vMax))
				return CS_INSIDE;
			else
				return CS_INTERSECT;
		}
		static Box fromString(const String& val);
		String& toString(String& s) const;

		// 扩展
		void expandBy( float w)
		{
			vMin -= Vector3(w, w, w);
			vMax += Vector3(w, w, w);
		}
	};
}

#endif
