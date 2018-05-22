#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"
#include "engine/core/render/render/Renderable.h"

namespace Echo
{
	class Spine : public Node
	{
		ECHO_CLASS(Spine, Node)

	public:
		Spine();
		virtual ~Spine();

		// bind class methods to script
		static void bindMethods();

		// set moc
		void setSpin(const ResourcePath& res);

		// get moc
		const ResourcePath& getSpin() { return m_spinRes; }

	protected:
		// update
		virtual void update();

		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

		// clear
		void clear();
		void clearRenderable();

	private:
		ResourcePath				m_spinRes;
		struct spAtlas*				m_spAtlas;
		struct spSkeleton*			m_spSkeleton;
		struct spAnimationState*	m_spAnimState;
		struct spAttachmentLoader*	m_attachmentLoader;
		Mesh*						m_mesh;				// Geometry Data for render
		MaterialInst*				m_materialInst;		// Material Instance
		Renderable*					m_renderable;
		Matrix4						m_matWVP;
	};
}