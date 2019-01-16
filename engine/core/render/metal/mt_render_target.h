//#ifndef ECHO_GLES2RENDERTAGER
//#define ECHO_GLES2RENDERTAGER
//
//#include "render/RenderTarget.h"
//
//namespace Echo
//{
//	class GLES2TargetGPUProxy;
//
//	class GLES2RenderTarget : public RenderTarget
//	{
//		friend class GLES2RenderTargetManager;
//	protected:
//		virtual bool doCreate();
//		virtual bool doCreateCubemap();
//		virtual bool doBeginRender( bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue );
//		virtual bool doEndRender();
//		virtual bool doInvalidateFrameBuffer(bool invalidateColor, bool invalidateDepth, bool invalidateStencil);
//		virtual void bindTarget_left();
//		virtual void bindTarget_right();
//		virtual void doClear(bool clear_color, const Color& color, bool clear_depth, float depth_value, bool clear_stencil, ui8 stencil_value);
//		virtual void doOnResize( ui32 _width, ui32 _height );
//		virtual void doSetCubeFace( Texture::CubeFace cf );
//		virtual bool doSaveTo( const char* file );
//
//		virtual bool doStoreDefaultRenderTarget();
//
//		virtual bool doRestoreDefaultRenderTarget();
//
//	private:
//		GLuint m_fbo;
//		GLuint m_rbo;
//
//	public:
//		GLES2RenderTarget(ui32 _id, ui32 _width, ui32 _height, PixelFormat _pixelFormat, const Options& option = Options());
//		virtual ~GLES2RenderTarget();
//	};
//
//// 	class ECHO_GLES2RENDER_API GLES2RenderTargetManager : public RenderTargetManager
//// 	{
//// 	public:
//// 		GLES2RenderTargetManager();
//// 		virtual ~GLES2RenderTargetManager();
//// 
//// 	protected:
//// 		virtual bool doBeginRenderTarget( RenderTargetID _id, bool _clearColor, const Color& _backgroundColor,  bool _clearDepth, float _depthValue, bool _clearStencil, ui8 stencilValue );
//// 
//// 		virtual bool doEndRenderTarget( RenderTargetID _id );
//// 
//// 		virtual RenderTarget* doCreateRenderTarget( RenderTargetID _id, RenderTargetCreateFlags _cf, float _width, float _height, PixelFormat _pixelFormat, bool _hasDepth = false, bool _msaa = false, bool _multi_resolution = false, bool _is_cubemap =false );
//// 
//// 		virtual bool doStoreDefaultRenderTarget();
//// 
//// 		virtual bool doRestoreDefaultRenderTarget();
//// 	};
//
//#ifdef ECHO_PLATFORM_WINDOWS
//#define DIB_HEADER_MARKER   ((WORD) ('M' << 8) | 'B')
//#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
//WORD  DIBNumColors(void* lpbi);
//WORD  PaletteSize(void* lpbi);
//#endif
//}
//
//
//#endif
