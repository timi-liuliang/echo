#include "Render/Shader.h"
#include "engine/core/Util/LogManager.h"
#include <engine/core/resource/ResourceGroupManager.h>
#include "engine/core/Util/PathUtil.h"
#include "Render/Renderer.h"
#include "Render/ShaderProgram.h"

namespace Echo
{
	// 构造函数
	Shader::Shader(ShaderType type, const ShaderDesc &desc, const String &filename)
		: m_pProgram(NULL)
		, m_filename(filename)
	{
		DataStream* pShaderStream = ResourceGroupManager::instance()->openResource(filename);
		if(!pShaderStream)
		{
			EchoException("Shader file isn't exist. [%s]", filename.c_str());
		}

		m_validata = true;
		m_shaderType = type;
		m_desc = desc;
		m_shaderSize =  pShaderStream->size()+1;
		m_srcData.resize( m_shaderSize);
		m_srcData[m_shaderSize - 1] = '\0';

		pShaderStream->read( &m_srcData[0], m_shaderSize);

		EchoSafeDelete(pShaderStream, DataStream);
	}

	// 构造函数
	Shader::Shader(ShaderType type, const ShaderDesc& desc, const Buffer& buff)
		: m_pProgram(NULL)
	{
		m_validata	 = true;
		m_shaderType = type;
		m_desc		 = desc;
		m_shaderSize = buff.getSize();
		
		m_srcData.resize( m_shaderSize);
		memcpy( &m_srcData[0], buff.getData(), buff.getSize());

	}

	// 析构函数
	Shader::~Shader()
	{
	}

	// 设置着色器
	void Shader::setShaderProgram(ShaderProgram* pProgram)
	{
		m_pProgram = pProgram;
	}

	ShaderProgram* Shader::getShaderProgram() const
	{
		return m_pProgram;
	}

	Shader::ShaderType Shader::getShaderType() const
	{
		return m_shaderType;
	}

	String Shader::GetShaderTypeDesc(ShaderType type)
	{
		switch(type)
		{
		case ST_VERTEXSHADER:	return "ST_VERTEXSHADER";
		case ST_PIXELSHADER:	return "ST_PIXELSHADER";
		default:				return "UNKNOWN";
		}
	}

	bool Shader::Validate() const
	{
		return m_validata;
	}

}
