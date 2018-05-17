#pragma once

#include "RenderState.h"
#include "ShaderProgram.h"
#include "RenderInput.h"
#include "Renderable.h"
#include <utility>

namespace Echo
{
	/**
	 * 材质
	 */
	class Renderer;
	class Material : public PtrMonitor<Material>
	{
		typedef map<String, const SamplerState*>::type SamplerStateMap;
		typedef vector<std::pair<String, String> >::type SeparatedSamplerArray;
		typedef vector<String>::type TextureSamplerStateArray;
	public:
		Material();
		~Material();

		// 获取名称
		const String& getName() { return m_name; }

		// 释放所有状态
		void free();

		// load and parse by file
		bool loadFromFile(const String& filename, const String& macros);
		bool loadFromContent(const char* content, const String& macros);
		bool loadShaderFrom(void* node, const String& macros);
		bool loadBlendState(void* pNode);
		bool loadMacro(void * pNode);
		bool loadRasterizerState(void* pNode);
		bool loadDepthStencilState(void* pNode);
		
		//bool loadSamplerState(void* pNode, int stage);
		bool loadSamplerState_Ext( void* pNode );
		bool loadTexture_Ext( void* pNode );

		// create manual
		void createBlendState(BlendState::BlendDesc& desc);
		void createDepthState(DepthStencilState::DepthStencilDesc& desc);
		void createRasterizerState(RasterizerState::RasterizerDesc& desc);

		// 创建着色器
		bool createShaderProgram( const String& vsContent, const String& psContent);

		// 获取混合状态
		BlendState* getBlendState() const { return m_pBlendState; }

		// 获取深度模板状态
		DepthStencilState* getDepthState() const { return m_pDepthState; }

		// 获取光栅化状态
		RasterizerState* getRasterizerState() const { return m_pRasterizerState; }

		// 获取采样状态
		const SamplerState* getSamplerState(int stage) const;

		// 获取着色器
		ShaderProgram* getShaderProgram() const { return m_pShaderProgram; }

		// 绑定shader
		void activeShader();

		// 设置队列
		void setQueue(RenderQueue* queue) { m_queue = queue; }

		bool			hasMacro		(const char* const macro) const;

		// 获取材质可选宏定义列表
		static StringArray getEnabledMacros(const String& matFileName, bool withEnabled = false);


		struct DefaultUniform
		{
			i32 count;
			ShaderParamType type;
			ui32 sizeInByte;
			void* value;

			~DefaultUniform();
		};
		bool					loadDefaultUniform(void* pNode);
		const DefaultUniform*	getDefaultUniformValue(const String& name);
		void*					createDefaultUniformValue(const String& strType, const i32 count, const String& strValue, ui32& outSize, ShaderParamType& outType);

	private:
		// 获取采样状态
		const SamplerState* getSamplerStateByTexStage(int stage);

		// 从内容加载
		bool loadFromContent(char* content, const String& macros);

	private:
		String				m_name;
		Shader::ShaderDesc	m_shaderDesc;							// 材质使用的宏定义
		RenderQueue*		m_queue;								// 当前材质所处的渲染队列
		BlendState*			m_pBlendState;
		DepthStencilState*	m_pDepthState;
		RasterizerState*	m_pRasterizerState;
		const SamplerState*	m_pSamplerState[MAX_TEXTURE_SAMPLER];
		SamplerStateMap		m_mapSamplerState;
		TextureSamplerStateArray	m_arrTexSamplerState;
		ShaderProgram*		m_pShaderProgram;
		typedef map<String, DefaultUniform*>::type MapDefaultUniforms;
		MapDefaultUniforms	m_defaultUniforms;
	};
}
