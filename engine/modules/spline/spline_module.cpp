#include "spline_module.h"
#include "spline.h"
#include "spline_point.h"
#include "spline_segment.h"
#include "editor/spline_editor.h"
#include "editor/spline_point_editor.h"
#include "editor/spline_segment_editor.h"

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

		REGISTER_OBJECT_EDITOR(Spline, SplineEditor)
		REGISTER_OBJECT_EDITOR(SplinePoint, SplinePointEditor)
		REGISTER_OBJECT_EDITOR(SplineSegment, SplineSegmentEditor)
	}
}