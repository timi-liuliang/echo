#include "Vector2.h"

namespace Lightmass
{
	// predefined specific vectors
	const Vector2 Vector2::ZERO(0, 0);
	const Vector2 Vector2::ONE(1, 1);
	const Vector2 Vector2::UNIT_X(1, 0);
	const Vector2 Vector2::UNIT_Y(0, 1);
	const Vector2 Vector2::NEG_UNIT_X(-1, 0);
	const Vector2 Vector2::NEG_UNIT_Y(0, -1);
	const Vector2 Vector2::INVALID(Math::MAX_REAL, Math::MAX_REAL);
}