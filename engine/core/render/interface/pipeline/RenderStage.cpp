#include "RenderStage.h"
#include "RenderPipeline.h"
#include "Engine/core/main/Engine.h"

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

	void RenderStage::bindMethods()
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
		RenderPipeline::instance()->beginRenderTarget(RenderPipeline::RTI_DefaultBackBuffer);

		for (RenderQueue* item : m_items)
		{
			item->render();
		}

		RenderPipeline::instance()->endRenderTarget(RenderPipeline::RTI_DefaultBackBuffer);
	}
}