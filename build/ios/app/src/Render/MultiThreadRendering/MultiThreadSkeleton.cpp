#include "MultiThreadSkeleton.h"
#include "Engine/Scene/Scene.h"
//#include "Extension/Actor/ActorObject.h"
//#include "Extension/Actor/ActorManager.h"

using namespace LORD;

namespace Examples
{
	static LORD::String SCENENAME = "zjtest";

	MultiThreadSkeleton::MultiThreadSkeleton()
		: m_scene(NULL)
		, m_camera(NULL)
		//, m_actorMgr(nullptr)
	{

	}

	MultiThreadSkeleton::~MultiThreadSkeleton()
	{
		m_scene = NULL;
		m_camera = NULL;
	}

	const LORD::String MultiThreadSkeleton::getHelp()
	{
		return "MultiThreadSkeleton";
	}

	bool MultiThreadSkeleton::initialize()
	{
		/*m_actorMgr = LordNew(LORD::ActorManager);
		LordRoot->setActorManager(m_actorMgr);*/

		m_camera = LordSceneManager->getMainCamera();
		m_camera->setFarClip(300);
		m_camera->setNearClip(10);
		m_camera->setPosition(LORD::Vector3(40.0f, 40, -40.0f));
		m_camera->setDirection(LORD::Vector3(0.0f, -0.39f, 0.55f));
		m_camera->update();
		m_scene = LordSceneManager->loadScene(SCENENAME, Vector3::ZERO);
		m_scene->setActorAmbientColor(Color(1.0f, 1.0f, 1.0f, 1.0f));

		if (!m_scene)
			return false;

/*
		const int ROW_COUNT = 20;
		const int COL_COUNT = 20;
		const float DIST = 3.0f;
		for (int row = 0; row < ROW_COUNT; ++row)
		{
			for (int col = -COL_COUNT/2; col < COL_COUNT/2; ++col)
			{
				LORD::ActorObject* actor = LORD::ActorManager::Instance()->CreateActor("m030_moxiang.actor", "run");
				Vector3 pos;
				pos.x = col * DIST;
				pos.y = 0;
				pos.z = row * DIST;
				actor->SetPosition(pos);
				actor->SetOrientation(LORD::Quaternion::IDENTITY);
			}
		}*/


		return true;
	}

	void MultiThreadSkeleton::tick(LORD::ui32 elapsedTime)
	{

	}

	void MultiThreadSkeleton::destroy()
	{
		LordSceneManager->closeScene();
		LordSceneManager->getMainCamera()->setFov(LORD::Math::PI_DIV4);
		m_scene = NULL;
		m_camera = NULL;

		//LordRoot->setActorManager(nullptr);
		//LordSafeDelete(m_actorMgr);
	}
}