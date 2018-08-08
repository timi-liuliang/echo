#include "SpaceDistortion.h"

namespace Examples
{
	SpaceDistortion::SpaceDistortion()
		: m_scene(NULL)
		, m_camera(NULL)
	{
		sceneName = "nosieeffect";
	}

	SpaceDistortion::~SpaceDistortion()
	{
		m_scene = NULL;
		m_camera = NULL;
	}

	const LORD::String SpaceDistortion::getHelp()
	{
		return "SpaceDistortion";
	}

	bool SpaceDistortion::initialize()
	{
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(2000.f);
		m_camera->setNearClip(10.f);
		m_camera->setPosition(LORD::Vector3(226.45f, 29.94f, 139.94f));
		m_camera->setDirection(LORD::Vector3(-0.45f, -0.75f, -0.48f));
		m_camera->update();
		m_scene = LordSceneManager->loadScene(sceneName, LORD::Vector3::ZERO);

		if (!m_scene)
			return false;

		return true;
	}

	void SpaceDistortion::tick(LORD::ui32 elapsedTime)
	{

	}

	void SpaceDistortion::destroy()
	{
		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_scene = NULL;
		m_camera = NULL;
	}

}