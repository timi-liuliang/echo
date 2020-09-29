#include "glsl_cross_compiler.h"
#include <thirdparty/glslang/glslang/Public/ShaderLang.h>
#include <thirdparty/glslang/SPIRV/SpvTools.h>
#include <thirdparty/glslang/SPIRV/GlslangToSpv.h>
#include <thirdparty/spirv-cross/spirv_cross.hpp>
#include <thirdparty/spirv-cross/spirv_glsl.hpp>
#include <thirdparty/spirv-cross/spirv_hlsl.hpp>
#include <thirdparty/spirv-cross/spirv_msl.hpp>
#include "engine/core/log/Log.h"

const TBuiltInResource k_defaultConf = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
//#ifdef NV_EXTENSIONS
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,
//#endif
    
    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }
};

namespace Echo
{
    // Includer
    class Includer : public glslang::TShader::Includer
    {
    public:
        virtual ~Includer() {}
        
        IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
        {
            for (auto i = m_systemDirs.begin(); i != m_systemDirs.end(); ++i) 
			{
                std::string header_path(*i);
                if (header_path.back() != '/')
                    header_path += "/";
                header_path += headerName;
                
                /*if (sx_os_stat(header_path.c_str()).type == SX_FILE_TYPE_REGULAR) 
				{
                    sx_mem_block* mem = sx_file_load_bin(g_alloc, header_path.c_str());
                    if (mem)  
					{
                        return new IncludeResult(header_path, (const char*)mem->data, (size_t)mem->size, mem);
                    }
                }*/
            }
            return nullptr;
        }
        
        IncludeResult* includeLocal(const char* headerName,
                                    const char* includerName,
                                    size_t inclusionDepth) override
        {
            /*
            char cur_dir[256];
            sx_os_path_pwd(cur_dir, sizeof(cur_dir));
            std::string header_path(cur_dir);
            std::replace(header_path.begin(), header_path.end(), '\\', '/');
            if (header_path.back() != '/')
                header_path += "/";
            header_path += headerName;
            
            sx_mem_block* mem = sx_file_load_bin(g_alloc, header_path.c_str());
            if (mem)  {
                return new(sx_malloc(g_alloc, sizeof(IncludeResult)))
                IncludeResult(header_path, (const char*)mem->data, (size_t)mem->size, mem);
            }*/
            return nullptr;
        }
        
        // Signals that the parser will no longer use the contents of the
        // specified IncludeResult.
        void releaseInclude(IncludeResult* result) override
        {
            /*
            if (result) {
                sx_mem_block* mem = (sx_mem_block*)result->userData;
                if (mem)
                    sx_mem_destroy_block(mem);
                result->~IncludeResult();
                sx_free(g_alloc, result);
            }*/
        }
        
        void addSystemDir(const char* dir)
        {
            std::string std_dir(dir);
            std::replace(std_dir.begin(), std_dir.end(), '\\', '/');
            m_systemDirs.push_back(std_dir);
        }
        
    private:
        std::vector<std::string> m_systemDirs;
    };
    
    void GLSLCrossCompiler::setInput(const char* vs, const char* fs, const char* cs)
    {
        m_inputGlsl[ShaderType::VS] = vs ? vs : "";
		m_inputGlsl[ShaderType::FS] = fs ? fs : "";
		m_inputGlsl[ShaderType::CS] = cs ? cs : "";

		m_isNeedUpdateSpriv = true;
		m_isNeedUpdateOutput = true;
    }
    
    const vector<ui32>::type& GLSLCrossCompiler::getSPIRV(ShaderType Type)
    {
        compileGlslToSpirv();
        
        return m_spirv[Type];
    }
    
    std::string GLSLCrossCompiler::getOutput(ShaderLanguage language, ShaderType shaderType)
    {
		// compile gles to spirv
		compileGlslToSpirv();

		// cross compile spirv to target language
		switch (language)
		{
		case ShaderLanguage::GLES: return compileSpirvToGles(shaderType);
		case ShaderLanguage::GLSL: return compileSpirvToGlsl(shaderType);
		case ShaderLanguage::MSL:  return compileSpirvToMsl(shaderType);
		case ShaderLanguage::HLSL: return compileSpirvToHlsl(shaderType);
		}

        return std::string();
    }
    
