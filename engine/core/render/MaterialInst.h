#pragma once

#include "engine/core/render/render/ShaderProgram.h"
#include "engine/core/render/render/RenderState.h"
#include <engine/core/render/render/RenderQueue.h>
#include "engine/core/render/TextureRes.h"
#include "engine/core/Util/Singleton.h"

namespace Echo
{
	struct PBRLight;
	class Material;
	class MaterialController;

	/**
	* 材质实例
	*/
	class MaterialInst
	{
		friend class Model;
		friend class MaterialManager;
		friend class MaterialController;
		typedef vector<String>::type StringVec;
		typedef vector<String>::type::iterator StringVecItor;
		typedef map<int, TextureRes*>::type TextureMap;
		typedef map<int, TextureRes*>::type::iterator TextureMapItor;
		typedef map<int, String>::type TextureNameMap;
		typedef map<int, String>::type::iterator TextureNameMapItor;

	public:
		struct uniform
		{
			String				name;		// 名称
			ShaderParamType		type;		// 类型
			i32					count;		// 数量
			void*				value;		// 值

			// 克隆
			uniform* clone();
		};

		typedef map<String, uniform* >::type ParamMap;

		typedef map<String, MaterialController*>::type MaterialControllerMap;
		typedef MaterialControllerMap::iterator MaterialControllerItor;

	public:
		MaterialInst();
		~MaterialInst();

		// 加载 --> 可以异步。
		bool loadByFile(const String& name, const String& macros);

		// 应用数据到内存 && GPU --> 必须在主线程。
		bool applyLoadedData();

		// 保存
		void saveToFile(const String& name);

		// 克隆
		void cloneFromTemplate(MaterialInst* _template);

		// 参数继承
		void deriveUniforms( MaterialInst* from);

		// 资源加载线程准备纹理
		void prepareTexture();

		// 加载纹理
		void loadTexture();

		// 卸载纹理
		void unloadTexture();

		// 获取纹理
		TextureRes* getTexture(const int& index);

		// 更新函数
		void update(ui32 delta);

		// 获取材质实例名
		const String& getName() const { return m_name; }

		// 设置材质实例模板名
		void setName(const String& name) { m_name = name; }

		// 设置默认渲染队列名
		void setMaterialTemplate(const String& name) { m_materialTemplate = name; }

		// 阶段相关函数
		const String& getStage() { return m_stage; }
		void setStage(const String& stage) { m_stage = stage; }

		// 设置宏定义
		void setMacros(const String& macros);

		// 获取默认渲染队列名
		const String& getMaterialTemplate() const { return m_materialTemplate; }

		// 获取渲染队列
		RenderQueue* getRenderQueue() { return m_renderQueue; }

		// 添加uniform变量
		void AddUniformParam(uniform* param);

		// 判断变量是否存在
		bool isUniformExist(const String& name);

		// 修改uinifrom变量
		void ModifyUniformParam(const String& name, const ShaderParamType& type, void* value);

		// 获取uniform变量的值
		void* GetuniformValue(const String& name, ShaderParamType type);

		// 获取uniform变量
		uniform* GetUniform(const String& name);

		// 获取纹理数量(不包含全局纹理)
		int getTextureNum() { return static_cast<int>(m_TexturesName.size()); }

		// 添加贴图文件名
		void AddTextureName(int idex, const String& name);

		// 设置贴图
		TextureRes* SetTexture(int idex, const String& name);

		// 通过索引获取贴图名字
		const String& GetTextureName(int index) { return m_TexturesName[index]; }

		// 获取属性队列
		ParamMap& GetUniformSet() { return m_unifromParamSet; }

		// 添加参数控制器
		MaterialController* AddController(const String& name, ui32 type);

		// 使用模版添加参数控制器
		MaterialController* AddControllerFromTemplate(const String& name, MaterialController* controllerTemplate);

		// 删除参数控制器
		void DelController(const String& name);

		void LoadBlendState(void* pNode);
		void LoadRasterizerState(void* pNode);
		void LoadDepthStencilState(void* pNode);

		// 获取控制器
		MaterialController* GetController(const String& name);

		ShaderParamType S2ShaderParamType(const String& value);
		String			ShaderParamType2S(const ShaderParamType& type) const;
		static void		delVoid2Value(const ShaderParamType& type, void* value, const int count = 1);
		static void*	createValue2Void(const ShaderParamType& type, const int count = 1);
		static void*	cloneVoid2Value(const ShaderParamType& type, void* value, const int count = 1);
		static void		CopyUniformValue(void* dstValue, const ShaderParamType& type, void* srcValue);

		// 是否是全局变量
		static bool isGlobalUniform(const String& name);

