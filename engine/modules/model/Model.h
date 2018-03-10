#pragma once

#include <engine/core/Util/Array.hpp>
#include "engine/core/Util/PtrMonitor.h"
#include <engine/modules/Light/LightArray.h>
#include <engine/core/Resource/EchoThread.h>
#include <engine/core/main/EngineSettings.h>

namespace Echo
{
	/**
	 * 模型资源
	 */
	class Camera;
	class ModelManager;
	class MaterialInstance;
	class Mesh;
	class RenderInput;
	class Renderable;
	class TextureRes;
	class Texture;
	class Skeleton;
	class AnimBlender;
	struct TextureSampler;
	class Model : public PtrMonitor<Model>
	{
		friend class ModelManager;
		friend class ModelLoadEvent; 
		friend class ModelUnLoadEvent; 

	public: 
		static void ReleaseMesh(Mesh* p); 
		static void DeleteLightArray(LightArray* p); 

	public:
		// 几何体类型
		enum GeometryType
		{
			GT_StaticMesh,		// 静态模型
			GT_Skin,			// 动画模型
			GT_Terrain,			// 地形
		};

		// 渲染阶段枚举
		enum RenderPhaseType
		{
			RP_Normal0 = 0,
			RP_Normal1,
			RP_Normal2,
			RP_Normal3,
			RP_ShadowMap,		// 官方
			RP_Total,
		};

		enum DoubleLMPType
		{
			LMP_USE_NONE	= 0,
			LMP_USE_FIRST	= 1,
			LMP_USE_SECOND	= 2,
			LMP_USE_BOTH	= 3,
		};

		enum DynamicMtlIst
		{
			RECEIVE_SHADOW = 1,
			DOUBLE_LMP = 2,
		};

		// 静态烘焙配置
		struct LightmassConfig
		{
			std::string		m_textureDiffuse;		// 漫反射贴图
			std::string		m_textureNormal;		// 法线贴图
			std::string		m_textureEmissive;		// 自发光贴图
			std::string		m_textureSpecular;		// 高光贴图
		};
		typedef vector<LightmassConfig>::type LightmassConfigArray;

		// 模型信息
		struct Info
		{
			String					m_name;				// 模型名称
			vector<String>::type	m_originNames;		// inst Model，的原model名称
			bool					m_isCastShadow;		// 是否投射阴影
			bool					m_isReceiveShadow;	// 是否接收阴影
			bool					m_isActor;			// 是否是Actor
			bool					m_isMainActor;		// 是否是MainActor
			bool					m_isForUI;
			GeometryType			m_geometryType;		// 几何体类型
			String					m_meshName;			// mesh名
			array<String, RP_Total>	m_materialInsts;	// 材质实例(模型可以有多个材质，类比于人可以有多件衣服)
			String					m_materialInstsAll;	// 所有可使用的材质实例
			String					m_lightArray;		// 光源阵列
			LightmassConfigArray	m_lightmassConfig;	// 烘焙配置

			// 构造函数
			Info() 
			: m_geometryType(GT_StaticMesh)
			, m_isCastShadow(false)
			, m_isReceiveShadow(false)
			, m_isActor(false)
			, m_isMainActor(false)
			, m_isForUI(false)
			{}

			// 是否为蒙皮几何体
			bool isSkinModel() const { return m_geometryType == GT_Skin ? true : false; }
		};

		// 渲染阶段
		struct RenderPhase
		{
			String							m_tag;									// 阶段标识;
			vector<MaterialInstance*>::type	m_materialInsts;						// 材质实例
			vector<vector<MaterialInstance*>::type>::type	m_materialInsts_dym;	// 动态材质实例
			vector<RenderInput*>::type		m_renderInputs;							// 几何体数据
			vector<Renderable*>::type		m_renderables;							// 可渲染对象

			RenderPhase(){}
			~RenderPhase();

			// 清理
			void clean();

			// 更新
			void update( ui32 delta);
		};
		typedef array<RenderPhase*, RP_Total> RenderPhaseArray;
		typedef map<String, RenderPhase*>::type RenderPhaseMap;
		typedef vector<PBRLight*>::type PBRLights;

	public:
		// for async: Load Model Config From Xml.
		void loadFromFile(); 

		// for async: Apply Model Config.
		void applyLoadedData(); 

		//卸载 --> must same as load thread.
		void unload();

