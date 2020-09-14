#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include <QPainter>

namespace Procedural
{
	class QGraphicsRoundRectItem : public QGraphicsRectItem
	{
	public:
		QGraphicsRoundRectItem(QGraphicsItem* parent = nullptr)
			: QGraphicsRectItem(parent)
		{

		}

		// custom paint
		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
		{
			painter->drawRoundedRect(rect(), m_radius, m_radius);
		}

		// set radisu
		void setRadius(float radius) { m_radius = radius; }

	protected:
		float	m_radius = 3.f;
	};
}

#endif
