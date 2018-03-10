#ifndef __ECHO_SHADER_H__
#define __ECHO_SHADER_H__

#include "engine/core/Util/Buffer.h"
#include "RenderDef.h"

namespace Echo
{
	class ShaderProgram;
	class ECHO_EXPORT_RENDER Shader
	{
	public:
		enum ShaderType
		{
			ST_VERTEXSHADER, 
			ST_PIXELSHADER, 
			ST_SHADERCOUNT, 
		};

		enum ShaderLang
		{
			SL_HLSL,
			SL_GLSL,
			SL_CG,
		};

		struct ShaderDesc
		{
			String		func;
			String		param;
			String		macros;		// 宏定义
		};

	public:
		Shader(ShaderType type, const ShaderDesc& desc, const String& filename);
		Shader(ShaderType type, const ShaderDesc& desc, const Buffer& buff);
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
		ShaderDesc			m_desc;					// 描述信息
	};
}

#endif