#include "Engine/core/Scene/Node.h"
#include "Engine/core/Camera/Camera.h"
#include "Engine/core/main/Root.h"
#include "Engine/modules/Model/Model.h"
#include "Render/RenderInput.h"
#include "Render/Renderer.h"
#include "Render/Material.h"
#include "engine/core/Render/MaterialInst.h"
#include "Render/GPUBuffer.h"
#include "Render/RenderTarget.h"
#include "Engine/modules/Anim/AABBKeyFrame.h"
#include "Engine/modules/Anim/Skeleton.h"
#include "Engine/modules/Anim/AnimState.h"
#include "Engine/modules/Anim/AnimBlender.h"
#include "Engine/core/Render/RenderTargetManager.h"
#include "Engine/core/Render/RenderStage/ShadowMapRenderStage.h"
#include "Engine/core/Render/RenderStage/RenderStageManager.h"
#include "Engine/core/Render/TextureRes.h"
#include "engine/core/Util/LogManager.h"
#include "Render/PixelFormat.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_helper.hpp"

using namespace rapidxml;

namespace Echo
{
	static ui32 s_maxBoneCount = 38;
	static const ui32 SceneEnvSlot = 6;
	static const ui32 SMSlot  = 7;
	static const ui32 LMSlot1 = 5;
	static const ui32 LMSlot2 = 6;
	static const String s_DefaultLMName = "lmdefault.tga";

	static const ui32 RefectSLot = 3;
	static const ui32 RefractionSLot = 4;

	static const ui32 ShadowAlbedoSlot = 0;

	static const int MACROS_NUM = 2;
	static const String MACROS[MACROS_NUM] = {
		"RECEIVE_SHADOW",
		"DOUBLE_LMP"
	};
	
	// 渲染阶段析构函数
	Model::RenderPhase::~RenderPhase()
	{
		clean();
	}

	// 清理
	void Model::RenderPhase::clean()
	{
		// 删除材质实例
		if (m_materialInst)
		{
			EchoSafeDelete(m_materialInst, MaterialInst);
			m_materialInst = NULL;
		}

		// 删除几何体缓冲
		EchoSafeDelete(m_renderInput, RenderInput);

		// 删除渲染单元
		Renderer::instance()->destroyRenderables(&m_renderable, 1);
	}

	// 更新
	void Model::RenderPhase::update( ui32 delta)
	{
		if ( m_materialInst)
			m_materialInst->update(delta);
	}

	void Model::ReleaseMesh(Mesh* p)
	{
		p->release();
	}

	void Model::DeleteLightArray(LightArray* p)
	{
		EchoSafeDelete(p, LightArray); 
	}

	template<> HashNode<Model>** PtrMonitor<Model>::m_ptrs = NULL;
	template<> int PtrMonitor<Model>::m_hashcount = 0;
	template<> unsigned int PtrMonitor<Model>::m_maxhash = 10;

	// 构造函数
	Model::Model( const Model::Info& modelInfo )
		: PtrMonitor()
		, m_info(modelInfo)
		, m_mesh(nullptr)
		, m_pSkeleton(NULL)
		, m_pAnimBlender(NULL)
		, m_boneCount(0)
		, m_boneMatRows(NULL)
		, m_lightArray(nullptr, &DeleteLightArray)
		, m_IsVisible(true)
		, m_alpha(1.f)
		, m_currentTime( 0.f)
		, m_isEnable(false)
		, m_isNeedUpdateMatWater(false)
		, m_isNeedUpdateMatSky(false)
		, m_isInShowdownBox(true)
		, m_worldBox(Box::ZERO)
		, m_dymOffset(0)
	{
		for (int type = RP_Normal0; type < RP_Total; type++)
		{
			m_phases[type] = EchoNew(RenderPhase);
		}
	}

	// 析构函数
	Model::~Model()
	{
		unload();
		EchoSafeFree(m_boneMatRows);
		EchoSafeFreeVector(m_subMeshBoneMatRows);

		// 清空材质实例
		for (int type = RP_Normal0; type < RP_Total; type++)
		{
			m_phases[type]->clean();
			EchoSafeDelete(m_phases[type], RenderPhase);
		}
	}

	void Model::loadFromFile()
	{
		loadMaterialInstanceFromFile(); 

		loadMeshFromFile();

		loadLightArrayFromFile(); 
	}

	void Model::applyLoadedData()
	{
		applyLoadedMaterialInstanceData(); 

		applyLoadedMeshData(); 

		applyLoadedLightArrayData(); 
	}

	// 加载材质实例
	void Model::loadMaterialInstanceFromFile()
	{
		String invalidTag = String("null");
		for (int type = RP_Normal0; type <= RP_Normal3; type++)
		{
			const String& materialInstanceName = m_info.m_materialInsts[type];
			if (materialInstanceName.empty())
				continue;

			Echo::StringArray materialInstances = Echo::StringUtil::Split(materialInstanceName, ";");
			if (materialInstances.empty())
				continue;

			m_phases[type]->m_materialInst = nullptr;
			for (size_t i = 0; i < materialInstances.size(); i++)
			{
				if (Echo::StringUtil::Equal(materialInstances[i], invalidTag, false))
					continue;
					
				m_phases[type]->m_materialInst = createMaterialInst(materialInstances[i]);
			}
		}
	}

	// 创建材质实例
	MaterialInst* Model::createMaterialInst(const String& materialName)
	{
		String macros;
		if (m_info.isSkinModel())
			macros += "SKIN_MESH;";
		
		if (EchoEngineSettings.isActorCastShadow() && m_info.m_isReceiveShadow)
			macros += "RECEIVE_SHADOW;";

		if (m_info.m_isForUI)
			macros += "IS_FOR_UI;";

		MaterialInst* materialInst = MaterialInst::create(); // MaterialManager::instance()->createMaterialIst(materialName, macros);
		materialInst->prepareTexture();
		//materialInst->loadTexture();
		return materialInst;
	}

	// 加载模型
	void Model::loadMeshFromFile()
	{
		if (m_info.m_meshName.empty())
		{
			EchoLogInfo("[Model::loadMeshFromFile()]::meshName empty, so Model does not contain a Mesh Now."); 
			return; 
		} 
	}

	// 加载光阵列
	void Model::loadLightArrayFromFile()
	{
		m_lightArray.reset(EchoNew(LightArray));
		if (!m_info.m_lightArray.empty())
		{
			m_lightArray->load(m_info.m_lightArray.c_str());
		}
	}

	// 应用材质数据
	void Model::applyLoadedMaterialInstanceData(RenderPhase& phase)
	{
		if (!phase.m_materialInst)
			return;

		if (phase.m_materialInst->getStage() == "Transparent" || phase.m_materialInst->getStage() == "Opaque")
		{
			String first = phase.m_materialInst->getStage();
			String second = m_info.m_isActor ? "_Actor" : "";
			phase.m_materialInst->setStage(first + second);
		}

		phase.m_materialInst->applyLoadedData(); // --> must sync.
		phase.m_materialInst->loadTexture();     // --> must sync.
	}

	// apply材质信息
	void Model::applyLoadedMaterialInstanceData()
	{
		// apply m_materialInsts loaded data
		for (int type = RP_Normal0; type <= RP_Normal3; type++)
		{
			applyLoadedMaterialInstanceData(*m_phases[type]);
		}
	}

