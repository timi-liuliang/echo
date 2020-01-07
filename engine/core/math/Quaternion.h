#pragma once

#include "Matrix4.h"

namespace Echo
{
	/**
	\brief Quaternion
	*/
	class Vector3;
	class Vector4;
	class Quaternion
	{
	public:
		union
		{
			struct 
			{
				Real x, y, z, w;
			};

			Real m[4];
		};

		static const Quaternion IDENTITY;		//!< Quan(0, 0, 0, 1)
		static const Quaternion INVALID;		//!< Quan(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL)

	public:
		inline Quaternion()
			: x(0)
			, y(0)
			, z(0)
			, w(1)
		{}

		inline Quaternion(const Quaternion& quan)
			: x(quan.x)
			, y(quan.y)
			, z(quan.z)
			, w(quan.w)
		{}

		inline Quaternion(const Real *pf)
		{
			EchoAssertX(pf != NULL, "The array data is NULL.");
			x = pf[0];
			y = pf[1];
			z = pf[2];
			w = pf[3];
		}

		inline Quaternion(Real fx, Real fy, Real fz, Real fw)
			: x(fx)
			, y(fy)
			, z(fz)
			, w(fw)
		{
		}

		inline Quaternion(Real yaw, Real pitch, Real roll)
		{
			// NOTE: arguments order
			fromEulerAngle(pitch, yaw, roll);
		}

		inline Quaternion(const Vector3& vAixs, Real radian)
		{
			fromAxisAngle(vAixs, radian);
		}

