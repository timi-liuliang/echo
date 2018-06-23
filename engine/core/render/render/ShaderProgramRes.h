#pragma once

#include "RenderState.h"
#include "ShaderProgram.h"
#include "RenderInput.h"
#include <utility>
#include "engine/core/resource/Res.h"

namespace Echo
{
	/**
	 * 材质
	 */
	class Renderer;
	class ShaderProgramRes : public Res
	{
		ECHO_RES(ShaderProgramRes, Res, ".shader", Res::create<ShaderProgramRes>, ShaderProgramRes::load);

	public:
		struct DefaultUniform
		{
			i32 count;
			ShaderParamType type;
			ui32 sizeInByte;
			void* value;

			~DefaultUniform();
		};
		typedef map<String, DefaultUniform*>::type MapDefaultUniforms;

	public:
		ShaderProgramRes();
		ShaderProgramRes(const ResourcePath& path);
		~ShaderProgramRes();

		// 释放所有状态
		void free();

		// load and parse by file
		bool loadFromFile(const String& filename, const String& macros);
		bool loadFromContent(const char* content, const String& macros);

		// 获取混合状态
		BlendState* getBlendState() const { return m_blendState; }

		// 获取深度模板状态
		DepthStencilState* getDepthState() const { return m_depthState; }

		// 获取光栅化状态
		RasterizerState* getRasterizerState() const { return m_rasterizerState; }

		// 获取着色器
		ShaderProgram* getShaderProgram() const { return m_shaderProgram; }

		// 绑定shader
		void activeShader();

		// is have macro
		bool hasMacro(const char* const macro) const;

		// 获取材质可选宏定义列表
		static StringArray getEnabledMacros(const String& matFileName, bool withEnabled = false);

		bool					loadDefaultUniform(void* pNode);
		const DefaultUniform*	getDefaultUniformValue(const String& name);
		void*					createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType);

		// save
		virtual void save();

	protected:
		// load
		static Res* ShaderProgramRes::load(const ResourcePath& path);

	private:
		// 从内容加载
		bool loadFromContent(char* content, const String& macros);
		bool loadShaderFrom(void* node, const String& macros);
		bool loadBlendState(void* pNode);
		bool loadMacro(void * pNode);
		bool loadRasterizerState(void* pNode);
		bool loadDepthStencilState(void* pNode);
		void createBlendState(BlendState::BlendDesc& desc);
		void createDepthState(DepthStencilState::DepthStencilDesc& desc);
		void createRasterizerState(RasterizerState::RasterizerDesc& desc);
		bool createShaderProgram(const String& vsContent, const String& psContent);

	private:
		Shader::ShaderDesc	m_shaderDesc;							// 材质使用的宏定义
		BlendState*			m_blendState;
		DepthStencilState*	m_depthState;
		RasterizerState*	m_rasterizerState;
		ShaderProgram*		m_shaderProgram;
		MapDefaultUniforms	m_defaultUniforms;
	};
	typedef ResRef<ShaderProgramRes> ShaderProgramResPtr;
}