		// Model重新加载,暂时也用于重新加载Mesh 材质 光阵列;
		void reload();

		////设置场景节点
		//void attachTo(node* pNode){ m_pSceneNode = pNode; }

		//// 获取场景节点
		//node* getSceneNode() const{ return m_pSceneNode; }

		// 创建渲染单元
		void createRenderable( bool isForUI=false);

		// 创建渲染单元
		void createRenderable(RenderPhase& phase, bool isForUI = false);

		// 更新
		void update(bool isForUI = false, ui32 delta = 0);

		// 提交到渲染队列
		void submitToRenderQueue();

		// 提交到阴影渲染队列
		void submitToSMRenderQueue();

		//是否带动画
		bool isSkinModel() const { return m_info.isSkinModel(); }

		// 获取mesh包围盒
		Box getLocalMeshAABB();

		//获取本地包围盒
		Box getLocalAABB() const;

		// 获取世界包围盒
		const Box& getWorldAABB() const { return m_worldBox; }

		//获取模型信息
		const Model::Info& getModelInfo() const { return m_info; }

		// 获取模型信息(指针)
		Model::Info* getModelInfoPtr() { return &m_info; }

		//设置模型信息
		void setInfo(const Model::Info& info) { m_info = info; }

		// 获取材质实例
		MaterialInstance* getMaterialInstance(RenderPhaseType type, i32 subMeshIdx) { return m_phases[type]->m_materialInsts[subMeshIdx]; }

		size_t getMaterialInstanceNum(RenderPhaseType type) { return m_phases[type]->m_materialInsts.size(); }

		// 设置材质实例
		void setMaterialInstance(RenderPhaseType type, i32 subMeshIdx, MaterialInstance* mate);

		// 设置Mesh
		void setMesh(Mesh* mesh);

		// 设置原模型名，没有为空
		void setOriginNames(vector<String>::type& originNames){ m_info.m_originNames = originNames;}

		// 设置光阵列
		void setLightArray(LightArray* lightArray); 

		// 获取mesh
		Mesh* getMesh() { return m_mesh.get(); }

		// 获取模型阶段
		RenderPhase* getPhase(RenderPhaseType type) { return m_phases[type]; }

		// 设置纹理
		void setTexture(RenderPhaseType type, i32 subMeshIdx, i32 index, const String& name);

		//设置骨骼
		void  setSkeleton(Skeleton* pSkeleton);

		// 设置动画混合
		void  setAnimBlender(AnimBlender* pAnimation);

		// 可见性设置
		void  setVisible(bool visibel) { m_IsVisible = visibel; }

		// 是否可见
		bool  isVisible() { return m_IsVisible; }
 
		// 设置光照图纹理
		void setLMTexture(ui32 subId, TextureRes* pTexture, ui32 lmID = 0);

		// 设置光照图相关参数
		void setLightMapUVBias(i32 subId, const Vector4& lmUVparam1, const Vector3& scale1, const Vector4& lmUVparam2 = Vector4::ZERO, const Vector3& scale2 = Vector3::ONE);

		// 卸载光照图纹理
		void unloadLMTexture();

		// 获取全局变量值
		void* getGlobalUniformValue(const String& name, i32 subMesh, bool isUIModel = false);
		
		// 额外的计算
		void* extraCalcUniform(const String& name, void* value, i32 subMeshId);

		bool modifyUniformValue( const String& name, void* value, RenderPhaseType phase = RP_Normal0 );
		void modifySubMeshUniformValue(i32 subId, const String &name, void* value, RenderPhaseType phase = RP_Normal0);

		bool saveMesh( bool useNornalMap = true );

		// 获取光阵列
		LightArray* getLightArray() { return m_lightArray.get(); }
		
		// 获取是否可用.
		bool isEnable() const { return m_isEnable; }

		// 设置加载完成状态.
		void setEnable(bool isEnable) { m_isEnable = isEnable; }

		void setUseXRay(bool state, Vector4* color = NULL);

		// 获取烘焙配置
		LightmassConfigArray& getLightmassConfig() { return m_info.m_lightmassConfig; }


		void updateCameraPos();

		bool haveWaterMaterial(){ return m_have_water_material; }

		bool isSkyBox(){ return m_isNeedUpdateMatSky; }

		void execute_render();

