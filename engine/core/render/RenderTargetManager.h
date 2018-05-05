#pragma once

#include "engine/core/Base/EchoDef.h"
#include "engine/core/render/render/Texture.h"
#include "engine/core/render/render/RenderTarget.h"
#include "engine/core/render/render/Renderer.h"
#include "VRModeAUX.h"

namespace Echo
{
	enum RenderTargetID
	{
		RTI_DefaultBackBuffer,
		RTI_SceneColorMap,
		RTI_LDRSceneColorMap,
		RTI_LDRVRSceneColorMap,

		// 遮蔽渲染分两个阶段
		RTI_CoverageMap,

		RTI_ShadowMap,
		RTI_ShadowMapVBlur,

		// 水面反射
		RTI_WaterRefectMap,
		// 以后还有水面折射
		RTI_WaterRefractionMap,

		// 下面三条定义顺序不能变（深度缓冲复用需要保证resize顺序）
		RTI_FinalLDRBuffer,
		RTI_HalfResDepth,
		// 多PassGlow
		RTI_Glow,

		RTI_PostProcess,

		// Post Image Effect begin
		RTI_POSTIMAGEEFFECT_BEGIN,
		RTI_POSTIMAGEEFFECT_END = RTI_POSTIMAGEEFFECT_BEGIN + 50,
		// Post Image Effect end

		// UI Actor
		RTI_UI_ACTOR,

		// GUI RENDER TARGET BEGIN
		RTI_GUI_BEGIN,
		RTI_GUI_END = RTI_GUI_BEGIN + MAX_GUI_RT_COUNT,
		// GUI RENDER TARGET END

		RTI_LIGHTPROBE_BEGIN = RTI_GUI_END + 10,

		RTI_LIGHTPROBE_END = (MAX_LIGHTPROBE_COUNT * 6),

#ifdef ECHO_EDITOR_MODE
		RTI_IBLSPHERE22D,    //天空球展开成2D平面
		RTI_IBLVBlur,
		RTI_IBLHBlur,
#endif


		RTI_End
	};

	class TextureRes;
	/**
	 * 渲染目标管理器
	 */
	class RenderTargetManager
	{
	public:
		virtual ~RenderTargetManager();

		// 获取渲染实例
		static RenderTargetManager* instance();

		// 初始化
		bool initialize();

		// !!!note!!! : do NOT use the RTI_DefaultBackBuffer as _srcID !
		bool updateRenderTarget( ui32 _srcID, ui32 _dstID, bool _clear_color =false, bool _clear_depth = false, bool _clear_stencil =false );
		bool updateRenderTargetWithFilter( ui32 _srcID, ui32 _dstID, bool _clear_color =false, bool _clear_depth = false, bool _clear_stencil =false );
		bool downsampleDepthTarget(ui32 srcID, ui32 dstID);

		void changeFilterBlendmapName(const String& texName);

		// 开始渲染目标
		bool beginRenderTarget( ui32 _id,bool _clearColor = true, const Color& _backgroundColor = Renderer::BGCOLOR,  bool _clearDepth = true, float _depthValue = 1.0f, bool _clearStencil = false, ui8 stencilValue = 0, ui32 rbo = RenderTarget::Invalid_Value );

		bool endRenderTarget( ui32 _id )
		{
			EchoAssert( m_inUsingRenderTarget == _id );

			return doEndRenderTarget( _id );
		}
		
		bool invalidateFrameBuffer(ui32 id, bool invalidateColor, bool invalidateDepth, bool invalidateStencil)
		{
			//EchoAssert( m_inUsingRenderTarget == id );
			return doInvalidateFrameBuffer(id, invalidateColor, invalidateDepth, invalidateStencil);
		}

		// 创建渲染目标
		RenderTarget* createRenderTarget( ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, RenderTarget::Options option = RenderTarget::Options())
		{
			return doCreateRenderTarget(_id, _width, _height, _pixelFormat, option);
		}

		void destroyRenderTargetByID(ui32 _id);

		bool storeDefaultRenderTarget()
		{
			return doStoreDefaultRenderTarget();
		}

		bool restoreDefaultRenderTarget()
		{
			return doRestoreDefaultRenderTarget();
		}

		// 获取当前下在使用的RenderTarget
		RenderTarget* getInUsingRenderTarget()
		{
			return getRenderTargetByID( getInUsingRenderTargetID());
		}

