#include "EffectOcean.h"

namespace Examples
{
	LORD::String SCENENAME = "map_jy_lingdichengbao";

	EffectOcean::EffectOcean()
		: m_scene(NULL)
		, m_camera(NULL)
	{

	}

	EffectOcean::~EffectOcean()
	{
		m_scene = NULL;
		m_camera = NULL;
	}

	const LORD::String EffectOcean::getHelp()
	{
		return "EffectOcean";
	}

	bool EffectOcean::initialize()
	{
		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(2000.f);
		m_camera->setNearClip(100.f);
		m_camera->setPosition(LORD::Vector3(-200.f,400.f,495));
		m_camera->setDirection(LORD::Vector3(0.59f, -0.59f, -0.55f));
		m_camera->update();
		m_scene = LordSceneManager->loadScene(SCENENAME, LORD::Vector3::ZERO);

		if (!m_scene)
			return false;

		return true;
	}

	void EffectOcean::tick(LORD::ui32 elapsedTime)
	{

	}

	void EffectOcean::destroy()
	{
		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_scene = NULL;
		m_camera = NULL;
	}

}