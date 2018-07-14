#include "live2d_cubism.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/ShaderProgramRes.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Engine.h"

// Ä¬ÈÏ²ÄÖÊ
static const char* g_live2dDefaultMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
<VS>#version 100

attribute vec3 a_Position;
attribute vec2 a_UV;

uniform mat4 u_WorldViewProjMatrix;

varying vec2 texCoord;

void main(void)
{
	vec4 position = u_WorldViewProjMatrix * vec4(a_Position, 1.0);
	gl_Position = position;

	texCoord = a_UV;
}
</VS>
<PS>#version 100

uniform sampler2D u_BaseColorSampler;
varying mediump vec2 texCoord;

void main(void)
{
	mediump vec4 textureColor = texture2D(u_BaseColorSampler, texCoord);
	gl_FragColor = textureColor;
}
	</PS>
	<BlendState>
		<BlendEnable value = "true" />
		<SrcBlend value = "BF_SRC_ALPHA" />
		<DstBlend value = "BF_INV_SRC_ALPHA" />
	</BlendState>
	<RasterizerState>
		<CullMode value = "CULL_NONE" />
	</RasterizerState>
	<DepthStencilState>
		<DepthEnable value = "false" />
		<WriteDepth value = "false" />
	</DepthStencilState>
</Shader>
)";

namespace Echo
{
	Live2dCubism::Live2dCubism()
		: m_mocRes("", ".moc3")
		, m_textureRes("", ".png")
		, m_curMotionRes("", ".json")
		, m_curMotion(nullptr)
		, m_mocMemory(nullptr)
		, m_moc(nullptr)
		, m_model(nullptr)
		, m_tableSize(0)
		, m_tableMemory(nullptr)
		, m_table(nullptr)
		, m_modelSize(0)
		, m_modelMemory(nullptr)
		, m_mesh(nullptr)
		, m_materialDefault(nullptr)
		, m_renderable(nullptr)
	{
		m_materialDefault = ECHO_CREATE_RES(Material);
		m_materialDefault->setShaderContent(g_live2dDefaultMaterial);
		m_materialDefault->setRenderStage("Transparent");
	}

	Live2dCubism::~Live2dCubism()
	{
		clear();
	}

