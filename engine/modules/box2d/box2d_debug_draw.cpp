#include "box2d_debug_draw.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/Log.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/math/Math.h"
#include "box2d_world.h"

namespace Echo
{
	Box2DDebugDraw::Box2DDebugDraw()
	{
		m_gizmosNode = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmosNode->setAutoClear(true);
		m_gizmosNode->setLocalPosition(Vector3(0.f, 0.f, 100.f));
	}
    
    Box2DDebugDraw::~Box2DDebugDraw()
    {
        EchoSafeDelete(m_gizmosNode, Gizmos);
    }

	void Box2DDebugDraw::Update(float elapsedTime)
	{
		m_gizmosNode->update(elapsedTime, false);
	}

	void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{ 
		float pixelsPerMeter = Box2DWorld::instance()->getPixelsPerMeter();
		for (i32 idx = 0; idx < vertexCount; idx++)
		{
			const b2Vec2& from = vertices[idx % vertexCount];
			const b2Vec2& to = vertices[(idx + 1) % vertexCount];
			m_gizmosNode->drawLine(Vector3( from.x, from.y, 0.f) * pixelsPerMeter, Vector3(to.x, to.y, 0.f) * pixelsPerMeter, Color(color.r, color.g, color.b, color.a));
		}
	}

	void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		float pixelsPerMeter = Box2DWorld::instance()->getPixelsPerMeter();
		for (i32 idx = 2; idx < vertexCount; idx++)
		{
			const b2Vec2& v0 = vertices[0];
			const b2Vec2& v1 = vertices[idx-1];
			const b2Vec2& v2 = vertices[idx];
			m_gizmosNode->drawTriangle(Vector3(v0.x, v0.y, 0.f) * pixelsPerMeter, Vector3(v1.x, v1.y, 0.f) * pixelsPerMeter, Vector3(v2.x, v2.y, 0.f) * pixelsPerMeter, Color(color.r, color.g, color.b, color.a * 0.8f));
		}
	}

	void Box2DDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawCircle not implented");
	}
    
	void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawSolidCircle not implented");
	}

	void Box2DDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
	{
		EchoLogError("Box2DDebugDraw::DrawSegment not implented");
	}

	void Box2DDebugDraw::DrawTransform(const b2Transform& xf)
	{
		EchoLogError("Box2DDebugDraw::DrawTransform not implented");
	}
}
