#include "MultiThreadRendering.h"

namespace Examples
{
	static LORD::String SCENENAME = "zjtest2";

	MultiThreadRendering::MultiThreadRendering()
		: m_scene(NULL)
		, m_camera(NULL)
	{

	}

	MultiThreadRendering::~MultiThreadRendering()
	{
		m_scene = NULL;
		m_camera = NULL;
	}

	const LORD::String MultiThreadRendering::getHelp()
	{
		return "MultiThreadRendering";
	}

	bool MultiThreadRendering::initialize()
	{
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(300);
		m_camera->setNearClip(10);
		m_camera->setPosition(LORD::Vector3(11.0f, 40, -40.0f));
		m_camera->setDirection(LORD::Vector3(0.0f, -0.39f, 0.55f));
		m_camera->update();
		m_scene = LordSceneManager->loadScene(SCENENAME, LORD::Vector3::ZERO);

		if (!m_scene)
			return false;

		return true;
	}

	void MultiThreadRendering::tick(LORD::ui32 elapsedTime)
	{

	}

	void MultiThreadRendering::destroy()
	{
		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_scene = NULL;
		m_camera = NULL;
	}
}