    void GLSLCrossCompiler::compileGlslToSpirv()
    {
		if (m_isNeedUpdateSpriv)
		{
			// initialize process (wrong place)
			glslang::InitializeProcess();

			// create shader program
			glslang::TProgram* prog = EchoNew(glslang::TProgram);

			// shader
			glslang::TShader* shaders[ShaderType::Total] = { nullptr, nullptr, nullptr };
			EShLanguage types[ShaderType::Total] = { EShLangVertex, EShLangFragment, EShLangCompute };
			for (int i = 0; i < ShaderType::Total; i++)
			{
				if (!m_inputGlsl[i].empty())
				{
					const char* shaderSrc = m_inputGlsl[i].c_str();
					int shaderLen = m_inputGlsl[i].size();
					int defaultVersion = 100; // 110 for desktop

					glslang::TShader* shader = shaders[i] = EchoNew(glslang::TShader(types[i]));
					shader->setStringsWithLengths(&shaderSrc, &shaderLen, 1);
					shader->setInvertY(false);
					shader->setEnvInput(glslang::EShSourceGlsl, types[i], glslang::EShClientVulkan, defaultVersion);
					shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
					shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
					shader->setPreamble(getPreamble());
					shader->addProcesses(getProcesses());

					// parse
					Includer  inc;
					if (shader->parse(&k_defaultConf, defaultVersion, false, EShMsgDefault, inc))
					{
						prog->addShader(shader);
					}
					else
					{
						StringArray lines = StringUtil::Split(shader->getInfoLog(), "\n");
						for (String& line : lines)
						{
							EchoLogError(("[GLSLCrossCompiler] " + line).c_str());
						}
					}
				}
			}

			// link
			EShMessages messages = EShMsgDefault;
			if (prog->link(messages))
			{
				// Output and save SPIR-V for each shader
				for (int i = 0; i < ShaderType::Total; i++)
				{
					glslang::TIntermediate* intermediate = prog->getIntermediate(types[i]);
					if (intermediate)
					{
						glslang::SpvOptions spvOptions;
						spvOptions.validate = true;
						spv::SpvBuildLogger spvBuildLogger;
						glslang::GlslangToSpv(*intermediate, m_spirv[i], &spvBuildLogger, &spvOptions);
						std::string messages = spvBuildLogger.getAllMessages();
						if (!messages.empty())
						{
							EchoLogError(messages.c_str());
						}
					}
				}
			}

			// deallocate shaders
			for (int i = 0; i < ShaderType::Total; i++)
			{
				EchoSafeDelete(shaders[i], TShader);
			}

			// deallocate program
			EchoSafeDelete(prog, TProgram);;

			// finalize process (wrong place)
			glslang::FinalizeProcess();

			m_isNeedUpdateSpriv = false;
		}
    }

    std::string GLSLCrossCompiler::compileSpirvToGles(ShaderType shaderType)
	{
        const vector<ui32>::type& spirv = getSPIRV( shaderType);
        if(!spirv.empty())
        {
            spirv_cross::CompilerGLSL compiler(spirv);
            spirv_cross::ShaderResources       shaderResources = compiler.get_shader_resources();
                
            // modify options
            spirv_cross::CompilerGLSL::Options options = compiler.get_common_options();
            options.flatten_multidimensional_arrays = true;
            options.es = true;
            options.version = 100;
            if ( false/*args.flatten_ubos*/)
            {
                for (spirv_cross::Resource& ubo : shaderResources.uniform_buffers)
                    compiler.flatten_buffer_block(ubo.id);
                for (spirv_cross::Resource& ubo : shaderResources.push_constant_buffers)
                    compiler.flatten_buffer_block(ubo.id);
            }
            compiler.set_common_options(options);
                
            return compiler.compile();
        }
        
		return std::string();
	}
    