	void Live2dCubism::bindMethods()
	{
		CLASS_BIND_METHOD(Live2dCubism, getMoc, DEF_METHOD("getMoc"));
		CLASS_BIND_METHOD(Live2dCubism, setMoc, DEF_METHOD("setMoc"));
		CLASS_BIND_METHOD(Live2dCubism, getTextureRes, DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(Live2dCubism, setTextureRes, DEF_METHOD("setTextureRes"));
		CLASS_BIND_METHOD(Live2dCubism, getMotionRes, DEF_METHOD("getMotionRes"));
		CLASS_BIND_METHOD(Live2dCubism, setMotionRes, DEF_METHOD("setMotionRes"));
		CLASS_BIND_METHOD(Live2dCubism, getMaterial, DEF_METHOD("getMaterial"));
		CLASS_BIND_METHOD(Live2dCubism, setMaterial, DEF_METHOD("setMaterial"));

		CLASS_REGISTER_PROPERTY(Live2dCubism, "Material", Variant::Type::Object, "getMaterial", "setMaterial");
		CLASS_REGISTER_PROPERTY(Live2dCubism, "Texture", Variant::Type::ResourcePath, "getTextureRes", "setTextureRes");
		CLASS_REGISTER_PROPERTY(Live2dCubism, "Moc", Variant::Type::ResourcePath, "getMoc", "setMoc");
		CLASS_REGISTER_PROPERTY(Live2dCubism, "Motion", Variant::Type::ResourcePath, "getMotionRes", "setMotionRes");
	}

	// parse paramters
	void Live2dCubism::parseParams()
	{
		int paramerCount = csmGetParameterCount(m_model);
		if (paramerCount > 0)
		{
			m_params.resize(paramerCount);
			const char** ids = csmGetParameterIds(m_model);
			const float* curValues = csmGetParameterValues(m_model);
			const float* defaultValues = csmGetParameterDefaultValues(m_model);
			const float* minValues = csmGetParameterMinimumValues(m_model);
			const float* maxValues = csmGetParameterMaximumValues(m_model);
			for (int i = 0; i < paramerCount; i++)
			{
				Paramter& param = m_params[i];
				param.m_name = ids[i];
				param.m_value = curValues[i];
				param.m_defaultValue = defaultValues[i];
				param.m_minValue = minValues[i];
				param.m_maxValue = maxValues[i];
			}
		}
	}

	// parse parts
	void Live2dCubism::parseParts()
	{
		int partCount = csmGetPartCount(m_model);
		if (partCount > 0)
		{
			m_parts.resize(partCount);
			const char** ids = csmGetPartIds(m_model);
			const float* opacities = csmGetPartOpacities(m_model);
			for (int i = 0; i < partCount; i++)
			{
				Part& part = m_parts[i];
				part.m_name = ids[i];
				part.m_opacities = opacities[i];
			}
		}
	}

	// parse drawables
	void Live2dCubism::parseDrawables()
	{
		m_localAABB.reset();

		int drawableCount = csmGetDrawableCount(m_model);
		if (drawableCount > 0)
		{
			m_drawables.resize(drawableCount);
			const char** ids = csmGetDrawableIds(m_model);
			const csmFlags* constantFlags = csmGetDrawableConstantFlags(m_model);
			const csmFlags* dynamicFlags = csmGetDrawableDynamicFlags(m_model);
			const int* textureIndices = csmGetDrawableTextureIndices(m_model);
			const int* drawOrders = csmGetDrawableDrawOrders(m_model);
			const int* renderOrders = csmGetDrawableRenderOrders(m_model);
			const float* opacities = csmGetDrawableOpacities(m_model);
			const int*  maskCounts = csmGetDrawableMaskCounts(m_model);
			const int** masks = csmGetDrawableMasks(m_model);
			const int* vertexCounts = csmGetDrawableVertexCounts( m_model);
			const csmVector2** positions = csmGetDrawableVertexPositions( m_model);
			const csmVector2** uvs = csmGetDrawableVertexUvs( m_model);
			const int* indexCounts = csmGetDrawableIndexCounts(m_model);
			const unsigned short** indices = csmGetDrawableIndices( m_model);
			for (int i = 0; i < drawableCount; i++)
			{
				// reference
				Drawable& drawable = m_drawables[i];
				drawable.reset();

				drawable.m_name = ids[i];
				drawable.m_constantFlag = constantFlags[i];
				drawable.m_dynamicFlag = dynamicFlags[i];
				drawable.m_textureIndex = textureIndices[i];
				drawable.m_drawOrder = drawOrders[i];
				drawable.m_renderOrder = renderOrders[i];
				drawable.m_opacitie = opacities[i];
				ui32 maskCount = maskCounts[i];
				for (ui32 j = 0; j < maskCount; j++)
				{
					drawable.m_masks.push_back(masks[i][j]);
				}
				
				// vertexs
				ui32 vertexCount = vertexCounts[i];
				drawable.m_box.reset();
				for (ui32 j = 0; j < vertexCount; j++)
				{
					const csmVector2& pos = positions[i][j];
					const csmVector2& uv = uvs[i][j];

					VertexFormat vert;
					vert.m_position = Vector3(pos.X * m_canvas.m_pixelsPerUnit, pos.Y * m_canvas.m_pixelsPerUnit, 0.f);
					vert.m_uv = Vector2(uv.X, 1.f-uv.Y);

					drawable.m_vertices.push_back( vert);
					drawable.m_box.addPoint(vert.m_position);
				}

				// calc local aabb
				m_localAABB.unionBox(drawable.m_box);

				// indices
				ui32 indeceCount = indexCounts[i];
				for (ui32 j = 0; j < indeceCount; j++)
				{
					drawable.m_indices.push_back( indices[i][j]);
				}
			}

			std::sort(m_drawables.begin(), m_drawables.end(), [](const Drawable& a, const Drawable& b) ->int {return a.m_renderOrder<b.m_renderOrder; });
		}
	}

	// parse canvas info
	void Live2dCubism::parseCanvasInfo()
	{
		csmVector2 sizeInPixels;
		csmVector2 originInPixels;
		float	   pixelsPerUnit;
		csmReadCanvasInfo(m_model, &sizeInPixels, &originInPixels, &pixelsPerUnit);

		m_canvas.m_width = sizeInPixels.X;
		m_canvas.m_height = sizeInPixels.Y;
		m_canvas.m_originInPixels = Vector2(originInPixels.X, originInPixels.Y);
		m_canvas.m_pixelsPerUnit = pixelsPerUnit;
	}

	// set moc
	void Live2dCubism::setMoc(const ResourcePath& res)
	{
		if (m_mocRes.setPath(res.getPath()))
		{
			clear();

			m_mocMemory = EchoNew(MemoryReaderAlign(m_mocRes.getPath(), csmAlignofMoc));
			if (m_mocMemory->getSize())
			{
				m_moc = csmReviveMocInPlace(m_mocMemory->getData<void*>(), m_mocMemory->getSize());
				if (m_moc)
				{
					m_modelSize = csmGetSizeofModel(m_moc);
					m_modelMemory = EchoMallocAlign(m_modelSize, csmAlignofModel);
					m_model = csmInitializeModelInPlace(m_moc, m_modelMemory, m_modelSize);

					m_tableSize = csmGetSizeofModelHashTable(m_model);
					m_tableMemory = EchoMalloc(m_tableSize);
					m_table = csmInitializeModelHashTableInPlace(m_model, m_tableMemory, m_tableSize);

					parseCanvasInfo();
					parseParams();
					parseParts();
					parseDrawables();

					buildRenderable();
				}
			}
		}
	}

	// set texture res path
	void Live2dCubism::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			m_materialDefault->setTexture("u_BaseColorSampler", m_textureRes.getPath());
		}
	}

	// set anim res path
	void Live2dCubism::setMotionRes(const ResourcePath& path)
	{
		if (m_curMotionRes.setPath(path.getPath()))
		{
			auto it = m_motions.find(path.getPath());
			if (it == m_motions.end())
			{
				m_curMotion = EchoNew(Live2dCubismMotion(m_curMotionRes));
				m_curMotion->play();
				m_motions[path.getPath()] = m_curMotion;
			}
			else
			{
				m_curMotion = it->second;
				m_curMotion->play();
			}
		}
	}

	// set parameter value
	void Live2dCubism::setParameter(const String& name, float value)
	{
		float* curValues = csmGetParameterValues(m_model);
		for (size_t i = 0; i < m_params.size(); i++)
		{
			if (m_params[i].m_name == name)
			{
				curValues[i] = value;
				m_params[i].m_value = value;
			}
		}
	}

	// set material
	void Live2dCubism::setMaterial(Object* material)
	{
		m_material = (Material*)material;
	}

	// build drawable
	void Live2dCubism::buildRenderable()
	{
		if (!m_textureRes.getPath().empty() && !m_drawables.empty())
		{
			VertexArray	vertices;
			IndiceArray	indices;
			buildMeshDataByDrawables(vertices, indices);

			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(indices.size(), indices.data());
			m_mesh->updateVertexs(define, vertices.size(), (const Byte*)vertices.data(), m_localAABB);

			m_renderable = Renderable::create(m_mesh, m_materialDefault, this);
		}
	}

	// update per frame
	void Live2dCubism::update()
	{
		if (isNeedRender())
		{
			Render::update();

			if (m_model && m_renderable)
			{
				if (m_curMotion)
				{
					m_curMotion->tick(Engine::instance()->getFrameTime(), m_model, m_table);
				}

				csmUpdateModel((csmModel*)m_model);

				updateMeshBuffer();

				m_renderable->submitToRenderQueue();
			}
		}
	}

	// build mesh data by drawables data
	void Live2dCubism::buildMeshDataByDrawables(VertexArray& oVertices, IndiceArray& oIndices)
	{
		for (Drawable& drawable : m_drawables)
		{
			ui32 vertOffset = oVertices.size();

			// vertices
			for (VertexFormat& vert : drawable.m_vertices)
				oVertices.push_back(vert);

			// indices
			for (int idx : drawable.m_indices)
				oIndices.push_back(idx + vertOffset);
		}
	}

	// update vertex buffer
	void Live2dCubism::updateMeshBuffer()
	{
		parseDrawables();

		VertexArray	vertices;
		IndiceArray	indices;
		buildMeshDataByDrawables(vertices, indices);

		MeshVertexFormat define;
		define.m_isUseUV = true;

		m_mesh->updateIndices(indices.size(), indices.data());
		m_mesh->updateVertexs( define, vertices.size(), (const Byte*)vertices.data(), m_localAABB);
	}

	void Live2dCubism::clear()
	{
		EchoSafeDelete(m_mocMemory, MemoryReaderAlign);
		EchoSafeFreeAlign(m_mocMemory, csmAlignofModel);
		EchoSafeFree(m_tableMemory);
		EchoSafeDeleteMap(m_motions, Live2dCubismMotion);
		m_curMotion = nullptr;

		clearRenderable();
	}

	void Live2dCubism::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeRelease(m_mesh);
	}
}