	public:
		/**
		\Create Phase by Phase, Used for Model replace MaterialInsts.
		\param[name] used to remember the result
		\param[materialTemplate]
		\param[srcPhase] srcPhase used for create new phase
		\ return true if succed else false
		*/
		bool createLodRenderPhase(const String& name, const String& materialName, bool isDeriveUniformsFromOtherPhase=false, RenderPhase* parentPhase=nullptr);

		/**
		\ Get Lod Phase
		*/
		RenderPhase* getLodPhase(const String& name);
		
		/**
		\ Delete Phase by Name
		*/
		bool deleteLodPhase(const String& phaseName);

		/**
		\ Swap Phase 
		*/
		bool swapPhase(const String& phaseName, RenderPhaseType phaseSlot);
		
		bool swapPhase(RenderPhase* phase, RenderPhaseType phaseSlot);

		void updateRenderablesPosition();

		void resetRenderInput();

	protected:
		// 获取
		Texture* getGlobalTexture(const String& name, const int subMesh = 0);
		const TextureSampler* getGlobalTextureSample(const String& name, const int subMesh = 0);

		// 加载材质实例
		void loadMaterialInstanceFromFile();

		// 加载模型
		void loadMeshFromFile();

		// 加载光阵列
		void loadLightArrayFromFile();

		// 
		void applyLoadedMaterialInstanceData();

		//
		void applyLoadedMeshData();

		//
		void applyLoadedLightArrayData();

		//
		void recursionDymMat(int type, ui32 num, ui32 flag, ui32 exp, String& matName, String& macros);

	public:
		Model(const Model::Info& modelInfo);
		~Model();

	private:
		// 更新子模型用骨骼动画数据
		void refreshSubMeshBoneMatRows();

		// 创建投射阴影渲染数据
		void createRenderabeSM(bool isForUI);

		// 创建材质实例
		MaterialInstance* createMaterialInst(const String& materialName);

		// 应用材质数据
		void applyLoadedMaterialInstanceData( RenderPhase& phase);

	private:
		Info						m_info;				// 模型信息(加载存储)
		RenderPhaseArray			m_phases;			// 渲染阶段
		RenderPhaseMap				m_phasesLod;		// 渲染阶段(LOD)

		typedef std::unique_ptr<Mesh, decltype(&ReleaseMesh)> MeshPtr; 
		MeshPtr						m_mesh;				// 关联模型

		typedef std::unique_ptr<LightArray, decltype(&DeleteLightArray)> LightArrayPtr; 
		LightArrayPtr				m_lightArray;		// 光阵列
		PBRLights					m_pbrlights;
		bool						m_IsVisible;        // 是否可见

		//带动画相关数据
		Skeleton*					m_pSkeleton;
		AnimBlender*				m_pAnimBlender;
		ui32						m_boneCount;
		Vector4*					m_boneMatRows;			// 骨架对应整体骨骼矩阵
		vector<Vector4*>::type		m_subMeshBoneMatRows;	// 子模型对应骨骼矩阵

		//全局数据
		Matrix4						m_matWV;
		Matrix4						m_matWVP;				// 世界观察投影矩阵
		Matrix4						m_matWVPSM;				// 阴影贴图对应世界观察投影矩阵
		Matrix4						m_matWSM;
		Real						m_alpha;				// alpha值
		bool						m_isNeedUpdateMatWater;	// 是否更新水矩阵
		Matrix4						m_matWVPWater;		    // 水面反射世界观察投影矩阵
		bool						m_isNeedUpdateMatSky;	// 是否更新天空盒矩阵
		Matrix4						m_matWVPSky;			// 天空盒使用世界观察投影矩阵

		vector<Vector4>::type	    m_lightmapUV1;			// 光照图相关参数
		vector<Vector4>::type	    m_lightmapUV2;
		vector<Vector3>::type       m_lightmapScale1;
		vector<Vector3>::type       m_lightmapScale2;
		vector<Vector4>::type		m_lightmapUV1_bak;
		vector<Vector3>::type		m_lightmapScale1_bak;

		vector<TextureSampler>::type		m_submeshAlbedo;

		float						m_currentTime;			// 当前时间(生命)
		Box							m_worldBox;				// 世界包围盒

		bool						m_isEnable;				// 是否加载完成。

		bool						m_have_water_material;	// 是否有水材质
		float						m_original_water_refect_degree; //原始的水面材质反射度
		float						m_close_refect_degree;  // 关闭水面反射
		WaterQuality				m_last_water_quality;	// 上一次的水面材质质量