	std::string  GLSLCrossCompiler::compileSpirvToMsl(ShaderType shaderType)
	{
        const vector<ui32>::type& spirv = getSPIRV( shaderType);
        if(!spirv.empty())
        {
            spirv_cross::CompilerMSL compiler(spirv);
            spirv_cross::ShaderResources shaderResources = compiler.get_shader_resources();
                
            // modify options
            spirv_cross::CompilerGLSL::Options options = compiler.get_common_options();
            options.flatten_multidimensional_arrays = true;
            if ( false/*args.flatten_ubos*/)
            {
				for (spirv_cross::Resource& ubo : shaderResources.uniform_buffers)
                    compiler.flatten_buffer_block(ubo.id);
                for (spirv_cross::Resource& ubo : shaderResources.push_constant_buffers)
                    compiler.flatten_buffer_block(ubo.id);
            }
                
            // Reset vertex input locations for MSL
            if (shaderType == ShaderType::VS)
            {
                for (int i = 0; i < shaderResources.stage_inputs.size(); i++)
                {
                    spirv_cross::Resource& res = shaderResources.stage_inputs[i];
                    spirv_cross::Bitset mask = compiler.get_decoration_bitset(res.id);
                    if (mask.get(spv::DecorationLocation))
                    {
                        compiler.set_decoration(res.id, spv::DecorationLocation, (uint32_t)i);
                    } 
                }
            }

            compiler.set_common_options(options);
				
			// compile
            return compiler.compile();
        }
        
        return std::string();
	}
    
	std::string GLSLCrossCompiler::compileSpirvToGlsl(ShaderType shaderType)
	{
		const vector<ui32>::type& spirv = getSPIRV(shaderType);
		if (!spirv.empty())
		{
			spirv_cross::CompilerGLSL compiler(spirv);
			spirv_cross::ShaderResources       shaderResources = compiler.get_shader_resources();

			// modify options
			spirv_cross::CompilerGLSL::Options options = compiler.get_common_options();
			options.flatten_multidimensional_arrays = true;
			options.es = false;
			options.version = 450;
			compiler.set_common_options(options);

			return compiler.compile();
		}

		return std::string();
	}
    
    // don't have time implement this function, please learn from glslcc
    // https://github.com/septag/glslcc/blob/master/src/glslcc.cpp
	std::string GLSLCrossCompiler::compileSpirvToHlsl(ShaderType shaderType)
	{
		//spirv_cross::Compiler* compiler = EchoNew(spirv_cross::CompilerGLSL(getSPIRV(shaderType)));

		return std::string();
	}

	const char* GLSLCrossCompiler::getPreamble()
	{
		static std::string preambles;
		preambles += "#extension GL_GOOGLE_include_directive : require\n";
		preambles += "#define POSITION 0\n";
		preambles += "#define NORMAL 1\n";
		preambles += "#define TEXCOORD0 2\n";
		preambles += "#define TEXCOORD1 3\n";
		preambles += "#define TEXCOORD2 4\n";
		preambles += "#define TEXCOORD3 5\n";
		preambles += "#define TEXCOORD4 6\n";
		preambles += "#define TEXCOORD5 7\n";
		preambles += "#define TEXCOORD6 8\n";
		preambles += "#define TEXCOORD7 9\n";
		preambles += "#define COLOR0 10\n";
		preambles += "#define COLOR1 11\n";
		preambles += "#define COLOR2 12\n";
		preambles += "#define COLOR3 13\n";
		preambles += "#define TANGENT 14\n";
		preambles += "#define BINORMAL 15\n";
		preambles += "#define BLENDINDICES 16\n";
		preambles += "#define BLENDWEIGHT 17\n";
		preambles += "#define SV_Target0 0\n";
		preambles += "#define SV_Target1 1\n";
		preambles += "#define SV_Target2 2\n";
		preambles += "#define SV_Target3 3\n";
		preambles += "#define SV_Target4 4\n";
		preambles += "#define SV_Target5 5\n";
		preambles += "#define SV_Target6 6\n";
		preambles += "#define SV_Target7 7\n";

		return preambles.c_str();
	}

	const std::vector<std::string> GLSLCrossCompiler::getProcesses()
	{
		std::vector<std::string> processes;

		return processes;
	}
}
