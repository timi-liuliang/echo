#include "Renderer.h"
#include "RenderTarget.h"
#include "engine/core/log/Log.h"
#include "Viewport.h"
#include "image/PixelFormat.h"
#include "Renderable.h"

namespace Echo
{
	Color Renderer::BGCOLOR = Echo::Color(0.298f, 0.298f, 0.322f);	// render target clear color
	static Renderer* g_render = NULL;								// global renderer

	Renderer* Renderer::instance()
	{
		EchoAssert(g_render);
		return g_render;
	}

	bool Renderer::replaceInstance(Renderer* inst)
	{
		EchoAssert(!g_render);
		g_render = inst;

		return true;
	}

	Renderer::Renderer()
		: m_bVSync(false)
		, m_frameBuffer(NULL)
		, m_startMipmap(0)
		, m_pDefaultRasterizerState(NULL)
		, m_pDefaultDepthStencilState(NULL)
		, m_pDefaultBlendState(NULL)
		, m_pRasterizerState(NULL)
		, m_pDepthStencilState(NULL)
		, m_pBlendState(NULL)
		, m_dirtyTexSlot(false)
		, m_renderableIdentifier(1)
	{
		EchoAssert(!g_render);
		g_render = this;
	}

	Renderer::~Renderer()
	{
		for (std::map<ui32, Renderable*>::iterator it = m_renderables.begin(); it != m_renderables.end(); ++it)
		{
			EchoSafeDelete(it->second, Renderable);
		}
		m_renderables.clear();
	}

	void Renderer::setRasterizerState(RasterizerState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pRasterizerState)
		{
			pState->active();
			m_pRasterizerState = pState;
		}
	}

	void Renderer::setDepthStencilState(DepthStencilState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pDepthStencilState)
		{
			pState->active();
			m_pDepthStencilState = pState;
		}
	}

	void Renderer::setBlendState(BlendState* pState)
	{
		EchoAssert(pState);
		if (pState != m_pBlendState)
		{
			pState->active();
			m_pBlendState = pState;
		}
	}

	bool Renderer::isFullscreen() const
	{
		return m_cfg.bFullscreen;
	}

	bool Renderer::isVSync() const
	{
		return m_bVSync;
	}

	FrameBuffer* Renderer::getFrameBuffer() const
	{
		return m_frameBuffer;
	}

	RasterizerState* Renderer::getDefaultRasterizerState() const
	{
		return m_pDefaultRasterizerState;
	}

	DepthStencilState* Renderer::getDefaultDepthStencilState() const
	{
		return m_pDefaultDepthStencilState;
	}

	BlendState* Renderer::getDefaultBlendState() const
	{
		return m_pDefaultBlendState;
	}

	RasterizerState* Renderer::getRasterizerState() const
	{
		return m_pRasterizerState;
	}

	DepthStencilState* Renderer::getDepthStencilState() const
	{
		return m_pDepthStencilState;
	}

	BlendState* Renderer::getBlendState() const
	{
		return m_pBlendState;
	}

	void Renderer::project(Vector3& screenPos, const Vector3& worldPos, const Matrix4& matVP, Viewport* pViewport)
	{
		if (!pViewport)
			pViewport = m_frameBuffer->getViewport();

		Vector4 vSSPos = Vector4(worldPos, 1.0);
		vSSPos = matVP.transform(vSSPos);

		Real invW = 1.0f / vSSPos.w;
		vSSPos.x *= invW;
		vSSPos.y *= invW;
		vSSPos.z *= invW;

		screenPos.x = (Real)pViewport->getLeft() + (1.0f + vSSPos.x) * (Real)pViewport->getWidth() * 0.5f;
		screenPos.y = (Real)pViewport->getTop() + (1.0f - vSSPos.y) * (Real)pViewport->getHeight() * 0.5f;
		screenPos.z = (1.0f + vSSPos.z) * 0.5f;
	}

	void Renderer::unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* pViewport)
	{
		if (!pViewport)
			pViewport = m_frameBuffer->getViewport();

		Matrix4 matVPInv = matVP;
		matVPInv.detInverse();

		Vector4 vWSPos = Vector4(screenPos, 1.0f);
		vWSPos.x = (screenPos.x - pViewport->getLeft()) / (Real)pViewport->getWidth() * 2.0f - 1.0f;
		vWSPos.y = 1 - (screenPos.y - pViewport->getTop()) / (Real)pViewport->getHeight() * 2.0f;

		Vector4 vWorld = vWSPos * matVPInv;
		vWorld /= vWorld.w;

		worldPos = (Vector3)vWorld;
	}

	Renderable* Renderer::getRenderable(RenderableID id)
	{
		std::map<ui32, Renderable*>::iterator it = m_renderables.find(id);
		if (it != m_renderables.end())
			return it->second;

		return NULL;
	}

	void Renderer::destroyRenderables(Renderable** renderables, int num)
	{
		for (int i = 0; i < num; i++)
		{
			Renderable* renderable = renderables[i];
			if (renderable)
			{
				std::map<ui32, Renderable*>::iterator it = m_renderables.find(renderable->getIdentifier());
				EchoAssert(it != m_renderables.end());
				m_renderables.erase(it);

				EchoSafeDelete(renderable, Renderable);
				renderables[i] = NULL;
			}
		}
	}

	void Renderer::destroyRenderables(vector<Renderable*>::type& renderables)
	{
		destroyRenderables(renderables.data(), static_cast<int>(renderables.size()));
		renderables.clear();
	}
}
