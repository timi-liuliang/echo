#pragma once

#include "engine/core/base/echo_def.h"
#include "engine/core/render/interface/Texture.h"
#include "engine/core/render/interface/RenderTarget.h"
#include "engine/core/render/interface/Renderer.h"

namespace Echo
{
	enum RenderTargetID
	{
		RTI_DefaultBackBuffer,
		RTI_End
	};

	class RenderTargetManager
	{
	public:
		virtual ~RenderTargetManager();

		// 获取渲染实例
		static RenderTargetManager* instance();

		// 初始化
		bool initialize();

		// 开始渲染目标
		bool beginRenderTarget( ui32 _id,bool _clearColor = true, const Color& _backgroundColor = Renderer::BGCOLOR,  bool _clearDepth = true, float _depthValue = 1.0f, bool _clearStencil = false, ui8 stencilValue = 0, ui32 rbo = RenderTarget::Invalid_Value );

		// 结束渲染目标
		bool endRenderTarget(ui32 _id);

		// 创建渲染目标
		RenderTarget* createRenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, RenderTarget::Options option = RenderTarget::Options());

		// destory render target
		void destroyRenderTargetByID(ui32 _id);

		// 根据ID获取渲染目标
		RenderTarget* getRenderTargetByID(ui32 _id);

		// get current render target
		ui32 getCurrentRenderTarget() const { return m_currentRenderTarget; }

		// 当屏幕大小改变时调用 
		void onScreensizeChanged( ui32 _width, ui32 _height );

	private:
		RenderTargetManager();

		// invalide
		bool invalidate(ui32 id, bool invalidateColor, bool invalidateDepth, bool invalidateStencil);

	protected:
		RenderTargetMap			m_renderTargets;					// 所有渲染目标
		ui32					m_currentRenderTarget;
	};
}
