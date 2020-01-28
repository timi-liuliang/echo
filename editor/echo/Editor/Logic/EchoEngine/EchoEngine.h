#pragma once

#include <string>
#include <engine/core/log/Log.h>
#include <engine/core/main/Engine.h>
#include <engine/core/scene/node_tree.h>
#include <engine/core/render/base/mesh/Mesh.h>

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

		// 每帧渲染
		void Render( float elapsedTime, bool isRenderWindowVisible);

		// 修改窗口大小
		void Resize(int cx, int cy);

		// 设置project
		static bool SetProject( const char* projectFile);

	public:
		// on open node tree
		bool onOpenNodeTree(const Echo::String& resPath);

		// current edit node
		void setCurrentEditNode(Echo::Node* node);
		Echo::Node* getCurrentEditNode() { return m_currentEditNode; }

		// save path
		void setCurrentEditNodeSavePath(const Echo::String& savePath);
		const Echo::String& getCurrentEditNodeSavePath() { return m_currentEditNodeSavePath; }

	public:
		// 预览声音
		virtual void previewAudioEvent( const char* audioEvent);

		// 停止正在预览的声源
		virtual void stopCurPreviewAudioEvent();

		//设置显示或隐藏背景网格
		virtual void setBackGridVisibleOrNot(bool showFlag);

		// resize grid
		void resizeBackGrid3d();
		void resizeBackGrid2d();

		// 获取模型半径
		float GetMeshRadius();

		// 保存缩略图
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
		Echo::LogOutput*		m_log;
		Echo::Node*				m_currentEditNode;
		Echo::String			m_currentEditNodeSavePath;
		Echo::Node*				m_invisibleNodeForEditor;
		Echo::Gizmos*			m_gizmosNodeGrid3d;
		Echo::Gizmos*			m_gizmosNodeGrid2d;
		static std::string		m_projectFile;
		static RenderWindow*	m_renderWindow;
	};
}
