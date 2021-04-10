#include "pcg_heightfield_output.h"
#include "engine/modules/pcg/data/image/pcg_image.h"
#include "engine/modules/pcg/pcg_flow_graph.h"
#include "engine/modules/scene/terrain/terrain.h"

namespace Echo
{
	PCGHeightfieldOutput::PCGHeightfieldOutput()
	{
		m_inputs.push_back(EchoNew(PCGConnectPoint(this, "Image")));
	}

	PCGHeightfieldOutput::~PCGHeightfieldOutput()
	{

	}

	void PCGHeightfieldOutput::bindMethods()
	{
		CLASS_BIND_METHOD(PCGHeightfieldOutput, getTerrainPath, DEF_METHOD("getTerrainPath"));
		CLASS_BIND_METHOD(PCGHeightfieldOutput, setTerrainPath, DEF_METHOD("setTerrainPath"));
		CLASS_BIND_METHOD(PCGHeightfieldOutput, isAutoCreate, DEF_METHOD("isAutoCreate"));
		CLASS_BIND_METHOD(PCGHeightfieldOutput, setAutoCreate, DEF_METHOD("setAutoCreate"));

		CLASS_REGISTER_PROPERTY(PCGHeightfieldOutput, "Terrain", Variant::Type::NodePath, "getTerrainPath", "setTerrainPath");
		CLASS_REGISTER_PROPERTY(PCGHeightfieldOutput, "AutoCreate", Variant::Type::Bool, "isAutoCreate", "setAutoCreate");
	}

	void PCGHeightfieldOutput::setTerrainPath(const NodePath& terrainPath)
	{
		m_terrainPath.setPath(terrainPath.getPath());
	}

	void PCGHeightfieldOutput::run()
	{
		if (m_graph)
		{
			Terrain* terrain = ECHO_DOWN_CAST<Terrain*>(m_graph->getNode(m_terrainPath.getPath().c_str()));
			if (terrain)
			{
				PCGDataPtr inputData = m_inputs[0]->getData();
				if (inputData)
				{
					PCGImagePtr image = dynamic_cast<PCGImage*>(inputData.ptr());
					if (image)
					{
						vector<float>::type heightData;
						heightData.reserve(image->getColors().size());

						for (const Color& color : image->getColors())
						{
							heightData.push_back(color.r);
						}

						terrain->setHeight(0, 0, image->getWidth(), image->getHeight(), heightData);
					}
				}
			}
		}
	}
}