#include "procedural_module.h"
#include "procedural_texture.h"
#include "procedural_geometry.h"
#include "pg/node/connect/pg_connect.h"
#include "pg/node/primitive/pg_sphere.h"
#include "pg/node/primitive/pg_grid.h"
#include "editor/procedural_geometry_editor.h"
#include "editor/procedural_texture_editor.h"
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

		Class::registerType<PGNode>();
		Class::registerType<PGConnnect>();
		Class::registerType<PGSphere>();
		Class::registerType<PGGrid>();

		REGISTER_OBJECT_EDITOR(ProceduralTexture, ProceduralTextureEditor)
		REGISTER_OBJECT_EDITOR(ProceduralGeometry, ProceduralGeometryEditor)
	}
}