		void*	getUniformValue(const String& name);
		void	modifyUniformValue(const String& name, void* value);
		void	void2s(uniform* param, String& value);
		void	S2Void(const ShaderParamType& type, const String& value, void* dstValue, const int count = 1);

		// 是否使用了宏定义
		bool isMacroUsed(const String& macro);

		// 是否提交到主渲染队列
		bool isSubmitToStageRenderQueue() { return m_isSubmitToStageRenderQueue; }

		// 设置是否提交到主渲染队列
		void setSubmitToStageRenderQueue(bool isSubmitToStageRenderQueue) { m_isSubmitToStageRenderQueue = isSubmitToStageRenderQueue; }

#ifdef ECHO_EDITOR_MODE
		// 设置渲染状态
		void setBlendState(BlendState* state) { m_blendState = state; }
		void setRasterizerState(RasterizerState* state) { m_rasterizerState = state; }
		void setDepthStencil(DepthStencilState* state) { m_depthStencil = state; }

		void setPBRLight(const vector<PBRLight*>::type& lights);

		bool isUsingSceneEnvMap() const;

		// 设置宏定义
		void setMacro(const String& macro, bool enabled);

		void refresh();
#endif

		// 获取渲染状态
		BlendState* getBlendState() { return m_blendState; }
		RasterizerState* getRasterizerState() { return m_rasterizerState; }
		DepthStencilState* getDepthStencilState() { return m_depthStencil; }

		const BlendState::BlendDesc& getBlendDesc() { return m_blendDesc; }
		const RasterizerState::RasterizerDesc& getRasterizerStateDesc() { return m_rasterizerStateDesc; }
		const DepthStencilState::DepthStencilDesc& getDepthStencilDesc() { return m_depthStencilDesc; }

		// 构建渲染队列
		void buildRenderQueue();

	private:	
		// 参数匹配
		void matchUniforms();

		// 准备资源IO
		TextureRes* prepareTextureImp(const String& texName);

	private:
		String				m_name;					// 材质实例名称
		String				m_materialTemplate;		// 所使用的材质模板
		String				m_stage;				// 所处渲染阶段
		StringArray			m_macros;				// 宏定义
		StringArray			m_macrosEx;				// 外部宏定义
		RenderQueue*		m_renderQueue;			// 渲染队列
		ParamMap			m_unifromParamSet;
		ParamMap			m_unifromParamSetFromFile;
		TextureMap			m_textures;
		TextureNameMap 		m_TexturesName;
		int					m_TextureCount;
		MaterialControllerMap m_ControllerList;   //参数控制器列表

		bool					 m_isHaveCustomBlend;
		BlendState*				 m_blendState;
		BlendState::BlendDesc	 m_blendDesc;

		bool							m_isHaveCustomRasterizer;
		RasterizerState*				m_rasterizerState;
		RasterizerState::RasterizerDesc m_rasterizerStateDesc;

		bool								m_isHaveCustomDepthStencil;
		DepthStencilState*					m_depthStencil;
		DepthStencilState::DepthStencilDesc m_depthStencilDesc;

		bool				m_isTemplate;
		bool				m_isSubmitToStageRenderQueue;
	};

	/**
	* 材质实例管理器
	*/
	class MaterialManager
	{
		typedef map<String, MaterialInst* >::type MaterialIstTemplateMap;
		typedef map<String, MaterialInst* >::type::iterator MaterialIstTemplateItor;
		typedef set<MaterialInst* >::type MaterialIstSet;
		typedef set<MaterialInst* >::type::iterator MaterialIstItor;
		typedef list<Material* >::type MaterialList;

		__DeclareSingleton(MaterialManager);

	public:
		MaterialManager();
		~MaterialManager();

		MaterialInst* createMaterialIst(const String& materialName, const String& macros);
		void   destroyMaterialIst(MaterialInst* material);

		// 删除所有材质实例
		void  destroyAllMaterialIst();

		bool AddMaterialIstTemplate(const String& materialName);

		// 获取材质模板
		MaterialInst* getMaterialTemplate(const String& materialName, const String& macros);

		// 删除材质模板
		bool DelMaterialIstTemplate(const String& materialName);

		// 删除所有材质模板
		void DelAllMaterIstialTemplate();

		// 控制器相关操作
		void GetControllerTypes(StringArray& stringArry);
		ui32 GetControllerType(const String& typeName);
		String GetControllerTypeString(ui32 type);

		void setIsEditorModel(bool b) { m_isEditModel = b; }

		Material* createMaterial();
		void destroyMaterial(Material* mtrl);

	private:
		MaterialIstTemplateMap	m_MaterialIstTemplates;
		std::mutex				m_materialIstTemplatesMutex;
		MaterialIstSet			m_MaterialIsts;
		std::mutex				m_materialIstsMutex;
		bool					m_isEditModel;
		MaterialList			m_materialList;
	};
}