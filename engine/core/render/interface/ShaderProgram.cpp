#include "ShaderProgram.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/log/Log.h"
#include "engine/core/util/Exception.h"
#include "Renderer.h"
#include "image/PixelFormat.h"
#include "engine/core/io/IO.h"
#include "MaterialDesc.h"
#include <thirdparty/pugixml/pugixml.hpp>

namespace Echo
{
	// set value
	void ShaderProgram::Uniform::setValue(const void* value)
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
		else if (memcmp(m_value, value, m_sizeInBytes) != 0)
		{
			m_isDirty = true;
			memcpy(m_value, value, m_sizeInBytes);
		}
	}

	ShaderProgram::ShaderProgram()
	{
		m_shaders.assign(nullptr);
	}

	ShaderProgram::ShaderProgram(const ResourcePath& path)
		: Res(path)
	{
		m_shaders.assign(nullptr);
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

		for (MapDefaultUniforms::iterator iter = m_defaultUniforms.begin(); iter != m_defaultUniforms.end(); ++iter)
		{
			EchoSafeDelete(iter->second, DefaultUniform);
		}
		m_defaultUniforms.clear();
	}

	bool ShaderProgram::loadFromFile(const String& filename, const String& macros)
	{
		try 
		{
			clear();

			setPath( filename);
			m_shaderDesc.macros = macros;

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

		MemoryReader memReader(content, strlen(content));
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
    
    // create
    Res* ShaderProgram::create()
    {
        return Renderer::instance()->createShaderProgram();
    }

	// load
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

	static int MappingStringArrayIdx(const String* array, int count, const String& value)
	{
		for (int i = 0; i < count; i++)
		{
			if (value == array[i])
				return i;
		}

		EchoLogError("Mapping string array idx failed [%s]", value.c_str());

		return 0;
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
					blendDesc.srcBlend = (BlendState::BlendFactor)(MappingStringArrayIdx(s_BlendFactor, BlendState::BF_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "DstBlend")
					blendDesc.dstBlend = (BlendState::BlendFactor)(MappingStringArrayIdx(s_BlendFactor, BlendState::BF_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "BlendOP")
					blendDesc.blendOP = (BlendState::BlendOperation)(MappingStringArrayIdx(s_BlendOperation, 6, elementNode.attribute("value").as_string("")));
				else if (strName == "SrcAlphaBlend")
					blendDesc.srcAlphaBlend = (BlendState::BlendFactor)(MappingStringArrayIdx(s_BlendFactor, 6, elementNode.attribute("value").as_string("")));
				else if (strName == "DstAlphaBlend")
					blendDesc.dstAlphaBlend = (BlendState::BlendFactor)(MappingStringArrayIdx(s_BlendFactor, 6, elementNode.attribute("value").as_string("")));
				else if (strName == "AlphaBlendOP")
					blendDesc.alphaBlendOP = (BlendState::BlendOperation)(MappingStringArrayIdx(s_BlendOperation, 6, elementNode.attribute("value").as_string("")));
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
					rasterizerState.polygonMode = (RasterizerState::PolygonMode)(MappingStringArrayIdx(s_PolygonMode, 3, elementNode.attribute("value").as_string("")));
				else if (strName == "ShadeModel")
					rasterizerState.shadeModel = (RasterizerState::ShadeModel)(MappingStringArrayIdx(s_ShadeModel, 2, elementNode.attribute("value").as_string("")));
				else if (strName == "CullMode")
					rasterizerState.cullMode = (RasterizerState::CullMode)(MappingStringArrayIdx(s_CullMode, 3, elementNode.attribute("value").as_string()));
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
					depthStencilState.depthFunc = (RenderState::ComparisonFunc)(MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementNode.attribute("value").as_string("")));
				else if (strName == "FrontStencilEnable")
					depthStencilState.bFrontStencilEnable = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "FrontStencilFunc")
					depthStencilState.frontStencilFunc = (RenderState::ComparisonFunc)(MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementNode.attribute("value").as_string("")));
				else if (strName == "FrontStencilReadMask")
					depthStencilState.frontStencilReadMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "FrontStencilWriteMask")
					depthStencilState.frontStencilWriteMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "FrontStencilFailOP")
					depthStencilState.frontStencilFailOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "FrontStencilDepthFailOP")
					depthStencilState.frontStencilDepthFailOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "FrontStencilPassOP")
					depthStencilState.frontStencilPassOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "FrontStencilRef")
					depthStencilState.frontStencilRef = elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilEnable")
					depthStencilState.bBackStencilEnable = StringUtil::ParseBool(elementNode.attribute("value").as_string());
				else if (strName == "BackStencilFunc")
					depthStencilState.backStencilFunc = (RenderState::ComparisonFunc)(MappingStringArrayIdx(s_ComparisonFunc, RenderState::CF_MAXNUM, elementNode.attribute("value").as_string("")));
				else if (strName == "BackStencilReadMask")
					depthStencilState.backStencilReadMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilWriteMask")
					depthStencilState.backStencilWriteMask = (ui16)elementNode.attribute("value").as_uint();
				else if (strName == "BackStencilFailOP")
					depthStencilState.backStencilFailOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "BackStencilDepthFailOP")
					depthStencilState.backStencilDepthFailOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
				else if (strName == "BackStencilPassOP")
					depthStencilState.backStencilPassOP = (DepthStencilState::StencilOperation)(MappingStringArrayIdx(s_StencilOperation, DepthStencilState::SOP_MAX, elementNode.attribute("value").as_string("")));
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

	bool ShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
	{
		Shader::ShaderDesc vsDesc(m_shaderDesc);
		Renderer* pRenderer = Renderer::instance();
		Shader *pVertexShader = pRenderer->createShader(Shader::ST_VERTEXSHADER, vsDesc, vsContent.data(), vsContent.size());
		if(!pVertexShader)
		{
			EchoLogError("Error in create vs file: ");
			return false;
		}

		Shader::ShaderDesc psDesc(m_shaderDesc);
		Shader *pPixelShader = pRenderer->createShader(Shader::ST_PIXELSHADER, psDesc, psContent.data(), psContent.size());
		if(!pPixelShader)
		{
			EchoLogError("Error in create ps file: ");
			return false;
		}

		// create shader program
        attachShader(pVertexShader);
		attachShader(pPixelShader);
		linkShaders();

		return true;
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

	ShaderProgram::DefaultUniform::~DefaultUniform()
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

				DefaultUniform* uniform = EchoNew(DefaultUniform);
				void* value = createDefaultUniformValue(strTypeValue, count, strValueValue, uniform->sizeInByte, uniform->type);
				if (value)
				{
					uniform->count = count;
					uniform->value = value;

					m_defaultUniforms[strNameValue] = uniform;
				}
				else
				{
					EchoSafeDelete(uniform, DefaultUniform);
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

	const ShaderProgram::DefaultUniform* ShaderProgram::getDefaultUniformValue(const String& name)
	{
		MapDefaultUniforms::iterator iter = m_defaultUniforms.find(name);
		if (iter != m_defaultUniforms.end())
		{
			return iter->second;
		}

		return NULL;
	}

	void* ShaderProgram::createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType)
	{
		// 解析字符串，数据分段
		StringArray valueStr = StringUtil::Split(strValue, ";");
		ui32 size = valueStr.size();
		EchoAssertX(size == count, "Material::createDefaultUniformValue");

		// 根据类型计算相关数据
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

		// 分配内存数据
		void* value = nullptr;
		if (outSize)
		{
			value = (void*)EchoMalloc(outSize);

			// 赋值
			for (ui32 i = 0; i < size; ++i)
			{
				switch (outType)
				{
				case Echo::SPT_INT:  ((i32*)value)[i] = StringUtil::ParseInt(valueStr[i]);		break;
				case Echo::SPT_FLOAT:((float*)value)[i] = StringUtil::ParseFloat(valueStr[i]);	break;
				case Echo::SPT_VEC2: ((Vector2*)value)[i] = StringUtil::ParseVec2(valueStr[i]);	break;
				case Echo::SPT_VEC3: ((Vector3*)value)[i] = StringUtil::ParseVec3(valueStr[i]);	break;
				case Echo::SPT_VEC4: ((Vector4*)value)[i] = StringUtil::ParseVec4(valueStr[i]); break;
				}
			}
		}

		return value;
	}

	bool ShaderProgram::hasMacro(const char* const macro) const
	{
		String fullMacro;
		fullMacro = "#define ";
		fullMacro += macro;
		fullMacro += "\n";
		int i = m_shaderDesc.macros.find(fullMacro.c_str());
		return i != String::npos;
	}

	void ShaderProgram::save()
	{

	}
    
    int ShaderProgram::getParamPhysicsIndex(const String& paramName)
    {
        for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            if (it->second.m_name == paramName)
                return it->second.m_location;
        }
        
        EchoLogError("%getParamPhysicsIndex failed");
        return 0;
    }
    
    void ShaderProgram::setUniform( const char* name, const void* value, ShaderParamType uniformType, ui32 count)
    {
        setUniform(getParamPhysicsIndex(name), value, uniformType, count);
    }
    
    void ShaderProgram::setUniform(ui32 physicIdx, const void* value, ShaderParamType uniformType, ui32 count)
    {
        EchoAssert(value);
        
        UniformArray::iterator it = m_uniforms.find(physicIdx);
        if (it != m_uniforms.end())
        {
            Uniform& uniform = it->second;
            uniform.setValue(value);
        }
        else
        {
            EchoLogError("ShaderProgram uniform [%d] not exist!", physicIdx);
        }
    }
    
    bool ShaderProgram::attachShader(Shader* pShader)
    {
        if(!pShader)
            return false;
        
        Shader::ShaderType type = pShader->getShaderType();
        if(m_shaders[(ui32)type])
        {
            EchoLogError("The shader [%s] has been already attached.", Shader::GetShaderTypeDesc(type).c_str());
            return false;
        }
        
        m_shaders[(ui32)type] = pShader;
        pShader->setShaderProgram(this);
        m_isLinked = false;
        
        return true;
    }
    
    Shader* ShaderProgram::detachShader(Shader::ShaderType type)
    {
        Shader* pShader = m_shaders[(ui32)type];
        m_shaders[(ui32)type] = NULL;
        
        m_isLinked = false;
        
        return pShader;
    }

	int ShaderProgram::getUniformByteSizeByUniformType(ShaderParamType uniformType)
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
