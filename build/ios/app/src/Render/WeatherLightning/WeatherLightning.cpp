#include "WeatherLightning.h"

namespace Examples
{
	WeatherLightning::WeatherLightning()
		: m_scene(NULL)
		, m_camera(NULL)
	{
		sceneName = "scene_dl_0002_denglujiemian";
	}

	WeatherLightning::~WeatherLightning()
	{
		m_scene = NULL;
		m_camera = NULL;
	}

	const LORD::String WeatherLightning::getHelp()
	{
		return "WeatherLightning";
	}

	bool WeatherLightning::initialize()
	{
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(2000.f);
		m_camera->setNearClip(10.f);
		m_camera->setPosition(LORD::Vector3(89.866f, 0.802f, 128.701f));
		m_camera->setDirection(LORD::Vector3( 0.008f, 0.345f, -0.938f));
		m_camera->update();
		m_scene = LordSceneManager->loadScene(sceneName, LORD::Vector3::ZERO);

		if (!m_scene)
			return false;

		return true;
	}

	void WeatherLightning::tick(LORD::ui32 elapsedTime)
	{

	}

	void WeatherLightning::destroy()
	{
		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_scene = NULL;
		m_camera = NULL;
	}

}