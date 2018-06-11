#include "sprite_2d.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Engine.h"

// 默认材质
static const char* g_spriteDefaultMaterial = R"(<?xml version = "1.0" encoding = "utf-8"?>
<material>
<vs>#version 100

attribute vec3 a_Position;
attribute vec2 a_UV;

uniform mat4 u_WVPMatrix;

varying vec2 texCoord;

void main(void)
{
	vec4 position = u_WVPMatrix * vec4(a_Position, 1.0);
	gl_Position = position;
	
	texCoord = a_UV;
}
</vs>
<ps>#version 100

uniform sampler2D u_BaseColorSampler;

varying mediump vec2 texCoord;

void main(void)
{
	mediump vec4 textureColor = texture2D(u_BaseColorSampler, texCoord);
	gl_FragColor = textureColor;
}
	</ps>
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
	<SamplerState>
		<BiLinearMirror>
			<MinFilter value = "FO_LINEAR" />
			<MagFilter value = "FO_LINEAR" />
			<MipFilter value = "FO_NONE" />
			<AddrUMode value = "AM_CLAMP" />
			<AddrVMode value = "AM_CLAMP" />
		</BiLinearMirror>
	</SamplerState>
	<Texture>
		<stage no = "0" sampler = "BiLinearMirror" />
	</Texture>
</material>
)";

namespace Echo
{
	Sprite2D::Sprite2D()
		: m_textureRes("", ".png")
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
		, m_renderable(nullptr)
		, m_width(0)
		, m_height(0)
	{
	}

	Sprite2D::~Sprite2D()
	{
		clear();
	}

	void Sprite2D::bindMethods()
	{
		CLASS_BIND_METHOD(Sprite2D, getTextureRes,	DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(Sprite2D, setTextureRes,	DEF_METHOD("setTextureRes"));
		CLASS_BIND_METHOD(Sprite2D, getWidth,		DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(Sprite2D, setWidth,		DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(Sprite2D, getHeight,		DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(Sprite2D, setHeight,		DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(Sprite2D, "Width", Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(Sprite2D, "Height", Variant::Type::Int, "getHeight", "setHeight");
		CLASS_REGISTER_PROPERTY(Sprite2D, "Texture", Variant::Type::ResourcePath, "getTextureRes", "setTextureRes");
	}

	// set texture res path
	void Sprite2D::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			buildRenderable();
		}
	}

	void Sprite2D::setWidth(i32 width) 
	{ 
		if (m_width != width)
		{
			m_width = width;

			buildRenderable();
		}
	}

	// width
	void Sprite2D::setHeight(i32 height) 
	{
		if (m_height != height)
		{
			m_height = height;

			buildRenderable();
		}
	}

	// build drawable
	void Sprite2D::buildRenderable()
	{
		if (!m_textureRes.getPath().empty())
		{
			clearRenderable();

			// material
			m_materialInst = MaterialInst::create();
			m_materialInst->setOfficialMaterialContent(g_spriteDefaultMaterial);
			m_materialInst->setRenderStage("Transparent");
			m_materialInst->applyLoadedData();

			m_materialInst->setTexture("u_BaseColorSampler", m_textureRes.getPath());

			// mesh
			VertexArray	vertices;
			IndiceArray	indices;
			buildMeshData(vertices, indices);

			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(indices.size(), indices.data());
			m_mesh->updateVertexs(define, vertices.size(), (const Byte*)vertices.data(), m_localAABB);

			m_renderable = Renderable::create(m_mesh, m_materialInst, this);
		}
	}

	// update per frame
	void Sprite2D::update()
	{
		if ( m_renderable)
		{
			m_matWVP = getWorldMatrix() * NodeTree::instance()->get2DCamera()->getViewProjMatrix();;
			m_renderable->submitToRenderQueue();
		}
	}

	// build mesh data by drawables data
	void Sprite2D::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		TextureRes*	texture = m_materialInst->getTexture(0);
		if (texture)
		{
			if(!m_width) m_width  = texture->getWidth();
			if(!m_height) m_height = texture->getHeight();
		}

		float hw = m_width * 0.5f;
		float hh = m_height * 0.5f;

		// vertices
		oVertices.push_back(VertexFormat(Vector3(-hw, -hh, 0.f), Vector2(0.f, 1.f)));
		oVertices.push_back(VertexFormat(Vector3(-hw,  hh, 0.f), Vector2(0.f, 0.f)));
		oVertices.push_back(VertexFormat(Vector3(hw,   hh, 0.f), Vector2(1.f, 0.f)));
		oVertices.push_back(VertexFormat(Vector3(hw,  -hh, 0.f), Vector2(1.f, 1.f)));

		// indices
		oIndices.push_back(0);
		oIndices.push_back(1);
		oIndices.push_back(2);
		oIndices.push_back(0);
		oIndices.push_back(2);
		oIndices.push_back(3);
	}

	// update vertex buffer
	void Sprite2D::updateMeshBuffer()
	{
		VertexArray	vertices;
		IndiceArray	indices;
		buildMeshData(vertices, indices);

		MeshVertexFormat define;
		define.m_isUseUV = true;

		m_mesh->updateIndices(indices.size(), indices.data());
		m_mesh->updateVertexs(define, vertices.size(), (const Byte*)vertices.data(), m_localAABB);
	}

	// 获取全局变量值
	void* Sprite2D::getGlobalUniformValue(const String& name)
	{
		void* value = Node::getGlobalUniformValue(name);
		if (value)
			return value;

		if (name == "u_WVPMatrix")
			return (void*)(&m_matWVP);

		return nullptr;
	}

	void Sprite2D::clear()
	{
		clearRenderable();
	}

	void Sprite2D::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeRelease(m_materialInst);
		EchoSafeRelease(m_mesh);
	}
}