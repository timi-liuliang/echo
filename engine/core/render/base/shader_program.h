#pragma once

#include "render_state.h"
#include <utility>
#include "engine/core/util/Array.hpp"
#include "engine/core/resource/Res.h"
#include "mesh/mesh_vertex_data.h"

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
    class Material;
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

        // enum texture type
        enum TextureType
        {
            General,    // general 2d texture
            Normal,     // normal map texture
            Atla,       // atlas of 2d texture
            Cube,       // cube map
            Ibl,        // image based lighting, HDRI environment map
        };

        // Uniform
        struct Uniform : public Refable
        {
            String              m_name = "UnKnown";
			ShaderType			m_shader = ShaderType::Total;
            ShaderParamType     m_type = SPT_UNKNOWN;
            int                 m_count = -1;
            int                 m_sizeInBytes = 0;
            int                 m_location = -1;
            vector<Byte>::type  m_value;

            Uniform() {}
            ~Uniform() {}

            // set value
			void setValue(const void* value);
            const vector<Byte>::type& getValue() { return m_value; }

            // value default
            virtual void setValueDefault(const void* value) {}
            virtual vector<Byte>::type& getValueDefault() { static vector<Byte>::type empty; return empty; }

            // texture default
            virtual void setTextureDefault(const ResourcePath& path) {}
            virtual const ResourcePath& getTextureDefault() { return ResourcePath::BLANK; }
        };

        // data
        struct UniformNormal : public Uniform
        {
			vector<Byte>::type  m_valueDefault;

            // set value default
            virtual void setValueDefault(const void* value) override;
            virtual vector<Byte>::type& getValueDefault() override { return m_valueDefault; }
        };

        // texture
        struct UniformTexture : public Uniform
        {
			TextureType         m_type = TextureType::General;
			ResourcePath		m_texturePathDefault = ResourcePath("", ".png|.rt");

            virtual void setTextureDefault(const ResourcePath& path) override;
            virtual const ResourcePath& getTextureDefault() override;
        };

        typedef ResRef<Uniform> UniformPtr;
        typedef map<String, UniformPtr>::type UniformMap;

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

		// clear
		void clear();

    public:
		// is global uniform
		static bool isGlobalUniform(const String& name);

        // uniform
        void setUniform(const char* name, const void* value, ShaderParamType uniformType, ui32 count);
        UniformPtr getUniform(const String& name);

        // get all uniforms
        UniformMap& getUniforms(){ return m_uniforms; }

		// ByteSize
		static int mapUniformTypeSize(ShaderParamType uniformType);
        
    public:
        // type
        const String& getType() const { return m_type; }
        void setType(const String& type) { m_type = type; }
        
        // vs code
        const String& getVsCode() const;
        void setVsCode(const String& vsCode);
        
        // ps code
        const String& getPsCode() const;
        void setPsCode(const String& psCode);
        
        // data flow programming shader graph
        const String& getGraph() const { return m_graph; }
        void setGraph(const String& graph) { m_graph = graph; }

    public:
		// property value
		virtual bool getPropertyValue(const String& propertyName, Variant& oVar) override;
		virtual bool setPropertyValue(const String& propertyName, const Variant& propertyValue) override;
        
    public:
        // blend sate
        BlendState* getBlendState();
        void setBlendState(BlendState* blendState) { m_blendState = blendState; }
        
        // depth state
        DepthStencilState* getDepthStencilState();
        void setDepthStencilState(DepthStencilState* depthState) { m_depthState = depthState; }
        
        // raster state
        RasterizerState* getRasterizerState();
        void setRasterizerState(RasterizerState* rasterState) { m_rasterizerState = rasterState; }
        
        // sample state
        MultisampleState* getMultisampleState();
        void setMultisampleState(MultisampleState* sampleState) { m_multiSampleState = sampleState; }

    public:
		// cull mode
		const StringOption& getCullMode() const { return m_cullMode; }
        void setCullMode(const StringOption& option);

        // blend mode
		const StringOption& getBlendMode() const { return m_blendMode; }
		void setBlendMode(const StringOption& option);
        
    public:
        // create
        static Res* create();
        
        // default shader
		static ResRef<ShaderProgram> getDefault(const String& shaderPath);
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
        UniformMap          m_uniforms;

    protected:
        StringOption        m_cullMode = StringOption("CULL_BACK", { "CULL_NONE", "CULL_FRONT", "CULL_BACK"});
        StringOption        m_blendMode = StringOption("Opaque", { "Opaque", "Transparent" });
	};
	typedef ResRef<ShaderProgram> ShaderProgramPtr;
}
