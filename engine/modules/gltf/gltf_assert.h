#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include <nlohmann/json.hpp>

namespace Echo
{
	using GltfAttributes = std::unordered_map<String, ui32>;

	struct GltfMetaInfo
	{
		String	m_copyright;
		String	m_generator;
		String	m_version;
		String	m_minversion;
	};

	struct GltfSceneInfo
	{
		String				m_name;
		vector<ui32>::type	m_nodes;
	};

	enum GltfPrimitiveMode
	{
		Points = 0,
		Lines,
		LineLoop,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan
	};

	struct GltfPrimitive
	{
		ui32				m_indices = -1;		// index to accessor containing the indices
		ui32				m_material = -1;	// index to the material
		GltfPrimitiveMode	m_mode = GltfPrimitiveMode::Triangles; // each attribute is mapped with his name and accessor index to the data
		GltfAttributes		m_attributes;
	};

	struct GltfMeshInfo
	{
		String						m_name;
		vector<float>::type			m_weights;
		vector<GltfPrimitive>::type	m_primitives;
	};

	struct GltfAsset
	{
		GltfMetaInfo				m_metaInfo;
		vector<GltfSceneInfo>::type	m_scenes;
		vector<GltfMeshInfo>::type	m_meshes;

		// load
		bool load(const String& path);
		bool loadAsset(nlohmann::json& json);
		bool loadScenes(nlohmann::json& json);
		bool loadMeshes(nlohmann::json& json);
	};
}