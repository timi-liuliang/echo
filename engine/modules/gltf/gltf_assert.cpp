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

			if (!loadMeshes(j))
			{
				EchoLogError("gltf parse meshes failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadNodes(j))
			{
				EchoLogError("gltf parse nodes failed when load resource [%s].", path.c_str());
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

	bool GltfAsset::loadMeshes(nlohmann::json& json)
	{
		if (json.find("meshes") == json.end())
			return false;

		nlohmann::json& meshes = json["meshes"];
		if (!meshes.is_array())
			return false;

		m_meshes.resize(meshes.size());
		for (ui32 i = 0; i < meshes.size(); i++)
		{
			if (meshes[i].find("name") != meshes[i].end())
			{
				if (!meshes[i]["name"].is_string())
					return false;

				m_meshes[i].m_name = meshes[i]["name"].get<std::string>();
			}

			if (meshes[i].find("primitives") == meshes[i].end())
				return false;

			nlohmann::json& primitives = meshes[i]["primitives"];
			if (!primitives.is_array())
				return false;

			m_meshes[i].m_primitives.resize(primitives.size());
			for (ui32 j = 0; j < primitives.size(); j++)
			{
				// indices
				if (primitives[j].find("indices") != primitives[j].end())
				{
					if (!primitives[j]["indices"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_indices = primitives[j]["indices"].get<ui32>();
				}

				// material
				if (primitives[j].find("material") != primitives[j].end())
				{
					if (!primitives[j]["material"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_material = primitives[j]["material"].get<ui32>();
				}

				// mode
				if (primitives[j].find("mode") != primitives[j].end())
				{
					if (!primitives[j]["mode"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_mode = GltfPrimitiveMode(primitives[j]["mode"].get<ui8>());
				}

				if (primitives[j].find("attributes") == primitives[j].end())
					return false;

				nlohmann::json& attributes = primitives[j]["attributes"];
				if (!attributes.is_object())
					return false;

				for (nlohmann::json::iterator it = attributes.begin(); it != attributes.end(); it++)
				{
					m_meshes[i].m_primitives[j].m_attributes[it.key()] = it.value();
				}
			}
		}

		return true;
	}

	bool GltfAsset::loadNodes(nlohmann::json& json)
	{
		if (json.find("nodes") == json.end())
			return false;

		nlohmann::json& nodes = json["nodes"];
		if (!nodes.is_array())
			return false;

		m_nodes.resize(nodes.size());
		for (ui32 i = 0; i < nodes.size(); i++)
		{
			// name
			if (nodes[i].find("name") != nodes[i].end())
			{
				if (!nodes[i]["name"].is_string())
					return false;

				m_nodes[i].m_name = nodes[i]["name"].get<std::string>();
			}

			// camera
			if (nodes[i].find("camera") != nodes[i].end())
			{
				if (!nodes[i]["camera"].is_number())
					return false;

				m_nodes[i].m_camera = nodes[i]["camera"].get<i32>();
			}

			// children
			if (nodes[i].find("children") != nodes[i].end())
			{
				nlohmann::json& children = nodes[i]["children"];
				if (!children.is_array())
					return false;

				m_nodes[i].m_children.resize(children.size());
				for (ui32 j = 0; j < children.size(); j++)
				{
					m_nodes[i].m_children[j] = children[j].get<ui32>();
				}
			}

			// skin
			if (nodes[i].find("skin") != nodes[i].end()) 
			{
				if (!nodes[i]["skin"].is_number())
					return false;

				m_nodes[i].m_skin = nodes[i]["skin"].get<i32>();
			}

			// mesh
			if (nodes[i].find("mesh") != nodes[i].end())
			{
				if (!nodes[i]["mesh"].is_number())
					return false;

				m_nodes[i].m_mesh = nodes[i]["mesh"].get<i32>();
			}

			// translation
			if (nodes[i].find("translation") != nodes[i].end())
			{
				if (!nodes[i]["translation"].is_array())
					return false;

				if (nodes[i]["translation"].size() != 3)
					return false;

				for (ui32 j = 0; j < 3; j++)
				{
					if (!nodes[i]["translation"][j].is_number())
						return false;

					m_nodes[i].m_translation[j] = nodes[i]["translation"][j].get<float>();
				}
			}

			// rotation
			if (nodes[i].find("rotation") != nodes[i].end())
			{
				if (!nodes[i]["rotation"].is_array())
					return false;

				if (nodes[i]["rotation"].size() != 4)
					return false;

				for (ui32 j = 0; j < 4; j++)
				{
					if (!nodes[i]["rotation"][j].is_number())
						return false;

					m_nodes[i].m_rotation[j] = nodes[i]["rotation"][j].get<float>();
				}
			}

			// scale
			if (nodes[i].find("scale") != nodes[i].end())
			{
				if (!nodes[i]["scale"].is_array())
					return false;

				if (nodes[i]["scale"].size() != 3)
					return false;

				for (ui32 j = 0; j < 3; j++)
				{
					if (!nodes[i]["scale"][j].is_number())
						return false;

					m_nodes[i].m_scale[j] = nodes[i]["scale"][j].get<float>();
				}
			}
		}

		return true;
	}
}