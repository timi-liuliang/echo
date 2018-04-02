#pragma once

#include <engine/core/Base/EchoDef.h>
#include <engine/core/Memory/MemManager.h>

namespace Echo
{
	// 特效品质
	enum EffectQuality
	{
		EQ_Low = 1,
		EQ_High,
		EQ_Auto,
	};

	enum WaterQuality
	{
		WQ_Low = 1,
		WQ_Mid,
		WQ_High,
	};

	/**
	 * EngineSettingsMgr 2014-12-5
	 * 引擎设置管理器,所有引擎设置相关接口必须由此类完成。
	 */
	class EngineSettingsMgr
	{
		friend class Root;
	public:
		EngineSettingsMgr();
		~EngineSettingsMgr();

		// 获取实例
		static EngineSettingsMgr* instance();

		// 应用设置
		void Apply(const String& fileName);

		// 设置特效是否支持空间扭曲
		void setEnableDistortion(bool isDistortion) { m_bEnableDistortion = isDistortion; }

		// 是否空间扭曲
		bool isEnableDistortion() const { return m_bEnableDistortion; }

		bool isEnableLensFlare() const { return false; }

		bool isEnableDownsampleDepth() const { return false; }

		bool isEnableToneMapping() const { return m_bEnableToneMapping; }
		void setEnableToneMapping(bool val) { m_bEnableToneMapping = val; }

		bool isEnableFXAA() const { return m_bEnableFXAA; }
		void setEnableFXAA(bool val) { m_bEnableFXAA = val; }

		bool isEnableFocusBlur() const { return m_bEnableFocusBlur; }
		void setEnableFocusBlur(bool val) { m_bEnableFocusBlur = val; }

		bool isEnableMainActorShadow() const { return m_bEnableMainActorShadow; }
		void setEnableMainActorShadow(bool val) { m_bEnableMainActorShadow = val; }
		
		// 是否显示遮挡剔除物(编辑器模式可用)
		bool isShowOcclusionOccluder() { return m_isShowOcclusionOccluder; }

		// 设置是否渲染物理
		void setEnablePhysicsRender(bool physicsRender);

		// 是否渲染物理
		bool isEnablePhysicsRender() { return m_bPhysicsRender; }

		// 设置是否使用流加载
		void setEnableStreamThread(bool isUseStreamThread);

		void setEnableSmallObjectCull(bool enable);
		bool isEnableSmallObjectCull() { return m_bEnableSmallObjectCull; }

		// 是否使用VR方式渲染
		bool isInitVRMode(){ return m_bInitVrMode; }
		bool isUseVRMode(){ return m_bUseVRMode; }
		void setUserVRMode(bool use = true){ m_bUseVRMode = use; }

		// 获取批次最大特效数量
		ui32 getMaxEffectBatchNum() const { return m_maxEffectBatchNum; }

		// 获取默认粒子大小(第批次)
		ui32 getDefaultEffectParticleSizePerBatch() const { return m_defaultEffectParticleSizePerBatch; }

		// 获取每批次粒子最大允许数量
		ui32 getMaxEffectParticlesSizePerBatch() const { return m_maxEffectParticlesSizePerBatch; }

		// 设置是否开启多线程渲染
		void setEnableMultiThreadRendering(bool isEnable);

		// 设置高斯模糊品质
		void setGuassScaleSetting(float guass_setting_scale);

		// 获得高斯模糊品质
		float getGuassScaleSetting();

		// 设置水面品质
		void setWaterQuality(WaterQuality water_qulity);

		// 获得水面品质
		WaterQuality getWaterQuality();

		// 设置水面反射范围
		void setWaterRefectRadius(float refect_radius){ m_waterRefectRadius = refect_radius; }
		float getWaterRefectRadius(){ return m_waterRefectRadius; }

		void setAutoOptMatInt(bool setting) { m_bAutoOptMatInt = setting; }
		// 自动优化材质（目前只做了阴影相关的）
		bool isAutoOptMatInt() const { return m_bAutoOptMatInt; }

		void setEnableRenderLayer(bool setting) { m_bEnableRenderLayer = setting; }
		bool isEnableRenderLayer() { return m_bEnableRenderLayer; }

		void setSlowDownExclusiveUI(bool value);
		bool isSlowDownExclusiveUI() const;

		void setEnableAnimOptimize(bool enable);
		bool isEnableAnimOptimize() const;

		void setEnableBatchMerge(bool enable);
		bool isEnableBatchMerge() const;
		

	protected:
		bool			m_isShowOcclusionOccluder;					// 是否显示遮挡物
		bool			m_bEnableHighEffectActor;
		bool			m_bEnableBloom;
		bool			m_bEnableToneMapping;
		bool			m_bEnableFXAA;
		bool			m_bEnableCoverage;
		bool			m_bEnableFilterAdditional;
		bool			m_bEnableDistortion;
		bool			m_bEnableGlow;
		bool			m_bEnableLensFlare;
		bool			m_bPhysicsRender;
		bool			m_bEnableCalcThread;
		bool			m_bEnableStreamThread;
		bool			m_bUseVRMode;							// 针对某一个场景是否开启VR渲染模式	
		bool			m_bInitVrMode;							// 在引擎初始化时决定运行期是否有可能开启VR
		bool			m_bSlowDownExclusiveUI;
		ui32			m_maxEffectBatchNum;					// (特效)特效批次数量
		ui32			m_defaultEffectParticleSizePerBatch;	// (特效)每批次粒子最大数量
		ui32			m_maxEffectParticlesSizePerBatch;		// (特效)每批次最大粒子数量
		EffectQuality	m_effectQuality;						// (特效)特效品质
		float			m_guassScaleSetting;					// 高斯模糊品质
		WaterQuality	m_waterQuality;							// 水面品质
		float			m_waterRefectRadius;					// 水面发射范围
		bool			m_bAutoOptMatInt;						// 自动优化材质（目前只做了阴影相关的）
		bool			m_bEnableSmallObjectCull;				// 视角边缘小物体剪裁
		ui32			m_shadowMapSize;						// 阴影图大小
		bool			m_bEnableRenderLayer;					// 开启场景物体的渲染层级
		bool			m_bEnableFocusBlur;
		bool			m_bEnableAnimOptimize;
		bool			m_bEnableMainActorShadow;				// 是否仅渲染主角影子
		bool			m_bEnableBatchMerge;
	};
}