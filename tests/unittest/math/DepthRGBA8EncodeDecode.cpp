#include <gtest/gtest.h>
#include <engine/core/math/Vector4.h>
#include <engine/core/math/Math.h>

namespace Echo
{
	float DecodeRgbaToFloat(Vector4 rgba)
	{
		return rgba.dot(Vector4(1.0, 1.0/255.0, 1.0/65025.0, 1.0/16581375.0));
	}

	Vector4 EncodeFloatToRgba( float depth)
	{
		Vector4 enc = Vector4(1.0, 255.0, 65025.0, 16581375.0) * Math::Clamp(depth, 0.0f, 0.9999f);
		enc.fract();
		enc -= Vector4(enc.y, enc.z, enc.w, enc.w) * Vector4(1.0 /255.0, 1.0/255.0, 1.0/255.0, 0.0);
		return enc;
	}
}

TEST(DepthRGBA8EncodeDecode, encode)
{
	using namespace Echo;

	Vector4 color0 = EncodeFloatToRgba(0.0);
	Vector4 color1 = EncodeFloatToRgba(0.1);
	Vector4 color2 = EncodeFloatToRgba(0.3);
	Vector4 color3 = EncodeFloatToRgba(0.5);
	Vector4 color4 = EncodeFloatToRgba(0.7);
	Vector4 color5 = EncodeFloatToRgba(0.9);
	Vector4 color6 = EncodeFloatToRgba(1.0);

	float depth0 = DecodeRgbaToFloat(color0);
	float depth1 = DecodeRgbaToFloat(color1);
	float depth2 = DecodeRgbaToFloat(color2);
	float depth3 = DecodeRgbaToFloat(color3);
	float depth4 = DecodeRgbaToFloat(color4);
	float depth5 = DecodeRgbaToFloat(color5);
	float depth6 = DecodeRgbaToFloat(color6);

	EXPECT_FLOAT_EQ(depth0, 0.0);
	EXPECT_FLOAT_EQ(depth1, 0.1);
	EXPECT_FLOAT_EQ(depth2, 0.3);
	EXPECT_FLOAT_EQ(depth3, 0.5);
	EXPECT_FLOAT_EQ(depth4, 0.7);
	EXPECT_FLOAT_EQ(depth5, 0.9);
	EXPECT_FLOAT_EQ(depth6, 0.9999);
}
