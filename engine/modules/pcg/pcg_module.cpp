#include "pcg_module.h"
#include "pcg_flow_graph.h"
#include "node/primitive/pcg_box.h"
#include "node/image/pcg_image_perlin_noise.h"
#include "node/image/pcg_image_voronoi.h"
#include "node/image/pcg_image_save.h"
#include "node/terrain/pcg_heightfield_output.h"
#include "editor/pcg_flow_graph_editor.h"

namespace Echo
{
	DECLARE_MODULE(PCGModule, __FILE__)

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
        Class::registerType<PCGFlowGraph>();

		Class::registerType<PCGNode>();
		Class::registerType<PCGBox>();

		Class::registerType<PCGImagePerlinNoise>();
		Class::registerType<PCGImageVoronoi>();
		Class::registerType<PCGImageSave>();

		Class::registerType<PCGHeightfieldOutput>();

		CLASS_REGISTER_EDITOR(PCGFlowGraph, PCGFlowGraphEditor)
	}
}
