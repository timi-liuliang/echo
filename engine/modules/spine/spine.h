#pragma once

#include "engine/core/scene/Node.h"
#include "engine/core/render/mesh/Mesh.h"
#include "engine/core/render/MaterialInst.h"
#include "engine/core/render/render/Renderable.h"

struct spAtlas;
struct spSkeleton;
struct spAnimationState;
struct spAttachmentLoader;

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

		// set spin
		void setSpin(const ResourcePath& res);

		// get spin
		const ResourcePath& getSpin() { return m_spinRes; }

		// set atlas
		void setAtlas(const ResourcePath& res);

		// get atlas
		const ResourcePath& getAtlas() { return m_atlasRes; }
		
		// play anim
		void playAnim(const String& animName);

	protected:
		// update
		virtual void update();

		// get global uniforms
		virtual void* getGlobalUniformValue(const String& name);

		// submit to renderqueue
		void submitToRenderQueue();

		// clear
		void clear();
		void clearRenderable();

	private:
		ResourcePath		m_spinRes;
		ResourcePath		m_atlasRes;
		spAtlas*			m_spAtlas;
		spSkeleton*			m_spSkeleton;
		spAnimationState*	m_spAnimState;
		spAttachmentLoader*	m_attachmentLoader;
		Matrix4				m_matWVP;
	};
}