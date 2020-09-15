#include "procedural_module.h"
#include "procedural_texture.h"
#include "procedural_geometry.h"
#include "node/pg/primitive/pg_sphere.h"
#include "node/pg/primitive/procedural_grid.h"
#include "node/pg/primitive/procedural_sphere.h"
#include "editor/procedural_geometry_editor.h"
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

		Class::registerType<PGNode>();
		Class::registerType<PGSphere>();

		REGISTER_OBJECT_EDITOR(ProceduralGeometry, ProceduralGeometryEditor)
	}
}
