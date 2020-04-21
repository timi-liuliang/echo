#include "ShaderProgram.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/magic_enum.hpp"
#include "engine/core/log/Log.h"
#include "engine/core/util/Exception.h"
#include "Renderer.h"
#include "image/PixelFormat.h"
#include "engine/core/io/IO.h"
#include <thirdparty/pugixml/pugixml.hpp>
#include "glslcc/GLSLCrossCompiler.h"

static const char* g_2dVsCode = R"(#version 450

// uniforms
layout(binding = 0) uniform UBO
{
	mat4 u_WorldMatrix;
	mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_UV;

// outputs
layout(location = 0) out vec2 v_TexCoord;

void main(void)
{
    vec4 position = vs_ubo.u_WorldMatrix * vec4(a_Position, 1.0);
    position = vs_ubo.u_ViewProjMatrix * position;
    gl_Position = position;
    
    v_TexCoord = a_UV;
}
)";

static const char* g_2dPsCode = R"(#version 450

precision mediump float;

// uniforms
layout(binding = 0) uniform UBO
{
    float u_Alpha;
} fs_ubo;

// uniforms
layout(binding = 3) uniform sampler2D BaseColor;

// inputs
layout(location = 0) in vec2  v_TexCoord;

// outputs
layout(location = 0) out vec4 o_FragColor;

void main(void)
{
    vec4 textureColor = texture(BaseColor, v_TexCoord);
    vec4 finalColor = textureColor;

#ifdef ALPHA_ADJUST
    finalColor.a = finalColor.a * fs_ubo.u_Alpha;
#endif

    o_FragColor = finalColor;
}
)";

static const char* g_3dVsCode = R"(
#version 450

#define ENABLE_VERTEX_NORMAL

// uniforms
layout(binding = 0) uniform UBO
{
    mat4 u_WorldMatrix;
    mat4 u_ViewProjMatrix;
} vs_ubo;

// inputs
layout(location = 0) in vec3 a_Position;

// outputs
layout(location = 0) out vec3 v_Position;

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 1) in vec3 a_Normal;
layout(location = 1) out vec3 v_Normal;
#endif

void main(void)
{
    vec4 position = vec4(a_Position, 1.0);
    position = vs_ubo.u_WorldMatrix * position;

    v_Position  = position.xyz;
    gl_Position = vs_ubo.u_ViewProjMatrix * position;

#ifdef ENABLE_VERTEX_NORMAL
	v_Normal = normalize(vec3(vs_ubo.u_WorldMatrix * vec4(a_Normal.xyz, 0.0)));
#endif
}
)";

static const char* g_3dPsCode = R"(
#version 450

#define ENABLE_VERTEX_NORMAL

precision mediump float;

// inputs
layout(location = 0) in vec3  v_Position;

#ifdef ENABLE_VERTEX_NORMAL
layout(location = 1) in vec3 v_Normal;
#endif

// outputs
layout(location = 0) out vec4 o_FragColor;

// functions
#define SRGB_FAST_APPROXIMATION

vec3 SRgbToLinear(vec3 srgbIn)
{
#ifdef SRGB_FAST_APPROXIMATION
    return pow(srgbIn,vec3(2.2));
#else
    return srgbIn;
#endif
}

vec3 LinearToSRgb(vec3 linearIn)
{
#ifdef SRGB_FAST_APPROXIMATION
    return pow(linearIn,vec3(1.0/2.2));
#else
    return srgbIn;
#endif
}

void main(void)
{
	vec3 __BaseColor = SRgbToLinear(vec3(0.75));

    vec3 _lightDir = normalize(vec3(1.0, 1.0, 1.0));
    vec3 _lightColor = SRgbToLinear(vec3(1.2, 1.2, 1.2));
    vec3 _AmbientColor = SRgbToLinear(vec3(0.4, 0.4, 0.4));
    __BaseColor = max(dot(v_Normal, _lightDir), 0.0) * _lightColor * __BaseColor + _AmbientColor;

    o_FragColor = vec4(LinearToSRgb(__BaseColor.rgb), 1.0);
}
)";

