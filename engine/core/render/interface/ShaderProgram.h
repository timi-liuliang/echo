#pragma once

#include "RenderState.h"
#include "Shader.h"
#include <utility>
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
            String              m_name;            // √˚≥∆
            ShaderParamType     m_type;            // ¿‡–Õ
            int                 m_count;        // ¥Û–°
            int                 m_sizeInBytes;    // ¥Û–°
            int                 m_location;        // Œª÷√
            Byte*               m_origin_value;
            Byte*               m_value;        // µ±«∞÷µ
            bool                m_isDirty;        // ÷µ «∑Ò∑¢…˙¡À±‰ªØ
            
            // ππ‘Ï∫Ø ˝
            Uniform()
            : m_name("UnKnown")
            , m_type(SPT_UNKNOWN)
            , m_count(-1)
            , m_location(-1)
            , m_origin_value(NULL)
            , m_value(NULL)
            , m_isDirty(true)
            {}
            
            // Œˆππ∫Ø ˝
            ~Uniform()
            {
                ECHO_FREE(m_value);
            }
            
            // ÷ÿ÷√µ±«∞÷µ
            void resetValue()
            {
                m_isDirty = true;
            }
            
            // …Ë÷√÷µ
            void setValue(const void* value)
            {
                EchoAssert(value);
                m_origin_value = (Byte*)value;
                
                // ∑÷≈‰∂—ø’º‰
                if (!m_value)
                {
                    m_value = (Byte*)ECHO_MALLOC(m_sizeInBytes);
                }
                
                // »Ù‘‡±Íº«Œ™true,‘Ú∏’≥ı ºªØ£¨÷±Ω”øΩ±¥º¥ø…
                if (m_isDirty)
                {
                    memcpy(m_value, value, m_sizeInBytes);
                }
                else if ( memcmp(m_value, value, m_sizeInBytes) != 0)
                {
                    m_isDirty = true;
                    memcpy(m_value, value, m_sizeInBytes);
                }
            }
            
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

		// get Blend state
		BlendState* getBlendState() const { return m_blendState; }

		// get depth state
		DepthStencilState* getDepthState() const { return m_depthState; }

		// get rasterizer state
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
        // shaders operate
        Shader* getShader(Shader::ShaderType type) const { return m_pShaders[(ui32)type]; }
        virtual bool attachShader(Shader* pShader);
        virtual Shader* detachShader(Shader::ShaderType type);
        virtual bool linkShaders() {return false;}
        
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

	private:
		// private functions
		bool loadFromContent(char* content, const String& macros);
		bool loadShaderFrom(void* node, const String& macros);
		bool loadBlendState(void* pNode);
		bool loadRasterizerState(void* pNode);
		bool loadDepthStencilState(void* pNode);
		bool loadDefaultUniform(void* pNode);
		void createBlendState(BlendState::BlendDesc& desc);
		void createDepthState(DepthStencilState::DepthStencilDesc& desc);
		void createRasterizerState(RasterizerState::RasterizerDesc& desc);
		bool createShaderProgram(const String& vsContent, const String& psContent);
		void* createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType);

	protected:
		Shader::ShaderDesc	m_shaderDesc;							// Shader info
		BlendState*			m_blendState = nullptr;
		DepthStencilState*	m_depthState = nullptr;
		RasterizerState*	m_rasterizerState = nullptr;
		MapDefaultUniforms	m_defaultUniforms;
        Shader*             m_pShaders[Shader::ST_SHADERCOUNT];
        bool                m_bLinked;
        UniformArray        m_uniforms;
	};
	typedef ResRef<ShaderProgram> ShaderProgramPtr;
}
