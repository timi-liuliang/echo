#include "PostProcessRenderStage.h"
#include "engine/core/render/PostImageEffect.h"
#include "engine/core/render/render/Renderer.h"
#include "rapidxml/rapidxml.hpp"
#include "Engine/core/Render/VRModeAUX.h"
#include "Engine/core/Render/RenderTargetManager.h"
#include "Engine/core/Render/MaterialInst.h"
#include "Engine/core/main/Root.h"

namespace Echo
{
	PostProcessRenderStage::PostProcessRenderStage()
		: RenderStage(RSI_PostProcess)
		, m_finalOutput(-1)
		, m_isForUI(false)
	{
		// 顶点格式
		m_verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_POSITION, Echo::PF_RGB32_FLOAT));
		m_verElementLists.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TEXCOORD0, Echo::PF_RG32_FLOAT));
		float vertices[] =
		{
			-1.f, -1.f, 0.0f, 0.f, 0.f,
			1.f, -1.f, 0.0f, 1.f, 0.f,
			1.f, 1.f, 0.0f, 1.f, 1.f,
			-1.f, 1.f, 0.0f, 0.f, 1.f
		};

		ui16 indices[] = { 0, 3, 2, 0, 2, 1 };
		Buffer bufferVertices(sizeof(vertices), vertices);
		Buffer bufferIndices(sizeof(indices), indices);
		m_screenAlignedQuadVB = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
		m_screenAlignedQuadIB = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, bufferIndices);
	}

	PostProcessRenderStage::~PostProcessRenderStage()
	{
		EchoSafeDeleteContainer(m_postEffects, PostImageEffect);
		EchoSafeDelete(m_screenAlignedQuadVB, GPUBuffer);
		EchoSafeDelete(m_screenAlignedQuadIB, GPUBuffer);

		// vr Mode
		if (EngineSettingsMgr::instance()->isInitVRMode())
		{
			EchoSafeDelete(m_EyeGlassVB[0], GPUBuffer);
			EchoSafeDelete(m_EyeGlassVB[1], GPUBuffer);
			EchoSafeDelete(m_EyeGlassIB[0], GPUBuffer);
			EchoSafeDelete(m_EyeGlassIB[1], GPUBuffer);
			EchoSafeDelete(m_material, Material);

			EchoSafeDelete(m_renderInput[0], RenderInput);
			EchoSafeDelete(m_renderInput[1], RenderInput);
			Renderer::instance()->destroyRenderables(&m_renderable[0], 1);
			Renderer::instance()->destroyRenderables(&m_renderable[1], 1);
		}
	}

	bool PostProcessRenderStage::initialize()
	{
		MemoryReader memReader("PostEffectQueue.xml");
		rapidxml::xml_document<> doc;
		doc.parse<0>(memReader.getData<char*>());
		rapidxml::xml_node<>* postEffectQueue = doc.first_node();
		if (!postEffectQueue)
		{
			EchoLogInfo("invalid postEffectQueue define file.");
			return false;
		}

		rapidxml::xml_node<>* postEffectNode = postEffectQueue->first_node();
		while (postEffectNode)
		{
			rapidxml::xml_attribute<>* name = postEffectNode->first_attribute();
			String postEffectName = name->value();
			rapidxml::xml_attribute<>* pipelineFile = name->next_attribute();
			String pipelineFileName = pipelineFile->value();
			rapidxml::xml_attribute<>* defaultStatus = pipelineFile->next_attribute();
			bool enable = defaultStatus ? StringUtil::ParseBool(defaultStatus->value()) : false;
			PostImageEffect* postImageEffect = PostImageEffectFactory::create(postEffectName);
			postImageEffect->setGPUBuffers(&m_verElementLists, m_screenAlignedQuadVB, m_screenAlignedQuadIB);
			postImageEffect->loadFromFile(pipelineFileName);
			m_postEffects.push_back(postImageEffect);
			postImageEffect->setEnable(enable);
			postEffectNode = postEffectNode->next_sibling();
		}
		m_postEffects.back()->setFinal(true);

		// vr Mode
		if (EngineSettingsMgr::instance()->isInitVRMode())
		{
			m_material = EchoNew(Material);
			m_material->loadFromFile("pp_vrGlass.xml", "");
			ShaderProgram* shader = m_material->getShaderProgram();
			m_renderable[0] = Renderer::instance()->createRenderable(NULL, m_material);
			m_renderable[1] = Renderer::instance()->createRenderable(NULL, m_material);

			// 创建镜片
			// 顶点格式
			m_verElementLists_vr.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_SCREEN_POS_NDC, Echo::PF_RG32_FLOAT));
			m_verElementLists_vr.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TAN_EYE_ANGLE_R, Echo::PF_RG32_FLOAT));
			m_verElementLists_vr.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TAN_EYE_ANGLE_G, Echo::PF_RG32_FLOAT));
			m_verElementLists_vr.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_TAN_EYE_ANGLE_B, Echo::PF_RG32_FLOAT));
			m_verElementLists_vr.push_back(Echo::RenderInput::VertexElement(Echo::RenderInput::VS_COLOR, Echo::PF_RGBA8_UNORM));

			ovrDistortionMesh meshData;
			for (int eyeNum = 0; eyeNum < 2; eyeNum++)
			{
				// Allocate & generate distortion mesh vertices.

				if (!CreateDistortionMeshInternal(eyeNum, RenderTargetManager::instance()->m_fovPort[eyeNum],
					&meshData))
				{
					assert(false);
					continue;
				}

				// Now parse the vertex data and create a render ready vertex buffer from it
				DistortionVertex *   pVBVerts = (DistortionVertex*)malloc(sizeof(DistortionVertex)*meshData.VertexCount);
				DistortionVertex *   pCurVBVert = pVBVerts;


				ovrDistortionVertex* pCurOvrVert = meshData.pVertexData;

				for (unsigned vertNum = 0; vertNum < meshData.VertexCount; vertNum++)
				{
					pCurVBVert->ScreenPosNDC.x = pCurOvrVert->ScreenPosNDC.x;
					pCurVBVert->ScreenPosNDC.y = pCurOvrVert->ScreenPosNDC.y;

					memcpy(&pCurVBVert->TanEyeAnglesR, &pCurOvrVert->TanEyeAnglesR, sizeof(pCurVBVert->TanEyeAnglesR));
					memcpy(&pCurVBVert->TanEyeAnglesG, &pCurOvrVert->TanEyeAnglesG, sizeof(pCurVBVert->TanEyeAnglesG));
					memcpy(&pCurVBVert->TanEyeAnglesB, &pCurOvrVert->TanEyeAnglesB, sizeof(pCurVBVert->TanEyeAnglesB));

					// [0,255]
					Color tempCol;
					tempCol.r = Echo::Math::Max(pCurOvrVert->VignetteFactor, 0.0f);

					tempCol.g = tempCol.r;
					tempCol.b = tempCol.r;
					tempCol.a = pCurOvrVert->TimeWarpFactor;
					pCurVBVert->Col = tempCol.getABGR();
					pCurOvrVert++;
					pCurVBVert++;
				}

				Buffer bufferVertices(meshData.VertexCount*sizeof(DistortionVertex), pVBVerts);
				Buffer bufferIndices(sizeof(short)* meshData.IndexCount, meshData.pIndexData);
				m_EyeGlassVB[eyeNum] = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, bufferVertices);
				m_EyeGlassIB[eyeNum] = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, bufferIndices);

				m_renderInput[eyeNum] = Renderer::instance()->createRenderInput(shader);
				m_renderInput[eyeNum]->bindVertexStream(m_verElementLists_vr, m_EyeGlassVB[eyeNum]);
				m_renderInput[eyeNum]->bindIndexStream(m_EyeGlassIB[eyeNum]);
				m_renderInput[eyeNum]->setTopologyType(RenderInput::TT_TRIANGLELIST);
				//m_renderable[eyeNum]->setRenderInput(m_renderInput[eyeNum]);

				free(pVBVerts);
				DestroyDistortionMesh(&meshData);
			}

			// Uniform
			m_eyeToSourceUVScale = Vector2(0.32f, -0.23f);
			m_eyeToSourceUVOffset = Vector2(0.32f, 0.5f);
			m_normalizeMat = Matrix4::IDENTITY;
			ShaderProgram::UniformArray* ptrUnifroms = shader->getUniforms();
			for (int i = 0; i < 2; i++)
			{
				m_renderable[i]->beginShaderParams(ptrUnifroms->size());
				ShaderProgram::UniformArray::iterator itUnifrom = ptrUnifroms->begin();
				for (; itUnifrom != ptrUnifroms->end(); ++itUnifrom)
				{
					const ShaderProgram::Uniform& uniform = itUnifrom->second;

					// 初始化三个变量
					if (uniform.m_name == "Texture0")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&SHADER_TEXTURE_SLOT0);
					}
					else if (uniform.m_name == "EyesOffset")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&m_eyesOffset);
					}
					else if (uniform.m_name == "EyeToSourceUVScale")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&m_eyeToSourceUVScale);
					}
					else if (uniform.m_name == "EyeToSourceUVOffset")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&m_eyeToSourceUVOffset);
					}
					else if (uniform.m_name == "EyeRotationStart")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&m_normalizeMat);
					}
					else if (uniform.m_name == "EyeRotationEnd")
					{
						m_renderable[i]->setShaderParam(shader->getParamPhysicsIndex(uniform.m_name),
							uniform.m_type, (void*)&m_normalizeMat);
					}


					if (uniform.m_type == SPT_TEXTURE)
					{
						// 0纹理单元
						int index = /**(int*)*/0;

						const TextureSampler* textureSample =
							&RenderTargetManager::instance()->getRenderTargetByID(RTI_LDRVRSceneColorMap)->getBindTextureSampler();

						Texture* texture = NULL;
						const SamplerState* sampleState = NULL;
						if (textureSample)
						{
							texture = textureSample->m_texture;
							sampleState = textureSample->m_samplerState;
						}

						m_renderable[i]->setTexture(index, texture, sampleState);
					}
				}
				m_renderable[i]->endShaderParams();
			}
		}

		return true;
	}

	void PostProcessRenderStage::render()
	{
		// 由于UI角色渲染依赖于后处理 但是如果UI角色窗口时全屏的时候不渲染场景导致UI角色渲染不对
		// 所以此处屏蔽掉不渲场景就不走后处理的逻辑
		if (!m_bEnable || !Root::instance()->isRenderScene())
		{
			m_finalOutput = -1;
			return;
		}
		
		renderImp();
	}

	void PostProcessRenderStage::destroy()
	{

	}

	bool PostProcessRenderStage::isEffectEnable(const String& name) const
	{
		auto it = std::find_if(
			m_postEffects.begin(),
			m_postEffects.end(),
			[&](PostImageEffect* effect) { return effect->name() == name; });
		return it != m_postEffects.end();
	}

	bool PostProcessRenderStage::isEnabled() const
	{
		return std::any_of(m_postEffects.begin(), m_postEffects.end(), [](PostImageEffect* effect) { return effect->getEnable(); });
	}

	void PostProcessRenderStage::setImageEffectEnable(const String& name, bool enable)
	{
		for (PostImageEffect* effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				effect->setEnable(enable);
				m_bEnable = enable || isEnabled();
				return;
			}
		}
		EchoLogWarning("Unexpected code path. %s effect is not in postEffectQueue.", name.c_str());
	}

	bool PostProcessRenderStage::getImageEffectEnable(const String& name) const
	{
		if (name == "ToneMapping")
		{
			return EngineSettingsMgr::instance()->isEnableToneMapping();
		}
		else if (name == "FocusBlur")
		{
			return EngineSettingsMgr::instance()->isEnableFocusBlur();
		}
		else if (name == "IsForUI")
		{
			return m_isForUI;
		}
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				return effect->getEnable();
			}
		}
		EchoLogWarning("Unexpected code path. %s effect is not in postEffectQueue.", name.c_str());
		return false;
	}

	void PostProcessRenderStage::setImageEffectSourceRT(const ui32 rtId)
	{
		for (auto& effect : m_postEffects)
		{
			effect->setSourceRT(rtId);
		}
	}

	void PostProcessRenderStage::setImageEffectParameter(const String& name, const String& uniform, const Vector4& value)
	{
		m_postEffects.back()->setForEffect(name, uniform, value);
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				effect->setParameter(uniform, value);
				return;
			}
		}
	}

	void PostProcessRenderStage::setLogicCalcParameter(const String& name, const String& uniform, const String& value)
	{
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				effect->setLogicCalcParameter(uniform, value);
				return;
			}
		}
		EchoLogWarning("Unexpected code path. %s effect is not in postEffectQueue.", name.c_str());
	}

	Vector4 PostProcessRenderStage::getImageEffectParameter(const String& name, const String& uniform)
	{
		Vector4 v;
		bool find = m_postEffects.back()->findForEffect(name, uniform, v);
		if (find) return v;
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				return effect->getParameter(uniform);
			}
		}
		return Vector4::ZERO;
	}

	String PostProcessRenderStage::getLogicCalcParameter(const String& name, const String& uniform)
	{
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				return effect->getLogicCalcParameter(uniform);
			}
		}
		EchoLogWarning("PostEffect[%s] is not contain uniform[%s]", name.c_str(), uniform.c_str());
		return "";
	}

	PostImageEffect* PostProcessRenderStage::getPostImageEffect(const String& name)
	{
		for (auto& effect : m_postEffects)
		{
			if (effect->name() == name)
			{
				return effect;
			}
		}
		EchoLogWarning("error PostImageEffect name");

		return NULL;
	}

	StringArray PostProcessRenderStage::getAllEffect() const
	{
		StringArray effects;
		effects.reserve(m_postEffects.size() + 1);
		for (auto& effect : m_postEffects)
		{
			effects.push_back(effect->name());
		}
		effects.push_back("ToneMapping");
		effects.push_back("FocusBlur");
		if (!Renderer::instance()->getDeviceFeatures().supportHFColorBf())
			effects.push_back("IsForUI");
		return effects;
	}

	void PostProcessRenderStage::renderImp()
	{
		if (!m_bEnable)
		{
			return;
		}

		for (const auto& effect : m_postEffects)
		{
			effect->render();
		}

		m_finalOutput = m_postEffects.back()->outputRenderTarget();
		if (EngineSettingsMgr::instance()->isUseVRMode())
		{
			RenderTargetManager::instance()->beginRenderTarget(RTI_DefaultBackBuffer, true, Color::BLACK, true);

			// modified uniforms
			m_eyeToSourceUVOffset = Vector2(0.68f, 0.5f);
			m_eyesOffset = 0.0;
			m_renderable[0]->render();

			m_eyeToSourceUVOffset = Vector2(0.31f, 0.5f);
			m_eyesOffset = 0.5;
			m_renderable[1]->render();
			RenderTargetManager::instance()->endRenderTarget(RTI_DefaultBackBuffer);
		}
	}

}  // namespace Echo
