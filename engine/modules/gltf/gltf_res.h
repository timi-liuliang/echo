#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/MathLib.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/scene/node.h"
#include "engine/core/resource/Res.h"
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

	struct GltfPrimitive
	{
		ui32				m_indices = -1;		// index to accessor containing the indices
		ui32				m_material = -1;	// index to the material
		enum Mode
		{
			Points = 0,
			Lines,
			LineLoop,
			LineStrip,
			Triangles,
			TriangleStrip,
			TriangleFan
		}					m_mode = Triangles; // each attribute is mapped with his name and accessor index to the data
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
		String				m_name;
		String				m_uri;
		enum class UriType
		{
			Uri = 0,
			Data,
			Blob
		}					m_uriType = UriType::Uri;
		i32					m_byteLength;
		MemoryReader*		m_data = nullptr;
	};

	struct GltfBufferViewInfo
	{
		String		m_name;
		ui32		m_bufferIdx;
		ui32		m_byteOffset;
		ui32		m_byteLength;
		ui32		m_byteStride;
		enum class TargetType : ui16
		{
			None = 0,
			ArrayBuffer = 34962,
			ElementArrayBuffer = 34963
		}			m_target;
	};

	struct GltfAccessorInfo
	{
		String				m_name;
		i32					m_bufferView = -1;
		i32					m_byteOffset = 0;
		enum ComponentType
		{
			Byte = 5120,
			UnsignedByte,
			Short,
			UnsignedShort,
			UnsignedInt,
			Float
		}					m_componentType;
		bool				m_normalized = false;
		ui32				m_count;
		enum Type
		{
			Scalar = 0,
			Vec2,
			Vec3,
			Vec4,
			Mat2,
			Mat3,
			Mat4
		}					m_type;
	};

	struct GltfMaterialInfo
	{
		String		m_name;

		struct Texture
		{
			i32		m_index = -1;
			i32		m_texCoord = 0;
		};

		// base color texture and metalic roughness texture
		struct Pbr
		{
			float	m_baseColorFactor[4] = { 1.f, 1.f, 1.f, 1.f };
			Texture	m_baseColorTexture;
			float	m_metallicFactor = 1.f;
			float	m_roughnessFactor = 1.f;
			Texture	m_metallicRoughnessTexture;

		}			m_pbr;

		// normal texture
		struct NormalTexture : Texture
		{
			float	m_scale;
		}			m_normalTexture;

		// occlusion texture
		struct OcclusionTexture : Texture
		{
			float	m_strength = 1.f;
		}			m_occlusionTexture;

		// emissive texture
		float		m_emissiveFactor[3] = { 0.f, 0.f, 0.f };
		Texture		m_emissiveTexture;

		// alpha mode
		enum AlphaMode
		{
			Opaque,
			Mask,
			Blend
		}			m_alphaMode = Opaque;
		float		m_alphaCutoff = 0.5f;

		// is doublesided
		bool		m_isDoubleSided = false;
	};

	struct GltfImageInfo
	{
		String	m_name;
		String	m_uri;
		String	m_mimeType;
		i32		m_bufferView = -1;
	};

	struct GltfSamplerInfo
	{
		String		m_name;

		// mag filter
		enum class MagFilter
		{
			None,
			Nearest = 9728,
			Linear = 9729
		}			m_magFilter = MagFilter::None;

		// min filter
		enum class MinFilter
		{
			None,
			Nearest = 9728,
			Linear = 9729,
			NearestMipMapNearest = 9984,
			LinearMipMapNearest = 9985,
			NearestMipMapLinear = 9986,
			LinearMipMapLinear = 9987
		}				m_minFilter = MinFilter::None;

		// wrapping mode
		enum WrappingMode
		{
			ClampToEdge	= 33071,
			MirroredRepeat = 33648,
			Repeat = 10497
		};

		WrappingMode	m_wrapS = Repeat;
		WrappingMode	m_wrapT = Repeat;
	};

	struct GltfTextureInfo
	{
		String		m_name;
		i32			m_sampler = -1;
		i32			m_source  = -1;
	};

	struct GltfRes : public Res
	{
		GltfMetaInfo						m_metaInfo;
		vector<GltfSceneInfo>::type			m_scenes;
		vector<GltfMeshInfo>::type			m_meshes;
		vector<GltfNodeInfo>::type			m_nodes;
		vector<GltfBufferInfo>::type		m_buffers;
		vector<GltfBufferViewInfo>::type	m_bufferViews;
		vector<GltfAccessorInfo>::type		m_accessors;
		vector<GltfMaterialInfo>::type		m_materials;
		vector<GltfImageInfo>::type			m_images;
		vector<GltfSamplerInfo>::type		m_samplers;
		vector<GltfTextureInfo>::type		m_textures;

		// create
		static GltfRes* create(const ResourcePath& path);

		// build echo node
		Node* build();

	private:
		GltfRes(const ResourcePath& path);
		~GltfRes();
		bool load();
		bool loadAsset(nlohmann::json& json);
		bool loadScenes(nlohmann::json& json);
		bool loadMeshes(nlohmann::json& json);
		bool loadNodes(nlohmann::json& json);
		bool loadBuffers(nlohmann::json& json);
		bool loadAccessors(nlohmann::json& json);
		bool loadBufferViews(nlohmann::json& json);
		bool loadBufferData(GltfBufferInfo& buffer);
		bool loadMaterials(nlohmann::json& json);
		bool loadTextureInfo(GltfMaterialInfo::Texture& texture, nlohmann::json& json);
		bool loadImages(nlohmann::json& json);
		bool loadSamplers(nlohmann::json& json);
		bool loadTextures(nlohmann::json& json);
		Node* createNode(Node* parent, int idx);
	};
	typedef Echo::ResRef<Echo::GltfRes> GltfResPtr;
}