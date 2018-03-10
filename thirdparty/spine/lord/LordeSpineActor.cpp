#include <Engine/Scene/SceneManager.h>
#include "EchoeSpineActor.h"
#include "EchoSpineAttachment.h"

namespace Spine
{
	static void animationCallback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) 
	{
		//((SkeletonAnimation*)state->rendererObject)->onAnimationStateEvent(trackIndex, type, event, loopCount);
	}

	static void trackEntryCallback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) 
	{
		//((SkeletonAnimation*)state->rendererObject)->onTrackEntryEvent(trackIndex, type, event, loopCount);
	}

	// 构造函数
	Actor::Actor(const char* resName)
		: m_spAtlas(nullptr)
		, m_spAnimState(nullptr)
		, m_sceneNode(nullptr)
	{
		m_worldVertices = new float[1000];

		m_spAtlas = spAtlas_createFromFile(ECHO::StringUtil::Format("%s.atlas", resName).c_str(), NULL);

		// 0.Attachment
		m_attachmendLoader = &(EchoAttachmentLoader_create(m_spAtlas)->super);

		// 1.Json
		spSkeletonJson* json = spSkeletonJson_createWithLoader(m_attachmendLoader);
		json->scale = 1.f;

		// SkeletonData
		spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, ECHO::StringUtil::Format("%s.json", resName).c_str());
		m_spSkeleton = spSkeleton_create(skeletonData);

		// Animation
		m_spAnimState = spAnimationState_create(spAnimationStateData_create(m_spSkeleton->data));
		m_spAnimState->rendererObject = this;
		m_spAnimState->listener = animationCallback;

		//_spAnimationState* stateInternal = (_spAnimationState*)m_spAnimState;
		//stateInternal->disposeTrackEntry = disposeTrackEntry;

		m_sceneNode = ECHO::SceneManager::instance()->getRootNode()->createChild();
		m_sceneNode->update();

		//spSkeletonJson_dispose(json);
		//spSkeletonData_dispose(skeletonData);
	}

	Actor::~Actor()
	{
		ECHO::SceneManager::instance()->getRootNode()->destroyChild(m_sceneNode);
	}

	// 播放动画
	void Actor::playAnimation(const char* animName)
	{
		// 播放动画
		spAnimationState_setAnimationByName(m_spAnimState, 0, animName, true);
	}

	// 更新
	void Actor::update(float delta, bool isForUI)
	{
		// 确认摄像机
		ECHO::Camera* camera = isForUI ? ECHO::SceneManager::instance()->getGUICamera() : ECHO::SceneManager::instance()->getMainCamera();

		// 更新世界观察投影矩阵
		m_sceneNode->update();
		m_matWVP = m_sceneNode->getWorldMatrix() *camera->getViewProjMatrix();

		// 动画数据更新
		spSkeleton_update(m_spSkeleton, delta);
		spAnimationState_update(m_spAnimState, delta);
		spAnimationState_apply(m_spAnimState, m_spSkeleton);
		spSkeleton_updateWorldTransform(m_spSkeleton);
	}

	// 提交到渲染队列
	void Actor::submitToRenderQueue()
	{
		for (int i = 0; i < m_spSkeleton->slotsCount; i++)
		{
			spSlot* slot = m_spSkeleton->drawOrder[i];
			if (!slot->attachment)
				continue;

			Spine::AttachmentVertices* attachmentVertices = nullptr;
			switch (slot->attachment->type)
			{
			case SP_ATTACHMENT_REGION:
				{
					spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
					spRegionAttachment_computeWorldVertices(attachment, slot->bone, m_worldVertices);
					attachmentVertices = (Spine::AttachmentVertices*)attachment->rendererObject;
					break;
				}
			case SP_ATTACHMENT_MESH:
				{
					spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
					spMeshAttachment_computeWorldVertices(attachment, slot, m_worldVertices);
					attachmentVertices = (Spine::AttachmentVertices*)attachment->rendererObject;
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
				Spine::VertexFormat* vertex = attachmentVertices->m_verticesData.data() + v;
				vertex->m_position.x = m_worldVertices[w];
				vertex->m_position.y = m_worldVertices[w + 1];
				vertex->m_position.z = 0.f;
				vertex->m_diffuse = ECHO::Color::WHITE;
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

			attachmentVertices->submitToRenderQueue( m_matWVP);
		}
	}
}