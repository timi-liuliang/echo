#include "RenderStage.h"
#include "RenderPipeline.h"
#include "RenderQueue.h"
#include "ImageFilter.h"
#include "engine/core/main/Engine.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	RenderStage::RenderStage(RenderPipeline* pipeline)
		: m_pipeline(pipeline)
	{
	}

	RenderStage::~RenderStage()
	{
        EchoSafeDeleteContainer(m_renderQueues, IRenderQueue);
	}

	void RenderStage::parseXml(void* pugiNode)
	{
		pugi::xml_node* stageNode = (pugi::xml_node*)pugiNode;
		if (stageNode)
		{
			// queues
			for (pugi::xml_node queueNode = stageNode->child("queue"); queueNode; queueNode = queueNode.next_sibling("queue"))
			{
				String type = queueNode.attribute("type").as_string();
				if (type == "queue")
				{
					RenderQueue* queue = EchoNew(RenderQueue(m_pipeline, this));
					queue->setName(queueNode.attribute("name").as_string("Opaque"));
					queue->setSort(queueNode.attribute("sort").as_bool(false));
					m_renderQueues.push_back(queue);
				}
				else if (type == "filter")
				{
					ImageFilter* queue = EchoNew(ImageFilter(m_pipeline, this));
					queue->setName(queueNode.attribute("name").as_string());
					m_renderQueues.push_back(queue);
				}
			}

			// frame buffer
			pugi::xml_node framebufferNode = stageNode->child("framebuffer");
			if (framebufferNode)
			{
				m_frameBufferId = framebufferNode.attribute("id").as_uint();
			}
		}
	}

	void RenderStage::addRenderable(const String& name, RenderableID id)
	{
		for (IRenderQueue* iqueue : m_renderQueues)
		{
			RenderQueue* queue = dynamic_cast<RenderQueue*>(iqueue);
			if (queue)
			{
				if (queue->getName() == name)
					queue->addRenderable(id);
			}
		}
	}

	void RenderStage::render()
	{
		if (m_frameBufferId != -1)
		{
			RenderPipeline::current()->beginFramebuffer(m_frameBufferId);

			for (IRenderQueue* iqueue : m_renderQueues)
			{
				iqueue->render();
			}

			RenderPipeline::current()->endFramebuffer(m_frameBufferId);
		}
	}
}
