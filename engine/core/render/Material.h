#pragma once

#include "engine/core/render/render/ShaderProgram.h"
#include "engine/core/render/render/RenderState.h"
#include "engine/core/render/TextureRes.h"

namespace Echo
{
	class ShaderProgramRes;

	/**
	* 材质实例
	*/
	class Material
	{
	public:
		// texture info
		struct TextureInfo
		{
			String		m_name;
			int			m_idx = -1;
			String		m_uri;
			TextureRes* m_texture = nullptr;
		};
		typedef map<int, TextureInfo>::type TextureInfoMap;

	public:
		struct Uniform
		{
			String				m_name;				// 名称
			ShaderParamType		m_type;				// 类型
			i32					m_count;				// 数量
			Byte*				m_value = nullptr;	// 值

			// destructor
			~Uniform();

			// get value bytes
			i32 getValueBytes();

			// set value
			void setValue(const void* value);

			// alloc Value
			void allocValue();

			// 克隆
			Uniform* clone();
		};

		typedef map<String, Uniform* >::type ParamMap;

	public:
		// create a material instance
		static Material* create();

		// release
		void release();

		// 应用数据到内存 && GPU --> 必须在主线程。
		bool applyLoadedData();

		// 克隆
		void clone(Material* orig);

		// 资源加载线程准备纹理
		void prepareTexture();

		// 加载纹理
		void loadTexture();

		// 卸载纹理
		void unloadTexture();

		// 获取纹理
		TextureRes* getTexture(const int& index);

		// 获取材质实例名
		const String& getName() const { return m_name; }

		// 设置材质实例模板名
		void setName(const String& name) { m_name = name; }

		// 设置默认渲染队列名
		void setMaterialTemplate(const String& name) { m_materialTemplate = name; }
		const String& getMaterialTemplate() const { return m_materialTemplate; }

		// 设置使用官方材质
		void setOfficialMaterialContent(const char* content) { m_officialMaterialContent = content; }

		// 阶段相关函数
		const String& getRenderStage() { return m_renderStage; }
		void setRenderStage(const String& stage) { m_renderStage = stage; }

		// 设置宏定义
		void setMacros(const String& macros);

		// 获取渲染队列
		ShaderProgramRes* getMaterial() { return m_material; }

		// operate uniform
		bool isUniformExist(const String& name);
		void setUniformValue(const String& name, const ShaderParamType& type, void* value);
		Uniform* getUniform(const String& name);

		// 获取纹理数量(不包含全局纹理)
		int getTextureNum() { return static_cast<int>(m_textures.size()); }

		// 设置贴图
		TextureRes* setTexture(const String& name, const String& uri);
		TextureRes* setTexture(const String& name, TextureRes* textureRes);

		// 获取属性队列
		ParamMap& GetUniformSet() { return m_unifroms; }

		// get uniform value
		void* getUniformValue(const String& name);

		// 是否使用了宏定义
		bool isMacroUsed(const String& macro);

		// 设置宏定义
		void setMacro(const String& macro, bool enabled);

		// 构建渲染队列
		void buildRenderQueue();

	private:
		// 添加贴图文件名
		void addTexture(int idx, const String& name);

	private:
		Material();
		~Material();

		// 参数匹配
		void matchUniforms();

		// 准备资源IO
		TextureRes* prepareTextureImp(const String& texName);

	private:
		String				m_name;						// 材质实例名称
		String				m_materialTemplate;			// 所使用的材质模板
		const char*			m_officialMaterialContent;	// 官方材质
		String				m_renderStage;				// 所处渲染阶段
		StringArray			m_macros;					// 宏定义
		ShaderProgramRes*			m_material;					// 对应材质
		ParamMap			m_unifroms;
		TextureInfoMap 		m_textures;
	};
}