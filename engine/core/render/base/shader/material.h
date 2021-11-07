#pragma once

#include "shader_program.h"
#include "engine/core/render/base/render_state.h"
#include "engine/core/resource/Res.h"
#include "base/texture/texture.h"
#include "base/texture/texture_atla.h"

namespace Echo
{
	class Material : public Res
	{
		ECHO_RES(Material, Res, ".material", Res::create<Material>, Res::load)

	public:
		struct UniformValue
		{
			ShaderProgram::UniformPtr		m_uniform;

			virtual ~UniformValue() {}

			// is empty
			virtual bool isEmpty() = 0;

			// value
			virtual const void* getValue() { return nullptr; }
			virtual void setValue(const void* value) {}
			virtual void setValue(float value) {}

			// get texture path
			virtual Texture* getTexture() { return nullptr; }
			virtual ResourcePath getTexturePath() { return StringUtil::BLANK; }

			// texture
			virtual Texture* setTexture(const String& uri) { return nullptr; }
			virtual Texture* setTexture(TexturePtr texture) { return nullptr; }
		};

		struct UniformNormalValue : public UniformValue
		{		
			vector<Byte>::type				m_value;

			// destructor
			UniformNormalValue(const ShaderProgram::UniformPtr uniform);

			// is empty
			virtual bool isEmpty() override { return m_value.empty(); }

			// value
			virtual const void* getValue() override;
			virtual void setValue(const void* value) override;
			virtual void setValue(float value) override;
		};

		struct UniformTextureValue : public UniformValue
		{
			Material*		m_owner;
			ResourcePath	m_uri;
			TexturePtr		m_texture;
			TextureAtlaPtr	m_atla;

			// destructor
			UniformTextureValue(const ShaderProgram::UniformPtr uniform, Material* owner);

			// is empty
			virtual bool isEmpty() override { return m_uri.isEmpty(); }

			// get texture path
			virtual Texture* getTexture() override;
			virtual ResourcePath getTexturePath() override;

			// texture
			virtual Texture* setTexture(const String& uri) override;
			virtual Texture* setTexture(TexturePtr texture) override;
		};
		typedef map<String, UniformValue*>::type UniformValueMap;

	public:
		Material();
		Material(const ResourcePath& path);
		~Material();

		// get shader
		ShaderProgram* getShader();

		// render stage
		const String& getRenderStage();

		// set shader
		void setShaderPath(const ResourcePath& path);
		const ResourcePath& getShaderPath() const { return m_shaderPath; }

		// macro
        bool isMacroUsed(const String& macro);
		void setMacros(const String& macros);
        void setMacro(const String& macro, bool enabled);

		// operate uniform
		bool isUniformExist(const String& name);
        
        // get uniforms
		UniformValue* getUniform(const String& name);
        UniformValueMap& GetAllUniforms() { return m_uniformValues; }

		// set uniform value
		void setUniformValue(const String& name, const void* value);
		void setUniformValue(const String& name, float value) { setUniformValue(name, &value); }
		void setUniformTexture(const String& name, const String& texturePath);
		void setUniformTexture(const String& name, TexturePtr texture);

		// build shader program
		void buildShaderProgram();

	public:
		// raster state
		RasterizerState* getRasterizerState();
		void setRasterizerState(RasterizerState* rasterState) { m_rasterizerState = rasterState; }

	public:
		// Modified signal
		DECLARE_SIGNAL(Signal0, onShaderChanged)

	protected:
		// property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar) override;
		virtual bool getPropertyValueDefault(const String& propertyName, Variant& oVar) override;
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue) override;

		// property flags
		virtual i32 getPropertyFlag(const String& propertyName) override;

	private:
		// match uniforms
		void matchUniforms();

	private:
		ResourcePath			m_shaderPath;				// shader res path
		StringArray				m_macros;
		ShaderProgramPtr		m_shaderProgram;
		UniformValueMap			m_uniformValues;
		DepthStencilStatePtr    m_depthState;
		RasterizerStatePtr		m_rasterizerState;
	};
	typedef ResRef<Material> MaterialPtr;
}
