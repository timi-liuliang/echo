#pragma once

#include <engine/core/util/Array.hpp>
#include "ShaderProgram.h"
#include "Texture.h"
#include "RenderState.h"
#include "ShaderProgram.h"

namespace Echo
{
	static const int MAX_TEXTURE_SAMPLER = 16;
	static const int SHADER_TEXTURE_SLOT0= 0;
	static const int SHADER_TEXTURE_SLOT1= 1;
	static const int SHADER_TEXTURE_SLOT2= 2;
	static const int SHADER_TEXTURE_SLOT3= 3;
	static const int SHADER_TEXTURE_SLOT4= 4;
	static const int SHADER_TEXTURE_SLOT5= 5;
	static const int SHADER_TEXTURE_SLOT6= 6;
	static const int SHADER_TEXTURE_SLOT7= 7;
	static const int SHADER_TEXTURE_SLOT8 = 8;
	static const int SHADER_TEXTURE_SLOT9 = 9;
	static const int SHADER_TEXTURE_SLOT10 = 10;
	static const int SHADER_TEXTURE_SLOT11 = 11;
	static const int SHADER_TEXTURE_SLOT12 = 12;
	static const int SHADER_TEXTURE_SLOT13 = 13;
	static const int SHADER_TEXTURE_SLOT14 = 14;
	static const int SHADER_TEXTURE_SLOT15 = 15;

	class Render;
	class Mesh;
	class Material;
	class Renderable
	{
		friend class Renderer;
		typedef array<Texture*, MAX_TEXTURE_SAMPLER> MaxTextureArray;
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
		// identifier
		ui32 getIdentifier() const { return m_identifier; }

		// create method
		static Renderable* create(Mesh* mesh, Material* matInst, Render* node);

		// release
		void release();

		// set mesh
		Mesh* getMesh() { return m_mesh; }
		void setMesh(Mesh* mesh);

		// 开始参数
		void beginShaderParams(size_t paramNum);

		// 结束参数绑定
		void endShaderParams();

		// 设置参数
		void setShaderParam(size_t physicsIndex, ShaderParamType type, const void* param, size_t num=1);
		
		// 修改参数
		void modifyShaderParam(ui32 physics, ShaderParamType type, void* param, size_t num=1);

		// 设置纹理
		void setTexture( ui32 stage, Texture* texture);

		// 获取纹理
		Texture* getTexture(ui32 stage) const { return m_textures[stage]; }

		// 执行渲染
		virtual void render();
		
		// 设置渲染队列
		void setRenderStage(const String& renderStage) { m_renderStage = renderStage; }

		// 获取渲染队列
		const String& getRenderStage(void){ return m_renderStage; }

		// 提交到渲染队列
		void submitToRenderQueue();

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

		// set node
		void setNode( Render* node) { m_node = node; }

		// get node
		Render* getNode() { return m_node; }

		// get shader
		ShaderProgram* getShader();

	protected:
		// 向纹理槽中设置纹理
		void bindTextures();

		// 绑定参数
		void bindShaderParams();

		// 绑定渲染状态
		void bindRenderState();

		// link shader and program
		virtual void link() = 0;

	protected:
		Renderable( const String& renderStage, ShaderProgram* shader, int identifier);
		virtual ~Renderable();

	public:
		ui32									m_identifier;
		Render*									m_node;
		String									m_renderStage;
		ShaderProgramPtr						m_shaderProgram;		// material
		Mesh*									m_mesh;
		MaxTextureArray							m_textures;				// now only one texture sampler.
		vector<ShaderParam>::type				m_shaderParams;			// third shader params
		size_t									m_SParamWriteIndex;

		bool									m_bRenderState;
		BlendState*								m_pBlendState;
		RasterizerState*						m_pRasterizerState;
		DepthStencilState*						m_pDepthStencil;
	};
	typedef ui32 RenderableID;
}
