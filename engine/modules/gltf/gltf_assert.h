#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <nlohmann/json.hpp>

namespace Echo
{
	struct GltfMetaData
	{
		String		m_copyright;
		String		m_generator;
		String		m_version;
		String		m_minversion;
	};

	struct GltfAsset
	{
		GltfMetaData	m_metaData;

		// load
		bool load(const String& path);
		bool loadAsset(nlohmann::json& j);
	};
}