		inline Real& operator[] (int index)
		{
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		inline const Real& operator[] (int index) const
		{
			EchoAssertX(index >= 0 && index < 4, "Access out of bounds");
			return m[index];
		}

		inline Quaternion& operator= (const Quaternion& quan)
		{
			x = quan.x;
			y = quan.y;
			z = quan.z;
			w = quan.w;

			return *this;
		}

		inline operator Real* ()
		{
			return m;
		}

		inline operator const Real* () const
		{
			return m;
		}

		inline Quaternion& operator += (const Quaternion& q)
		{
			w += q.w;
			x += q.x;
			y += q.y;
			z += q.z;

			return *this;
		}

		inline Quaternion& operator -= (const Quaternion& q)
		{
			w -= q.w;
			x -= q.x;
			y -= q.y;
			z -= q.z;

			return *this;
		}

		inline Quaternion& operator *= (const Quaternion& q)
		{
			float x0 = x;
			float y0 = y;
			float z0 = z;
			float w0 = w;
			x = w0 * q.x + x0 * q.w + z0 * q.y - y0 * q.z;
			y = w0 * q.y + y0 * q.w + x0 * q.z - z0 * q.x;
			z = w0 * q.z + z0 * q.w + y0 * q.x - x0 * q.y;
			w = w0 * q.w - x0 * q.x - y0 * q.y - z0 * q.z;

			return *this;
		}

		inline Quaternion& operator *= (Real f)
		{
			x *= f;
			y *= f;
			z *= f;
			w *= f;

			return *this;
		}

		inline Quaternion& operator /= (Real f)
		{
			Real fInv = 1.0f / f;
			x *= fInv;
			y *= fInv;
			z *= fInv;
			w *= fInv;

			return *this;
		}

		inline Real* ptr()
		{
			return &x;
		}

		inline const Real* ptr() const
		{
			return &x;
		}

		inline const Quaternion& operator + () const
		{
			return *this;
		}

		inline Quaternion operator - () const
		{
			return Quaternion(-x, -y, -z, -w);
		}

		inline bool operator == (const Quaternion& b) const
		{
			return (w==b.w && x==b.x && y==b.y && z==b.z);
		}

		inline bool operator != (const Quaternion& b) const
		{
			return !(w==b.w && x==b.x && y==b.y && z==b.z);
		}

		inline Quaternion operator + (const Quaternion& b) const
		{
			Quaternion quan;

			quan.x = x + b.x;
			quan.y = y + b.y;
			quan.z = z + b.z;
			quan.w = w + b.w;

			return quan;
		}

		inline Quaternion operator- (const Quaternion& b) const
		{
			Quaternion quan;

			quan.x = x - b.x;
			quan.y = y - b.y;
			quan.z = z - b.z;
			quan.w = w - b.w;

			return quan;
		}

		inline friend Quaternion operator* (const Real f, const Quaternion& q)
		{
			Quaternion result;

			result.x = f * q.x;
			result.y = f * q.y;
			result.z = f * q.z;
			result.w = f * q.w;

			return result;
		}

		inline Quaternion operator* (const Real f) const
		{
			Quaternion result;

			result.x = f * x;
			result.y = f * y;
			result.z = f * z;
			result.w = f * w;

			return result;
		}

		inline Quaternion operator* (const Quaternion& b) const
		{
			Quaternion quan;
			quan.x = w * b.x + x * b.w + y * b.z - z *b.y;
			quan.y = w * b.y + y * b.w + z * b.x - x *b.z;
			quan.z = w * b.z + z * b.w + x * b.y - y *b.x;
			quan.w = w * b.w - x * b.x - y * b.y - z * b.z;

			return quan;
		}

		inline Vector3 operator* (const Vector3& v) const
		{
			// nVidia SDK implementation
			Vector3 uv, uuv;
			Vector3 qvec(x, y, z);
			uv = qvec.cross(v);
			uuv = qvec.cross(uv);
			uv *= 2.0f * w;
			uuv *= 2.0f;

			return v + uv + uuv;
		}

		inline Vector4 operator* (const Vector4& v) const
		{
			// nVidia SDK implementation
			Vector3 vec(v);
			Vector3 uv, uuv;
			Vector3 qvec(x, y, z);
			uv = qvec.cross(vec);
			uuv = qvec.cross(uv);
			uv *= 2.0f * w;
			uuv *= 2.0f;

			return Vector4(vec + uv + uuv, 1.0f);
		}

		inline Quaternion operator/ (const Real f) const
		{
			Quaternion quan;
			Real fInv = 1.0f / f;

			quan.x = x * fInv;
			quan.y = y * fInv;
			quan.z = z * fInv;
			quan.w = w * fInv;

			return quan;
		}

		inline friend Quaternion operator/ (const Real f, const Quaternion& a)
		{
			Quaternion quan;

			quan.x = f / a.x;
			quan.y = f / a.y;
			quan.z = f / a.z;
			quan.w = f / a.w;

			return quan;
		}

		inline void zero()
		{
			w = 0.0;
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		inline void invalid()
		{
			*this = INVALID;
		}

		inline void set(Real w, Real x, Real y, Real z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		inline void set(Real value)
		{
			this->x = value;
			this->y = value;
			this->z = value;
			this->w = value;
		}

		inline void set(Real* p)
		{
			this->x = p[1];
			this->y = p[2];
			this->z = p[3];
			this->w = p[0];
		}

		inline Real dot(const Quaternion& rhs) const
		{
			return (w * rhs.w + x * rhs.x + y * rhs.y + z * rhs.z);
		}

		inline Quaternion cross(const Quaternion& rhs)
		{
			Quaternion quan;
			quan.x = w * rhs.x + x * rhs.w + y * rhs.z - z *rhs.y;
			quan.y = w * rhs.y + y * rhs.w + z * rhs.x - x *rhs.z;
			quan.z = w * rhs.z + z * rhs.w + x * rhs.y - y *rhs.x;
			quan.w = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;

			return quan;
		}

		inline void conjugate()
		{
			x = -x;
			y = -y;
			z = -z;
		}

		inline void inverse()
		{
			//conjugate();
			const Real norm = lenSqr();

			if(norm == 0.0)
			{
				invalid();
				return;
			}

			Real invNorm = 1 / norm;
			x *= -invNorm;
			y *= -invNorm;
			z *= -invNorm;
			w *= invNorm;
		}

		inline Real len() const
		{
			Real qLen;

			Real sum = w * w + x * x + y * y + z * z;
			qLen = Math::Sqrt(sum);

			return qLen;
		}

		inline Real lenSqr() const
		{
			return w * w + x * x + y * y + z * z;
		}

		inline void identity()
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
			w = 1.0;
		}

		inline Quaternion log() const
		{
			Real a = Math::ACos(w);
			Real sina = Math::Sin(a);

			Quaternion quat;
			quat.w = 0;

			if (sina > 0)
			{
				quat.x = a * x / sina;
				quat.y = a * y / sina;
				quat.z = a * z / sina;
			}
			else
				quat.x = quat.y = quat.z = 0;

			return quat;
		}

		inline Quaternion exp() const
		{
			Real a = len();
			Real sina = Math::Sin(a);
			Real cosa = Math::Cos(a);

			Quaternion quan;
			quan.w = cosa;

			if(a > 0)
			{
				quan.x = sina * x / a;
				quan.y = sina * y / a;
				quan.z = sina * z / a;
			}
			else
				quan.x = quan.y = quan.z = 0;

			return quan;
		}

		inline Quaternion pow(Real fExp) const
		{
			if(Math::Abs(w) >= 1.0)
				return *this;

			// alpha = theta / 2
			Real alpha = Math::ACos(w);
			Real newAlpha = alpha * fExp;

			// calculate w
			Quaternion quan;
			quan.w = Math::Cos(newAlpha);

			// calculate xyz
			Real mult = Math::Sin(newAlpha) / Math::Sin(alpha);
			quan.x = x * mult;
			quan.y = y * mult;
			quan.z = z * mult;

			return quan;
		}

		inline void normalize()
		{
			Real length = len();
			if (length > 1e-5f)
			{
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}
		}


		inline void toMat4(Matrix4& mat) const
		{
			Real xs = x * 2.0f;
			Real ys = y * 2.0f;
			Real zs = z * 2.0f;
			Real wx = w * xs;
			Real wy = w * ys;
			Real wz = w * zs;
			Real xx = x * xs;
			Real xy = x * ys;
			Real xz = x * zs;
			Real yy = y * ys;
			Real yz = y * zs;
			Real zz = z * zs;

			mat.m00 = 1.0f - (yy + zz);	mat.m01 = xy + wz;			mat.m02 = xz - wy;			mat.m03 = 0.0f;
			mat.m10 = xy - wz;			mat.m11 = 1.0f - (xx + zz);	mat.m12 = yz + wx;			mat.m13 = 0.0f;
			mat.m20 = xz + wy;			mat.m21 = yz - wx;			mat.m22 = 1.0f - (xx + yy);	mat.m23 = 0.0f;
			mat.m30 = 0.0f;				mat.m31 = 0.0f;				mat.m32 = 0.0f;				mat.m33 = 1.0f;
		}


		inline void fromMat4(const Matrix4& mat)
		{
			Real fTrace = mat.m00 + mat.m11 + mat.m22;
			Real fRoot;

			if (fTrace > 0.0)
			{
				// |w| > 1/2, may as well choose w > 1/2
				fRoot = Math::Sqrt(fTrace + 1.0f);  // 2w
				w = 0.5f * fRoot;
				fRoot = 0.5f / fRoot;  // 1/(4w)
				x = (mat.m12 - mat.m21) * fRoot;
				y = (mat.m20 - mat.m02) * fRoot;
				z = (mat.m01 - mat.m10) * fRoot;
			}
			else
			{
				// |w| <= 1/2
				static int nNext[3] = {1, 2, 0};
				int i = 0;
				if (mat.m11 > mat.m00)
					i = 1;
				if (mat.m22 > mat(i, i))
					i = 2;
				int j = nNext[i];
				int k = nNext[j];

				fRoot = Math::Sqrt(mat(i, i) - mat(j, j) - mat(k, k) + 1.0f);
				Real* nQuan[3] = { &x, &y, &z };
				*nQuan[i] = 0.5f * fRoot;
				fRoot = 0.5f / fRoot;
				w = (mat[j][k] - mat[k][j]) * fRoot;
				*nQuan[j] = (mat(i, j) + mat(j, i)) * fRoot;
				*nQuan[k] = (mat(i, k) + mat(k, i)) * fRoot;
			}
		}

		static Quaternion fromVec3ToVec3(const Vector3& from, const Vector3& to);

		inline void fromAxes(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis)
		{
			Matrix4 kRot = Matrix4::IDENTITY;
			kRot.m00 = xAxis.x;
			kRot.m01 = xAxis.y;
			kRot.m02 = xAxis.z;

			kRot.m10 = yAxis.x;
			kRot.m11 = yAxis.y;
			kRot.m12 = yAxis.z;

			kRot.m20 = zAxis.x;
			kRot.m21 = zAxis.y;
			kRot.m22 = zAxis.z;

			Real fTrace = kRot.m00+kRot.m11+kRot.m22;
			Real fRoot;

			if ( fTrace > 0.0 )
			{
				// |w| > 1/2, may as well choose w > 1/2
				fRoot = Math::Sqrt(fTrace + 1.0f);  // 2w
				w = 0.5f*fRoot;
				fRoot = 0.5f/fRoot;  // 1/(4w)
				x = (kRot.m12-kRot.m21)*fRoot;
				y = (kRot.m20-kRot.m02)*fRoot;
				z = (kRot.m01-kRot.m10)*fRoot;
			}
			else
			{
				// |w| <= 1/2
				static ui32 s_iNext[3] = { 1, 2, 0 };
				ui32 i = 0;
				if ( kRot.m11 > kRot.m00 )
					i = 1;
				if ( kRot.m22 > kRot[i][i] )
					i = 2;
				ui32 j = s_iNext[i];
				ui32 k = s_iNext[j];

				fRoot = Math::Sqrt(kRot[i][i]-kRot[j][j]-kRot[k][k] + 1.0f);
				Real* apkQuat[3] = { &x, &y, &z };
				*apkQuat[i] = 0.5f*fRoot;
				fRoot = 0.5f/fRoot;
				w = (kRot[j][k]-kRot[k][j])*fRoot;
				*apkQuat[j] = (kRot[j][i]+kRot[i][j])*fRoot;
				*apkQuat[k] = (kRot[k][i]+kRot[i][k])*fRoot;
			}
		}
        
        // rotate direction
        Vector3 rotateVec3( const Vector3& vec) const;

		inline void diff(const Quaternion& q1, const Quaternion& q2)
		{
			Quaternion invQ1 = q1;
			invQ1.inverse();
			*this = invQ1 * q2;
		}

		inline Quaternion& abs()
		{
			x = Math::Abs(x);
			y = Math::Abs(y);
			z = Math::Abs(z);
			w = Math::Abs(w);

			return *this;
		}

		inline Quaternion& neg()
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;

			return *this;
		}

		inline bool isInvalid() const
		{
			return Math::IsInvalid(w) || Math::IsInvalid(x) || Math::IsInvalid(y) || Math::IsInvalid(z);
		}

		inline Vector3 getAxis() const
		{
			return Vector3(x, y, z);
		}

		inline Vector3 xAxis() const 
		{
			Real fTy  = 2.0f*y;
			Real fTz  = 2.0f*z;
			Real fTwy = fTy*w;
			Real fTwz = fTz*w;
			Real fTxy = fTy*x;
			Real fTxz = fTz*x;
			Real fTyy = fTy*y;
			Real fTzz = fTz*z;

			return Vector3(1.0f-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy);
		}

		inline Vector3 yAxis() const 
		{
			Real fTx  = 2.0f*x;
			Real fTy  = 2.0f*y;
			Real fTz  = 2.0f*z;
			Real fTwx = fTx*w;
			Real fTwz = fTz*w;
			Real fTxx = fTx*x;
			Real fTxy = fTy*x;
			Real fTyz = fTz*y;
			Real fTzz = fTz*z;

			return Vector3(fTxy-fTwz, 1.0f-(fTxx+fTzz), fTyz+fTwx);
		}

		inline Vector3 zAxis() const
		{
			Real fTx  = 2.0f*x;
			Real fTy  = 2.0f*y;
			Real fTz  = 2.0f*z;
			Real fTwx = fTx*w;
			Real fTwy = fTy*w;
			Real fTxx = fTx*x;
			Real fTxz = fTz*x;
			Real fTyy = fTy*y;
			Real fTyz = fTz*y;

			return Vector3(fTxz+fTwy, fTyz-fTwx, 1.0f-(fTxx+fTyy));
		}

		inline Real getRadian() const
		{
			EchoAssertX(w >= -1.0 && w <= 1.0, "The quaternion is not identity.");	
			return Math::ACos(w) * 2.0f;
		}

		static Quaternion fromAxisAngle(const Vector3& axis, Real radian)
		{
			Quaternion result;
			// assert:  axis is unit length
			// The quaternion representing the rotation is
			//   q = cos(A / 2) + sin(A / 2 ) * (x * i + y * j + z * k)

			Real alpha = 0.5f * radian;
			Real sina = Math::Sin(alpha);
			result.w = Math::Cos(alpha);
			result.x = sina * axis.x;
			result.y = sina * axis.y;
			result.z = sina * axis.z;

			return result;
		}

		inline void fromEulerAngle(Real pitch, Real yaw, Real roll)
		{
			float fCosHRoll = Math::Cos(roll * Math::DEG2RAD * 0.5f);
			float fSinHRoll = Math::Sin(roll * Math::DEG2RAD * 0.5f);
			float fCosHPitch = Math::Cos(pitch * Math::DEG2RAD * 0.5f);
			float fSinHPitch = Math::Sin(pitch * Math::DEG2RAD * 0.5f);
			float fCosHYaw = Math::Cos(yaw * Math::DEG2RAD * 0.5f);
			float fSinHYaw = Math::Sin(yaw * Math::DEG2RAD * 0.5f);

			w = fCosHRoll * fCosHPitch * fCosHYaw + fSinHRoll * fSinHPitch * fSinHYaw;
			x = fCosHRoll * fSinHPitch * fCosHYaw + fSinHRoll * fCosHPitch * fSinHYaw;
			y = fCosHRoll * fCosHPitch * fSinHYaw - fSinHRoll * fSinHPitch * fCosHYaw;
			z = fSinHRoll * fCosHPitch * fCosHYaw - fCosHRoll * fSinHPitch * fSinHYaw;
		}

		inline void toEulerAngle(Real& pitch, Real& yaw, Real& roll) const
		{
			roll = Math::ATan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (z * z + x * x));
			pitch = Math::ASin(2.0f * (w * x - y * z));
			yaw = Math::ATan2(2.0f * (w * y + z * x), 1.0f - 2.0f * (x * x + y * y));

			pitch = pitch * Math::RAD2DEG;
			yaw = yaw * Math::RAD2DEG;
			roll = roll * Math::RAD2DEG;
		}

