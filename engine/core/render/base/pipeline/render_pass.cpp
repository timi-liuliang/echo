#include "render_pass.h"
#include "render_pipeline.h"
#include "render_queue.h"
#include "image_filter.h"
#include "engine/core/main/Engine.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	RenderPass::RenderPass(RenderPipeline* pipeline)
		: m_pipeline(pipeline)
	{
	}

	RenderPass::~RenderPass()
	{
        EchoSafeDeleteContainer(m_renderQueues, IRenderQueue);
	}

	void RenderPass::bindMethods()
	{
		CLASS_BIND_METHOD(RenderPass, getName,		  DEF_METHOD("getName"));
		CLASS_BIND_METHOD(RenderPass, setName,		  DEF_METHOD("setName"));
		CLASS_BIND_METHOD(RenderPass, getFrameBuffer, DEF_METHOD("getFrameBuffer"));
		CLASS_BIND_METHOD(RenderPass, setFrameBuffer, DEF_METHOD("setFrameBuffer"));

		CLASS_REGISTER_PROPERTY(RenderPass, "Name", Variant::Type::String, "getName", "setName");
		CLASS_REGISTER_PROPERTY(RenderPass, "FrameBuffer", Variant::Type::Object, "getFrameBuffer", "setFrameBuffer");

		CLASS_REGISTER_PROPERTY_HINT(RenderPass, "FrameBuffer", PropertyHintType::ResourceType, "FrameBufferOffScreen|FrameBufferWindow");
	}

	ImageFilter* RenderPass::addImageFilter(const String& name)
	{
		ImageFilter* queue = EchoNew(ImageFilter(this));
		queue->setName(name);
		m_renderQueues.emplace_back(queue);

		return queue;
	}

	void RenderPass::addRenderQueue(IRenderQueue* queue, ui32 position)
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

	void RenderPass::removeRenderQueue(IRenderQueue* renderQueue)
	{
		renderQueue->setStage(nullptr);
		m_renderQueues.erase(std::find(m_renderQueues.begin(), m_renderQueues.end(), renderQueue));
	}

	void RenderPass::deleteRenderQueue(IRenderQueue* renderQueue)
	{
		removeRenderQueue(renderQueue);
		EchoSafeDelete(renderQueue, IRenderQueue);
	}

	void RenderPass::parseXml(void* pugiNode)
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
		}
	}

	void RenderPass::saveXml(void* pugiNode)
	{
		pugi::xml_node* parentNode = (pugi::xml_node*)pugiNode;
		if (parentNode)
		{
			pugi::xml_node stageNode = parentNode->append_child("stage");
			savePropertyRecursive(&stageNode, this, this->getClassName());

			for (IRenderQueue* renderQueue : m_renderQueues)
			{
				pugi::xml_node queueNode = stageNode.append_child("queue");
				savePropertyRecursive(&queueNode, renderQueue, renderQueue->getClassName());
			}
		}
	}

	void RenderPass::addRenderable(const String& name, RenderableID id)
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

	void RenderPass::onSize(ui32 width, ui32 height)
	{
		if (m_frameBuffer)
			m_frameBuffer->onSize(width, height);
	}

	void RenderPass::render()
	{
		if (m_frameBuffer)
		{
			if (m_frameBuffer->begin(Renderer::BGCOLOR, 1.f, false, 0))
			{
				for (IRenderQueue* iqueue : m_renderQueues)
				{
					if (iqueue->isEnable())
						iqueue->render();
				}

				m_frameBuffer->end();
			}
		}
	}
}
