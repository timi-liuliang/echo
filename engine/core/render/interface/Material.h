#pragma once

#include "engine/core/render/interface/ShaderProgram.h"
#include "engine/core/render/interface/RenderState.h"
#include "engine/core/resource/Res.h"

namespace Echo
{
	class ShaderProgramRes;
	class Material : public Res
	{
		ECHO_RES(Material, Res, ".material", Res::create<Material>, Res::load)

	public:
		// texture info
		struct TextureInfo
		{
			String		m_name;
			int			m_idx = -1;
			String		m_uri;
			Texture*	m_texture = nullptr;
		};
		typedef map<int, TextureInfo>::type TextureInfoMap;

	public:
		struct Uniform
		{
			String				m_name;				// 名称
			ShaderParamType		m_type;				// 类型
			i32					m_count;			// 数量
			Byte*				m_value = nullptr;	// 值

			// destructor
			~Uniform();

			// get value bytes
			ui32 getValueBytes();

			// set value
			void setValue(const void* value);

			// alloc Value
			void allocValue();

			// 克隆
			Uniform* clone();
		};

		typedef map<String, Uniform*>::type ParamMap;

	public:
		Material();
		Material(const ResourcePath& path);
		~Material();

		// release
		void release();

		// 克隆
		void clone(Material* orig);

		// 加载纹理
		void loadTexture();

		// 卸载纹理
		void unloadTexture();

		// 获取纹理
		Texture* getTexture(const int& index);
		const String& getTexturePath(const int& index);

		// 设置默认渲染队列名
		void setShaderPath(const ResourcePath& path);
		void setShaderContent(const String& virtualPath, const char* content);
		const ResourcePath& getShaderPath() const { return m_shaderPath; }

		// 阶段相关函数
		const StringOption& getRenderStage() { return m_renderStage; }
		void setRenderStage(const StringOption& stage) { m_renderStage.setValue(stage.getValue()); }

		// 设置宏定义
		void setMacros(const String& macros);

		// 获取渲染队列
		ShaderProgramRes* getShader();

		// operate uniform
		bool isUniformExist(const String& name);
		void setUniformValue(const String& name, const ShaderParamType& type, const void* value);
		Uniform* getUniform(const String& name);

		// 获取纹理数量(不包含全局纹理)
		int getTextureNum() { return static_cast<int>(m_textures.size()); }

		// 设置贴图
		Texture* setTexture(const String& name, const String& uri);
		Texture* setTexture(const String& name, Texture* texture);

		// 获取属性队列
		ParamMap& GetUniformSet() { return m_uniforms; }

		// get uniform value
		void* getUniformValue(const String& name);

		// 是否使用了宏定义
		bool isMacroUsed(const String& macro);

		// 设置宏定义
		void setMacro(const String& macro, bool enabled);

		// 构建渲染队列
		void buildShaderProgram();

	protected:
		// propertys (script property or dynamic property)
		virtual const PropertyInfos& getPropertys();

		// get property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar);

		// set property value
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue);

	private:
		// 添加贴图文件名
		void addTexture(int idx, const String& name);

		// 参数匹配
		void matchUniforms();

		// is global uniform
		bool isGlobalUniform( const String& name);

	private:
		bool				m_isDirty;					// need rebuild
		ResourcePath		m_shaderPath;				// shader res path
		String				m_shaderContentVirtualPath;	// virtual path of shader content
		const char*			m_shaderContent;			// 官方材质
		StringOption		m_renderStage;				// 所处渲染阶段
		StringArray			m_macros;					// 宏定义
		ShaderProgramRes*	m_shaderProgramRes;			// 对应着色器
		ParamMap			m_uniforms;
		TextureInfoMap 		m_textures;
	};
	typedef ResRef<Material> MaterialPtr;
}