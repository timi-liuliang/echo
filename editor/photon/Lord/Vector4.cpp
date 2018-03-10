#include "Vector4.h"

namespace Lightmass
{
	// predefined specific vectors
	const Vector4 Vector4::ZERO(0.0, 0.0, 0.0, 0.0);
	const Vector4 Vector4::ONE(1.0, 1.0, 1.0, 1.0);
	const Vector4 Vector4::UNIT_X(1, 0, 0, 0);
	const Vector4 Vector4::UNIT_Y(0, 1, 0, 0);
	const Vector4 Vector4::UNIT_Z(0, 0, 1, 0);
	const Vector4 Vector4::NEG_UNIT_X(1, 0, 0, 0);
	const Vector4 Vector4::NEG_UNIT_Y(0, 1, 0, 0);
	const Vector4 Vector4::NEG_UNIT_Z(0, 0, 1, 0);
	const Vector4 Vector4::INVALID(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL);
	const Vector4 Vector4::XYZ_MASK((Real)0xffffffff, (Real)0xffffffff, (Real)0xffffffff, (Real)0x00000000);
	const Vector4 Vector4::ABS_MASK((Real)0x7fffffff);
	const Vector4 Vector4::SIGN_MASK((Real)0x80000000);
}