#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/MathLib.h"
#include "engine/core/io/DataStream.h"
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
		vector<GltfPrimitive>::type	m_primitives;
	};

	struct GltfNodeInfo
	{
		String				m_name;
		i32					m_camera = -1;
		i32					m_mesh = -1;
		i32					m_skin = -1;
		vector<i32>::type	m_children;
		Quaternion			m_rotation = { 0, 0, 0, 1 };
		Vector3				m_scale = { 1, 1, 1 };
		Vector3				m_translation = { 0, 0, 0 };
	};

	struct GltfBufferInfo
	{
		String			m_name;
		String			m_uri;
		ui32			m_byteLength;
		MemoryReader*	m_data = nullptr;
	};

	struct GltfAsset
	{
		GltfMetaInfo					m_metaInfo;
		vector<GltfSceneInfo>::type		m_scenes;
		vector<GltfMeshInfo>::type		m_meshes;
		vector<GltfNodeInfo>::type		m_nodes;
		vector<GltfBufferInfo>::type	m_buffers;

		// load
		bool load(const String& path);
		bool loadAsset(nlohmann::json& json);
		bool loadScenes(nlohmann::json& json);
		bool loadMeshes(nlohmann::json& json);
		bool loadNodes(nlohmann::json& json);
		bool loadBuffers(nlohmann::json& json);
		bool loadBufferData(GltfBufferInfo& buffer);
	};
}