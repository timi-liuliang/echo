#include "Scene_Manager.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/Util/LogManager.h"
#include "Engine/core/main/Root.h"
#include "scene.h"
#include "engine/core/render/render/Material.h"
#include "Engine/core/Resource/EchoThread.h"
#include "Engine/modules/Anim/Animation.h"
#include "Engine/modules/Effect/EffectSystemManager.h"
#include "Engine/modules/Model/MeshManager.h"
#include "Engine/modules/Anim/AnimManager.h"
#include "Engine/modules/Anim/SkeletonManager.h"
#include "engine/core/render/RenderStage/PostProcessRenderStage.h"
#include "engine/core/render/RenderStage/RenderStageManager.h"
#include "Engine/modules/Audio/FMODStudio/FSAudioManager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_helper.hpp"
#include "EngineSettings.h"

using namespace rapidxml;

namespace Echo
{
	__ImplementSingleton(SceneManager);

	// 构造函数
	SceneManager::SceneManager()
		: m_isSceneVisible(true)
		, m_pMainCamera(NULL)
		, m_p2DCamera(NULL)
		, m_pGUICamera(NULL)
		, m_pShadowCamera(NULL)
		, m_bNeedUpate(false)
		, m_bIsRenderActorHigh(false)
		, m_isUseXRay(false)
		, m_bisComputeActorFog(false)
		, m_mainActorPos(Vector3::ZERO)
	{
		__ConstructSingleton;

		memset(m_LightParam, 0, sizeof(Vector4) * 3);
		memset(m_FogParam, 0, sizeof(Vector4) * 9);
		memset(m_UIFogParam, 0, sizeof(Vector4) * 3);
		memset(m_LightParamForActor, 0, sizeof(Vector4) * 3);

		m_heightFogParam = Vector4(-1000.0f, 1.0f, 0.0f, 0.0f);
		m_UIheightFogParam = Vector4::ZERO;
		// add by huangteng
		// 避免shader出现除0，不同机型出现不同表现
		m_UIFogParam[0].y = 1.0f;
		m_UIheightFogParam.y = 1.0f;
		// z值作为高度雾开关
		m_UIheightFogParam.z = 0.0f;
		m_heightFogParam.z = 1.0f;
	}

	SceneManager::~SceneManager()
	{
		__DestructSingleton;
	}

	// 初始化渲染队列
	void SceneManager::initRenderQueueGroup()
	{
		MemoryReader memReader("renderqueuedefine.xml");

		xml_document<> doc;        // character type defaults to cha
		doc.parse<0>(memReader.getData<char*>());

		xml_node<>* pRenderQueueGroupNode = doc.first_node();
		if (!pRenderQueueGroupNode)
		{
			EchoLogInfo("invalid render queue define file.");
		}
		else
		{
			xml_node<>* pRenderQueueNode = pRenderQueueGroupNode->first_node();
			while (pRenderQueueNode)
			{
				String	strRenderQueueName;
				String	strMaterialName;

				for (xml_attribute<>* attr = pRenderQueueNode->first_attribute(); attr; attr = attr->next_attribute())
				{
					if (0 == strcmp(attr->name(), "name"))
						strRenderQueueName = attr->value();
					else if (0 == strcmp(attr->name(), "material"))
						strMaterialName = attr->value();
				}

				RenderQueue*	pRenderQueue = EchoNew(RenderQueue)(strRenderQueueName);
				Material*		pMaterial = pRenderQueue->getMaterial();

				if (!strMaterialName.empty())
				{
					pMaterial->loadFromFile(strMaterialName, "");
					if (pMaterial->hasMacro("FILTER_COLOR"))
					{
					}
				}

				m_RenderQueueGroup.push_back(pRenderQueue);

				pRenderQueueNode = pRenderQueueNode->next_sibling();
			}
		}
	}

	// 根据材质模板添加渲染队列
	RenderQueue* SceneManager::addRenderQueue(const String& materialTemplate, const char* stage, const String& macros, bool isAlphaTest)
	{
		String queueName = String(stage) + "@" + String(isAlphaTest ? "AlphaTest" : "_") + "@" + macros + "@" + materialTemplate;
		RenderQueue* renderQueue = getRenderQueue(queueName);
		if (!renderQueue)
		{
			renderQueue = EchoNew(RenderQueue)(queueName);
			Material* pMaterial = renderQueue->getMaterial();
			pMaterial->loadFromFile(materialTemplate, macros);

			int offset = getRenderQueueIndex(stage) + 1;
			m_RenderQueueGroup.insert(m_RenderQueueGroup.begin() + offset, renderQueue);
		}

		return renderQueue;
	}

	// 删除渲染队列
	void SceneManager::destroyRenderQueueGroup()
	{
		EchoSafeDeleteContainer(m_RenderQueueGroup, RenderQueue);
	}

	// 初始化
	bool SceneManager::initialize()
	{
		initRenderQueueGroup();

		// create main camera
		m_pMainCamera = EchoNew(CameraMain(Camera::PM_PERSPECTIVE));
		m_pShadowCamera = EchoNew(CameraShadow);

		Vector3 vCamPos(0, 100, 100);
		Vector3 vCamDir = Vector3::ZERO - vCamPos;
		vCamDir.normalize();

		m_pMainCamera->setPosition(vCamPos);
		m_pMainCamera->setDirection(vCamDir);

		// create 2D camera
		Vector3 vCam2DPos(0, 0, 1);
		Vector3 vCam2DDir = -Vector3::UNIT_Z;
		m_p2DCamera = EchoNew(Camera(Camera::PM_ORTHO));

		m_p2DCamera->setPosition(vCam2DPos);
		m_p2DCamera->setDirection(vCam2DDir);
		m_p2DCamera->setNearClip(0.1f);
		m_p2DCamera->setFarClip(100.0f);
		m_p2DCamera->update();

		update(0.001f);

		return true;
	}

