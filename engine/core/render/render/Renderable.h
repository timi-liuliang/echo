#pragma once

#include <engine/core/Util/Array.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderState.h"
#include "ShaderProgramRes.h"

namespace Echo
{
	static const size_t MAX_TEXTURE_SAMPLER = 8;
	static const int	SHADER_TEXTURE_SLOT0= 0;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT1= 1;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT2= 2;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT3= 3;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT4= 4;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT5= 5;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT6= 6;	// 默认使用纹理槽
	static const int	SHADER_TEXTURE_SLOT7= 7;	// 默认使用纹理槽

	/**
	 * 最小可渲染体封装
	 */
	class Node;
	class Mesh;
	class Material;
	class Renderable
	{
		friend class Renderer;
		typedef array<TextureSampler, MAX_TEXTURE_SAMPLER> MaxTextureArray;
	public:
		// 着色器参数
		struct ShaderParam
		{
			ui32			physicsIndex;
			ShaderParamType stype;
			const void*		pData;
			ui32			ParamsLength;  // shader constance register num.
		};

	public:
		// 获取唯一标识符
		ui32 getIdentifier() const { return m_identifier; }

		// 构建数据
		static Renderable* create(Mesh* mesh, Material* matInst, Node* node);

		// release
		void release();

		// 设置渲染几何数据
		void setRenderInput(GPUBuffer* vertexStream, const RenderInput::VertexElementList& vertElements,  GPUBuffer* indexStream, ui32 idxStride, RenderInput::TopologyType topologyType);

		// 开始参数
		void beginShaderParams(size_t paramNum);

		// 结束参数绑定
		void endShaderParams();

		// 设置参数
		void setShaderParam(size_t physicsIndex, ShaderParamType type, const void* param, size_t num=1);
		
		// 修改参数
		void modifyShaderParam(ui32 physics, ShaderParamType type, void* param, size_t num=1);

		// 设置纹理
		void setTexture( ui32 stage, Texture* texture, const SamplerState* state);
		void setTexture( ui32 stage, ui32 globalTexture, const SamplerState* state);

		// 获取纹理
		Texture* getTexture(ui32 stage) const { return m_textures[stage].m_texture; }

		// 执行渲染
		virtual void render();
		
		// 设置渲染队列
		void setRenderStage(const String& renderStage) { m_renderStage = renderStage; }

		// 获取渲染队列
		const String& getRenderStage(void){ return m_renderStage; }

		// 提交到渲染队列
		void submitToRenderQueue();

		void setVisible(bool* state) { m_visible = state; }
		bool isVisible() { return *m_visible; }

		// 是否有自己的渲染状态
		bool isHaveRenderState( ) { return m_bRenderState; }

		// 设置渲染状态
		void setBlendState( BlendState* state );

		// 设置光栅化状态
		void setRasterizerState( RasterizerState* state );

		// 设置深度模板状态
		void setDepthStencilState(DepthStencilState* state);

		// 获取渲染状态
		BlendState*	getBlendState() { return m_pBlendState; }

		// 获取光栅化状态
		RasterizerState* getRasterizerState() { return m_pRasterizerState; }

		// 获取深度模板状态
		DepthStencilState* getDepthStencilState() { return m_pDepthStencil; }

		// 设置拥有者信息,方便调试
		void setOwnerDesc(const String& ownerDesc) { m_ownerDesc = ownerDesc; }

	protected:
		// 向纹理槽中设置纹理
		void bindTextures();

		// 绑定参数
		void bindShaderParams();

		// 绑定渲染状态
		void bindRenderState();

	private:
		Renderable( const String& renderStage, ShaderProgramRes* material, int identifier);
		virtual ~Renderable();

	public:
		ui32									m_identifier;
		String									m_ownerDesc;			// 拥有者信息
		String									m_renderStage;
		ShaderProgramResPtr						m_shaderProgram;			// material
		RenderInput*							m_renderInput;			// first  VB, IB, etc.
		MaxTextureArray							m_textures;				// now only one texture sampler.
		vector<ShaderParam>::type				m_shaderParams;			// third shader params
		size_t									m_SParamWriteIndex;
		bool*									m_visible;

		bool									m_bRenderState;
		BlendState*								m_pBlendState;
		RasterizerState*						m_pRasterizerState;
		DepthStencilState*						m_pDepthStencil;
		Vector3									m_worldPos;
	};
	typedef ui32 RenderableID;
}