	// apply Mesh信息
	void Model::applyLoadedMeshData()
	{
		if (!m_mesh)
		{
			EchoLogInfo("No Mesh, applyData do nothing."); 
			return; 
		}

		//m_mesh->load();

		if (m_info.isSkinModel())
		{
			EchoSafeFreeVector(m_subMeshBoneMatRows);
			/*
			// 分配数据
			if (m_mesh->getUseSubMeshBoneMatrices())
			{
				m_subMeshBoneMatRows.resize(m_mesh->getSubMeshNum(), NULL);
				for (int i = 0; i < m_mesh->getSubMeshNum(); i++)
				{
					if (pSubMesh && pSubMesh->getBoneNum())
					{
						if (pSubMesh->getBoneNum() > s_maxBoneCount)
						{
							EchoLogError("Bone Count Beyond Max Bone Count.");
							EchoAssertX(false, "Bone Count beyond Max Bone Count.");
							return;
						}
						m_subMeshBoneMatRows[i] = EchoAlloc(Vector4, s_maxBoneCount * 3);

						// 赋初值
						const Matrix4& mat = Matrix4::IDENTITY;
						for (ui32 j = 0; j < s_maxBoneCount; j++)
						{
							m_subMeshBoneMatRows[i][j * 3].set(mat.m00, mat.m10, mat.m20, mat.m30);
							m_subMeshBoneMatRows[i][j * 3 + 1].set(mat.m01, mat.m11, mat.m21, mat.m31);
							m_subMeshBoneMatRows[i][j * 3 + 2].set(mat.m02, mat.m12, mat.m22, mat.m32);
						}
					}
				}
			}*/
		}
	}

	// apply光阵列
	void Model::applyLoadedLightArrayData()
	{
		// now empty
#ifdef ECHO_EDITOR_MODE
		m_pbrlights.clear();
		int size = m_lightArray->getLightNum();
		for (int i = 0; i < size; ++i)
		{
			Light* light = m_lightArray->getLight(i);
			if (light->getTypeEnum() == LT_PBRLight)
			{
				m_pbrlights.push_back((PBRLight*)light);
			}
		}

		if (m_mesh)
		{
			for (int j = 0; j < RP_ShadowMap; ++j)
			{
				RenderPhase& phase = *m_phases[j];
				phase.m_materialInst->setPBRLight(m_pbrlights);
			}
		}
#endif
	}

	// 设置材质实例
	void Model::setMaterialInstance(RenderPhaseType type, MaterialInst* mate)
	{
		m_phases[type]->m_materialInst = mate;
	}

	// 设置纹理
	void Model::setTexture(RenderPhaseType type, i32 index, const String& name)
	{
		MaterialInst* materialInst = m_phases[type]->m_materialInst;
		EchoAssert(materialInst);
		EchoAssert(m_mesh);

		TextureRes* pTexture = materialInst->setTexture(index, name);
		const SamplerState* defaultState = Renderer::instance()->getSamplerState(SamplerState::SamplerDesc());
		m_phases[type]->m_renderable->setTexture(index, pTexture->getTexture(), defaultState);
	}

	// 设置模型
	void Model::setMesh(Mesh* mesh)
	{
		//m_info.m_meshName = mesh->getName();
		//EchoSafeFreeVector(m_subMeshBoneMatRows);
		//i32 submeshNum = m_mesh->getSubMeshNum();
		//// 分配数据
		//if (m_mesh->getUseSubMeshBoneMatrices())
		//{
		//	m_subMeshBoneMatRows.resize(m_mesh->getSubMeshNum(), NULL);
		//	for (int i = 0; i < submeshNum; i++)
		//	{
		//		SubMesh* pSubMesh = m_mesh->getSubMesh(i);
		//		if (pSubMesh && pSubMesh->getBoneNum())
		//			m_subMeshBoneMatRows[i] = EchoAlloc(Vector4, /*pSubMesh->getBoneNum()*/s_maxBoneCount * 3);

		//		if (pSubMesh && static_cast<int>(m_lightmapUV1.size()) <= i)
		//		{
		//			m_lightmapUV1.push_back(Vector4(0.f, 0.f, 1.f, 1.f));
		//			m_lightmapUV2.push_back(Vector4(0.f, 0.f, 1.f, 1.f));
		//			m_lightmapScale1.push_back(Vector3::ONE);
		//			m_lightmapScale2.push_back(Vector3::ONE);
		//		}
		//	}
		//}
	}

	// 设置光阵列
	void Model::setLightArray(LightArray* lightArray)
	{
		m_lightArray.reset(lightArray); 
	}

	// 卸载
	void Model::unload()
	{
		// 清空材质实例
		for (int type = RP_Normal0; type < RP_Total; type++)
		{
			m_phases[type]->clean();
		}

		m_mesh->release(); 
		m_lightArray.reset(); 
		m_pbrlights.clear();
	}

	void Model::reload()
	{
		unload(); 

		loadFromFile(); 

		applyLoadedData(); 
	}

	// 更新
	void Model::update(bool isForUI,ui32 delta)
	{
		/*
		EchoAssert(m_pSceneNode);
		
		m_currentTime += 0.001f * delta;

		Camera* mainCamera = SceneManager::instance()->getMainCamera();

		// 更新包围盒
		if (m_mesh)
		{
			m_worldBox = m_mesh->getBox().transform(m_pSceneNode->getWorldMatrix());
		}

		// 更新世界观察投影矩阵
		if (isForUI)
		{
			m_matWV = m_pSceneNode->getWorldMatrix() * SceneManager::instance()->getGUICamera()->getViewMatrix();
			m_matWVP = m_pSceneNode->getWorldMatrix() * SceneManager::instance()->getGUICamera()->getViewProjMatrix();
		}
		else
		{
			m_matWV = m_pSceneNode->getWorldMatrix() * mainCamera->getViewMatrix();
			m_matWVP = m_pSceneNode->getWorldMatrix() * mainCamera->getViewProjMatrix();
		}

		// 更新阴影图世界观察投影矩阵
		if (EchoEngineSettings.isActorCastShadow() && (m_info.m_isCastShadow || m_info.m_isReceiveShadow))
		{
			CameraShadow pCameraShadow = SceneManager::instance()->getShadowCamera();
			if (m_isUseDynamicMatIst)
			{
				m_isInShowdownBox = pCameraShadow.isEnable() && pCameraShadow.getCalcBox().isIntersected(getWorldAABB());
				if (m_isInShowdownBox)
				{
					m_matWVPSM = m_pSceneNode->getWorldMatrix() * pCameraShadow.getViewProjMatrix();
					m_matWSM = pCameraShadow.getViewMatrix();
				}
			}
			else
			{
				m_matWVPSM = m_pSceneNode->getWorldMatrix() * SceneManager::instance()->getShadowCamera().getViewProjMatrix();
				m_matWSM = pCameraShadow.getViewMatrix();
			}
		}

		if (m_isNeedUpdateMatWater)
		{
			m_matWVPWater = m_pSceneNode->getWorldMatrix() * mainCamera->getMirrViewMatric() * mainCamera->getProjMatrix();
		}

		if (m_isNeedUpdateMatSky)
		{
			m_matWVPSky = m_pSceneNode->getWorldMatrix() * mainCamera->getSkyViewProj();
		}

		if (m_mesh && m_mesh->isValid() && m_mesh->isNeedUpdate())
		{
			m_mesh->update(delta, m_pSceneNode->getWorldPosition(), m_pSceneNode->getWorldOrientation());
		}

		// 更新光阵列
		if ( m_lightArray )
		{
			Vector4 pos = Vector4( m_pSceneNode->getWorldPosition( ), 0.f );
#ifdef ECHO_EDITOR_MODE
			if (m_lightArray->isDirty())
			{
				applyLoadedLightArrayData();
			}
#endif
			m_lightArray->update( );
			m_lightArray->updatePointLightPos( pos );
		}

		// 更新动画
		if(m_pAnimBlender && m_pAnimBlender->getAnimState())
		{
			// only to modify matrixs
			for(size_t i=0; i<m_boneCount; ++i)
			{
				const Matrix4& mat = m_pAnimBlender->getAnimMatrix(i);
				m_boneMatRows[i*3].set(mat.m00, mat.m10, mat.m20, mat.m30);
				m_boneMatRows[i*3+1].set(mat.m01, mat.m11, mat.m21, mat.m31);
				m_boneMatRows[i*3+2].set(mat.m02, mat.m12, mat.m22, mat.m32);
			}
		}

		if(m_info.isSkinModel())
		{
			refreshSubMeshBoneMatRows();
		}

		if (haveWaterMaterial() && m_last_water_quality != Root::instance()->getSettingsMgr().getWaterQuality())
		{
			if (Root::instance()->getSettingsMgr().getWaterQuality() < WQ_High)
			{
				modifyUniformValue("refect_degree", &m_close_refect_degree);
			}
			else
			{
				modifyUniformValue("refect_degree", &m_original_water_refect_degree);
			}
			m_last_water_quality = Root::instance()->getSettingsMgr().getWaterQuality();
		}

		// 材质实例更新
		for (int i = RP_Normal0; i < RP_Total; i++)
		{
			m_phases[i]->update(delta);
		}

		if (m_isUseXRay)
		{
			for (size_t i = 0; i < m_xrayRenderables.size(); i++)
			{
				m_xrayRenderables[i]->submitToRenderQueue();
			}
		}
		*/
	}

