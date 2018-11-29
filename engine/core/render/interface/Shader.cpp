#include "Shader.h"
#include "engine/core/log/Log.h"
#include <engine/core/io/IO.h>
#include "engine/core/Util/PathUtil.h"
#include "Renderer.h"
#include "ShaderProgram.h"

namespace Echo
{
	Shader::Shader(ShaderType type, const ShaderDesc &desc, const String &filename)
		: m_pProgram(NULL)
		, m_filename(filename)
	{
		DataStream* pShaderStream = IO::instance()->open(filename);
		if(!pShaderStream)
		{
			EchoLogError("Shader file isn't exist. [%s]", filename.c_str());
		}

		m_validata = true;
		m_shaderType = type;
		m_desc = desc;
		m_shaderSize =  static_cast<ui32>(pShaderStream->size()+1);
		m_srcData.resize( m_shaderSize);
		m_srcData[m_shaderSize - 1] = '\0';

		pShaderStream->read( &m_srcData[0], m_shaderSize);

		EchoSafeDelete(pShaderStream, DataStream);
	}

	Shader::Shader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size)
		: m_pProgram(NULL)
	{
		m_validata	 = true;
		m_shaderType = type;
		m_desc		 = desc;
		m_shaderSize = size;
		
		m_srcData.resize( m_shaderSize);
		memcpy( &m_srcData[0], srcBuffer, size);

	}

	Shader::~Shader()
	{
	}

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
