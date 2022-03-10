#include "spine.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"
#include "base/renderer.h"
#include "base/shader/shader_program.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/math/color.h"
#include "engine/core/main/Engine.h"
#include <spine/spine.h>
#include <spine/extension.h>
#include "AttachmentLoader.h"

namespace Echo
{
	static void animationCallback(spAnimationState* state, spEventType type, spTrackEntry* entry, spEvent* event)
	{

	}

	//static void trackEntryCallback(spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount)
	//{
		//((SkeletonAnimation*)state->rendererObject)->onTrackEntryEvent(trackIndex, type, event, loopCount);
	//}

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
		CLASS_BIND_METHOD(Spine, getBillboardType);
		CLASS_BIND_METHOD(Spine, setBillobardType);
		CLASS_BIND_METHOD(Spine, getSpin);
		CLASS_BIND_METHOD(Spine, setSpin);
		CLASS_BIND_METHOD(Spine, getAtlas);
		CLASS_BIND_METHOD(Spine, setAtlas);
		CLASS_BIND_METHOD(Spine, setAnim);
		CLASS_BIND_METHOD(Spine, getAnim);
		CLASS_BIND_METHOD(Spine, playAnim);

		CLASS_REGISTER_PROPERTY(Spine, "Billboard", Variant::Type::StringOption, getBillboardType, setBillobardType);
		CLASS_REGISTER_PROPERTY(Spine, "Spin", Variant::Type::ResourcePath, getSpin, setSpin);
		CLASS_REGISTER_PROPERTY(Spine, "Atlas", Variant::Type::ResourcePath, getAtlas, setAtlas);
		CLASS_REGISTER_PROPERTY(Spine, "Anim", Variant::Type::StringOption, getAnim, setAnim);
	}

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

	void Spine::setAnim(const StringOption& animName)
	{
		if (m_animations.setValue(animName.getValue()))
		{
			spAnimationState_setAnimationByName(m_spAnimState, 0, m_animations.getValue().c_str(), true);
		}
	}

	void Spine::playAnim(const String& animName, bool loop)
	{
		if (m_animations.getValue() != animName && m_animations.setValue(animName))
		{
			spAnimationState_setAnimationByName(m_spAnimState, 0, m_animations.getValue().c_str(), loop);
		}
	}

	StringOption Spine::getBillboardType()
	{
		StringOption result;
		result.fromEnum(m_billboardType);

		return result;
	}

	void Spine::setBillobardType(const StringOption& type)
	{
		m_billboardType = type.toEnum(BillboardType::None);
	}

	void Spine::updateInternal(float elapsedTime)
	{
		if (isNeedRender())
		{
			updateBillboard();

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

	void Spine::updateBillboard()
	{
		if (m_billboardType != BillboardType::None)
		{
			Camera* camera = getCamera();
			if (camera)
			{
				Vector3 faceDir = m_billboardType == BillboardType::LookAt ? getWorldPosition() - camera->getPosition() : camera->getDirection();
				faceDir.normalize();

				Vector3 hDir(faceDir.x, 0.f, faceDir.z);
				hDir.normalize();

				Quaternion quat = Quaternion::fromVec3ToVec3(hDir, faceDir) * Quaternion::fromVec3ToVec3(Vector3::UNIT_Z, hDir);
				setWorldOrientation(quat);
			}
		}
	}

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

					int count = int(attachmentVertices->m_verticesData.size() * sizeof(SpineVertexFormat) / 4);
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
		
		if (m_renderable)
			m_renderable->setSubmitToRenderQueue(isNeedRender());
	}

	void Spine::updateRenderable()
	{
		if (!m_renderable)
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh = Mesh::create(true, true);
			m_mesh->updateIndices(ui32(m_batch.m_indicesData.size()), sizeof(Word), m_batch.m_indicesData.data());
			m_mesh->updateVertexs(define, ui32(m_batch.m_verticesData.size()), (const Byte*)m_batch.m_verticesData.data());
            
            StringArray macros;
            m_shader = ShaderProgram::getDefault2D(macros);
            
			m_material = ECHO_CREATE_RES(Material);
			m_material->setShaderPath(m_shader->getPath());

			m_material->setUniformTexture("BaseColor", m_batch.m_texture);

			m_renderable = RenderProxy::create(m_mesh, m_material, this, false);
		}
		else
		{
			MeshVertexFormat define;
			define.m_isUseVertexColor = true;
			define.m_isUseUV = true;

			m_mesh->updateIndices(ui32(m_batch.m_indicesData.size()), sizeof(Word), m_batch.m_indicesData.data());
			m_mesh->updateVertexs(define, ui32(m_batch.m_verticesData.size()), (const Byte*)m_batch.m_verticesData.data());
		}
	}

	void Spine::clear()
	{

	}
}