	// 创建渲染单元
	void Model::createRenderable(bool isForUI)
	{
		createRenderable(*m_phases[RP_Normal0], isForUI);
		createRenderable(*m_phases[RP_Normal1], isForUI);
		createRenderable(*m_phases[RP_Normal2], isForUI);
		createRenderable(*m_phases[RP_Normal3], isForUI);

		if (m_info.m_isCastShadow)
		{
			createRenderabeSM(isForUI);
		}
	}

	// 创建投射阴影渲染数据
	void Model::createRenderabeSM( bool isForUI)
	{
		// 添加材质
		if (m_mesh)
		{
			m_phases[RP_ShadowMap]->m_materialInst = nullptr;
			{
				bool isAlphaTest = false;
				if (m_phases[RP_Normal0]->m_materialInst != nullptr)
				{
					isAlphaTest = m_phases[RP_Normal0]->m_materialInst->isMacroUsed("ALPHA_TEST");
				}
				
				Echo::String MACROS = isAlphaTest ? "ALPHA_TEST;" : "";
				if (m_mesh->isSkin())
					MACROS += "SKIN_MESH;";

				MaterialInst* materialInst = nullptr;// MaterialManager::instance()->createMaterialIst(
				//	"official_shadowmap.material", MACROS.c_str());
				materialInst->applyLoadedData(); // --> must sync.
				materialInst->loadTexture();

				m_phases[RP_ShadowMap]->m_materialInst = materialInst;
			}

			// 组织渲染数据
			createRenderable(*m_phases[RP_ShadowMap], isForUI);
		}
	}

	// 创建渲染单元
	void Model::createRenderable(RenderPhase& phase, bool isForUI)
	{
		if (!m_mesh)
			return;

		/*
		EchoAssert(m_pSceneNode&&m_mesh&&(phase.m_renderInputs.empty()));

		int subNum = m_mesh->getSubMeshNum();
		m_submeshAlbedo.resize(subNum);

		for ( int i=0; i<subNum; i++)
		{
			// 拥有材质才可以执行渲染
			if (i < static_cast<int>(phase.m_materialInsts.size()))
			{

				RenderQueue*		pRenderQueue = nullptr;
				Renderable*			pRenderable = nullptr;
				MaterialInstance*	pMaterialInst = nullptr;
				Material*			pMaterial = nullptr;
				SubMesh*			pSubMesh = m_mesh->getSubMesh(i); EchoAssert(pSubMesh);
				if (phase.m_materialInsts[i])
				{
					if (phase.m_materialInsts[i]->getMaterialTemplate() == "water.mt"
						|| phase.m_materialInsts[i]->getMaterialTemplate() == "water1.mt"
						|| phase.m_materialInsts[i]->getMaterialTemplate() == "water2.mt")
					{
						m_have_water_material = true;

						RenderStage* rs = RenderStageManager::instance()->getRenderStageByID(RSI_WaterRefectMap);
						if (rs)
						{
							rs->setEnable(true);
						}
					}

					pMaterialInst = phase.m_materialInsts[i];
					pRenderQueue = phase.m_materialInsts[i]->getRenderQueue();
					pRenderable = Renderer::instance()->createRenderable(pRenderQueue, NULL);
					pRenderable->setOwnerDesc(Echo::StringUtil::Format("Model:[%s] MaterialInst:[%s]", m_info.m_name.c_str(), pMaterialInst->getName().c_str()));
					pMaterial = pRenderQueue->getMaterial();
					ShaderProgram* shaderProgram = pMaterial->getShaderProgram();
					assert(NULL != shaderProgram);
					ShaderProgram::UniformArray* ptrUnifroms = shaderProgram->getUniforms();

					// pRenderable->setVisible(pSubMesh->isVisible());
					pRenderable->beginShaderParams(ptrUnifroms->size());
					ShaderProgram::UniformArray::iterator itUnifrom = ptrUnifroms->begin();
					for (; itUnifrom != ptrUnifroms->end(); ++itUnifrom)
					{
						const ShaderProgram::Uniform& uniform = itUnifrom->second;

						// 全局变量直接设置，否则去材质实例里面查找
						void* value = getGlobalUniformValue(uniform.m_name, i, isForUI);
						if (!value)
						{
							value = phase.m_materialInsts[i]->getUniformValue(uniform.m_name);
							value = extraCalcUniform(uniform.m_name, value, i);
							if (!value)
							{
								EchoLogError("Model [%s] MaterialInstance [%s] Unifrom [%s] not find! ", m_info.m_name.c_str(), phase.m_materialInsts[i]->getName().c_str(), uniform.m_name.c_str());
								continue;
							}
							if (m_have_water_material && uniform.m_name == "refect_degree")
							{
								m_original_water_refect_degree = *((float*)(value));
							}
						}

						pRenderable->setShaderParam(shaderProgram->getParamPhysicsIndex(uniform.m_name), uniform.m_type, value, uniform.m_count);

						if (uniform.m_type == SPT_TEXTURE)
						{
							
							int index = *(int*)value;

							const TextureSampler* textureSample = getGlobalTextureSample(uniform.m_name);
							Texture* texture = NULL;
							const SamplerState* sampleState = NULL;
							if (textureSample)
							{
								texture = textureSample->m_texture;
								sampleState = textureSample->m_samplerState;
							}

							if (!texture && phase.m_materialInsts[i]->getTexture(index))
							{
								texture = phase.m_materialInsts[i]->getTexture(index)->getTexture();
								sampleState = pMaterial->getSamplerState(index);
							}

							if (uniform.m_name == "tAlbedo" || uniform.m_name == "Albedo")
							{
								m_submeshAlbedo[i].m_texture = texture;
								m_submeshAlbedo[i].m_samplerState = sampleState;
							}

							pRenderable->setTexture(index, texture, sampleState);
						}
					}
					pRenderable->endShaderParams();
				}
				else if (m_isUseDynamicMatIst)
				{
					size_t size = phase.m_materialInsts_dym[i].size();

					if (size)
					{
						if (phase.m_materialInsts_dym[i][size - 1]->getMaterialTemplate() == "water.mt"
							|| phase.m_materialInsts_dym[i][size - 1]->getMaterialTemplate() == "water1.mt"
							|| phase.m_materialInsts_dym[i][size - 1]->getMaterialTemplate() == "water2.mt")
						{
							m_have_water_material = true;

							RenderStage* rs = RenderStageManager::instance()->getRenderStageByID(RSI_WaterRefectMap);
							if (rs)
							{
								rs->setEnable(true);
							}
						}

						pMaterialInst = phase.m_materialInsts_dym[i][size - 1];
						pRenderQueue = phase.m_materialInsts_dym[i][size - 1]->getRenderQueue();
						pRenderable = Renderer::instance()->createRenderable(pRenderQueue, NULL);
						pRenderable->setOwnerDesc(Echo::StringUtil::Format("Model:[%s] MaterialInst:[%s]", m_info.m_name.c_str(), pMaterialInst->getName().c_str()));
						pMaterial = pRenderQueue->getMaterial();
						ShaderProgram* shaderProgram = pMaterial->getShaderProgram();
						assert(NULL != shaderProgram);
						ShaderProgram::UniformArray* ptrUnifroms = shaderProgram->getUniforms();

						pRenderable->resizeLowShaderParam(size);
						for (size_t j = 0; j < size; j++)
						{
							ShaderProgram* lowShaderProgram;

							lowShaderProgram = phase.m_materialInsts_dym[i][j]->getRenderQueue()->getMaterial()->getShaderProgram();
							pRenderable->setLowRenderQueue(phase.m_materialInsts_dym[i][j]->getRenderQueue(), j,
								lowShaderProgram->getUniforms()->size());

							//pRenderable->beginShaderParams(ptrUnifroms->size());
							ShaderProgram::UniformArray::iterator itUnifrom = ptrUnifroms->begin();
							for (; itUnifrom != ptrUnifroms->end(); ++itUnifrom)
							{
								const ShaderProgram::Uniform& uniform = itUnifrom->second;

								// 全局变量直接设置，否则去材质实例里面查找
								void* value = getGlobalUniformValue(uniform.m_name, i, isForUI);
								if (!value)
								{
									value = phase.m_materialInsts_dym[i][size-1]->getUniformValue(uniform.m_name);
									value = extraCalcUniform(uniform.m_name, value, i);
									if (!value)
									{
										EchoLogError("MaterialInstance %s Unifrom %s not find! ",
											phase.m_materialInsts_dym[i][j]->getName().c_str(), uniform.m_name.c_str());
										continue;
									}
									if (m_have_water_material && uniform.m_name == "refect_degree")
									{
										m_original_water_refect_degree = *((float*)(value));
									}
								}

								if (lowShaderProgram->getUniform(uniform.m_name))
								{
									pRenderable->setLowShaderParam(j, lowShaderProgram->getParamPhysicsIndex(uniform.m_name),
										uniform.m_type, value, uniform.m_count);
								}

								// 只有列表最后一个有纹理
								if (uniform.m_type == SPT_TEXTURE && j == size-1)
								{
									int index = *(int*)value;

									const TextureSampler* textureSample = getGlobalTextureSample(uniform.m_name);
									Texture* texture = NULL;
									const SamplerState* sampleState = NULL;
									if (textureSample)
									{
										texture = textureSample->m_texture;
										sampleState = textureSample->m_samplerState;
									}

									if (!texture)
									{
										texture = phase.m_materialInsts_dym[i][size-1]->getTexture(index)->getTexture();
										sampleState = pMaterial->getSamplerState(index);
									}

									pRenderable->setTexture(index, texture, sampleState);
								}
							}
							//pRenderable->endShaderParams();
						}
					}
					
				}

				if (pRenderable)
				{
					pRenderable->setVisible(pSubMesh->isVisible());
					RenderInput* pRenderInput = Renderer::instance()->createRenderInput(pMaterial->getShaderProgram());
					pRenderInput->enableWireFrame(true);
					pRenderInput->setOwnerInfo(StringUtil::Format("model [%s]'s subMesh [%s] ", m_info.m_name.c_str(), pSubMesh->getName().c_str()));	
					if (pSubMesh->isMultiVertexBuff())
					{
						pRenderInput->set_is_muti_stream(true);
						SubMesh::itrMutiVertexBuff itrVertexbuff = pSubMesh->getMultiVertexBuff().begin();
						for (; itrVertexbuff != pSubMesh->getMultiVertexBuff().end(); itrVertexbuff++)
						{
							if (pSubMesh->getMultiVertexBuff().size() == 1)
							{
								pRenderInput->bindVertexStream(itrVertexbuff->m_vertex_element_list, itrVertexbuff->m_gpubuff);
							}
							else
							{
								if (itrVertexbuff == pSubMesh->getMultiVertexBuff().begin())
								{
									pRenderInput->bindVertexStream(itrVertexbuff->m_vertex_element_list, itrVertexbuff->m_gpubuff, RenderInput::BS_BEGIN);
								}
								else
								{
									SubMesh::itrMutiVertexBuff itr_next_buff = itrVertexbuff;
									itr_next_buff++;
									if (itr_next_buff == pSubMesh->getMultiVertexBuff().end())
									{
										pRenderInput->bindVertexStream(itrVertexbuff->m_vertex_element_list, itrVertexbuff->m_gpubuff, RenderInput::BS_END);
									}
									else
									{
										pRenderInput->bindVertexStream(itrVertexbuff->m_vertex_element_list, itrVertexbuff->m_gpubuff, RenderInput::BS_NORMAL);
									}
								}
							}

						}
					}
					else
					{
						pRenderInput->bindVertexStream(pSubMesh->getVertexElements(), pSubMesh->getVertexBuffer());
					}

					pRenderInput->bindIndexStream(pSubMesh->getIndexBuffer(), pSubMesh->getIndexStride());
					pRenderable->setRenderInput(pRenderInput);
					pRenderable->setRasterizerState((phase.m_materialInsts[i] ? phase.m_materialInsts[i] : 
						phase.m_materialInsts_dym[i][phase.m_materialInsts_dym[i].size() - 1])->getRasterizerState());
					pRenderable->setBlendState((phase.m_materialInsts[i] ? phase.m_materialInsts[i] :
						phase.m_materialInsts_dym[i][phase.m_materialInsts_dym[i].size() - 1])->getBlendState());
					pRenderable->setDepthStencilState((phase.m_materialInsts[i] ? phase.m_materialInsts[i] :
						phase.m_materialInsts_dym[i][phase.m_materialInsts_dym[i].size() - 1])->getDepthStencilState());

					phase.m_renderInputs.push_back(pRenderInput);
					phase.m_renderables.push_back(pRenderable);

					pRenderable->setLowRenderQueue(NULL);

					// 设置是否提交到主渲染队列
					if (pMaterialInst->isSubmitToStageRenderQueue())
					{
						RenderQueue* stageRenderQueue = SceneManager::instance()->getRenderQueue(pMaterialInst->getStage());
						pRenderable->setStageRenderQueue(stageRenderQueue);
					}
				}
			}			
		}*/
	}

