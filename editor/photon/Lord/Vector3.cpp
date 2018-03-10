#include "Vector3.h"

namespace Lightmass
{
	// predefined specific vectors
	const Vector3 Vector3::ZERO(0, 0, 0);
	const Vector3 Vector3::ONE(1, 1, 1);
	const Vector3 Vector3::UNIT_X(1, 0, 0);
	const Vector3 Vector3::UNIT_Y(0, 1, 0);
	const Vector3 Vector3::UNIT_Z(0, 0, 1);
	const Vector3 Vector3::NEG_UNIT_X(-1, 0, 0);
	const Vector3 Vector3::NEG_UNIT_Y(0, -1, 0);
	const Vector3 Vector3::NEG_UNIT_Z(0, 0, -1);
	const Vector3 Vector3::INVALID(Math::MAX_REAL, Math::MAX_REAL, Math::MAX_REAL);
}