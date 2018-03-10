#pragma once

#include "engine/core/Memory/MemManager.h"
#include "Color.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"

namespace Echo
{
	class ECHO_EXPORT_RENDER FrameBuffer
	{
	public:
		static const ui32 MAX_RENDERTARGETS = 4;

	public:
		FrameBuffer();
		virtual ~FrameBuffer();

		enum ClearMask
		{
			CM_COLOR	= 0x00000001, 
			CM_DEPTH	= 0x00000002, 
			CM_STENCIL	= 0x00000004, 
		};

	public:
		virtual void					attachRTV(ui32 idx, RenderTargetView* pRTV);
		virtual void					detachRTV(ui32 idx);
		virtual void					attachDSV(DepthStencilView* pDSV);
		virtual void					detachDSV();
		virtual RenderTargetView*		getRenderTargetView(ui32 idx) const;
		virtual DepthStencilView*		getDepthStencilView() const;
		virtual void					setViewport(Viewport* pViewport);
		virtual Viewport*				getViewport() const; 

	protected:
		RenderTargetView*		m_pRTVs[MAX_RENDERTARGETS];
		DepthStencilView*		m_pDSV;
		Viewport*				m_pViewport;
	};
}