#pragma once

#include "node.h"
#include "bvh.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/camera/Camera.h"
#include "engine/core/camera/CameraShadow.h"

namespace Echo
{
	class NodeTree
	{	
	public:
		virtual ~NodeTree();

		// instance
		static NodeTree* instance();

		// init
		bool init();

		// root node
        Node* getInvisibleRootNode();

		// get bounding volume hierarchy accelerator
		Bvh& get2dBvh() { return m_2dBvh; }
		Bvh& get3dBvh() { return m_3dBvh; }

	public:
		// get main 3d camera
		Camera* get3dCamera() const { return m_3dCamera; }

		// set main 3d camera
		void set3dCamera(Camera* camera) { m_3dCamera = camera; }

		// get 2d camera
		Camera* get2dCamera() const { return m_2dCamera; }

		// get ui camera
		Camera* getUiCamera() const { return m_uiCamera; }

		// set ui camera
		void setUiCamera(Camera* pCamera) { m_uiCamera = pCamera; }

	public:
		void update( float elapsedTime);

		// get shadow camera
		CameraShadow& getShadowCamera() { EchoAssert( m_shadowCamera);  return *m_shadowCamera; }

	private:
		NodeTree();

	protected:
		Camera*			    m_3dCamera;
		Camera*				m_2dCamera;
		Camera*				m_uiCamera;
		CameraShadow*		m_shadowCamera;
		Bvh					m_2dBvh;
		Bvh					m_3dBvh;
        Node*				m_invisibleRoot = nullptr;	// invisible root node
	};
}
