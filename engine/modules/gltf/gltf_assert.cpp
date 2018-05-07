#include "gltf_assert.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	// load
	bool GltfAsset::load(const String& path)
	{
		MemoryReader memReader(path);
		if (memReader.getSize())
		{
			using namespace nlohmann;
			json j =  json::parse(memReader.getData<char*>());

			// load asset
			if (!loadAsset(j))
			{
				EchoLogError("gltf parse asset failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadScenes(j))
			{
				EchoLogError("gltf parse scenes failed when load resource [%s].", path.c_str());
				return false;
			}

			return true;
		}

		return false;
	}

	bool GltfAsset::loadAsset(nlohmann::json& json)
	{
		if (json.find("asset") != json.end())
		{
			nlohmann::json& asset = json["asset"];
			if (asset.find("version") != asset.end())
				m_metaInfo.m_version = asset["version"].get<std::string>();

			// copyright
			if (asset.find("copyright") != asset.end())
				m_metaInfo.m_copyright = asset["copyright"].get<std::string>();

			// generator
			if (asset.find("generator") != asset.end())
				m_metaInfo.m_generator = asset["generator"].get<std::string>();

			// minVersion
			if (asset.find("minVersion") != asset.end())
				m_metaInfo.m_minversion = asset["minVersion"].get<std::string>();

			return true;
		}

		return false;
	}

	bool GltfAsset::loadScenes(nlohmann::json& json)
	{
		if (json.find("scenes") == json.end())
			return false;

		nlohmann::json& scenes = json["scenes"];
		if (!scenes.is_array())
			return false;

		m_scenes.resize(scenes.size());
		for (ui32 i = 0; i < scenes.size(); i++)
		{
			// name
			if (scenes[i].find("name") != scenes[i].end())
			{
				if (!scenes[i]["name"].is_string())
					return false;

				m_scenes[i].m_name = scenes[i]["name"].get<std::string>();
			}

			// nodes
			if (scenes[i].find("nodes") != scenes[i].end())
			{
				nlohmann::json& nodes = scenes[i]["nodes"];
				if (!nodes.is_array())
					return false;

				m_scenes[i].m_nodes.resize(nodes.size());
				for (ui32 j = 0; j < nodes.size(); j++)
				{
					m_scenes[i].m_nodes[j] = nodes[j].get<ui32>();
				}
			}
		}

		return true;
	}
}