#pragma once

#include <Box2D/Box2D.h>

namespace Echo
{
	class Gizmos;
	class Box2DDebugDraw : public b2Draw
	{
	public:
		Box2DDebugDraw();
        ~Box2DDebugDraw();

		// void step
		void Update(float elapsedTime);

		// Draw a closed polygon provided in CCW order.
		virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

		// Draw a solid closed polygon provided in CCW order.
		virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

		// Draw a circle.
		virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) override;

		// Draw a solid circle.
		virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) override;

		// Draw a line segment.
		virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

		// Draw a transform. Choose your own length scale.
		// @param xf a transform.
		virtual void DrawTransform(const b2Transform& xf) override;

	private:
		Gizmos*			m_gizmosNode;		// used for render, update by this b2Draw
	};
}
