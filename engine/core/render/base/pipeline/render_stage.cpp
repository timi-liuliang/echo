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
		CLASS_BIND_METHOD(RenderStage, getName,				DEF_METHOD("getName"));
		CLASS_BIND_METHOD(RenderStage, setName,				DEF_METHOD("setName"));
		CLASS_BIND_METHOD(RenderStage, getFrameBufferType,  DEF_METHOD("getFrameBufferType"));
		CLASS_BIND_METHOD(RenderStage, setFrameBufferType,  DEF_METHOD("setFrameBufferType"));
		CLASS_BIND_METHOD(RenderStage, isClearColor,		DEF_METHOD("isClearColor"));
		CLASS_BIND_METHOD(RenderStage, setClearColor,		DEF_METHOD("setClearColor"));
		CLASS_BIND_METHOD(RenderStage, isClearDepth,		DEF_METHOD("isClearDepth"));
		CLASS_BIND_METHOD(RenderStage, setClearDepth,		DEF_METHOD("setClearDepth"));

		CLASS_REGISTER_PROPERTY(RenderStage, "Name", Variant::Type::String, "getName", "setName");
		CLASS_REGISTER_PROPERTY(RenderStage, "FrameBuffer.Type", Variant::Type::StringOption, "getFrameBufferType", "setFrameBufferType");
		CLASS_REGISTER_PROPERTY(RenderStage, "FrameBuffer.IsClearColor", Variant::Type::Bool, "isClearColor", "setClearColor");
		CLASS_REGISTER_PROPERTY(RenderStage, "FrameBuffer.IsClearDepth", Variant::Type::Bool, "isClearDepth", "setClearDepth");

		CLASS_REGISTER_PROPERTY_HINT(RenderStage, "FrameBuffer.Type",		  PropertyHintType::Category, "FrameBuffer");
		CLASS_REGISTER_PROPERTY_HINT(RenderStage, "FrameBuffer.IsClearColor", PropertyHintType::Category, "FrameBuffer");
		CLASS_REGISTER_PROPERTY_HINT(RenderStage, "FrameBuffer.IsClearDepth", PropertyHintType::Category, "FrameBuffer");
	}

	void RenderStage::setFrameBufferType(const StringOption& type)
	{
		if (m_frameBufferType.setValue(type.getValue()))
		{
			if(m_frameBufferType.getIdx()==1)
				m_frameBuffer = Renderer::instance()->getWindowFrameBuffer();
		}
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
		}
	}

	void RenderStage::saveXml(void* pugiNode)
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

	void RenderStage::onSize(ui32 width, ui32 height)
	{
		if (m_frameBuffer)
			m_frameBuffer->onSize(width, height);
	}

	void RenderStage::render()
	{
		if (m_frameBuffer)
		{
			m_frameBuffer->begin(m_isClearColor, Renderer::BGCOLOR, m_isClearDepth, 1.f, false, 0);

			for (IRenderQueue* iqueue : m_renderQueues)
			{
				if (iqueue->isEnable())
					iqueue->render();
			}

			m_frameBuffer->end();
		}
	}
}
