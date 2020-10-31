#include "spline_segment_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplineSegmentEditor::SplineSegmentEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	SplineSegmentEditor::~SplineSegmentEditor()
	{
	}

	ImagePtr SplineSegmentEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline_segment.png");
	}

	void SplineSegmentEditor::onEditorSelectThisNode()
	{ 
		m_isSelected = true;

		SplineSegment* segment = ECHO_DOWN_CAST<SplineSegment*>(m_object);
		if (segment)
		{
			Spline* spline = ECHO_DOWN_CAST<Spline*>(segment->getParent());
			if (spline)
			{
				SplinePoint* pointA = spline->getPoint(segment->getEndPointA());
				SplinePoint* pointB = spline->getPoint(segment->getEndPointB());
				Vector3 offset = 0.5f * (pointA->getWorldPosition() + pointB->getWorldPosition()) - segment->getWorldPosition();
				if (offset.lenSqr())
				{
					segment->setWorldPosition(segment->getWorldPosition()+offset);
					segment->update(0.f, true);

					SplineControlPoint* controlPointA = segment->getControlPointA();
					SplineControlPoint* controlPointB = segment->getControlPointB();

					if (controlPointA) controlPointA->setWorldPosition(controlPointA->getWorldPosition() - offset);
					if (controlPointB) controlPointB->setWorldPosition(controlPointB->getWorldPosition() - offset);
				}
			}
		}
	}

	void SplineSegmentEditor::editor_update_self()
	{

	}
#endif
}

