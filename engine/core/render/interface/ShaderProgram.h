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
        // Uniform
        struct Uniform
        {
            String              m_name = "UnKnown";
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
        typedef std::map<int, Uniform> UniformArray;
        
		struct DefaultUniform
		{
			i32 count;
			ShaderParamType type;
			ui32 sizeInByte;
			void* value;

			~DefaultUniform();
		};
		typedef map<String, DefaultUniform*>::type MapDefaultUniforms;

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
		const DefaultUniform* getDefaultUniformValue(const String& name);

		// load and parse by file
		bool loadFromFile(const String& filename, const String& macros);

		// load from content
		bool loadFromContent(const String& fileName, const char* content, const String& macros);

		// save
		virtual void save() override;

		// clear
		void clear();
        
    public: 
        // get physics index by uniform name
        virtual int getParamPhysicsIndex(const String& paramName);
        
        // set uniform
        void setUniform(const char* name, const void* value, ShaderParamType uniformType, ui32 count);
        virtual void setUniform(ui32 physicIdx, const void* value, ShaderParamType uniformType, ui32 count);
        
        // uniform operate
        UniformArray* getUniforms(){ return &m_uniforms; }

		// uniforms
		virtual void bindUniforms() {}
		virtual void unbind() {}

		// bind renderable
		virtual void bindRenderable(Renderable* renderable) {}

		// get attribute location
		virtual i32 getAtrribLocation(VertexSemantic vertexSemantic) { return 0; }

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
		bool loadFromContent(char* content, const String& macros);
		bool loadShaderFrom(void* node, const String& macros);
		bool loadBlendState(void* pNode);
		bool loadRasterizerState(void* pNode);
		bool loadDepthStencilState(void* pNode);
		bool loadDefaultUniform(void* pNode);
		void createBlendState(BlendState::BlendDesc& desc);
		void createDepthState(DepthStencilState::DepthStencilDesc& desc);
		void createRasterizerState(RasterizerState::RasterizerDesc& desc);
		void* createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType);

	protected:
		String				m_macros;
		BlendState*			m_blendState = nullptr;
		DepthStencilState*	m_depthState = nullptr;
		RasterizerState*	m_rasterizerState = nullptr;
		MapDefaultUniforms	m_defaultUniforms;
        bool                m_isLinked = false;
        UniformArray        m_uniforms;
	};
	typedef ResRef<ShaderProgram> ShaderProgramPtr;
}
