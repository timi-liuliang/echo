#pragma once

#include <string>
#include <engine/core/log/Log.h>
#include <engine/core/main/Engine.h>
#include <engine/core/scene/node_tree.h>
#include <engine/core/render/interface/mesh/Mesh.h>

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

		// 卸载
		void Release();

	public:
		// on open node tree
		bool onOpenNodeTree(const Echo::String& resPath);

		// set current edit node
		void setCurrentEditNode(Echo::Node* node);

		// 获取当前编辑结点
		Echo::Node* getCurrentEditNode() { return m_currentEditNode; }

		// 设置当前结点树存储路径
		void setCurrentEditNodeSavePath(const Echo::String& savePath);

		// 获取当前结点树存储路径
		const Echo::String& getCurrentEditNodeSavePath() { return m_currentEditNodeSavePath; }

	public:
		// 预览声音
		virtual void previewAudioEvent( const char* audioEvent);

		// 停止正在预览的声源
		virtual void stopCurPreviewAudioEvent();

		//设置显示或隐藏背景网格
		virtual void setBackGridVisibleOrNot(bool showFlag);

		//调整背景网格的参数
		void resizeBackGrid3d();
		void resizeBackGrid2d();

		// 获取模型半径
		float GetMeshRadius();

		// 保存缩略图
		void SaveSceneThumbnail(bool setCam = true);

		// 场景相关
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

		// 初始化背景网格
		void InitializeBackGrid();

	protected:
		Echo::LogOutput*		m_log;						// 日志
		Echo::Node*				m_currentEditNode;			// 当前编辑场景
		Echo::String			m_currentEditNodeSavePath;	// 当前场景存储路径
		Echo::Node*				m_invisibleNodeForEditor;	// 编辑器结点
		Echo::Gizmos*			m_gizmosNodeBackGrid;
		Echo::Gizmos*			m_gizmosNodeGrid2d;			// 2d 框
		static std::string		m_projectFile;				// 项目名称
		static RenderWindow*	m_renderWindow;				// 渲染窗口
	};
}