	// 获取全局纹理
	Texture* Model::getGlobalTexture(const String& name, const int subMesh /* = 0 */)
	{
		if (name == "SMSampler")
		{
			RenderTarget* shadowMapRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_ShadowMap); EchoAssert(shadowMapRT);
			if (shadowMapRT)
			{
				return shadowMapRT->getBindTexture();
			}
		}
		else if (name == "LMSampler1")
		{
			TextureRes* pDefault = TextureResManager::instance()->createTexture(s_DefaultLMName);
			if (pDefault)
			{
				return pDefault->getTexture();
			}
		}
		else if (name == "LMSampler2")
		{
			TextureRes* pDefault = TextureResManager::instance()->createTexture(s_DefaultLMName);
			if (pDefault)
			{
				return pDefault->getTexture();
			}
		}
		else if (name == "RefectSampler")
		{
			RenderTarget* waterRefectRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_WaterRefectMap);
			if (waterRefectRT)
			{
				return waterRefectRT->getBindTexture();
			}
		}
		else if (name == "RefractionSampler")
		{
			RenderTarget* waterRefractionRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_WaterRefractionMap);
			if (waterRefractionRT)
			{
				return waterRefractionRT->getBindTexture();
			}
		}

		return NULL;
	}

	const TextureSampler* Model::getGlobalTextureSample(const String& name, const int subMesh /* = 0 */)
	{
		if (name == "SMSampler")
		{
			RenderTarget* shadowMapRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_ShadowMap); EchoAssert(shadowMapRT);
			if ( shadowMapRT )
			{
				return &shadowMapRT->getBindTextureSampler();
			}
		}
		else if (name == "LMSampler1")
		{
			TextureRes* pDefault = TextureResManager::instance()->createTexture(s_DefaultLMName);
			pDefault->prepareLoad();
			if (pDefault)
			{
				static TextureSampler texSampler;
				texSampler.m_texture = pDefault->getTexture();
				const SamplerState* samplerState = pDefault->getTexture()->getCurSamplerState();
				if (samplerState)
				{
					texSampler.m_samplerState = samplerState;
				}
				else
				{
					texSampler.m_samplerState = Renderer::instance()->getSamplerState(SamplerState::SamplerDesc());
				}

				return &texSampler;
			}
		}
		else if (name == "LMSampler2")
		{
			TextureRes* pDefault = TextureResManager::instance()->createTexture(s_DefaultLMName);
			pDefault->prepareLoad();
			if (pDefault)
			{
				static TextureSampler texSampler;
				texSampler.m_texture = pDefault->getTexture();
				const SamplerState* samplerState = pDefault->getTexture()->getCurSamplerState();
				if (samplerState)
				{
					texSampler.m_samplerState = samplerState;
				}
				else
				{
					texSampler.m_samplerState = Renderer::instance()->getSamplerState(SamplerState::SamplerDesc());
				}

				return &texSampler;
			}
		}
		else if (name == "RefectSampler")
		{
			RenderTarget* refectRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_WaterRefectMap);
			if (refectRT)
			{
				return &refectRT->getBindTextureSampler();
			}
		}
		else if (name == "RefractionSampler")
		{
			RenderTarget* refractionRT = RenderTargetManager::instance()->getRenderTargetByID(RTI_WaterRefractionMap);
			if (RefractionSLot)
			{
				return &refractionRT->getBindTextureSampler();
			}
		}
		else if (name == "SceneEnvSampler")
		{
			//scene* scene = SceneManager::instance()->getCurrentScene();
			//if (scene)
			//{
			//	return scene->getEnvTextureSampler();
			//}

			return nullptr;
		}

		return NULL;
	}

	// 获取着色器全局变量
	void* Model::getGlobalUniformValue(const String& name, i32 subMesh, bool isUIModel /* = false */)
	{
		if(name =="matW")
		{
			//return (void*)&(m_pSceneNode->getWorldMatrix());
		}
		else if (name == "matV")
		{
			//return (void*)&(SceneManager::instance()->getMainCamera()->getViewMatrix());
		}
		else if (name == "matWV")
		{
			return (void*)&m_matWV;
		}
		else if(name =="matWVP")
		{
			return (void*)&m_matWVP;
		}
		else if (name == "matWVPWater")
		{
			m_isNeedUpdateMatWater = true;
			return (void*)&m_matWVPWater;
		}
		else if (name == "matWVPSky")
		{
			m_isNeedUpdateMatSky = true;
			return (void*)&m_matWVPSky;
		}
		else if(name =="camPos" || name=="CameraPosition")
		{
			if ( isUIModel )
			{
				return (void*)NodeTree::instance()->getGUICamera()->getPositionPtr();
			}
			return (void*)NodeTree::instance()->getMainCamera()->getPositionPtr();
		}
		else if (name == "CameraDirection")
		{
			if ( isUIModel )
			{
				return (void*)&NodeTree::instance()->getGUICamera()->getDirection();
			}
			return (void*)&NodeTree::instance()->getMainCamera()->getDirection();
		}
		else if (name == "LightArrayInfo")
		{
			return (void*)m_lightArray->getLightInfo();
		}
		else if (name == "LightArrayData")
		{
			return (void*)m_lightArray->getLightData();
		}
		else if (name == "ActorLitParam" || name == "LightParams")
		{
			//return (void*)SceneManager::instance()->getActorLightParamPtr();
		}
		else if (name == "SceneObjectLightParams")
		{
			//return (void*)SceneManager::instance()->getSceneObjectLightParamPtr();
		}
		//else if(name =="matBoneRows")
		//{
		//	Vector4* pBoneMatRaws;/* = m_mesh->getUseSubMeshBoneMatrices() ? m_subMeshBoneMatRows[subMesh] : m_boneMatRows;*/
		//	return (void*)pBoneMatRaws;
		//}
		else if(name == "fogParam")
		{
			//if (isUIModel)
			//{
			//	return (void*)SceneManager::instance()->getUIFogParamVec4Ptr();
			//}
			//return (void*)SceneManager::instance()->getFogParamVec4Ptr();
		}
		else if (name == "heightFogParam")
		{
			//if (isUIModel)
			//{
			//	return (void*)&SceneManager::instance()->getUIHeightFogParam();
			//}
			//return (void*)&SceneManager::instance()->getHeightFogParam();
		}
		else if (name == "matWVPSM")
		{
			return (void*)&m_matWVPSM;
		}
		else if (name == "matWSM")
		{
			return (void*)&m_matWSM;
		}
		else if (name == "ShadowShade")
		{
			ShadowMapRenderStage* pStage = (ShadowMapRenderStage*)RenderStageManager::instance()->getRenderStageByID(RSI_ShadowMap);
			return (void*)pStage->getShadowShadePtr();
		}
		else if (name == "SMSampler")
		{
			return (void*)&SMSlot;
		}
		else if (name == "SceneEnvSampler")
		{
			return (void*)&SceneEnvSlot;
		}
		else if (name == "CurrentTime" || name == "offset")
		{
			return (void*)&m_currentTime;
		}
		else if (name == "uLightPositions")
		{
			return m_lightArray->getPBRLightPos();
		}
		else if (name == "uLightDirections")
		{
			return m_lightArray->getPBRLightDir();
		}
		else if (name == "uLightColors")
		{
			return m_lightArray->getPBRLightColor();
		}
		else if (name == "uLightParams")
		{
			return m_lightArray->getPBRLightParam();
		}
		else if (name == "uLightSpot")
		{
			return m_lightArray->getPBRLightSopt();
		}
		else if (name == "RefectSampler")
		{
			return (void*)&RefectSLot;
		}
		else if(name == "RefractionSampler")
		{
			return (void*)&RefractionSLot;
		}
		else if (name == "tShadowAlbedo")
		{
			return (void*)&ShadowAlbedoSlot;
		}
#ifndef ECHO_EDITOR_MODE
		else if ( name == "coveragePos" )
		{
			return (void*)NodeTree::instance()->getMainActorPositionPtr();
		}
		else if (name == "filterColor")
		{
			return const_cast<void*>(static_cast<const void*>(&NodeTree::instance()->getFilterColor()));
		}
		else if (name == "filterGray")
		{
			return const_cast<void*>(static_cast<const void*>(&NodeTree::instance()->getFilterGray()));
		}
#endif
		else
		{
			return NULL;
		}

		return NULL;
	}

	void* Model::extraCalcUniform(const String& name, void* value, i32 subMeshId)
	{
		if (name == "hsvColorRChannel" || name == "hsvColorGChannel" || name == "hsvColorBChannel")
		{
#ifdef ECHO_EDITOR_MODE
			Vector3 hsvColor = *(Vector3*)value;
			hsvColor.x = Math::Clamp(hsvColor.x, 0.f, 360.f);
			hsvColor.x /= 360.f;
			hsvColor.y = Math::Clamp(hsvColor.y, 0.f, 1.f);
			hsvColor.z = Math::Clamp(hsvColor.z, 0.f, 1.f);

			Color::HSV_to_RGB(hsvColor);

#else
			Vector3* hsvColor = (Vector3*)value;
			hsvColor->x = Math::Clamp(hsvColor->x, 0.f, 360.f);
			hsvColor->x /= 360.f;
			hsvColor->y = Math::Clamp(hsvColor->y, 0.f, 1.f);
			hsvColor->z = Math::Clamp(hsvColor->z, 0.f, 1.f);

			Color::HSV_to_RGB(*hsvColor);
#endif
		}

		return value;
	}

	bool Model::modifyUniformValue(const String& name, void* value, RenderPhaseType phase /* = RP_Normal0 */)
	{
		bool isSuc = false;
		RenderPhase& renderPhase = *m_phases[phase];
		if (renderPhase.m_materialInst)
		{
			if (renderPhase.m_materialInst->GetUniform(name))
			{
				renderPhase.m_materialInst->modifyUniformValue(name, value);
				isSuc = true;
			}
		}

		return isSuc;
	}

	void Model::modifySubMeshUniformValue(i32 subId, const String &name, void* value, RenderPhaseType phase /* = RP_Normal0 */)
	{
		RenderPhase& renderPhase = *m_phases[phase];

		if (renderPhase.m_materialInst && renderPhase.m_materialInst->GetUniform(name))
			renderPhase.m_materialInst->modifyUniformValue(name, value);
	}

	// 提交到渲染队列
	void Model::submitToRenderQueue()
	{
		for (int i = RP_Normal0; i <= RP_Normal3; i++)
		{
			Renderable* renderable = m_phases[i]->m_renderable;
			if (renderable->isVisible())
			{
				renderable->submitToRenderQueue();
			}
		}
	}

	// 提交到阴影渲染队列
	void Model::submitToSMRenderQueue()
	{
		if (!m_isInShowdownBox)
			return;

		Renderable* renderable = m_phases[RP_ShadowMap]->m_renderable;
		if (renderable)
		{
			renderable->submitToRenderQueue();
		}
	}

	// 设置骨架
	void Model::setSkeleton(Skeleton* pSkeleton)
	{
		// if you need re use the skinned entity to render another skin mesh 
		// please delete this, and malloc a new skinedEntity.
		EchoAssert(m_pSkeleton==NULL);
		m_pSkeleton = pSkeleton;

		m_boneCount = m_pSkeleton->getBoneCount();

		m_boneMatRows = EchoAlloc(Vector4, m_boneCount*3);

		Matrix4 mat = Matrix4::IDENTITY;
		for(size_t i=0; i<m_boneCount; ++i)
		{
			m_boneMatRows[i*3].set(mat.m00, mat.m10, mat.m20, mat.m30);
			m_boneMatRows[i*3+1].set(mat.m01, mat.m11, mat.m21, mat.m31);
			m_boneMatRows[i*3+2].set(mat.m02, mat.m12, mat.m22, mat.m32);
		}
	}

	void Model::setAnimBlender( AnimBlender* pAnimation )
	{
		// only one animBlender
		EchoAssert(m_pAnimBlender == NULL);
		EchoAssert(m_pSkeleton);
		EchoAssert(m_pSkeleton == pAnimation->getSkeleton());

		m_pAnimBlender = pAnimation;
	}

	void Model::refreshSubMeshBoneMatRows()
	{
		/*
		// 复制动画结果数据
		if (m_mesh->getUseSubMeshBoneMatrices() && m_boneMatRows)
		{
			for (int i = 0; i < m_mesh->getSubMeshNum(); i++)
			{
				SubMesh* pSubMesh = m_mesh->getSubMesh(i);
				for (size_t j = 0; j < pSubMesh->getBoneNum(); j++)
				{
					ui32 srcIdx = pSubMesh->getBoneIdx(j);
					memcpy(&m_subMeshBoneMatRows[i][j * 3], &m_boneMatRows[srcIdx * 3], sizeof(Vector4)* 3);
				}
			}
		}*/
	}

	Box Model::getLocalMeshAABB()
	{
		return m_mesh ? m_mesh->getLocalBox() : Box(-3.f, -3.f, -3.f, 3.f, 3.f, 3.f);
	}


	// 获取本地包围盒
	Echo::Box Model::getLocalAABB() const
	{
		if (m_pAnimBlender && m_pAnimBlender->getAnimState())
		{
			AABBKeyFrame* pKeyFrame = (AABBKeyFrame*)m_pAnimBlender->getAnimState()->getCurrentAnimKeyFrame();
			Box ret = pKeyFrame->getBox();
			ret.unionBox(m_mesh->getLocalBox());	
			return ret;
		}
		
		return m_mesh ? m_mesh->getLocalBox() : Box( -3.f, -3.f, -3.f, 3.f, 3.f, 3.f);
	}

	void Model::updateCameraPos()
	{
		//m_matWV = m_pSceneNode->getWorldMatrix() * SceneManager::instance()->getMainCamera()->getViewMatrix();
		//m_matWVP = m_pSceneNode->getWorldMatrix() * SceneManager::instance()->getMainCamera()->getViewProjMatrix();
	}

	void Model::updateRenderablesPosition()
	{
		//if (!m_mesh || !m_pSceneNode)
		{
			return;
		}

		//for (RenderPhase* phase : m_phases)
		//{
		//	for (int i = 0; i < m_mesh->getSubMeshNum(); ++i)
		//	{
		//		if (i >= static_cast<int>(phase->m_materialInsts.size()) || phase->m_materialInsts[i] == nullptr)
		//		{
		//			continue;
		//		}

		//		SubMesh* subMesh = m_mesh->getSubMesh(i);
		//		Renderable* renderable = phase->m_renderables[i];
		//		renderable->m_worldPos = m_pSceneNode->getWorldPosition() + subMesh->getLocalBox().getCenter();
		//	}
		//}
	}

	void Model::execute_render()
	{
		for (int i = RP_Normal0; i <= RP_Normal3; i++)
		{	 
			if (m_phases[i]->m_renderable->isVisible())
			{
				m_phases[i]->m_renderable->render();
			}
		}
	}
	
	__ImplementSingleton(ModelManager);

	// 构造函数
	ModelManager::ModelManager()
	{
		__ConstructSingleton;
	}

	// 析构函数
	ModelManager::~ModelManager()
	{
		destroyAllModels();
		delAllModelTemplate();

		__DestructSingleton;
	}

	// 添加模型
	void ModelManager::addModel(Model* model)
	{
		m_Models.insert(model);

		// 是否投射阴影
		if (model->getModelInfo().m_isCastShadow)
			m_modelsCastShadow.insert(model);
	}

	// 准备模板
	Model::Info* ModelManager::prepareModelTemplate(const String& modelName)
	{
		// 加载模板
		Model_InfoItor it = m_ModelInfoMap.find(modelName);
		if (it == m_ModelInfoMap.end())
		{
			addModelTemplate(modelName);

			// 返回结果
			it = m_ModelInfoMap.find(modelName);
			if (it != m_ModelInfoMap.end())
			{
				return it->second;
			}

			// 模版加载失败
			EchoLogError("addModelTemplate failed, check wether the model %s is exist", modelName.c_str());
			return NULL;
		}
		else
		{
			return it->second;
		}
	}

	// 创建模型
	void ModelManager::createModel(const String& modelName, bool isSync, std::function<bool(Model*)> onCreateModelComplete, bool isForUI)
	{
#ifdef ECHO_PLATFORM_ANDROID
		// isSync = true; 
#endif
 		if (modelName.empty())
		{
			EchoLogDebug("modelName empty, CreateModel Failed."); 
			return; 
		}
	
		Model::Info* info = prepareModelTemplate(modelName);
		if (info)
		{
			info->m_isForUI = isForUI;
			
			Model* model = EchoNew(Model(*info));
			if (isSync)
			{
				model->loadFromFile();
				bool ret = onCreateModelComplete(model);

				addModel(model);

				model->setEnable(true);

				if (!ret)
				{
					destroyModel(model);
				}
			}
			else
			{
				StreamThread* thread = StreamThread::Instance();
				if (thread && thread->IsRunning())
				{
					ModelLoadEvent* task = EchoNew(ModelLoadEvent(model, onCreateModelComplete));
					thread->addTask(task);
				}
			}
		}
	}

	// 删除模型
	void ModelManager::destroyModel( Model* model, bool isSync /*= false*/ )
	{
#ifdef ECHO_EDITOR_MODE
		if (model)
		{
			Model::Info modelInfo = model->getModelInfo();
			delModelTemplate(modelInfo.m_name);
		}
#endif
		ModelItor it = m_Models.find(model);
		if (it != m_Models.end())
		{
			m_Models.erase(it);
		}

		// 移出投射阴影模型列表
		it = m_modelsCastShadow.find(model);
		if (it != m_modelsCastShadow.end())
		{
			m_modelsCastShadow.erase(it);
		}

		auto onDestroyComplete = [](Model* m)
		{
			EchoAssert(m);
			m->unload();
			EchoSafeDelete(m, Model);
			
		};

		if (isSync)
		{
			model->setEnable(false);
			onDestroyComplete(model);
		}
		else
		{
			StreamThread* thread = StreamThread::Instance();
			ModelUnLoadEvent* task = EchoNew(ModelUnLoadEvent(model, onDestroyComplete));
			thread->addTask(task); 
		}
	}

	// 删除所有模型
	void ModelManager::destroyAllModels()
	{
		EchoSafeDeleteContainer( m_Models, Model);
		m_modelsCastShadow.clear();
	}

	// 获取投影阴影模型包围盒
	void ModelManager::buildBoxSM(Box& oBox, const Vector3& mainPos, Camera* cam, float shadowDistance)
	{
		//m_criticalValue = shadowDistance;
		//for (ModelSet::iterator it = m_modelsCastShadow.begin(); it != m_modelsCastShadow.end(); it++)
		//{
		//	Model* model = *it;
		//	if (model->isVisible() && model->getModelInfo().m_isCastShadow)
		//	{
		//		Vector3 center = model->getSceneNode()->getWorldPosition();
		//		float centerLen = (center - mainPos).len();
		//		bool flag = centerLen <= shadowDistance;
		//		if (!flag)
		//		{
		//			if (centerLen <= m_criticalValue + 0.5)
		//			{
		//				if (m_criticalValue <= shadowDistance + 5.0)
		//					m_criticalValue += 0.5;
		//				flag = true;
		//			}
		//		}
		//		if (flag && !mainPos.isInvalid())
		//		{
		//			node* pNode = model->getSceneNode();
		//			const Box& modelBox = model->getLocalAABB().transform(pNode->getWorldMatrix());
		//			if (!cam->getFrustum().isVisible(modelBox))
		//			{
		//				continue;
		//			}

		//			if (EchoEngineSettings.isEnableMainActorShadow() && !model->getModelInfo().m_isMainActor)
		//			{
		//				continue;
		//			}

		//			oBox.unionBox(modelBox);
		//			model->submitToSMRenderQueue();
		//		}
		//	}
		//}
	}

	// 删除所有模型模板
	void ModelManager::delAllModelTemplate()
	{
		Model_InfoItor it = m_ModelInfoMap.begin();
		for (; it!=m_ModelInfoMap.end();++it)
		{
			EchoSafeDelete(it->second, Info);
		}
		m_ModelInfoMap.clear();
	}

	// 添加模型模板
	bool ModelManager::addModelTemplate( const String& modelName )
	{
		Model_InfoItor it = m_ModelInfoMap.find(modelName);
		if (it == m_ModelInfoMap.end())
		{
			Model::Info* pModelInfo = EchoNew(Model::Info);
			if (!loadModelByFile(modelName, pModelInfo))
			{
				EchoSafeDelete(pModelInfo, Info);
				return false;
			}

			m_ModelInfoMap[modelName] = pModelInfo;
		}

		return true;
	}

	bool ModelManager::delModelTemplate(const String& modelName)
	{
		Model_InfoItor it = m_ModelInfoMap.find(modelName);
		if(it != m_ModelInfoMap.end())
		{
			EchoSafeDelete(it->second, Info);
			m_ModelInfoMap.erase(it);
			return true;
		}

		return false;
	}

	// 获取模型模板
	Model::Info* ModelManager::getModelTemplate(const String& modelName)
	{
		Model_InfoItor it = m_ModelInfoMap.find(modelName);
		if (it == m_ModelInfoMap.end())
		{
			if (!addModelTemplate(modelName))
			{
				return nullptr;
			}
			it = m_ModelInfoMap.find(modelName);
		}

		return it->second;
	}

	// 保存辅助
	inline String safesavestr(const String& str)
	{
		return str.empty() ? "null" : str;
	}

	// 读取字符串辅助
	inline String safegetstr(const String& str)
	{
		return str == "null" ? "" : str;
	}

	// 加载模型资源
	bool ModelManager::loadModelByFile( const String& modelName ,Model::Info* modelInfo )
	{
		try
		{
			MemoryReader reader( modelName.c_str());
			if (reader.getData<char*>() == nullptr)
			{
				return false;
			}

			xml_document<> doc;        
			doc.parse<0>( reader.getData<char*>());

			modelInfo->m_name = modelName;

			xml_node<> *pNode = doc.first_node();
			
			String str = pNode->value();
			modelInfo->m_geometryType = StringUtil::ParseBool(str) ? Model::GT_Skin : Model::GT_StaticMesh;
			pNode = pNode->next_sibling();
			modelInfo->m_meshName = pNode->value();

			//modelInfo->m_originName = "";
			xml_node<>* originNameNode = pNode->next_sibling();
			char* name = originNameNode->name();
			for (; strcmp(name, "OriginName") == 0; )
			{
				String originName = originNameNode->value();
				modelInfo->m_originNames.push_back(originName);

				originNameNode = originNameNode->next_sibling();
				name = originNameNode->name();
			}
			
			// 是否投射接收阴影
			xml_node<>* modelNode = doc.first_node("Model");
			if (modelNode)
			{
				modelInfo->m_isCastShadow    = rapidxml_helper::parsebool(modelNode->first_attribute("CastShadow"), false);
				modelInfo->m_isReceiveShadow = rapidxml_helper::parsebool(modelNode->first_attribute("ReceiveShadow"), false);
			}

			xml_node<>* materialInstNode = doc.first_node("MaterialInsts");
			if (materialInstNode)
			{
				xml_attribute<>* rp0 = materialInstNode->first_attribute("rp0");
				xml_attribute<>* rp1 = materialInstNode->first_attribute("rp1");
				xml_attribute<>* rp2 = materialInstNode->first_attribute("rp2");
				xml_attribute<>* rp3 = materialInstNode->first_attribute("rp3");
				xml_attribute<>* all = materialInstNode->first_attribute("all");
				modelInfo->m_materialInsts[Model::RP_Normal0] = rp0->value();
				modelInfo->m_materialInsts[Model::RP_Normal1] = rp1->value();
				modelInfo->m_materialInsts[Model::RP_Normal2] = rp2->value();
				modelInfo->m_materialInsts[Model::RP_Normal3] = rp3->value();
				modelInfo->m_materialInstsAll = all->value();
			}

			// 静态烘焙配置
			xml_node<>* lightmassNode = doc.first_node("lightmass");
			if (lightmassNode)
			{
				StringArray normalMaps = Echo::StringUtil::Split(rapidxml_helper::get_string(lightmassNode->first_attribute("Normal")), ";");
				StringArray diffuseMaps = Echo::StringUtil::Split(rapidxml_helper::get_string(lightmassNode->first_attribute("Diffuse")), ";");
				StringArray emissiveMaps = Echo::StringUtil::Split(rapidxml_helper::get_string(lightmassNode->first_attribute("Emissive")), ";");
				StringArray specularMaps = Echo::StringUtil::Split(rapidxml_helper::get_string(lightmassNode->first_attribute("Specular")), ";");
				modelInfo->m_lightmassConfig.resize(normalMaps.size());
				for (size_t i = 0; i < normalMaps.size(); i++)
				{
					modelInfo->m_lightmassConfig[i].m_textureNormal = safegetstr(normalMaps[i]);
					modelInfo->m_lightmassConfig[i].m_textureDiffuse = safegetstr(diffuseMaps[i]);
					modelInfo->m_lightmassConfig[i].m_textureEmissive = safegetstr(emissiveMaps[i]);
					modelInfo->m_lightmassConfig[i].m_textureSpecular = safegetstr(specularMaps[i]);
				}
			}

			// 光源阵列
			xml_node<>* lightArrayNode = doc.first_node( "LightArray");
			if (lightArrayNode)
			{
				xml_attribute<>* nameatt = lightArrayNode->first_attribute("name");
				if (nameatt)
					modelInfo->m_lightArray = nameatt->value();
			}

			return true;
		}
		catch(...)
		{
			EchoLogError("Parse model file [%s] failed.", modelName.c_str());
		}

		return false;
	}

	// 保存模型文件
	void ModelManager::saveModelToFile( const String& modelName ,const Model::Info& modelInfo)
	{
		xml_document<> doc;
		rapidxml_helper helper( &doc);


		xml_node<>* rot = doc.allocate_node(rapidxml::node_pi,doc.allocate_string("xml version='1.0' encoding='utf-8'"));
		doc.append_node(rot);

		// 写属性
		char* strValue = doc.allocate_string(StringUtil::ToString(modelInfo.isSkinModel()).c_str());
		xml_node<>* pNode = doc.allocate_node(rapidxml::node_element,"IsSkinMesh",strValue);
		doc.append_node(pNode);
		strValue = doc.allocate_string(modelInfo.m_meshName.c_str());
		pNode = doc.allocate_node(rapidxml::node_element,"ModelName",strValue);
		doc.append_node(pNode);
		for (size_t i = 0; i < modelInfo.m_originNames.size(); i++)
		{
			strValue = doc.allocate_string(modelInfo.m_originNames[i].c_str());
			pNode = doc.allocate_node(rapidxml::node_element, "OriginName", strValue);
			doc.append_node(pNode);
		}

		// 是事投射接收阴影
		xml_node<>* modelNode = doc.allocate_node(rapidxml::node_element, "Model");
		modelNode->append_attribute(doc.allocate_attribute("CastShadow", helper.tostr(modelInfo.m_isCastShadow)));
		modelNode->append_attribute(doc.allocate_attribute("ReceiveShadow", helper.tostr(modelInfo.m_isReceiveShadow)));
		doc.append_node(modelNode);

		// 材质实例设置
		xml_node<>* materialInstsNode = doc.allocate_node(node_element, "MaterialInsts");
		xml_attribute<>* renderPhase0 = doc.allocate_attribute("rp0", modelInfo.m_materialInsts[Model::RP_Normal0].c_str());
		xml_attribute<>* renderPhase1 = doc.allocate_attribute("rp1", modelInfo.m_materialInsts[Model::RP_Normal1].c_str());
		xml_attribute<>* renderPhase2 = doc.allocate_attribute("rp2", modelInfo.m_materialInsts[Model::RP_Normal2].c_str());
		xml_attribute<>* renderPhase3 = doc.allocate_attribute("rp3", modelInfo.m_materialInsts[Model::RP_Normal3].c_str());
		xml_attribute<>* allMaterials = doc.allocate_attribute("all", modelInfo.m_materialInstsAll.c_str());
		materialInstsNode->append_attribute(renderPhase0);
		materialInstsNode->append_attribute(renderPhase1);
		materialInstsNode->append_attribute(renderPhase2);
		materialInstsNode->append_attribute(renderPhase3);
		materialInstsNode->append_attribute(allMaterials);
		doc.append_node(materialInstsNode);

		// 静态烘焙配置
		String normalStr,diffuseStr, emissiveStr, specularStr;
		for (size_t i = 0; i < modelInfo.m_lightmassConfig.size(); i++)
		{
			normalStr += safesavestr(modelInfo.m_lightmassConfig[i].m_textureNormal) + ";";
			diffuseStr += safesavestr(modelInfo.m_lightmassConfig[i].m_textureDiffuse) + ";";
			emissiveStr += safesavestr(modelInfo.m_lightmassConfig[i].m_textureEmissive) + ";";
			specularStr += safesavestr(modelInfo.m_lightmassConfig[i].m_textureSpecular) + ";";
		}

		xml_node<>* lightmassNode = doc.allocate_node(node_element, "lightmass");
		lightmassNode->append_attribute(doc.allocate_attribute("Normal", normalStr.c_str()));
		lightmassNode->append_attribute(doc.allocate_attribute("Diffuse", diffuseStr.c_str()));
		lightmassNode->append_attribute(doc.allocate_attribute("Emissive", emissiveStr.c_str()));
		lightmassNode->append_attribute(doc.allocate_attribute("Specular", specularStr.c_str()));
		doc.append_node(lightmassNode);

		// 光源阵列
		xml_node<>* lightArrayNode = doc.allocate_node( node_element, "LightArray");
		xml_attribute<>* value = doc.allocate_attribute("name", modelInfo.m_lightArray.c_str());
		lightArrayNode->append_attribute(value);
		doc.append_node(lightArrayNode);

		std::ofstream out(modelName.c_str());
		out << doc;
	}

	bool ModelManager::refreshModelTemplate(const String& modelName)
	{
		Model_InfoItor it = m_ModelInfoMap.find(modelName);
		if (it != m_ModelInfoMap.end())
		{
			Model::Info* pModelInfo = it->second;
			if (!loadModelByFile(modelName, pModelInfo))
			{
				return false;
			}

			ModelItor it = m_Models.begin();
			for (; it != m_Models.end(); ++it)
			{
				if ((*it)->getModelInfo().m_name == modelName)
				{
					(*it)->setInfo(*pModelInfo);
					(*it)->reload();
					(*it)->createRenderable();
				}
			}
			
			return true;
		}

		return false;
	}

	bool Model::saveMesh(bool useNornalMap /* = true */)
	{
		if ( m_mesh )
		{
			m_mesh->generateTangentData( useNornalMap );

			String filePath = m_mesh->getName( );
			filePath =  IO::instance()->getFileLocation( filePath );
			//return m_mesh->saveToFile( filePath.c_str() );
		}

		return false;
	}

	void Model::resetRenderInput()
	{/*
#ifdef ECHO_EDITOR_MODE
		for (RenderPhase* phase : m_phases)
		{
			EchoSafeDeleteContainer(phase->m_renderInputs, RenderInput);
			for (int i = 0; i < m_mesh->getSubMeshNum(); ++i)
			{
				if (i >= static_cast<int>(phase->m_materialInsts.size()) || phase->m_materialInsts[i] == nullptr)
				{
					continue;
				}
				
				SubMesh*			pSubMesh = m_mesh->getSubMesh(i); EchoAssert(pSubMesh);
				Renderable*			pRenderable = phase->m_renderables[i];
				Material*			pMaterial = phase->m_materialInsts[i]->getRenderQueue()->getMaterial();

				RenderInput* pRenderInput = Renderer::instance()->createRenderInput(pMaterial->getShaderProgram());
				pRenderInput->enableWireFrame(true);
				pRenderInput->setOwnerInfo(StringUtil::Format("model [%s]'s subMesh [%s] ", m_info.m_name.c_str(), pSubMesh->getName().c_str()));

				pRenderInput->bindVertexStream(pSubMesh->getVertexElements(), pSubMesh->getVertexBuffer());
				pRenderInput->bindIndexStream(pSubMesh->getIndexBuffer(), pSubMesh->getIndexStride());
				pRenderable->setRenderInput(pRenderInput);

				phase->m_renderInputs.push_back(pRenderInput);
			}
			
		}
#endif // ECHO_EDITOR_MODE
*/
	}

	ModelLoadEvent::ModelLoadEvent(Model* model, std::function<bool(Model*)> onModelLoadComplete)
		: m_model(model)
		, m_modelId(model->getIdentifier())
		, m_onModelLoadComplete(onModelLoadComplete)
	{}

	ModelLoadEvent::~ModelLoadEvent()
	{}

	// 执行模型加载
	bool ModelLoadEvent::process()
	{
		if (m_model)
		{
			if (!m_model->m_isEnable)
				m_model->loadFromFile();
		}

		return true; 
	}

	bool ModelLoadEvent::finished()
	{
		Model* model = IdToPtr(Model, m_modelId);
		if (model)
		{
			if (model->m_isEnable)
				return true;

			bool ret = m_onModelLoadComplete(model);

			model->m_isEnable = true;

			ModelManager::instance()->addModel(model);
			if (!ret)
			{
				ModelManager::instance()->destroyModel(model);
			}
		}

		return true; 
	}

	ModelUnLoadEvent::ModelUnLoadEvent(Model* model, std::function<void(Model*)> onDestroyComplete)
		: m_model(model)
		, m_onDestroyComplete(onDestroyComplete)
	{}

	ModelUnLoadEvent::~ModelUnLoadEvent()
	{}

	bool ModelUnLoadEvent::process()
	{
		// empty implement. just for secquence.
		return true; 
	}

	bool ModelUnLoadEvent::finished()
	{
		if (!m_model->m_isEnable)
			return true;

		m_model->m_isEnable = false; 

		m_onDestroyComplete(m_model);
		return true; 
	}
}