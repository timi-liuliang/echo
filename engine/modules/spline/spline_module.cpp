#include "spline_module.h"
#include "spline.h"
#include "spline_point.h"
#include "spline_segment.h"
#include "spline_control_point.h"
#include "editor/spline_editor.h"
#include "editor/spline_point_editor.h"
#include "editor/spline_segment_editor.h"
#include "editor/spline_control_point_editor.h"

namespace Echo
{
	DECLARE_MODULE(SplineModule)

	SplineModule::SplineModule()
	{
	}

	SplineModule* SplineModule::instance()
	{
		static SplineModule* inst = EchoNew(SplineModule);
		return inst;
	}

	void SplineModule::bindMethods()
	{

	}

	void SplineModule::registerTypes()
	{
		Class::registerType<Spline>();
		Class::registerType<SplinePoint>();
		Class::registerType<SplineSegment>();
		Class::registerType<SplineControlPoint>();

		CLASS_REGISTER_EDITOR(Spline, SplineEditor)
		CLASS_REGISTER_EDITOR(SplinePoint, SplinePointEditor)
		CLASS_REGISTER_EDITOR(SplineSegment, SplineSegmentEditor)
		CLASS_REGISTER_EDITOR(SplineControlPoint, SplineControlPointEditor)
	}
}