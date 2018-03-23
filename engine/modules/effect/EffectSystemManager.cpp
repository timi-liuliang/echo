#include "Engine/core/main/Root.h"
#include "EffectSystemManager.h"
#include "EffectSystem.h"
#include "EffectParticleControllerSet.h"
#include "EffectLayer.h"
#include "EffectMaterial.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/Util/Exception.h"
#include "Render/Renderer.h"
#include "Render/ShaderProgram.h"
#include "Engine/core/Scene/Scene_Manager.h"
#include "EffectLayer2DBillBoard.h"
#include "EffectLayer3DBillBoard.h"
#include "EffectLayerColumn.h"
#include "EffectLayerTrail.h"
#include "EffectLayerParaboloidal.h"
#include "EffectLayer3DGrid.h"
#include "EffectLayer2DGrid.h"
#include "EffectLayerLighting.h"
#include "EffectLayerParticlesPoint.h"
#include "EffectLayerParticlesCube.h"
#include "EffectLayerParticlesSphere.h"
#include "EffectLayerParticlesCylinder.h"
#include "EffectLayerParticlesCurve.h"
#include "EffectLayerSpriteAnimation.h"
#include "engine/core/resource/ResourceGroupManager.h"
#include "Render/Material.h"
#include "engine/core/render/RenderTargetManager.h"
#include "Engine/core/Render/TextureResManager.h"
#include "Engine/core/Render/MaterialInst.h"
#include "render/RenderQueueGroup.h"

namespace Echo
{
	void EffectManualRender::render()
	{
		EffectSystemManager::instance()->endRender();
	}

	void EffectManualRender::beginRender()
	{
		EffectSystemManager::instance()->beginRender();
	}

	__ImplementSingleton(EffectSystemManager);

	// 构造函数
	EffectSystemManager::EffectSystemManager()
		: mEffectRenderInput(NULL)
		, mEffectUV2RenderInput(NULL)
		, mEffectSamplerState(NULL)
		, mEffectAlphaLighting(NULL)
		, mEffectHighLighting(NULL)
		, mEffectWireFrameLighting(NULL)
		, mEffectDepthState(NULL)
		, mEffectUnWriteDepthState(NULL)
		, mEffectRasterizerState(NULL)
		, mRenderalbeSetCreated(false)
		, mMtlDistortion(0)
		, mTextureNoise(0)
		, mMtlUV2(NULL)
		, mEnableRender( true )
		, mRenderUIEffectOnly(false)
		, m_vertexBuffer(NULL)
		, m_indexBuffer(NULL)
		, m_renderlevelThreshhold( ERL_High)
		, m_camera( NULL)
		, m_userCamera( NULL)
		, mEffectRenderablePtr(NULL)
		, mEffectMaterialPtr(NULL)
		, m_bRenderGlowOver(false)
		, bUseWireFrameMode(false)
		, mEffectRenderQueue(NULL)
	{
		__ConstructSingleton;
		m_particlePool = EchoNew(EffectParticlePool(1024));
	}

	// 析构函数
	EffectSystemManager::~EffectSystemManager (void)
	{
		destroyAllEffectSystemTemplates();
		destroyAllEffectSystems();
		destroyRenderableSet();
		MaterialManager::instance()->destroyMaterial(mMtlDistortion);
		MaterialManager::instance()->destroyMaterial(mMtlUV2);
		EchoSafeDelete(m_particlePool, EffectParticlePool);

		__DestructSingleton;
	}

	// 初始化顶点索引缓冲
	void EffectSystemManager::initBuffer()
	{
		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		Buffer vertBuff( 0, NULL);
		m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ|GPUBuffer::GBU_CPU_WRITE, vertBuff);

