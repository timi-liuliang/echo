#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <nlohmann/json.hpp>

namespace Echo
{
	struct GltfMetaInfo
	{
		String		m_copyright;
		String		m_generator;
		String		m_version;
		String		m_minversion;
	};

	struct GltfSceneInfo
	{
		String				m_name;
		vector<ui32>::type	m_nodes;
	};

	struct GltfAsset
	{
		GltfMetaInfo				m_metaInfo;
		vector<GltfSceneInfo>::type	m_scenes;

		// load
		bool load(const String& path);
		bool loadAsset(nlohmann::json& json);
		bool loadScenes(nlohmann::json& json);
	};
}