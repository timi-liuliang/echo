#include <gtest/gtest.h>
#include <engine/core/math/Vector4.h>
#include <engine/core/math/Math.h>

// range (-PI, PI)
static float Atan2(float angle)
{
	Echo::Vector2 dir = Echo::Vector2(sin(angle), cos(angle));
	float   result = atan2(dir.x, dir.y);

	return result;
}

static float Atan2Custom(float angle)
{
	Echo::Vector2 dir = Echo::Vector2(sin(angle), cos(angle));
	float   result = asin(dir.x) > 0 ? acos(dir.y) : -acos(dir.y);

	return result;
}

TEST(Atan2Test, Custom)
{
	using namespace Echo::Math;

	EXPECT_FLOAT_EQ(Atan2(0.0), Atan2Custom(0.0));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV8), Atan2Custom(PI_DIV8));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV6), Atan2Custom(PI_DIV6));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV5), Atan2Custom(PI_DIV5));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV4), Atan2Custom(PI_DIV4));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV3), Atan2Custom(PI_DIV3));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV2), Atan2Custom(PI_DIV2));

	EXPECT_FLOAT_EQ(Atan2(PI_DIV8 + PI_DIV2), Atan2Custom(PI_DIV8 + PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV6 + PI_DIV2), Atan2Custom(PI_DIV6 + PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV5 + PI_DIV2), Atan2Custom(PI_DIV5 + PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV4 + PI_DIV2), Atan2Custom(PI_DIV4 + PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(PI_DIV3 + PI_DIV2), Atan2Custom(PI_DIV3 + PI_DIV2));

	EXPECT_FLOAT_EQ(Atan2(-PI_DIV8), Atan2Custom(-PI_DIV8));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV6), Atan2Custom(-PI_DIV6));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV5), Atan2Custom(-PI_DIV5));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV4), Atan2Custom(-PI_DIV4));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV3), Atan2Custom(-PI_DIV3));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV2), Atan2Custom(-PI_DIV2));

	EXPECT_FLOAT_EQ(Atan2(-PI_DIV8 - PI_DIV2), Atan2Custom(-PI_DIV8 - PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV6 - PI_DIV2), Atan2Custom(-PI_DIV6 - PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV5 - PI_DIV2), Atan2Custom(-PI_DIV5 - PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV4 - PI_DIV2), Atan2Custom(-PI_DIV4 - PI_DIV2));
	EXPECT_FLOAT_EQ(Atan2(-PI_DIV3 - PI_DIV2), Atan2Custom(-PI_DIV3 - PI_DIV2));
}
