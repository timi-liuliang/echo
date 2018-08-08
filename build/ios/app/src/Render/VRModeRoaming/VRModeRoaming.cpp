#include "VRModeRoaming.h"
#include "Engine/Object/Root.h"
#include "Engine/Scene/Scene.h"

namespace Examples
{
	VRModeRoaming::VRModeRoaming()
		: m_scene(NULL)
		, m_camera(NULL)
		, pCamAnim(NULL)
	{
		m_sceneName = "scene_fb_0505_dixin";

#ifdef LORD_EDITOR_MODE
		LordAssert(false && "Please Close Editor Mode! Then Try Again!");
#endif
	}

	VRModeRoaming::~VRModeRoaming()
	{
		m_scene = NULL;
		m_camera = NULL;
		LordEngineSettings.setUserVRMode(false);
	}

	const LORD::String VRModeRoaming::getHelp()
	{
		return "VRModeRoaming";
	}

	bool VRModeRoaming::initialize()
	{
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(2000.f);
		m_camera->setNearClip(10.f);
		m_camera->setPosition(LORD::Vector3(89.866f, 0.802f, 128.701f));
		m_camera->setDirection(LORD::Vector3( 0.008f, 0.345f, -0.938f));
		m_scene = LordSceneManager->loadScene(m_sceneName, LORD::Vector3::ZERO);

		if (!m_scene)
			return false;

		LordEngineSettings.setUserVRMode(true);
		
		pCamAnim = m_scene->importCameraAnimation("scene_fb_0505_dixin.camAnim");
		if (!pCamAnim)
			return false;
		pCamAnim->play(LORD::CameraAnimation::LOOPING);

		return true;
	}

	void VRModeRoaming::tick(LORD::ui32 elapsedTime)
	{

	}

	void VRModeRoaming::destroy()
	{
		pCamAnim->stop();
		m_scene->destroyCameraAnimation(pCamAnim);

		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
	}

}