	// 销毁
	void SceneManager::destroy()
	{
		EchoSafeDelete(m_pMainCamera, Camera);
		EchoSafeDelete(m_p2DCamera, Camera);
		EchoSafeDelete(m_pShadowCamera, CameraShadow);

		destroyRenderQueueGroup();
	}

	// 根据渲染队列名称获取渲染队列
	RenderQueue* SceneManager::getRenderQueue(const String& strQueueName) const
	{
		RenderQueue* pRenderQueue = NULL;
		size_t nCount = m_RenderQueueGroup.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			if (strQueueName == m_RenderQueueGroup[i]->getName())
			{
				pRenderQueue = m_RenderQueueGroup[i];
				break;
			}
		}

		return pRenderQueue;
	}

	// 根据索引获取渲染队列
	RenderQueue* SceneManager::getRenderQueueByIndex(ui32 nIndex) const
	{
		if (nIndex >= m_RenderQueueGroup.size())
		{
			return NULL;
		}
		else
		{
			return m_RenderQueueGroup[nIndex];
		}
	}

	// 根据名称获取队列索引
	ui8 SceneManager::getRenderQueueIndex(const String& strQueueName) const
	{

		size_t nCount = m_RenderQueueGroup.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			if (strQueueName == m_RenderQueueGroup[i]->getName())
			{
				return i;
			}
		}

		EchoLogError("Can not found RenderQueue [%s]", strQueueName.c_str());
		return -1;
	}

	// 渲染队列执行(包含endQueue)
	void SceneManager::execRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue)
	{
		// 立即处理渲染队列中数据(直接渲染)
		size_t beginIdx = getRenderQueueIndex(startQueue);
		size_t endIdx = getRenderQueueIndex(endQueue); 
		endIdx = includeEndQueue ? endIdx : endIdx - 1;
		for (size_t i = beginIdx; i <= endIdx; ++i)
		{
			Echo::RenderQueue* renderQuene = getRenderQueueByIndex(i);
			if (renderQuene)
				renderQuene->renderQueue();
		}
	}

	// 清空渲染队列(包含endQueue)
	void SceneManager::clearRenderQueue(const String& startQueue, const String& endQueue, bool includeEndQueue)
	{
		// 立即处理渲染队列中数据(直接渲染)
		size_t beginIdx = getRenderQueueIndex(startQueue);
		size_t endIdx = getRenderQueueIndex(endQueue);
		endIdx = includeEndQueue ? endIdx : endIdx - 1;
		for (size_t i = beginIdx; i <= endIdx; ++i)
		{
			Echo::RenderQueue* renderQuene = getRenderQueueByIndex(i);
			if (renderQuene)
				renderQuene->clearRenderables();
		}
	}


	// 更新 
	void SceneManager::update(float elapsedTime)
	{
		static Box visibleShadowAABB;

		// 更新摄像机
		m_pMainCamera->frameMove(elapsedTime);
		// VR Mode需要手动更新
		if (!(EngineSettingsMgr::instance()->isInitVRMode() && EngineSettingsMgr::instance()->isUseVRMode()))
			m_pMainCamera->update();

		// 更新所有Actors
		if (EngineSettingsMgr::instance()->isActorCastShadow())
		{
			// update Shadow AABB Begine
			visibleShadowAABB.reset();

			// 更新Model包围盒
			//if (Root::instance()->getActorManagerUnsafe())
			//{
			//	ModelManager::instance()->buildBoxSM(visibleShadowAABB, EchoEngineActorManager->getMainActor() ? EchoEngineActorManager->getMainActorPosition() : SceneManager::instance()->getMainPosition(), m_pMainCamera, m_shadowDistance);
			//	if (visibleShadowAABB.isValid())
			//	{
			//		visibleShadowAABB.vMin -= Vector3(0.1f, 0.1f, 0.1f);
			//		visibleShadowAABB.vMax += Vector3(0.1f, 0.1f, 0.1f);
			//	}
			//}

			m_pShadowCamera->update(&visibleShadowAABB);
			// update Shadow AABB end

			// actors and breakables real update
			//if (Root::instance()->getActorManagerUnsafe())
			//	EchoEngineActorManager->updateActors(elapsedTime, m_pMainCamera, NULL);
		}
		else
		{
			//if (Root::instance()->getActorManagerUnsafe())
			//	EchoEngineActorManager->updateActors(elapsedTime, m_pMainCamera);
		}

		// 雾效过渡插值
		//m_FogParam[0][0] = m_FogParam[1][0] * (1.f - m_fog1ToFog2) + m_FogParam[2][0] * m_fog1ToFog2;
		//m_FogParam[0][1] = m_FogParam[1][1] * (1.f - m_fog1ToFog2) + m_FogParam[2][1] * m_fog1ToFog2;
		//m_FogParam[0][2] = m_FogParam[1][2];
	}

	void SceneManager::render()
	{
		size_t nCount = m_RenderQueueGroup.size();
		for (size_t i = 0; i < nCount; ++i)
		{
			if (m_RenderQueueGroup[i])
			{
				m_RenderQueueGroup[i]->renderQueue();
				m_RenderQueueGroup[i]->beginRender();
			}
		}
	}
}
