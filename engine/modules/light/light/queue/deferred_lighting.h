#pragma once

#include "engine/core/render/base/pipeline/render_stage.h"

namespace Echo
{
	class DeferredLighting : public IRenderQueue
	{
		ECHO_VIRTUAL_CLASS(DeferredLighting, IRenderQueue)

	public:
		DeferredLighting();
		virtual ~DeferredLighting();

		// Material
		Material* getMaterial() const { return m_material; }
		void setMaterial(Object* material);

		// Process
		virtual void render(FrameBufferPtr& frameBuffer) override;

	protected:
		bool			m_dirty = true;
		MaterialPtr		m_material;
	};
}