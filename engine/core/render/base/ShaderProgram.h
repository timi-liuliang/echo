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
		virtual ~ShaderProgram();

		// bind shader
        virtual void bind() {}
        virtual void unbind() {}

		// get editable macros
		static StringArray getEditableMacros();
        void setMacros(const StringArray& macros) { m_macros = macros; }

		// get default value of uniform
		const UniformValue* getDefaultUniformValue(const String& name);

		// clear
		void clear();

    public:
        // uniform
        void setUniform(const char* name, const void* value, ShaderParamType uniformType, ui32 count);
        UniformArray* getUniforms(){ return &m_uniforms; }

		// ByteSize
		static int MapUniformTypeSize(ShaderParamType uniformType);
        
    public:
        // type
        const String& getType() const { return m_type; }
        void setType(const String& type) { m_type = type; }
        
        // vs code
        const String& getVsCode() const { return m_vsCode; }
        void setVsCode(const String& vsCode);
        
        // ps code
        const String& getPsCode() const { return m_psCode; }
        void setPsCode(const String& psCode);
        
        // data flow programming shader graph
        const String& getGraph() const { return m_graph; }
        void setGraph(const String& graph) { m_graph = graph; }
        
    public:
        // blend sate
        BlendState* getBlendState();
        void setBlendState(BlendState* blendState) { m_blendState = blendState; }
        
        // depth state
        DepthStencilState* getDepthState();
        void setDepthState(DepthStencilState* depthState) { m_depthState = depthState; }
        
        // rasterizer state
        RasterizerState* getRasterizerState();
        void setRasterizerState(RasterizerState* rasterState) { m_rasterizerState = rasterState; }
        
        // smaple state
        MultisampleState* getMultisampleState();
        void setMultisampleState(MultisampleState* sampleState) { m_multiSampleState = sampleState; }
        
    public:
        // create
        static Res* create();
        
        // default shader
        static ResRef<ShaderProgram> getDefault2D(const StringArray& macros);
        static ResRef<ShaderProgram> getDefault3D(const StringArray& macros);

    public:
		// Modified signal
		DECLARE_SIGNAL(Signal0, onShaderChanged)
        
    protected:
        // Create
        virtual bool createShaderProgram(const String& vsContent, const String& psContent);

        // build
        bool build();
        
        // insert macros
        void insertMacros(String& code);

	protected:
		StringArray			m_macros;
        String              m_type = "glsl";
        String              m_vsCode;
        String              m_psCode;
        String              m_graph;
		BlendState*			m_blendState = nullptr;
		DepthStencilState*	m_depthState = nullptr;
		RasterizerState*	m_rasterizerState = nullptr;
        MultisampleState*   m_multiSampleState = nullptr;
        UniformArray        m_uniforms;
        UniformValuesMap	m_uniformDefaultValues;
	};
	typedef ResRef<ShaderProgram> ShaderProgramPtr;
}
