#ifndef __ECHO_EFFECT_SYSTEM_MANAGER_H__
#define __ECHO_EFFECT_SYSTEM_MANAGER_H__

#include "Engine/Core.h"
#include "EffectHeader.h"
#include "EffectController.h"
#include "EffectLayer.h"
#include "engine/core/Util/Singleton.h"
#include "engine/core/Util/StringUtil.h"
#include "engine/core/render/render/Material.h"
#include "engine/core/render/render/RenderState.h"

namespace Echo
{
	class EffectSystemManager;
	class  EffectManualRender
	{
	public:
		virtual void render();
		virtual void beginRender();
	};

	/**
	 * 特效系统管理器 2014-6-20
	 */
	class  EffectSystemManager
	{
		__DeclareSingleton(EffectSystemManager);

		friend class EffectDestroyEvent;
		typedef map<String, EffectSystem*>::type EffectSystemMap;
		typedef map<ui32, EffectSystem*>::type	 EffectSystemIDMap;
		typedef map<ui64, EffectRenderableID>::type RenderableMap;

	public:
		EffectSystemManager();
		~EffectSystemManager();

		// 初始化顶点索引缓冲
		void initBuffer();

		// 更新
		void tick( i32 elapsedTime);

		EffectLayer* createLayer(EFFECT_LAYER_TYPE layerType);
		EffectLayer* cloneLayer(EffectLayer* layer);
		void destroyLayer(EffectLayer* layer);
		// for editor
		EFFECT_LAYER_TYPE getLayerType(const String& typeName);
		String getLayerTypeString(EFFECT_LAYER_TYPE type);

		EffectKeyFrame* createKeyFrame(void);
		EffectKeyFrame* cloneKeyFrame(EffectKeyFrame* keyFrame);
		void destroyKeyFrame(EffectKeyFrame* keyFrame);

		EffectParticleControllerSet* createParticleControllerSet(void);
		EffectParticleControllerSet* cloneParticleControllerSet(EffectParticleControllerSet* particleControllerSet);
		void destroyParticleControllerSet(EffectParticleControllerSet* particleControllerSet);

		EffectMaterial* createMaterial();
		EffectMaterial* cloneMaterial(EffectMaterial* material);
		void destroyMaterial(EffectMaterial* material);

		EffectController* createController(EFFECT_CONTROL_TYPE controlType);
		EffectController* cloneController(EffectController* controller);
		void destroyController(EffectController* controller);
		void getControllerTypes(StringArray& stringArry);
		// for editor
		EFFECT_CONTROL_TYPE getControllerType(const String& typeName);
		String getControllerTypeString(EFFECT_CONTROL_TYPE type);

		// for editor
		void replaceEffectSystemTemplate(const String& name, EffectSystem* system);
		EffectSystem* createEffectSystemTemplate(const String& name);
		EffectSystem* getEffectSystemTemplate(const String& templateName);
		void destoryEffectSystemTemplate(const String& templateName);
		void destroyAllEffectSystemTemplates(void);

		EffectSystemID createEffectSystem(const String& name, const String& templateName, bool isUIeffect = false);
		EffectSystem* createEffectSystem(const String& name);
		EffectSystemID getEffectSystem(const String& name);
		
		// 根据ID获取特效系统
		EffectSystem* getEffectSystem( EffectSystemID id);
		void destroyEffectSystem( EffectSystemID& effectSystemID);
		void destroyAllEffectSystems();
		void removeEffectSystemFromMap(EffectSystem* effectSystem);

		void createRenderableSet();
		void destroyRenderableSet();

		// 删除未使用的Renderable
		void destroyAllUsedRenderables();

		// 根据纹理，特效组，混合模式，材质获取Renderable
		EffectRenderable* getRenderable(TextureRes* texture, EFFECT_RENDER_GROUP rg, EFFECT_BLEND_MODE bm, EffectMaterial* pEm  );
		void beginRender();
		void endRender(const std::vector<EffectRenderable*>* effectRenderables = NULL);
		void endRenderUI(const std::vector<EffectRenderable*>* effectRenderables = NULL);

		void renderDistortionLayer();

		// 是否有空间扭曲特效层
		bool isHaveDistortionLayer();

		// 上传数据
		void updateBufferData( const EffectRenderable& effectRenderable);

		void renderGLowEffectLayer();

