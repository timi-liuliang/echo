#include "sprite.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/Renderer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"

// Ä¬ÈÏ²ÄÖÊ
static const char* g_spriteDefaultMaterial = R"(<?xml version = "1.0" encoding = "utf-8"?>
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
</Shader>
)";

namespace Echo
{
	Sprite::Sprite()
		: m_textureRes("", ".png")
		, m_mesh(nullptr)
		, m_material(nullptr)
		, m_renderable(nullptr)
		, m_width(0)
		, m_height(0)
	{
	}

	Sprite::~Sprite()
	{
		clear();
	}

	void Sprite::bindMethods()
	{
		CLASS_BIND_METHOD(Sprite, getTextureRes,	DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(Sprite, setTextureRes,	DEF_METHOD("setTextureRes"));
		CLASS_BIND_METHOD(Sprite, getWidth,		DEF_METHOD("getWidth"));
		CLASS_BIND_METHOD(Sprite, setWidth,		DEF_METHOD("setWidth"));
		CLASS_BIND_METHOD(Sprite, getHeight,		DEF_METHOD("getHeight"));
		CLASS_BIND_METHOD(Sprite, setHeight,		DEF_METHOD("setHeight"));

		CLASS_REGISTER_PROPERTY(Sprite, "Width", Variant::Type::Int, "getWidth", "setWidth");
		CLASS_REGISTER_PROPERTY(Sprite, "Height", Variant::Type::Int, "getHeight", "setHeight");
		CLASS_REGISTER_PROPERTY(Sprite, "Texture", Variant::Type::ResourcePath, "getTextureRes", "setTextureRes");
	}

	// set texture res path
	void Sprite::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			buildRenderable();
		}
	}

	void Sprite::setWidth(i32 width) 
	{ 
		if (m_width != width)
		{
			m_width = width;

			buildRenderable();
		}
	}

	// width
	void Sprite::setHeight(i32 height) 
	{
		if (m_height != height)
		{
			m_height = height;

			buildRenderable();
		}
	}

	// build drawable
	void Sprite::buildRenderable()
	{
		if (!m_textureRes.getPath().empty())
		{
			clearRenderable();

			// material
			m_material = EchoNew(Material(Echo::StringUtil::Format("SpriteMaterial_%d", getId())));
			m_material->setShaderContent("echo_sprite_default_shader", g_spriteDefaultMaterial);
			m_material->setRenderStage("Transparent");

			m_material->setTexture("u_BaseColorSampler", m_textureRes.getPath());

			// mesh
			VertexArray	vertices;
			IndiceArray	indices;
			buildMeshData(vertices, indices);

			MeshVertexFormat define;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
			m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);

			m_renderable = Renderable::create(m_mesh, m_material, this);
		}
	}

	// update per frame
	void Sprite::update_self()
	{
		if (isNeedRender())
		{
			if (m_renderable)
			{
				m_matWVP = getWorldMatrix() * NodeTree::instance()->get2dCamera()->getViewProjMatrix();
				m_renderable->submitToRenderQueue();
			}
		}
	}

	// build mesh data by drawables data
	void Sprite::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		Texture* texture = m_material->getTexture(0);
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

		// calc aabb
		m_localAABB.reset();
		for (VertexFormat& vert : oVertices)
			m_localAABB.addPoint(vert.m_position);

		// indices
		oIndices.push_back(0);
		oIndices.push_back(1);
		oIndices.push_back(2);
		oIndices.push_back(0);
		oIndices.push_back(2);
		oIndices.push_back(3);
	}

	// update vertex buffer
	void Sprite::updateMeshBuffer()
	{
		VertexArray	vertices;
		IndiceArray	indices;
		buildMeshData(vertices, indices);

		MeshVertexFormat define;
		define.m_isUseUV = true;

		m_mesh->updateIndices(static_cast<ui32>(indices.size()), sizeof(Word), indices.data());
		m_mesh->updateVertexs(define, static_cast<ui32>(vertices.size()), (const Byte*)vertices.data(), m_localAABB);
	}

	void Sprite::clear()
	{
		clearRenderable();
	}

	void Sprite::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeDelete(m_mesh, Mesh);
	}
}
