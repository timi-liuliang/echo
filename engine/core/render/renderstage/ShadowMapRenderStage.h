#pragma once

#include "RenderStage.h"
#include "engine/core/render/render/Viewport.h"

namespace Echo
{
	/**
	* 阴影图渲染阶段
	*/
	class ShadowMapRenderStage : public RenderStage
	{
		friend class RenderStageManager;
		typedef vector<int>::type RenderableList;
	public:
		ShadowMapRenderStage();
		virtual ~ShadowMapRenderStage();

		// 初始化
		virtual bool initialize();

		// 渲染
		virtual void render();

		// 销毁
		virtual void destroy();

		// 设置阴影浓度（0 - 1 由浅到深）
		void setShadowShade(float t){ m_shadowShade = t; }
		float* getShadowShadePtr(){ return &m_shadowShade; }

		// 设置阴影图尺寸，2的次幂

		float getShadowMapSize(){ return static_cast<float>(m_shadowMapSize); }
		void setShadowMapSize(ui32 size);

		void enableBlurShadow(bool isEnable);
		bool isEnableBlurShadow() const;

	protected:
		const SamplerState*		m_shadowMapSamplerState;		// 阴影图采样状态

		ui32 m_shadowMapSize;

		float m_shadowShade;

		bool m_init;

		RenderInput::VertexElementList m_verElementLists;	// 顶点格式
		class GPUBuffer* m_pScreenAlignedQuadVB;
		class GPUBuffer* m_pScreenAlignedQuadIB;

		bool m_bNeedBlur;

		bool m_isShadowCameraEnable;

		// materal
		class Material* m_pMtlShadowMapVBlurPass;
		class Material* m_pMtlShadowMapHBlurPass;

		// renderable
		class Renderable* m_pRaShadowMapVBlurPass;
		class Renderable* m_pRaShadowMapHBlurPass;

		Viewport	m_renderViewPort;
	};
}