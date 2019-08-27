#pragma once

#include "RenderState.h"
#include <utility>
#include "engine/core/util/Array.hpp"
#include "engine/core/resource/Res.h"
#include "mesh/MeshVertexData.h"

namespace Echo
{
    // Param Type
    enum ShaderParamType
    {
        SPT_UNKNOWN,
        SPT_INT,
        SPT_FLOAT,
        SPT_VEC2,
        SPT_VEC3,
        SPT_VEC4,
        SPT_MAT4,
        SPT_TEXTURE,
        
        SPT_MAX
    };
    
	class Renderer;
	class Renderable;
	class ShaderProgram : public Res
	{
		ECHO_RES(ShaderProgram, Res, ".shader", ShaderProgram::create, ShaderProgram::load);

	public:
		// ShaderType
		enum ShaderType
		{
			VS = 0,
			FS,
			CS,
			Total,
		};

        // Uniform
        struct Uniform
        {
            String              m_name = "UnKnown";
			ShaderType			m_shader = ShaderType::Total;
            ShaderParamType     m_type = SPT_UNKNOWN;
            int                 m_count = -1;
            int                 m_sizeInBytes = 0;
            int                 m_location = -1;
            Byte*               m_value = nullptr;
            bool                m_isDirty = true;
            
            Uniform() {}       
            ~Uniform() { ECHO_FREE(m_value); }
            
            // reset
            void resetValue() { m_isDirty = true; }
            
            // set value
			void setValue(const void* value);
        };
        typedef std::map<String, Uniform> UniformArray;
        
        // UniformValue
		struct UniformValue
		{
			i32 count;
			ShaderParamType type;
			ui32 sizeInByte;
			void* value;

			~UniformValue();
		};
		typedef map<String, UniformValue*>::type UniformValuesMap;

	public:
		ShaderProgram();
		ShaderProgram(const ResourcePath& path);
		~ShaderProgram();

		// bind shader
        virtual void bind() {}

		// get states
		BlendState* getBlendState() const { return m_blendState; }
		DepthStencilState* getDepthState() const { return m_depthState; }
		RasterizerState* getRasterizerState() const { return m_rasterizerState; }

		// is have macro
		bool hasMacro(const char* const macro) const;

		// get editable macros
		static StringArray getEditableMacros(const String& shaderFileName);

		// get default value of uniform
		const UniformValue* getDefaultUniformValue(const String& name);

		// load and parse by file|content
		bool loadFromFile(const String& filename, const String& macros);
		bool loadFromContent(const String& fileName, const char* content, const String& macros);

		// save
		virtual void save() override;

		// clear
		void clear();
        
    public:   
        // set uniform
        void setUniform(const char* name, const void* value, ShaderParamType uniformType, ui32 count);
        
        // uniform operate
        UniformArray* getUniforms(){ return &m_uniforms; }

		// uniforms
		virtual void bindUniforms() {}
		virtual void unbind() {}

		// bind renderable
		virtual void bindRenderable(Renderable* renderable) {}

		// ByteSize
		static int getUniformByteSizeByUniformType(ShaderParamType uniformType);

	public:
        // create
        static Res* create();
        
		// load
		static Res* load(const ResourcePath& path);

	protected:
		// Create
		virtual bool createShaderProgram(const String& vsContent, const String& psContent);

	private:
        // load
		bool loadFromContent(char* content, const String& macros);
		bool loadShaderFrom(void* node, const String& macros);

        // load states
		bool loadBlendState(void* pNode);
		bool loadRasterizerState(void* pNode);
		bool loadDepthStencilState(void* pNode);

        // create states
        void createBlendState(BlendState::BlendDesc& desc);
        void createDepthState(DepthStencilState::DepthStencilDesc& desc);
        void createRasterizerState(RasterizerState::RasterizerDesc& desc);

        // uniforms
		bool loadDefaultUniform(void* pNode);
		void* createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType);

	protected:
		String				m_macros;
		BlendState*			m_blendState = nullptr;
		DepthStencilState*	m_depthState = nullptr;
		RasterizerState*	m_rasterizerState = nullptr;
        UniformArray        m_uniforms;
        UniformValuesMap	m_uniformDefaultValues;
	};
	typedef ResRef<ShaderProgram> ShaderProgramPtr;
}
