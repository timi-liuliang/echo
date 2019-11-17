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
        }
        else
        {
            if (Renderer::instance()->getType() == Renderer::Type::Vulkan)
            {
                vsSrc.clear();
                psSrc.clear();
            }
        }
        
        return true;
    }
    
	void ShaderProgram::Uniform::setValue(const void* value)
	{
		EchoAssert(value);

		if (!m_value)
		{
			m_value = (Byte*)ECHO_MALLOC(m_sizeInBytes);
		}

		if (m_isDirty)
		{
			memcpy(m_value, value, m_sizeInBytes);
		}
		else if (memcmp(m_value, value, m_sizeInBytes) != 0)
		{
			m_isDirty = true;
			memcpy(m_value, value, m_sizeInBytes);
		}
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

	// bind methods to script
	void ShaderProgram::bindMethods()
	{
	}

	void ShaderProgram::clear()
	{
		EchoSafeDelete(m_blendState, BlendState);
		EchoSafeDelete(m_depthState, DepthStencilState);
		EchoSafeDelete(m_rasterizerState, RasterizerState);

		for (UniformValuesMap::iterator iter = m_uniformDefaultValues.begin(); iter != m_uniformDefaultValues.end(); ++iter)
		{
			EchoSafeDelete(iter->second, UniformValue);
		}
		m_uniformDefaultValues.clear();
	}

	bool ShaderProgram::loadFromFile(const String& filename, const String& macros)
	{
		try 
		{
			clear();

			setPath( filename);
			m_macros = macros;

			MemoryReader memReader(filename.c_str());
			if (!memReader.getData<char*>())
			{
				return false;
			}

			return loadFromContent(memReader.getData<char*>(), macros);
		}
		catch(bool)
		{
			clear();
			EchoLogError("Material::loadFromFile: Fail to Parse Material file [%s].", filename.c_str());
			return false;
		}
	}

	bool ShaderProgram::loadFromContent(const String& fileName, const char* content, const String& macros)
	{
		setPath(fileName);

		MemoryReader memReader(content, ui32(strlen(content)));
		return loadFromContent(memReader.getData<char*>(), macros);
	}

	bool ShaderProgram::loadFromContent(char* content, const String& macros)
	{
		pugi::xml_document doc;
		doc.load(content);

		pugi::xml_node rootNode = doc.child("Shader");
		if (!rootNode)
		{
			EchoLogError("The Material file content is invalid.");
			return false;
		}

		return loadShaderFrom(&rootNode, macros);
	}
    
    Res* ShaderProgram::create()
    {
        return Renderer::instance()->createShaderProgram();
    }

	Res* ShaderProgram::load(const ResourcePath& path)
	{
		ShaderProgram* res = (ShaderProgram*)create();
		if (res)
		{
			res->loadFromFile(path.getPath(), "");
			return res;
		}

		return nullptr;
	}

	bool ShaderProgram::loadShaderFrom(void* node, const String& macros)
	{
		pugi::xml_node* rootNode = static_cast<pugi::xml_node*>(node);
		try
		{
            // type glsl
            String type = rootNode->attribute("type").as_string();
            
			pugi::xml_node vsNode = rootNode->child("VS");
			String vsSrc, psSrc;
			if (vsNode)
			{
				vsSrc = macros + vsNode.text().as_string();
			}

			pugi::xml_node psNode = rootNode->child("PS");
			if (psNode)
			{
				psSrc = macros + psNode.text().as_string();
			}

			for(pugi::xml_node elementNode = rootNode->first_child(); elementNode; elementNode=elementNode.next_sibling())
			{
				String strName = elementNode.name();
				if (strName == "BlendState")
				{
					if (!loadBlendState(&elementNode))
						throw false;
				}
				else if (strName == "RasterizerState")
				{
					if (!loadRasterizerState(&elementNode))
						throw false;
				}
				else if (strName == "DepthStencilState")
				{
					if (!loadDepthStencilState(&elementNode))
						throw false;
				}
				else if ( strName == "DefaultUniformValue" )
				{
					if ( !loadDefaultUniform( &elementNode ) )
						throw false;
				}
			}
            
            // convert based on renderer type
            convert(type, vsSrc, psSrc);

			if(!createShaderProgram( vsSrc, psSrc))
			{
				EchoLogError("create shader program failed");
				return false;
			}

			return true;
		}
		catch (bool)
		{
			clear();
			return false;
		}
	}

	static const String s_ColorMask[6] =
	{
		"CMASK_NONE"
		"CMASK_RED",
		"CMASK_GREEN",
		"CMASK_BLUE",
		"CMASK_ALPHA",
		"CMASK_COLOR",
		"CMASK_ALL",
	};

	INLINE RenderState::ColorMask MappingColorMask(const String& strValue)
	{
		RenderState::ColorMask results[] =
		{
			RenderState::CMASK_NONE,
			RenderState::CMASK_RED,
			RenderState::CMASK_GREEN,
			RenderState::CMASK_BLUE,
			RenderState::CMASK_ALPHA,
			RenderState::CMASK_COLOR,
			RenderState::CMASK_ALL
		};

		i32 result = 0;
		for (int i = 0; i < sizeof(results); i++)
		{
			if (StringUtil::Contain(strValue, s_ColorMask[i]))
				if (strValue == s_ColorMask[i])
					result |= results[i];
		}

		return RenderState::ColorMask(result);
	}

	bool ShaderProgram::loadBlendState( void* pNode )
	{
		BlendState::BlendDesc blendDesc;
		pugi::xml_node* pSubNode = static_cast<pugi::xml_node*>(pNode);
		try
		{
			for( pugi::xml_node elementNode = pSubNode->first_child(); elementNode; elementNode=elementNode.next_sibling())
			{
				String strName = elementNode.name();
				if(strName == "BlendEnable")
					blendDesc.bBlendEnable = elementNode.attribute("value").as_bool();
				else if (strName == "SrcBlend")
					blendDesc.srcBlend = magic_enum::enum_cast<BlendState::BlendFactor>(elementNode.attribute("value").as_string("")).value_or(blendDesc.srcBlend);
				else if (strName == "DstBlend")
					blendDesc.dstBlend = magic_enum::enum_cast<BlendState::BlendFactor>(elementNode.attribute("value").as_string("")).value_or(blendDesc.dstBlend);
				else if (strName == "BlendOP")
					blendDesc.blendOP = magic_enum::enum_cast<BlendState::BlendOperation>( elementNode.attribute("value").as_string("")).value_or(blendDesc.blendOP);
				else if (strName == "SrcAlphaBlend")
					blendDesc.srcAlphaBlend = magic_enum::enum_cast<BlendState::BlendFactor>(elementNode.attribute("value").as_string("")).value_or(blendDesc.srcAlphaBlend);
				else if (strName == "DstAlphaBlend")
					blendDesc.dstAlphaBlend = magic_enum::enum_cast<BlendState::BlendFactor>(elementNode.attribute("value").as_string("")).value_or(blendDesc.dstAlphaBlend);
				else if (strName == "AlphaBlendOP")
					blendDesc.alphaBlendOP = magic_enum::enum_cast<BlendState::BlendOperation>(elementNode.attribute("value").as_string("")).value_or(blendDesc.alphaBlendOP);
				else if (strName == "ColorWriteMask")
					blendDesc.colorWriteMask = MappingColorMask(elementNode.attribute("value").as_string(""));
				else if (strName == "A2CEnable")
					blendDesc.bA2CEnable = elementNode.attribute("value").as_bool();
				else if (strName == "IndependBlendEnable")
					blendDesc.bIndependBlendEnable = elementNode.attribute("value").as_bool();
				else if (strName == "BlendFactor")
					blendDesc.blendFactor = StringUtil::ParseColor(elementNode.attribute("value").as_string());
			}

			createBlendState(blendDesc);

			return true;
		}
		catch(bool)
		{
			clear();
			return false;
		}
	}

	bool ShaderProgram::loadRasterizerState( void* pNode )
	{
		RasterizerState::RasterizerDesc rasterizerState;
		pugi::xml_node* pSubNode = static_cast<pugi::xml_node*>(pNode);
		try
		{
			for(pugi::xml_node elementNode=pSubNode->first_child(); elementNode; elementNode=elementNode.next_sibling())
			{
				String strName = elementNode.name();
				if (strName == "PolygonMode")
					rasterizerState.polygonMode = magic_enum::enum_cast<RasterizerState::PolygonMode>(elementNode.attribute("value").as_string("")).value_or(rasterizerState.polygonMode);
				else if (strName == "ShadeModel")
					rasterizerState.shadeModel = magic_enum::enum_cast<RasterizerState::ShadeModel>(elementNode.attribute("value").as_string("")).value_or(rasterizerState.shadeModel);
				else if (strName == "CullMode")
					rasterizerState.cullMode = magic_enum::enum_cast<RasterizerState::CullMode>( elementNode.attribute("value").as_string()).value_or(rasterizerState.cullMode);
				else if (strName == "FrontFaceCCW")
					rasterizerState.bFrontFaceCCW = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "DepthBias")
					rasterizerState.depthBias = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "DepthBiasFactor")
					rasterizerState.depthBiasFactor = elementNode.attribute("value").as_float();
				else if (strName == "DepthClip")
					rasterizerState.bDepthClip = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "Scissor")
					rasterizerState.bScissor = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "Multisample")
					rasterizerState.bMultisample = StringUtil::ParseBool(elementNode.attribute("value").as_string());
			}

			createRasterizerState(rasterizerState);
			return true;
		}
		catch (bool)
		{
			clear();
			return false;
		}
	}

	bool ShaderProgram::loadDepthStencilState( void* pNode )
	{
		DepthStencilState::DepthStencilDesc depthStencilState;
		pugi::xml_node* pSubNode = static_cast<pugi::xml_node*>(pNode);
		try
		{
			for (pugi::xml_node elementNode = pSubNode->first_child(); elementNode; elementNode = elementNode.next_sibling())
			{
				String strName = elementNode.name();
				if (strName == "DepthEnable")
					depthStencilState.bDepthEnable = StringUtil::ParseBool( elementNode.attribute("value").as_string());
				else if (strName == "WriteDepth")
					depthStencilState.bWriteDepth = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "DepthFunc")
					depthStencilState.depthFunc = magic_enum::enum_cast<RenderState::ComparisonFunc>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.depthFunc);
				else if (strName == "FrontStencilEnable")
					depthStencilState.bFrontStencilEnable = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "FrontStencilFunc")
					depthStencilState.frontStencilFunc = magic_enum::enum_cast<RenderState::ComparisonFunc>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.frontStencilFunc);
				else if (strName == "FrontStencilReadMask")
					depthStencilState.frontStencilReadMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "FrontStencilWriteMask")
					depthStencilState.frontStencilWriteMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "FrontStencilFailOP")
					depthStencilState.frontStencilFailOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.frontStencilFailOP);
				else if (strName == "FrontStencilDepthFailOP")
					depthStencilState.frontStencilDepthFailOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>( elementNode.attribute("value").as_string("")).value_or(depthStencilState.frontStencilDepthFailOP);
				else if (strName == "FrontStencilPassOP")
					depthStencilState.frontStencilPassOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>( elementNode.attribute("value").as_string("")).value_or(depthStencilState.frontStencilPassOP);
				else if (strName == "FrontStencilRef")
					depthStencilState.frontStencilRef = elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilEnable")
					depthStencilState.bBackStencilEnable = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "BackStencilFunc")
					depthStencilState.backStencilFunc = magic_enum::enum_cast<RenderState::ComparisonFunc>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.backStencilFunc);
				else if (strName == "BackStencilReadMask")
					depthStencilState.backStencilReadMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilWriteMask")
					depthStencilState.backStencilWriteMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilFailOP")
					depthStencilState.backStencilFailOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>( elementNode.attribute("value").as_string("")).value_or(depthStencilState.backStencilFailOP);
				else if (strName == "BackStencilDepthFailOP")
					depthStencilState.backStencilDepthFailOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.backStencilDepthFailOP);
				else if (strName == "BackStencilPassOP")
					depthStencilState.backStencilPassOP = magic_enum::enum_cast<DepthStencilState::StencilOperation>(elementNode.attribute("value").as_string("")).value_or(depthStencilState.backStencilPassOP);
				else if (strName == "BackStencilRef")
					depthStencilState.backStencilRef = elementNode.attribute("value").as_int();
			}

			createDepthState(depthStencilState);
			return true;
		}
		catch(bool)
		{
			clear();
			return false;
		}
	}

	void ShaderProgram::createBlendState(BlendState::BlendDesc& desc)
	{
		EchoSafeDelete(m_blendState, BlendState);
		m_blendState = Renderer::instance()->createBlendState(desc);
	}

	void ShaderProgram::createDepthState(DepthStencilState::DepthStencilDesc& desc)
	{
		EchoSafeDelete(m_depthState, DepthStencilState);
		m_depthState = Renderer::instance()->createDepthStencilState(desc);
	}

	void ShaderProgram::createRasterizerState(RasterizerState::RasterizerDesc& desc)
	{
		EchoSafeDelete(m_rasterizerState, RasterizerState);
		m_rasterizerState = Renderer::instance()->createRasterizerState(desc);
	}

    void ShaderProgram::createMultisampleState()
    {
        EchoSafeDelete(m_multiSampleState, MultisampleState);
        m_multiSampleState = Renderer::instance()->createMultisampleState();
    }

	bool ShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
	{
		EchoLogError("createShaderProgram function unimplement: ");
		return false;
	}

	StringArray ShaderProgram::getEditableMacros(const String& shaderFileName)
	{
		StringArray macros;
		if (!shaderFileName.empty())
		{
			MemoryReader memReader(shaderFileName.c_str());
			pugi::xml_document doc;
			doc.load(memReader.getData<char*>());
			pugi::xml_node shaderNode = doc.child("Shader");
			if (shaderNode)
			{
				pugi::xml_node macrosNode = shaderNode.child("Macros");
				if (macrosNode)
				{
					for (pugi::xml_node macroNode = macrosNode.child("Macro"); macroNode; macroNode = macroNode.next_sibling("Macro"))
					{
						macros.push_back(macroNode.attribute("name").as_string());
						macros.push_back(macroNode.attribute("default").as_string());
					}
				}
			}
		}

		return macros;
	}

	ShaderProgram::UniformValue::~UniformValue()
	{
		EchoSafeFree(value);
	}

	bool ShaderProgram::loadDefaultUniform(void* pNode)
	{
		pugi::xml_node* pDefaultUniforms = static_cast<pugi::xml_node*>(pNode);
		try
		{
			for(pugi::xml_node pUniform = pDefaultUniforms->first_child(); pUniform; pUniform = pUniform.next_sibling())
			{
				String strNameValue = pUniform.attribute("name").as_string();
				String strTypeValue = pUniform.attribute("type").as_string();
				i32 count = pUniform.attribute("count").as_int();
				String strValueValue = pUniform.attribute("value").as_string();

				UniformValue* uniform = EchoNew(UniformValue);
				void* value = createDefaultUniformValue(strTypeValue, count, strValueValue, uniform->sizeInByte, uniform->type);
				if (value)
				{
					uniform->count = count;
					uniform->value = value;

					m_uniformDefaultValues[strNameValue] = uniform;
				}
				else
				{
					EchoSafeDelete(uniform, UniformValue);
				}
			}

			return true;
		}
		catch (bool)
		{
			clear();
			return false;
		}
	}

	const ShaderProgram::UniformValue* ShaderProgram::getDefaultUniformValue(const String& name)
	{
		UniformValuesMap::iterator iter = m_uniformDefaultValues.find(name);
		if (iter != m_uniformDefaultValues.end())
		{
			return iter->second;
		}

		return NULL;
	}

	void* ShaderProgram::createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType)
	{
		// parse string
		StringArray valueStr = StringUtil::Split(strValue, ";");
		ui32 size = static_cast<ui32>(valueStr.size());
		EchoAssertX(size == count, "Material::createDefaultUniformValue");

		// calculate data
		if (strType == "SPT_INT")
		{
			outType = SPT_INT;
			outSize = sizeof(i32) * count;
		}
		else if (strType == "SPT_FLOAT")
		{
			outType = SPT_FLOAT;
			outSize = sizeof(Real) * count;
		}
		else if (strType == "SPT_VEC2")
		{
			outType = SPT_VEC2;
			outSize = sizeof(Vector2) * count;
		}
		else if (strType == "SPT_VEC3")
		{
			outType = SPT_VEC3;
			outSize = sizeof(Vector3) * count;
		}
		else if (strType == "SPT_VEC4")
		{
			outType = SPT_VEC4;
			outSize = sizeof(Vector4) * count;
		}
		else if (strType == "SPT_MAT4")
		{
			outType = SPT_MAT4;
			outSize = 0;// sizeof(Matrix4) * count;
		}
		else if (strType == "SPT_TEXTURE")
		{
			outType = SPT_TEXTURE;
			outSize = 0;
		}
		else
		{
			outType = SPT_UNKNOWN;
		}

		// allocate memory
		void* value = nullptr;
		if (outSize)
		{
			value = (void*)EchoMalloc(outSize);

			// set value
			for (ui32 i = 0; i < size; ++i)
			{
				switch (outType)
				{
				case Echo::SPT_INT:  ((i32*)value)[i] = StringUtil::ParseInt(valueStr[i]);		break;
				case Echo::SPT_FLOAT:((float*)value)[i] = StringUtil::ParseFloat(valueStr[i]);	break;
				case Echo::SPT_VEC2: ((Vector2*)value)[i] = StringUtil::ParseVec2(valueStr[i]);	break;
				case Echo::SPT_VEC3: ((Vector3*)value)[i] = StringUtil::ParseVec3(valueStr[i]);	break;
				case Echo::SPT_VEC4: ((Vector4*)value)[i] = StringUtil::ParseVec4(valueStr[i]); break;
                default:                                                                        break;
				}
			}
		}

		return value;
	}

    BlendState* ShaderProgram::getBlendState()
    { 
        if (!m_blendState)
        {
            BlendState::BlendDesc desc;
            createBlendState(desc);
        }

        return m_blendState; 
    }

    DepthStencilState* ShaderProgram::getDepthState()
    { 
        if (!m_depthState)
        {
            DepthStencilState::DepthStencilDesc desc;
            createDepthState(desc);
        }

        return m_depthState; 
    }

    RasterizerState* ShaderProgram::getRasterizerState()
    { 
        if (!m_rasterizerState)
        {
            RasterizerState::RasterizerDesc desc;
            createRasterizerState(desc);
        }

        return m_rasterizerState; 
    }

    MultisampleState* ShaderProgram::getMultisampleState()
    {
        if (!m_multiSampleState)
        {
            createMultisampleState();
        }

        return m_multiSampleState;
    }

	bool ShaderProgram::hasMacro(const char* const macro) const
	{
		String fullMacro;
		fullMacro = "#define ";
		fullMacro += macro;
		fullMacro += "\n";
		return m_macros.find(fullMacro.c_str()) != String::npos;
	}

	void ShaderProgram::save()
	{

	}
    
    void ShaderProgram::setUniform( const char* name, const void* value, ShaderParamType uniformType, ui32 count)
    {
		UniformArray::iterator it = m_uniforms.find(name);
		if (it != m_uniforms.end())
		{
			Uniform& uniform = it->second;
			uniform.setValue(value);
		}
		else
		{
			EchoLogError("ShaderProgram uniform [%d] not exist!", name);
		}
    }
 
	int ShaderProgram::MapUniformTypeSize(ShaderParamType uniformType)
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
}
