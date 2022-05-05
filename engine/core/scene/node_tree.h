#pragma once

#include "node.h"
#include "engine/core/gizmos/gizmos.h"
#include "engine/core/camera/camera.h"
#include "engine/core/render/base/scene/render_scene.h"

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

	private:
		NodeTree();

	protected:
		Camera*			    m_3dCamera = nullptr;
		Camera*				m_2dCamera = nullptr;
		Camera*				m_uiCamera = nullptr;
		RenderScenePtr		m_renderScene;				// Main render scene
        Node*				m_invisibleRoot = nullptr;	// Invisible root node
	};
}
