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
				EchoLogError("gltf load asset [%s] failed.", path.c_str());
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
				m_metaData.m_version = asset["version"].get<std::string>();

			// copyright
			if (asset.find("copyright") != asset.end())
				m_metaData.m_copyright = asset["copyright"].get<std::string>();

			// generator
			if (asset.find("generator") != asset.end())
				m_metaData.m_generator = asset["generator"].get<std::string>();

			// minVersion
			if (asset.find("minVersion") != asset.end())
				m_metaData.m_minversion = asset["minVersion"].get<std::string>();

			return true;
		}

		return false;
	}
}