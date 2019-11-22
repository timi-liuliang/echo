#include "RenderStage.h"
#include "RenderPipeline.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	RenderStage::RenderStage()
	{
		m_items.push_back(EchoNew(DefaultRenderQueueOpaque));
		m_items.push_back(EchoNew(DefaultRenderQueueTransparent));
	}

	RenderStage::~RenderStage()
	{
        EchoSafeDeleteContainer(m_items, RenderQueue);
	}

	void RenderStage::bindMethods()
	{

	}

	RenderStage* RenderStage::instance()
	{
		static RenderStage* inst = EchoNew(RenderStage);

		return inst;
	}

	void RenderStage::destroy()
	{
		RenderStage* inst = instance();
		EchoSafeDelete(inst, RenderStage);
	}

	void RenderStage::addRenderable(const String& name, RenderableID id)
	{
		for (RenderQueue* item : m_items)
		{
			if (item->getName() == name)
				item->addRenderable(id);
		}
	}

	void RenderStage::process()
	{
		RenderPipeline::instance()->beginFramebuffer(RenderPipeline::FB_Window);

		for (RenderQueue* item : m_items)
		{
			item->render();
		}

		RenderPipeline::instance()->endFramebuffer(RenderPipeline::FB_Window);
	}
}