		EchoSafeDelete(m_indexBuffer, GPUBuffer);
		Buffer idxBuff( 0, NULL);
		m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ|GPUBuffer::GBU_CPU_WRITE, idxBuff);
	}

	// 更新
	void EffectSystemManager::tick(i32 elapsedTime)
	{
		OpenMPTaskMgr::instance()->execTasks(OpenMPTaskMgr::TT_EffectSystem);
		OpenMPTaskMgr::instance()->waitForEffectSystemUpdateComplete();

		// 更新粒子池
		m_particlePool->tick(elapsedTime);

		// 更新渲染单元,移除空闲渲染单元
		for (RenderableMap::iterator it = m_usedRenderables.begin(); it != m_usedRenderables.end();)
		{
			EffectRenderable* renderable = IdToPtr( EffectRenderable, it->second);
			if (renderable)
			{
				renderable->tick( elapsedTime);
				if (renderable->getIdleTime() > 5000)
				{
					EchoSafeDelete( renderable, EffectRenderable);
					m_usedRenderables.erase(it++);
				}
				else
				{
					it++;
				}
			}
			else
			{
				m_usedRenderables.erase(it++);
			}
		}
	}

	// 销毁指定特效模板
	void EffectSystemManager::destoryEffectSystemTemplate(const String& templateName)
	{
		EffectSystemMap::iterator it = mEffectSystemTemplates.find(templateName);
		if (it != mEffectSystemTemplates.end())
		{
			EchoSafeDelete(it->second, EffectSystem);
			mEffectSystemTemplates.erase(it);
		}
	}

	// 销毁所有特效模板
	void EffectSystemManager::destroyAllEffectSystemTemplates() 
	{ 
		EffectSystemMap::iterator t;
		for (t = mEffectSystemTemplates.begin(); t != mEffectSystemTemplates.end(); ++t)
		{
			EchoSafeDelete(t->second, EffectSystem);
		}
		mEffectSystemTemplates.clear();
	}

	// 创建关键帧
	EffectKeyFrame* EffectSystemManager::createKeyFrame(void)
	{
		EffectKeyFrame* keyFrame = EchoNew(EffectKeyFrame);
		return keyFrame;
	}
	EffectKeyFrame* EffectSystemManager::cloneKeyFrame(EffectKeyFrame *keyFrame)
	{
		if(!keyFrame)
			return 0;

		EffectKeyFrame* clonekeyframe = createKeyFrame();
		keyFrame->copyAttributesTo(clonekeyframe);
		return clonekeyframe;
	}
	void EffectSystemManager::destroyKeyFrame(EffectKeyFrame* keyFrame)
	{
		EchoSafeDelete(keyFrame, EffectKeyFrame);
	}
	//-----------------------------------------------------------------------
	EffectParticleControllerSet* EffectSystemManager::createParticleControllerSet(void)
	{
		EffectParticleControllerSet* particleControllerSet = 
			EchoNew(EffectParticleControllerSet);
		return particleControllerSet;
	}
	EffectParticleControllerSet* EffectSystemManager::cloneParticleControllerSet(
		EffectParticleControllerSet* particleControllerSet)
	{
		if(!particleControllerSet)
			return 0;

		EffectParticleControllerSet* cloneparticleControllerSet = createParticleControllerSet();
		particleControllerSet->copyAttributesTo(cloneparticleControllerSet);
		return cloneparticleControllerSet;
	}
	void EffectSystemManager::destroyParticleControllerSet(EffectParticleControllerSet* particleControllerSet)
	{
		EchoSafeDelete(particleControllerSet, EffectParticleControllerSet);
	}
	//-----------------------------------------------------------------------
	EffectMaterial* EffectSystemManager::createMaterial()
	{
		EffectMaterial* material = EchoNew(EffectMaterial);
		mEffectMaterialPtr = PtrMonitor<EffectMaterial>::m_ptrs;
		return material;
	}
	EffectMaterial* EffectSystemManager::cloneMaterial(EffectMaterial* material)
	{
		if(!material)
			return 0;

		EffectMaterial* cloneMaterial = createMaterial();
		material->copyAttributesTo(cloneMaterial);
		return cloneMaterial;
	}
	void EffectSystemManager::destroyMaterial(EffectMaterial* material)
	{
		EchoSafeDelete(material, EffectMaterial);
	}
	EffectLayer* EffectSystemManager::createLayer(EFFECT_LAYER_TYPE layerType)
	{
		EffectLayer* pResult = NULL;
		switch(layerType)
		{
		case ELT_2DBillboard:	pResult = EchoNew(EffectLayer2DBillBoard); break;
		case ELT_3DBillboard:	pResult = EchoNew(EffectLayer3DBillBoard); break;
		case ELT_Column:		pResult = EchoNew(EffectLayerColumn); break;
		case ELT_TRAIL:			pResult = EchoNew(EffectLayerTrail); break;
		case ELT_Paraboloidal:	pResult = EchoNew(EffectLayerParaboloidal); break;
		case ELT_3DEditableSurface: pResult = EchoNew(EffectLayer3DGrid); break;
		case ELT_2DEditableSurface: pResult = EchoNew(EffectLayer2DGrid); break;
		case ELT_Lighting:		pResult = EchoNew(EffectLayerLighting); break;
		case ELT_ParticlesPoint: pResult = EchoNew(EffectLayerParticlesPoint); break;
		case ELT_ParticlesCube:	 pResult = EchoNew(EffectLayerParticlesCube); break;
		case ELT_ParticlesSphere: pResult= EchoNew(EffectLayerParticlesSphere); break;
		case ELT_ParticlesColumn: pResult = EchoNew(EffectLayerParticlesCylinder); break;
		case ELT_ParticlesCruve:  pResult = EchoNew(EffectLayerParticlesCurve); break;
		case ELT_SpriteAnimation: pResult = EchoNew(EffectLayerSpriteAnimation); break;
		default: break;
		}
		return pResult;
	}
	EffectLayer* EffectSystemManager::cloneLayer(EffectLayer* layer)
	{
		if (!layer)
			return 0;

		EffectLayer* clonedLayer = createLayer(layer->getLayerType());
		layer->copyAttributesTo(clonedLayer);
		return clonedLayer;
	}
	void EffectSystemManager::destroyLayer(EffectLayer* layer)
	{
		EchoSafeDelete(layer, EffectLayer);
	}
	EFFECT_LAYER_TYPE EffectSystemManager::getLayerType(const String& typeName)
	{
		if(typeName == "2DBillboard")		return ELT_2DBillboard;
		if(typeName == "3DBillboard")		return ELT_3DBillboard;
		if(typeName == "Trail")				return ELT_TRAIL;
		if(typeName == "Column")			return ELT_Column;
		if(typeName == "Paraboloidal")		return ELT_Paraboloidal;
		if(typeName == "ParticlesPoint")	return ELT_ParticlesPoint;
		if(typeName == "ParticlesCube")		return ELT_ParticlesCube;
		if(typeName == "ParticlesSphere")	return ELT_ParticlesSphere;
		if(typeName == "ParticleColumn")	return ELT_ParticlesColumn;
		if(typeName == "ParticlesCruve")	return ELT_ParticlesCruve;
		if(typeName == "Model")				return ELT_Model;
		if(typeName == "MTModel")			return ELT_MT_Model;
		if(typeName == "Skin" )				return ELT_Skin;
		if(typeName == "Lighting")			return ELT_Lighting;
		if(typeName == "2DEditableSurface") return ELT_2DEditableSurface;
		if(typeName == "3DEditableSurface") return ELT_3DEditableSurface;
		if(typeName == "SpriteAnimation")	return ELT_SpriteAnimation;
		if(typeName == "Child")				return ELT_Child;
		return ELT_Unknow;
	}
	String EffectSystemManager::getLayerTypeString(EFFECT_LAYER_TYPE type)
	{
		switch(type)
		{
		case ELT_2DBillboard:		return "2DBillboard";
		case ELT_3DBillboard:		return "3DBillboard";
		case ELT_TRAIL:				return "Trail";
		case ELT_Column:			return "Column";
		case ELT_Paraboloidal:		return "Paraboloidal";
		case ELT_ParticlesPoint:	return "ParticlesPoint";
		case ELT_ParticlesCube:		return "ParticlesCube";
		case ELT_ParticlesSphere:	return "ParticlesSphere";
		case ELT_ParticlesColumn:	return "ParticleColumn";
		case ELT_ParticlesCruve:	return "ParticlesCruve";
		case ELT_Model:				return "Model";
		case ELT_Skin:				return "Skin";
		case ELT_SpriteAnimation:	return "SpriteAnimation";
		case ELT_Lighting:			return "Lighting";
		case ELT_2DEditableSurface:	return "2DEditableSurface";
		case ELT_3DEditableSurface:	return "3DEditableSurface";
		case ELT_Child:				return "Child";
		default:	return "Unknow";
		}
		return "Unknow";
	}
	EffectController* EffectSystemManager::createController(EFFECT_CONTROL_TYPE controlType)
	{
		EffectController* pResult = NULL;
		switch(controlType)
		{
		case ECT_Translation:		pResult = EchoNew(EffectTranslation); break;
		case ECT_RotateSelf:		pResult = EchoNew(EffectRotationSelf); break;
		case ECT_RotateAxis:		pResult = EchoNew(EffectRotationAxis); break;
		case ECT_RotateRevolution:	pResult = EchoNew(EffectRotationRevolution); break;
		case ECT_Offcenter:			pResult = EchoNew(EffectOffcenter); break;
		case ECT_ColorChange:		pResult = EchoNew(EffectColorChange); break;
		case ECT_ColorNoise:		pResult = EchoNew(EffectColorNoise); break;
		case ECT_ColorSet:			pResult = EchoNew(EffectColorSet); break;
		case ECT_ScaleChange:		pResult = EchoNew(EffectScaleChange); break;
		case ECT_ScaleNoise:		pResult = EchoNew(EffectScaleNoise); break;
		case ECT_ScaleSet:			pResult = EchoNew(EffectScaleSet); break;
		case ECT_UVSpeedSet:		pResult = EchoNew(EffectUVSpeedSet); break;
		default: EchoException("Error: no supperted controller type!"); 
		}
		return pResult;
	}
	EffectController* EffectSystemManager::cloneController(EffectController* controller)
	{
		if (!controller)
			return 0;

		EffectController* clonedController = createController(controller->getControllerType());
		controller->copyAttributesTo(clonedController);
		return clonedController;
	}
	void EffectSystemManager::destroyController(EffectController* controller)
	{
		EchoSafeDelete(controller, EffectController);
	}
	void EffectSystemManager::getControllerTypes(StringArray& stringArry)
	{
		stringArry.clear();
		stringArry.push_back("Translation");
		stringArry.push_back("RotateSelf");
		stringArry.push_back("RotateAxis");
		stringArry.push_back("RotateRevolution");
		stringArry.push_back("Offcenter");
		stringArry.push_back("ColorChange");
		stringArry.push_back("ColorNoise");
		stringArry.push_back("ColorSet");
		stringArry.push_back("ScaleChange");
		stringArry.push_back("ScaleNoise");
		stringArry.push_back("ScaleSet");
		stringArry.push_back("UVSpeedSet");
	}
	EFFECT_CONTROL_TYPE EffectSystemManager::getControllerType(const String& typeName)
	{
		if(typeName == "Translation") return ECT_Translation;
		if(typeName == "RotateSelf") return ECT_RotateSelf;
		if(typeName == "RotateAxis") return ECT_RotateAxis;
		if(typeName == "RotateRevolution") return ECT_RotateRevolution;
		if(typeName == "Offcenter") return ECT_Offcenter;
		if(typeName == "ColorChange") return ECT_ColorChange;
		if(typeName == "ColorNoise") return ECT_ColorNoise;
		if(typeName == "ColorSet") return ECT_ColorSet;
		if(typeName == "ScaleChange") return ECT_ScaleChange;
		if(typeName == "ScaleNoise") return ECT_ScaleNoise;
		if(typeName == "ScaleSet") return ECT_ScaleSet;
		if (typeName == "UVSpeedSet") return ECT_UVSpeedSet;
		return ECT_Unknow;
	}
	String EffectSystemManager::getControllerTypeString(EFFECT_CONTROL_TYPE type)
	{
		switch(type)
		{
		case ECT_Translation:		return "Translation";
		case ECT_RotateSelf:		return "RotateSelf";
		case ECT_RotateAxis:		return "RotateAxis";
		case ECT_RotateRevolution:	return "RotateRevolution";
		case ECT_Offcenter:			return "Offcenter";
		case ECT_ColorChange:		return "ColorChange";
		case ECT_ColorNoise:		return "ColorNoise";
		case ECT_ColorSet:			return "ColorSet";
		case ECT_ScaleChange:		return "ScaleChange";
		case ECT_ScaleNoise:		return "ScaleNoise";
		case ECT_ScaleSet:			return "ScaleSet";
		case ECT_UVSpeedSet:		return "UVSpeedSet";
		default: EchoException("Error: no supperted controller type!"); 
		}
		return "Unknow";
	}
	//-----------------------------------------------------------------------
	void EffectSystemManager::replaceEffectSystemTemplate(const String& name, EffectSystem* system)
	{
		EffectSystem* systemTemplate = getEffectSystemTemplate(name);
		if (systemTemplate)
		{
			*systemTemplate = *system;
		}
	}
	EffectSystem* EffectSystemManager::createEffectSystemTemplate(const String& name)
	{
		// Validate name and add a prefix if needed.
		if(mEffectSystemTemplates.find(name) != mEffectSystemTemplates.end())
		{
			EchoLogInfo("WARNING: Effect: Effect system template %s already exists. return older.", name.c_str());
			return NULL;
		}

		EffectSystem* particleSystemTemplate = EchoNew(EffectSystem(name));
		mEffectSystemTemplates[name] = particleSystemTemplate;
		return particleSystemTemplate;
	}
	//-----------------------------------------------------------------------
	EffectSystem* EffectSystemManager::getEffectSystemTemplate(const String& templateName)
	{
		EffectSystemMap::iterator i = mEffectSystemTemplates.find(templateName);
		if (i != mEffectSystemTemplates.end())
		{
			return i->second;
		}

		return 0;
	}

	// 获取特效当前使用的摄像机
	Camera* EffectSystemManager::getCamera(EffectRenderable* pRenderable)
	{
		//return m_camera ? m_camera : EchoSceneManager->getMainCamera();
		if (pRenderable->isUiEffect())
		{
			if (NULL != m_userCamera)
				return m_userCamera;
			else
				return SceneManager::instance()->getGUICamera();
		}
		else
			return SceneManager::instance()->getMainCamera();
	}

	//-----------------------------------------------------------------------
	EffectSystemID EffectSystemManager::createEffectSystem(const String& name, const String& templateName,bool isUIeffect)
	{
		// Validate name
		if ( mEffectSystems.find(name)==mEffectSystems.end())
		{
			EffectSystem* pTemplate = getEffectSystemTemplate(templateName);
			if (!pTemplate)
			{
				// 创建模板
				DataStream* pEffectStream = ResourceGroupManager::instance()->openResource(templateName);
				if (!pEffectStream || pEffectStream->size() == 0)
				{
					EchoLogError("The effect file does not exsit or size is 0. [%s]", templateName.c_str());
					return 0;
				}

				if(pEffectStream)
				{
					pTemplate = EchoNew(EffectSystem(templateName));
					pTemplate->importData(pEffectStream);
					EchoSafeDelete(pEffectStream, DataStream);
					mEffectSystemTemplates[templateName] = pTemplate;
				}
				else
				{
					EchoLogError("Effect file isn't exist. [%s]", templateName.c_str());
				}
			}

			// 根据模板创建特效
			if( pTemplate)
			{
				EffectSystem* system = EchoNew(EffectSystem(name,isUIeffect));
				system->setTemplateName(templateName);
				mEffectSystems[name] = system;
				m_effectSystemIDs[system->getIdentifier()] = system;
				*system = *pTemplate;

				return system->getIdentifier();
			}
		}
		else
		{
			EchoLogError("Error: Effect: EffectSystem with name %s already exists.", name.c_str());
		}

		return 0;
	}

	// 创建特效系统
	EffectSystem* EffectSystemManager::createEffectSystem(const String& name)
	{
		if (mEffectSystems.find(name) != mEffectSystems.end())
		{
			EchoException("Error: Effect: EffectSystem with name %s already exists.", name.c_str());
		}

		EffectSystem* system = EchoNew(EffectSystem(name));
		// need to repared.
		mEffectSystems[name] = system;
		m_effectSystemIDs[system->getIdentifier()] = system;

		return system;
	}

	// 获取特效系统
	EffectSystemID EffectSystemManager::getEffectSystem(const String& name)
	{
		if (name == StringUtil::BLANK)
			return 0;

		EffectSystemMap::iterator i = mEffectSystems.find(name);
		if (i != mEffectSystems.end())
		{
			return i->second->getIdentifier();
		}

		return 0;
	}

	// 根据ID获取特效系统
	EffectSystem* EffectSystemManager::getEffectSystem( EffectSystemID id)
	{
		EffectSystemIDMap::iterator it = m_effectSystemIDs.find( id);
		if( it!=m_effectSystemIDs.end())
			return it->second;

		return NULL;
	}

	// 销毁特效系统
	void EffectSystemManager::destroyEffectSystem( EffectSystemID& effectSystemID)
	{
		EffectSystem* pEffectSystem = getEffectSystem( effectSystemID);
		if ( pEffectSystem)
		{
			if(pEffectSystem->mIsPreparing)
			{
				pEffectSystem->mIsDestroying = true;

				// add by ajohn for childlayer
				pEffectSystem->setParentSceneNode(NULL);

				removeEffectSystemFromMap(pEffectSystem);

				EffectDestroyEvent* pEffectDestroyEvent = EchoNew( EffectDestroyEvent(pEffectSystem));
				StreamThread* streamThread = StreamThread::Instance();
				streamThread->addTask(pEffectDestroyEvent);
			}
			else
			{
				removeEffectSystemFromMap(pEffectSystem);
				//{
				//	EffectSystemIDMap::iterator it = m_effectSystemIDs.find( pEffectSystem->getIdentifier());
				//	if( it!=m_effectSystemIDs.end())
				//	{
				//		m_effectSystemIDs.erase( it);
				//	}
				//}	

				//{
				//	EffectSystemMap::iterator it = mEffectSystems.find( pEffectSystem->getName());
				//	if (it != mEffectSystems.end())
				//	{
				//		EchoAssert( pEffectSystem==it->second);

				//		EchoSafeDelete( pEffectSystem, EffectSystem);
				//		mEffectSystems.erase(it);
				//	}
				//}	

				EchoSafeDelete(pEffectSystem, EffectSystem);
			}
		}

		effectSystemID = 0;	
	}

	// 销毁所有特效系统
	void EffectSystemManager::destroyAllEffectSystems()
	{ 
		// Delete all Effect Systems. This doesnot include the templates
		EffectSystemMap::iterator t;
		for (t = mEffectSystems.begin(); t != mEffectSystems.end();)
		{
			EffectSystem* particleSystem = t->second;
			int id = particleSystem->getIdentifier();
			EchoSafeDelete(particleSystem, EffectSystem);
			t = mEffectSystems.erase(t);
			m_effectSystemIDs.erase(id);
		}
		//mEffectSystems.clear();
		//m_effectSystemIDs.clear();
	}
	//-----------------------------------------------------------------------
	EffectSystem* EffectSystemManager::_createSystemImpl(const String& name)
	{
		EffectSystem* sys = EchoNew(EffectSystem(name));

		mEffectSystems[name] = sys;
		m_effectSystemIDs[sys->getIdentifier()] = sys;
		return sys;
	}
	//-----------------------------------------------------------------------
	void EffectSystemManager::_destroySystemImpl(EffectSystem* particleSystem)
	{
		removeEffectSystemFromMap(particleSystem);

		//{
		//	EffectSystemIDMap::iterator it=m_effectSystemIDs.find( particleSystem->getIdentifier());
		//	if( it!=m_effectSystemIDs.end())
		//		m_effectSystemIDs.erase( it);
		//}

		//EffectSystemMap::iterator i = mEffectSystems.find(particleSystem->getName());
		//if (i != mEffectSystems.end())
		//{
		//	mEffectSystems.erase(i);
		//}

		EchoSafeDelete(particleSystem, EffectSystem);
	}

	void EffectSystemManager::createRenderableSet()
	{
		if(!mRenderalbeSetCreated)
		{
			RenderQueue* pRenderQueue = NULL;
			Material* pMaterial = NULL;
			ShaderProgram* shaderProgram = NULL;

			pRenderQueue = RenderQueueGroup::instance()->getRenderQueue("Effect");/*m_camera ? pSceneManager->getRenderQueue("EffectUI") :*/
			if (!pRenderQueue)
				return;

			pMaterial = pRenderQueue->getMaterial();
			shaderProgram = pMaterial->getShaderProgram();

			mSPFogParamIndex = shaderProgram->getParamPhysicsIndex("fogParam");
			mSPHeightFogParamIndex = shaderProgram->getParamPhysicsIndex("heightFogParam");
			mSPMatIndex = shaderProgram->getParamPhysicsIndex("matWVP");
			mSPTextureIndex = shaderProgram->getParamPhysicsIndex("texSampler");

			// 特效系统顶点格式
			m_vertexElementList.push_back(RenderInput::VertexElement(RenderInput::VS_POSITION, PF_RGB32_FLOAT));
			m_vertexElementList.push_back(RenderInput::VertexElement(RenderInput::VS_COLOR, PF_RGBA8_UNORM));
			m_vertexElementList.push_back(RenderInput::VertexElement(RenderInput::VS_TEXCOORD0, PF_RG32_FLOAT));
			m_vertexElementList.push_back(RenderInput::VertexElement(RenderInput::VS_TEXCOORD1, PF_RG32_FLOAT));

			mEffectRenderInput = Renderer::instance()->createRenderInput(shaderProgram);
			mEffectRenderInput->enableWireFrame(true);

			// Sampler State use default.
			SamplerState::SamplerDesc desc;
			mEffectSamplerState = Renderer::instance()->getSamplerState(desc);

			// AlphaBlendState
			BlendState::BlendDesc bsDescAlphaLighting;
			bsDescAlphaLighting.bBlendEnable = true;
			bsDescAlphaLighting.srcBlend = BlendState::BF_SRC_ALPHA;
			bsDescAlphaLighting.dstBlend = BlendState::BF_INV_SRC_ALPHA;
			bsDescAlphaLighting.srcAlphaBlend = BlendState::BF_SRC_ALPHA;
			bsDescAlphaLighting.dstAlphaBlend = BlendState::BF_INV_SRC_ALPHA;
			mEffectAlphaLighting = Renderer::instance()->createBlendState(bsDescAlphaLighting);

			// ColorBlendState
			BlendState::BlendDesc bsDescHighLighting;
			bsDescHighLighting.bBlendEnable = true;
			bsDescHighLighting.srcBlend = BlendState::BF_SRC_ALPHA;
			bsDescHighLighting.dstBlend = BlendState::BF_ONE;
			bsDescHighLighting.srcAlphaBlend = BlendState::BF_ONE;
			bsDescHighLighting.dstAlphaBlend = BlendState::BF_ZERO;
			mEffectHighLighting = Renderer::instance()->createBlendState(bsDescHighLighting);

			BlendState::BlendDesc bsDescWireFrameLighting;
			bsDescWireFrameLighting.bBlendEnable = false;
			mEffectWireFrameLighting = Renderer::instance()->createBlendState(bsDescWireFrameLighting);

			// Depth State
			DepthStencilState::DepthStencilDesc dssDesc;
			dssDesc.bWriteDepth	= false;
			dssDesc.bDepthEnable = true;
			mEffectDepthState = Renderer::instance()->createDepthStencilState(dssDesc);

			dssDesc.bWriteDepth = false;
			dssDesc.bDepthEnable = false;
			mEffectUnWriteDepthState = Renderer::instance()->createDepthStencilState(dssDesc);

			// Rasterizer State
			RasterizerState::RasterizerDesc rasDesc;
			rasDesc.cullMode = RasterizerState::CULL_NONE;
			mEffectRasterizerState = Renderer::instance()->createRasterizerState(rasDesc);

			// 初始化Buffer
			initBuffer();

			mRenderalbeSetCreated = true;

			// add the manual render process
			pRenderQueue->setManalRenderEnd(&mEffectManualRender);

			if( mMtlDistortion == NULL )
			{
				mMtlDistortion = MaterialManager::instance()->createMaterial();
				mMtlDistortion->loadFromFile("DistortionEffect.xml", "");
				
				EchoAssert( mMtlDistortion );

				ShaderProgram* pShaderProgram = mMtlDistortion->getShaderProgram();

				EchoAssert( pShaderProgram );

				pShaderProgram->getParamPhysicsIndex("matWVP");
				pShaderProgram->getParamPhysicsIndex("scSampler");
				pShaderProgram->getParamPhysicsIndex("noiseSampler");
				pShaderProgram->getParamPhysicsIndex("dissrotStrength");
			}

			if( mTextureNoise == NULL )
			{
				mTextureNoise = TextureResManager::instance()->createTexture("noise.jpg"); 
				mTextureNoise->prepareLoad();
			}

			if (mMtlUV2 == NULL)
			{
				mMtlUV2 = MaterialManager::instance()->createMaterial();
				mMtlUV2->loadFromFile("effect_uv2.xml","");
				mEffectUV2RenderInput = Renderer::instance()->createRenderInput(mMtlUV2->getShaderProgram());
				mEffectUV2RenderInput->enableWireFrame(true);
			}
		}
	}

	void EffectSystemManager::destroyRenderableSet()
	{
		if(mRenderalbeSetCreated)
		{
			EchoSafeDelete( m_vertexBuffer, GPUBuffer);
			EchoSafeDelete( m_indexBuffer, GPUBuffer);
			EchoSafeDelete(mEffectRasterizerState, RasterizerState);
			EchoSafeDelete(mEffectDepthState, DepthStencilState);
			EchoSafeDelete(mEffectUnWriteDepthState, DepthStencilState);
			EchoSafeDelete(mEffectHighLighting, BlendState);
			EchoSafeDelete(mEffectAlphaLighting, BlendState);
			EchoSafeDelete(mEffectWireFrameLighting, BlendState);

			destroyAllUsedRenderables();

			mEffectRenderInput->unbindGPUBuffers();
			EchoSafeDelete(mEffectRenderInput, RenderInput);

			if (mEffectUV2RenderInput)
			{
				mEffectUV2RenderInput->unbindGPUBuffers();
				EchoSafeDelete(mEffectUV2RenderInput, RenderInput);
			}

			mRenderalbeSetCreated = false;
		}
	}

	// 删除未使用的Renderable
	void EffectSystemManager::destroyAllUsedRenderables()
	{
		EffectRenderable* pRenderable = NULL;
		for (RenderableMap::iterator jt = m_usedRenderables.begin(); jt != m_usedRenderables.end(); ++jt)
		{
			pRenderable = IdToPtr(EffectRenderable, jt->second);
			EchoSafeDelete(pRenderable, EffectRenderable);
		}
		m_usedRenderables.clear();
	}

	// 设置是否开启扭曲效果
	void EffectSystemManager::setEnableDistortionRender( bool _val )
	{
		if( _val && !mEnableRender )
		{
			return ;
		}
	}

	// 清空渲染数据
	void EffectSystemManager::clearUsedRenderablesRenderData()
	{
		RenderableMap::iterator it;
		for (it = m_usedRenderables.begin(); it != m_usedRenderables.end(); ++it)
		{
			EffectRenderable* pRenderable = IdToPtr(EffectRenderable, it->second);
			pRenderable->clearGeometryData();
		}
	}

	// 根据纹理，特效组，混合模式，材质获取Renderable
	EffectRenderable* EffectSystemManager::getRenderable(TextureRes* texture, EFFECT_RENDER_GROUP rg, EFFECT_BLEND_MODE bm, EffectMaterial* pEm )
	{
		ui64 materialkey = (ui32)((ui64)texture);	//低32位用来保存texture指针
		materialkey &= 0xFFFFFFFF;					//将高32位置零
		
		if (bm == HighLightMode) materialkey |= 0x100000000;
		if (rg == ERG_GROUPUP)	 materialkey |= 0x200000000;

		if (pEm->getDistortionEnable())
		{
			materialkey |= 0x400000000;
		}

		if (pEm->getUV2Enable())
		{
			materialkey |= 0x800000000;
		}

		if (pEm->getGlowEnable())
		{
			materialkey |= 0x1000000000;
		}

		if (pEm->getIsUIEffect())
		{
			materialkey |= 0x2000000000;
		}

		if (!pEm->isTestDepth())
		{
			materialkey |= 0x4000000000;
		}

		if (pEm->isDissolution())
		{
			materialkey |= 0x8000000000;
		}

		materialkey |= (1LL << (32 + 16 + pEm->getRenderPriority()));

		EffectRenderable* pRenderable = NULL;
		RenderableMap::iterator it = m_usedRenderables.find(materialkey);
		if (it == m_usedRenderables.end())
		{
			ui32 epSizePerBatch = EngineSettingsMgr::instance()->getDefaultEffectParticleSizePerBatch();

			pRenderable = EchoNew(EffectRenderable);
			mEffectRenderablePtr = PtrMonitor<EffectRenderable>::m_ptrs;
			pRenderable->createVertexPool(epSizePerBatch * 4);
			pRenderable->createIndexPool(epSizePerBatch * 6);

			pRenderable->setMaterialParam(texture, rg, bm, pEm);
			pRenderable->clearGeometryData();

			m_usedRenderables[materialkey] = pRenderable->getIdentifier();
		}
		else
		{
			pRenderable = IdToPtr( EffectRenderable, it->second);
			pRenderable->refreshMaterialParam(pEm);
		}

		return pRenderable;
	}

	// 开始渲染
	void EffectSystemManager::beginRender()
	{
		if (EngineSettingsMgr::instance()->isEnableGlow() && !m_bRenderGlowOver)
		{
			return;
		}

		clearUsedRenderablesRenderData();
	}

	// 结束渲染
	void EffectSystemManager::endRender(const std::vector<EffectRenderable*>* effectRenderables)
	{
		if( !mEnableRender )
		{
			beginRender();
			return ;
		}

		//flush all renderalbes to draw.
		if (NULL == effectRenderables)
		{
			if (m_usedRenderables.empty())
				return;
		}
		else
		{
			if (effectRenderables->empty())
				return;
		}

		m_bRenderGlowOver = false;

		int current_target_id = RenderTargetManager::instance()->getInUsingRenderTargetID();

		if (Root::instance()->getEnableDistortion()
			&& RenderTargetManager::instance()->getRenderTargetByID(RTI_SceneColorMap)
			&& current_target_id != 0
			&& isHaveDistortionLayer())
		{
			
			RenderTargetManager::instance()->endRenderTarget(current_target_id);
			if (Root::instance()->getEnableFilterAdditional())
			{
				RenderTargetManager::instance()->updateRenderTargetWithFilter(current_target_id, RTI_SceneColorMap);
			}
			else
			{
				RenderTargetManager::instance()->updateRenderTarget(current_target_id, RTI_SceneColorMap);
			}

			RenderTargetManager::instance()->beginRenderTarget(current_target_id, false, Renderer::BGCOLOR, false, 1.0);
			renderDistortionLayer();
		}
	
		Renderer* pRender = Renderer::instance();
		//Camera* camera;// = getCamera();
		//SceneManager*  pSceneManager = SceneManager::instance();
		RenderQueue*   pRenderQueue  = _getEffectRenderQueue();
		Material*      pMaterial     = pRenderQueue->getMaterial();
		ShaderProgram* shaderProgram =  pMaterial->getShaderProgram();
		ShaderProgram* shaderProgramUV2 = mMtlUV2->getShaderProgram();
		int uv2_MatIndex = shaderProgramUV2->getParamPhysicsIndex("matWVP");
		int uv2_TextureIndex = shaderProgramUV2->getParamPhysicsIndex("texSampler");
		int uv2_TextureIndex1 = shaderProgramUV2->getParamPhysicsIndex("tex1Sampler");
		int uv2_blendModeIndex = shaderProgramUV2->getParamPhysicsIndex("blendMode");
		int uv2_dissolutionIndex = shaderProgramUV2->getParamPhysicsIndex("dissolution");
		
		//pRender->setDepthStencilState(mEffectDepthState);
		pRender->setRasterizerState(mEffectRasterizerState);

		//render all used renderables
		RenderableMap::iterator it;
		EffectRenderable* pRenderable;
		//EFFECT_BLEND_MODE blendMode;
		if (NULL == effectRenderables)
		{
			for (it = m_usedRenderables.begin(); it != m_usedRenderables.end(); ++it)
			{
				pRenderable = IdToPtr(EffectRenderable, it->second);
				if (NULL == pRenderable)
					continue;
				_renderRenderable(pRenderable, uv2_MatIndex, uv2_TextureIndex, uv2_TextureIndex1, uv2_blendModeIndex, uv2_dissolutionIndex);
				pRenderable->clearGeometryData();
				//else
				//{
				//	EchoLogError("EffectSystemManager::endRender failed...");
				//}
			}
		}
		else
		{
			for (size_t i = 0; i < effectRenderables->size(); ++i)
			{
				EffectRenderable* r = (*effectRenderables)[i];
				_renderRenderable(r, uv2_MatIndex, uv2_TextureIndex, uv2_TextureIndex1, uv2_blendModeIndex, uv2_dissolutionIndex);
				r->clearGeometryData();
			}
		}
		//if (!EchoRoot->getEnableDistortion())
		{
			//beginRender();
		}
	}

	void EffectSystemManager::endRenderUI(const std::vector<EffectRenderable*>* effectRenderables)
	{
		endRender(effectRenderables);
		//clearUsedRenderablesRenderData();
	}

	// 是否有空间扭曲特效层
	bool EffectSystemManager::isHaveDistortionLayer()
	{
		for(RenderableMap::iterator it=m_usedRenderables.begin(); it!=m_usedRenderables.end(); ++it)
		{
			EffectRenderable* pRenderable = IdToPtr(EffectRenderable, it->second);
			if (pRenderable && pRenderable->isDistortion())
				return true;
		}

		return false;
	}

	// 渲染扭曲
	void EffectSystemManager::renderDistortionLayer()
	{
		if( !mEnableRender ) return ;

		if(m_usedRenderables.empty())
			return;

		Renderer* pRender = Renderer::instance();
		SceneManager* pSceneManager = SceneManager::instance();
		RenderQueue* pRenderQueue = NULL;
		Material* pMaterial = NULL;
		ShaderProgram* shaderProgram = NULL;

		Texture* scTexture = RenderTargetManager::instance()->getRenderTargetByID(RTI_SceneColorMap)->getBindTexture();
		SamplerState::SamplerDesc desc;
		desc.addrUMode = SamplerState::AM_CLAMP;
		desc.addrVMode = SamplerState::AM_CLAMP;

		const Echo::SamplerState* samplerState = Renderer::instance()->getSamplerState(desc);

		TextureSampler scTexSampler( scTexture, samplerState);

		EchoAssert( scTexture );
		
		shaderProgram = mMtlDistortion->getShaderProgram();
		shaderProgram->bind();
		//pRender->setDepthStencilState(mEffectDepthState);

		//get the camera to fill the matrix.
		Camera* camera = SceneManager::instance()->getMainCamera();
		Matrix4 viewProjMat;

		viewProjMat = camera->getViewProjMatrix();

		shaderProgram->setUniform(shaderProgram->getParamPhysicsIndex("matWVP"), &viewProjMat, SPT_MAT4, 1);
		Renderer::instance()->setRasterizerState(mEffectRasterizerState);

		//render all used renderables
		RenderableMap::iterator it;
		EffectRenderable* pRenderable;
		EFFECT_BLEND_MODE blendMode;

		int scIndex = shaderProgram->getParamPhysicsIndex("scSampler");
		int noiseIndex = shaderProgram->getParamPhysicsIndex("noiseSampler");
		int strengthIndex = shaderProgram->getParamPhysicsIndex("dissrotStrength");
		int centerUVIndex = shaderProgram->getParamPhysicsIndex("centerUV");

		for(it=m_usedRenderables.begin(); it!=m_usedRenderables.end(); ++it)
		{
			pRenderable = IdToPtr( EffectRenderable, it->second);
			if (pRenderable)
			{
				if (!pRenderable->isDistortion()) continue;

				if (mRenderUIEffectOnly && !pRenderable->isUiEffect())
					continue;

				pRenderable->updateSinceCreateTime(pRenderable->getSinceCreateTime() + Root::instance()->getFrameTime());

				ui16 vertexNum = pRenderable->getVertexNum();
				if (vertexNum > 0)
				{
					blendMode = pRenderable->getBlendMode();

					shaderProgram->setUniform(noiseIndex, &SHADER_TEXTURE_SLOT0, SPT_TEXTURE, 1);
					shaderProgram->setUniform(scIndex, &SHADER_TEXTURE_SLOT1, SPT_TEXTURE, 1);

					float dissortStrength = pRenderable->getDistortionBlendWeight();
					shaderProgram->setUniform(strengthIndex, (const void *)&dissortStrength, SPT_FLOAT, 1);

					Vector2 centerUV = pRenderable->getCenterUV();
					if (pRenderable->getDistortionType() == DISTORTION_LINEAR)
					{
						centerUV = Vector2(0, 0);
					}

					shaderProgram->setUniform(centerUVIndex, (const void *)&centerUV, SPT_VEC2, 1);

					pRender->setTexture(0, pRenderable->getTextureSampler());
					pRender->setTexture(1, scTexSampler,true);

					if (pRenderable->isTestDepth())
						pRender->setDepthStencilState(mEffectDepthState);
					else
						pRender->setDepthStencilState(mEffectUnWriteDepthState);

					updateBufferData(*pRenderable);

					mEffectRenderInput->bindVertexStream(m_vertexElementList, m_vertexBuffer);
					mEffectRenderInput->bindIndexStream(m_indexBuffer, sizeof(ui16));
					mEffectRenderInput->setVertexCount(vertexNum);
					mEffectRenderInput->setIndexCount(pRenderable->getIndexNum());

					pRender->render(mEffectRenderInput, shaderProgram);
				}
			}		
		}
		//m_bRenderDistortionOver = true;
		//beginRender();
	}

	// 上传数据到GPU缓冲
	void EffectSystemManager::updateBufferData( const EffectRenderable& effectRenderable)
	{
		const vector<EffectVertexFmt>::type& vertexBufferData = effectRenderable.getVertexBufferData();
		const vector<ui16>::type&			 indexBufferData  = effectRenderable.getIndexBufferData();

		// const强转为非const(注意)
		Buffer vbuff( vertexBufferData.size() * sizeof(EffectVertexFmt), (void*)vertexBufferData.data(), false);
		m_vertexBuffer->updateData( vbuff);
		
		// const强转为非const(注意)
		Buffer iBuff( indexBufferData.size()*sizeof(ui16), (void*)indexBufferData.data());
		m_indexBuffer->updateData( iBuff);
	}

	// 渲染扭曲特效层
	void EffectSystemManager::renderGLowEffectLayer()
	{
		if (!mEnableRender)
		{
			beginRender();
			return;
		}

		//flush all renderalbes to draw.
		if (m_usedRenderables.empty())
			return;

		Renderer* pRender = Renderer::instance();
		Camera* camera;// = getCamera();
		RenderQueue*   pRenderQueue =  RenderQueueGroup::instance()->getRenderQueue("Effect");
		Material*      pMaterial = pRenderQueue->getMaterial();
		ShaderProgram* shaderProgram = pMaterial->getShaderProgram();
		ShaderProgram* shaderProgramUV2 = mMtlUV2->getShaderProgram();
		int uv2_MatIndex = shaderProgramUV2->getParamPhysicsIndex("matWVP");
		int uv2_TextureIndex = shaderProgramUV2->getParamPhysicsIndex("texSampler");
		int uv2_TextureIndex1 = shaderProgramUV2->getParamPhysicsIndex("tex1Sampler");
		int uv2_blendModeIndex = shaderProgramUV2->getParamPhysicsIndex("blendMode");

		//pRender->setDepthStencilState(mEffectDepthState);
		pRender->setRasterizerState(mEffectRasterizerState);

		//render all used renderables
		RenderableMap::iterator it;
		EffectRenderable* pRenderable;
		EFFECT_BLEND_MODE blendMode;
		for (it = m_usedRenderables.begin(); it != m_usedRenderables.end(); ++it)
		{
			pRenderable = IdToPtr(EffectRenderable, it->second);
			if (pRenderable)
			{
				if (!pRenderable->isGlow())
					continue;

				if (mRenderUIEffectOnly && !pRenderable->isUiEffect())
					continue;

				ui16 vertexNum = pRenderable->getVertexNum();
				if (vertexNum > 0)
				{
					camera = getCamera(pRenderable);
					blendMode = pRenderable->getBlendMode();

					if (!pRenderable->isUseUV2())
					{
						shaderProgram->bind();
						shaderProgram->setUniform(mSPMatIndex, &camera->getViewProjMatrix(), SPT_MAT4, 1);
						shaderProgram->setUniform(mSPTextureIndex, &SHADER_TEXTURE_SLOT0, SPT_TEXTURE, 1);
						pRender->setTexture(0, pRenderable->getTextureSampler());
					}
					else
					{
						shaderProgramUV2->bind();
						shaderProgramUV2->setUniform(uv2_MatIndex, &camera->getViewProjMatrix(), SPT_MAT4, 1);
						shaderProgramUV2->setUniform(uv2_TextureIndex, &SHADER_TEXTURE_SLOT0, SPT_TEXTURE, 1);
						shaderProgramUV2->setUniform(uv2_TextureIndex1, &SHADER_TEXTURE_SLOT1, SPT_TEXTURE, 1);
						shaderProgramUV2->setUniform(uv2_blendModeIndex, pRenderable->getUV2BlendMode(), SPT_VEC3, 2);
						pRender->setTexture(0, pRenderable->getTextureSampler());
						pRender->setTexture(1, pRenderable->getTextureSampler1());
					}

					if (bUseWireFrameMode)
					{
						pRender->setBlendState(mEffectWireFrameLighting);
					}
					else
					{
						if (blendMode == AlphaLightMode)
							pRender->setBlendState(mEffectAlphaLighting);
						else if (blendMode == HighLightMode)
							pRender->setBlendState(mEffectHighLighting);

						if (pRenderable->isTestDepth())
							pRender->setDepthStencilState(mEffectDepthState);
						else
							pRender->setDepthStencilState(mEffectUnWriteDepthState);
					}

					// 更新数据到GPUBuffer
					updateBufferData(*pRenderable);

					RenderInput* pRenderInput = pRenderable->isUseUV2() ? mEffectUV2RenderInput : mEffectRenderInput;

					pRenderInput->bindVertexStream(m_vertexElementList, m_vertexBuffer);
					pRenderInput->setVertexCount(vertexNum);
					pRenderInput->bindIndexStream(m_indexBuffer, sizeof(ui16));
					pRenderInput->setIndexCount(pRenderable->getIndexNum());

					pRender->render(pRenderInput, pRenderable->isUseUV2() ? shaderProgramUV2 : shaderProgram);
				}
			}
			else
			{
				EchoLogError("EffectSystemManager::endRender failed...");
			}
		}
		
		m_bRenderGlowOver = true;
		beginRender();
	}

	void EffectSystemManager::setWireFramePattern()
	{
		bUseWireFrameMode = !bUseWireFrameMode;
#ifdef ECHO_EDITOR_MODE
		if (bUseWireFrameMode)
		{
			Renderer::instance()->setPolygonFillMode(RasterizerState::PM_LINE);
		}
		else
		{
			Renderer::instance()->setPolygonFillMode(RasterizerState::PM_FILL);
		}
#endif
	}

	void EffectSystemManager::closeWireFramePattern()
	{
		bUseWireFrameMode = false;
#ifdef ECHO_EDITOR_MODE
		Renderer::instance()->setPolygonFillMode(RasterizerState::PM_FILL);
#endif
	}

	void EffectSystemManager::_renderRenderable(EffectRenderable* pRenderable, int uv2_MatIndex, int uv2_TextureIndex, int uv2_TextureIndex1, int uv2_blendModeIndex, int uv2_dissolutionIndex)
	{
		if (NULL == pRenderable)
		{
			EchoLogError("EffectSystemManager::endRender failed...");
			return;
		}

		if (pRenderable->isDistortion())
			return;

		if (mRenderUIEffectOnly && !pRenderable->isUiEffect())
			return;

		ui16 vertexNum = pRenderable->getVertexNum();
		if (vertexNum <= 0)
			return;

		RenderQueue* pRenderQueue = _getEffectRenderQueue();
		Camera* camera = getCamera(pRenderable);
		EFFECT_BLEND_MODE blendMode = pRenderable->getBlendMode();
		Renderer* pRender = Renderer::instance();
		Material*      pMaterial     = pRenderQueue->getMaterial();
		ShaderProgram* shaderProgram =  pMaterial->getShaderProgram();
		ShaderProgram* shaderProgramUV2 = mMtlUV2->getShaderProgram();

		if (!pRenderable->isUseUV2())
		{
			shaderProgram->bind();
			//if (!pRenderable->isUiEffect())
			//{
			//	shaderProgram->setUniform(mSPFogParamIndex, (void*)SceneManager::instance()->getFogParamVec4Ptr(), SPT_VEC4, 3);
			//	shaderProgram->setUniform(mSPHeightFogParamIndex, (void*)&SceneManager::instance()->getHeightFogParam(), SPT_VEC4, 1);
			//}
			//else
			//{
			//	shaderProgram->setUniform(mSPFogParamIndex, (void*)SceneManager::instance()->getUIFogParamVec4Ptr(), SPT_VEC4, 3);
			//	shaderProgram->setUniform(mSPHeightFogParamIndex, (void*)&SceneManager::instance()->getUIHeightFogParam(), SPT_VEC4, 1);
			//}

			shaderProgram->setUniform(mSPMatIndex, &camera->getViewProjMatrix(), SPT_MAT4, 1);
			shaderProgram->setUniform(mSPTextureIndex, &SHADER_TEXTURE_SLOT0, SPT_TEXTURE, 1);
			pRender->setTexture(0, pRenderable->getTextureSampler());
		}
		else
		{
			shaderProgramUV2->bind();
			shaderProgramUV2->setUniform(uv2_MatIndex, &camera->getViewProjMatrix(), SPT_MAT4, 1);
			shaderProgramUV2->setUniform(uv2_TextureIndex, &SHADER_TEXTURE_SLOT0, SPT_TEXTURE, 1);
			shaderProgramUV2->setUniform(uv2_TextureIndex1, &SHADER_TEXTURE_SLOT1, SPT_TEXTURE, 1);
			shaderProgramUV2->setUniform(uv2_blendModeIndex, pRenderable->getUV2BlendMode(), SPT_VEC3, 2);
			shaderProgramUV2->setUniform(uv2_dissolutionIndex, pRenderable->getDissolutionParameter(), SPT_VEC3, 1);
			pRender->setTexture(0, pRenderable->getTextureSampler());
			pRender->setTexture(1, pRenderable->getTextureSampler1());
		}

		if (bUseWireFrameMode)
		{
			pRender->setBlendState(mEffectWireFrameLighting);
		}
		else
		{
			if (blendMode == AlphaLightMode)
				pRender->setBlendState(mEffectAlphaLighting);
			else if (blendMode == HighLightMode)
				pRender->setBlendState(mEffectHighLighting);

			if (pRenderable->isTestDepth())
				pRender->setDepthStencilState(mEffectDepthState);
			else
				pRender->setDepthStencilState(mEffectUnWriteDepthState);
		}

		// 更新数据到GPUBuffer
		updateBufferData(*pRenderable);

		RenderInput* pRenderInput = pRenderable->isUseUV2() ? mEffectUV2RenderInput : mEffectRenderInput;

		pRenderInput->bindVertexStream(m_vertexElementList, m_vertexBuffer);
		pRenderInput->setVertexCount(vertexNum);
		pRenderInput->bindIndexStream(m_indexBuffer, sizeof(ui16));
		pRenderInput->setIndexCount(pRenderable->getIndexNum());

		pRender->render(pRenderInput, pRenderable->isUseUV2() ? shaderProgramUV2 : shaderProgram);
	} // end of EffectSystemManager::_renderRenderable

	RenderQueue* EffectSystemManager::_getEffectRenderQueue()
	{
		if (NULL == mEffectRenderQueue)
			mEffectRenderQueue = RenderQueueGroup::instance()->getRenderQueue("Effect");
		return mEffectRenderQueue;
	}

	void EffectSystemManager::removeEffectSystemFromMap(EffectSystem* pEffectSystem)
	{
		{
			EffectSystemIDMap::iterator it = m_effectSystemIDs.find(pEffectSystem->getIdentifier());
			if (it != m_effectSystemIDs.end())
			{
				m_effectSystemIDs.erase(it);
			}
		}

		{
		EffectSystemMap::iterator it = mEffectSystems.find(pEffectSystem->getName());
		if (it != mEffectSystems.end())
		{
			EchoAssert(pEffectSystem == it->second);

			//EchoSafeDelete(pEffectSystem, EffectSystem);
			mEffectSystems.erase(it);
		}
	}
	}

} //namespace Echo
