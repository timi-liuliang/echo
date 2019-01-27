#pragma once

#include "engine/core/util/Buffer.h"
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	class ShaderProgram;
	class Shader
	{
	public:
		enum ShaderType
		{
			ST_VERTEXSHADER, 
			ST_PIXELSHADER, 
			ST_SHADERCOUNT, 
		};

		struct ShaderDesc
		{
			String		macros;
		};

	public:
		Shader(ShaderType type, const ShaderDesc& desc, const String& filename);
		Shader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size);
		virtual ~Shader();

		const String& getName() { return m_filename; }
		
		virtual void setShaderProgram(ShaderProgram* pProgram);

		virtual ShaderProgram*	getShaderProgram() const;
		virtual ShaderType getShaderType() const;
		static String GetShaderTypeDesc(ShaderType type);
		bool Validate() const;

	protected:
		ShaderProgram*		m_pProgram;
		ShaderType			m_shaderType;
		String				m_filename;
		String				m_srcData;
		ui32				m_shaderSize;
		bool				m_validata;
		ShaderDesc			m_desc;
	};
}
