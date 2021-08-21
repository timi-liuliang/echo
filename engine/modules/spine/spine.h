#pragma once

#include "engine/core/util/StringOption.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/material.h"
#include "engine/core/render/base/proxy/render_proxy.h"
#include "AttachmentLoader.h"

struct spAtlas;
struct spSkeleton;
struct spSkeletonData;
struct spAnimationState;
struct spAttachmentLoader;

namespace Echo
{
	class Spine : public Render
	{
		ECHO_CLASS(Spine, Render)

	public:
		enum BillboardType
		{
			None,
			LookAt,
			ViewDir,
		};

	public:
		Spine();
		virtual ~Spine();

		// type
		StringOption getBillboardType();
		void setBillobardType(const StringOption& type);

		// spin
		void setSpin(const ResourcePath& res);
		const ResourcePath& getSpin() { return m_spinRes; }

		// atlas
		void setAtlas(const ResourcePath& res);
		const ResourcePath& getAtlas() { return m_atlasRes; }
		
		// anim
		void setAnim(const StringOption& animName);
		const StringOption& getAnim() { return m_animations; }

		// play
		void playAnim(const String& animName, bool loop);

	protected:
		// update
		virtual void updateInternal(float elapsedTime) override;

		// update bilboard
		void updateBillboard();

		// submit to renderqueue
		void submitToRenderQueue();

		// clear
		void clear();

		// update renderable
		void updateRenderable();

	private:
		ResourcePath		m_spinRes;
		ResourcePath		m_atlasRes;
		StringOption		m_animations;
		spAtlas*			m_spAtlas;
		spSkeletonData*		m_spSkeletonData;
		spSkeleton*			m_spSkeleton;
		spAnimationState*	m_spAnimState;
		spAttachmentLoader*	m_attachmentLoader;

		BillboardType		m_billboardType = BillboardType::None;
		AttachmentVertices	m_batch;
		MeshPtr				m_mesh;
        ShaderProgramPtr    m_shader;
		Material*			m_material;
		RenderProxy*		m_renderable;
	};
}
