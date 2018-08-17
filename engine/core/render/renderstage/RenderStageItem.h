#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/render/render/RenderState.h>
#include <engine/core/render/render/RenderInput.h>
#include <engine/core/render/render/Renderable.h>

namespace Echo
{
	/**
	* äÖÈ¾½×¶ÎÏî
	*/
	class RenderStageItem
	{
	public:
		RenderStageItem();
		virtual ~RenderStageItem();

		// render
		virtual void render();

		// add renderalbe
		void addRenderable(RenderableID id) { m_renderables.push_back(id); }

		// set name
		void setName(const String& name) { m_name = name; }

		// get name
		const String& getName() const { return m_name; }

	protected:
		String							m_name;
		vector<RenderableID>::type		m_renderables;
	};


	class DefaultRenderStageItemOpaque : public RenderStageItem
	{
	public:
		DefaultRenderStageItemOpaque();

		// render
		virtual void render();
	};

	class DefaultRenderStageItemTransparent : public RenderStageItem
	{
	public:
		DefaultRenderStageItemTransparent();

		// sort
		void sort();

		// render
		virtual void render();
	};
}
