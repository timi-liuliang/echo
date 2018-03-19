#include "Scene_Manager.h"
#include "engine/core/render/render/Renderer.h"
#include "engine/core/Util/LogManager.h"
#include "Engine/core/main/Root.h"
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
		: m_pMainCamera(NULL)
		, m_p2DCamera(NULL)
		, m_pGUICamera(NULL)
		, m_pShadowCamera(NULL)
		, m_bNeedUpate(false)
		, m_invisibleRoot(nullptr)
	{
		__ConstructSingleton;
	}

	SceneManager::~SceneManager()
	{
		__DestructSingleton;
	}

	// 初始化
	bool SceneManager::initialize()
	{
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
	}

	// 更新 
	void SceneManager::update(float elapsedTime)
	{
		if (m_invisibleRoot)
		{
			m_invisibleRoot->update(true);
		}
	}
}
