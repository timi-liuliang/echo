#include "box2d_debug_draw.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
	Box2DDebugDraw::Box2DDebugDraw()
	{
		m_gizmosNode = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmosNode->setAutoClear(true);
	}

	// void step
	void Box2DDebugDraw::Update(float elapsedTime)
	{
		m_gizmosNode->update(elapsedTime, false);
	}

	// Draw a closed polygon provided in CCW order.
	void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawPolygon not implented");
	}

	// Draw a solid closed polygon provided in CCW order.
	void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawSolidPolygon not implented");
	}

	// Draw a circle.
	void Box2DDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawCircle not implented");
	}

	// Draw a solid circle.
	void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawSolidCircle not implented");
	}

	// Draw a line segment.
	void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawSegment not implented");
	}

	// Draw a transform. Choose your own length scale.
	// @param xf a transform.
	void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
	{
		EchoLogError("Box2DDebugDraw::DrawTransform not implented");
	}
}