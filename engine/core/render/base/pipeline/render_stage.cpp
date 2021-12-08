#include "render_stage.h"
#include "render_pipeline.h"
#include "render_queue.h"
#include "image_filter.h"
#include "engine/core/main/Engine.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	RenderStage::~RenderStage()
	{
        EchoSafeDeleteContainer(m_renderQueues, IRenderQueue);
	}

	void RenderStage::bindMethods()
	{
		CLASS_BIND_METHOD(RenderStage, getName);
		CLASS_BIND_METHOD(RenderStage, setName);
		CLASS_BIND_METHOD(RenderStage, isEnable);
		CLASS_BIND_METHOD(RenderStage, setEnable);
		CLASS_BIND_METHOD(RenderStage, isEditorOnly);
		CLASS_BIND_METHOD(RenderStage, setEditorOnly);
		CLASS_BIND_METHOD(RenderStage, getFrameBuffer);
		CLASS_BIND_METHOD(RenderStage, setFrameBuffer);

		CLASS_REGISTER_PROPERTY(RenderStage, "Name", Variant::Type::String, getName, setName);
		CLASS_REGISTER_PROPERTY(RenderStage, "Enable", Variant::Type::Bool, isEnable, setEnable);
		CLASS_REGISTER_PROPERTY(RenderStage, "EditorOnly", Variant::Type::Bool, isEditorOnly, setEditorOnly);
		CLASS_REGISTER_PROPERTY(RenderStage, "FrameBuffer", Variant::Type::Object, getFrameBuffer, setFrameBuffer);

		CLASS_REGISTER_PROPERTY_HINT(RenderStage, "FrameBuffer", PropertyHintType::ObjectType, "FrameBufferOffScreen|FrameBufferWindow");
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
		EchoSafeDeleteContainer(m_renderQueues, IRenderQueue);

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
		if (!m_enable)				return;
		if (!m_frameBuffer)			return;
		if (IsGame && m_editorOnly) return;

		if (m_frameBuffer->begin())
		{
			onRenderBegin();
			{
				for (IRenderQueue* iqueue : m_renderQueues)
				{
					if (iqueue->isEnable())
						iqueue->render(m_frameBuffer);
				}
			}
			onRenderEnd();

			m_frameBuffer->end();
		}
	}
}
