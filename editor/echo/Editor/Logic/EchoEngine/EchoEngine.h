#pragma once

#include <string>
#include <engine/core/log/Log.h>
#include <engine/core/main/Engine.h>
#include <engine/core/scene/node_tree.h>
#include <engine/core/render/base/mesh/mesh.h>

namespace Studio
{
	class RenderWindow; 
	class EchoEngine
	{
	public:
		virtual ~EchoEngine();

		// inst
		static EchoEngine* instance();

		// initialize
		bool Initialize( size_t hwnd);

		// render
		void Render( float elapsedTime, bool isRenderWindowVisible);

		// resize
		void Resize(int cx, int cy);

		// set project
		static bool SetProject( const char* projectFile);

	public:
		// on open node tree
		bool onOpenNodeTree(const Echo::String& resPath);

		// get invisible editor root node
		Echo::Node* getInvisibleEditorNode() { return m_invisibleNodeForEditor; }

		// current edit node
		void setCurrentEditNode(Echo::Node* node);
		Echo::Node* getCurrentEditNode() { return m_currentEditNode; }

		// save path
		void setCurrentEditNodeSavePath(const Echo::String& savePath);
		const Echo::String& getCurrentEditNodeSavePath() { return m_currentEditNodeSavePath; }

	public:
		// resize grid
		void resizeBackGrid3d();
		void resizeBackGrid2d();

		// save thumbnail
		void SaveSceneThumbnail(bool setCam = true);

	public:
		// new
		void newEditNodeTree();

		// save current node tree
		void saveCurrentEditNodeTree();
		void saveCurrentEditNodeTreeAs(const Echo::String& savePath);

		// save branch node as scene
		void saveBranchAsScene(const Echo::String& savePath, Echo::Node* node);

	private:
		EchoEngine();

		// init back grid
		void InitializeBackGrid();

	protected:
		Echo::Node*				m_currentEditNode = nullptr;
		Echo::String			m_currentEditNodeSavePath;
		Echo::Node*				m_invisibleNodeForEditor = nullptr;
		Echo::Gizmos*			m_gizmosNodeGrid3d = nullptr;
		Echo::Gizmos*			m_gizmosNodeGrid2d = nullptr;
		static std::string		m_projectFile;
		static RenderWindow*	m_renderWindow;
	};
}
