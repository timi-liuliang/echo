#pragma once

#include "engine/core/memory/MemAllocDef.h"
#include "engine/core/math/Math.h"
#include "engine/core/io/IO.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/scene/node.h"
#include "engine/core/render/base/Material.h"
#include "engine/modules/anim/anim_property.h"
#include <nlohmann/json.hpp>

#ifdef ECHO_EDITOR_MODE

using namespace Echo;

namespace Gltf
{
	using GltfAttributes = std::unordered_map<Echo::String, Echo::ui32>;

	struct MetaInfo
	{
		String	m_copyright;
		String	m_generator;
		String	m_version;
		String	m_minversion;
	};

	struct SceneInfo
	{
		String				m_name;
		vector<ui32>::type	m_nodes;
	};

	struct Primitive
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
		MeshPtr			m_mesh;				// geometry Data for render
		ShaderProgramPtr    m_shader;
		MaterialPtr			m_materialInst;
	};

	struct MeshInfo
	{
		String					m_name;
		vector<Primitive>::type	m_primitives;
	};

	struct SkinInfo
	{
		String					m_name;
		i32						m_inverseBindMatrices = -1;
		vector<Matrix4>::type	m_inverseMatrixs;
		i32						m_skeleton;
		vector<i32>::type		m_joints;
	};

	struct NodeInfo
	{
		String				m_name;
		i32					m_camera = -1;
		i32					m_mesh = -1;
		i32					m_skin = -1;
		i32					m_parent = -1;
		vector<i32>::type	m_children;
		Quaternion			m_rotation = { 0, 0, 0, 1 };
		Vector3				m_scale = { 1, 1, 1 };
		Vector3				m_translation = { 0, 0, 0 };
	};

	struct BufferInfo
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
		MemoryReader* m_data = nullptr;

		// destructor
		~BufferInfo()
		{
			EchoSafeDelete(m_data, MemoryReader);
		}

		// get data
		void* getData(ui32 offset)
		{
			return m_data ? m_data->getData<char*>() + offset : nullptr;
		}
	};

	struct BufferViewInfo
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

	struct AccessorInfo
	{
		String				m_name;
		i32					m_bufferView = -1;
		i32					m_byteOffset = 0;
		enum ComponentType
		{
			Byte = 5120,
			UnsignedByte = 5121,
			Short = 5122,
			UnsignedShort = 5123,
			UnsignedInt = 5125,
			Float = 5126
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

	struct MaterialInfo
	{
		static MaterialInfo  DEFAULT;

		String		m_name;

		struct Texture
		{
			i32		m_index = -1;
			i32		m_texCoord = 0;
		};

		// base color texture and metalic roughness texture
		struct PbrMetalicRoughness
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
			float	m_scale = 1.f;
		}			m_normalTexture;

		// occlusion texture
		struct OcclusionTexture : Texture
		{
			float	m_strength = 1.f;
		}			m_occlusionTexture;

		// emissive texture
		struct EmissiveTexture : Texture
		{
			float	m_factor[3] = { 0.f, 0.f, 0.f };
		}			m_emissiveTexture;

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

	struct ImageInfo
	{
		String	m_name;
		String	m_uri;
		String	m_mimeType;
		i32		m_bufferView = -1;
	};

	struct SamplerInfo
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
			ClampToEdge = 33071,
			MirroredRepeat = 33648,
			Repeat = 10497
		};

		WrappingMode	m_wrapS = Repeat;
		WrappingMode	m_wrapT = Repeat;
	};

	struct TextureInfo
	{
		String		m_name;
		i32			m_sampler = -1;
		i32			m_source = -1;
	};

	struct AnimChannel
	{
		i32		m_node = -1;			// node idx
		//String	m_path;				// "translation" "rotation" "scale" "weights"
		enum class Path
		{
			Translation,
			Rotation,
			Scale,
			Weights,
		}		m_path;
		i32		m_sampler = -1;		// anim sampler index
	};

	struct AnimSampler
	{
		i32			m_input;
		i32			m_output;
		String		m_interpolation;	// "LINEAR" "STEP" "CATMULLROMSPLINE" "CUBICSPLINE"
	};

	struct AnimClip;
	struct AnimInfo
	{
		String							m_name;
		vector<AnimChannel>::type	m_channels;
		vector<AnimSampler>::type	m_samplers;
		//AnimClip* m_clip;
	};

	struct Loader
	{
		ResourcePath						m_path;
		MetaInfo						m_metaInfo;
		vector<SceneInfo>::type			m_scenes;
		vector<MeshInfo>::type			m_meshes;
		vector<SkinInfo>::type			m_skins;
		vector<NodeInfo>::type			m_nodes;
		vector<BufferInfo>::type		m_buffers;
		vector<BufferViewInfo>::type	m_bufferViews;
		vector<AccessorInfo>::type		m_accessors;
		vector<MaterialInfo>::type		m_materials;
		vector<ImageInfo>::type			m_images;
		vector<SamplerInfo>::type		m_samplers;
		vector<TextureInfo>::type		m_textures;
		vector<AnimInfo>::type			m_animations;

		// get node index of mesh
		i32 getNodeIdxByMeshIdx(i32 meshIdx);

		Loader();
		~Loader();
		bool load(const ResourcePath& path);
		bool loadAsset(nlohmann::json& json);
		bool loadScenes(nlohmann::json& json);
		bool loadNodes(nlohmann::json& json);
		bool loadBuffers(nlohmann::json& json);
		bool loadAccessors(nlohmann::json& json);
		bool loadBufferViews(nlohmann::json& json);
		bool loadBufferData(BufferInfo& buffer);
		bool loadMaterials(nlohmann::json& json);
		bool loadTextureInfo(MaterialInfo::Texture& texture, nlohmann::json& json);
		bool loadImages(nlohmann::json& json);
		bool loadSamplers(nlohmann::json& json);
		bool loadTextures(nlohmann::json& json);
		bool loadMeshes(nlohmann::json& json);
		bool loadSkins(nlohmann::json& json);
		bool loadAnimations(nlohmann::json& json);
		bool buildAnimationData();
		bool buildPrimitiveData(int meshIdx, int primitiveIdx);
		bool buildMaterial(int meshIdx, int primitiveIdx);

	private:
		// help function for get access data
		template<typename T> T getAccessData(AccessorInfo& access)
		{
			BufferViewInfo& bufferView = m_bufferViews[access.m_bufferView];
			BufferInfo& buffer = m_buffers[bufferView.m_bufferIdx];
			return (T)buffer.getData(bufferView.m_byteOffset + access.m_byteOffset);
		}

		// add key to property
		template<typename DataTypeT, typename AnimPropertyTypeT> void addKeyToAnimProperty(AccessorInfo& timeAccess, AccessorInfo& keyAccess, AnimProperty* animProperty)
		{
			DataTypeT* keyData = getAccessData<DataTypeT*>(keyAccess);
			float* timeData = getAccessData<float*>(timeAccess);
			for (ui32 i = 0; i < timeAccess.m_count; i++)
			{
				ui32 time = ui32(timeData[i] * 1000.f);
				((AnimPropertyTypeT*)animProperty)->addKey(time, keyData[i]);
			}
		}
	};
}
#endif
