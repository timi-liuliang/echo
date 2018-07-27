#pragma once

#include "box2d_shape.h"

namespace Echo
{
	class Box2DShapeBox : public Box2DShape
	{
		ECHO_CLASS(Box2DShapeBox, Box2DShape)

	public:
		Box2DShapeBox();
		~Box2DShapeBox();

		// width
		float getWidth() const { return m_width; }
		void setWidth(float width) { m_width = width; }

		// height
		float getHeight() const { return m_height; }
		void setHeight(float height) { m_height = height; }

	protected:
		// get shape
		virtual b2Shape* getShape();

	private:
		float			m_width;
		float			m_height;
		b2PolygonShape*	m_shape;
	};
} 