#pragma once

#include "engine/core/util/StringOption.h"
#include "engine/core/scene/render_node.h"
#include "engine/core/render/base/mesh/mesh.h"
#include "engine/core/render/base/Material.h"
#include "engine/core/render/base/Renderable.h"
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
		Spine();
		virtual ~Spine();

		// set spin
		void setSpin(const ResourcePath& res);

		// get spin
		const ResourcePath& getSpin() { return m_spinRes; }

		// set atlas
		void setAtlas(const ResourcePath& res);

		// get atlas
		const ResourcePath& getAtlas() { return m_atlasRes; }
		
		// play anim
		void setAnim(const StringOption& animName);

		// get animations
		const StringOption& getAnim() { return m_animations; }

	protected:
		// update
		virtual void update_self() override;

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

		AttachmentVertices	m_batch;
		MeshResPtr			m_mesh;
        ShaderProgramPtr    m_shader;
		Material*			m_material;
		Renderable*			m_renderable;
	};
}
