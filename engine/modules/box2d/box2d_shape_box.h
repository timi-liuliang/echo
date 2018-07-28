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
		void setWidth(float width);

		// height
		float getHeight() const { return m_height; }
		void setHeight(float height);

	protected:
		// get shape
		virtual b2Shape* createb2Shape();

	private:
		float			m_width;
		float			m_height;
	};
} 