	public:
		static inline Quaternion Log(const Quaternion& quan)
		{
			Real a = Math::ACos(quan.w);
			Real sina = Math::Sin(a);

			Quaternion q;
			q.w = 0;

			if (sina > 0)
			{
				q.x = a * quan.x / sina;
				q.y = a * quan.y / sina;
				q.z = a * quan.z / sina;
			}
			else
				q.x = q.y = q.z = 0;

			return q;
		}

		static inline Quaternion Exp(const Quaternion& quan)
		{
			Real a = quan.len();
			Real sina = Math::Sin(a);
			Real cosa = Math::Cos(a);

			Quaternion q;
			q.w = cosa;

			if(a > 0)
			{
				q.x = sina * quan.x / a;
				q.y = sina * quan.y / a;
				q.z = sina * quan.z / a;
			}
			else
				q.x = q.y = q.z = 0;

			return q;
		}

		static inline Quaternion Pow(const Quaternion& quan, Real fExp)
		{
			if(Math::Abs(quan.w) >= 1.0)
				return quan;

			// alpha = theta / 2
			Real alpha = Math::ACos(quan.w);
			Real newAlpha = alpha * fExp;

			// calculate w
			Quaternion q;
			q.w = Math::Cos(newAlpha);

			// calculate xyz
			Real mult = Math::Sin(newAlpha) / Math::Sin(alpha);
			q.x = quan.x * mult;
			q.y = quan.y * mult;
			q.z = quan.z * mult;

			return q;
		}

