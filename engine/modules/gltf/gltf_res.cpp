#include "gltf_res.h"
#include "gltf_mesh.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/util/PathUtil.h"

namespace Echo
{
	// parse float value of json node
	static bool parseJsonValueFloat(float& oValue, nlohmann::json& json, const String& key, bool isMustExist)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_number())
				return false;

			oValue = subJson.get<float>();

			return true;
		}

		return isMustExist ? false : true;
	}

	// parse int value of json node
	static bool parseJsonValueI32(i32& oValue, nlohmann::json& json, const String& key, bool isMustExist)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_number())
				return false;

			oValue = subJson.get<i32>();

			return true;
		}

		return isMustExist ? false : true;
	}

	// parse int value of json node
	static bool parseJsonValueUI32(ui32& oValue, nlohmann::json& json, const String& key, bool isMustExist)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_number())
				return false;

			oValue = subJson.get<i32>();

			return true;
		}

		return isMustExist ? false : true;
	}


	// parse bool value of json node
	static bool parseJsonValueBool(bool& oValue, nlohmann::json& json, const String& key, bool isMustExist)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_boolean())
				return false;

			oValue = subJson.get<bool>();

			return true;
		}

		return isMustExist ? false : true;
	}

	// parse string value of json node
	static bool parseJsonValueString(String& oValue, nlohmann::json& json, const String& key, bool isMustExist)
	{
		if (json.find(key) != json.end())
		{
			nlohmann::json& subJson = json[key];
			if (!subJson.is_string())
				return false;

			oValue = subJson.get<std::string>();

			return true;
		}

		return isMustExist ? false : true;
	}

	// contructor
	GltfRes::GltfRes(const ResourcePath& path)
		: Res(path)
	{
		load();
	}

	GltfRes::~GltfRes()
	{

	}

	GltfRes* GltfRes::create(const ResourcePath& path)
	{
		Res* res = Res::get(path);
		if (res)
			return dynamic_cast<GltfRes*>(res);
		else
			return EchoNew(GltfRes(path));
	}

	// load
	bool GltfRes::load()
	{
		MemoryReader memReader(m_path.getPath());
		if (memReader.getSize())
		{
			using namespace nlohmann;
			json j =  json::parse(memReader.getData<char*>());

			// load asset
			if (!loadAsset(j))
			{
				EchoLogError("gltf parse asset failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadScenes(j))
			{
				EchoLogError("gltf parse scenes failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadMeshes(j))
			{
				EchoLogError("gltf parse meshes failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadNodes(j))
			{
				EchoLogError("gltf parse nodes failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadBuffers(j))
			{
				EchoLogError("gltf parse buffers failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadBufferViews(j))
			{
				EchoLogError("gltf parse bufferViews failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadAccessors(j))
			{
				EchoLogError("gltf parse accessors failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadMaterials(j))
			{
				EchoLogError("gltf parse materials failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadImages(j))
			{
				EchoLogError("gltf parse images failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadSamplers(j))
			{
				EchoLogError("gltf parse samples failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			if (!loadTextures(j))
			{
				EchoLogError("gltf parse textures failed when load resource [%s].", m_path.getPath().c_str());
				return false;
			}

			return true;
		}

		return false;
	}

	bool GltfRes::loadAsset(nlohmann::json& json)
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

	bool GltfRes::loadScenes(nlohmann::json& json)
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
			if (!parseJsonValueString(m_scenes[i].m_name, scene, "name", false))
				return false;

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

	bool GltfRes::loadMeshes(nlohmann::json& json)
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

			// name
			if (!parseJsonValueString(m_meshes[i].m_name, mesh, "name", false))
				return false;

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
				if (!parseJsonValueUI32(m_meshes[i].m_primitives[j].m_indices, primitive, "indices", true))
					return false;

				// material
				if (!parseJsonValueUI32(m_meshes[i].m_primitives[j].m_material, primitive, "material", false))
					return false;

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

	bool GltfRes::loadNodes(nlohmann::json& json)
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
			if (!parseJsonValueString(m_nodes[i].m_name, node, "name", false))
				return false;

			// camera
			if (!parseJsonValueI32(m_nodes[i].m_camera, node, "camera", false))
				return false;

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
			if (!parseJsonValueI32(m_nodes[i].m_skin, node, "skin", false))
				return false;

			// mesh
			if (!parseJsonValueI32(m_nodes[i].m_mesh, node, "mesh", false))
				return false;

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

	bool GltfRes::loadBuffers(nlohmann::json& json)
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
			if (!parseJsonValueString(m_buffers[i].m_name, buffer, "name", false))
				return false;

			// byteLength
			if (!parseJsonValueI32(m_buffers[i].m_byteLength, buffer, "byteLength", true))
				return false;

			// uri
			if (buffer.find("uri") != buffer.end())
			{
				nlohmann::json& uri = buffer["uri"];
				if (!uri.is_string())
					return false;

				// resolve url
				String bufferUri = uri.get<std::string>();
				if (StringUtil::StartWith(bufferUri, "data:"))
				{
					m_buffers[i].m_uri = bufferUri;
					m_buffers[i].m_uriType = GltfBufferInfo::UriType::Data;
				}
				else if (StringUtil::StartWith(bufferUri, "blob:"))
				{
					m_buffers[i].m_uri = bufferUri;
					m_buffers[i].m_uriType = GltfBufferInfo::UriType::Blob;
				}
				else
				{
					m_buffers[i].m_uri = PathUtil::GetFileDirPath(m_path.getPath()) + bufferUri;
				}
			}

			if (!loadBufferData(m_buffers[i]))
				return false;
		}

		return true;
	}

	bool GltfRes::loadBufferData(GltfBufferInfo& buffer)
	{
		if (buffer.m_uri.empty() && buffer.m_byteLength > 0)
			return false;

		if (buffer.m_uriType != GltfBufferInfo::UriType::Uri)
			return true;

		buffer.m_data = EchoNew(MemoryReader(buffer.m_uri));
		if (buffer.m_data->getSize())
		{
			return true;
		}

		return false;
	}

	bool GltfRes::loadAccessors(nlohmann::json& json)
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
			if (!parseJsonValueI32(m_accessors[i].m_bufferView, accessor, "bufferView", false))
				return false;

			// byteOffset
			if (!parseJsonValueI32(m_accessors[i].m_byteOffset, accessor, "byteOffset", false))
				return false;

			// componentType
			if (accessor.find("componentType") == accessor.end())
				return false;
			if (!accessor["componentType"].is_number())
				return false;

			m_accessors[i].m_componentType = static_cast<GltfAccessorInfo::ComponentType>(accessor["componentType"].get<ui16>());

			// normalized
			if (!parseJsonValueBool(m_accessors[i].m_normalized, accessor, "normalized", false))
				return false;

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

	bool GltfRes::loadBufferViews(nlohmann::json& json)
	{
		if (json.find("bufferViews") == json.end())
			return true;

		nlohmann::json& bufferViews = json["bufferViews"];
		if (!bufferViews.is_array())
			return false;

		m_bufferViews.resize(bufferViews.size());
		for (ui32 i = 0; i < bufferViews.size(); i++)
		{
			nlohmann::json& bufferView = bufferViews[i];

			// name
			if (!parseJsonValueString(m_bufferViews[i].m_name, bufferView, "name", false))
				return false;

			// buffer index
			if (!parseJsonValueUI32(m_bufferViews[i].m_bufferIdx, bufferView, "buffer", true))
				return false;

			// byteOffset
			if (!parseJsonValueUI32(m_bufferViews[i].m_byteOffset, bufferView, "byteOffset", true))
				return false;

			// byteLength
			if (!parseJsonValueUI32(m_bufferViews[i].m_byteLength, bufferView, "byteLength", true))
				return false;

			// byteStride
			if (!parseJsonValueUI32(m_bufferViews[i].m_byteStride, bufferView, "byteStride", false))
				return false;

			// target
			ui32 target;
			if (parseJsonValueUI32(target, bufferView, "target", true))
				m_bufferViews[i].m_target = static_cast<GltfBufferViewInfo::TargetType>(target);
			else
				return false;
		}

		return true;
	}

	bool GltfRes::loadMaterials(nlohmann::json& json)
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
			if (!parseJsonValueString(m_materials[i].m_name, material, "name", false))
				return false;

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
						if (!baseColorFactor[j].is_number())
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
				if (!parseJsonValueFloat(m_materials[i].m_pbr.m_metallicFactor, pbrMetallicRoughness, "metallicFactor", false))
					return false;

				// pbrMetallicRoughness.roughnessFactor
				if (!parseJsonValueFloat(m_materials[i].m_pbr.m_roughnessFactor, pbrMetallicRoughness, "roughnessFactor", false))
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
				if (!parseJsonValueFloat(m_materials[i].m_normalTexture.m_scale, normalTexture, "scale", false))
					return false;
			}

			// occlusion texture
			if (material.find("occlusionTexture") != material.end())
			{
				nlohmann::json& occlusionTexture = material["occlusionTexture"];
				if (!loadTextureInfo(m_materials[i].m_occlusionTexture, occlusionTexture))
					return false;

				// scale
				if (!parseJsonValueFloat(m_materials[i].m_occlusionTexture.m_strength, occlusionTexture, "strength", false))
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
					if (!emissiveFactor[j].is_number())
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
			if (!parseJsonValueFloat(m_materials[i].m_alphaCutoff, material, "alphaCutoff", false))
				return false;

			// doubleSided
			if (!parseJsonValueBool(m_materials[i].m_isDoubleSided, material, "doubleSided", false))
				return false;

			// TODO: materials[i]["extensions"]
			// TODO: materials[i]["extras"]
		}

		return true;
	}

	bool GltfRes::loadTextureInfo(GltfMaterialInfo::Texture& texture, nlohmann::json& json)
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
		if (!parseJsonValueI32(texture.m_texCoord, json, "texCoord", false))
			return false;

		// TODO json["extensions"]
		// TODO json["extras"]

		return true;
	}

	bool GltfRes::loadImages(nlohmann::json& json)
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
			if (!parseJsonValueString(m_images[i].m_name, image, "name", false))
				return false;

			// uri
			if (parseJsonValueString(m_images[i].m_uri, image, "uri", false))
				m_images[i].m_uri = PathUtil::GetFileDirPath(m_path.getPath()) + m_images[i].m_uri;
			else
				return false;

			// mimeType
			if (!parseJsonValueString(m_images[i].m_mimeType, image, "mimeType", false))
				return false;

			// bufferView
			if (!parseJsonValueI32(m_images[i].m_bufferView, image, "bufferView", false))
				return false;
		}

		// TODO: Handle dependencies between mimeType and bufferView
		// TODO: Handle the fact that we want an uri OR a bufferView
		// TODO: images[i]["extensions"]
		// TODO: images[i]["extras"]

		return true;
	}

	bool GltfRes::loadSamplers(nlohmann::json& json)
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
			if (!parseJsonValueString(m_samplers[i].m_name, sampler, "name", false))
				return false;

			i32 iValue;

			// magFilter
			if (parseJsonValueI32(iValue, sampler, "magFilter", false))
				m_samplers[i].m_magFilter = GltfSamplerInfo::MagFilter(iValue);
			else
				return false;

			// minFilter
			if (parseJsonValueI32(iValue, sampler, "minFilter", false))
				m_samplers[i].m_minFilter = GltfSamplerInfo::MinFilter(iValue);
			else
				return false;

			// wrapS
			if (parseJsonValueI32(iValue, sampler, "wrapS", false))
				m_samplers[i].m_wrapS = GltfSamplerInfo::WrappingMode(iValue);
			else
				return false;

			// wrapT
			if (parseJsonValueI32(iValue, sampler, "wrapT", false))
				m_samplers[i].m_wrapT = GltfSamplerInfo::WrappingMode(iValue);
			else
				return false;

			// TODO: samplers[i]["extensions"]
			// TODO: samplers[i]["extras"]
		}

		return true;
	}

	bool GltfRes::loadTextures(nlohmann::json& json)
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
			if (!parseJsonValueString(m_textures[i].m_name, texture, "name", false))
				return false;

			// sampler
			if (!parseJsonValueI32(m_textures[i].m_sampler, texture, "sampler", false))
				return false;

			// source
			if (!parseJsonValueI32(m_textures[i].m_source, texture, "source", false))
				return false;
		}

		// TODO: textures[i]["extensions"]
		// TODO: textures[i]["extras"]

		return true;
	}

	// build echo node
	Node* GltfRes::build()
	{
		vector<Node*>::type nodes;
		for (GltfSceneInfo& scene : m_scenes)
		{
			for (ui32 rootNodeIdx : scene.m_nodes)
			{
				Node* node = createNode(nullptr, rootNodeIdx);

				nodes.push_back(node);
			}
		}

		// only have one root node
		if (nodes.size() == 1)
			return nodes[0];

		// have multi root node
		if (nodes.size() > 1)
		{
			Node* node = Class::create<Node*>("GltfScene");
			for (Node* child : nodes)
			{
				child->setParent(node);
			}

			return node;
		}

		// no root node
		return nullptr;
	}

	Node* GltfRes::createNode(Node* parent, int idx)
	{
		if (idx < 0 || idx >= (int)m_nodes.size())
			return nullptr;

		Node* node = nullptr;

		// create node base info
		GltfNodeInfo& info = m_nodes[idx];
		if (info.m_mesh != -1)
		{
			GltfMeshInfo& meshInfo = m_meshes[info.m_mesh];
			if (meshInfo.m_primitives.size() > 1)
			{
				// create multi mesh nodes
				node = Class::create<GltfMesh*>("Node");
				if (node)
				{
					for (size_t i = 0; i < meshInfo.m_primitives.size(); i++)
					{
						GltfMesh* mesh = Class::create<GltfMesh*>("GltfMesh");
						mesh->setName(info.m_name.empty() ? node->getClassName() : info.m_name);
						mesh->setGeometryData(this, info.m_mesh, i);
						mesh->setParent(node);
					}
				}
			}
			else if(meshInfo.m_primitives.size() == 1)
			{
				// create one mesh node
				GltfMesh* mesh = Class::create<GltfMesh*>("GltfMesh");
				mesh->setGeometryData(this, info.m_mesh, 0);

				node = mesh;
			}
		}
		else
		{
			node = Class::create<Node*>("Node");
		}

		// set node property
		node->setName(info.m_name.empty() ? node->getClassName() : info.m_name);
		if(parent)
			node->setParent(parent);

		// create children
		for (ui32 childIdx : info.m_children)
		{
			createNode(node, childIdx);
		}

		return node;
	}
}