#include "RenderStage.h"
#include "Engine/core/main/Engine.h"
#include "engine/core/render/interface/RenderTargetManager.h"

namespace Echo
{
	RenderStage::RenderStage()
	{
		m_items.push_back(EchoNew(DefaultRenderQueueOpaque));
		m_items.push_back(EchoNew(DefaultRenderQueueTransparent));
	}

	RenderStage::~RenderStage()
	{
	}

	// get instance
	RenderStage* RenderStage::instance()
	{
		static RenderStage* inst = EchoNew(RenderStage);

		return inst;
	}

	// destroy
	void RenderStage::destroy()
	{
		RenderStage* inst = instance();
		EchoSafeDelete(inst, RenderStage);
	}

	// add renderable
	void RenderStage::addRenderable(const String& name, RenderableID id)
	{
		for (RenderQueue* item : m_items)
		{
			if (item->getName() == name)
				item->addRenderable(id);
		}
	}

	// process all render stage item
	void RenderStage::process()
	{
		RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer);

		for (RenderQueue* item : m_items)
		{
			item->render();
		}

		RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
	}
}