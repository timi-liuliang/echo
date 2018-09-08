#include "spine.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "interface/renderer.h"
#include "interface/ShaderProgramRes.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/math/color.h"
#include "engine/core/main/Engine.h"
#include <spine/spine.h>
#include <spine/extension.h>
#include "AttachmentLoader.h"

// spine default material
static const char* g_spinDefaultMaterial = R"(
<?xml version = "1.0" encoding = "utf-8"?>
<Shader>
	<VS>#version 100

		attribute vec3 a_Position;
		attribute vec4 a_Color;
		attribute vec2 a_UV;

		uniform mat4 u_WorldViewProjMatrix;

		varying vec4 v_Color;
		varying vec2 v_UV;

		void main(void)
		{
			vec4 position = u_WorldViewProjMatrix * vec4(a_Position, 1.0);
			gl_Position = position;

			v_Color = a_Color;
			v_UV = a_UV;
		}
	</VS>
	<PS>#version 100

		precision mediump float;

		uniform sampler2D u_BaseColorSampler;

		varying vec4	  v_Color;
		varying vec2	  v_UV;

		void main(void)
		{
			vec4 textureColor = texture2D(u_BaseColorSampler, v_UV);
			gl_FragColor = textureColor * v_Color;
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
	static void animationCallback(spAnimationState* state, spEventType type, spTrackEntry* entry, spEvent* event)
	{
		int  a = 10;
	}

	static void trackEntryCallback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount)
	{
		//((SkeletonAnimation*)state->rendererObject)->onTrackEntryEvent(trackIndex, type, event, loopCount);
	}

	Spine::Spine()
		: m_spinRes("", ".json")
		, m_atlasRes("", ".atlas")
		, m_animations("")
		, m_spSkeletonData(nullptr)
		, m_spSkeleton(nullptr)
		, m_spAnimState(nullptr)
		, m_attachmentLoader(nullptr)
		, m_mesh(nullptr)
		, m_material(nullptr)
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
		CLASS_BIND_METHOD(Spine, getAtlas,DEF_METHOD("getAtlas"));
		CLASS_BIND_METHOD(Spine, setAtlas, DEF_METHOD("setAtlas"));
		CLASS_BIND_METHOD(Spine, setAnim, DEF_METHOD("setAnim"));
		CLASS_BIND_METHOD(Spine, getAnim, DEF_METHOD("getAnim"));

		CLASS_REGISTER_PROPERTY(Spine, "Spin", Variant::Type::ResourcePath, "getSpin", "setSpin");
		CLASS_REGISTER_PROPERTY(Spine, "Atlas", Variant::Type::ResourcePath, "getAtlas", "setAtlas");
		CLASS_REGISTER_PROPERTY(Spine, "Anim", Variant::Type::StringOption, "getAnim", "setAnim");
	}

	// set moc
	void Spine::setSpin(const ResourcePath& res)
	{
		if (m_spinRes.setPath(res.getPath()) && m_attachmentLoader)
		{
			// json
			spSkeletonJson* json = spSkeletonJson_createWithLoader(m_attachmentLoader);
			json->scale = 1.f;

			// skeleton data
			m_spSkeletonData = spSkeletonJson_readSkeletonDataFile(json, PathUtil::GetRenameExtFile(res.getPath().c_str(), ".json").c_str());
			if (m_spSkeletonData)
			{
				m_spSkeleton = spSkeleton_create(m_spSkeletonData);

				// animation names
				for (int i = 0; i < m_spSkeletonData->animationsCount; i++)
				{
					spAnimation* spAnim = m_spSkeletonData->animations[i];
					m_animations.addOption(spAnim->name);
				}

				// animation state
				m_spAnimState = spAnimationState_create(spAnimationStateData_create(m_spSkeleton->data));
				m_spAnimState->rendererObject = this;
				m_spAnimState->listener = animationCallback;
			}
			else
			{
				EchoLogError("Read spine skeleton data [%s] failed", res.getPath().c_str());
			}
		}
	}

	// set atlas
	void Spine::setAtlas(const ResourcePath& res)
	{
		if (m_atlasRes.setPath(res.getPath()))
		{
			// atlas
			m_spAtlas = spAtlas_createFromFile(m_atlasRes.getPath().c_str(), nullptr);

			// attachment
			m_attachmentLoader = &(EchoAttachmentLoader_create(m_spAtlas)->super);

			// animation
			if (!m_spinRes.getPath().empty())
				setSpin(m_spinRes.getPath());
		}
	}

	// play anim
	void Spine::setAnim(const StringOption& animName)
	{
		if (m_animations.setValue(animName.getValue()))
		{
			spAnimationState_setAnimationByName(m_spAnimState, 0, m_animations.getValue().c_str(), true);
		}
	}

	// update per frame
	void Spine::update_self()
	{
		if (isNeedRender())
		{
			float delta = Engine::instance()->getFrameTime();

			if (m_spSkeleton && m_spAnimState)
			{
				spSkeleton_update(m_spSkeleton, delta);
				spAnimationState_update(m_spAnimState, delta);
				spAnimationState_apply(m_spAnimState, m_spSkeleton);
				spSkeleton_updateWorldTransform(m_spSkeleton);

				submitToRenderQueue();
			}
		}
	}

	// submit to render
	void Spine::submitToRenderQueue()
	{
		m_batch.clear();
		for (int i = 0; i < m_spSkeleton->slotsCount; i++)
		{
			spSlot* slot = m_spSkeleton->drawOrder[i];
			if (!slot->attachment)
				continue;

			AttachmentVertices* attachmentVertices = nullptr;
			switch (slot->attachment->type)
			{
			case SP_ATTACHMENT_REGION:
				{
					spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
					attachmentVertices = (AttachmentVertices*)attachment->rendererObject;

					int stride = sizeof(SpineVertexFormat) / 4;
					spRegionAttachment_computeWorldVertices(attachment, slot->bone, (float*)attachmentVertices->m_verticesData.data(), 0, stride);
				}
				break;

			case SP_ATTACHMENT_MESH:
				{
					spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
					attachmentVertices = (AttachmentVertices*)attachment->rendererObject;

					int count = attachmentVertices->m_verticesData.size() * sizeof(SpineVertexFormat) / 4;
					int stride = sizeof(SpineVertexFormat) / 4;
					spVertexAttachment_computeWorldVertices(SUPER(attachment), slot, 0, count, (float*)attachmentVertices->m_verticesData.data(), 0, stride);
				}
				break;
			default:
				{
				continue;
				}
			}

			// diffuse
			for (size_t v = 0; v < attachmentVertices->m_verticesData.size(); v++)
			{
				SpineVertexFormat* vertex = attachmentVertices->m_verticesData.data() + v;
				vertex->m_position.z = 0.f;
				vertex->m_diffuse = Color(slot->color.r, slot->color.g, slot->color.b, slot->color.a);
			}

			// blend mode
			switch (slot->data->blendMode)
			{
			case SP_BLEND_MODE_ADDITIVE:
			{
				break;
			}
			case SP_BLEND_MODE_MULTIPLY:
			{
				break;
			}
			case SP_BLEND_MODE_SCREEN:
			{
				break;
			}
			default:
			{
				break;
			}
			}

			m_batch.merge( *attachmentVertices);
		}

		updateRenderable();
		m_renderable->submitToRenderQueue();
	}

	// update renderable
	void Spine::updateRenderable()
	{
		if (!m_renderable)
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(m_batch.m_indicesData.size(), sizeof(Word), m_batch.m_indicesData.data());
			m_mesh->updateVertexs(define, m_batch.m_verticesData.size(), (const Byte*)m_batch.m_verticesData.data(), AABB());

			m_material = ECHO_CREATE_RES(Material);
			m_material->setShaderContent("echo_spine_default_shader", g_spinDefaultMaterial);
			m_material->setRenderStage("Transparent");

			m_material->setTexture("u_BaseColorSampler", m_batch.m_texture);

			m_renderable = Renderable::create(m_mesh, m_material, this);
		}
		else
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices(m_batch.m_indicesData.size(), sizeof(Word), m_batch.m_indicesData.data());
			m_mesh->updateVertexs(define, m_batch.m_verticesData.size(), (const Byte*)m_batch.m_verticesData.data(), AABB());
		}
	}

	void Spine::clear()
	{

	}
}