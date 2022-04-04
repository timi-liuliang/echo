#include "node_tree.h"
#include "engine/core/camera/Camera.h"

namespace Echo
{
	NodeTree::NodeTree()
	{
        m_invisibleRoot = EchoNew(Node);
	}

	NodeTree::~NodeTree()
	{
        m_invisibleRoot->queueFree();
        
        EchoSafeDelete(m_2dCamera, Camera);
        EchoSafeDelete(m_3dCamera, Camera);
        EchoSafeDelete(m_uiCamera, Camera);
	}

	NodeTree* NodeTree::instance()
	{
		static NodeTree* inst = EchoNew(NodeTree);
		return inst;
	}

    Node* NodeTree::getInvisibleRootNode()
    {
        return m_invisibleRoot;
    }
    
	bool NodeTree::init()
	{
		// create main camera
		m_3dCamera = EchoNew(Camera(Camera::ProjMode::PM_PERSPECTIVE));

		m_3dCamera->setPosition(Vector3::ZERO);
		m_3dCamera->setOrientation(Quaternion::fromPitchYawRoll(0.f, 0.f, 0.f));
		m_3dCamera->setNear(0.1f);
		m_3dCamera->setFar(250.f);
		m_3dCamera->update();

		// create 2D camera
		m_2dCamera = EchoNew(Camera(Camera::ProjMode::PM_ORTHO));
		m_2dCamera->setPosition(Vector3(0, 0, -257));
		m_2dCamera->setOrientation(Quaternion::fromAxisAngle(Vector3::UNIT_Y, Math::PI_DIV2));
		m_2dCamera->setNear(1);
		m_2dCamera->setFar(513.0f);
		m_2dCamera->update();

		// create gui camera
		m_uiCamera = EchoNew(Camera(Camera::ProjMode::PM_ORTHO));
		m_uiCamera->setPosition(Vector3(0, 0, -257));
		m_uiCamera->setOrientation(Quaternion::fromAxisAngle(Vector3::UNIT_Y, Math::PI_DIV2));
		m_uiCamera->setNear(1);
		m_uiCamera->setFar(513.0f);
		m_uiCamera->update();

		// Main render scene
		m_renderScene = EchoNew(RenderScene);

		update(0.001f);

		return true;
	}

	void NodeTree::update(float elapsedTime)
	{
		// update 3d camera
		m_2dCamera->update();
		m_3dCamera->update();
		m_uiCamera->update();

		m_renderScene->update(m_3dCamera->getFrustum());
		
		// update nodes
		m_invisibleRoot->update(elapsedTime, true);

		// update scripts
		LuaBinder::instance()->execString("update_all_nodes()", true);
        
        // update channels
        Channel::syncAll();
    }
}
