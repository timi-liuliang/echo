#ifndef __HK_VECTOR_H__
#define __HK_VECTOR_H__

#include "Engine/Physics/PhysicsManager.h"

namespace Echo
{
	inline void hkVec4toVec3( const hkVector4& vec4, Vector3& vec3 )
	{
		vec3.x = vec4( 0 );
		vec3.y = vec4( 1 );
		vec3.z = vec4( 2 );
	}

	inline Vector3 hkVec4toVec3( const hkVector4& vec4 )
	{
		return Vector3( vec4(0), vec4(1), vec4(2) );
	}

	inline void Vec3tohkVec4( const Vector3& vec3, hkVector4& vec4 )
	{
		vec4.set( vec3.x, vec3.y, vec3.z );
	}

	inline hkVector4 Vec3tohkVec4( const Vector3& vec3 )
	{
		hkVector4 vec4;
		vec4.set( vec3.x, vec3.y, vec3.z );

		return vec4;
	}

	inline void hkQuatoQua( const hkQuaternion& hkQua, Quaternion& qua )
	{
		qua.x = hkQua( 0 );
		qua.y = hkQua( 1 );
		qua.z = hkQua( 2 );
		qua.w = hkQua( 3 );
	}

	inline Quaternion hkQuatoQua( const hkQuaternion& hkqua )
	{
		return Quaternion( hkqua( 3 ), hkqua( 0 ), hkqua( 1 ), hkqua( 2 ) );
	}
}

#endif