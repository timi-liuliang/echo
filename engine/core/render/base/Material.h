#pragma once

#include "ShaderProgram.h"
#include "engine/core/render/base/RenderState.h"
#include "engine/core/resource/Res.h"
#include "Texture.h"

namespace Echo
{
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
			TexturePtr	m_texture;
		};
		typedef vector<TextureInfo>::type TextureInfoArray;

	public:
		struct Uniform
		{
			String				m_name;
			ShaderParamType		m_type;
			i32					m_count;
			vector<Byte>::type	m_value;

			// destructor
			Uniform(const String& name, ShaderParamType type, i32 count);
			~Uniform();

			// get value bytes
			ui32 getValueBytes();

			// set value
			void setValue(const void* value);
		};
		typedef map<String, Uniform*>::type ParamMap;

	public:
		Material();
		Material(const ResourcePath& path);
		~Material();

		// load|unload texture
		void loadTexture();
		void unloadTexture();

		// get texure
		Texture* getTexture(const int& index);
		const String& getTexturePath(const int& index);

		// set shader
		void setShaderPath(const ResourcePath& path);
        const ResourcePath& getShaderPath() const { return m_shaderPath; }

		// render stage
		const StringOption& getRenderStage() { return m_renderStage; }
		void setRenderStage(const StringOption& stage) { m_renderStage.setValue(stage.getValue()); }

		// macro
        bool isMacroUsed(const String& macro);
		void setMacros(const String& macros);
        void setMacro(const String& macro, bool enabled);

		// get shader
		ShaderProgram* getShader();

		// operate uniform
		bool isUniformExist(const String& name);
		void setUniformValue(const String& name, const ShaderParamType& type, const void* value);
        
        // get uniforms
		Uniform* getUniform(const String& name);
        void* getUniformValue(const String& name);
        ParamMap& GetUniformSet() { return m_uniforms; }

		// get texture number
		int getTextureNum() { return static_cast<int>(m_textures.size()); }

		// texture
		Texture* setTexture(const String& name, const String& uri);
		Texture* setTexture(const String& name, TexturePtr texture);

		// build shader program
		void buildShaderProgram();

	public:
		// Modified signal
		DECLARE_SIGNAL(Signal0, onShaderChanged)

	protected:
		// propertys (script property or dynamic property)
		virtual const PropertyInfos& getPropertys() override;

		// property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar) override;
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue) override;

	private:
		// add texture
		void addTexture(int idx, const String& name);

		// match uniforms
		void matchUniforms();

		// is global uniform
		bool isGlobalUniform( const String& name);

	private:
		ResourcePath		m_shaderPath;				// shader res path
		StringOption		m_renderStage;
		StringArray			m_macros;
		ShaderProgramPtr	m_shaderProgram;
		ParamMap			m_uniforms;
		TextureInfoArray 	m_textures;
	};
	typedef ResRef<Material> MaterialPtr;
}
