#include "NodeTree.h"
#include "engine/core/camera/Camera.h"

namespace Echo
{
	// 构造函数
	NodeTree::NodeTree()
		: m_3dCamera(nullptr)
		, m_2dCamera(nullptr)
		, m_uiCamera(nullptr)
		, m_shadowCamera(nullptr)
	{
	}

	NodeTree::~NodeTree()
	{
	}

	// instance
	NodeTree* NodeTree::instance()
	{
		static NodeTree* inst = EchoNew(NodeTree);
		return inst;
	}

	// 初始化
	bool NodeTree::init()
	{
		// create main camera
		m_3dCamera = EchoNew(Camera(Camera::PM_PERSPECTIVE));
		m_shadowCamera = EchoNew(CameraShadow);

		Vector3 vCamPos(0, 100, 100);
		Vector3 vCamDir = Vector3::ZERO - vCamPos;
		vCamDir.normalize();

		m_3dCamera->setPosition(vCamPos);
		m_3dCamera->setDirection(vCamDir);

		// create 2D camera
		Vector3 vCam2DPos(0, 0, 0);
		Vector3 vCam2DDir = -Vector3::UNIT_Z;
		m_2dCamera = EchoNew(Camera(Camera::PM_ORTHO));

		m_2dCamera->setPosition(vCam2DPos);
		m_2dCamera->setDirection(vCam2DDir);
		m_2dCamera->setNearClip(-256.f);
		m_2dCamera->setFarClip(256.0f);
		m_2dCamera->update();

		update(0.001f);

		return true;
	}

	// 销毁
	void NodeTree::destroy()
	{
		EchoSafeDelete(m_3dCamera, Camera);
		EchoSafeDelete(m_2dCamera, Camera);
		EchoSafeDelete(m_shadowCamera, CameraShadow);
	}

	// 更新 
	void NodeTree::update(float elapsedTime)
	{
		m_invisibleRoot.update(elapsedTime, true);

		// update scripts
		luaex::LuaEx::instance()->callf("_update_all_nodes");
	}
}
