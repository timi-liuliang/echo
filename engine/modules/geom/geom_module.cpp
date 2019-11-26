#include "geom_module.h"
#include "curve.h"
#include "editor/curve_editor.h"

namespace Echo
{
	GeomModule::GeomModule()
	{

	}

	GeomModule* GeomModule::instance()
	{
		GeomModule* inst = EchoNew(GeomModule);
		return inst;
	}

	void GeomModule::bindMethods()
	{

	}

	void GeomModule::registerTypes()
	{
		Class::registerType<Curve>();

		REGISTER_OBJECT_EDITOR(Curve, CurveEditor)
	}
}