		bool						m_isUseXRay;
		vector<Renderable*>::type	m_xrayRenderables;	// 可渲染对象供Xray用
		Vector4						m_xrayColor;		// xray 颜色
		
		bool						m_isUseDynamicMatIst; // 是否开启动态切换材质
		bool						m_isInShowdownBox;  // 是否在投射阴影包围盒中
		ui32						m_dymOffset;

		float						m_LM1ToLM2;				// 两张光照图之间插值
		
		float						m_Sky1ToSky2;			// 两张天空贴图之间插值

		ui32						m_LMSlot1;
		ui32						m_LMSlot2;

#ifdef ECHO_EDITOR_MODE
		vector<Vector3>::type		m_hsvColor0;
		vector<Vector3>::type		m_hsvColor1;
		vector<Vector3>::type		m_hsvColor2;
#endif
	};

	/**
	 * 模型管理器
	 **/
	class ModelManager
	{
		friend class ModelTemplateLoadEvent; 
		friend class ModelUnLoadEvent; 
		friend class ModelLoadEvent;

		typedef map<String, Model::Info* >::type Model_InfoMap;
		typedef map<String, Model::Info* >::type::iterator Model_InfoItor;
		typedef set<Model* >::type ModelSet;
		typedef set<Model* >::type::iterator ModelItor;

		__DeclareSingleton(ModelManager);

	public:
		ModelManager();
		~ModelManager();

		// 创建模型
		void createModel(const String& modelName, bool isSync, std::function<bool(Model* )> onCreateModelComplete, bool isForUI = false);

		// 销毁模型
		void destroyModel(Model* model, bool isSync = false);

		// 删除所有模型
		void destroyAllModels();

		// 添加模型模板
		bool addModelTemplate(const String& modelName);

		// 删除模型模板
		bool delModelTemplate(const String& modelName);

		// 获取模型模板
		Model::Info* getModelTemplate(const String& modelName);

		// 删除所有模型模板
		void delAllModelTemplate();

		// 获取投影阴影模型包围盒
		void buildBoxSM(Box& oBox, const Vector3& mainPos, Camera* cam, float shadowDistance = 10.f);

		// 加载模型文件
		bool loadModelByFile( const String& modelName, Model::Info* modelInfo);

		// 保存模型文件
		void saveModelToFile(const String& modelName ,const Model::Info& modelInfo);

		bool refreshModelTemplate(const String& modelName);

	private:
		// 添加模型
		void addModel(Model* model);

		// 准备模板
		Model::Info* prepareModelTemplate(const String& modelName);

	private:
		ModelSet		m_Models;
		ModelSet		m_modelsCastShadow;			// 投射阴影模型
		Model_InfoMap	m_ModelInfoMap;

		float			m_criticalValue;
	};

	class ModelLoadEvent : public StreamThread::Task
	{
	public:
		ModelLoadEvent(Model* model, std::function<bool(Model*)> onModelLoadComplete);
		virtual ~ModelLoadEvent();

		// 失败后是否重复通知
		virtual bool isRepeatNotifyIfFail()	{ return true; }

		// 是否自动销毁
		virtual bool isAutoDestroy() { return true; }

		// 处理
		virtual bool process();

		// 完成
		virtual bool finished();

		// 返回该事件的优先级别
		virtual StreamThread::TaskLevel GetLevel() { return StreamThread::TL_Normal; }

	private:
		Model*	m_model;
		ui32	m_modelId;

		std::function<bool(Model*)> m_onModelLoadComplete; 
	};

	class ModelUnLoadEvent : public StreamThread::Task
	{
	public:
		ModelUnLoadEvent(Model* model, std::function<void(Model*)> onDestroyComplete);
		virtual ~ModelUnLoadEvent();

		// 失败后是否重复通知
		virtual bool isRepeatNotifyIfFail()	{ return true; }

		// 是否自动销毁
		virtual bool isAutoDestroy() { return true; }

		// 处理
		virtual bool process();

		// 完成
		virtual bool finished();

		// 返回该事件的优先级别
		virtual StreamThread::TaskLevel GetLevel() { return StreamThread::TL_Normal; }

	private:
		Model* m_model;

		std::function<void(Model*)> m_onDestroyComplete;
	};
}