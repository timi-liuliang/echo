#include "render_stage.h"
#include "render_pipeline.h"
#include "render_queue.h"
#include "image_filter.h"
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

	void RenderStage::bindMethods()
	{
		CLASS_BIND_METHOD(RenderStage, getName, DEF_METHOD("getName"));
		CLASS_BIND_METHOD(RenderStage, setName, DEF_METHOD("setName"));

		CLASS_REGISTER_PROPERTY(RenderStage, "Name", Variant::Type::String, "getName", "setName");
	}

	ImageFilter* RenderStage::addImageFilter(const String& name)
	{
		ImageFilter* queue = EchoNew(ImageFilter(this));
		queue->setName(name);
		m_renderQueues.emplace_back(queue);

		return queue;
	}

	void RenderStage::addRenderQueue(IRenderQueue* queue, ui32 position)
	{
		if (position < m_renderQueues.size())
		{
			queue->setStage(this);
			m_renderQueues.insert(m_renderQueues.begin() + position, queue);
		}
		else
		{
			queue->setStage(this);
			m_renderQueues.emplace_back(queue);
		}
	}

	void RenderStage::removeRenderQueue(IRenderQueue* renderQueue)
	{
		renderQueue->setStage(nullptr);
		m_renderQueues.erase(std::find(m_renderQueues.begin(), m_renderQueues.end(), renderQueue));
	}

	void RenderStage::deleteRenderQueue(IRenderQueue* renderQueue)
	{
		removeRenderQueue(renderQueue);
		EchoSafeDelete(renderQueue, IRenderQueue);
	}

	void RenderStage::parseXml(void* pugiNode)
	{
		pugi::xml_node* stageNode = (pugi::xml_node*)pugiNode;
		if (stageNode)
		{
			// queues
			for (pugi::xml_node queueNode = stageNode->child("queue"); queueNode; queueNode = queueNode.next_sibling("queue"))
			{
				IRenderQueue* queue = ECHO_DOWN_CAST<IRenderQueue*>(instanceObject(&queueNode));
				if (queue)
				{
					queue->setStage(this);
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

	void RenderStage::saveXml(void* pugiNode)
	{
		pugi::xml_node* parentNode = (pugi::xml_node*)pugiNode;
		if (parentNode)
		{
			pugi::xml_node stageNode = parentNode->append_child("stage");
			stageNode.append_attribute("name").set_value(getName().c_str());

			for (IRenderQueue* renderQueue : m_renderQueues)
			{
				pugi::xml_node queueNode = stageNode.append_child("queue");
				savePropertyRecursive(&queueNode, renderQueue, renderQueue->getClassName());
			}

			pugi::xml_node frameNode = stageNode.append_child("framebuffer");
			frameNode.append_attribute("id").set_value(m_frameBufferId);
		}
	}

	void RenderStage::addRenderable(const String& name, RenderableID id)
	{
		for (IRenderQueue* iqueue : m_renderQueues)
		{
			RenderQueue* queue = dynamic_cast<RenderQueue*>(iqueue);
			if (queue && queue->isEnable())
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
				if(iqueue->isEnable())
					iqueue->render();
			}

			RenderPipeline::current()->endFramebuffer(m_frameBufferId);
		}
	}
}
