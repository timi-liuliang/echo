#include "GLES2RenderStd.h"
#include "GLES2ShaderProgramTasks.h"
#include "GLES2ShaderProgram.h"
#include "GLES2Shader.h"
#include "GLES2ShaderGPUProxy.h"
#include "GLES2ShaderProgramGPUProxy.h"

namespace LORD
{
	GLES2ShaderProgramTaskBase::GLES2ShaderProgramTaskBase(GLES2ShaderProgramGPUProxy* proxy)
		: m_proxy(proxy)
	{

	}

	GLES2ShaderProgramTaskDestroyProxy::GLES2ShaderProgramTaskDestroyProxy(GLES2ShaderProgramGPUProxy* proxy)
		: GLES2ShaderProgramTaskBase(proxy)
	{
	}

	void GLES2ShaderProgramTaskDestroyProxy::Execute()
	{
		m_proxy->deleteProgram();
		LordDelete(m_proxy);
	}

	GLES2ShaderProgramTaskCreateProgram::GLES2ShaderProgramTaskCreateProgram(GLES2ShaderProgramGPUProxy* proxy)
		: GLES2ShaderProgramTaskBase(proxy)
	{
	}

	void GLES2ShaderProgramTaskCreateProgram::Execute()
	{
		m_proxy->createProgram();
	}

	GLES2ShaderProgramTaskLinkShaders::GLES2ShaderProgramTaskLinkShaders(GLES2ShaderProgramGPUProxy* proxy)
		: GLES2ShaderProgramTaskBase(proxy)
	{
	}

	void GLES2ShaderProgramTaskLinkShaders::Execute()
	{
		m_proxy->linkShaders();
	}

	GLES2ShaderProgramTaskUniformReflection::GLES2ShaderProgramTaskUniformReflection(GLES2ShaderProgramGPUProxy* proxy, ShaderProgram::UniformArray* uniform_array)
		: GLES2ShaderProgramTaskBase(proxy)
		, m_uniform_array(uniform_array)
	{
	}

	void GLES2ShaderProgramTaskUniformReflection::Execute()
	{
		m_proxy->uniformsReflection(m_uniform_array);
	}

	GLES2ShaderProgramTaskBindUniforms::GLES2ShaderProgramTaskBindUniforms(GLES2ShaderProgramGPUProxy* proxy, const ShaderProgram::UniformArray& uniform_array, const String& material_name)
		: GLES2ShaderProgramTaskBase(proxy)
		, m_material_name(material_name)
	{
		for (ShaderProgram::UniformArray::const_iterator it = uniform_array.begin(); it != uniform_array.end(); it++)
		{
			ShaderProgram::Uniform& uniform = const_cast<ShaderProgram::Uniform&>(it->second);
			if (uniform.m_value)
			{
				if (uniform.m_isDirty && uniform.m_type != SPT_UNKNOWN)
				{
					UniformData uniformData = { uniform.m_type, uniform.m_location, uniform.m_count, uniform.m_origin_value };
					m_uniform_list.push_back(uniformData);

					// ÐÞ¸Ä×´Ì¬
					uniform.m_isDirty = false;
				}
			}
			else
			{
				LordLogError("Shader param [%s] is NULL, Material [%s]", uniform.m_name.c_str(), material_name.c_str());
			}
		}
	}

	void GLES2ShaderProgramTaskBindUniforms::Execute()
	{
		for (size_t i = 0; i < m_uniform_list.size(); ++i)
		{
			UniformData& uni = m_uniform_list[i];
			m_proxy->bindUniforms(uni.type, uni.location, uni.count, uni.value);
		}
	}

	GLES2ShaderProgramTaskAttachShader::GLES2ShaderProgramTaskAttachShader(GLES2ShaderProgramGPUProxy* proxy, GLES2ShaderGPUProxy* shader_proxy)
		: GLES2ShaderProgramTaskBase(proxy)
		, m_shader_proxy(shader_proxy)
	{
		LordAssert(m_shader_proxy != nullptr);
	}

	void GLES2ShaderProgramTaskAttachShader::Execute()
	{
		m_proxy->attachShader(m_shader_proxy->m_hShader);
	}

	GLES2ShaderProgramTaskDetachShader::GLES2ShaderProgramTaskDetachShader(GLES2ShaderProgramGPUProxy* proxy, GLES2ShaderGPUProxy* shader_proxy)
		: GLES2ShaderProgramTaskBase(proxy)
		, m_shader_proxy(shader_proxy)
	{
		LordAssert(m_shader_proxy != nullptr);
	}

	void GLES2ShaderProgramTaskDetachShader::Execute()
	{
		m_proxy->detachShader(m_shader_proxy->m_hShader);
	}

	GLES2ShaderProgramTaskUseProgram::GLES2ShaderProgramTaskUseProgram(GLES2ShaderProgramGPUProxy* proxy)
		: GLES2ShaderProgramTaskBase(proxy)
	{
	}

	void GLES2ShaderProgramTaskUseProgram::Execute()
	{
		m_proxy->useProgram();
	}

}