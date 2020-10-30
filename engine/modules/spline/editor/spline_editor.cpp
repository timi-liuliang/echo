#include "spline_editor.h"
#include "engine/core/editor/editor.h"
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
			// points
			for (SplinePoint* point : spline->getPoints())
			{
				m_gizmo->drawPoint(point->getWorldPosition(), Color(0.f, 0.f, 1.f, 0.9f), 38.f, 4);
			}

			// segments
			for (SplineSegment* segment : spline->getSegments())
			{
				SplinePoint* pointA = spline->getPoint(segment->getEndPointA());
				SplinePoint* pointB = spline->getPoint(segment->getEndPointB());
				if (pointA && pointB)
				{
					m_gizmo->drawLine(pointA->getWorldPosition(), pointB->getWorldPosition(), Color(1.f, 1.f, 1.f, 0.5f));
				}
			}
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

