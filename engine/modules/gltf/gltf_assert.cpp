#include "gltf_assert.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/util/LogManager.h"

namespace Echo
{
	// parse float value of json node
	static bool parseJsonValueFloat(float& oValue, nlohmann::json& json, const String& key)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_number())
				return false;

			oValue = subJson.get<float>();

			return true;
		}

		return false;
	}

	// parse int value of json node
	static bool parseJsonValueI32(i32& oValue, nlohmann::json& json, const String& key)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_number())
				return false;

			oValue = subJson.get<i32>();

			return true;
		}

		return false;
	}


	// parse bool value of json node
	static bool parseJsonValueBool(bool& oValue, nlohmann::json& json, const String& key)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_boolean())
				return false;

			oValue = subJson.get<bool>();

			return true;
		}

		return false;
	}

	// parse string value of json node
	static bool parseJsonValueString(String& oValue, nlohmann::json& json, const String& key)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_string())
				return false;

			oValue = subJson.get<std::string>();

			return true;
		}

		return false;
	}

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

			if (!loadBuffers(j))
			{
				EchoLogError("gltf parse buffers failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadAccessors(j))
			{
				EchoLogError("gltf parse accessors failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadMaterials(j))
			{
				EchoLogError("gltf parse materials failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadImages(j))
			{
				EchoLogError("gltf parse images failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadSamplers(j))
			{
				EchoLogError("gltf parse samples failed when load resource [%s].", path.c_str());
				return false;
			}

			if (!loadTextures(j))
			{
				EchoLogError("gltf parse textures failed when load resource [%s].", path.c_str());
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
			nlohmann::json& scene = scenes[i];

			// name
			if (scene.find("name") != scene.end())
			{
				if (!scene["name"].is_string())
					return false;

				m_scenes[i].m_name = scene["name"].get<std::string>();
			}

			// nodes
			if (scene.find("nodes") != scene.end())
			{
				nlohmann::json& nodes = scene["nodes"];
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
			nlohmann::json& mesh = meshes[i];

			if (mesh.find("name") != mesh.end())
			{
				if (!mesh["name"].is_string())
					return false;

				m_meshes[i].m_name = mesh["name"].get<std::string>();
			}

			if (mesh.find("primitives") == mesh.end())
				return false;

			nlohmann::json& primitives = mesh["primitives"];
			if (!primitives.is_array())
				return false;

			m_meshes[i].m_primitives.resize(primitives.size());
			for (ui32 j = 0; j < primitives.size(); j++)
			{
				nlohmann::json& primitive = primitives[j];

				// indices
				if (primitive.find("indices") != primitive.end())
				{
					if (!primitive["indices"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_indices = primitive["indices"].get<ui32>();
				}

				// material
				if (primitive.find("material") != primitive.end())
				{
					if (!primitive["material"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_material = primitive["material"].get<ui32>();
				}

				// mode
				if (primitive.find("mode") != primitive.end())
				{
					if (!primitive["mode"].is_number())
						return false;

					m_meshes[i].m_primitives[j].m_mode = GltfPrimitive::Mode(primitive["mode"].get<ui8>());
				}

				if (primitive.find("attributes") == primitive.end())
					return false;

				nlohmann::json& attributes = primitive["attributes"];
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
			nlohmann::json& node = nodes[i];

			// name
			if (node.find("name") != node.end())
			{
				if (!node["name"].is_string())
					return false;

				m_nodes[i].m_name = node["name"].get<std::string>();
			}

			// camera
			if (node.find("camera") != node.end())
			{
				if (!nodes[i]["camera"].is_number())
					return false;

				m_nodes[i].m_camera = node["camera"].get<i32>();
			}

			// children
			if (node.find("children") != node.end())
			{
				nlohmann::json& children = node["children"];
				if (!children.is_array())
					return false;

				m_nodes[i].m_children.resize(children.size());
				for (ui32 j = 0; j < children.size(); j++)
				{
					m_nodes[i].m_children[j] = children[j].get<ui32>();
				}
			}

			// skin
			if (node.find("skin") != node.end()) 
			{
				if (!node["skin"].is_number())
					return false;

				m_nodes[i].m_skin = node["skin"].get<i32>();
			}

			// mesh
			if (node.find("mesh") != node.end())
			{
				if (!node["mesh"].is_number())
					return false;

				m_nodes[i].m_mesh = node["mesh"].get<i32>();
			}

			// translation
			if (node.find("translation") != node.end())
			{
				nlohmann::json& translation = node["translation"];
				if (!translation.is_array())
					return false;

				if (translation.size() != 3)
					return false;

				for (ui32 j = 0; j < 3; j++)
				{
					if (!translation[j].is_number())
						return false;

					m_nodes[i].m_translation[j] = translation[j].get<float>();
				}
			}

			// rotation
			if (node.find("rotation") != node.end())
			{
				nlohmann::json& rotation = node["rotation"];
				if (!rotation.is_array())
					return false;

				if (rotation.size() != 4)
					return false;

				for (ui32 j = 0; j < 4; j++)
				{
					if (!rotation[j].is_number())
						return false;

					m_nodes[i].m_rotation[j] = rotation[j].get<float>();
				}
			}

			// scale
			if (node.find("scale") != node.end())
			{
				nlohmann::json& scale = node["scale"];
				if (!scale.is_array())
					return false;

				if (scale.size() != 3)
					return false;

				for (ui32 j = 0; j < 3; j++)
				{
					if (!scale[j].is_number())
						return false;

					m_nodes[i].m_scale[j] = scale[j].get<float>();
				}
			}
		}

		return true;
	}

	bool GltfAsset::loadBuffers(nlohmann::json& json)
	{
		if (json.find("buffers") == json.end())
			return false;

		nlohmann::json& buffers = json["buffers"];
		if (!buffers.is_array())
			return false;

		m_buffers.resize(buffers.size());
		for(ui32 i=0; i<buffers.size(); i++)
		{
			nlohmann::json& buffer = buffers[i];

			// name
			if (buffer.find("name") != buffer.end())
			{
				nlohmann::json& name = buffer["name"];
				if (name.is_string())
					return false;

				m_buffers[i].m_name = name.get<std::string>();
			}

			// byteLength
			if (buffer.find("byteLength") == buffer.end())
				return false;

			nlohmann::json& byteLength = buffer["byteLength"];
			if (!byteLength.is_number())
				return false;

			m_buffers[i].m_byteLength = byteLength.get<i32>();

			// uri
			if (buffer.find("uri") != buffer.end())
			{
				nlohmann::json& uri = buffer["uri"];
				if (!uri.is_string())
					return false;

				m_buffers[i].m_uri = uri.get<std::string>();
			}

			if (!loadBufferData(m_buffers[i]))
				return false;
		}

		return true;
	}

	bool GltfAsset::loadBufferData(GltfBufferInfo& buffer)
	{
		if (buffer.m_uri.empty() && buffer.m_byteLength > 0)
			return false;

		buffer.m_data = EchoNew(MemoryReader(buffer.m_uri));
		if (buffer.m_data->getSize())
		{
			return true;
		}

		return false;
	}

	bool GltfAsset::loadAccessors(nlohmann::json& json)
	{
		if (json.find("accessors") == json.end())
			return false;

		nlohmann::json& accessors = json["accessors"];
		if (!accessors.is_array())
			return false;

		m_accessors.resize(accessors.size());
		for ( ui32 i = 0; i < accessors.size(); ++i)
		{
			nlohmann::json& accessor = accessors[i];

			// bufferView
			if (accessor.find("bufferView") != accessor.end())
			{
				if (!accessor["bufferView"].is_number())
					return false;

				m_accessors[i].m_bufferView = accessor["bufferView"].get<i32>();
			}

			// byteOffset
			if (accessor.find("byteOffset") != accessor.end()) 
			{
				if (!accessor["byteOffset"].is_number())
					return false;

				m_accessors[i].m_byteOffset = accessor["byteOffset"].get<ui32>();
			}

			// componentType
			if (accessor.find("componentType") == accessor.end())
				return false;
			if (!accessor["componentType"].is_number())
				return false;

			m_accessors[i].m_componentType = static_cast<GltfAccessorInfo::ComponentType>(accessor["componentType"].get<ui16>());

			// normalized
			if (accessor.find("normalized") != accessor.end())
			{
				if (!accessor["normalized"].is_boolean())
					return false;

				m_accessors[i].m_normalized = accessor["normalized"].get<bool>();
			}

			// count
			if (accessor.find("count") == accessor.end())
				return false;
			else if (!accessor["count"].is_number())
				return false;

			m_accessors[i].m_count = accessor["count"].get<ui32>();

			// type
			if (accessor.find("type") == accessors[i].end())
				return false;
			if (!accessor["type"].is_string())
				return false;

			String type = accessor["type"].get<std::string>();
			if (type == "SCALAR")		m_accessors[i].m_type = GltfAccessorInfo::Type::Scalar;
			else if (type == "VEC2")	m_accessors[i].m_type = GltfAccessorInfo::Type::Vec2;
			else if (type == "VEC3")	m_accessors[i].m_type = GltfAccessorInfo::Type::Vec3;
			else if (type == "VEC4")	m_accessors[i].m_type = GltfAccessorInfo::Type::Vec4;
			else if (type == "MAT2")	m_accessors[i].m_type = GltfAccessorInfo::Type::Mat2;
			else if (type == "MAT3")	m_accessors[i].m_type = GltfAccessorInfo::Type::Mat3;
			else if (type == "MAT4")	m_accessors[i].m_type = GltfAccessorInfo::Type::Mat4;
			else						return false;

			// TODO: accessors[i]["sparse"]
			// TODO: accessors[i]["extensions"]
			// TODO: accessors[i]["min"]
			// TODO: accessors[i]["max"]
		}

		return true;
	}

	bool GltfAsset::loadMaterials(nlohmann::json& json)
	{
		if (json.find("materials") == json.end())
			return true;

		nlohmann::json& materials = json["materials"];
		if (!materials.is_array())
			return false;

		m_materials.resize(materials.size());
		for (ui32 i = 0; i < materials.size(); i++)
		{
			nlohmann::json& material = materials[i];
			
			// name
			if (material.find("name") != material.end())
			{
				if (!material["name"].is_string())
					return false;

				m_materials[i].m_name = material["name"].get<std::string>();
			}

			// pbr metalic roughness
			if (material.find("pbrMetallicRoughness") != material.end())
			{
				nlohmann::json& pbrMetallicRoughness = material["pbrMetallicRoughness"];

				if (!pbrMetallicRoughness.is_object())
					return false;

				// pbrMetallicRoughness.baseColorFactor
				if (pbrMetallicRoughness.find("baseColorFactor") != pbrMetallicRoughness.end())
				{
					nlohmann::json& baseColorFactor = pbrMetallicRoughness["baseColorFactor"];
					if (!baseColorFactor.is_array())
						return false;

					if (baseColorFactor.size() != 4)
						return false;

					for (ui32 j = 0; j < 4; j++)
					{
						if (baseColorFactor[j].is_number())
							return false;

						m_materials[i].m_pbr.m_baseColorFactor[j] = baseColorFactor[j].get<float>();
					}				
				}

				// pbrMetallicRoughness.baseColorTexture
				if (pbrMetallicRoughness.find("baseColorTexture") != pbrMetallicRoughness.end())
				{
					if (!loadTextureInfo(m_materials[i].m_pbr.m_baseColorTexture, pbrMetallicRoughness["baseColorTexture"]))
						return false;
				}

				// pbrMetallicRoughness.metallicFactor
				if (!parseJsonValueFloat(m_materials[i].m_pbr.m_metallicFactor, pbrMetallicRoughness, "metallicFactor"))
					return false;

				// pbrMetallicRoughness.roughnessFactor
				if (!parseJsonValueFloat(m_materials[i].m_pbr.m_roughnessFactor, pbrMetallicRoughness, "roughnessFactor"))
					return false;

				// pbrMetallicRoughness.metallicRoughnessTexture
				if (pbrMetallicRoughness.find("metallicRoughnessTexture") != pbrMetallicRoughness.end())
				{
					if (!loadTextureInfo(m_materials[i].m_pbr.m_metallicRoughnessTexture, pbrMetallicRoughness["metallicRoughnessTexture"]))
						return false;
				}
			}

			// normal texture
			if (material.find("normalTexture") != material.end())
			{
				nlohmann::json& normalTexture = material["normalTexture"];
				if (!loadTextureInfo(m_materials[i].m_normalTexture, normalTexture))
					return false;

				// scale
				if (!parseJsonValueFloat(m_materials[i].m_normalTexture.m_scale, normalTexture, "scale"))
					return false;
			}

			// occlusion texture
			if (material.find("occlusionTexture") != material.end())
			{
				nlohmann::json& occlusionTexture = material["occlusionTexture"];
				if (!loadTextureInfo(m_materials[i].m_occlusionTexture, occlusionTexture))
					return false;

				// scale
				if (!parseJsonValueFloat(m_materials[i].m_occlusionTexture.m_strength, occlusionTexture, "strength"))
					return false;
			}

			// emissiveTexture
			if (material.find("emissiveTexture") != material.end()) 
			{
				if (!loadTextureInfo(m_materials[i].m_emissiveTexture, material["emissiveTexture"]))
					return false;
			}

			// emissiveFactor
			if (material.find("emissiveFactor") != material.end())
			{
				nlohmann::json& emissiveFactor = material["emissiveFactor"];
				if (!emissiveFactor.is_array())
					return false;

				if (emissiveFactor.size() != 3)
					return false;

				for (uint32_t j = 0; j < 3; ++j)
				{
					if (!emissiveFactor.is_number())
						return false;

					m_materials[i].m_emissiveFactor[j] = emissiveFactor[j].get<float>();
				}
			}

			// alphaMode
			if (material.find("alphaMode") != material.end())
			{
				if (!material["alphaMode"].is_string())
					return false;

				std::string type = material["alphaMode"].get<std::string>();
				if (type == "OPAQUE")		m_materials[i].m_alphaMode = GltfMaterialInfo::AlphaMode::Opaque;
				else if (type == "MASK")	m_materials[i].m_alphaMode = GltfMaterialInfo::AlphaMode::Mask;
				else if (type == "BLEND")	m_materials[i].m_alphaMode = GltfMaterialInfo::AlphaMode::Blend;
				else                        return false;

				return true;
			}

			// alphaCutoff
			if (parseJsonValueFloat(m_materials[i].m_alphaCutoff, material, "alphaCutoff"))
				return false;

			// doubleSided
			if (!parseJsonValueBool(m_materials[i].m_isDoubleSided, material, "doubleSided"))
				return false;

			// TODO: materials[i]["extensions"]
			// TODO: materials[i]["extras"]
		}

		return true;
	}

	bool GltfAsset::loadTextureInfo(GltfMaterialInfo::Texture& texture, nlohmann::json& json)
	{
		if (!json.is_object())
			return false;

		// index
		if (json.find("index") == json.end())
			return false;

		if (!json["index"].is_number())
			return false;

		texture.m_index = json["index"].get<i32>();

		// texCoord
		if (json.find("texCoord") != json.end())
		{
			nlohmann::json& texCoord = json["texCoord"];
			if (!texCoord.is_number())
				return false;

			texture.m_texCoord = texCoord.get<i32>();
		}

		// TODO json["extensions"]
		// TODO json["extras"]

		return true;
	}

	bool GltfAsset::loadImages(nlohmann::json& json)
	{
		if (json.find("images") == json.end())
			return true;

		nlohmann::json& images = json["images"];
		if (!images.is_array())
			return false;

		m_images.resize(images.size());
		for (ui32 i = 0; i < images.size(); i++)
		{
			nlohmann::json& image = images[i];

			// name
			if (!parseJsonValueString(m_images[i].m_name, image, "name"))
				return false;

			// uri
			if (!parseJsonValueString(m_images[i].m_uri, image, "uri"))
				return false;

			// mimeType
			if (!parseJsonValueString(m_images[i].m_mimeType, image, "mimeType"))
				return false;

			// bufferView
			if (!parseJsonValueI32(m_images[i].m_bufferView, image, "bufferView"))
				return false;
		}

		// TODO: Handle dependencies between mimeType and bufferView
		// TODO: Handle the fact that we want an uri OR a bufferView
		// TODO: images[i]["extensions"]
		// TODO: images[i]["extras"]

		return true;
	}

	bool GltfAsset::loadSamplers(nlohmann::json& json)
	{
		if (json.find("samplers") == json.end())
			return true;

		nlohmann::json& samplers = json["samplers"];
		if (!samplers.is_array())
			return false;

		m_samplers.resize(samplers.size());
		for (ui32 i = 0; i < samplers.size(); i++)
		{
			nlohmann::json& sampler = samplers[i];

			// name
			if (!parseJsonValueString(m_samplers[i].m_name, sampler, "name"))
				return false;

			i32 iValue;

			// magFilter
			if (parseJsonValueI32(iValue, sampler, "magFilter"))
				m_samplers[i].m_magFilter = GltfSamplerInfo::MagFilter(iValue);
			else
				return false;

			// minFilter
			if (parseJsonValueI32(iValue, sampler, "minFilter"))
				m_samplers[i].m_minFilter = GltfSamplerInfo::MinFilter(iValue);
			else
				return false;

			// wrapS
			if (parseJsonValueI32(iValue, sampler, "wrapS"))
				m_samplers[i].m_wrapS = GltfSamplerInfo::WrappingMode(iValue);
			else
				return false;

			// wrapT
			if (parseJsonValueI32(iValue, sampler, "wrapT"))
				m_samplers[i].m_wrapT = GltfSamplerInfo::WrappingMode(iValue);
			else
				return false;

			// TODO: samplers[i]["extensions"]
			// TODO: samplers[i]["extras"]
		}

		return true;
	}

	bool GltfAsset::loadTextures(nlohmann::json& json)
	{
		if (json.find("textures") == json.end())
			return true;

		nlohmann::json& textures = json["textures"];
		if (!textures.is_array())
			return false;

		m_textures.resize(textures.size());
		for (ui32 i = 0; i < textures.size(); i++)
		{
			nlohmann::json& texture = textures[i];

			// name
			if (!parseJsonValueString(m_textures[i].m_name, texture, "name"))
				return false;

			// sampler
			if (!parseJsonValueI32(m_textures[i].m_sampler, texture, "sampler"))
				return false;

			// source
			if (!parseJsonValueI32(m_textures[i].m_source, texture, "source"))
				return false;
		}

		// TODO: textures[i]["extensions"]
		// TODO: textures[i]["extras"]

		return true;
	}

	// build echo node
	Node* GltfAsset::build()
	{
		return nullptr;
	}
}