		/** Performs Normalised linear interpolation between two quaternions, and returns the result.
		Lerp ( 0.0f, A, B ) = A
		Lerp ( 1.0f, A, B ) = B
		@remarks
		Nlerp is faster than Slerp.
		Nlerp has the proprieties of being commutative (@see Slerp;
		commutativity is desired in certain places, like IK animation), and
		being torque-minimal (unless shortestPath=false). However, it's performing
		the interpolation at non-constant velocity; sometimes this is desired,
		sometimes it is not. Having a non-constant velocity can produce a more
		natural rotation feeling without the need of tweaking the weights; however
		if your scene relies on the timing of the rotation or assumes it will point
		at a specific angle at a specific weight value, Slerp is a better choice.
		*/
		static inline void Lerp(Quaternion& quan, const Quaternion& q1, const Quaternion& q2, Real t, bool bShortestPath = false)
		{
			Real fCos = q1.dot(q2);
			if (fCos < 0.0f && bShortestPath)
			{
				quan = q1 + t * ((-q2) - q1);
			}
			else
			{
				quan = q1 + t * (q2 - q1);
			}

			quan.normalize();
		}


		/** Performs Spherical linear interpolation between two quaternions, and returns the result.
		Slerp ( 0.0f, A, B ) = A
		Slerp ( 1.0f, A, B ) = B
		@return Interpolated quaternion
		@remarks
		Slerp has the proprieties of performing the interpolation at constant
		velocity, and being torque-minimal (unless shortestPath=false).
		However, it's NOT commutative, which means
		Slerp ( 0.75f, A, B ) != Slerp ( 0.25f, B, A );
		therefore be careful if your code relies in the order of the operands.
		This is specially important in IK animation.
		*/
		static void		Slerp(Quaternion &quan, const Quaternion &q1, const Quaternion &q2, Real t, bool bShortestPath = false);

		// spherical quadratic interpolation
		static void		Squad(Quaternion &quan, const Quaternion &q1,const Quaternion &q2, const Quaternion &a, const Quaternion &b, Real t, bool bShortestPath = false);

		void			Spline(Quaternion &quan, const Quaternion &q1,const Quaternion &q2, const Quaternion &q3);
	};
}
