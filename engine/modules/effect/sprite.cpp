#include "sprite.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Root.h"

// 默认材质
static const char* g_spriteDefaultMaterial = "\
<?xml version = \"1.0\" encoding = \"GB2312\"?>\
<material> \
<vs>#version 100\n\
\n\
attribute vec3 inPosition;\n\
attribute vec2 inTexCoord;\n\
\n\
uniform mat4 matWVP;\n\
\n\
varying vec2 texCoord;\n\
\n\
void main(void)\n\
{\n\
	vec4 position = matWVP * vec4(inPosition, 1.0);\n\
	gl_Position = position;\n\
	\n\
	texCoord = inTexCoord;\n\
}\n\
</vs>\
<ps>#version 100\n\
\n\
uniform sampler2D DiffuseSampler;\n\
varying mediump vec2 texCoord;\n\
\n\
void main(void)\n\
{\n\
	mediump vec4 textureColor = texture2D(DiffuseSampler, texCoord);\n\
	gl_FragColor = textureColor;\n\
}\n\
	</ps>\
	<BlendState>\
		<BlendEnable value = \"true\" />\
		<SrcBlend value = \"BF_SRC_ALPHA\" />\
		<DstBlend value = \"BF_INV_SRC_ALPHA\" />\
	</BlendState>\
	<RasterizerState>\
		<CullMode value = \"CULL_NONE\" />\
	</RasterizerState>\
	<DepthStencilState>\
		<DepthEnable value = \"false\" />\
		<WriteDepth value = \"false\" />\
	</DepthStencilState>\
	<SamplerState>\
		<BiLinearMirror>\
			<MinFilter value = \"FO_LINEAR\" />\
			<MagFilter value = \"FO_LINEAR\" />\
			<MipFilter value = \"FO_NONE\" />\
			<AddrUMode value = \"AM_CLAMP\" />\
			<AddrVMode value = \"AM_CLAMP\" />\
		</BiLinearMirror>\
	</SamplerState>\
	<Texture>\
		<stage no = \"0\" sampler = \"BiLinearMirror\" />\
	</Texture>\
</material>";

namespace Echo
{
	static void csmLogFunc(const char* message)
	{
		EchoLogError( message);
	}

	Sprite::Sprite()
		: m_textureRes("", ".png")
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
		, m_renderable(nullptr)
	{
	}

	Sprite::~Sprite()
	{
		clear();
	}

	void Sprite::bindMethods()
	{
		CLASS_BIND_METHOD(Sprite, getTextureRes, DEF_METHOD("getTextureRes"));
		CLASS_BIND_METHOD(Sprite, setTextureRes, DEF_METHOD("setTextureRes"));

		CLASS_REGISTER_PROPERTY(Sprite, "Texture", Variant::Type_ResourcePath, "getTextureRes", "setTextureRes");
	}

	// set texture res path
	void Sprite::setTextureRes(const ResourcePath& path)
	{
		if (m_textureRes.setPath(path.getPath()))
		{
			clearRenderable();
			buildRenderable();
		}
	}

	// build drawable
	void Sprite::buildRenderable()
	{
		if (!m_textureRes.getPath().empty())
		{
			VertexArray	vertices;
			IndiceArray	indices;
			buildMeshData(vertices, indices);

			Mesh::VertexDefine define;
			define.m_isUseDiffuseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->set(define, vertices.size(), (const Byte*)vertices.data(), indices.size(), indices.data(), m_localAABB);

			m_materialInst = MaterialInst::create();
			m_materialInst->setOfficialMaterialContent(g_spriteDefaultMaterial);
			m_materialInst->setRenderStage("Transparent");
			m_materialInst->applyLoadedData();

			m_materialInst->setTexture(0, m_textureRes.getPath());

			m_renderable = Renderable::create(m_mesh, m_materialInst, this);
		}
	}

	// update per frame
	void Sprite::update()
	{
		if ( m_renderable)
		{
			m_matWVP = getWorldMatrix() * NodeTree::instance()->get2DCamera()->getViewProjMatrix();;
			m_renderable->submitToRenderQueue();
		}
	}

	// build mesh data by drawables data
	void Sprite::buildMeshData(VertexArray& oVertices, IndiceArray& oIndices)
	{
		float hw = 255.5f;
		float hh = 255.5f;

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
	void Sprite::updateMeshBuffer()
	{
		VertexArray	vertices;
		IndiceArray	indices;
		buildMeshData(vertices, indices);

		m_mesh->updateIndices(indices.size(), indices.data());
		m_mesh->updateVertexs( vertices.size(), (const Byte*)vertices.data(), m_localAABB);
	}

	// 获取全局变量值
	void* Sprite::getGlobalUniformValue(const String& name)
	{
		if (name == "matWVP")
			return (void*)(&m_matWVP);

		return nullptr;
	}

	void Sprite::clear()
	{
		clearRenderable();
	}

	void Sprite::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeRelease(m_materialInst);
		EchoSafeRelease(m_mesh);
	}
}