		void setEnableRender( bool _val ){ mEnableRender = _val; }

		bool getEnableRender() const { return mEnableRender; }

		void setRenderUIEffectOnly(bool _val){ mRenderUIEffectOnly = _val; }

		bool getRenderUIEffectOnly() const { return mRenderUIEffectOnly; }

		// 设置特效系统所使用的摄像机
		void setCamera( Camera* camera) { m_camera = camera; }

		void setUserCamera( Camera* camera) { m_userCamera = camera; }

		// 获取特效当前使用的摄像机
		Camera* getCamera(EffectRenderable* pRenderable);

		// 设置是否开启扭曲效果
		void setEnableDistortionRender( bool _val);

		// 获取顶点缓冲
		GPUBuffer* getVertexBuffer() const { return m_vertexBuffer; }
		
		// 获取索引缓冲
		GPUBuffer* getIndexBuffer() const { return m_indexBuffer; }

		// 设置渲染级别
		void setRenderLevel( EFFECT_RENDER_LEVEL level) { m_renderlevelThreshhold = static_cast<EFFECT_RENDER_LEVEL>( ERL_Super-level);}

		// 设置渲染级别阀值
		void setRenderLevelThreshhold( EFFECT_RENDER_LEVEL level) { m_renderlevelThreshhold = level; } 

		// 获取渲染级别
		EFFECT_RENDER_LEVEL getRenderLevelThreshhold() const { return m_renderlevelThreshhold; }

		// 获取粒子池
		EffectParticlePool* getParticlePool() { return m_particlePool; }

		void setWireFramePattern();

		void closeWireFramePattern();

		Material* getMtlUV2(){ return mMtlUV2; }

	private:
		// 扩容未使用的Renderables
		void enlargeUnUsedRenderables();

		void clearUsedRenderablesRenderData();

		void _renderRenderable(EffectRenderable* pRenderable, int uv2_MatIndex, int uv2_TextureIndex, int uv2_TextureIndex1, int uv2_blendModeIndex, int uv2_dissolutionIndex);

		RenderQueue* _getEffectRenderQueue();

	protected:
		EffectSystem* _createSystemImpl(const String& name);
		void _destroySystemImpl(EffectSystem* particleSystem);

	protected:
		Camera*							m_camera;					// 当前使用摄像机
		Camera*							m_userCamera;					// 当前使用摄像机
		EFFECT_RENDER_LEVEL				m_renderlevelThreshhold;	// 渲染级别				
		bool							mEnableRender;
		bool							mRenderUIEffectOnly;
		RenderInput::VertexElementList	m_vertexElementList;		// 顶点数据格式
		GPUBuffer*						m_vertexBuffer;				// 顶点缓冲
		GPUBuffer*						m_indexBuffer;				// 索引缓冲
		EffectSystemMap					mEffectSystems;				// 特效系统集合
		EffectSystemIDMap				m_effectSystemIDs;			// 已创建特效
		EffectSystemMap					mEffectSystemTemplates;
		RenderableMap					m_usedRenderables;			// 当前使用中的所有特效渲染单元
		RenderInput*					mEffectRenderInput;
		RenderInput*					mEffectUV2RenderInput;
		int								mSPFogParamIndex;
		int								mSPHeightFogParamIndex;
		int								mSPMatIndex;
		int								mSPTextureIndex;
		const SamplerState*				mEffectSamplerState;		// 采样状态
		BlendState*						mEffectAlphaLighting;
		BlendState*						mEffectHighLighting;
		BlendState*						mEffectWireFrameLighting;
		DepthStencilState*				mEffectDepthState;
		DepthStencilState*				mEffectUnWriteDepthState;
		RasterizerState*				mEffectRasterizerState;
		bool							mRenderalbeSetCreated;
		EffectManualRender				mEffectManualRender;
		Material*						mMtlDistortion;
		TextureRes*						mTextureNoise;
		Material*						mMtlUV2;					// 有2套UV的特效
		bool							m_bRenderGlowOver;
		EffectParticlePool*				m_particlePool;				// 粒子池
		bool							bUseWireFrameMode;
		RenderQueue*					mEffectRenderQueue;

	public:
		HashNode<EffectRenderable>**	mEffectRenderablePtr;
		HashNode<EffectMaterial>**		mEffectMaterialPtr;
	};
}
#endif
