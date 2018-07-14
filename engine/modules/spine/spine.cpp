#include "spine.h"
#include "engine/core/log/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/ShaderProgramRes.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"
#include <spine/spine.h>
#include <spine/extension.h>
#include "AttachmentLoader.h"

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
		, m_animations("", nullptr)
		, m_spSkeletonData(nullptr)
		, m_spSkeleton(nullptr)
		, m_spAnimState(nullptr)
		, m_attachmentLoader(nullptr)
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
	void Spine::update()
	{
		if (isNeedRender())
		{
			float delta = Engine::instance()->getFrameTime();

			Render::update();

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

			// 更新位置颜色数据
			//for (int v = 0, w = 0, vn = attachmentVertices->m_verticesData.size(); v < vn; ++v, w += 2)
			//{
			//	SpineVertexFormat* vertex = attachmentVertices->m_verticesData.data() + v;
			//	vertex->m_position.x = m_worldVertices[w];
			//	vertex->m_position.y = m_worldVertices[w + 1];
			//	vertex->m_position.z = 0.f;
			//	vertex->m_diffuse = Color::WHITE;
			//}

			// diffuse
			for (size_t v = 0; v < attachmentVertices->m_verticesData.size(); v++)
			{
				SpineVertexFormat* vertex = attachmentVertices->m_verticesData.data() + v;
				vertex->m_position.z = 0.f;
				vertex->m_diffuse = Color::WHITE;
			}

			// 混合状态
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

			attachmentVertices->submitToRenderQueue(this);
		}
	}

	void Spine::clear()
	{
		clearRenderable();
	}

	void Spine::clearRenderable()
	{
	}
}