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
		struct UniformValue
		{
			ShaderProgram::UniformPtr		m_uniform;

			// is empty
			virtual bool isEmpty() = 0;

			// value
			virtual const void* getValue() { return nullptr; }
			virtual void setValue(const void* value) {}

			// get texture path
			virtual Texture* getTexture() { return nullptr; }
			virtual const String& getTexturePath() { return StringUtil::BLANK; }

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
		};

		struct UniformTextureValue : public UniformValue
		{
			String		m_uri;
			TexturePtr	m_texture;

			// destructor
			UniformTextureValue(const ShaderProgram::UniformPtr uniform);

			// is empty
			virtual bool isEmpty() override { return m_uri.empty(); }

			// get texture path
			virtual Texture* getTexture() override;
			virtual const String& getTexturePath() override;

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

		// build shader program
		void buildShaderProgram();

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
		ResourcePath		m_shaderPath;				// shader res path
		StringArray			m_macros;
		ShaderProgramPtr	m_shaderProgram;
		UniformValueMap		m_uniformValues;
	};
	typedef ResRef<Material> MaterialPtr;
}
