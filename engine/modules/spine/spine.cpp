#include "spine.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/main/Root.h"
#include <spine/spine.h>

// 默认材质
static const char* g_spinDefaultMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
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
	Spine::Spine()
		: m_spinRes("", ".json")
		, m_mesh(nullptr)
		, m_materialInst(nullptr)
		, m_renderable(nullptr)
	{
	}

	Spine::~Spine()
	{
		clear();
	}

	void Spine::bindMethods()
	{
		CLASS_BIND_METHOD(Spine, getSpin, DEF_METHOD("getSpin"));
		CLASS_BIND_METHOD(Spine, setSpin, DEF_METHOD("setSpin"));

		CLASS_REGISTER_PROPERTY(Spine, "Spin", Variant::Type::ResourcePath, "getSpin", "setSpin");
	}

	// set moc
	void Spine::setSpin(const ResourcePath& res)
	{
		if (m_spinRes.setPath(res.getPath()))
		{
			int a = 10;
		}
	}

	// update per frame
	void Spine::update()
	{
		//if (m_model && m_renderable)
		//{
		//	m_matWVP = getWorldMatrix() * NodeTree::instance()->get2DCamera()->getViewProjMatrix();;

		//	if (m_curMotion)
		//	{
		//		m_curMotion->tick( Root::instance()->getFrameTime(), m_model, m_table);
		//	}

		//	csmUpdateModel((csmModel*)m_model);

		//	updateMeshBuffer();

		//	m_renderable->submitToRenderQueue();
		//}
	}

	// 获取全局变量值
	void* Spine::getGlobalUniformValue(const String& name)
	{
		void* value = Node::getGlobalUniformValue(name);
		if (value)
			return value;

		if (name == "u_WVPMatrix")
			return (void*)(&m_matWVP);

		return nullptr;
	}

	void Spine::clear()
	{
		//EchoSafeDelete(m_mocMemory, MemoryReaderAlign);
		//EchoSafeFreeAlign(m_mocMemory, csmAlignofModel);
		//EchoSafeFree(m_tableMemory);
		//EchoSafeDeleteMap(m_motions, Live2dCubismMotion);
		//m_curMotion = nullptr;

		clearRenderable();
	}

	void Spine::clearRenderable()
	{
		EchoSafeRelease(m_renderable);
		EchoSafeRelease(m_materialInst);
		EchoSafeRelease(m_mesh);
	}
}