		// 根据ID获取渲染目标
		RenderTarget* getRenderTargetByID( ui32 _id )
		{
			RenderTargetMap::iterator fit = m_mapRenderTargets.find( _id );
			if( fit == m_mapRenderTargets.end() )
			{
				EchoLogError( "Could not found RenderTarget[%d]", _id );

				return NULL;
			}

			return fit->second;
		}

		ui32 getInUsingRenderTargetID() const
		{
			return m_inUsingRenderTarget;
		}

		void renderScreenAlignedQuad();
		void renderScreenAlignedQuadQuarterLB();

		bool checkRenderTargetIDValid( ui32 _id )
		{
			if( _id == RTI_End) return false;

			if( NULL != getRenderTargetByID(_id) )
			{
				return true;
			}

			return false;
		}

		Material* getBaseMaterial() const
		{
			return m_pMaterialBaseUpdate;
		}

		Material* getFilterMaterial() const
		{
			return m_pMaterialFilterUpdate;
		}

		// 当屏幕大小改变时调用 
		void onScreensizeChanged( ui32 _width, ui32 _height );

		const ui32& logicWidth() const
		{
			return m_logicWidth;
		}

		const ui32& logicHeight() const
		{
			return m_logicHeight;
		}

		bool getEnableLogicResolution() const
		{
			return m_bEnableMultiResolution;
		}

		const float& logicWidthScale() const
		{
			return m_logicWidthScale;
		}

		const float& logicHeightScale() const
		{
			return m_logicHeightScale;
		}

		void setRenderTargetCubeFace( ui32 _id, Texture::CubeFace cf )
		{
#ifdef ECHO_PLATFORM_WINDOWS
			RenderTarget* rt = getRenderTargetByID(_id);

			if( rt )
			{
				EchoAssert( rt->isCubemap() )
				rt->doSetCubeFace(cf);
			}
#endif
		}

		ui32 allocateRenderTargetID(const String& name);

		// NOTE: O(n) time complexity
		ui32 getRenderTargetID(const String& name) const;

		// 根据名称获取渲染目标
		RenderTarget* getRenderTarget(const String& name);

	protected:
		// 调整大小
		void AdaptSize(ui32 nWidth, ui32 nHeight, ui32& outWidth, ui32& outHeight);


		virtual bool doBeginRenderTarget( ui32 _id, bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue );
		virtual bool doEndRenderTarget( ui32 _id );
		virtual RenderTarget* doCreateRenderTarget( ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, RenderTarget::Options option = RenderTarget::Options());
		virtual bool doStoreDefaultRenderTarget();
		virtual bool doRestoreDefaultRenderTarget();
		
		virtual bool doInvalidateFrameBuffer(ui32 _id, bool invalidateColor, bool invalidateDepth, bool invalidateStencil);

		bool createScreenAlignedQuad_ext();
		bool createScreenAlignedQuadQuarterLB_ext();

	private:
		RenderTargetManager();

	protected:
		RenderTargetMap			m_mapRenderTargets;					// 所有渲染目标
		map<ui32, String>::type	m_allocatedRenderTargetIDs;
		vector<String>::type	m_preDefinedTargetIDs;
		Renderable*				m_pScreenAlignedQuad;
		Renderable*				m_pScreenAlignedQuadQuarterLB;
		Renderable*				m_pScreenAlignedQuadDownsampleDepth;
		Material*				m_pMaterialBaseUpdate;
		Material*				m_pMaterialNightSight;
		Material*				m_pMtlAddBlend;
		Material*				m_pMaterialDownsampleDepth;
		ui32					m_inUsingRenderTarget;
		bool					m_bEnableMultiResolution;
		bool					m_bEnableFilter;
		Material*				m_pMaterialFilterUpdate;
		String					m_pFilterBlendmapName;
		TextureRes*				m_filterBlendmapTexture;
		TextureSampler			m_pFilterBlendmap;
		Renderable*				m_pScreenAlignedQuadFilter;
		ui32					m_FilterTex[2];	
		float					m_logicWidthScale;
		float					m_logicHeightScale;
		ui32					m_logicWidth;						// 逻辑宽
		ui32					m_logicHeight;						// 逻辑高

		// vr Mode
	public:
		FovPort					m_fovPort[2];
		Vector2					m_vrTexSize;
	};
}
