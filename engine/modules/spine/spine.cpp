#include "spine.h"
#include "engine/core/util/LogManager.h"
#include "engine/core/scene/NodeTree.h"
#include "render/renderer.h"
#include "render/Material.h"
#include "engine/core/script/lua/luaex.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Root.h"
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
		, m_spSkeleton(nullptr)
		, m_spAnimState(nullptr)
	{
		m_worldVertices = new float[1000];
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
			// atlas
			String atlasRes = PathUtil::GetRenameExtFile(res.getPath(), ".atlas");
			m_spAtlas = spAtlas_createFromFile(atlasRes.c_str(), nullptr);

			// attachment
			m_attachmentLoader = &(EchoAttachmentLoader_create(m_spAtlas)->super);

			// json
			spSkeletonJson* json = spSkeletonJson_createWithLoader(m_attachmentLoader);
			json->scale = 1.f;

			// skeleton data
			spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, PathUtil::GetRenameExtFile(res.getPath().c_str(), ".json").c_str());
			m_spSkeleton = spSkeleton_create(skeletonData);

			// Animation
			m_spAnimState = spAnimationState_create(spAnimationStateData_create(m_spSkeleton->data));
			m_spAnimState->rendererObject = this;
			m_spAnimState->listener = animationCallback;
		}
	}

	// play anim
	void Spine::playAnim(const String& animName)
	{
		spAnimationState_setAnimationByName(m_spAnimState, 0, animName.c_str(), true);
	}

	// update per frame
	void Spine::update()
	{
		float delta = Root::instance()->getFrameTime();

		m_matWVP = getWorldMatrix()* NodeTree::instance()->get2DCamera()->getViewProjMatrix();

		if (m_spSkeleton && m_spAnimState)
		{
			spSkeleton_update(m_spSkeleton, delta);
			spAnimationState_update(m_spAnimState, delta);
			spAnimationState_apply(m_spAnimState, m_spSkeleton);
			spSkeleton_updateWorldTransform(m_spSkeleton);
		}

		submitToRenderQueue();
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
				spRegionAttachment_computeWorldVertices(attachment, slot->bone, m_worldVertices, 0, sizeof(SpineVertexFormat));
				attachmentVertices = (AttachmentVertices*)attachment->rendererObject;
				break;
			}
			case SP_ATTACHMENT_MESH:
			{
				spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
				spVertexAttachment_computeWorldVertices(SUPER(attachment), slot, 0, attachmentVertices->m_verticesData.size(), m_worldVertices, 0, sizeof(SpineVertexFormat));
				attachmentVertices = (AttachmentVertices*)attachment->rendererObject;
				break;
			}
			default:
			{
				continue;
			}
			}

			// 更新位置颜色数据
			for (int v = 0, w = 0, vn = attachmentVertices->m_verticesData.size(); v < vn; ++v, w += 2)
			{
				SpineVertexFormat* vertex = attachmentVertices->m_verticesData.data() + v;
				vertex->m_position.x = m_worldVertices[w];
				vertex->m_position.y = m_worldVertices[w + 1];
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
		clearRenderable();
	}

	void Spine::clearRenderable()
	{
	}
}