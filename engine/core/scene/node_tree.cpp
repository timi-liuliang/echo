#include "node_tree.h"
#include "engine/core/camera/Camera.h"

namespace Echo
{
	NodeTree::NodeTree()
		: m_3dCamera(nullptr)
		, m_2dCamera(nullptr)
		, m_uiCamera(nullptr)
		, m_shadowCamera(nullptr)
	{
        m_invisibleRoot = EchoNew(Node);
	}

	NodeTree::~NodeTree()
	{
        m_invisibleRoot->queueFree();
        
        EchoSafeDelete(m_shadowCamera, CameraShadow);
        EchoSafeDelete(m_2dCamera, Camera);
        EchoSafeDelete(m_3dCamera, Camera);
        EchoSafeDelete(m_uiCamera, Camera);
	}

	NodeTree* NodeTree::instance()
	{
		static NodeTree* inst = EchoNew(NodeTree);
		return inst;
	}

    // root node
    Node* NodeTree::getInvisibleRootNode()
    {
        return m_invisibleRoot;
    }
    
	bool NodeTree::init()
	{
		// create main camera
		m_3dCamera = EchoNew(Camera(Camera::PM_PERSPECTIVE));
		m_shadowCamera = EchoNew(CameraShadow);

		Vector3 vCamPos(0.f, 3.f, 3.f);
		Vector3 vCamDir = Vector3::ZERO - vCamPos;
		vCamDir.normalize();

		m_3dCamera->setPosition(vCamPos);
		m_3dCamera->setUp(Vector3::UNIT_Y);
		m_3dCamera->setDirection(vCamDir);
		m_3dCamera->setNearClip(0.1f);
		m_3dCamera->setFarClip(250.f);

		// create 2D camera
		Vector3 vCam2DPos(0, 0, 0);
		Vector3 vCam2DDir = -Vector3::UNIT_Z;
		m_2dCamera = EchoNew(Camera(Camera::PM_ORTHO));

		m_2dCamera->setPosition(vCam2DPos);
		m_2dCamera->setDirection(vCam2DDir);
		m_2dCamera->setNearClip(-256.f);
		m_2dCamera->setFarClip(256.0f);
		m_2dCamera->update();

		// create gui camera
		m_uiCamera = EchoNew(Camera(Camera::PM_ORTHO));

		m_uiCamera->setPosition(vCam2DPos);
		m_uiCamera->setDirection(vCam2DDir);
		m_uiCamera->setNearClip(-256.f);
		m_uiCamera->setFarClip(256.0f);
		m_uiCamera->update();

		update(0.001f);

		return true;
	}

	void NodeTree::update(float elapsedTime)
	{
		// update 3d camera
		m_2dCamera->update();
		m_3dCamera->update();
		m_uiCamera->update();
		
		// update nodes
		m_invisibleRoot->update(elapsedTime, true);

		// update scripts
		LuaBinder::instance()->execString("update_all_nodes()", true);
        
        // update channels
        Channel::syncAll();

		// render & test
		//Frustum frustum;
		//m_2dBvh.query(nullptr, frustum);
		//m_2dBvh.query(nullptr, AABB());
		//m_2dBvh.rayCast(nullptr, Vector3::ZERO, Vector3::ONE);
    }
}
