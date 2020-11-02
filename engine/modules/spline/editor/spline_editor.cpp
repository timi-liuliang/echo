#include "spline_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplineEditor::SplineEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("3d");
	}

	SplineEditor::~SplineEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr SplineEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline.png");
	}

	void SplineEditor::onEditorSelectThisNode()
	{
	}

	void SplineEditor::editor_update_self()
	{
		m_gizmo->clear();

		Spline* spline = ECHO_DOWN_CAST<Spline*>(m_object);
		if (spline)
		{
			const float pointPixels = 18.f;

			// points
			for (SplinePoint* point : spline->getPoints())
			{
				m_gizmo->drawPoint(point->getWorldPosition(), point->getEditor()->isSelected() ? Color::BLUE : Color(0.5f, 0.5f, 0.5f, 1.f), pointPixels, 4);
			}

			// segments
			for (SplineSegment* segment : spline->getSegments())
			{
				SplinePoint* pointA = spline->getPoint(segment->getEndPointA());
				SplinePoint* pointB = spline->getPoint(segment->getEndPointB());
				SplineControlPoint* controlPointA = segment->getControlPointA();
				SplineControlPoint* controlPointB = segment->getControlPointB();
				if (pointA && pointB)
				{
					Color segmentColor = segment->getEditor()->isSelected() ? Color::BLUE : Color(1.f, 1.f, 1.f, 0.5f);

					if (controlPointA && controlPointB)
					{
						i32 segments = (pointB->getWorldPosition() - pointA->getWorldPosition()).len() / 0.1f;
						for (i32 i = 0; i < segments; i++)
						{
							Vector3 startPos, endPos;
							Bezier3(startPos, pointA->getWorldPosition(), controlPointA->getWorldPosition(), controlPointB->getWorldPosition(), pointB->getWorldPosition(), float(i) / segments);
							Bezier3(endPos,   pointA->getWorldPosition(), controlPointA->getWorldPosition(), controlPointB->getWorldPosition(), pointB->getWorldPosition(), float(i + 1) / segments);
							
							m_gizmo->drawLine(startPos, endPos, segmentColor);
						}

						if (m_showControlPoint)
						{
							m_gizmo->drawPoint(controlPointA->getWorldPosition(), Color::fromRGBA(231, 0, 18), pointPixels, 4);
							m_gizmo->drawPoint(controlPointB->getWorldPosition(), Color::fromRGBA(231, 0, 18), pointPixels, 4);

							m_gizmo->drawLine(pointA->getWorldPosition(), controlPointA->getWorldPosition(), Color::fromRGBA(231, 0, 18));
							m_gizmo->drawLine(pointB->getWorldPosition(), controlPointB->getWorldPosition(), Color::fromRGBA(231, 0, 18));
						}
					}
					else
					{
						m_gizmo->drawLine(pointA->getWorldPosition(), pointB->getWorldPosition(), segmentColor);
					}
				}
			}
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

