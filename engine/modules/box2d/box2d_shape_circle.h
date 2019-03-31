#pragma once

#include "box2d_shape.h"

namespace Echo
{
	class Box2DShapeCircle : public Box2DShape
	{
		ECHO_CLASS(Box2DShapeCircle, Box2DShape)

	public:
		Box2DShapeCircle();
		~Box2DShapeCircle();

		// radius
		float getRadius() const;
		void setRadius(float radius);

	protected:
		// get shape
		virtual b2Shape* createb2Shape() override;

	private:
		float				m_radius;
	};
}
