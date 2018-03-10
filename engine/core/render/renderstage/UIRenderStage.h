#pragma once

#include "RenderStage.h"
#include <engine/core/render/render/Renderable.h>

namespace Echo
{
	/**
	 * UI渲染阶段
	 */

	class UIRenderStage : public RenderStage
	{
		friend class RenderStageManager;

	public:
		virtual bool	initialize					();
		virtual void	render						();
		virtual void	destroy						();

		// catui
		void			addCatUIManualRenderable	(IManualRenderable* renderable);
		void			addCatUIRenderable			(IManualRenderable* renderable, const int renderLevel);
		void			addCatUIEffectRenderable	(IManualRenderable* renderable, const int renderLevel);

	protected:
		UIRenderStage();
		virtual ~UIRenderStage();

		void			_postRenderClear			();
		void			_renderCatui				();
		void			_renderCEGUI				();
		void			_renderCatuiTop				();
		void			_renderCatui3DModel			();

	private:
		vector<IManualRenderable*>::type			m_catuiManualRenderables;	// CatUI手动可渲染对象
		vector<IManualRenderable*>::type			m_catuiRenderables;			// CatUI可渲染对象
		vector<IManualRenderable*>::type			m_catuiTopRenderables;		// 位于顶层的CatUI可渲染对象，在CEGUI上面
		vector<IManualRenderable*>::type			m_autoDeletes;			
	};
}