#include "pcg_module.h"
#include "procedural_texture.h"
#include "procedural_geometry.h"
#include "procedural_depend_graph.h"

namespace Echo
{
	DECLARE_MODULE(PCGModule)

	PCGModule::PCGModule()
	{

	}

	PCGModule* PCGModule::instance()
	{
		static PCGModule* inst = EchoNew(PCGModule);
		return inst;
	}

	void PCGModule::bindMethods()
	{

	}

	void PCGModule::registerTypes()
	{
		Class::registerType<ProceduralTexture>();
        Class::registerType<ProceduralGeometry>();

//		REGISTER_OBJECT_EDITOR(Curve, CurveEditor)
	}
}
