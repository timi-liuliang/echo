#include "box2d_debug_draw.h"
#include "engine/core/util/AssertX.h"

namespace Echo
{
	// Draw a closed polygon provided in CCW order.
	void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		int a = 10;
		EchoAssert(false);
	}

	// Draw a solid closed polygon provided in CCW order.
	void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		int a = 10;
		EchoAssert(false);
	}

	// Draw a circle.
	void Box2DDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		int a = 10;
		EchoAssert(false);
	}

	// Draw a solid circle.
	void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		int a = 10;
		EchoAssert(false);
	}

	// Draw a line segment.
	void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		int a = 10;
		EchoAssert(false);
	}

	// Draw a transform. Choose your own length scale.
	// @param xf a transform.
	void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
	{
		int a = 10;
		EchoAssert(false);
	}
}