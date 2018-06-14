#include "RenderStage.h"
#include "Engine/core/main/Engine.h"

namespace Echo
{
	// 构造函数
	RenderStage::RenderStage()
	{
		m_items.push_back(EchoNew(DefaultRenderStageItemOpaque));
		m_items.push_back(EchoNew(DefaultRenderStageItemTransparent));
	}

	// 析构函数
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
		for (RenderStageItem* item : m_items)
		{
			if (item->getName() == name)
				item->addRenderable(id);
		}
	}

	// 处理所有渲染阶段
	void RenderStage::process()
	{
		for (RenderStageItem* item : m_items)
		{
			item->render();
		}
	}
}