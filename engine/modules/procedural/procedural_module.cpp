#include "procedural_module.h"
#include "procedural_texture.h"
#include "procedural_geometry.h"
#include "procedural_grid.h"
#include "procedural_sphere.h"
#include "procedural_depend_graph.h"

namespace Echo
{
	DECLARE_MODULE(ProceduralModule)

	ProceduralModule::ProceduralModule()
	{

	}

	ProceduralModule* ProceduralModule::instance()
	{
		static ProceduralModule* inst = EchoNew(ProceduralModule);
		return inst;
	}

	void ProceduralModule::bindMethods()
	{

	}

	void ProceduralModule::registerTypes()
	{
		Class::registerType<ProceduralTexture>();
        Class::registerType<ProceduralGeometry>();
		Class::registerType<ProceduralGird>();
		Class::registerType<ProceduralSphere>();

//		REGISTER_OBJECT_EDITOR(Curve, CurveEditor)
	}
}