namespace Echo
{
    static bool convert(String& type, String& vsSrc, String& psSrc)
    {
        if (type == "glsl")
        {
            // convert to metal
            if (Renderer::instance()->getType() == Renderer::Type::Metal)
            {
                GLSLCrossCompiler glslCompiler;

                // set input
                glslCompiler.setInput(vsSrc.c_str(), psSrc.c_str(), nullptr);

                vsSrc = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::MSL, Echo::GLSLCrossCompiler::ShaderType::VS).c_str();
                psSrc = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::MSL, Echo::GLSLCrossCompiler::ShaderType::FS).c_str();
            }
			else if (Renderer::instance()->getType() == Renderer::Type::OpenGLES)
			{
				GLSLCrossCompiler glslCompiler;

				// set input
				glslCompiler.setInput(vsSrc.c_str(), psSrc.c_str(), nullptr);

				vsSrc = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::GLES, Echo::GLSLCrossCompiler::ShaderType::VS).c_str();
				psSrc = glslCompiler.getOutput(Echo::GLSLCrossCompiler::ShaderLanguage::GLES, Echo::GLSLCrossCompiler::ShaderType::FS).c_str();
			}
        }
        else
        {
            if (Renderer::instance()->getType() != Renderer::Type::OpenGLES)
            {
                vsSrc.clear();
                psSrc.clear();
            }
        }
        
        return true;
    }
    
	void ShaderProgram::Uniform::setValue(const void* value)
	{
        if (value)
        {
			m_value.resize(m_sizeInBytes);
			memcpy(m_value.data(), value, m_sizeInBytes);
        }
        else
        {
            m_value.clear();
        }
	}

    void ShaderProgram::UniformNormal::setValueDefault(const void* value)
    {
        if (value)
        {
			m_valueDefault.resize(m_sizeInBytes);
			memcpy(m_valueDefault.data(), value, m_sizeInBytes);
        }
    }

    void ShaderProgram::UniformTexture::setTextureDefault(const ResourcePath& path)
    {
        m_texturePathDefault = path;
    }

    const ResourcePath& ShaderProgram::UniformTexture::getTextureDefault()
    {
        return m_texturePathDefault;
    }

	ShaderProgram::ShaderProgram()
	{
	}

	ShaderProgram::ShaderProgram(const ResourcePath& path)
		: Res(path)
	{

	}

	ShaderProgram::~ShaderProgram()
	{
		clear();
	}

	void ShaderProgram::bindMethods()
	{
        CLASS_BIND_METHOD(ShaderProgram, getType, DEF_METHOD("getType"));
        CLASS_BIND_METHOD(ShaderProgram, setType, DEF_METHOD("setType"));
        CLASS_BIND_METHOD(ShaderProgram, getVsCode, DEF_METHOD("getVsCode"));
        CLASS_BIND_METHOD(ShaderProgram, setVsCode, DEF_METHOD("setVsCode"));
        CLASS_BIND_METHOD(ShaderProgram, getPsCode, DEF_METHOD("getPsCode"));
        CLASS_BIND_METHOD(ShaderProgram, setPsCode, DEF_METHOD("setPsCode"));
        CLASS_BIND_METHOD(ShaderProgram, getGraph, DEF_METHOD("getGraph"));
        CLASS_BIND_METHOD(ShaderProgram, setGraph, DEF_METHOD("setGraph"));
        CLASS_BIND_METHOD(ShaderProgram, getCullMode, DEF_METHOD("getCullMode"));
		CLASS_BIND_METHOD(ShaderProgram, setCullMode, DEF_METHOD("setCullMode"));
		CLASS_BIND_METHOD(ShaderProgram, getBlendMode, DEF_METHOD("getBlendMode"));
		CLASS_BIND_METHOD(ShaderProgram, setBlendMode, DEF_METHOD("setBlendMode"));

        CLASS_REGISTER_PROPERTY(ShaderProgram, "Type", Variant::Type::String, "getType", "setType");
        CLASS_REGISTER_PROPERTY(ShaderProgram, "VertexShader", Variant::Type::String, "getVsCode", "setVsCode");
        CLASS_REGISTER_PROPERTY(ShaderProgram, "FragmentShader", Variant::Type::String, "getPsCode", "setPsCode");
        CLASS_REGISTER_PROPERTY(ShaderProgram, "Graph", Variant::Type::String, "getGraph", "setGraph");
        CLASS_REGISTER_PROPERTY(ShaderProgram, "CullMode", Variant::Type::StringOption, "getCullMode", "setCullMode");
        CLASS_REGISTER_PROPERTY(ShaderProgram, "BlendMode", Variant::Type::StringOption, "getBlendMode", "setBlendMode");

        CLASS_REGISTER_PROPERTY_HINT(ShaderProgram, "CullMode", PropertyHintType::Category, "RasterizerState");
        CLASS_REGISTER_PROPERTY_HINT(ShaderProgram, "BlendMode", PropertyHintType::Category, "BlendState");
	}

	void ShaderProgram::clear()
	{
		EchoSafeDelete(m_blendState, BlendState);
		EchoSafeDelete(m_depthState, DepthStencilState);
		EchoSafeDelete(m_rasterizerState, RasterizerState);
	}
    
    Res* ShaderProgram::create()
    {
        return Renderer::instance()->createShaderProgram();
    }

    void ShaderProgram::setVsCode(const String& vsCode)
    {
        m_vsCode=vsCode;
    }

    void ShaderProgram::setPsCode(const String& psCode)
    {
        m_psCode=psCode;
        build();
    }

	bool ShaderProgram::isGlobalUniform(const String& name)
	{
		return StringUtil::StartWith(name, "u_") ? true : false;
	}

    void ShaderProgram::insertMacros(String& code)
    {
        // make sure macros
        String finalMacros; finalMacros.reserve(512);
        for (const String& macro : m_macros)
            finalMacros += "#define " + macro + "\n";
        
        if (!finalMacros.empty())
        {
            finalMacros = "\r\n" + finalMacros + "\r\n";

            size_t pos = code.find_first_of('\n') + 1;
            code.insert(pos, finalMacros);
        }
    }

	bool ShaderProgram::build()
	{
        if(!m_vsCode.empty() && !m_psCode.empty())
        {
            String vsSrc = m_vsCode;
            String psSrc = m_psCode;
            
            insertMacros(vsSrc);
            insertMacros(psSrc);
            
            // convert based on renderer type
            convert(m_type, vsSrc, psSrc);

            if (createShaderProgram(vsSrc, psSrc))
            {
				clearPropertys();

				PropertyHintArray hints;
				hints.push_back({ PropertyHintType::Category, "Uniforms" });

				for (auto& it : m_uniforms)
				{
					if (!isGlobalUniform(it.first))
					{
						switch (it.second->m_type)
						{
						case ShaderParamType::SPT_INT: registerProperty(ECHO_CLASS_NAME(ShaderProgram), "Uniforms." + it.first, Variant::Type::Int, hints); break;
						case ShaderParamType::SPT_FLOAT:registerProperty(ECHO_CLASS_NAME(ShaderProgram), "Uniforms." + it.first, Variant::Type::Real, hints); break;
						case ShaderParamType::SPT_VEC3: registerProperty(ECHO_CLASS_NAME(ShaderProgram), "Uniforms." + it.first, Variant::Type::Vector3, hints); break;
						case ShaderParamType::SPT_VEC4: registerProperty(ECHO_CLASS_NAME(ShaderProgram), "Uniforms." + it.first, Variant::Type::Color, hints); break;
						case ShaderParamType::SPT_TEXTURE: registerProperty(ECHO_CLASS_NAME(ShaderProgram), "Uniforms." + it.first, Variant::Type::ResourcePath, hints); break;
						default: break;
						}
					}
				}

				onShaderChanged();
            }
            else
            {
                EchoLogError("create shader program failed");
                return false;
            }
        }
        
        return true;
	}

	bool ShaderProgram::getPropertyValue(const String& propertyName, Variant& oVar)
	{
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			Uniform* uniform = getUniform(ops[1]);
            if (uniform)
            {
                bool isValueEmpty = uniform->m_type != ShaderParamType::SPT_TEXTURE && uniform->getValueDefault().empty();
				if (!isValueEmpty)
				{
					switch (uniform->m_type)
					{
					case ShaderParamType::SPT_FLOAT:	oVar = *(float*)(uniform->getValueDefault().data()); break;
					case ShaderParamType::SPT_VEC2:		oVar = *(Vector2*)(uniform->getValueDefault().data()); break;
					case ShaderParamType::SPT_VEC3:		oVar = *(Vector3*)(uniform->getValueDefault().data()); break;
					case ShaderParamType::SPT_VEC4:		oVar = *(Color*)(uniform->getValueDefault().data()); break;
					case ShaderParamType::SPT_TEXTURE:  oVar = uniform->getTextureDefault(); break;
					default:							oVar = *(float*)(uniform->getValueDefault().data()); break;
					}

					return true;
				}
            }
		}

		return false;
	}

	bool ShaderProgram::setPropertyValue(const String& propertyName, const Variant& propertyValue)
	{
		StringArray ops = StringUtil::Split(propertyName, ".");
		if (ops[0] == "Uniforms")
		{
			Uniform* uniform = getUniform(ops[1]);
            if (uniform)
            {
				switch (uniform->m_type)
				{
				case ShaderParamType::SPT_FLOAT:	uniform->setValueDefault(&(propertyValue.toReal())); break;
				case ShaderParamType::SPT_VEC2:		uniform->setValueDefault(&(propertyValue.toVector3())); break;
				case ShaderParamType::SPT_VEC4:		uniform->setValueDefault(&(propertyValue.toColor())); break;
                case ShaderParamType::SPT_TEXTURE:  uniform->setTextureDefault(propertyValue.toResPath()); break;
				default:							uniform->setValueDefault(&(propertyValue.toReal())); break;
				}

                return true;
            }

		}

		return false;
	}

	bool ShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
	{
		EchoLogError("createShaderProgram function unimplement: ");
		return false;
	}

	StringArray ShaderProgram::getEditableMacros()
	{
		StringArray macros;
		return macros;
	}

    void ShaderProgram::setCullMode(const StringOption& option)
    { 
        if (m_cullMode.setValue(option.getValue()))
        {
            m_rasterizerState = nullptr;
        }
    }

	void ShaderProgram::setBlendMode(const StringOption& option)
	{
		if (m_blendMode.setValue(option.getValue()))
		{
			m_blendState = nullptr;
		}
	}

    BlendState* ShaderProgram::getBlendState()
    { 
        if (!m_blendState)
        {
            BlendState::BlendDesc desc;
            if (m_blendMode.getIdx() == 1)
            {
                desc.bBlendEnable = true;
				desc.srcBlend = BlendState::BF_SRC_ALPHA;
				desc.dstBlend = BlendState::BF_INV_SRC_ALPHA;
            }

            m_blendState = Renderer::instance()->createBlendState(desc);
        }

        return m_blendState; 
    }

    DepthStencilState* ShaderProgram::getDepthState()
    { 
        if (!m_depthState)
        {
            DepthStencilState::DepthStencilDesc desc;
            m_depthState = Renderer::instance()->createDepthStencilState(desc);
        }

        return m_depthState; 
    }

    RasterizerState* ShaderProgram::getRasterizerState()
    { 
        if (!m_rasterizerState)
        {
            RasterizerState::RasterizerDesc desc;
            desc.cullMode = magic_enum::enum_cast<RasterizerState::CullMode>(m_cullMode.getValue().c_str()).value_or(RasterizerState::CullMode::CULL_NONE);
            m_rasterizerState = Renderer::instance()->createRasterizerState(desc);
        }

        return m_rasterizerState; 
    }

    MultisampleState* ShaderProgram::getMultisampleState()
    {
        if (!m_multiSampleState)
        {
            m_multiSampleState = Renderer::instance()->createMultisampleState();
        }

        return m_multiSampleState;
    }
    
    void ShaderProgram::setUniform( const char* name, const void* value, ShaderParamType uniformType, ui32 count)
    {
		UniformMap::iterator it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			UniformPtr uniform = it->second;
			uniform->setValue(value);
		}
		else
		{
			EchoLogError("ShaderProgram uniform [%d] not exist!", name);
		}
    }

    ShaderProgram::UniformPtr ShaderProgram::getUniform(const String& name)
	{
        UniformMap::iterator it = m_uniforms.find(name);
        if (it != m_uniforms.end())
            return it->second;

		return nullptr;
	}
 
	int ShaderProgram::mapUniformTypeSize(ShaderParamType uniformType)
	{
		switch (uniformType)
		{
		case SPT_UNKNOWN:			return 0;
		case SPT_INT:				return 4;
		case SPT_FLOAT:				return 4;
		case SPT_VEC2:				return 8;
		case SPT_VEC3:				return 12;
		case SPT_VEC4:				return 16;
		case SPT_MAT4:				return 64;
		case SPT_TEXTURE:			return 4;
		default:					return 0;
		}
	}

	ResRef<ShaderProgram> ShaderProgram::getDefault(const String& shaderPath)
	{
		if (StringUtil::StartWith(shaderPath, "_echo_default_2d_shader_"))
		{
			return ShaderProgram::getDefault2D(StringArray());
		}
		else if (StringUtil::StartWith(shaderPath, "_echo_default_3d_shader_"))
		{
			return ShaderProgram::getDefault3D(StringArray());
		}

		return nullptr;
	}

    ResRef<ShaderProgram> ShaderProgram::getDefault2D(const StringArray& macros)
    {
        String shaderVirtualPath = "_echo_default_2d_shader_" + StringUtil::ToString(macros);
        ShaderProgramPtr shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
        if(!shader)
        {
            shader = ECHO_CREATE_RES(ShaderProgram);
            shader->setMacros(macros);
            
            // blend state
            shader->setBlendMode("Transparent");
            
            // depth state
            DepthStencilState::DepthStencilDesc depthDesc;
            depthDesc.bDepthEnable = false;
            depthDesc.bWriteDepth = false;
            DepthStencilState* depthState = Renderer::instance()->createDepthStencilState(depthDesc);
            shader->setDepthState(depthState);

            // reaster state
            shader->setCullMode("CULL_NONE");
            
            shader->setPath(shaderVirtualPath);
            shader->setType("glsl");
            shader->setVsCode(g_2dVsCode);
            shader->setPsCode(g_2dPsCode);
        }
        
        return shader;
    }

    ResRef<ShaderProgram> ShaderProgram::getDefault3D(const StringArray& macros)
    {
		String shaderVirtualPath = "_echo_default_3d_shader_" + StringUtil::ToString(macros);
		ShaderProgramPtr shader = ECHO_DOWN_CAST<ShaderProgram*>(ShaderProgram::get(shaderVirtualPath));
		if (!shader)
		{
			shader = ECHO_CREATE_RES(ShaderProgram);
			shader->setMacros(macros);

			// blend state
			shader->setBlendMode("Opaque");

			// depth state
			DepthStencilState::DepthStencilDesc depthDesc;
			depthDesc.bDepthEnable = true;
			depthDesc.bWriteDepth = true;
			DepthStencilState* depthState = Renderer::instance()->createDepthStencilState(depthDesc);
			shader->setDepthState(depthState);

			// reaster state
			shader->setCullMode("CULL_NONE");

			shader->setPath(shaderVirtualPath);
			shader->setType("glsl");
			shader->setVsCode(g_3dVsCode);
			shader->setPsCode(g_3dPsCode);
		}

		return shader;
    }
}
