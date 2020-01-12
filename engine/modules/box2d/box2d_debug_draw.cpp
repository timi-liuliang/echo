#include "box2d_debug_draw.h"
#include "engine/core/util/AssertX.h"
#include "engine/core/log/Log.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/math/Math.h"
#include "box2d_module.h"

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
        m_gizmosNode->queueFree();
    }

	void Box2DDebugDraw::Update(float elapsedTime)
	{
		m_gizmosNode->update(elapsedTime, false);
	}

	void Box2DDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{ 
		float pixelsPerMeter = Box2DModule::instance()->getPixelsPerMeter();
		for (i32 idx = 0; idx < vertexCount; idx++)
		{
			const b2Vec2& from = vertices[idx % vertexCount];
			const b2Vec2& to = vertices[(idx + 1) % vertexCount];
			m_gizmosNode->drawLine(Vector3( from.x, from.y, 0.f) * pixelsPerMeter, Vector3(to.x, to.y, 0.f) * pixelsPerMeter, Color(color.r, color.g, color.b, color.a));
		}
	}

	void Box2DDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
	{
		float pixelsPerMeter = Box2DModule::instance()->getPixelsPerMeter();
		for (i32 idx = 2; idx < vertexCount; idx++)
		{
			const b2Vec2& v0 = vertices[0];
			const b2Vec2& v1 = vertices[idx-1];
			const b2Vec2& v2 = vertices[idx];
			m_gizmosNode->drawTriangle(Vector3(v0.x, v0.y, 0.f) * pixelsPerMeter, Vector3(v1.x, v1.y, 0.f) * pixelsPerMeter, Vector3(v2.x, v2.y, 0.f) * pixelsPerMeter, Color(color.r, color.g, color.b, color.a * 0.7f));
		}
	}

	void Box2DDebugDraw::DrawCircle(const b2Vec2& b2Center, float32 radius, const b2Color& color)
	{
        float degreeStep = 10.f;
        float pixelsPerMeter = Box2DModule::instance()->getPixelsPerMeter();
        for (float degree = 0.f; degree < 360.f; degree+=degreeStep)
        {
            Quaternion quat0 = Quaternion::fromAxisAngle(Vector3::UNIT_Z, degree * Math::DEG2RAD);
            Quaternion quat1 = Quaternion::fromAxisAngle(Vector3::UNIT_Z, (degree + degreeStep) * Math::DEG2RAD);
            Vector3    dir0  = quat0.rotateVec3(Vector3::UNIT_X);
            Vector3    dir1  = quat1.rotateVec3(Vector3::UNIT_X);
            Vector3    center= Vector3(b2Center.x, b2Center.y, 0.f);
            Vector3    from  = center + dir0 * radius;
            Vector3    to    = center + dir1 * radius;
            m_gizmosNode->drawLine(from * pixelsPerMeter, to * pixelsPerMeter, Color(color.r, color.g, color.b, color.a));
        }
	}
    
	void Box2DDebugDraw::DrawSolidCircle(const b2Vec2& b2Center, float32 radius, const b2Vec2& axis, const b2Color& color)
	{
        float degreeStep = 10.f;
        float pixelsPerMeter = Box2DModule::instance()->getPixelsPerMeter();
        for (float degree = 0.f; degree < 360.f; degree+=degreeStep)
        {
            Quaternion quat0 = Quaternion::fromAxisAngle(Vector3::UNIT_Z, degree * Math::DEG2RAD);
            Quaternion quat1 = Quaternion::fromAxisAngle(Vector3::UNIT_Z, (degree + degreeStep) * Math::DEG2RAD);
            Vector3    dir0  = quat0.rotateVec3(Vector3::UNIT_X);
            Vector3    dir1  = quat1.rotateVec3(Vector3::UNIT_X);
            Vector3    center= Vector3(b2Center.x, b2Center.y, 0.f);
            Vector3    from  = center + dir0 * radius;
            Vector3    to    = center + dir1 * radius;
            
            m_gizmosNode->drawTriangle(center * pixelsPerMeter, from * pixelsPerMeter, to * pixelsPerMeter, Color(color.r, color.g, color.b, color.a * 0.7f));
        }
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
