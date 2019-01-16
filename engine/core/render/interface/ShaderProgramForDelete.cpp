/*#include <engine/core/log/Log.h>
#include "interface/ShaderProgram.h"
#include "interface/ShaderProgramRes.h"
#include "interface/Renderer.h"

namespace Echo
{
	ShaderProgram::ShaderProgram( ShaderProgramRes* material)
		: m_bLinked(false)
		, m_material( material)
	{
		for(ui32 i = 0; i < Shader::ST_SHADERCOUNT; ++i)
		{
			m_pShaders[i] = NULL;
		}
	}

	ShaderProgram::~ShaderProgram()
	{
		for(ui32 i = 0; i < Shader::ST_SHADERCOUNT; ++i)
		{
			EchoSafeDelete(m_pShaders[i], Shader);
		}
	}

	bool ShaderProgram::attachShader(Shader* pShader)
	{
		if(!pShader)
			return false;

		Shader::ShaderType type = pShader->getShaderType();
		if(m_pShaders[(ui32)type])
		{
			EchoLogError("The shader [%s] has been already attached.", Shader::GetShaderTypeDesc(type).c_str());
			return false;
		}

		m_pShaders[(ui32)type] = pShader;
		pShader->setShaderProgram(this);
		m_bLinked = false;

		return true;
	}

	Shader* ShaderProgram::detachShader(Shader::ShaderType type)
	{
		Shader* pShader = m_pShaders[(ui32)type];
		m_pShaders[(ui32)type] = NULL;

		m_bLinked = false;

		return pShader;
	}

	void ShaderProgram::setUniform( const char* name, const void* value, ShaderParamType uniformType, ui32 count)
	{
		setUniform(getParamPhysicsIndex(name), value, uniformType, count);
	}

	int ShaderProgram::getParamPhysicsIndex(const String& paramName)
	{
		for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			if (it->second.m_name == paramName)
				return it->second.m_location;
		}

		EchoLogError("%s is not param of shader program [%s] vs-[%s] ps-[%s]", paramName.c_str(), m_material->getName().c_str(), m_pShaders[Shader::ST_VERTEXSHADER]->getName().c_str(), m_pShaders[Shader::ST_PIXELSHADER]->getName().c_str());
		EchoAssert(false);

		return 0;
	}

	int ShaderProgram::getUniformByteSizeByUniformType( ShaderParamType uniformType)
	{
		switch( uniformType)
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

	ShaderProgram::Uniform* ShaderProgram::getUniform(const String& name)
	{
		for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			if (it->second.m_name == name)
				return &it->second;
		}

		return NULL;
	}

	bool ShaderProgram::checkValid()
	{
		for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			if (!it->second.m_value)
				return false;
		}

		return true;
	}

	i32 ShaderProgram::getAtrribLocation(VertexSemantic vertexSemantic)
	{
		return 0;
	}
}
*/
